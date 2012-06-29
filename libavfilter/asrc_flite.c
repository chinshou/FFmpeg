/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * flite voice synth source
 */

#include <flite/flite.h>
#include "libavutil/audioconvert.h"
#include "libavutil/file.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "audio.h"
#include "formats.h"

typedef struct {
    const AVClass *class;
    char *voice_str;
    char *textfile;
    char *text;
    cst_wave *wave;
    int16_t *wave_samples;
    int      wave_nb_samples;
    int64_t pts;
} FliteContext;

#define OFFSET(x) offsetof(FliteContext, x)

static const AVOption flite_options[] = {
    { "textfile", "set text filename to speech", OFFSET(textfile),  AV_OPT_TYPE_STRING, {.str=NULL}, CHAR_MIN, CHAR_MAX },
    { "text",     "set text to speech",          OFFSET(text),      AV_OPT_TYPE_STRING, {.str=NULL}, CHAR_MIN, CHAR_MAX },
    { "voice",    "set voice",                   OFFSET(voice_str), AV_OPT_TYPE_STRING, {.str=NULL}, CHAR_MIN, CHAR_MAX },
    { NULL }
};

static const AVClass flite_class = {
    .class_name = "flite",
    .item_name  = av_default_item_name,
    .option     = flite_options,
    .version    = LIBAVUTIL_VERSION_INT,
    .category   = AV_CLASS_CATEGORY_FILTER,
};

cst_voice *register_cmu_us_kal(void *);

static int init(AVFilterContext *ctx, const char *args, void *opaque)
{
    FliteContext *flite = ctx->priv;
    int err = 0;
    cst_voice *voice;

    flite->class = &flite_class;
    av_opt_set_defaults(flite);

    if ((err = av_set_options_string(flite, args, "=", ":")) < 0) {
        av_log(ctx, AV_LOG_ERROR, "Error parsing options string: '%s'\n", args);
        return err;
    }

    if ((err = flite_init())) {
        av_log(ctx, AV_LOG_ERROR, "Could not init flite");
        return AVERROR(EINVAL);
    }

    voice = register_cmu_us_kal(NULL);

    if (flite->textfile) {
        uint8_t *textbuf;
        size_t textbuf_size;

        if (flite->text) {
            av_log(ctx, AV_LOG_ERROR,
                   "Both text and text file provided. Please provide only one\n");
            return AVERROR(EINVAL);
        }
        if ((err = av_file_map(flite->textfile, &textbuf, &textbuf_size, 0, ctx)) < 0) {
            av_log(ctx, AV_LOG_ERROR,
                   "The text file '%s' could not be read or is empty\n",
                   flite->textfile);
            return err;
        }

        if (!(flite->text = av_malloc(textbuf_size+1)))
            return AVERROR(ENOMEM);
        memcpy(flite->text, textbuf, textbuf_size);
        flite->text[textbuf_size] = 0;
        av_file_unmap(textbuf, textbuf_size);
    }

    /* synth all the file data in block */
    flite->wave = flite_text_to_wave(flite->text, voice);
    flite->wave_samples    = flite->wave->samples;
    flite->wave_nb_samples = flite->wave->num_samples;
    return 0;
}

static int config_props(AVFilterLink *outlink)
{
    FliteContext *flite = outlink->src->priv;

    outlink->sample_rate = flite->wave->sample_rate;
    outlink->time_base = (AVRational){1, flite->wave->sample_rate};
    return 0;
}

static int query_formats(AVFilterContext *ctx)
{
    FliteContext *flite = ctx->priv;

    static enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int64_t chlayouts[] = { av_get_default_channel_layout(flite->wave->num_channels), -1 };

    ff_set_common_formats(ctx, ff_make_format_list(sample_fmts));
    ff_set_common_channel_layouts(ctx, avfilter_make_format64_list(chlayouts));

    return 0;
}

static int request_frame(AVFilterLink *outlink)
{
    AVFilterBufferRef *samplesref;
    FliteContext *flite = outlink->src->priv;
    int nb_samples = FFMIN(flite->wave_nb_samples, 512);

    if (!nb_samples)
        return AVERROR_EOF;

    samplesref = ff_get_audio_buffer(outlink, AV_PERM_WRITE, nb_samples);

    memcpy(samplesref->data[0], flite->wave_samples,
           nb_samples * flite->wave->num_channels * 2);
    samplesref->pts = flite->pts;
    samplesref->pos = -1;
    samplesref->audio->sample_rate = flite->wave->sample_rate;
    flite->pts += nb_samples;
    flite->wave_samples += nb_samples * flite->wave->num_channels;
    flite->wave_nb_samples -= nb_samples;

    ff_filter_samples(outlink, samplesref);

    return 0;
}

AVFilter avfilter_asrc_flite = {
    .name        = "flite",
    .description = NULL_IF_CONFIG_SMALL("Flite voice synth source."),

    .query_formats = query_formats,
    .init        = init,
    .priv_size   = sizeof(FliteContext),

    .inputs      = (AVFilterPad[]) {{ .name = NULL}},

    .outputs     = (AVFilterPad[]) {{ .name = "default",
                                      .type = AVMEDIA_TYPE_AUDIO,
                                      .config_props = config_props,
                                      .request_frame = request_frame, },
                                    { .name = NULL}},
};
