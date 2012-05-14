/* $Id: dvd.h,v 1.1 2004/08/02 07:19:05 stebbins Exp $

   This file is part of the HandBrake source code.
   Homepage: <http://handbrake.fr/>.
   It may be used under the terms of the GNU General Public License. */

#ifndef DVD_INTERFACE_H
#define DVD_INTERFACE_H
void * hb_list_item( hb_list_t * l, int i );
hb_dvd_t    * hb_dvdnav_init( char * path );
int hb_dvdnav_reset( hb_dvdnav_t * d );
int hb_dvd_region(char *device, int *region_mask);
int           hb_dvdnav_title_count( hb_dvd_t * d );
hb_title_t  * hb_dvdnav_title_scan( hb_dvd_t * d, int t );
int           hb_dvdnav_start( hb_dvd_t * d, hb_title_t *title, int chapter );
void          hb_dvdnav_stop( hb_dvd_t * d );
dvdnav_status_t hb_dvdnav_get_position(hb_dvd_t *e, uint32_t *pos,
				    uint32_t *len) ;
dvdnav_status_t hb_dvdnav_sector_search(hb_dvd_t *e,
				     uint64_t offset, int32_t origin) ;
int hb_dvdnav_read( hb_dvd_t * e, uint8_t* b, int* len, int* dvd_event);
int hb_dvdnav_seek( hb_dvd_t * e, uint64_t sector );
int           hb_dvdnav_chapter( hb_dvd_t * d );
void          hb_dvdnav_close( hb_dvd_t ** _d );
int           hb_dvdnav_angle_count( hb_dvd_t * d );
void          hb_dvdnav_set_angle( hb_dvd_t * e, int angle );

hb_dvd_t * hb_dvdread_init( char * path );
void hb_dvdread_close( hb_dvd_t ** _d );
int hb_dvdread_title_count( hb_dvd_t * e );
hb_title_t * hb_dvdread_title_scan( hb_dvd_t * e, int t );
int hb_dvdread_start( hb_dvd_t * e, hb_title_t *title, int chapter );
void hb_dvdread_stop( hb_dvd_t * e );
dvdnav_status_t hb_dvdread_get_position(hb_dvd_t *e, uint32_t *pos,
				    uint32_t *len) ;
int hb_dvdread_seek( hb_dvd_t * e, uint64_t sector );
int hb_dvdread_read( hb_dvd_t * e, hb_buffer_t * b );
int hb_dvdread_chapter( hb_dvd_t * e );
int hb_dvdread_angle_count( hb_dvd_t * d );
void hb_dvdread_set_angle( hb_dvd_t * d, int angle );

void hb_title_close( hb_title_t ** _t );

#endif // DVD_INTERFACE_H


