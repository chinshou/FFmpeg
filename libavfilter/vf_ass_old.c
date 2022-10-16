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

#include <ass/ass.h>

#include "avfilter.h"
#include "formats.h"
#include "video.h"
#include "subreader.h"
#include "drawutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "internal.h"

extern char *sub_cp;

typedef struct
{
  const AVClass *class;
  /*ass_library_t *ass_library;*/
  void *ass_library;
  ASS_Renderer *ass_renderer;
  ASS_Track *ass_track;

  sub_data *subd;  

  int margin;
  char *filename;
  char *font;
  char *color;
  char *encoding;

  int frame_width, frame_height;
  int vsub,hsub;   //< chroma subsampling

} AssContext;

#define OFFSET(x) offsetof(AssContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM

static const AVOption options[] = {
    {"filename",       "set the filename of file to read",                         OFFSET(filename),   AV_OPT_TYPE_STRING,     {.str = NULL},  CHAR_MIN, CHAR_MAX, FLAGS },
    {"font",       "set the filename of font file to read",                         OFFSET(font),   AV_OPT_TYPE_STRING,     {.str = NULL},  CHAR_MIN, CHAR_MAX, FLAGS },
    {"color",       "set the color",                         OFFSET(color),   AV_OPT_TYPE_STRING,     {.str = NULL},  CHAR_MIN, CHAR_MAX, FLAGS },
    {NULL},
};

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

#define ass_old_options options
AVFILTER_DEFINE_CLASS(ass_old);

static av_cold int init_ass(AVFilterContext *ctx)
{
  AssContext *context= ctx->priv;

  /* defaults */
  context->margin = 10;
  context->encoding = "utf-8";

  if (!context->filename) {
      av_log(ctx, AV_LOG_ERROR, "No filename provided!\n");
      return AVERROR(EINVAL);
  }

  if (!context->font) {
      av_log(ctx, AV_LOG_ERROR, "No font filename provided!\n");
      return AVERROR(EINVAL);
  }

  if (!context->color) {
      av_log(ctx, AV_LOG_ERROR, "No color provided!\n");
      return AVERROR(EINVAL);
  }

  av_log(ctx, AV_LOG_ERROR,"file:%s font:%s\n", context->filename, context->font);

  return 0;
}

static int query_formats(AVFilterContext *ctx)
{
    ff_set_common_formats(ctx, ff_draw_supported_pixel_formats(0));
  return 0;
}

static int config_input(AVFilterLink *link)
{
  AssContext *context = link->dst->priv;

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

  context->hsub = av_pix_fmt_desc_get(link->format)->log2_chroma_w;
  context->vsub = av_pix_fmt_desc_get(link->format)->log2_chroma_h;


  return 0;
}

static av_cold void uninit(AVFilterContext *ctx)
{
    AssContext *ass = ctx->priv;

    //av_opt_free(ass);
    if (ass->ass_track)
        ass_free_track(ass->ass_track);
    if (ass->ass_renderer)
        ass_renderer_done(ass->ass_renderer);
    if (ass->ass_library)
        ass_library_done(ass->ass_library);
}

#define _r(c)  ((c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)&0xFF)
#define rgba2y(c)  ( (( 263*_r(c)  + 516*_g(c) + 100*_b(c)) >> 10) + 16  )                                                                     
#define rgba2u(c)  ( ((-152*_r(c) - 298*_g(c) + 450*_b(c)) >> 10) + 128 )                                                                      
#define rgba2v(c)  ( (( 450*_r(c) - 376*_g(c) -  73*_b(c)) >> 10) + 128 )                                                                      

static void draw_ass_image(AVFrame *picref, ASS_Image *img, AssContext *context)
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
    if ( y >= picref->height )
      break;

    row[0] = picref->data[0] + y * picref->linesize[0];

    for (channel = 1; channel < 3; channel++)
      row[channel] = picref->data[channel] +
	picref->linesize[channel] * (y>> context->vsub);

    for (j = 0; j < bitmap_w; ++j) {
      unsigned k = ((unsigned)src[j]) * opacity / 255;

      x = dst_x + j;
      if ( y >= picref->width )
	break;

      row[0][x] = (k*c_y + (255-k)*row[0][x]) / 255;
      row[1][x >> context->hsub] = (k*c_u + (255-k)*row[1][x >> context->hsub]) / 255;
      row[2][x >> context->hsub] = (k*c_v + (255-k)*row[2][x >> context->hsub]) / 255;
    }

    src += img->stride;
  } 
}

static int filter_frame(AVFilterLink *inlink, AVFrame *picref)
{
  AssContext *context = inlink->dst->priv;
  AVFilterLink* output = inlink->dst->outputs[0];
  

  //miliseconds
  //int scale = context->enc?1:1000;
  int scale = 1000;
  int64_t picref_time = picref->pts * av_q2d(inlink->time_base)*scale;

  ASS_Image* img = ass_render_frame(context->ass_renderer,
				      context->ass_track,
				      picref_time,
				      NULL);


  while ( img ) {
    draw_ass_image(picref, img, context);
    img = img->next;
  }

  return ff_filter_frame(output, picref);
}

AVFilter ff_vf_ass_old = {
    .name      = "ass_old",
    .description   = NULL_IF_CONFIG_SMALL("Render subtitles onto input video using the libass library."),
    .priv_size = sizeof(AssContext),
    .init      = init_ass,
    .uninit    = uninit,	
    FILTER_QUERY_FUNC(query_formats),

    .inputs = (const AVFilterPad[]) {
        { .name             = "default",
          .type             = AVMEDIA_TYPE_VIDEO,
          .get_buffer = ff_null_get_video_buffer,
          .filter_frame     = filter_frame,
          .config_props     = config_input },
        { .name = NULL}
    },
    .outputs = (const AVFilterPad[]) {
        { .name             = "default",
          .type             = AVMEDIA_TYPE_VIDEO, },
        { .name = NULL}
    },
    .priv_class    = &ass_old_class,
};
