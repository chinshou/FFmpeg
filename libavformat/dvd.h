/* $Id: dvd.h,v 1.1 2004/08/02 07:19:05 stebbins Exp $

   This file is part of the HandBrake source code.
   Homepage: <http://handbrake.fr/>.
   It may be used under the terms of the GNU General Public License. */

#ifndef HB_DVD_H
#define HB_DVD_H

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;


#include <dvdnav/dvdnav.h>
#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>

#define HB_DVD_READ_BUFFER_SIZE 2048

#define HB_ACODEC_MASK   0x00FF00
#define HB_ACODEC_FAAC   0x000100
#define HB_ACODEC_LAME   0x000200
#define HB_ACODEC_VORBIS 0x000400
#define HB_ACODEC_AC3    0x000800
#define HB_ACODEC_MPGA   0x001000
#define HB_ACODEC_LPCM   0x002000
#define HB_ACODEC_DCA    0x004000
#define HB_ACODEC_FFMPEG 0x008000
#define HB_ACODEC_CA_AAC 0x010000

enum
{
    WORK_SYNC = 1,
    WORK_DECMPEG2,
    WORK_DECCC608,
    WORK_DECVOBSUB,
    WORK_DECSRTSUB,
    WORK_ENCVOBSUB,
    WORK_RENDER,
    WORK_ENCAVCODEC,
    WORK_ENCX264,
    WORK_ENCTHEORA,
    WORK_DECA52,
    WORK_DECDCA,
    WORK_DECAVCODEC,
    WORK_DECAVCODECV,
    WORK_DECAVCODECVI,
    WORK_DECAVCODECAI,
    WORK_DECLPCM,
    WORK_ENCFAAC,
    WORK_ENCLAME,
    WORK_ENCVORBIS,
    WORK_ENC_CA_AAC
};

struct hb_subtitle_config_s
{
    enum subdest { RENDERSUB, PASSTHRUSUB } dest;
    int  force;
    int  default_track; 
    char src_filename[128];
    char src_codeset[40];
    int64_t offset;
};

typedef struct hb_subtitle_config_s hb_subtitle_config_t;

struct hb_subtitle_s
{
    int  id;
    int  track;

    hb_subtitle_config_t config;

    enum subtype { PICTURESUB, TEXTSUB } format;
    enum subsource { VOBSUB, SRTSUB, CC608SUB, CC708SUB } source;
    char lang[1024];
    char iso639_2[4];
    uint8_t type; /* Closed Caption, Childrens, Directors etc */

    int hits;     /* How many hits/occurrences of this subtitle */
    int forced_hits; /* How many forced hits in this subtitle */

#ifdef __LIBHB__
    /* Internal data */
    hb_fifo_t * fifo_in;  /* SPU ES */
    hb_fifo_t * fifo_raw; /* Decoded SPU */
    hb_fifo_t * fifo_sync;/* Synced */
    hb_fifo_t * fifo_out; /* Correct Timestamps, ready to be muxed */
    hb_mux_data_t * mux_data;
#endif
};

typedef struct hb_subtitle_s hb_subtitle_t;


struct hb_audio_config_s
{
#if 0
    /* Output */
    struct
    {
            int track;      /* Output track number */
            uint32_t codec;  /* Output audio codec.
                                 * HB_ACODEC_AC3 means pass-through, then bitrate and samplerate
                                 * are ignored.
                                 */
            int samplerate; /* Output sample rate (Hz) */
            int bitrate;    /* Output bitrate (kbps) */
            int mixdown;    /* The mixdown format to be used for this audio track (see HB_AMIXDOWN_*) */
            double dynamic_range_compression; /* Amount of DRC that gets applied to this track */
            char * name;    /* Output track name */
    } out;
#endif
    /* Input */
    struct
    {
        int track;                /* Input track number */
        uint32_t codec;   /* Input audio codec */
        uint32_t codec_param; /* per-codec config info */
        uint32_t version; /* Bitsream version */
        uint32_t mode;    /* Bitstream mode, codec dependent encoding */
        int samplerate; /* Input sample rate (Hz) */
        int bitrate;    /* Input bitrate (kbps) */
        int channel_layout;
        /* channel_layout is the channel layout of this audio this is used to
        * provide a common way of describing the source audio
        */
    } in;
#if 0
    /* Misc. */
    union
    {
        int ac3;    /* flags.ac3 is only set when the source audio format is HB_ACODEC_AC3 */
        int dca;    /* flags.dca is only set when the source audio format is HB_ACODEC_DCA */
    } flags;
#define AUDIO_F_DOLBY (1 << 31)  /* set if source uses Dolby Surround */
#endif
    struct
    {
        char description[1024];
        char simple[1024];
        char iso639_2[4];
        uint8_t type; /* normal, visually impared, directors */
    } lang;
};

typedef struct hb_audio_config_s hb_audio_config_t;
typedef struct hb_buffer_s hb_buffer_t;

struct hb_buffer_s
{
    int           size;
    int           alloc;
    uint8_t *     data;
    int           cur;

    int64_t       sequence;

    int           id;
    int64_t       start;
    int64_t       stop;
    int           new_chap;

#define HB_FRAME_IDR    0x01
#define HB_FRAME_I      0x02
#define HB_FRAME_AUDIO  0x04
#define HB_FRAME_P      0x10
#define HB_FRAME_B      0x20
#define HB_FRAME_BREF   0x40
#define HB_FRAME_KEY    0x0F
#define HB_FRAME_REF    0xF0
    uint8_t       frametype;
    uint16_t       flags;

    /* Holds the output PTS from x264, for use by b-frame offsets in muxmp4.c */
    int64_t     renderOffset;

    int           x;
    int           y;
    int           width;
    int           height;

    hb_buffer_t * sub;

    hb_buffer_t * next;
};



/* Fifo */
struct hb_fifo_s
{
    /*hb_lock_t    * lock;*/
    uint32_t       capacity;
    uint32_t       size;
    uint32_t       buffer_size;
    hb_buffer_t  * first;
    hb_buffer_t  * last;
};
typedef struct hb_fifo_s hb_fifo_t;

#define HB_CONFIG_MAX_SIZE 8192
union hb_esconfig_u
{

    struct
    {
        uint8_t bytes[HB_CONFIG_MAX_SIZE];
        int     length;
    } mpeg4;

	struct
	{
	    uint8_t  sps[HB_CONFIG_MAX_SIZE];
	    int       sps_length;
	    uint8_t  pps[HB_CONFIG_MAX_SIZE];
	    int       pps_length;
        uint32_t init_delay;
	} h264;

    struct
    {
        uint8_t headers[3][HB_CONFIG_MAX_SIZE];
    } theora;

    struct
    {
        uint8_t bytes[HB_CONFIG_MAX_SIZE];
        int     length;
    } aac;

    struct
    {
        uint8_t headers[3][HB_CONFIG_MAX_SIZE];
        char *language;
    } vorbis;

    struct
    {
    	/* ac3flags stores the flags from the AC3 source, as found in scan.c */
    	int     ac3flags;
        // next two items are used by the bsinfo routine to accumulate small
        // frames until we have enough to validate the crc.
        int     len;        // space currently used in 'buf'
        uint8_t buf[HB_CONFIG_MAX_SIZE-sizeof(int)];
    } a52;

    struct
    {
    	/* dcaflags stores the flags from the DCA source, as found in scan.c */
    	int  dcaflags;
    } dca;

};
typedef union  hb_esconfig_u     hb_esconfig_t;

typedef struct __attribute__((__packed__))
{
    uint32_t FourCC;
    uint32_t BytesCount;
    uint32_t Type;
    uint32_t Handler;
    uint32_t Flags;
    uint16_t Priority;
    uint16_t Language;
    uint32_t InitialFrames;
    uint32_t Scale;
    uint32_t Rate;
    uint32_t Start;
    uint32_t Length;
    uint32_t SuggestedBufferSize;
    uint32_t Quality;
    uint32_t SampleSize;
    int16_t  Left;
    int16_t  Top;
    int16_t  Right;
    int16_t  Bottom;

} hb_avi_stream_header_t;

typedef struct __attribute__((__packed__))
{
    uint32_t FourCC;
    uint32_t BytesCount;
    uint32_t VideoFormatToken;
    uint32_t VideoStandard;
    uint32_t dwVerticalRefreshRate;
    uint32_t dwHTotalInT;
    uint32_t dwVTotalInLines;
    uint16_t dwFrameAspectRatioDen;
    uint16_t dwFrameAspectRatioNum;
    uint32_t dwFrameWidthInPixels;
    uint32_t dwFrameHeightInLines;
    uint32_t nbFieldPerFrame;
    uint32_t CompressedBMHeight;
    uint32_t CompressedBMWidth;
    uint32_t ValidBMHeight;
    uint32_t ValidBMWidth;
    uint32_t ValidBMXOffset;
    uint32_t ValidBMYOffset;
    uint32_t VideoXOffsetInT;
    uint32_t VideoYValidStartLine;

} hb_avi_vprp_info_t;

typedef struct __attribute__((__packed__))
{
    uint32_t FourCC;
    uint32_t BytesCount;
    uint32_t Size;
    uint32_t Width;
    uint32_t Height;
    uint16_t Planes;
    uint16_t BitCount;
    uint32_t Compression;
    uint32_t SizeImage;
    uint32_t XPelsPerMeter;
    uint32_t YPelsPerMeter;
    uint32_t ClrUsed;
    uint32_t ClrImportant;

} hb_bitmap_info_t;

typedef struct __attribute__((__packed__))
{
    uint32_t FourCC;
    uint32_t BytesCount;
    uint16_t FormatTag;
    uint16_t Channels;
    uint32_t SamplesPerSec;
    uint32_t AvgBytesPerSec;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint16_t Size;

} hb_wave_formatex_t;

typedef struct __attribute__((__packed__))
{
    uint16_t Id;
    uint32_t Flags;
    uint16_t BlockSize;
    uint16_t FramesPerBlock;
    uint16_t CodecDelay;

} hb_wave_mp3_t;

struct hb_mux_data_s
{
    uint32_t				fourcc;
    hb_avi_stream_header_t	header;
    hb_avi_vprp_info_t		vprp_header;
    union
    {
        hb_bitmap_info_t   v;
        struct
        {
            hb_wave_formatex_t f;
            hb_wave_mp3_t      m;
        } a;
    } format;
};

typedef struct hb_mux_data_s hb_mux_data_t;


struct hb_audio_s
{
    int id;

    hb_audio_config_t config;
#if 0
    struct {
        hb_fifo_t * fifo_in;   /* AC3/MPEG/LPCM ES */
        hb_fifo_t * fifo_raw;  /* Raw audio */
        hb_fifo_t * fifo_sync; /* Resampled, synced raw audio */
        hb_fifo_t * fifo_out;  /* MP3/AAC/Vorbis ES */

        hb_esconfig_t config;
        hb_mux_data_t * mux_data;
    } priv;
#endif
};
typedef struct hb_audio_s hb_audio_t;


struct hb_chapter_s
{
    int      index;
    int      pgcn;
    int      pgn;
    int      cell_start;
    int      cell_end;
    int      block_start;
    int      block_end;
    int      block_count;

    /* Visual-friendly duration */
    int      hours;
    int      minutes;
    int      seconds;

    /* Exact duration (in 1/90000s) */
    uint64_t duration;

    /* Optional chapter title */
    char     title[1024];
};
typedef struct hb_chapter_s hb_chapter_t;


struct hb_list_s
{
    /* Pointers to items in the list */
    void ** items;

    /* How many (void *) allocated in 'items' */
    int     items_alloc;

    /* How many valid pointers in 'items' */
    int     items_count;
};

typedef struct hb_list_s hb_list_t;


struct hb_metadata_s 
{
    char  name[255];
    char  artist[255];
    char  composer[255];
    char  release_date[255];
    char  comment[1024];
    char  album[255];
    char  genre[255];
    uint32_t coverart_size;
    uint8_t *coverart;
};

typedef struct hb_metadata_s hb_metadata_t;


struct hb_title_s
{
    char        dvd[1024];
    char        name[1024];
    int         index;
    int         vts;
    int         ttn;
    int         cell_start;
    int         cell_end;
    int         block_start;
    int         block_end;
    int         block_count;
    int         angle_count;

    /* Visual-friendly duration */
    int         hours;
    int         minutes;
    int         seconds;

    /* Exact duration (in 1/90000s) */
    uint64_t    duration;

    double      aspect;             // aspect ratio for the title's video
    double      container_aspect;   // aspect ratio from container (0 if none)
    int         width;
    int         height;
    int         pixel_aspect_width;
    int         pixel_aspect_height;
    int         rate;
    int         rate_base;
    int         crop[4];
    enum { HB_MPEG2_PS_DEMUXER = 0, HB_MPEG2_TS_DEMUXER, HB_NULL_DEMUXER } demuxer;
    int         detected_interlacing;
    int         video_id;               /* demuxer stream id for video */
    int         video_codec;            /* worker object id of video codec */
    int         video_codec_param;      /* codec specific config */
    const char  *video_codec_name;
    int         video_bitrate;
    const char  *container_name;
    int         data_rate;

    uint32_t    palette[16];

    hb_metadata_t *metadata;

    hb_list_t * list_chapter;
    hb_list_t * list_audio;
    hb_list_t * list_subtitle;

    /* Job template for this title */
    /*hb_job_t  * job;*/

    uint32_t    flags;
                // set if video stream doesn't have IDR frames
#define         HBTF_NO_IDR (1 << 0)
};

typedef struct hb_title_s hb_title_t;

struct hb_dvdread_s
{
    char         * path;

    dvd_reader_t * reader;
    ifo_handle_t * vmg;

    int            vts;
    int            ttn;
    ifo_handle_t * ifo;
    dvd_file_t   * file;

    pgc_t        * pgc;
    int            cell_start;
    int            cell_end;
    int            title_start;
    int            title_end;
    int            title_block_count;
    int            cell_cur;
    int            cell_next;
    int            cell_overlap;
    int            block;
    int            pack_len;
    int            next_vobu;
    int            in_cell;
    int            in_sync;
    uint16_t       cur_vob_id;
    uint8_t        cur_cell_id;
};

struct hb_dvdnav_s
{
    char         * path;

    dvdnav_t     * dvdnav;
    dvd_reader_t * reader;
    ifo_handle_t * vmg;
    int            title;
    int            title_block_count;
    int            chapter;
    hb_list_t    * list_chapter;
    int            stopped;
};

typedef struct hb_dvdnav_s hb_dvdnav_t;
typedef struct hb_dvdread_s hb_dvdread_t;

union hb_dvd_s
{
    hb_dvdread_t dvdread;
    hb_dvdnav_t  dvdnav;
};

typedef union hb_dvd_s hb_dvd_t;

#if 0
struct hb_dvd_func_s
{
    /*hb_dvd_s *    (* init)        ( char * );*/
    /*void          (* close)       ( hb_dvd_s ** );*/
    char        * (* name)        ( char * );
    int           (* title_count) ( hb_dvd_s * );
    hb_title_t  * (* title_scan)  ( hb_dvd_s *, int );
    int           (* start)       ( hb_dvd_s *, hb_title_t *, int );
    void          (* stop)        ( hb_dvd_s * );
    int           (* seek)        ( hb_dvd_s *, float );
    int           (* read)        ( hb_dvd_s *, hb_buffer_t * );
    int           (* chapter)     ( hb_dvd_s * );
    int           (* angle_count) ( hb_dvd_s * );
    void          (* set_angle)   ( hb_dvd_s *, int );
};
typedef struct hb_dvd_func_s hb_dvd_func_t;

hb_dvd_func_t * hb_dvdnav_methods( void );
hb_dvd_func_t * hb_dvdread_methods( void );
#endif
#endif // HB_DVD_H


