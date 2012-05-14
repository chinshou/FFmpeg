/*
 * SSA/ASS subtitles rendering filter, using libssa.
 * Based on vf_drawbox.c from libavfilter and vf_ass.c from mplayer.
 *
 * Copyright (c) 2006 Evgeniy Stepanov <eugeni.stepa...@gmail.com>
 * Copyright (c) 2008 Affine Systems, Inc (Michael Sullivan, Bobby Impollonia)
 * Copyright (c) 2009 Alexey Lebedeff <bina...@binarin.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

/*
 * Usage: '-vfilters ass=filename:somefile.ass|margin:50|encoding:utf-8'
 * Only 'filename' param is mandatory.
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <ass.h>
#include <ass_library.h>

#include "avfilter.h"
#include "subreader.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avstring.h"

extern char *sub_cp;

typedef struct
{
  /*ass_library_t *ass_library;*/
  void *ass_library;
  ASS_Renderer *ass_renderer;
  ASS_Track *ass_track;

  sub_data *subd;  

  int margin;
  char *filename;
  char *font;
  char *color;
  //char filename[512];
  char *encoding;

  int frame_width, frame_height;
  int vsub,hsub;   //< chroma subsampling

  int enc; //1:encoder 0:player

} AssContext;

char **ass_force_style_list = NULL;
int ass_use_margins = 0;
//char* ass_color = NULL;
char* ass_border_color = NULL;
char* ass_styles_file = NULL;
int ass_hinting = ASS_HINTING_NATIVE + 4; // native hinting for unscaled osd
float text_font_scale_factor = 3.5;
// 0 = no autoscale
// 1 = video height
// 2 = video width
// 3 = diagonal
int subtitle_autoscale = 3;



static ASS_Track* ass_default_track(AssContext* context) {
	ASS_Track* track = ass_new_track(context->ass_library);

	track->track_type = TRACK_TYPE_ASS;
	track->Timer = 100.;
	track->PlayResY = 288;
	track->WrapStyle = 0;

	if (ass_styles_file)
		ass_read_styles(track, ass_styles_file, 0);

	if (track->n_styles == 0) {
		ASS_Style* style;
		int sid;
		double fs;
		uint32_t c1, c2;

		sid = ass_alloc_style(track);
		style = track->styles + sid;
		style->Name = av_strdup("Default");
		style->FontName = av_strdup("Arial");
		style->treat_fontname_as_pattern = 1;

		fs = track->PlayResY * text_font_scale_factor / 100.;
		// approximate autoscale coefficients
		if (subtitle_autoscale == 2)
			fs *= 1.3;
		else if (subtitle_autoscale == 3)
			fs *= 1.4;
		style->FontSize = fs;

		if (context->color) c1 = strtoll(context->color, NULL, 16);
		else c1 = 0xFFFF0000;
		if (ass_border_color) c2 = strtoll(ass_border_color, NULL, 16);
		else c2 = 0x00000000;

		style->PrimaryColour = c1;
		style->SecondaryColour = c1;
		style->OutlineColour = c2;
		style->BackColour = 0x00000000;
		style->BorderStyle = 1;
		style->Alignment = 2;
		style->Outline = 2;
		style->MarginL = 10;
		style->MarginR = 10;
		style->MarginV = 5;
		style->ScaleX = 1.;
		style->ScaleY = 1.;
	}

	ass_process_force_style(track);
	return track;
}

static int check_duplicate_plaintext_event(ASS_Track* track)
{
	int i;
	ASS_Event* evt = track->events + track->n_events - 1;

	for (i = 0; i<track->n_events - 1; ++i) // ignoring last event, it is the one we are comparing with
		if (track->events[i].Start == evt->Start &&
		    track->events[i].Duration == evt->Duration &&
		    strcmp(track->events[i].Text, evt->Text) == 0)
			return 1;
	return 0;
}

/**
 * \brief Convert subtitle to ass_event_t for the given track
 * \param ass_track_t track
 * \param sub subtitle to convert
 * \return event id
 * note: assumes that subtitle is _not_ fps-based; caller must manually correct
 *   Start and Duration in other case.
 **/
static int ass_process_subtitle(ASS_Track* track, subtitle* sub)
{
    int eid;
    ASS_Event* event;
	int len = 0, j;
	char* p;
	char* end;

	eid = ass_alloc_event(track);
	event = track->events + eid;

	event->Start = sub->start * 10;
	event->Duration = (sub->end - sub->start) * 10;
	event->Style = 0;

	for (j = 0; j < sub->lines; ++j)
		len += sub->text[j] ? strlen(sub->text[j]) : 0;

	len += 2 * sub->lines; // '\N', including the one after the last line
	len += 6; // {\anX}
	len += 1; // '\0'

	event->Text = av_malloc(len);
	end = event->Text + len;
	p = event->Text;

	if (sub->alignment)
		p += snprintf(p, end - p, "{\\an%d}", sub->alignment);

	for (j = 0; j < sub->lines; ++j)
		p += snprintf(p, end - p, "%s\\N", sub->text[j]);

	if (sub->lines > 0) p-=2; // remove last "\N"
	*p = 0;

	if (check_duplicate_plaintext_event(track)) {
		ass_free_event(track, eid);
		track->n_events--;
		return -1;
	}

	//mp_msg(MSGT_ASS, MSGL_V, "plaintext event at %" PRId64 ", +%" PRId64 ": %s  \n",
	//		(int64_t)event->Start, (int64_t)event->Duration, event->Text);

	return eid;
}


/**
 * \brief Convert subdata to ass_track
 * \param subdata subtitles struct from subreader
 * \param fps video framerate
 * \return newly allocated ass_track, filled with subtitles from subdata
 */
static ASS_Track* ass_read_subdata(AssContext* context, double fps) {
	ASS_Track* track;
	int i;

	track = ass_default_track(context);
	track->name = context->subd->filename ? av_strdup(context->subd->filename) : 0;

	for (i = 0; i < context->subd->sub_num; ++i) {
		int eid = ass_process_subtitle(track, context->subd->subtitles + i);
		if (eid < 0)
			continue;
		if (!context->subd->sub_uses_time) {
			track->events[eid].Start *= 100. / fps;
			track->events[eid].Duration *= 100. / fps;
		}
	}
	return track;
}

static int parse_args(AVFilterContext *ctx, AssContext *context, const char* args);

static av_cold int init(AVFilterContext *ctx, const char *args, void *opaque)
{
  AssContext *context= ctx->priv;
  //int num_fields;

  /* defaults */
  context->margin = 10;
  context->encoding = "utf-8";
  context->enc = 0;//player

  if ( parse_args(ctx, context, args) )
    return 1;

  av_log(ctx, AV_LOG_ERROR,"file:%s font:%s\n", context->filename, context->font);
#if 0  
  num_fields = sscanf(args, "%512[^|]",
					  context->filename);
  if (num_fields != 1) {
	  av_log(ctx, AV_LOG_ERROR,
			 " file=%s\n",
			 context->filename);
	  return -1;
  }
  
#endif
  return 0;
}

static int query_formats(AVFilterContext *ctx)
{

  enum PixelFormat pix_fmts[] = {
	  PIX_FMT_YUV444P,	PIX_FMT_YUV422P,  PIX_FMT_YUV420P,
	  PIX_FMT_YUV411P,	PIX_FMT_YUV410P,
	  PIX_FMT_YUVJ444P, PIX_FMT_YUVJ422P, PIX_FMT_YUVJ420P,
	  PIX_FMT_YUV440P,	PIX_FMT_YUVJ440P,
	  PIX_FMT_NONE
  };
  
  avfilter_set_common_pixel_formats
    (ctx, avfilter_make_format_list(pix_fmts));
  return 0;
}

static int config_input(AVFilterLink *link)
{
  AssContext *context = link->dst->priv;
  int i;

  context->frame_width = link->w;
  context->frame_height = link->h;

  context->ass_library = ass_library_init();

  if ( !context->ass_library ) {
    av_log(0, AV_LOG_ERROR, "ass_library_init() failed!\n");
    return 1;
  }

  ass_set_fonts_dir(context->ass_library, "");
  ass_set_extract_fonts(context->ass_library, 1);
  ass_set_style_overrides(context->ass_library, NULL);

  context->ass_renderer = ass_renderer_init(context->ass_library);
  if ( ! context->ass_renderer ) {
    av_log(0, AV_LOG_ERROR, "ass_renderer_init() failed!\n");
    return 1;
  }

  ass_set_frame_size(context->ass_renderer, link->w, link->h);
  ass_set_margins(context->ass_renderer, context->margin, context->margin, context->margin, context->margin);
  ass_set_use_margins(context->ass_renderer, 1);
  ass_set_font_scale(context->ass_renderer, 1.);
  ass_set_fonts(context->ass_renderer, context->font, "Arial", 0, NULL, 0);

  //Todo:fix fps , temporarily the fps is set to 24.
  context->subd = sub_read_file(context->filename, 24);
  if (!context->subd)
  	av_log(0, AV_LOG_ERROR, "Failed to read subtitle file with sub_read()\n");
  else
    av_log(0, AV_LOG_ERROR, "Successfully read subtitle file with sub_read()\n");
  	
  context->ass_track = ass_read_file(context->ass_library, context->filename, context->encoding);

  if (!context->ass_track && context->subd){
  	//Todo: fix the fps
	context->ass_track = ass_read_subdata(context, 24);
  	}
  
  if ( !context->ass_track ) {
    av_log(0, AV_LOG_ERROR, "Failed to read subtitle file %s with ass_read_file()!\n", context->filename);
    return 1;
  }

  //sub_free(context->subd);

#if 0
  for (i = 0; i < context->ass_track->n_events; ++i) {
	  ASS_Event *event = context->ass_track->events + i;
	  event->Duration = 100000;
	  av_log(0, AV_LOG_ERROR, "event:%d duration(ms):%d start(ms):%d text:%s !\n", i,event->Duration, event->Start, event->Text);
  }
#endif  

  context->hsub = av_pix_fmt_descriptors[link->format].log2_chroma_w;
  context->vsub = av_pix_fmt_descriptors[link->format].log2_chroma_h;

  return 0;
}

static void start_frame(AVFilterLink *link, AVFilterBufferRef *picref)
{
  avfilter_start_frame(link->dst->outputs[0], picref);
}

#define _r(c)  ((c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)&0xFF)
#define rgba2y(c)  ( (( 263*_r(c)  + 516*_g(c) + 100*_b(c)) >> 10) + 16  )                                                                     
#define rgba2u(c)  ( ((-152*_r(c) - 298*_g(c) + 450*_b(c)) >> 10) + 128 )                                                                      
#define rgba2v(c)  ( (( 450*_r(c) - 376*_g(c) -  73*_b(c)) >> 10) + 128 )                                                                      

static void draw_ass_image(AVFilterBufferRef *pic, ASS_Image *img, AssContext *context)
{
  unsigned char *row[4];
  unsigned char c_y = rgba2y(img->color);
  unsigned char c_u = rgba2u(img->color);
  unsigned char c_v = rgba2v(img->color);
  unsigned char opacity = 255 - _a(img->color);
  unsigned char *src;
  int i, j;

  unsigned char *bitmap = img->bitmap;
  int bitmap_w = img->w;
  int bitmap_h = img->h;
  int dst_x = img->dst_x;
  int dst_y = img->dst_y;

  int channel;
  int x,y;

  src = bitmap;

  for (i = 0; i < bitmap_h; ++i) {
    y = dst_y + i;
    if ( y >= pic->video->h )
      break;

    row[0] = pic->data[0] + y * pic->linesize[0];

    for (channel = 1; channel < 3; channel++)
      row[channel] = pic->data[channel] +
	pic->linesize[channel] * (y>> context->vsub);

    for (j = 0; j < bitmap_w; ++j) {
      unsigned k = ((unsigned)src[j]) * opacity / 255;

      x = dst_x + j;
      if ( y >= pic->video->w )
	break;

      row[0][x] = (k*c_y + (255-k)*row[0][x]) / 255;
      row[1][x >> context->hsub] = (k*c_u + (255-k)*row[1][x >> context->hsub]) / 255;
      row[2][x >> context->hsub] = (k*c_v + (255-k)*row[2][x >> context->hsub]) / 255;
    }

    src += img->stride;
  } 
}

static void end_frame(AVFilterLink *link)
{
  AssContext *context = link->dst->priv;
  AVFilterLink* output = link->dst->outputs[0];
  AVFilterBufferRef *pic = link->cur_buf;
  

  //miliseconds
  int scale = context->enc?1:1000;
  int64_t picref_time = pic->pts * av_q2d(link->time_base)*scale;
#if 0
  av_log(0, AV_LOG_ERROR, "draw_ass_image pts_time:%I64d pts:%I64d num:%d den:%d \n", picref_time, pic->pts, link->time_base.num, link->time_base.den);
#endif
  ASS_Image* img = ass_render_frame(context->ass_renderer,
				      context->ass_track,
				      picref_time,
				      NULL);


  while ( img ) {
    draw_ass_image(pic, img, context);
    img = img->next;
  }

  avfilter_draw_slice(output, 0, pic->video->h, 1);
  avfilter_end_frame(output);
}

static int parse_args(AVFilterContext *ctx, AssContext *context, const char* args)
{
  char *arg_copy = av_strdup(args);
  char *strtok_arg = arg_copy;
  char *param;

  while ( param = strtok(strtok_arg, "#") ) {
    char *tmp = param;
    char *param_name;
    char *param_value;

    strtok_arg = NULL;

    while ( *tmp && *tmp != '$' ) {
      tmp++;
    }

    if ( param == tmp || ! *tmp ) {
      av_log(ctx, AV_LOG_ERROR, "Error while parsing arguments - must be like 'param1:value1|param2:value2'\n");
      return 1;
    }
    //av_log(ctx, AV_LOG_ERROR, "param:%s param_name size:%d'\n", param, tmp-param);
    param_name = av_malloc(tmp - param + 1);
    memset(param_name, 0, tmp - param + 1);
    av_strlcpy(param_name, param, tmp-param+1);

    tmp++;

    if ( ! *tmp ) {
      av_log(ctx, AV_LOG_ERROR, "Error while parsing arguments - parameter value cannot be empty\n");
      return 1;
    }

    param_value = av_strdup(tmp);

    if ( !strcmp("margin", param_name ) ) {
      context->margin = atoi(param_value);
    } else if ( !strcmp("filename", param_name ) ) {
      context->filename = av_strdup(param_value);
    } else if ( !strcmp("encoding", param_name ) ) {
      context->encoding = av_strdup(param_value);
    } else if ( !strcmp("font", param_name ) ) {
      context->font = av_strdup(param_value);
    } else if ( !strcmp("color", param_name ) ) {
      context->color = av_strdup(param_value);
    } else if ( !strcmp("codepage", param_name ) ) {
      sub_cp = av_strdup(param_value);
    }else if ( !strcmp("enc", param_name ) ) {
      context->enc = atoi(param_value);
    }		
	
	else {
      av_log(ctx, AV_LOG_ERROR, "Error while parsing arguments - unsupported parameter '%s'\n", param_name);
      return 1;
    }
    av_free(param_name);
    av_free(param_value);
  }

  if ( ! context->filename ) {
    av_log(ctx, AV_LOG_ERROR, "Error while parsing arguments - mandatory parameter 'filename' missing\n");
    return 1;
  }

  if ( ! context->font ) {
    av_log(ctx, AV_LOG_ERROR, "Error while parsing arguments - mandatory parameter 'font' missing\n");
    return 1;
  }
  
  return 0;
}

AVFilter avfilter_vf_ass=
  {
    .name      = "ass",
    .priv_size = sizeof(AssContext),
    .init      = init,

    .query_formats   = query_formats,
    .inputs    = (AVFilterPad[]) {{ .name            = "default",
                                    .type            = AVMEDIA_TYPE_VIDEO,
                                    .get_video_buffer = avfilter_null_get_video_buffer,
                                    .start_frame     = start_frame,
                                    .end_frame       = end_frame,
                                    .config_props    = config_input,
                                    .min_perms       = AV_PERM_WRITE |
				    AV_PERM_READ,
                                    .rej_perms       = AV_PERM_REUSE |
				    AV_PERM_REUSE2},
                                  { .name = NULL}},
    .outputs   = (AVFilterPad[]) {{ .name            = "default",
                                    .type            = AVMEDIA_TYPE_VIDEO, },
                                  { .name = NULL}},
  };
