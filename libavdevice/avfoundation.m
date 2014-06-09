/*
 * AVFoundation input device
 * Copyright (c) 2014 Thilo Borgmann <thilo.borgmann@mail.de>
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
 * AVFoundation input device
 * @author Thilo Borgmann <thilo.borgmann@mail.de>
 */

#import <AVFoundation/AVFoundation.h>
#include <pthread.h>

#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
#include "libavformat/internal.h"
#include "libavutil/internal.h"
#include "libavutil/time.h"
#include "avdevice.h"

static const int avf_time_base = 1000000;//100

static const AVRational avf_time_base_q = {
    .num = 1,
    .den = avf_time_base
};

typedef struct
{
    AVClass*        class;

    float           frame_rate;
    int             frames_captured;
    int             audio_frames_captured;
    int64_t         first_pts;
    int64_t         first_audio_pts;
    pthread_mutex_t frame_lock;
    pthread_cond_t  frame_wait_cond;
    id              avf_delegate;
    id              avf_audio_delegate;

    int             list_devices;
    int             video_device_index;
    int             video_stream_index;
    int             audio_device_index;
    int             audio_stream_index;

    char            *video_filename;
    char            *audio_filename;

    int             audio_channels;
    int             audio_bits_per_sample;
    int             audio_float;
    int             audio_be;
    int             audio_signed_integer;
    int             audio_packed;
    int             audio_non_interleaved;

    int32_t         *audio_buffer;
    int             audio_buffer_size;

    AVCaptureSession         *capture_session;
    AVCaptureVideoDataOutput *video_output;
    AVCaptureAudioDataOutput *audio_output;
    CMSampleBufferRef         current_frame;
    CMSampleBufferRef         current_audio_frame;
} AVFContext;

static void lock_frames(AVFContext* ctx)
{
    pthread_mutex_lock(&ctx->frame_lock);
}

static void unlock_frames(AVFContext* ctx)
{
    pthread_mutex_unlock(&ctx->frame_lock);
}

/** FrameReciever class - delegate for AVCaptureSession
 */
@interface AVFFrameReceiver : NSObject
{
    AVFContext* _context;
}

- (id)initWithContext:(AVFContext*)context;

- (void)  captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)videoFrame
         fromConnection:(AVCaptureConnection *)connection;

@end

@implementation AVFFrameReceiver

- (id)initWithContext:(AVFContext*)context
{
    if (self = [super init]) {
        _context = context;
    }
    return self;
}

- (void)  captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)videoFrame
         fromConnection:(AVCaptureConnection *)connection
{
    lock_frames(_context);

    if (_context->current_frame != nil) {
        CFRelease(_context->current_frame);
    }

    _context->current_frame = (CMSampleBufferRef)CFRetain(videoFrame);

    pthread_cond_signal(&_context->frame_wait_cond);

    unlock_frames(_context);

    ++_context->frames_captured;
}

@end

/** AudioReciever class - delegate for AVCaptureSession
 */
@interface AVFAudioReceiver : NSObject
{
    AVFContext* _context;
}

- (id)initWithContext:(AVFContext*)context;

- (void)  captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)audioFrame
         fromConnection:(AVCaptureConnection *)connection;

@end

@implementation AVFAudioReceiver

- (id)initWithContext:(AVFContext*)context
{
    if (self = [super init]) {
        _context = context;
    }
    return self;
}

- (void)  captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)audioFrame
         fromConnection:(AVCaptureConnection *)connection
{
    lock_frames(_context);

    if (_context->current_audio_frame != nil) {
        CFRelease(_context->current_audio_frame);
    }

    _context->current_audio_frame = (CMSampleBufferRef)CFRetain(audioFrame);

    pthread_cond_signal(&_context->frame_wait_cond);

    unlock_frames(_context);

    ++_context->audio_frames_captured;
}

@end

static void destroy_context(AVFContext* ctx)
{
    [ctx->capture_session stopRunning];

    [ctx->capture_session release];
    [ctx->video_output    release];
    [ctx->audio_output    release];
    [ctx->avf_delegate    release];
    [ctx->avf_audio_delegate release];

    ctx->capture_session = NULL;
    ctx->video_output    = NULL;
    ctx->avf_delegate    = NULL;
    ctx->avf_audio_delegate = NULL;

    av_freep(&ctx->audio_buffer);

    pthread_mutex_destroy(&ctx->frame_lock);
    pthread_cond_destroy(&ctx->frame_wait_cond);

    if (ctx->current_frame) {
        CFRelease(ctx->current_frame);
    }
}

static void parse_device_name(AVFormatContext *s)
{
    AVFContext *ctx = (AVFContext*)s->priv_data;
    char *tmp = av_strdup(s->filename);

    if (tmp[0] != ':') {
        ctx->video_filename = strtok(tmp,  ":");
        ctx->audio_filename = strtok(NULL, ":");
    } else {
        ctx->audio_filename = strtok(tmp,  ":");
    }
}

static int add_video_device(AVFormatContext *s, AVCaptureDevice *video_device)
{
    AVFContext *ctx = (AVFContext*)s->priv_data;
    NSError *error  = nil;
    AVCaptureDeviceInput* capture_dev_input = [[[AVCaptureDeviceInput alloc] initWithDevice:video_device error:&error] autorelease];

    if (!capture_dev_input) {
        av_log(s, AV_LOG_ERROR, "Failed to create AV capture input device: %s\n",
               [[error localizedDescription] UTF8String]);
        return 1;
    }

    if ([ctx->capture_session canAddInput:capture_dev_input]) {
        [ctx->capture_session addInput:capture_dev_input];
    } else {
        av_log(s, AV_LOG_ERROR, "can't add video input to capture session\n");
        return 1;
    }

    // Attaching output
    // FIXME: Allow for a user defined pixel format
    ctx->video_output = [[AVCaptureVideoDataOutput alloc] init];

    if (!ctx->video_output) {
        av_log(s, AV_LOG_ERROR, "Failed to init AV video output\n");
        return 1;
    }

    NSNumber     *pixel_format = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32RGB];
    NSDictionary *capture_dict = [NSDictionary dictionaryWithObject:pixel_format
                                               forKey:(id)kCVPixelBufferPixelFormatTypeKey];

    [ctx->video_output setVideoSettings:capture_dict];
    [ctx->video_output setAlwaysDiscardsLateVideoFrames:YES];

    ctx->avf_delegate = [[AVFFrameReceiver alloc] initWithContext:ctx];

    dispatch_queue_t queue = dispatch_queue_create("avf_queue", NULL);
    [ctx->video_output setSampleBufferDelegate:ctx->avf_delegate queue:queue];
    dispatch_release(queue);

    if ([ctx->capture_session canAddOutput:ctx->video_output]) {
        [ctx->capture_session addOutput:ctx->video_output];
    } else {
        av_log(s, AV_LOG_ERROR, "adding video output to capture session failed\n");
        return 1;
    }

    return 0;
}

static int add_audio_device(AVFormatContext *s, AVCaptureDevice *audio_device)
{
    AVFContext *ctx = (AVFContext*)s->priv_data;
    NSError *error  = nil;
    AVCaptureDeviceInput* audio_dev_input = [[[AVCaptureDeviceInput alloc] initWithDevice:audio_device error:&error] autorelease];

    if (!audio_dev_input) {
        av_log(s, AV_LOG_ERROR, "Failed to create AV capture input device: %s\n",
               [[error localizedDescription] UTF8String]);
        return 1;
    }

    if ([ctx->capture_session canAddInput:audio_dev_input]) {
        [ctx->capture_session addInput:audio_dev_input];
    } else {
        av_log(s, AV_LOG_ERROR, "can't add audio input to capture session\n");
        return 1;
    }

    // Attaching output
    ctx->audio_output = [[AVCaptureAudioDataOutput alloc] init];

    if (!ctx->audio_output) {
        av_log(s, AV_LOG_ERROR, "Failed to init AV audio output\n");
        return 1;
    }

    ctx->avf_audio_delegate = [[AVFAudioReceiver alloc] initWithContext:ctx];

    dispatch_queue_t queue = dispatch_queue_create("avf_audio_queue", NULL);
    [ctx->audio_output setSampleBufferDelegate:ctx->avf_audio_delegate queue:queue];
    dispatch_release(queue);

    if ([ctx->capture_session canAddOutput:ctx->audio_output]) {
        [ctx->capture_session addOutput:ctx->audio_output];
    } else {
        av_log(s, AV_LOG_ERROR, "adding audio output to capture session failed\n");
        return 1;
    }

    return 0;
}


static int get_video_config(AVFormatContext *s)
{
    AVFContext *ctx = (AVFContext*)s->priv_data;

    // Take stream info from the first frame.
    while (ctx->frames_captured < 1) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, YES);
    }

    lock_frames(ctx);

    AVStream* stream = avformat_new_stream(s, NULL);

    if (!stream) {
        return 1;
    }

    ctx->video_stream_index = stream->index;

    avpriv_set_pts_info(stream, 64, 1, avf_time_base);

    CVImageBufferRef image_buffer = CMSampleBufferGetImageBuffer(ctx->current_frame);
    CGSize image_buffer_size      = CVImageBufferGetEncodedSize(image_buffer);

    stream->codec->codec_id   = AV_CODEC_ID_RAWVIDEO;
    stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codec->width      = (int)image_buffer_size.width;
    stream->codec->height     = (int)image_buffer_size.height;
    stream->codec->pix_fmt    = AV_PIX_FMT_RGB32;

    CFRelease(ctx->current_frame);
    ctx->current_frame = nil;

    unlock_frames(ctx);

    return 0;
}

static int get_audio_config(AVFormatContext *s)
{
    AVFContext *ctx = (AVFContext*)s->priv_data;

    // Take stream info from the first frame.
    while (ctx->audio_frames_captured < 1) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, YES);
    }

    lock_frames(ctx);

    AVStream* stream = avformat_new_stream(s, NULL);

    if (!stream) {
        return 1;
    }

    ctx->audio_stream_index = stream->index;

    avpriv_set_pts_info(stream, 64, 1, avf_time_base);

    CMFormatDescriptionRef format_desc = CMSampleBufferGetFormatDescription(ctx->current_audio_frame);
    const AudioStreamBasicDescription *basic_desc = CMAudioFormatDescriptionGetStreamBasicDescription(format_desc);

    if (!basic_desc) {
        av_log(s, AV_LOG_ERROR, "audio format not available\n");
        return 1;
    }

    stream->codec->codec_type     = AVMEDIA_TYPE_AUDIO;
    stream->codec->sample_rate    = basic_desc->mSampleRate;
    stream->codec->channels       = basic_desc->mChannelsPerFrame;
    stream->codec->channel_layout = av_get_default_channel_layout(stream->codec->channels);

    ctx->audio_channels        = basic_desc->mChannelsPerFrame;
    ctx->audio_bits_per_sample = basic_desc->mBitsPerChannel;
    ctx->audio_float           = basic_desc->mFormatFlags & kAudioFormatFlagIsFloat;
    ctx->audio_be              = basic_desc->mFormatFlags & kAudioFormatFlagIsBigEndian;
    ctx->audio_signed_integer  = basic_desc->mFormatFlags & kAudioFormatFlagIsSignedInteger;
    ctx->audio_packed          = basic_desc->mFormatFlags & kAudioFormatFlagIsPacked;
    ctx->audio_non_interleaved = basic_desc->mFormatFlags & kAudioFormatFlagIsNonInterleaved;

    if (basic_desc->mFormatID == kAudioFormatLinearPCM &&
        ctx->audio_float &&
        ctx->audio_packed) {
        stream->codec->codec_id = ctx->audio_be ? AV_CODEC_ID_PCM_F32BE : AV_CODEC_ID_PCM_F32LE;
    } else {
        av_log(s, AV_LOG_ERROR, "audio format is not supported\n");
        return 1;
    }

    if (ctx->audio_non_interleaved) {
        CMBlockBufferRef block_buffer = CMSampleBufferGetDataBuffer(ctx->current_audio_frame);
        ctx->audio_buffer_size        = CMBlockBufferGetDataLength(block_buffer);
        ctx->audio_buffer             = av_malloc(ctx->audio_buffer_size);
        if (!ctx->audio_buffer) {
            av_log(s, AV_LOG_ERROR, "error allocating audio buffer\n");
            return 1;
        }
    }

    CFRelease(ctx->current_audio_frame);
    ctx->current_audio_frame = nil;

    unlock_frames(ctx);

    return 0;
}

static int avf_read_header(AVFormatContext *s)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    AVFContext *ctx         = (AVFContext*)s->priv_data;
    ctx->first_pts          = av_gettime();
    ctx->first_audio_pts    = av_gettime();

    pthread_mutex_init(&ctx->frame_lock, NULL);
    pthread_cond_init(&ctx->frame_wait_cond, NULL);

    // List devices if requested
    if (ctx->list_devices) {
        av_log(ctx, AV_LOG_INFO, "AVFoundation video devices:\n");
        NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
        for (AVCaptureDevice *device in devices) {
            const char *name = [[device localizedName] UTF8String];
            int index  = [devices indexOfObject:device];
            av_log(ctx, AV_LOG_INFO, "[%d] %s\n", index, name);
        }
        av_log(ctx, AV_LOG_INFO, "AVFoundation audio devices:\n");
        devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];
        for (AVCaptureDevice *device in devices) {
            const char *name = [[device localizedName] UTF8String];
            int index  = [devices indexOfObject:device];
            av_log(ctx, AV_LOG_INFO, "[%d] %s\n", index, name);
        }
        goto fail;
    }

    // Find capture device
    AVCaptureDevice *video_device = nil;
    AVCaptureDevice *audio_device = nil;

    // parse input filename for video and audio device
    parse_device_name(s);

    // check for device index given in filename
    if (ctx->video_device_index == -1 && ctx->video_filename) {
        sscanf(ctx->video_filename, "%d", &ctx->video_device_index);
    }
    if (ctx->audio_device_index == -1 && ctx->audio_filename) {
        sscanf(ctx->audio_filename, "%d", &ctx->audio_device_index);
    }

    if (ctx->video_device_index >= 0) {
        NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];

        if (ctx->video_device_index >= [devices count]) {
            av_log(ctx, AV_LOG_ERROR, "Invalid device index\n");
            goto fail;
        }

        video_device = [devices objectAtIndex:ctx->video_device_index];
    } else if (ctx->video_filename &&
               strncmp(ctx->video_filename, "default", 7)) {
        NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];

        for (AVCaptureDevice *device in devices) {
            if (!strncmp(ctx->video_filename, [[device localizedName] UTF8String], strlen(ctx->video_filename))) {
                video_device = device;
                break;
            }
        }

        if (!video_device) {
            av_log(ctx, AV_LOG_ERROR, "Video device not found\n");
            goto fail;
        }
    } else {
        video_device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }



    // get audio device
    if (ctx->audio_device_index >= 0) {
        NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];

        if (ctx->audio_device_index >= [devices count]) {
            av_log(ctx, AV_LOG_ERROR, "Invalid audio device index\n");
            goto fail;
        }

        audio_device = [devices objectAtIndex:ctx->audio_device_index];
    } else if (ctx->audio_filename &&
               strncmp(ctx->audio_filename, "default", 7)) {
        NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];

        for (AVCaptureDevice *device in devices) {
            if (!strncmp(ctx->audio_filename, [[device localizedName] UTF8String], strlen(ctx->audio_filename))) {
                audio_device = device;
                break;
            }
        }

        if (!audio_device) {
            av_log(ctx, AV_LOG_ERROR, "Audio device not found\n");
            goto fail;
        }
    } else {
        audio_device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];
    }


    // Video nor Audio capture device not found, looking for AVMediaTypeVideo/Audio
    if (!video_device && !audio_device) {
        av_log(s, AV_LOG_ERROR, "No AV capture device found\n");
        goto fail;
    }

    if (video_device) {
        av_log(s, AV_LOG_DEBUG, "'%s' opened\n", [[video_device localizedName] UTF8String]);
    }
    if (audio_device) {
        av_log(s, AV_LOG_DEBUG, "audio device '%s' opened\n", [[audio_device localizedName] UTF8String]);
    }

    // Initialize capture session
    ctx->capture_session = [[AVCaptureSession alloc] init];

    if (video_device && add_video_device(s, video_device)) {
        goto fail;
    }
    if (audio_device && add_audio_device(s, audio_device)) {
        goto fail;
    }

    [ctx->capture_session startRunning];

    if (video_device && get_video_config(s)) {
        goto fail;
    }

    // set audio stream
    if (audio_device && get_audio_config(s)) {
        goto fail;
    }

    [pool release];
    return 0;

fail:
    [pool release];
    destroy_context(ctx);
    return AVERROR(EIO);
}

static int avf_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    AVFContext* ctx = (AVFContext*)s->priv_data;

    do {
        lock_frames(ctx);

        CVImageBufferRef image_buffer = CMSampleBufferGetImageBuffer(ctx->current_frame);

        if (ctx->current_frame != nil) {
            if (av_new_packet(pkt, (int)CVPixelBufferGetDataSize(image_buffer)) < 0) {
                return AVERROR(EIO);
            }

            pkt->pts = pkt->dts = av_rescale_q(av_gettime() - ctx->first_pts,
                                               AV_TIME_BASE_Q,
                                               avf_time_base_q);
            pkt->stream_index  = ctx->video_stream_index;
            pkt->flags        |= AV_PKT_FLAG_KEY;

            CVPixelBufferLockBaseAddress(image_buffer, 0);

            void* data = CVPixelBufferGetBaseAddress(image_buffer);
            memcpy(pkt->data, data, pkt->size);

            CVPixelBufferUnlockBaseAddress(image_buffer, 0);
            CFRelease(ctx->current_frame);
            ctx->current_frame = nil;
        } else if (ctx->current_audio_frame != nil) {
            CMBlockBufferRef block_buffer = CMSampleBufferGetDataBuffer(ctx->current_audio_frame);
            int block_buffer_size         = CMBlockBufferGetDataLength(block_buffer);

            if (!block_buffer || !block_buffer_size) {
                return AVERROR(EIO);
            }

            if (ctx->audio_non_interleaved && block_buffer_size > ctx->audio_buffer_size) {
                return AVERROR_BUFFER_TOO_SMALL;
            }

            if (av_new_packet(pkt, block_buffer_size) < 0) {
                return AVERROR(EIO);
            }

            pkt->pts = pkt->dts = av_rescale_q(av_gettime() - ctx->first_audio_pts,
                                               AV_TIME_BASE_Q,
                                               avf_time_base_q);

            pkt->stream_index  = ctx->audio_stream_index;
            pkt->flags        |= AV_PKT_FLAG_KEY;

            if (ctx->audio_non_interleaved) {
                int sample, c, shift;

                OSStatus ret = CMBlockBufferCopyDataBytes(block_buffer, 0, pkt->size, ctx->audio_buffer);
                if (ret != kCMBlockBufferNoErr) {
                    return AVERROR(EIO);
                }

                int num_samples = pkt->size / (ctx->audio_channels * (ctx->audio_bits_per_sample >> 3));

                // transform decoded frame into output format
                #define INTERLEAVE_OUTPUT(bps)                                         \
                {                                                                      \
                    int##bps##_t **src;                                                \
                    int##bps##_t *dest;                                                \
                    src = av_malloc(ctx->audio_channels * sizeof(int##bps##_t*));      \
                    if (!src) return AVERROR(EIO);                                     \
                    for (c = 0; c < ctx->audio_channels; c++) {                        \
                        src[c] = ((int##bps##_t*)ctx->audio_buffer) + c * num_samples; \
                    }                                                                  \
                    dest  = (int##bps##_t*)pkt->data;                                  \
                    shift = bps - ctx->audio_bits_per_sample;                          \
                    for (sample = 0; sample < num_samples; sample++)                   \
                        for (c = 0; c < ctx->audio_channels; c++)                      \
                            *dest++ = src[c][sample] << shift;                         \
                    av_freep(&src);                                                    \
                }

                if (ctx->audio_bits_per_sample <= 16) {
                    INTERLEAVE_OUTPUT(16)
                } else {
                    INTERLEAVE_OUTPUT(32)
                }
            } else {
                OSStatus ret = CMBlockBufferCopyDataBytes(block_buffer, 0, pkt->size, pkt->data);
                if (ret != kCMBlockBufferNoErr) {
                    return AVERROR(EIO);
                }
            }

            CFRelease(ctx->current_audio_frame);
            ctx->current_audio_frame = nil;
        } else {
            pkt->data = NULL;
            pthread_cond_wait(&ctx->frame_wait_cond, &ctx->frame_lock);
        }

        unlock_frames(ctx);
    } while (!pkt->data);

    return 0;
}

static int avf_close(AVFormatContext *s)
{
    AVFContext* ctx = (AVFContext*)s->priv_data;
    destroy_context(ctx);
    return 0;
}

static const AVOption options[] = {
    { "frame_rate", "set frame rate", offsetof(AVFContext, frame_rate), AV_OPT_TYPE_FLOAT, { .dbl = 30.0 }, 0.1, 30.0, AV_OPT_TYPE_VIDEO_RATE, NULL },
    { "list_devices", "list available devices", offsetof(AVFContext, list_devices), AV_OPT_TYPE_INT, {.i64=0}, 0, 1, AV_OPT_FLAG_DECODING_PARAM, "list_devices" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, AV_OPT_FLAG_DECODING_PARAM, "list_devices" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, AV_OPT_FLAG_DECODING_PARAM, "list_devices" },
    { "video_device_index", "select video device by index for devices with same name (starts at 0)", offsetof(AVFContext, video_device_index), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { "audio_device_index", "select audio device by index for devices with same name (starts at 0)", offsetof(AVFContext, audio_device_index), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass avf_class = {
    .class_name = "AVFoundation input device",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_avfoundation_demuxer = {
    .name           = "avfoundation",
    .long_name      = NULL_IF_CONFIG_SMALL("AVFoundation input device"),
    .priv_data_size = sizeof(AVFContext),
    .read_header    = avf_read_header,
    .read_packet    = avf_read_packet,
    .read_close     = avf_close,
    .flags          = AVFMT_NOFILE,
    .priv_class     = &avf_class,
};
