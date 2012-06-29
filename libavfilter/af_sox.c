/*
 * Copyright (c) 2011 Mina Nagy Zaki
<<<<<<< HEAD
=======
 * Copyright (c) 2012 Stefano Sabatini
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
 *
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
<<<<<<< HEAD
 * libsox wrapper filter
 */

#include <sox.h>
#include <string.h>
=======
 * Sox wrapper.
 */

#include <sox.h>
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be

#include "libavutil/avstring.h"
#include "libavutil/audioconvert.h"
#include "libavutil/mem.h"
#include "avfilter.h"
<<<<<<< HEAD
=======
#include "internal.h"
#include "audio.h"
#include "formats.h"
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be

static int soxinit = -1;

#define NUM_ADDED_ARGS 5

typedef struct {
    sox_effect_t *effect;
} SoxContext;

<<<<<<< HEAD
/* sox needs a variable number of params */
static inline void realloc_argv(char ***argv, int *numargs)
{
    *numargs += NUM_ADDED_ARGS;
    *argv = av_realloc(*argv, *numargs * sizeof(**argv));
    if (!*argv)
        av_free(*argv);
}

static av_cold int init(AVFilterContext *ctx, const char *args0, void *opaque)
{
    SoxContext *sox = ctx->priv;
    char **argv = NULL; int argc = 0, numargs = 0;
    char *args, *ptr = NULL;
    sox_encodinginfo_t *enc;

    // initialize SoX if necessary
    if (soxinit != SOX_SUCCESS && (soxinit = sox_init()) != SOX_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "Sox error occurred: '%s'\n",
               sox_strerror(soxinit));
        return AVERROR(EINVAL);
    }

    args = av_strdup(args0);

    /* create an array of arguments to pass to sox_create_effect */
    realloc_argv(&argv, &numargs);
    argv[argc++] = av_strtok(args, " ", &ptr);
    while (argv[argc++] = av_strtok(NULL, " ", &ptr)) {
        if (argc == numargs) {
            realloc_argv(&argv, &numargs);
            if (!numargs) {
                av_log(ctx, AV_LOG_ERROR, "Could not allocate memory!\n");
                av_free(args);
                return AVERROR(ENOMEM);
=======
static inline int realloc_argv(char ***argv, int *numargs)
{
    *numargs += NUM_ADDED_ARGS;
    *argv = av_realloc_f(*argv, *numargs, sizeof(**argv));
    if (!*argv)
        return AVERROR(ENOMEM);
    else
        return *numargs;
}

static av_cold int init(AVFilterContext *ctx, const char *args, void *opaque)
{
    SoxContext *sox = ctx->priv;
    char **argv = NULL;
    char *args1 = av_strdup(args);
    int argc = 0, numargs = 0, ret = 0;
    sox_encodinginfo_t *encoding;
    const sox_effect_handler_t *handler;
    char *saveptr;

#define FAIL(err) ret = err; goto end;

    // initialize SoX if necessary
    if (soxinit != SOX_SUCCESS && (soxinit = sox_init()) != SOX_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "Error when initing libsox: '%s'\n",
               sox_strerror(soxinit));
        FAIL(AVERROR(EINVAL));
    }

    // create arguments array
    if ((ret = realloc_argv(&argv, &numargs)) < 0) {
        FAIL(ret);
    }

    // parse arguments into a string array
    argv[argc++] = av_strtok(args1, " ", &saveptr);
    while (argv[argc++] = av_strtok(NULL, " ", &saveptr)) {
        if (argc == numargs) {
            if ((ret = realloc_argv(&argv, &numargs)) < 0) {
                FAIL(ret);
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
            }
        }
    }

<<<<<<< HEAD
    // Create the effect
    sox->effect = sox_create_effect(sox_find_effect(argv[0]));
    if (!sox->effect) {
        av_log(ctx, AV_LOG_ERROR, "Could not create Sox effect '%s'\n", argv[0]);
        av_free(args);
=======
    handler = sox_find_effect(argv[0]);
    if (!handler) {
        av_log(ctx, AV_LOG_ERROR, "Could not find Sox effect named '%s'\n", argv[0]);
        FAIL(AVERROR(EINVAL));
    }
    sox->effect = sox_create_effect(handler);
    if (!sox->effect) {
        av_log(ctx, AV_LOG_ERROR, "Could not create Sox effect '%s'\n", argv[0]);
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
        return AVERROR(EINVAL);
    }

    if (sox->effect->handler.getopts(sox->effect, argc-1, argv) != SOX_SUCCESS) {
<<<<<<< HEAD
        av_log(ctx, AV_LOG_ERROR, "Invalid arguments to Sox effect '%s'\n", argv[0]);
        if (sox->effect->handler.usage)
            av_log(ctx, AV_LOG_ERROR, "Usage: %s\n", sox->effect->handler.usage);
        av_free(args);
        return AVERROR(EINVAL);
    }

    if (!(enc = av_malloc(sizeof(sox_encodinginfo_t)))) {
        av_free(args);
        return AVERROR(ENOMEM);
    }
    memset(enc, 0, sizeof(sox_encodinginfo_t));
    enc->bits_per_sample = 32;
    enc->encoding        = SOX_DEFAULT_ENCODING;
    sox->effect->out_encoding = sox->effect->in_encoding = enc;
    sox->effect->clips        = 0;
    sox->effect->imin         = 0;

    av_free(args);

    return 0;
=======
        av_log(ctx, AV_LOG_ERROR, "Invalid arguments to Sox effect\n");
        if (sox->effect->handler.usage)
            av_log(ctx, AV_LOG_ERROR, "Usage: %s\n", sox->effect->handler.usage);
        return AVERROR(EINVAL);
    }

    if (!(encoding = av_mallocz(sizeof(sox_encodinginfo_t))))
        return AVERROR(ENOMEM);

    encoding->encoding = SOX_DEFAULT_ENCODING;
    encoding->bits_per_sample = 32;

    sox->effect->out_encoding = sox->effect->in_encoding = encoding;
    sox->effect->clips        = 0;
    sox->effect->imin         = 0;

end:
    av_free(args1);
    return ret;
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
}

static av_cold void uninit(AVFilterContext *ctx)
{
    SoxContext *sox = ctx->priv;

    if (sox->effect)
        sox_delete_effect(sox->effect);
<<<<<<< HEAD
    ctx->priv = NULL;
    sox_quit();
}

static int config_output(AVFilterLink *outlink)
{
    SoxContext *sox = outlink->src->priv;
    sox_effect_t *effect = sox->effect;
    AVFilterLink *inlink = outlink->src->inputs[0];

    effect->in_signal.precision = 32;
    effect->in_signal.rate      = inlink->sample_rate;
    effect->in_signal.channels  =
        av_get_channel_layout_nb_channels(inlink->channel_layout);

    if (!(effect->handler.flags & SOX_EFF_CHAN))
        effect->out_signal.channels = effect->in_signal.channels;
    if (!(effect->handler.flags & SOX_EFF_RATE))
        effect->out_signal.rate = effect->in_signal.rate;

    if (effect->handler.start(effect) != SOX_SUCCESS) {
        av_log(outlink->src, AV_LOG_ERROR,
               "Could not start the sox effect.\n");
        return AVERROR(EINVAL);
    }

    outlink->sample_rate = effect->out_signal.rate;
    outlink->time_base = (AVRational) {1, effect->out_signal.rate };

    return 0;
}

static int query_formats(AVFilterContext *ctx)
{
    AVFilterLink *outlink = ctx->outputs[0];
    SoxContext *sox = ctx->priv;
    sox_effect_t *effect = sox->effect;
    AVFilterFormats *formats = NULL;

    avfilter_add_format(&formats, AV_SAMPLE_FMT_S32);
    avfilter_set_common_sample_formats(ctx, formats);

    if (effect->handler.flags & SOX_EFF_CHAN) {
        int64_t chlayout = av_get_default_channel_layout(effect->out_signal.channels);
        if (!chlayout) {
            av_log(ctx, AV_LOG_ERROR, "Invalid number of channels '%d' provided\n",
                   effect->out_signal.channels);
            return AVERROR(EINVAL);
        }
        formats = NULL;
        avfilter_add_format(&formats, chlayout);

        avfilter_formats_ref(formats,
                             &outlink->in_chlayouts);
        avfilter_formats_ref(avfilter_make_all_channel_layouts(),
                             &outlink->out_chlayouts);
    } else {
        avfilter_set_common_channel_layouts(ctx, avfilter_make_all_channel_layouts());
    }

    avfilter_set_common_packing_formats(ctx, avfilter_make_all_packing_formats());
=======
    sox->effect = NULL;
    sox_quit();
}

static int query_formats(AVFilterContext *ctx)
{
    SoxContext *sox = ctx->priv;
    AVFilterFormats *formats = NULL;
    AVFilterChannelLayouts *layouts = NULL;

    ff_add_format(&formats, AV_SAMPLE_FMT_S32);
    ff_set_common_formats(ctx, formats);

    if (sox->effect->handler.flags & SOX_EFF_CHAN) {
        layouts = NULL;
        ff_add_channel_layout(&layouts, av_get_default_channel_layout(sox->effect->out_signal.channels));

        ff_channel_layouts_ref(layouts, &ctx->outputs[0]->in_channel_layouts);
        ff_channel_layouts_ref(ff_all_channel_layouts(), &ctx->inputs[0]->out_channel_layouts);
    } else {
        ff_set_common_channel_layouts(ctx, ff_all_channel_layouts());
    }

    return 0;
}

static int config_output(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    SoxContext *sox = ctx->priv;
    AVFilterLink *inlink = ctx->inputs[0];

    sox->effect->in_signal.precision = 32;
    sox->effect->in_signal.rate      = inlink->sample_rate;
    sox->effect->in_signal.channels  =
        av_get_channel_layout_nb_channels(inlink->channel_layout);

    if (!(sox->effect->handler.flags & SOX_EFF_CHAN))
        sox->effect->out_signal.channels = sox->effect->in_signal.channels;
    if (!(sox->effect->handler.flags & SOX_EFF_RATE))
        sox->effect->out_signal.rate = sox->effect->in_signal.rate;

    if (sox->effect->handler.start(sox->effect) != SOX_SUCCESS) {
        av_log(ctx, AV_LOG_ERROR, "Could not start the sox effect\n");
        return AVERROR(EINVAL);
    }

    outlink->sample_rate = sox->effect->out_signal.rate;
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be

    return 0;
}

<<<<<<< HEAD
static int request_frame(AVFilterLink *outlink)
{
    SoxContext *sox = outlink->dst->priv;
    sox_effect_t *effect = sox->effect;
    size_t out_nb_samples = 1024;
    AVFilterBufferRef *outsamples;
    int ret;

    if ((ret = avfilter_request_frame(outlink->src->inputs[0]))){
        if (ret == AVERROR_EOF) {
            /* drain cached samples */
            while (1) {
                outsamples =
                    avfilter_get_audio_buffer(outlink, AV_PERM_WRITE, out_nb_samples);
                ret = effect->handler.drain(sox->effect,
                                            (int32_t *)outsamples->data[0], &out_nb_samples);
                outsamples->audio->nb_samples = out_nb_samples / effect->out_signal.channels;
                avfilter_filter_samples(outlink, outsamples);
                if (ret == SOX_EOF)
                    break;
            }
        }
    }

    return 0;
}

static void filter_samples(AVFilterLink *inlink, AVFilterBufferRef *insamples)
{
    SoxContext *sox = inlink->dst->priv;
    AVFilterBufferRef *outsamples;
    sox_effect_t *effect = sox->effect;
    size_t in_nb_samples, out_nb_samples;
    int ret;

    // FIXME not handling planar data
    in_nb_samples = insamples->audio->nb_samples;
    outsamples = avfilter_get_audio_buffer(inlink, AV_PERM_WRITE, in_nb_samples);

    out_nb_samples =
    in_nb_samples  = in_nb_samples * effect->out_signal.channels;

    //FIXME not handling cases where not all the input is consumed
    ret = effect->handler.flow(effect,
                               (int32_t *)insamples ->data[0],
                               (int32_t *)outsamples->data[0],
                               &in_nb_samples, &out_nb_samples);
    if (ret == SOX_EOF)
        ; /* use drain API */

    avfilter_copy_buffer_ref_props(outsamples, insamples);
    outsamples->audio->nb_samples = out_nb_samples / effect->out_signal.channels;
    avfilter_filter_samples(inlink->dst->outputs[0], outsamples);
=======
static void filter_samples(AVFilterLink *inlink, AVFilterBufferRef *insamples)
{
    SoxContext *sox = inlink->dst->priv;
    AVFilterBufferRef *outsamples;
    size_t nb_in_samples, nb_out_samples;

    // FIXME not handling planar data
    outsamples = ff_get_audio_buffer(inlink, AV_PERM_WRITE, insamples->audio->nb_samples);
    avfilter_copy_buffer_ref_props(outsamples, insamples);

    nb_out_samples = nb_in_samples =
        insamples->audio->nb_samples * sox->effect->in_signal.channels;

    // FIXME not handling cases where not all the input is consumed
    sox->effect->handler.flow(sox->effect, (int32_t *)insamples->data[0],
        (int32_t *)outsamples->data[0], &nb_in_samples, &nb_out_samples);

    outsamples->audio->nb_samples = nb_out_samples / sox->effect->out_signal.channels;
    ff_filter_samples(inlink->dst->outputs[0], outsamples);
}

static int request_frame(AVFilterLink *outlink)
{
    SoxContext *sox = outlink->dst->priv;
    sox_effect_t *effect = sox->effect;
    size_t out_nb_samples = 1024;
    AVFilterBufferRef *outsamples;
    int ret;

    ret = ff_request_frame(outlink->src->inputs[0]);
    if (ret == AVERROR_EOF) {
        /* drain cached samples */
        while (1) {
            outsamples =
                ff_get_audio_buffer(outlink, AV_PERM_WRITE, out_nb_samples);
            ret = effect->handler.drain(sox->effect,
                                        (int32_t *)outsamples->data[0], &out_nb_samples);
            outsamples->audio->nb_samples = out_nb_samples / effect->out_signal.channels;
            ff_filter_samples(outlink, outsamples);
            if (ret == SOX_EOF)
                break;
        }
    }

    return ret;
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
}

AVFilter avfilter_af_sox = {
    .name          = "sox",
<<<<<<< HEAD
    .description   = NULL_IF_CONFIG_SMALL("Apply SoX effects."),
=======
    .description   = NULL_IF_CONFIG_SMALL("Apply SoX library effect."),
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
    .priv_size     = sizeof(SoxContext),
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,
<<<<<<< HEAD
    .inputs    = (const AVFilterPad[]) {
        { .name             = "default",
          .type             = AVMEDIA_TYPE_AUDIO,
          .filter_samples   = filter_samples,
          .min_perms        = AV_PERM_READ },
        { .name = NULL}},
    .outputs   = (const AVFilterPad[]) {
        { .name             = "default",
          .type             = AVMEDIA_TYPE_AUDIO,
          .config_props     = config_output,
          .request_frame    = request_frame, },
        { .name = NULL}
    },
};

=======

    .inputs = (const AVFilterPad[]) {
        {
            .name             = "default",
            .type             = AVMEDIA_TYPE_AUDIO,
            .filter_samples   = filter_samples,
            .min_perms        = AV_PERM_READ,
        },
        { .name = NULL }
    },
    .outputs = (const AVFilterPad[]) {
        {
            .name             = "default",
            .type             = AVMEDIA_TYPE_AUDIO,
            .config_props     = config_output,
            .request_frame    = request_frame,
        },
        { .name = NULL }
    },
};
>>>>>>> 8913b0ae18a1f60f52666bb345f4f72aa542f1be
