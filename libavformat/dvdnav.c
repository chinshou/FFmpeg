/* $Id: dvd.c,v 1.12 2005/11/25 15:05:25 titer Exp $

   This file is part of the HandBrake source code.
   Homepage: <http://handbrake.fr/>.
   It may be used under the terms of the GNU General Public License. */
#include <stdio.h>
#include <stdlib.h>

//#include "hb.h"
#include "lang.h"
#include "dvd.h"

#include "../libavutil/log.h"
#include "../libavutil/avstring.h"


#include <dvdnav/dvdnav.h>
#include <dvdread/ifo_read.h>
#include <dvdread/ifo_print.h>
#include <dvdread/nav_read.h>
#include "dvdint.h"

#define DVD_READ_CACHE 1

char        * hb_dvdnav_name( char * path );
#if 0
static hb_dvd_t    * hb_dvdnav_init( char * path );
static int           hb_dvdnav_title_count( hb_dvd_t * d );
static hb_title_t  * hb_dvdnav_title_scan( hb_dvd_t * d, int t );
static int           hb_dvdnav_start( hb_dvd_t * d, hb_title_t *title, int chapter );
static void          hb_dvdnav_stop( hb_dvd_t * d );
static int           hb_dvdnav_seek( hb_dvd_t * d, float f );
static int           hb_dvdnav_read( hb_dvd_t * d, hb_buffer_t * b );
static int           hb_dvdnav_chapter( hb_dvd_t * d );
static void          hb_dvdnav_close( hb_dvd_t ** _d );
static int           hb_dvdnav_angle_count( hb_dvd_t * d );
static void          hb_dvdnav_set_angle( hb_dvd_t * e, int angle );

hb_dvd_func_t hb_dvdnav_func =
{
    hb_dvdnav_init,
    hb_dvdnav_close,
    hb_dvdnav_name,
    hb_dvdnav_title_count,
    hb_dvdnav_title_scan,
    hb_dvdnav_start,
    hb_dvdnav_stop,
    hb_dvdnav_seek,
    hb_dvdnav_read,
    hb_dvdnav_chapter,
    hb_dvdnav_angle_count,
    hb_dvdnav_set_angle
};

/***********************************************************************
 * Local prototypes
 **********************************************************************/

hb_dvd_func_t * hb_dvdnav_methods( void )
{
    return &hb_dvdnav_func;
}

#endif

// there can be at most 999 PGCs per title. round that up to the nearest
// power of two.
#define MAX_PGCN 1024

static int FindChapterIndex( hb_list_t * list, int pgcn, int pgn );
static int FindNextCell( pgc_t *pgc, int cell_cur );
static void FindReadNextCell( hb_dvdread_t * d );

static void PgcWalkInit( uint32_t pgcn_map[MAX_PGCN/32] );
static int NextPgcn( ifo_handle_t *ifo, int pgcn, uint32_t pgcn_map[MAX_PGCN/32] );
static int dvdtime2msec( dvd_time_t * );

#define HB_LIST_DEFAULT_SIZE 20

/**********************************************************************
 * hb_list_init
 **********************************************************************
 * Allocates an empty list ready for HB_LIST_DEFAULT_SIZE items
 *********************************************************************/
static hb_list_t * hb_list_init()
{
    hb_list_t * l;

    l              = av_malloc( sizeof( hb_list_t ));
	memset(l, 0, sizeof(hb_list_t));
    l->items       = av_malloc( HB_LIST_DEFAULT_SIZE * sizeof( void * ));
	memset(l->items, 0, HB_LIST_DEFAULT_SIZE * sizeof( void * ));
    l->items_alloc = HB_LIST_DEFAULT_SIZE;

    return l;
}

/**********************************************************************
 * hb_list_count
 **********************************************************************
 * Returns the number of items currently in the list
 *********************************************************************/
static int hb_list_count( hb_list_t * l )
{
    return l->items_count;
}

/**********************************************************************
 * hb_list_add
 **********************************************************************
 * Adds an item at the end of the list, making it bigger if necessary.
 * Can safely be called with a NULL pointer to add, it will be ignored.
 *********************************************************************/
static void hb_list_add( hb_list_t * l, void * p )
{
    if( !p )
    {
        return;
    }

    if( l->items_count == l->items_alloc )
    {
        /* We need a bigger boat */
        l->items_alloc += HB_LIST_DEFAULT_SIZE;
        l->items        = av_realloc( l->items,
                                   l->items_alloc * sizeof( void * ) );
    }

    l->items[l->items_count] = p;
    (l->items_count)++;
}

/**********************************************************************
 * hb_list_item
 **********************************************************************
 * Returns item at position i, or NULL if there are not that many
 * items in the list
 *********************************************************************/
void * hb_list_item( hb_list_t * l, int i )
{
    if( i < 0 || i >= l->items_count )
    {
        return NULL;
    }

    return l->items[i];
}

/**********************************************************************
 * hb_list_close
 **********************************************************************
 * Free memory allocated by hb_list_init. Does NOT free contents of
 * items still in the list.
 *********************************************************************/
static void hb_list_close( hb_list_t ** _l )
{
    hb_list_t * l = *_l;

    av_free( l->items );
    av_free( l );

    *_l = NULL;
}


/**********************************************************************
 * hb_title_init
 **********************************************************************
 *
 *********************************************************************/
static hb_title_t * hb_title_init( char * dvd, int index )
{
    hb_title_t * t;

    t = av_malloc( sizeof( hb_title_t ) );
    memset(t, 0, sizeof(hb_title_t));

    t->index         = index;
    t->list_audio    = hb_list_init();
    t->list_chapter  = hb_list_init();
    t->list_subtitle = hb_list_init();
    av_strlcat( t->dvd, dvd, sizeof(t->dvd) );
    // default to decoding mpeg2
    t->video_id      = 0xE0;
    t->video_codec   = WORK_DECMPEG2;

    return t;
}

static void hb_list_rem( hb_list_t * l, void * p )
{
    int i;

    /* Find the item in the list */
    for( i = 0; i < l->items_count; i++ )
    {
        if( l->items[i] == p )
        {
            break;
        }
    }

    /* Shift all items after it sizeof( void * ) bytes earlier */
    memmove( &l->items[i], &l->items[i+1],
             ( l->items_count - i - 1 ) * sizeof( void * ) );

    (l->items_count)--;
}

void hb_title_close( hb_title_t ** _t )
{
    hb_title_t * t = *_t;
    hb_audio_t * audio;
    hb_chapter_t * chapter;
    hb_subtitle_t * subtitle;
#if 0
    hb_attachment_t * attachment;
#endif

    while( ( audio = hb_list_item( t->list_audio, 0 ) ) )
    {
        hb_list_rem( t->list_audio, audio );
        av_free( audio );
    }
    hb_list_close( &t->list_audio );

    while( ( chapter = hb_list_item( t->list_chapter, 0 ) ) )
    {
        hb_list_rem( t->list_chapter, chapter );
        av_free( chapter );
    }
    hb_list_close( &t->list_chapter );

    while( ( subtitle = hb_list_item( t->list_subtitle, 0 ) ) )
    {
        hb_list_rem( t->list_subtitle, subtitle );
#if 0
        if ( subtitle->extradata )
        {
            free( subtitle->extradata );
            subtitle->extradata = NULL;
        }
#endif
        av_free( subtitle );
    }
    hb_list_close( &t->list_subtitle );
 #if 0   
    while( ( attachment = hb_list_item( t->list_attachment, 0 ) ) )
    {
        hb_list_rem( t->list_attachment, attachment );
        if ( attachment->name )
        {
            free( attachment->name );
            attachment->name = NULL;
        }
        if ( attachment->data )
        {
            free( attachment->data );
            attachment->data = NULL;
        }
        free( attachment );
    }
    hb_list_close( &t->list_attachment );
#endif
    if( t->metadata )
    {
        if( t->metadata->coverart )
        {
            av_free( t->metadata->coverart );
        }
        av_free( t->metadata );
    }

    av_free( t );
    *_t = NULL;
}


char * hb_dvdnav_name( char * path )
{
    static char name[1024];
    unsigned char unused[1024];
    dvd_reader_t * reader;

    reader = DVDOpen( path );
    if( !reader )
    {
        return NULL;
    }

    if( DVDUDFVolumeInfo( reader, name, sizeof( name ),
                          unused, sizeof( unused ) ) )
    {
        DVDClose( reader );
        return NULL;
    }

    DVDClose( reader );
    return name;
}


/***********************************************************************
 * hb_dvdnav_reset
 ***********************************************************************
 * Once dvdnav has entered the 'stopped' state, it can not be revived
 * dvdnav_reset doesn't work because it doesn't remember the path
 * So this function re-opens dvdnav
 **********************************************************************/
int hb_dvdnav_reset( hb_dvdnav_t * d )
{
    if ( d->dvdnav ) 
        dvdnav_close( d->dvdnav );

    /* Open device */
    if( dvdnav_open(&d->dvdnav, d->path) != DVDNAV_STATUS_OK )
    {
        /*
         * Not an error, may be a stream - which we'll try in a moment.
         */
        av_log(NULL, AV_LOG_ERROR, "dvd: not a dvd - trying as a stream/file instead\n" );
        goto fail;
    }

    if (dvdnav_set_readahead_flag(d->dvdnav, DVD_READ_CACHE) !=
        DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_ERROR,"Error: dvdnav_set_readahead_flag: %s\n",
                 dvdnav_err_to_string(d->dvdnav));
        goto fail;
    }

    /*
     ** set the PGC positioning flag to have position information
     ** relatively to the whole feature instead of just relatively to the
     ** current chapter 
     **/
    if (dvdnav_set_PGC_positioning_flag(d->dvdnav, 1) != DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_ERROR,"Error: dvdnav_set_PGC_positioning_flag: %s\n",
                 dvdnav_err_to_string(d->dvdnav));
        goto fail;
    }
    return 1;

fail:
    if( d->dvdnav ) dvdnav_close( d->dvdnav );
    return 0;
}

int hb_dvd_region(char *device, int *region_mask)
{
#if defined( DVD_LU_SEND_RPC_STATE ) && defined( DVD_AUTH )
    struct stat  st;
    dvd_authinfo ai;
    int          fd, ret;

    fd = open( device, O_RDONLY );
    if ( fd < 0 )
        return -1;
    if ( fstat( fd, &st ) < 0 )
	{
        close( fd );
        return -1;
	}
    if ( !( S_ISBLK( st.st_mode ) || S_ISCHR( st.st_mode ) ) )
	{
        close( fd );
        return -1;
	}

    ai.type = DVD_LU_SEND_RPC_STATE;
    ret = ioctl(fd, DVD_AUTH, &ai);
    close( fd );
    if ( ret < 0 )
        return ret;

    *region_mask = ai.lrpcs.region_mask;
    return 0;
#else
    return -1;
#endif
}


/***********************************************************************
 * hb_dvdnav_init
 ***********************************************************************
 *
 **********************************************************************/
hb_dvd_t *hb_dvdnav_init( char * path)
{
    hb_dvd_t* e;
    hb_dvdnav_t * d;
    int region_mask;

    e = av_malloc( sizeof( hb_dvd_t ) );
	memset(e, 0, sizeof(hb_dvd_t));
	
    d = &(e->dvdnav);

	/* Log DVD drive region code */
    if ( hb_dvd_region( path, &region_mask ) == 0 )
    {
        av_log(NULL, AV_LOG_INFO, "dvd: Region mask 0x%02x\n", region_mask );
        if ( region_mask == 0xFF )
        {
            av_log(NULL, AV_LOG_WARNING,"dvd: Warning, DVD device has no region set\n" );
        }
    }

    /* Open device */
    if( dvdnav_open(&d->dvdnav, path) != DVDNAV_STATUS_OK )
    {
        /*
         * Not an error, may be a stream - which we'll try in a moment.
         */
        av_log(NULL, AV_LOG_INFO, "dvd:%s not a dvd - trying as a stream/file instead\n", path );
        goto fail;
    }

    if (dvdnav_set_readahead_flag(d->dvdnav, DVD_READ_CACHE) !=
        DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_ERROR,"Error: dvdnav_set_readahead_flag: %s\n",
                 dvdnav_err_to_string(d->dvdnav));
        goto fail;
    }

    /*
     ** set the PGC positioning flag to have position information
     ** relatively to the whole feature instead of just relatively to the
     ** current chapter 
     **/
    if (dvdnav_set_PGC_positioning_flag(d->dvdnav, 1) != DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_ERROR,"Error: dvdnav_set_PGC_positioning_flag: %s\n",
                 dvdnav_err_to_string(d->dvdnav));
        goto fail;
    }

    /* Open device */
    if( !( d->reader = DVDOpen( path ) ) )
    {
        /*
         * Not an error, may be a stream - which we'll try in a moment.
         */
        av_log(NULL, AV_LOG_INFO, "dvd: not a dvd - trying as a stream/file instead" );
        goto fail;
    }

    /* Open main IFO */
    if( !( d->vmg = ifoOpen( d->reader, 0 ) ) )
    {
        av_log(NULL, AV_LOG_ERROR, "dvd: ifoOpen failed" );
        goto fail;
    }

    d->path = av_strdup( path );

    return e;

fail:
    if( d->dvdnav ) dvdnav_close( d->dvdnav );
    if( d->vmg )    ifoClose( d->vmg );
    if( d->reader ) DVDClose( d->reader );
    av_free( e );
    return NULL;
}



/***********************************************************************
 * hb_dvdnav_title_count
 **********************************************************************/
int hb_dvdnav_title_count( hb_dvd_t * e )
{
    int titles = 0;
    hb_dvdnav_t * d = &(e->dvdnav);

    dvdnav_get_number_of_titles(d->dvdnav, &titles);
    return titles;
}


static uint64_t
PttDuration(ifo_handle_t *ifo, int ttn, int pttn, int *blocks, int *last_pgcn)
{
    int            pgcn, pgn;
    pgc_t        * pgc;
    uint64_t       duration = 0;
    int            cell_start, cell_end;
    int            i;

    // Initialize map of visited pgc's to prevent loops
    uint32_t pgcn_map[MAX_PGCN/32];
    PgcWalkInit( pgcn_map );
    pgcn   = ifo->vts_ptt_srpt->title[ttn-1].ptt[pttn-1].pgcn;
    pgn   = ifo->vts_ptt_srpt->title[ttn-1].ptt[pttn-1].pgn;
    if ( pgcn < 1 || pgcn > ifo->vts_pgcit->nr_of_pgci_srp || pgcn >= MAX_PGCN)
    {
        av_log(NULL, AV_LOG_ERROR, "invalid PGC ID %d, skipping\n", pgcn );
        return 0;
    }

    if( pgn <= 0 || pgn > 99 )
    {
        av_log(NULL, AV_LOG_ERROR, "scan: pgn %d not valid, skipping\n", pgn );
        return 0;
    }

    *blocks = 0;
    do
    {
        pgc = ifo->vts_pgcit->pgci_srp[pgcn-1].pgc;
        if (!pgc)
        {
            av_log(NULL, AV_LOG_ERROR, "scan: pgc not valid, skipping\n" );
            break;
        }
        if (pgn > pgc->nr_of_programs)
        {
            pgn = 1;
            continue;
        }

        duration += 90LL * dvdtime2msec( &pgc->playback_time );

        cell_start = pgc->program_map[pgn-1] - 1;
        cell_end = pgc->nr_of_cells - 1;
        for(i = cell_start; i <= cell_end; i = FindNextCell(pgc, i))
        {
            *blocks += pgc->cell_playback[i].last_sector + 1 -
                pgc->cell_playback[i].first_sector;
        }
        *last_pgcn = pgcn;
        pgn = 1;
    } while((pgcn = NextPgcn(ifo, pgcn, pgcn_map)) != 0);
    return duration;
}

/***********************************************************************
 * hb_dvdnav_title_scan
 **********************************************************************/
hb_title_t * hb_dvdnav_title_scan( hb_dvd_t * e, int t )
{

    hb_dvdnav_t * d = &(e->dvdnav);
    hb_title_t   * title;
    ifo_handle_t * ifo = NULL;
    int            pgcn, pgn, pgcn_end, i, c;
    int            title_pgcn;
    pgc_t        * pgc;
    int            cell_cur;
    hb_chapter_t * chapter;
    int            count;
    uint64_t       duration, longest;
    int            longest_pgcn, longest_pgn, longest_pgcn_end;
    float          duration_correction;
    const char   * name;

    av_log(NULL, AV_LOG_INFO, "scan: scanning title %d\n", t );

    title = hb_title_init( d->path, t );
    if (dvdnav_get_title_string(d->dvdnav, &name) == DVDNAV_STATUS_OK)
    {
        av_strlcpy( title->name, name, sizeof( title->name ) );
    }
    else
    {
        char * p_cur, * p_last = d->path;
        for( p_cur = d->path; *p_cur; p_cur++ )
        {
            if( p_cur[0] == '/' && p_cur[1] )
            {
                p_last = &p_cur[1];
            }
        }
        snprintf( title->name, sizeof( title->name ), "%s", p_last );
    }

    /* VTS which our title is in */
    title->vts = d->vmg->tt_srpt->title[t-1].title_set_nr;

    if ( !title->vts )
    {
        /* A VTS of 0 means the title wasn't found in the title set */
        av_log(NULL, AV_LOG_ERROR,"Invalid VTS (title set) number: %i\n", title->vts);
        goto fail;
    }

    av_log(NULL, AV_LOG_INFO, "scan: opening IFO for VTS %d\n", title->vts );
    if( !( ifo = ifoOpen( d->reader, title->vts ) ) )
    {
        av_log(NULL, AV_LOG_ERROR, "scan: ifoOpen failed\n" );
        goto fail;
    }

    /* ignore titles with bogus cell addresses so we don't abort later
     ** in libdvdread. */
    for ( i = 0; i < ifo->vts_c_adt->nr_of_vobs; ++i)
    {
        if( (ifo->vts_c_adt->cell_adr_table[i].start_sector & 0xffffff ) ==
            0xffffff )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: cell_adr_table[%d].start_sector invalid (0x%x) "
                      "- skipping title\n", i,
                      ifo->vts_c_adt->cell_adr_table[i].start_sector );
            goto fail;
        }
        if( (ifo->vts_c_adt->cell_adr_table[i].last_sector & 0xffffff ) ==
            0xffffff )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: cell_adr_table[%d].last_sector invalid (0x%x) "
                      "- skipping title\n", i,
                      ifo->vts_c_adt->cell_adr_table[i].last_sector );
            goto fail;
        }
        if( ifo->vts_c_adt->cell_adr_table[i].start_sector >=
            ifo->vts_c_adt->cell_adr_table[i].last_sector )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: cell_adr_table[%d].start_sector (0x%x) "
                      "is not before last_sector (0x%x) - skipping title\n", i,
                      ifo->vts_c_adt->cell_adr_table[i].start_sector,
                      ifo->vts_c_adt->cell_adr_table[i].last_sector );
            goto fail;
        }
    }

#if 0
    if( global_verbosity_level == 3 )
    {
        ifo_print( d->reader, title->vts );
    }
#endif	

    /* Position of the title in the VTS */
    title->ttn = d->vmg->tt_srpt->title[t-1].vts_ttn;
    if ( title->ttn < 1 || title->ttn > ifo->vts_ptt_srpt->nr_of_srpts )
    {
        av_log(NULL, AV_LOG_ERROR, "invalid VTS PTT offset %d for title %d, skipping\n", title->ttn, t );
        goto fail;
    }

    longest = 0LL;
    longest_pgcn = -1;
    longest_pgn = 1;
    longest_pgcn_end = -1;
    pgcn_end = -1;
    for( i = 0; i < ifo->vts_ptt_srpt->title[title->ttn-1].nr_of_ptts; i++ )
    {
        int blocks = 0;

        duration = PttDuration(ifo, title->ttn, i+1, &blocks, &pgcn_end);
        pgcn  = ifo->vts_ptt_srpt->title[title->ttn-1].ptt[i].pgcn;
        pgn   = ifo->vts_ptt_srpt->title[title->ttn-1].ptt[i].pgn;
        if( duration > longest )
        {
            longest_pgcn  = pgcn;
            longest_pgn   = pgn;
            longest_pgcn_end   = pgcn_end;
            longest = duration;
            title->block_count = blocks;
        }
        else if (pgcn == longest_pgcn && pgn < longest_pgn)
        {
            longest_pgn   = pgn;
            title->block_count = blocks;
        }
    }

    /* ignore titles under 10 seconds because they're often stills or
     * clips with no audio & our preview code doesn't currently handle
     * either of these. */
    if( longest < 900000LL )
    {
        av_log(NULL, AV_LOG_INFO, "scan: ignoring title (too short)\n" );
        goto fail;
    }

    pgcn       = longest_pgcn;
    pgcn_end   = longest_pgcn_end;
    pgn        = longest_pgn;;
    title_pgcn = pgcn;


    /* Get pgc */
    if ( pgcn < 1 || pgcn > ifo->vts_pgcit->nr_of_pgci_srp || pgcn >= MAX_PGCN)
    {
        av_log(NULL, AV_LOG_ERROR, "invalid PGC ID %d for title %d, skipping\n", pgcn, t );
        goto fail;
    }

    pgc = ifo->vts_pgcit->pgci_srp[pgcn-1].pgc;

    av_log(NULL, AV_LOG_INFO,"pgc_id: %d, pgn: %d: pgc: %p\n", pgcn, pgn, pgc);
    if (pgn > pgc->nr_of_programs)
    {
        av_log(NULL, AV_LOG_ERROR, "invalid PGN %d for title %d, skipping\n", pgn, t );
        goto fail;
    }

    /* Title start */
    title->cell_start = pgc->program_map[pgn-1] - 1;
    title->block_start = pgc->cell_playback[title->cell_start].first_sector;

    pgc = ifo->vts_pgcit->pgci_srp[pgcn_end-1].pgc;

    /* Title end */
    title->cell_end = pgc->nr_of_cells - 1;
    title->block_end = pgc->cell_playback[title->cell_end].last_sector;

    av_log(NULL, AV_LOG_INFO, "scan: vts=%d, ttn=%d, cells=%d->%d, blocks=%d->%d, "
            "%d blocks\n", title->vts, title->ttn, title->cell_start,
            title->cell_end, title->block_start, title->block_end,
            title->block_count );

    /* Get duration */
    title->duration = longest;
    title->hours    = title->duration / 90000 / 3600;
    title->minutes  = ( ( title->duration / 90000 ) % 3600 ) / 60;
    title->seconds  = ( title->duration / 90000 ) % 60;
    av_log(NULL, AV_LOG_ERROR, "scan: duration is %02d:%02d:%02d (%"PRId64" ms)\n",
            title->hours, title->minutes, title->seconds,
            title->duration / 90 );

#if 1
    /* Detect languages */
    for( i = 0; i < ifo->vtsi_mat->nr_of_vts_audio_streams; i++ )
    {
        hb_audio_t * audio, * audio_tmp;
        int          audio_format, lang_code, audio_control,
                     position, j;
        iso639_lang_t * lang;
        int lang_extension = 0;

        av_log(NULL, AV_LOG_INFO,"scan: checking audio %d\n", i + 1 );

        audio = av_malloc( sizeof( hb_audio_t ));
        memset(audio, 0, sizeof(hb_audio_t));

        audio_format  = ifo->vtsi_mat->vts_audio_attr[i].audio_format;
        lang_code     = ifo->vtsi_mat->vts_audio_attr[i].lang_code;
        lang_extension = ifo->vtsi_mat->vts_audio_attr[i].code_extension;
        audio_control =
            ifo->vts_pgcit->pgci_srp[title_pgcn-1].pgc->audio_control[i];

        if( !( audio_control & 0x8000 ) )
        {
            av_log(NULL, AV_LOG_INFO, "scan: audio channel is not active\n" );
            av_free( audio );
            continue;
        }

        position = ( audio_control & 0x7F00 ) >> 8;

        switch( audio_format )
        {
            case 0x00:
                audio->id    = ( ( 0x80 + position ) << 8 ) | 0xbd;
                audio->config.in.codec = HB_ACODEC_AC3;
                break;

            case 0x02:
            case 0x03:
                audio->id    = 0xc0 + position;
                audio->config.in.codec = HB_ACODEC_MPGA;
                break;

            case 0x04:
                audio->id    = ( ( 0xa0 + position ) << 8 ) | 0xbd;
                audio->config.in.codec = HB_ACODEC_LPCM;
                break;

            case 0x06:
                audio->id    = ( ( 0x88 + position ) << 8 ) | 0xbd;
                audio->config.in.codec = HB_ACODEC_DCA;
                break;

            default:
                audio->id    = 0;
                audio->config.in.codec = 0;
                av_log(NULL, AV_LOG_INFO, "scan: unknown audio codec (%x)\n",
                        audio_format );
                break;
        }
        if( !audio->id )
        {
            continue;
        }

        /* Check for duplicate tracks */
        audio_tmp = NULL;
        for( j = 0; j < hb_list_count( title->list_audio ); j++ )
        {
            audio_tmp = hb_list_item( title->list_audio, j );
            if( audio->id == audio_tmp->id )
            {
                break;
            }
            audio_tmp = NULL;
        }
        if( audio_tmp )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: duplicate audio track\n" );
            av_free( audio );
            continue;
        }

        audio->config.lang.type = lang_extension;

        lang = lang_for_code( ifo->vtsi_mat->vts_audio_attr[i].lang_code );

        snprintf( audio->config.lang.description, sizeof( audio->config.lang.description ), "%s (%s)",
            strlen(lang->native_name) ? lang->native_name : lang->eng_name,
            audio->config.in.codec == HB_ACODEC_AC3 ? "AC3" : ( audio->config.in.codec ==
                HB_ACODEC_DCA ? "DTS" : ( audio->config.in.codec ==
                HB_ACODEC_MPGA ? "MPEG" : "LPCM" ) ) );
        snprintf( audio->config.lang.simple, sizeof( audio->config.lang.simple ), "%s",
                  strlen(lang->native_name) ? lang->native_name : lang->eng_name );
        snprintf( audio->config.lang.iso639_2, sizeof( audio->config.lang.iso639_2 ), "%s",
                  lang->iso639_2);

        switch( lang_extension )
        {
        case 0:
        case 1:
            break;
        case 2:
            av_strlcat( audio->config.lang.description, " (Visually Impaired)", sizeof(audio->config.lang.description) );
            break;
        case 3:
            av_strlcat( audio->config.lang.description, " (Director's Commentary 1)",sizeof(audio->config.lang.description) );
            break;
        case 4:
            av_strlcat( audio->config.lang.description, " (Director's Commentary 2)" , sizeof(audio->config.lang.description) );
            break;
        default:
            break;
        }

        av_log(NULL, AV_LOG_INFO, "scan: id=%x, lang=%s, 3cc=%s ext=%i\n", audio->id,
                audio->config.lang.description, audio->config.lang.iso639_2,
                lang_extension );

        audio->config.in.track = i;
        hb_list_add( title->list_audio, audio );
    }
#endif
    memcpy( title->palette,
            ifo->vts_pgcit->pgci_srp[title_pgcn-1].pgc->palette,
            16 * sizeof( uint32_t ) );

    /* Check for subtitles */
    for( i = 0; i < ifo->vtsi_mat->nr_of_vts_subp_streams; i++ )
    {
        hb_subtitle_t * subtitle;
        int spu_control;
        int position;
        iso639_lang_t * lang;
        int lang_extension = 0;

        av_log(NULL, AV_LOG_INFO, "scan: checking subtitle %d\n", i + 1 );

        spu_control =
            ifo->vts_pgcit->pgci_srp[title_pgcn-1].pgc->subp_control[i];

        if( !( spu_control & 0x80000000 ) )
        {
            av_log(NULL, AV_LOG_INFO, "scan: subtitle channel is not active\n" );
            continue;
        }

        if( ifo->vtsi_mat->vts_video_attr.display_aspect_ratio )
        {
            switch( ifo->vtsi_mat->vts_video_attr.permitted_df )
            {
                case 1:
                    position = spu_control & 0xFF;
                    break;
                case 2:
                    position = ( spu_control >> 8 ) & 0xFF;
                    break;
                default:
                    position = ( spu_control >> 16 ) & 0xFF;
            }
        }
        else
        {
            position = ( spu_control >> 24 ) & 0x7F;
        }

        lang_extension = ifo->vtsi_mat->vts_subp_attr[i].code_extension;

        lang = lang_for_code( ifo->vtsi_mat->vts_subp_attr[i].lang_code );

        subtitle = calloc( sizeof( hb_subtitle_t ), 1 );
        subtitle->track = i+1;
        subtitle->id = ( ( 0x20 + position ) << 8 ) | 0xbd;
        snprintf( subtitle->lang, sizeof( subtitle->lang ), "%s",
             strlen(lang->native_name) ? lang->native_name : lang->eng_name);
        snprintf( subtitle->iso639_2, sizeof( subtitle->iso639_2 ), "%s",
                  lang->iso639_2);
        subtitle->format = PICTURESUB;
        subtitle->source = VOBSUB;
        subtitle->config.dest   = RENDERSUB;  // By default render (burn-in) the VOBSUB.

        subtitle->type = lang_extension;

        switch( lang_extension )
        {  
        case 0:
            break;
        case 1:
            break;
        case 2:
            av_strlcat( subtitle->lang, " (Caption with bigger size character)", sizeof(subtitle->lang));
            break;
        case 3: 
            av_strlcat( subtitle->lang, " (Caption for Children)", sizeof(subtitle->lang));
            break;
        case 4:
            break;
        case 5:
            av_strlcat( subtitle->lang, " (Closed Caption)", sizeof(subtitle->lang));
            break;
        case 6:
            av_strlcat( subtitle->lang, " (Closed Caption with bigger size character)", sizeof(subtitle->lang));
            break;
        case 7:
            av_strlcat( subtitle->lang, " (Closed Caption for Children)", sizeof(subtitle->lang));
            break;
        case 8:
            break;
        case 9:
            av_strlcat( subtitle->lang, " (Forced Caption)", sizeof(subtitle->lang));
            break;
        case 10:
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            av_strlcat( subtitle->lang, " (Director's Commentary)", sizeof(subtitle->lang));
            break;
        case 14:
            av_strlcat( subtitle->lang, " (Director's Commentary with bigger size character)", sizeof(subtitle->lang));
            break;
        case 15:
            av_strlcat( subtitle->lang, " (Director's Commentary for Children)", sizeof(subtitle->lang));
        default:
            break;
        }

        av_log(NULL, AV_LOG_INFO, "scan: id=%x, lang=%s, 3cc=%s\n", subtitle->id,
                subtitle->lang, subtitle->iso639_2 );

        hb_list_add( title->list_subtitle, subtitle );
    }

    /* Chapters */
    uint32_t pgcn_map[MAX_PGCN/32];
    PgcWalkInit( pgcn_map );
    c = 0;
    do
    {
        pgc = ifo->vts_pgcit->pgci_srp[pgcn-1].pgc;

        for (i = pgn; i <= pgc->nr_of_programs; i++)
        {
            chapter = calloc( sizeof( hb_chapter_t ), 1 );

            chapter->index = c + 1;
            chapter->pgcn = pgcn;
            chapter->pgn = i;
            hb_list_add( title->list_chapter, chapter );
            c++;
        }

        pgn = 1;
    } while ((pgcn = NextPgcn(ifo, pgcn, pgcn_map)) != 0);

    av_log(NULL, AV_LOG_INFO, "scan: title %d has %d chapters\n", t, c );

    duration = 0;
    count = hb_list_count( title->list_chapter );
    for (i = 0; i < count; i++)
    {
        chapter = hb_list_item( title->list_chapter, i );

        pgcn = chapter->pgcn;
        pgn = chapter->pgn;
        pgc = ifo->vts_pgcit->pgci_srp[pgcn-1].pgc;

        /* Start cell */
        chapter->cell_start  = pgc->program_map[pgn-1] - 1;
        chapter->block_start = pgc->cell_playback[chapter->cell_start].first_sector;
        // if there are no more programs in this pgc, the end cell is the
        // last cell. Otherwise it's the cell before the start cell of the
        // next program.
        if ( pgn == pgc->nr_of_programs )
        {
            chapter->cell_end = pgc->nr_of_cells - 1;
        }
        else
        {
            chapter->cell_end = pgc->program_map[pgn] - 2;;
        }
        chapter->block_end = pgc->cell_playback[chapter->cell_end].last_sector;

        /* Block count, duration */
        chapter->block_count = 0;
        chapter->duration = 0;

        cell_cur = chapter->cell_start;
        while( cell_cur <= chapter->cell_end )
        {
#define cp pgc->cell_playback[cell_cur]
            chapter->block_count += cp.last_sector + 1 - cp.first_sector;
            chapter->duration += 90LL * dvdtime2msec( &cp.playback_time );
#undef cp
            cell_cur = FindNextCell( pgc, cell_cur );
        }
        duration += chapter->duration;
    }

    /* The durations we get for chapters aren't precise. Scale them so
       the total matches the title duration */
    duration_correction = (float) title->duration / (float) duration;
    for( i = 0; i < hb_list_count( title->list_chapter ); i++ )
    {
        int seconds;
        chapter            = hb_list_item( title->list_chapter, i );
        chapter->duration  = duration_correction * chapter->duration;
        seconds            = ( chapter->duration + 45000 ) / 90000;
        chapter->hours     = seconds / 3600;
        chapter->minutes   = ( seconds % 3600 ) / 60;
        chapter->seconds   = seconds % 60;

        av_log(NULL, AV_LOG_INFO, "scan: chap %d c=%d->%d, b=%d->%d (%d), %"PRId64" ms\n",
                chapter->index, chapter->cell_start, chapter->cell_end,
                chapter->block_start, chapter->block_end,
                chapter->block_count, chapter->duration / 90 );
    }

    /* Get aspect. We don't get width/height/rate infos here as
       they tend to be wrong */
    switch( ifo->vtsi_mat->vts_video_attr.display_aspect_ratio )
    {
        case 0:
            title->container_aspect = 4. / 3.;
            break;
        case 3:
            title->container_aspect = 16. / 9.;
            break;
        default:
            av_log(NULL, AV_LOG_INFO, "scan: unknown aspect\n" );
            goto fail;
    }

    av_log(NULL, AV_LOG_INFO, "scan: aspect = %g\n", title->aspect );

    /* This title is ok so far */
    goto cleanup;

fail:
    hb_list_close( &title->list_audio );
    av_free( title );
    title = NULL;

cleanup:
    if( ifo ) ifoClose( ifo );

    return title;
}



/***********************************************************************
 * hb_dvdnav_start
 ***********************************************************************
 * Title and chapter start at 1
 **********************************************************************/
int hb_dvdnav_start( hb_dvd_t * e, hb_title_t *title, int c )
{
    hb_dvdnav_t * d = &(e->dvdnav);
    int t = title->index;
    hb_chapter_t *chapter;
    dvdnav_status_t result;

    d->title_block_count = title->block_count;
    d->list_chapter = title->list_chapter;

    if ( d->stopped && !hb_dvdnav_reset(d) )
    {
        return 0;
    }
    chapter = hb_list_item( title->list_chapter, c - 1);
    if (chapter != NULL)
        result = dvdnav_program_play(d->dvdnav, t, chapter->pgcn, chapter->pgn);
    else
        result = dvdnav_part_play(d->dvdnav, t, 1);
    if (result != DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_ERROR,"dvd: dvdnav_*_play failed - %s\n", 
                  dvdnav_err_to_string(d->dvdnav) );
        return 0;
    }
    d->title = t;
    d->stopped = 0;
    d->chapter = 0;
    return 1;
}


/***********************************************************************
 * hb_dvdnav_stop
 ***********************************************************************
 *
 **********************************************************************/
void hb_dvdnav_stop( hb_dvd_t * e )
{
}

/***********************************************************************
 * hb_dvdnav_seek
 ***********************************************************************
 *
 **********************************************************************/
int hb_dvdnav_seek( hb_dvd_t * e, uint64_t sector )
{
    hb_dvdnav_t * d = &(e->dvdnav);
    //uint64_t sector = f * d->title_block_count;
    sector = sector > d->title_block_count? d->title_block_count: sector;
    int result, event, len;
    uint8_t buf[HB_DVD_READ_BUFFER_SIZE];
    int done = 0, ii;

    if (d->stopped)
    {
        return 0;
    }

    // XXX the current version of libdvdnav can't seek outside the current
    // PGC. Check if the place we're seeking to is in a different
    // PGC. Position there & adjust the offset if so.
    hb_chapter_t *pgc_change = hb_list_item(d->list_chapter, 0 );
    for ( ii = 0; ii < hb_list_count( d->list_chapter ); ++ii )
    {
        hb_chapter_t *chapter = hb_list_item( d->list_chapter, ii );

        if ( chapter->pgcn != pgc_change->pgcn )
        {
            // this chapter's in a different pgc from the previous - note the
            // change so we can make sector offset's be pgc relative.
            pgc_change = chapter;
        }
        if ( chapter->block_start <= sector && sector <= chapter->block_end )
        {
            // this chapter contains the sector we want - see if it's in a
            // different pgc than the one we're currently in.
            int32_t title, pgcn, pgn;
            if (dvdnav_current_title_program( d->dvdnav, &title, &pgcn, &pgn ) != DVDNAV_STATUS_OK)
                av_log(NULL, AV_LOG_INFO,"dvdnav cur pgcn err: %s\n", dvdnav_err_to_string(d->dvdnav));
            if ( d->title != title || chapter->pgcn != pgcn )
            {
                // this chapter is in a different pgc - switch to it.
                if (dvdnav_program_play(d->dvdnav, d->title, chapter->pgcn, chapter->pgn) != DVDNAV_STATUS_OK)
                    av_log(NULL, AV_LOG_INFO,"dvdnav prog play err: %s\n", dvdnav_err_to_string(d->dvdnav));
            }
            // seek sectors are pgc-relative so remove the pgc start sector.
            sector -= pgc_change->block_start;
            break;
        }
    }

    // dvdnav will not let you seek or poll current position
    // till it reaches a certain point in parsing.  so we
    // have to get blocks until we reach a cell
    // Put an arbitrary limit of 100 blocks on how long we search
    for (ii = 0; ii < 100 && !done; ii++)
    {
        result = dvdnav_get_next_block( d->dvdnav, buf, &event, &len );
        if ( result == DVDNAV_STATUS_ERR )
        {
            av_log(NULL, AV_LOG_INFO,"dvdnav: Read Error, %s\n", dvdnav_err_to_string(d->dvdnav));
            return 0;
        }
        switch ( event )
        {
        case DVDNAV_BLOCK_OK:
        case DVDNAV_CELL_CHANGE:
            done = 1;
            break;

        case DVDNAV_STILL_FRAME:
            dvdnav_still_skip( d->dvdnav );
            break;

        case DVDNAV_WAIT:
            dvdnav_wait_skip( d->dvdnav );
            break;

        case DVDNAV_STOP:
            av_log(NULL, AV_LOG_INFO,"dvdnav: stop encountered during seek\n");
            d->stopped = 1;
            return 0;

        case DVDNAV_HOP_CHANNEL:
        case DVDNAV_NAV_PACKET:
        case DVDNAV_VTS_CHANGE:
        case DVDNAV_HIGHLIGHT:
        case DVDNAV_AUDIO_STREAM_CHANGE:
        case DVDNAV_SPU_STREAM_CHANGE:
        case DVDNAV_SPU_CLUT_CHANGE:
        case DVDNAV_NOP:
        default:
            break;
        }
    }

    if (dvdnav_sector_search(d->dvdnav, sector, SEEK_SET) != DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_ERROR, "dvd: dvdnav_sector_search failed - %s\n", 
                  dvdnav_err_to_string(d->dvdnav) );
        return 0;
    }
    return 1;
}

dvdnav_status_t hb_dvdnav_get_position(hb_dvd_t *e, uint32_t *pos,
				    uint32_t *len) 
{
     return dvdnav_get_position(e->dvdnav.dvdnav, pos, len);
}

dvdnav_status_t hb_dvdnav_sector_search(hb_dvd_t *e,
				     uint64_t offset, int32_t origin) 
{
    return dvdnav_sector_search(e->dvdnav.dvdnav, offset,origin);
} 

/***********************************************************************
 * hb_dvdnav_read
 ***********************************************************************
 *
 **********************************************************************/
int hb_dvdnav_read( hb_dvd_t * e, uint8_t* b, int* len, int* dvd_event)
{
    hb_dvdnav_t * d = &(e->dvdnav);
    int result, event;
    int chapter = 0;
    int error_count = 0;
	*dvd_event=-1;

    while ( 1 )
    {
        if (d->stopped)
        {
            return 0;
        }
        result = dvdnav_get_next_block( d->dvdnav, b, &event, len );
        if ( result == DVDNAV_STATUS_ERR )
        {
            av_log(NULL, AV_LOG_INFO,"dvdnav: Read Error, %s\n", dvdnav_err_to_string(d->dvdnav));
            if (dvdnav_sector_search(d->dvdnav, 1, SEEK_CUR) != DVDNAV_STATUS_OK)
            {
                av_log(NULL, AV_LOG_ERROR, "dvd: dvdnav_sector_search failed - %s\n",
                        dvdnav_err_to_string(d->dvdnav) );
                return -1;
            }
            error_count++;
            if (error_count > 10)
            {
                av_log(NULL, AV_LOG_ERROR,"dvdnav: Error, too many consecutive read errors\n");
                return -1;
            }
            continue;
        }
        error_count = 0;
		*dvd_event = event;
        switch ( event )
        {
        case DVDNAV_BLOCK_OK:
            // We have received a regular block of the currently playing
            // MPEG stream.

            // The muxers expect to only get chapter 2 and above
            // They write chapter 1 when chapter 2 is detected.
#if 0            
            if (chapter > 1)
                b->new_chap = chapter;
#endif			
            chapter = 0;
            return 1;

        case DVDNAV_NOP:
            /*
            * Nothing to do here. 
            */
            break;

        case DVDNAV_STILL_FRAME:
            /*
            * We have reached a still frame. A real player application
            * would wait the amount of time specified by the still's
            * length while still handling user input to make menus and
            * other interactive stills work. A length of 0xff means an
            * indefinite still which has to be skipped indirectly by some 
            * user interaction. 
            */
            dvdnav_still_skip( d->dvdnav );
            break;

        case DVDNAV_WAIT:
            /*
            * We have reached a point in DVD playback, where timing is
            * critical. Player application with internal fifos can
            * introduce state inconsistencies, because libdvdnav is
            * always the fifo's length ahead in the stream compared to
            * what the application sees. Such applications should wait
            * until their fifos are empty when they receive this type of
            * event. 
            */
            dvdnav_wait_skip( d->dvdnav );
            break;

        case DVDNAV_SPU_CLUT_CHANGE:
            /*
            * Player applications should pass the new colour lookup table 
            * to their SPU decoder 
            */
            break;

        case DVDNAV_SPU_STREAM_CHANGE:
            /*
            * Player applications should inform their SPU decoder to
            * switch channels 
            */
            break;

        case DVDNAV_AUDIO_STREAM_CHANGE:
            /*
            * Player applications should inform their audio decoder to
            * switch channels 
            */
            break;

        case DVDNAV_HIGHLIGHT:
            /*
            * Player applications should inform their overlay engine to
            * highlight the given button 
            */
            break;

        case DVDNAV_VTS_CHANGE:
            /*
            * Some status information like video aspect and video scale
            * permissions do not change inside a VTS. Therefore this
            * event can be used to query such information only when
            * necessary and update the decoding/displaying accordingly. 
            */
            break;

        case DVDNAV_CELL_CHANGE:
            /*
            * Some status information like the current Title and Part
            * numbers do not change inside a cell. Therefore this event
            * can be used to query such information only when necessary
            * and update the decoding/displaying accordingly. 
            */
            {
                int tt = 0, pgcn = 0, pgn = 0, c;

                dvdnav_current_title_program(d->dvdnav, &tt, &pgcn, &pgn);
                if (tt != d->title)
                {
                    // Transition to another title signals that we are done.
                    return 0;
                }
                c = FindChapterIndex(d->list_chapter, pgcn, pgn);
                if (c != d->chapter)
                {
                    if (c < d->chapter)
                    {
                        // Some titles end with a 'link' back to the beginning so
                        // a transition to an earlier chapter means we're done.
                        return 0;
                    }
                    chapter = d->chapter = c;
                }
            }
            break;

        case DVDNAV_NAV_PACKET:
            /*
            * A NAV packet provides PTS discontinuity information, angle
            * linking information and button definitions for DVD menus.
            * Angles are handled completely inside libdvdnav. For the
            * menus to work, the NAV packet information has to be passed
            * to the overlay engine of the player so that it knows the
            * dimensions of the button areas. 
            */

            // mpegdemux expects to get these.  I don't think it does
            // anything useful with them however.

            // The muxers expect to only get chapter 2 and above
            // They write chapter 1 when chapter 2 is detected.
#if 0            
            if (chapter > 1)
                b->new_chap = chapter;
#endif			
            chapter = 0;
            return 1;

            break;

        case DVDNAV_HOP_CHANNEL:
            /*
            * This event is issued whenever a non-seamless operation has
            * been executed. Applications with fifos should drop the
            * fifos content to speed up responsiveness. 
            */
            break;

        case DVDNAV_STOP:
            /*
            * Playback should end here. 
            */
            av_log(NULL, AV_LOG_ERROR,"dvdnav: Error, too many consecutive read errors\n");
            d->stopped = 1;
            return 0;

        default:
            break;
        }
    }
    return 0;
}

/***********************************************************************
 * hb_dvdnav_chapter
 ***********************************************************************
 * Returns in which chapter the next block to be read is.
 * Chapter numbers start at 1.
 **********************************************************************/
int hb_dvdnav_chapter( hb_dvd_t * e )
{
    hb_dvdnav_t * d = &(e->dvdnav);
    int32_t t, pgcn, pgn;
    int32_t c;

    if (dvdnav_current_title_program(d->dvdnav, &t, &pgcn, &pgn) != DVDNAV_STATUS_OK)
    {
        return -1;
    }
    c = FindChapterIndex( d->list_chapter, pgcn, pgn );
    return c;
}


/***********************************************************************
 * hb_dvdnav_close
 ***********************************************************************
 * Closes and frees everything
 **********************************************************************/
void hb_dvdnav_close( hb_dvd_t ** _d )
{
    hb_dvdnav_t * d = &((*_d)->dvdnav);

    if( d->dvdnav ) dvdnav_close( d->dvdnav );
    if( d->vmg ) ifoClose( d->vmg );
    if( d->reader ) DVDClose( d->reader );

    av_free( d );
    *_d = NULL;
}



/***********************************************************************
 * hb_dvdnav_angle_count
 ***********************************************************************
 * Returns the number of angles supported.
 **********************************************************************/
int hb_dvdnav_angle_count( hb_dvd_t * e )
{
    hb_dvdnav_t * d = &(e->dvdnav);
    int current, angle_count;

    if (dvdnav_get_angle_info( d->dvdnav, &current, &angle_count) != DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_INFO,"dvdnav_get_angle_info %s\n", dvdnav_err_to_string(d->dvdnav));
        angle_count = 1;
    }
    return angle_count;
}

/***********************************************************************
 * hb_dvdnav_set_angle
 ***********************************************************************
 * Sets the angle to read
 **********************************************************************/
void hb_dvdnav_set_angle( hb_dvd_t * e, int angle )
{
    hb_dvdnav_t * d = &(e->dvdnav);

    if (dvdnav_angle_change( d->dvdnav, angle) != DVDNAV_STATUS_OK)
    {
        av_log(NULL, AV_LOG_INFO,"dvdnav_angle_change %s\n", dvdnav_err_to_string(d->dvdnav));
    }
}


/***********************************************************************
 * FindChapterIndex
 ***********************************************************************
 * Assumes pgc and cell_cur are correctly set, and sets cell_next to the
 * cell to be read when we will be done with cell_cur.
 **********************************************************************/
int FindChapterIndex( hb_list_t * list, int pgcn, int pgn )
{
    int count, ii;
    hb_chapter_t *chapter;

    count = hb_list_count( list );
    for (ii = 0; ii < count; ii++)
    {
        chapter = hb_list_item( list, ii );
        if (chapter->pgcn == pgcn && chapter->pgn == pgn)
            return chapter->index;
    }
    return 0;
}

/***********************************************************************
 * FindNextCell
 ***********************************************************************
 * Assumes pgc and cell_cur are correctly set, and sets cell_next to the
 * cell to be read when we will be done with cell_cur.
 **********************************************************************/
int FindNextCell( pgc_t *pgc, int cell_cur )
{
    int i = 0;
    int cell_next;

    if( pgc->cell_playback[cell_cur].block_type ==
            BLOCK_TYPE_ANGLE_BLOCK )
    {

        while( pgc->cell_playback[cell_cur+i].block_mode !=
                   BLOCK_MODE_LAST_CELL )
        {
             i++;
        }
        cell_next = cell_cur + i + 1;
        av_log(NULL, AV_LOG_INFO, "dvd: Skipping multi-angle cells %d-%d\n",
                cell_cur,
                cell_next - 1 );
    }
    else
    {
        cell_next = cell_cur + 1;
    }
    return cell_next;
}

void FindReadNextCell( hb_dvdread_t * d )
{
    int i = 0;

    if( d->pgc->cell_playback[d->cell_cur].block_type ==
            BLOCK_TYPE_ANGLE_BLOCK )
    {

        while( d->pgc->cell_playback[d->cell_cur+i].block_mode !=
                   BLOCK_MODE_LAST_CELL )
        {
             i++;
        }
        d->cell_next = d->cell_cur + i + 1;
#if 0		
        hb_log( "dvd: Skipping multi-angle cells %d-%d",
                d->cell_cur,
                d->cell_next - 1 );
#endif
    }
    else
    {
        d->cell_next = d->cell_cur + 1;
    }
}


/***********************************************************************
 * NextPgcn
 ***********************************************************************
 * Assumes pgc and cell_cur are correctly set, and sets cell_next to the
 * cell to be read when we will be done with cell_cur.
 * Since pg chains can be circularly linked (either from a read error or
 * deliberately) pgcn_map tracks program chains we've already seen.
 **********************************************************************/
int NextPgcn( ifo_handle_t *ifo, int pgcn, uint32_t pgcn_map[MAX_PGCN/32] )
{
    int next_pgcn;
    pgc_t *pgc;

    pgcn_map[pgcn >> 5] |= (1 << (pgcn & 31));

    pgc = ifo->vts_pgcit->pgci_srp[pgcn-1].pgc;
    next_pgcn = pgc->next_pgc_nr;
    if ( next_pgcn < 1 || next_pgcn >= MAX_PGCN || next_pgcn > ifo->vts_pgcit->nr_of_pgci_srp )
        return 0;

    return pgcn_map[next_pgcn >> 5] & (1 << (next_pgcn & 31))? 0 : next_pgcn;
}

/***********************************************************************
 * PgcWalkInit
 ***********************************************************************
 * Pgc links can loop. I track which have been visited in a bit vector
 * Initialize the bit vector to empty.
 **********************************************************************/
void PgcWalkInit( uint32_t pgcn_map[MAX_PGCN/32] )
{
    memset(pgcn_map, 0, sizeof(pgcn_map) );
}

/***********************************************************************
 * dvdtime2msec
 ***********************************************************************
 * From lsdvd
 **********************************************************************/
int dvdtime2msec(dvd_time_t * dt)
{
    double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};
    double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
    long   ms;
    ms  = (((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f)) * 3600000;
    ms += (((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f)) * 60000;
    ms += (((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f)) * 1000;

    if( fps > 0 )
    {
        ms += ((dt->frame_u & 0x30) >> 3) * 5 +
              (dt->frame_u & 0x0f) * 1000.0 / fps;
    }

    return ms;
}

hb_dvd_t * hb_dvdread_init( char * path )
{
    hb_dvd_t * e;
    hb_dvdread_t * d;
    int region_mask;

    e = av_malloc( sizeof( hb_dvd_t ) );
	memset(e, 0, sizeof(hb_dvd_t));
    d = &(e->dvdread);

	/* Log DVD drive region code */
    if ( hb_dvd_region( path, &region_mask ) == 0 )
    {
        av_log(NULL, AV_LOG_INFO, "dvd: Region mask 0x%02x\n", region_mask );
        if ( region_mask == 0xFF )
        {
             av_log(NULL, AV_LOG_WARNING,"dvd: Warning, DVD device has no region set\n" );
        }
    }

    /* Open device */
    if( !( d->reader = DVDOpen( path ) ) )
    {
        /*
         * Not an error, may be a stream - which we'll try in a moment.
         */
        av_log(NULL, AV_LOG_ERROR, "dvd: not a dvd - trying as a stream/file instead" );
        goto fail;
    }

    /* Open main IFO */
    if( !( d->vmg = ifoOpen( d->reader, 0 ) ) )
    {
        av_log(NULL, AV_LOG_ERROR, "dvd: ifoOpen failed" );
        goto fail;
    }

    d->path = av_strdup( path );

    return e;

fail:
    if( d->vmg )    ifoClose( d->vmg );
    if( d->reader ) DVDClose( d->reader );
    av_free( d );
    return NULL;
}

void hb_dvdread_close( hb_dvd_t ** _d )
{
    hb_dvdread_t * d = &((*_d)->dvdread);

    if( d->vmg )
    {
        ifoClose( d->vmg );
    }
    if( d->reader )
    {
        DVDClose( d->reader );
    }

    av_free( d );
    *_d = NULL;
}

int hb_dvdread_title_count( hb_dvd_t * e )
{
    hb_dvdread_t *d = &(e->dvdread);
    return d->vmg->tt_srpt->nr_of_srpts;
}

hb_title_t * hb_dvdread_title_scan( hb_dvd_t * e, int t )
{

    hb_dvdread_t *d = &(e->dvdread);
    hb_title_t   * title;
    ifo_handle_t * vts = NULL;
    int            pgc_id, pgn, i;
    hb_chapter_t * chapter;
    int            c;
    uint64_t       duration;
    float          duration_correction;
    unsigned char  unused[1024];

    av_log(NULL, AV_LOG_INFO, "scan: scanning title %d\n", t );

    title = hb_title_init( d->path, t );
    if( DVDUDFVolumeInfo( d->reader, title->name, sizeof( title->name ),
                          unused, sizeof( unused ) ) )
    {
        char * p_cur, * p_last = d->path;
        for( p_cur = d->path; *p_cur; p_cur++ )
        {
            if( p_cur[0] == '/' && p_cur[1] )
            {
                p_last = &p_cur[1];
            }
        }
        snprintf( title->name, sizeof( title->name ), "%s", p_last );
    }

    /* VTS which our title is in */
    title->vts = d->vmg->tt_srpt->title[t-1].title_set_nr;

    if ( !title->vts )
    {
        /* A VTS of 0 means the title wasn't found in the title set */
        av_log(NULL, AV_LOG_ERROR,"Invalid VTS (title set) number: %i\n", title->vts);
        goto fail;
    }

    av_log(NULL, AV_LOG_INFO, "scan: opening IFO for VTS %d\n", title->vts );
    if( !( vts = ifoOpen( d->reader, title->vts ) ) )
    {
        av_log(NULL, AV_LOG_ERROR, "scan: ifoOpen failed\n" );
        goto fail;
    }

    /* ignore titles with bogus cell addresses so we don't abort later
     * in libdvdread. */
    for ( i = 0; i < vts->vts_c_adt->nr_of_vobs; ++i)
    {
        if( (vts->vts_c_adt->cell_adr_table[i].start_sector & 0xffffff ) ==
            0xffffff )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: cell_adr_table[%d].start_sector invalid (0x%x) "
                      "- skipping title\n", i,
                      vts->vts_c_adt->cell_adr_table[i].start_sector );
            goto fail;
        }
        if( (vts->vts_c_adt->cell_adr_table[i].last_sector & 0xffffff ) ==
            0xffffff )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: cell_adr_table[%d].last_sector invalid (0x%x) "
                      "- skipping title\n", i,
                      vts->vts_c_adt->cell_adr_table[i].last_sector );
            goto fail;
        }
        if( vts->vts_c_adt->cell_adr_table[i].start_sector >=
            vts->vts_c_adt->cell_adr_table[i].last_sector )
        {
            av_log(NULL, AV_LOG_ERROR, "scan: cell_adr_table[%d].start_sector (0x%x) "
                      "is not before last_sector (0x%x) - skipping title\n", i,
                      vts->vts_c_adt->cell_adr_table[i].start_sector,
                      vts->vts_c_adt->cell_adr_table[i].last_sector );
            goto fail;
        }
    }

#if 0
    if( global_verbosity_level == 3 )
    {
        ifo_print( d->reader, title->vts );
    }
#endif	

    /* Position of the title in the VTS */
    title->ttn = d->vmg->tt_srpt->title[t-1].vts_ttn;
    if ( title->ttn < 1 || title->ttn > vts->vts_ptt_srpt->nr_of_srpts )
    {
        av_log(NULL, AV_LOG_ERROR, "invalid VTS PTT offset %d for title %d, skipping\n", title->ttn, t );
        goto fail;
    }

    /* Get pgc */
    pgc_id = vts->vts_ptt_srpt->title[title->ttn-1].ptt[0].pgcn;
    if ( pgc_id < 1 || pgc_id > vts->vts_pgcit->nr_of_pgci_srp )
    {
        av_log(NULL, AV_LOG_ERROR, "invalid PGC ID %d for title %d, skipping\n", pgc_id, t );
        goto fail;
    }
    pgn    = vts->vts_ptt_srpt->title[title->ttn-1].ptt[0].pgn;
    d->pgc = vts->vts_pgcit->pgci_srp[pgc_id-1].pgc;

    av_log(NULL, AV_LOG_INFO,"pgc_id: %d, pgn: %d: pgc: %p\n", pgc_id, pgn, d->pgc);

    if( !d->pgc )
    {
        av_log(NULL, AV_LOG_ERROR, "scan: pgc not valid, skipping\n" );
        goto fail;
    }

    if( pgn <= 0 || pgn > 99 )
    {
        av_log(NULL, AV_LOG_ERROR, "scan: pgn %d not valid, skipping\n", pgn );
        goto fail;
    }

    /* Start cell */
    title->cell_start  = d->pgc->program_map[pgn-1] - 1;
    title->block_start = d->pgc->cell_playback[title->cell_start].first_sector;

    /* End cell */
    title->cell_end  = d->pgc->nr_of_cells - 1;
    title->block_end = d->pgc->cell_playback[title->cell_end].last_sector;

    /* Block count */
    title->block_count = 0;
    d->cell_cur = title->cell_start;
    while( d->cell_cur <= title->cell_end )
    {
#define cp d->pgc->cell_playback[d->cell_cur]
        title->block_count += cp.last_sector + 1 - cp.first_sector;
#undef cp
        FindReadNextCell( d );
        d->cell_cur = d->cell_next;
    }

    av_log(NULL, AV_LOG_INFO, "scan: vts=%d, ttn=%d, cells=%d->%d, blocks=%d->%d, "
            "%d blocks\n", title->vts, title->ttn, title->cell_start,
            title->cell_end, title->block_start, title->block_end,
            title->block_count );

    /* Get duration */
    title->duration = 90LL * dvdtime2msec( &d->pgc->playback_time );
    title->hours    = title->duration / 90000 / 3600;
    title->minutes  = ( ( title->duration / 90000 ) % 3600 ) / 60;
    title->seconds  = ( title->duration / 90000 ) % 60;
    av_log(NULL, AV_LOG_INFO, "scan: duration is %02d:%02d:%02d (%"PRId64" ms)\n",
            title->hours, title->minutes, title->seconds,
            title->duration / 90 );

    /* ignore titles under 10 seconds because they're often stills or
     * clips with no audio & our preview code doesn't currently handle
     * either of these. */
    if( title->duration < 900000LL )
    {
        av_log(NULL, AV_LOG_INFO, "scan: ignoring title (too short)\n" );
        goto fail;
    }
#if 0
    /* Detect languages */
    for( i = 0; i < vts->vtsi_mat->nr_of_vts_audio_streams; i++ )
    {
        hb_audio_t * audio, * audio_tmp;
        int          audio_format, lang_code, audio_control,
                     position, j;
        iso639_lang_t * lang;
        int lang_extension = 0;

        av_log(NULL, AV_LOG_INFO, "scan: checking audio %d\n", i + 1 );

        audio = calloc( sizeof( hb_audio_t ), 1 );

        audio_format  = vts->vtsi_mat->vts_audio_attr[i].audio_format;
        lang_code     = vts->vtsi_mat->vts_audio_attr[i].lang_code;
        lang_extension = vts->vtsi_mat->vts_audio_attr[i].code_extension;
        audio_control =
            vts->vts_pgcit->pgci_srp[pgc_id-1].pgc->audio_control[i];

        if( !( audio_control & 0x8000 ) )
        {
            av_log(NULL, AV_LOG_INFO, "scan: audio channel is not active\n" );
            av_free( audio );
            continue;
        }

        position = ( audio_control & 0x7F00 ) >> 8;

        switch( audio_format )
        {
            case 0x00:
                audio->id    = ( ( 0x80 + position ) << 8 ) | 0xbd;
                audio->config.in.codec = HB_ACODEC_AC3;
                break;

            case 0x02:
            case 0x03:
                audio->id    = 0xc0 + position;
                audio->config.in.codec = HB_ACODEC_MPGA;
                break;

            case 0x04:
                audio->id    = ( ( 0xa0 + position ) << 8 ) | 0xbd;
                audio->config.in.codec = HB_ACODEC_LPCM;
                break;

            case 0x06:
                audio->id    = ( ( 0x88 + position ) << 8 ) | 0xbd;
                audio->config.in.codec = HB_ACODEC_DCA;
                break;

            default:
                audio->id    = 0;
                audio->config.in.codec = 0;
                av_log(NULL, AV_LOG_INFO, "scan: unknown audio codec (%x)\n",
                        audio_format );
                break;
        }
        if( !audio->id )
        {
            continue;
        }

        /* Check for duplicate tracks */
        audio_tmp = NULL;
        for( j = 0; j < hb_list_count( title->list_audio ); j++ )
        {
            audio_tmp = hb_list_item( title->list_audio, j );
            if( audio->id == audio_tmp->id )
            {
                break;
            }
            audio_tmp = NULL;
        }
        if( audio_tmp )
        {
            av_log(NULL, AV_LOG_INFO, "scan: duplicate audio track\n" );
            av_free( audio );
            continue;
        }

        audio->config.lang.type = lang_extension;

        lang = lang_for_code( vts->vtsi_mat->vts_audio_attr[i].lang_code );

        snprintf( audio->config.lang.description, sizeof( audio->config.lang.description ), "%s (%s)",
            strlen(lang->native_name) ? lang->native_name : lang->eng_name,
            audio->config.in.codec == HB_ACODEC_AC3 ? "AC3" : ( audio->config.in.codec ==
                HB_ACODEC_DCA ? "DTS" : ( audio->config.in.codec ==
                HB_ACODEC_MPGA ? "MPEG" : "LPCM" ) ) );
        snprintf( audio->config.lang.simple, sizeof( audio->config.lang.simple ), "%s",
                  strlen(lang->native_name) ? lang->native_name : lang->eng_name );
        snprintf( audio->config.lang.iso639_2, sizeof( audio->config.lang.iso639_2 ), "%s",
                  lang->iso639_2);

        switch( lang_extension )
        {
        case 0:
        case 1:
            break;
        case 2:
            av_strlcat( audio->config.lang.description, " (Visually Impaired)", sizeof(audio->config.lang.description) );
            break;
        case 3:
            av_strlcat( audio->config.lang.description, " (Director's Commentary 1)",sizeof(audio->config.lang.description) );
            break;
        case 4:
            av_strlcat( audio->config.lang.description, " (Director's Commentary 2)" , sizeof(audio->config.lang.description) );
            break;
        default:
            break;
        }

        av_log(NULL, AV_LOG_INFO, "scan: id=%x, lang=%s, 3cc=%s ext=%i", audio->id,
                audio->config.lang.description, audio->config.lang.iso639_2,
                lang_extension );

        audio->config.in.track = i;
        hb_list_add( title->list_audio, audio );
    }
#endif
    memcpy( title->palette,
            vts->vts_pgcit->pgci_srp[pgc_id-1].pgc->palette,
            16 * sizeof( uint32_t ) );

    /* Check for subtitles */
    for( i = 0; i < vts->vtsi_mat->nr_of_vts_subp_streams; i++ )
    {
        hb_subtitle_t * subtitle;
        int spu_control;
        int position;
        iso639_lang_t * lang;
        int lang_extension = 0;

        av_log(NULL, AV_LOG_INFO, "scan: checking subtitle %d\n", i + 1 );

        spu_control =
            vts->vts_pgcit->pgci_srp[pgc_id-1].pgc->subp_control[i];

        if( !( spu_control & 0x80000000 ) )
        {
            av_log(NULL, AV_LOG_INFO, "scan: subtitle channel is not active\n" );
            continue;
        }

        if( vts->vtsi_mat->vts_video_attr.display_aspect_ratio )
        {
            switch( vts->vtsi_mat->vts_video_attr.permitted_df )
            {
                case 1:
                    position = spu_control & 0xFF;
                    break;
                case 2:
                    position = ( spu_control >> 8 ) & 0xFF;
                    break;
                default:
                    position = ( spu_control >> 16 ) & 0xFF;
            }
        }
        else
        {
            position = ( spu_control >> 24 ) & 0x7F;
        }

        lang_extension = vts->vtsi_mat->vts_subp_attr[i].code_extension;

        lang = lang_for_code( vts->vtsi_mat->vts_subp_attr[i].lang_code );

        subtitle = calloc( sizeof( hb_subtitle_t ), 1 );
        subtitle->track = i+1;
        subtitle->id = ( ( 0x20 + position ) << 8 ) | 0xbd;
        snprintf( subtitle->lang, sizeof( subtitle->lang ), "%s",
             strlen(lang->native_name) ? lang->native_name : lang->eng_name);
        snprintf( subtitle->iso639_2, sizeof( subtitle->iso639_2 ), "%s",
                  lang->iso639_2);
        subtitle->format = PICTURESUB;
        subtitle->source = VOBSUB;
        subtitle->config.dest   = RENDERSUB;  // By default render (burn-in) the VOBSUB.

        subtitle->type = lang_extension;

        switch( lang_extension )
        {  
        case 0:
            break;
        case 1:
            break;
        case 2:
            av_strlcat( subtitle->lang, " (Caption with bigger size character)", sizeof(subtitle->lang));
            break;
        case 3: 
            av_strlcat( subtitle->lang, " (Caption for Children)", sizeof(subtitle->lang));
            break;
        case 4:
            break;
        case 5:
            av_strlcat( subtitle->lang, " (Closed Caption)", sizeof(subtitle->lang));
            break;
        case 6:
            av_strlcat( subtitle->lang, " (Closed Caption with bigger size character)", sizeof(subtitle->lang));
            break;
        case 7:
            av_strlcat( subtitle->lang, " (Closed Caption for Children)", sizeof(subtitle->lang));
            break;
        case 8:
            break;
        case 9:
            av_strlcat( subtitle->lang, " (Forced Caption)", sizeof(subtitle->lang));
            break;
        case 10:
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            av_strlcat( subtitle->lang, " (Director's Commentary)", sizeof(subtitle->lang));
            break;
        case 14:
            av_strlcat( subtitle->lang, " (Director's Commentary with bigger size character)", sizeof(subtitle->lang));
            break;
        case 15:
            av_strlcat( subtitle->lang, " (Director's Commentary for Children)", sizeof(subtitle->lang));
        default:
            break;
        }

        av_log(NULL, AV_LOG_INFO, "scan: id=%x, lang=%s, 3cc=%s", subtitle->id,
                subtitle->lang, subtitle->iso639_2 );

        hb_list_add( title->list_subtitle, subtitle );
    }

    /* Chapters */
    av_log(NULL, AV_LOG_INFO, "scan: title %d has %d chapters", t,
            vts->vts_ptt_srpt->title[title->ttn-1].nr_of_ptts );
    for( i = 0, c = 1;
         i < vts->vts_ptt_srpt->title[title->ttn-1].nr_of_ptts; i++ )
    {
        chapter = calloc( sizeof( hb_chapter_t ), 1 );
        /* remember the on-disc chapter number */
        chapter->index = i + 1;

        pgc_id = vts->vts_ptt_srpt->title[title->ttn-1].ptt[i].pgcn;
        pgn    = vts->vts_ptt_srpt->title[title->ttn-1].ptt[i].pgn;
        d->pgc = vts->vts_pgcit->pgci_srp[pgc_id-1].pgc;

        /* Start cell */
        chapter->cell_start  = d->pgc->program_map[pgn-1] - 1;
        chapter->block_start =
            d->pgc->cell_playback[chapter->cell_start].first_sector;

        // if there are no more programs in this pgc, the end cell is the
        // last cell. Otherwise it's the cell before the start cell of the
        // next program.
        if ( pgn == d->pgc->nr_of_programs )
        {
            chapter->cell_end = d->pgc->nr_of_cells - 1;
        }
        else
        {
            chapter->cell_end = d->pgc->program_map[pgn] - 2;;
        }
        chapter->block_end = d->pgc->cell_playback[chapter->cell_end].last_sector;

        /* Block count, duration */
        chapter->block_count = 0;
        chapter->duration = 0;

        d->cell_cur = chapter->cell_start;
        while( d->cell_cur <= chapter->cell_end )
        {
#define cp d->pgc->cell_playback[d->cell_cur]
            chapter->block_count += cp.last_sector + 1 - cp.first_sector;
            chapter->duration += 90LL * dvdtime2msec( &cp.playback_time );
#undef cp
            FindReadNextCell( d );
            d->cell_cur = d->cell_next;
        }
        hb_list_add( title->list_chapter, chapter );
        c++;
    }

    /* The durations we get for chapters aren't precise. Scale them so
       the total matches the title duration */
    duration = 0;
    for( i = 0; i < hb_list_count( title->list_chapter ); i++ )
    {
        chapter = hb_list_item( title->list_chapter, i );
        duration += chapter->duration;
    }
    duration_correction = (float) title->duration / (float) duration;
    for( i = 0; i < hb_list_count( title->list_chapter ); i++ )
    {
        int seconds;
        chapter            = hb_list_item( title->list_chapter, i );
        chapter->duration  = duration_correction * chapter->duration;
        seconds            = ( chapter->duration + 45000 ) / 90000;
        chapter->hours     = seconds / 3600;
        chapter->minutes   = ( seconds % 3600 ) / 60;
        chapter->seconds   = seconds % 60;

        av_log(NULL, AV_LOG_INFO, "scan: chap %d c=%d->%d, b=%d->%d (%d), %"PRId64" ms",
                chapter->index, chapter->cell_start, chapter->cell_end,
                chapter->block_start, chapter->block_end,
                chapter->block_count, chapter->duration / 90 );
    }

    /* Get aspect. We don't get width/height/rate infos here as
       they tend to be wrong */
    switch( vts->vtsi_mat->vts_video_attr.display_aspect_ratio )
    {
        case 0:
            title->container_aspect = 4. / 3.;
            break;
        case 3:
            title->container_aspect = 16. / 9.;
            break;
        default:
            av_log(NULL, AV_LOG_INFO, "scan: unknown aspect" );
            goto fail;
    }

    av_log(NULL, AV_LOG_INFO, "scan: aspect = %g", title->aspect );

    /* This title is ok so far */
    goto cleanup;

fail:
    hb_list_close( &title->list_audio );
    av_free( title );
    title = NULL;

cleanup:
    if( vts ) ifoClose( vts );

    return title;
}

int hb_dvdread_start( hb_dvd_t * e, hb_title_t *title, int chapter )
{
    hb_dvdread_t *d = &(e->dvdread);
    int pgc_id, pgn;
    int i;
    int t = title->index;

    /* Open the IFO and the VOBs for this title */
    d->vts = d->vmg->tt_srpt->title[t-1].title_set_nr;
    d->ttn = d->vmg->tt_srpt->title[t-1].vts_ttn;
    if( !( d->ifo = ifoOpen( d->reader, d->vts ) ) )
    {
        av_log(NULL, AV_LOG_ERROR, "dvd: ifoOpen failed for VTS %d", d->vts );
        return 0;
    }
    if( !( d->file = DVDOpenFile( d->reader, d->vts,
                                  DVD_READ_TITLE_VOBS ) ) )
    {
        av_log(NULL, AV_LOG_ERROR, "dvd: DVDOpenFile failed for VTS %d", d->vts );
        return 0;
    }

    /* Get title first and last blocks */
    pgc_id         = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[0].pgcn;
    pgn            = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[0].pgn;
    d->pgc         = d->ifo->vts_pgcit->pgci_srp[pgc_id-1].pgc;
    d->cell_start  = d->pgc->program_map[pgn - 1] - 1;
    d->cell_end    = d->pgc->nr_of_cells - 1;
    d->title_start = d->pgc->cell_playback[d->cell_start].first_sector;
    d->title_end   = d->pgc->cell_playback[d->cell_end].last_sector;

    /* Block count */
    d->title_block_count = 0;
    for( i = d->cell_start; i <= d->cell_end; i++ )
    {
        d->title_block_count += d->pgc->cell_playback[i].last_sector + 1 -
            d->pgc->cell_playback[i].first_sector;
    }

    /* Get pgc for the current chapter */
    pgc_id = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[chapter-1].pgcn;
    pgn    = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[chapter-1].pgn;
    d->pgc = d->ifo->vts_pgcit->pgci_srp[pgc_id-1].pgc;

    /* Get the two first cells */
    d->cell_cur = d->pgc->program_map[pgn-1] - 1;
    FindReadNextCell( d );

    d->block     = d->pgc->cell_playback[d->cell_cur].first_sector;
    d->next_vobu = d->block;
    d->pack_len  = 0;
    d->cell_overlap = 0;
    d->in_cell = 0;
    d->in_sync = 2;

    return 1;
}

void hb_dvdread_stop( hb_dvd_t * e )
{
    hb_dvdread_t *d = &(e->dvdread);
    if( d->ifo )
    {
        ifoClose( d->ifo );
        d->ifo = NULL;
    }
    if( d->file )
    {
        DVDCloseFile( d->file );
        d->file = NULL;
    }
}

dvdnav_status_t hb_dvdread_get_position(hb_dvd_t *e, uint32_t *pos,
				    uint32_t *len) 
{
    hb_dvdread_t *d = &(e->dvdread);
    int count, sizeCell;
    int i;

	*pos = 0;

    *len = d->title_block_count;

    return 1;
}


int hb_dvdread_seek( hb_dvd_t * e, uint64_t sector )
{
    hb_dvdread_t *d = &(e->dvdread);
    int count, sizeCell;
    int i;

    //count = f * d->title_block_count;

	sector = sector > d->title_block_count? d->title_block_count: sector;

    for( i = d->cell_start; i <= d->cell_end; i++ )
    {
        sizeCell = d->pgc->cell_playback[i].last_sector + 1 -
            d->pgc->cell_playback[i].first_sector;

        if( count < sizeCell )
        {
            d->cell_cur = i;
            d->cur_cell_id = 0;
            FindReadNextCell( d );

            /* Now let hb_dvdread_read find the next VOBU */
            d->next_vobu = d->pgc->cell_playback[i].first_sector + count;
            d->pack_len  = 0;
            break;
        }

        sector -= sizeCell;
    }

    if( i > d->cell_end )
    {
        return 0;
    }

    /*
     * Assume that we are in sync, even if we are not given that it is obvious
     * that we are out of sync if we have just done a seek.
     */
    d->in_sync = 2;

    return 1;
}

static int is_nav_pack( unsigned char *buf )
{
    /*
     * The NAV Pack is comprised of the PCI Packet and DSI Packet, both
     * of these start at known offsets and start with a special identifier.
     *
     * NAV = {
     *  PCI = { 00 00 01 bf  # private stream header
     *          ?? ??        # length
     *          00           # substream
     *          ...
     *        }
     *  DSI = { 00 00 01 bf  # private stream header
     *          ?? ??        # length
     *          01           # substream
     *          ...
     *        }
     *
     * The PCI starts at offset 0x26 into the sector, and the DSI starts at 0x400
     *
     * This information from: http://dvd.sourceforge.net/dvdinfo/
     */
    if( ( buf[0x26] == 0x00 &&      // PCI
          buf[0x27] == 0x00 &&
          buf[0x28] == 0x01 &&
          buf[0x29] == 0xbf &&
          buf[0x2c] == 0x00 ) &&
        ( buf[0x400] == 0x00 &&     // DSI
          buf[0x401] == 0x00 &&
          buf[0x402] == 0x01 &&
          buf[0x403] == 0xbf &&
          buf[0x406] == 0x01 ) )
    {
        return ( 1 );
    } else {
        return ( 0 );
    }
}

static int hb_dvdread_is_break( hb_dvdread_t * d )
{
    int     i;
    int     pgc_id, pgn;
    int     nr_of_ptts = d->ifo->vts_ptt_srpt->title[d->ttn-1].nr_of_ptts;
    pgc_t * pgc;
    int     cell;

    for( i = nr_of_ptts - 1;
         i > 0;
         i-- )
    {
        /* Get pgc for chapter (i+1) */
        pgc_id = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[i].pgcn;
        pgn    = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[i].pgn;
        pgc    = d->ifo->vts_pgcit->pgci_srp[pgc_id-1].pgc;
        cell   = pgc->program_map[pgn-1] - 1;

        if( cell <= d->cell_start )
            break;

        // This must not match against the start cell.
        if( pgc->cell_playback[cell].first_sector == d->block && cell != d->cell_start )
        {
            return i + 1;
        }
    }

    return 0;
}


int hb_dvdread_read( hb_dvd_t * e, hb_buffer_t * b )
{
    hb_dvdread_t *d = &(e->dvdread);
 top:
    if( !d->pack_len )
    {
        /* New pack */
        dsi_t dsi_pack;
        int   error = 0;

        // if we've just finished the last cell of the title we don't
        // want to read another block because our next_vobu pointer
        // is probably invalid. Just return 'no data' & our caller
        // should check and discover we're at eof.
        if ( d->cell_cur > d->cell_end )
            return -1;

        for( ;; )
        {
            int block, pack_len, next_vobu, read_retry;

            for( read_retry = 0; read_retry < 3; read_retry++ )
            {
                if( DVDReadBlocks( d->file, d->next_vobu, 1, b->data ) == 1 )
                {
                    /*
                     * Successful read.
                     */
                    break;
                } else {
                    av_log(NULL, AV_LOG_ERROR, "dvd: Read Error on blk %d, attempt %d",
                            d->next_vobu, read_retry );
                }
            }

            if( read_retry == 3 )
            {
                av_log(NULL, AV_LOG_ERROR, "dvd: vobu read error blk %d - skipping to cell %d",
                        d->next_vobu, d->cell_next );
                d->cell_cur  = d->cell_next;
                if ( d->cell_cur > d->cell_end )
                    return -1;
                d->in_cell = 0;
                d->next_vobu = d->pgc->cell_playback[d->cell_cur].first_sector;
                FindReadNextCell( d );
                d->cell_overlap = 1;
                continue;
            }

            if ( !is_nav_pack( b->data ) ) {
                (d->next_vobu)++;
                if( d->in_sync == 1 ) {
                    av_log(NULL, AV_LOG_INFO,"dvd: Lost sync, searching for NAV pack at blk %d",
                           d->next_vobu);
                    d->in_sync = 0;
                }
                continue;
            }

            navRead_DSI( &dsi_pack, &b->data[DSI_START_BYTE] );

            if ( d->in_sync == 0 && d->cur_cell_id &&
                 (d->cur_vob_id != dsi_pack.dsi_gi.vobu_vob_idn ||
                  d->cur_cell_id != dsi_pack.dsi_gi.vobu_c_idn ) )
            {
                // We walked out of the cell we're supposed to be in.
                // If we're not at the start of our next cell go there.
                av_log(NULL, AV_LOG_INFO,"dvd: left cell %d (%u,%u) for (%u,%u) at block %u",
                       d->cell_cur, d->cur_vob_id, d->cur_cell_id,
                       dsi_pack.dsi_gi.vobu_vob_idn, dsi_pack.dsi_gi.vobu_c_idn,
                       d->next_vobu );
                if ( d->next_vobu != d->pgc->cell_playback[d->cell_next].first_sector )
                {
                    d->next_vobu = d->pgc->cell_playback[d->cell_next].first_sector;
                    d->cur_cell_id = 0;
                    continue;
                }
            }

            block     = dsi_pack.dsi_gi.nv_pck_lbn;
            pack_len  = dsi_pack.dsi_gi.vobu_ea;

            // There are a total of 21 next vobu offsets (and 21 prev_vobu
            // offsets) in the navpack SRI structure. The primary one is
            // 'next_vobu' which is the offset (in dvd blocks) from the current
            // block to the start of the next vobu. If the block at 'next_vobu'
            // can't be read, 'next_video' is the offset to the vobu closest to it.
            // The other 19 offsets are vobus at successively longer distances from
            // the current block (this is so that a hardware player can do
            // adaptive error recovery to skip over a bad spot on the disk). In all
            // these offsets the high bit is set to indicate when it contains a
            // valid offset. The next bit (2^30) is set to indicate that there's
            // another valid offset in the SRI that's closer to the current block.
            // A hardware player uses these bits to chose the closest valid offset
            // and uses that as its next vobu. (Some mastering schemes appear to
            // put a bogus offset in next_vobu with the 2^30 bit set & the
            // correct offset in next_video. This works fine in hardware players
            // but will mess up software that doesn't implement the full next
            // vobu decision logic.) In addition to the flag bits there's a
            // reserved value of the offset that indicates 'no next vobu' (which
            // indicates the end of a cell). But having no next vobu pointer with a
            // 'valid' bit set also indicates end of cell. Different mastering
            // schemes seem to use all possible combinations of the flag bits
            // and reserved values to indicate end of cell so we have to check
            // them all or we'll get a disk read error from following an
            // invalid offset.
            uint32_t next_ptr = dsi_pack.vobu_sri.next_vobu;
            if ( ( next_ptr & ( 1 << 31 ) ) == 0  ||
                 ( next_ptr & ( 1 << 30 ) ) != 0 ||
                 ( next_ptr & 0x3fffffff ) == 0x3fffffff )
            {
                next_ptr = dsi_pack.vobu_sri.next_video;
                if ( ( next_ptr & ( 1 << 31 ) ) == 0 ||
                     ( next_ptr & 0x3fffffff ) == 0x3fffffff )
                {
                    // don't have a valid forward pointer - assume end-of-cell
                    d->block     = block;
                    d->pack_len  = pack_len;
                    break;
                }
            }
            next_vobu = block + ( next_ptr & 0x3fffffff );

            if( pack_len >  0    &&
                pack_len <  1024 &&
                block    >= d->next_vobu &&
                ( block <= d->title_start + d->title_block_count ||
                  block <= d->title_end ) )
            {
                d->block     = block;
                d->pack_len  = pack_len;
                d->next_vobu = next_vobu;
                break;
            }

            /* Wasn't a valid VOBU, try next block */
            if( ++error > 1024 )
            {
                av_log(NULL, AV_LOG_ERROR, "dvd: couldn't find a VOBU after 1024 blocks" );
                return -1;
            }

            (d->next_vobu)++;
        }

        if( d->in_sync == 0 || d->in_sync == 2 )
        {
            if( d->in_sync == 0 )
            {
                av_log(NULL, AV_LOG_INFO, "dvd: In sync with DVD at block %d", d->block );
            }
            d->in_sync = 1;
        }

        // Revert the cell overlap, and check for a chapter break
        // If this cell is zero length (prev_vobu & next_vobu both
        // set to END_OF_CELL) we need to check for beginning of
        // cell before checking for end or we'll advance to the next
        // cell too early and fail to generate a chapter mark when this
        // cell starts a chapter.
        if( ( dsi_pack.vobu_sri.prev_vobu & (1 << 31 ) ) == 0 ||
            ( dsi_pack.vobu_sri.prev_vobu & 0x3fffffff ) == 0x3fffffff )
        {
            // A vobu that's not at the start of a cell can have an
            // EOC prev pointer (this seems to be associated with some
            // sort of drm). The rest of the content in the cell may be
            // booby-trapped so treat this like an end-of-cell rather
            // than a beginning of cell.
            if ( d->pgc->cell_playback[d->cell_cur].first_sector < dsi_pack.dsi_gi.nv_pck_lbn &&
                 d->pgc->cell_playback[d->cell_cur].last_sector >= dsi_pack.dsi_gi.nv_pck_lbn )
            {
                av_log(NULL, AV_LOG_INFO, "dvd: null prev_vobu in cell %d at block %d", d->cell_cur,
                        d->block );
                // treat like end-of-cell then go directly to start of next cell.
                d->cell_cur  = d->cell_next;
                d->in_cell = 0;
                d->next_vobu = d->pgc->cell_playback[d->cell_cur].first_sector;
                FindReadNextCell( d );
                d->cell_overlap = 1;
                goto top;
            }
            else
            {
                if ( d->block != d->pgc->cell_playback[d->cell_cur].first_sector )
                {
                    av_log(NULL, AV_LOG_INFO,"dvd: beginning of cell %d at block %d", d->cell_cur,
                           d->block );
                }
                if( d->in_cell )
                {
                    av_log(NULL, AV_LOG_ERROR,"dvd: assuming missed end of cell %d", d->cell_cur );
                    d->cell_cur  = d->cell_next;
                    d->next_vobu = d->pgc->cell_playback[d->cell_cur].first_sector;
                    FindReadNextCell( d );
                    d->cell_overlap = 1;
                    d->in_cell = 0;
                } else {
                    d->in_cell = 1;
                }
                d->cur_vob_id = dsi_pack.dsi_gi.vobu_vob_idn;
                d->cur_cell_id = dsi_pack.dsi_gi.vobu_c_idn;

                if( d->cell_overlap )
                {
                    b->new_chap = hb_dvdread_is_break( d );
                    d->cell_overlap = 0;
                }
            }
        }

        if( ( dsi_pack.vobu_sri.next_vobu & (1 << 31 ) ) == 0 ||
            ( dsi_pack.vobu_sri.next_vobu & 0x3fffffff ) == 0x3fffffff )
        {
            if ( d->block <= d->pgc->cell_playback[d->cell_cur].first_sector ||
                 d->block > d->pgc->cell_playback[d->cell_cur].last_sector )
            {
                av_log(NULL, AV_LOG_INFO, "dvd: end of cell %d at block %d", d->cell_cur,
                        d->block );
            }
            d->cell_cur  = d->cell_next;
            d->in_cell = 0;
            d->next_vobu = d->pgc->cell_playback[d->cell_cur].first_sector;
            FindReadNextCell( d );
            d->cell_overlap = 1;

        }
    }
    else
    {
        if( DVDReadBlocks( d->file, d->block, 1, b->data ) != 1 )
        {
            // this may be a real DVD error or may be DRM. Either way
            // we don't want to quit because of one bad block so set
            // things up so we'll advance to the next vobu and recurse.
            av_log(NULL, AV_LOG_ERROR, "dvd: DVDReadBlocks failed (%d), skipping to vobu %u",
                      d->block, d->next_vobu );
            d->pack_len = 0;
            goto top;  /* XXX need to restructure this routine & avoid goto */
        }
        d->pack_len--;
    }

    d->block++;

    return 1;
}

int hb_dvdread_chapter( hb_dvd_t * e )
{
    hb_dvdread_t *d = &(e->dvdread);
    int     i;
    int     pgc_id, pgn;
    int     nr_of_ptts = d->ifo->vts_ptt_srpt->title[d->ttn-1].nr_of_ptts;
    pgc_t * pgc;

    for( i = nr_of_ptts - 1;
         i >= 0;
         i-- )
    {
        /* Get pgc for chapter (i+1) */
        pgc_id = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[i].pgcn;
        pgn    = d->ifo->vts_ptt_srpt->title[d->ttn-1].ptt[i].pgn;
        pgc    = d->ifo->vts_pgcit->pgci_srp[pgc_id-1].pgc;

        if( d->cell_cur - d->cell_overlap >= pgc->program_map[pgn-1] - 1 &&
            d->cell_cur - d->cell_overlap <= pgc->nr_of_cells - 1 )
        {
            /* We are in this chapter */
            return i + 1;
        }
    }

    /* End of title */
    return -1;
}


int hb_dvdread_angle_count( hb_dvd_t * d )
{
    return 1;
}

void hb_dvdread_set_angle( hb_dvd_t * d, int angle )
{
}


