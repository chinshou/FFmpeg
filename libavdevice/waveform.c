/*                                                                                                                                                   
 * Waveform Audio input                                                                                                                              
 * Copyright (c) 2010 Ramiro Polla                                                                                                                   
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
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"                                                                                                                                                     
#include "libavformat/avformat.h"
#include "libavutil/samplefmt.h"            
#include "libavformat/internal.h"                                                                                                        
#include <windows.h>                                                                                                                                 
                                                                                                                                                     
#define AUDIO_BLOCK_COUNT 32                                                                                                                         
#define AUDIO_BLOCK_SIZE  16384                                                                                                                      
                                                                                                                                                     
struct {                                                                                                                                             
    int wave_fmt;                                                                                                                                    
    int sample_rate;                                                                                                                                 
    int channels;                                                                                                                                    
    int bits_per_sample;                                                                                                                             
} wave_fmt_caps[] = {                                                                                                                                
    { WAVE_FORMAT_1M08, 11025, 1,  8 },                                                                                                              
    { WAVE_FORMAT_1M16, 11025, 1, 16 },                                                                                                              
    { WAVE_FORMAT_1S08, 11025, 2,  8 },                                                                                                              
    { WAVE_FORMAT_1S16, 11025, 2, 16 },                                                                                                              
    { WAVE_FORMAT_2M08, 22050, 1,  8 },                                                                                                              
    { WAVE_FORMAT_2M16, 22050, 1, 16 },                                                                                                              
    { WAVE_FORMAT_2S08, 22050, 2,  8 },                                                                                                              
    { WAVE_FORMAT_2S16, 22050, 2, 16 },                                                                                                              
    { WAVE_FORMAT_4M08, 44100, 1,  8 },                                                                                                              
    { WAVE_FORMAT_4M16, 44100, 1, 16 },                                                                                                              
    { WAVE_FORMAT_4S08, 44100, 2,  8 },                                                                                                              
    { WAVE_FORMAT_4S16, 44100, 2, 16 },                                                                                                              
#if 1 /* Not defined in MinGW yet. */                                                                                                                
    { WAVE_FORMAT_96M08, 96000, 1,  8 },                                                                                                             
    { WAVE_FORMAT_96M16, 96000, 1, 16 },                                                                                                             
    { WAVE_FORMAT_96S08, 96000, 2,  8 },                                                                                                             
    { WAVE_FORMAT_96S16, 96000, 2, 16 },                                                                                                             
#endif                                                                                                                                               
};                                                                                                                                                   
                                                                                                                                                     
struct waveform_ctx {     
    const AVClass *class;	
    HWAVEIN wi;                                                                                                                                      
    HANDLE mutex;                                                                                                                                    
    HANDLE event;                                                                                                                                    
    AVPacketList *pktl;                                                                                                                              
    WAVEHDR headers[AUDIO_BLOCK_COUNT];                                                                                                              
    WAVEHDR *buffer_ready[AUDIO_BLOCK_COUNT];                                                                                                        
    int32_t pts[AUDIO_BLOCK_COUNT];                                                                                                                  
    int needs_pts[AUDIO_BLOCK_COUNT];                                                                                                                
    int next_buffer_write;                                                                                                                           
    int next_buffer_read;                                                                                                                            
    int started;    
	int samplerate;
	int channels;
};                       

static AVFormatContext *cc = NULL;                                                                                                                              
                                                                                                                                                     
static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance,                                                                        
                                DWORD_PTR dwParam1, DWORD_PTR dwParam2)                                                                              
{                                                                                                                                                    
    AVFormatContext *s = cc;//(AVFormatContext *) dwInstance;                                                                                             
    struct waveform_ctx *ctx = s->priv_data;                                                                                                         
                                                                                                                                                     
    if (uMsg == WIM_DATA) {                                                                                                                          
        WaitForSingleObject(ctx->mutex, INFINITE);                                                                                                   
                                                                                                                                                     
        if (ctx->needs_pts[ctx->next_buffer_write]) {                                                                                                
            ctx->needs_pts[ctx->next_buffer_write] = 0;                                                                                              
            ctx->pts      [ctx->next_buffer_write] = GetTickCount();                                                                                 
        }                                                                                                                                            
                                                                                                                                                     
        ctx->buffer_ready[ctx->next_buffer_write++] = (WAVEHDR *) dwParam1;                                                                          
        if (ctx->next_buffer_write == AUDIO_BLOCK_COUNT)                                                                                             
            ctx->next_buffer_write -= AUDIO_BLOCK_COUNT;                                                                                             
                                                                                                                                                     
        if (ctx->buffer_ready[ctx->next_buffer_write]) {                                                                                             
            av_log(s, AV_LOG_ERROR, "All buffers are full! Data will be lost.\n");                                                                   
            ctx->needs_pts[ctx->next_buffer_write] = 1;                                                                                              
        }                                                                                                                                            
                                                                                                                                                     
        SetEvent(ctx->event);                                                                                                                        
        ReleaseMutex(ctx->mutex);                                                                                                                    
    } else if (uMsg == WIM_OPEN) {                                                                                                                   
        ctx->started = 1;                                                                                                                            
    }                                                                                                                                                
}                                                                                                                                                    
                                                                                                                                                     
static void free_buffers(AVFormatContext *s)                                                                                                         
{                                                                                                                                                    
    struct waveform_ctx *ctx = s->priv_data;                                                                                                         
    int i;                                                                                                                                           
                                                                                                                                                     
    for (i = 0; i < AUDIO_BLOCK_COUNT; i++)                                                                                                          
        av_freep(&ctx->headers[i].lpData);                                                                                                           
}                                                                                                                                                    
                                                                                                                                                     
static int bail_out(AVFormatContext *s, int result, const char *error_func)                                                                          
{                                                                                                                                                    
    char error_str[256] = { 0 };                                                                                                                     
                                                                                                                                                     
    free_buffers(s);                                                                                                                                 
                                                                                                                                                     
    waveInGetErrorText(result, error_str, sizeof(error_str));                                                                                        
    av_log(s, AV_LOG_ERROR, "%s error: %s\n", error_func, error_str);                                                                                
                                                                                                                                                     
    return AVERROR(EIO);                                                                                                                               
}                                                                                                                                                    
                                                                                                                                                     
static int waveform_read_close(AVFormatContext *s)                                                                                                   
{                                                                                                                                                    
    struct waveform_ctx *ctx = s->priv_data;                                                                                                         
    int result;                                                                                                                                      
                                                                                                                                                     
    if (ctx->started) {                                                                                                                              
        int i;                                                                                                                                       
                                                                                                                                                     
        result = waveInStop(ctx->wi);                                                                                                                
        if (result != MMSYSERR_NOERROR)                                                                                                              
            return bail_out(s, result, "waveInStop");                                                                                                
                                                                                                                                                     
        result = waveInReset(ctx->wi);                                                                                                               
        if (result != MMSYSERR_NOERROR)                                                                                                              
            return bail_out(s, result, "waveInReset");                                                                                               
                                                                                                                                                     
        for (i = 0; i < AUDIO_BLOCK_COUNT; i++)                                                                                                      
            if (ctx->headers[i].dwFlags & WHDR_PREPARED)                                                                                             
                waveInUnprepareHeader(ctx->wi, &ctx->headers[i], sizeof(ctx->headers[i]));                                                           
                                                                                                                                                     
        result = waveInClose(ctx->wi);                                                                                                               
        if (result != MMSYSERR_NOERROR)                                                                                                              
            return bail_out(s, result, "waveInClose");                                                                                               
    }                                                                                                                                                
                                                                                                                                                     
    free_buffers(s);                                                                                                                                 
                                                                                                                                                     
    return 0;                                                                                                                                        
}                                                                                                                                                    
                                                                                                                                                     
static enum AVCodecID bits_to_codec_id(int n)                                                                                                          
{                                                                                                                                                    
    switch (n) {                                                                                                                                     
    case  8: return AV_CODEC_ID_PCM_U8;                                                                                                                 
    case 16: return AV_CODEC_ID_PCM_S16LE;                                                                                                              
    case 32: return AV_CODEC_ID_PCM_S32LE;                                                                                                              
    default: return AV_CODEC_ID_NONE;                                                                                                                   
    }                                                                                                                                                
}                                                                  
                                                                                  
static enum AVSampleFormat bits_to_sample_fmt(int n)                                                                                                   
{                                                                                                                                                    
    switch (n) {                                                                                                                                     
    case  8: return AV_SAMPLE_FMT_U8;                                                                                                                   
    case 16: return AV_SAMPLE_FMT_S16;                                                                                                                  
    case 32: return AV_SAMPLE_FMT_S32;                                                                                                                  
    default: return AV_SAMPLE_FMT_NONE;                                                                                                                 
    }                                                                                                                                                
}                                                                                                                                                    
                                                                                                                                                     
static int waveform_read_header(AVFormatContext *s)                                                                          
{                                                                                                                                                    
    int try_caps = FF_ARRAY_ELEMS(wave_fmt_caps) -1;                                                                                                 
    struct waveform_ctx *ctx = s->priv_data;                                                                                                         
    int num_devs = waveInGetNumDevs();                                                                                                               
    WAVEFORMATEX fx;                                                                                                                                 
    WAVEINCAPS caps;                                                                                                                                 
    int device_id;                                                                                                                                   
    AVStream *st;                                                                                                                                    
    int result;                                                                                                                                      
    int i, j;                                                                                                                                        
                                                                                                                                                     
    if (!num_devs) {                                                                                                                                 
        av_log(s, AV_LOG_ERROR, "There are no usable audio devices.\n");                                                                             
        return AVERROR(EIO);                                                                                                                           
    }                                                                                                                                                
                                                                                                                                                     
    if (!strcmp(s->filename, "list")) {                                                                                                              
        for (i = -1; i < num_devs; i++) {                                                                                                            
            result = waveInGetDevCaps(i, &caps, sizeof(caps));                                                                                       
            if (result != MMSYSERR_NOERROR)                                                                                                          
                break;                                                                                                                               
            av_log(s, AV_LOG_INFO, "Audio device %d\n", i);                                                                                          
            av_log(s, AV_LOG_INFO, " wMid           %d\n", caps.wMid);                                                                               
            av_log(s, AV_LOG_INFO, " wPid           %d\n", caps.wPid);                                                                               
            av_log(s, AV_LOG_INFO, " vDriverVersion %d\n", caps.vDriverVersion);                                                                     
            av_log(s, AV_LOG_INFO, " szPname        %s\n", caps.szPname);                                                                            
            av_log(s, AV_LOG_INFO, " dwFormats      %x\n", (uint32_t) caps.dwFormats);                                                               
            av_log(s, AV_LOG_INFO, "  sample_rate channels bits_per_sample\n");                                                                      
            for (j = 0; j < FF_ARRAY_ELEMS(wave_fmt_caps); j++) {                                                                                    
                if (caps.dwFormats & wave_fmt_caps[j].wave_fmt)                                                                                      
                    av_log(s, AV_LOG_INFO, "  %11d %8d %15d\n",                                                                                      
                           wave_fmt_caps[j].sample_rate,                                                                                             
                           wave_fmt_caps[j].channels,                                                                                                
                           wave_fmt_caps[j].bits_per_sample);                                                                                        
            }                                                                                                                                        
            av_log(s, AV_LOG_INFO, " wChannels      %d\n", caps.wChannels);                                                                          
        }                                                                                                                                            
        return AVERROR(EIO);                                                                                                                           
    } else if (!strcmp(s->filename, "mapper"))                                                                                                       
        device_id = -1; /* WAVE_MAPPER */                                                                                                            
    else                                                                                                                                             
        device_id = atoi(s->filename);                                                                                                               
                                                                                                                                                     
    if (device_id < -1 || device_id >= num_devs) {                                                                                                   
        av_log(s, AV_LOG_ERROR, "Invalid device id %d\n", device_id);                                                                                
        return AVERROR(EIO);                                                                                                                           
    }                                                                                                                                                
                                                                                                                                                     
    result = waveInGetDevCaps(device_id, &caps, sizeof(caps));                                                                                       
    if (result != MMSYSERR_NOERROR) {                                                                                                                
        av_log(s, AV_LOG_ERROR, "Could not get information about device %d\n",                                                                       
               device_id);                                                                                                                           
        return AVERROR(EIO);                                                                                                                           
    }                                                                                                                                                
                                                                                                                                                     
    ctx->mutex = CreateMutex(NULL, 0, NULL);                                                                                                         
    if (!ctx->mutex) {                                                                                                                               
        av_log(s, AV_LOG_ERROR, "Could not create Mutex.\n");                                                                                        
        return AVERROR(EIO);                                                                                                                           
    }                                                                                                                                                
    ctx->event = CreateEvent(NULL, 1, 0, NULL);                                                                                                      
    if (!ctx->event) {                                                                                                                               
        av_log(s, AV_LOG_ERROR, "Could not create Event.\n");                                                                                        
        return AVERROR(EIO);                                                                                                                           
    }                                                                                                                                                
                                                                                                                                                     
    ctx->needs_pts[ctx->next_buffer_write] = 1;                                                                                                      
                                                                                                                                                     
    fx.wFormatTag      = WAVE_FORMAT_PCM;                                                                                                            
    fx.nChannels       = ctx->channels;//ap->channels;                                                                                                               
    fx.nSamplesPerSec  = ctx->samplerate;                                                                                                         
    fx.wBitsPerSample  = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) << 3;//av_get_bits_per_sample_fmt(AV_SAMPLE_FMT_S16);//(ap->sample_fmt);                                                                              
    fx.nBlockAlign     = fx.nChannels * (fx.wBitsPerSample >> 3);                                                                                    
    fx.nAvgBytesPerSec = fx.nSamplesPerSec * fx.nBlockAlign;                                                                                         
    fx.cbSize          = 0;                                                                                                                          
     av_log(s, AV_LOG_ERROR, "wave 4\n");     
    cc=s;                                                                                                                                                
    for (;;) {                                                                                                                                       
        result = waveInOpen(&ctx->wi, device_id, &fx, (DWORD) waveInProc,                                                                            
                            (DWORD) s, CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);                                                                      
        if (result == MMSYSERR_NOERROR)                                                                                                              
            break;                                                                                                                                   
        av_log(s, AV_LOG_DEBUG, "Could not open device with %d %d %d\n",                                                                             
               (unsigned int) fx.nSamplesPerSec, fx.wBitsPerSample, fx.nChannels);                                                                   
        while (!(wave_fmt_caps[try_caps].wave_fmt & caps.dwFormats) &&                                                                               
               --try_caps >= 0);                                                                                                                     
        if (try_caps < 0)                                                                                                                            
            return bail_out(s, result, "waveInOpen");                                                                                                
        fx.nChannels       = wave_fmt_caps[try_caps].channels;                                                                                       
        fx.nSamplesPerSec  = wave_fmt_caps[try_caps].sample_rate;                                                                                    
        fx.wBitsPerSample  = wave_fmt_caps[try_caps].bits_per_sample;                                                                                
        fx.nBlockAlign     = fx.nChannels * (fx.wBitsPerSample >> 3);                                                                                
        fx.nAvgBytesPerSec = fx.nSamplesPerSec * fx.nBlockAlign;                                                                                     
        try_caps--;                                                                                                                                  
        av_log(s, AV_LOG_DEBUG, "Trying with %d %d %d\n",                                                                                            
               (unsigned int) fx.nSamplesPerSec, fx.wBitsPerSample, fx.nChannels);                                                                   
    }                                                                                                                                                
                                                                                                                                                     
    st = avformat_new_stream(s, NULL);                                                                                                                        
    if (!st)                                                                                                                                         
        return AVERROR(ENOMEM);                                                                                                                      
    av_log( s, AV_LOG_ERROR, "new gdi stream %d", 
         s->nb_streams);                                                                                                                                                
    st->codecpar->codec_type  = AVMEDIA_TYPE_AUDIO;                                                                                                       
    st->codecpar->codec_id    = bits_to_codec_id(fx.wBitsPerSample);                                                                                    
    st->codecpar->sample_rate = fx.nSamplesPerSec;                                                                                                      
    st->codecpar->format  = bits_to_sample_fmt(fx.wBitsPerSample);                                                                                  
    st->codecpar->channels    = fx.nChannels;                                                                                                           
                                                                                                                                                     
    for (i = 0; i < AUDIO_BLOCK_COUNT; i++) {                                                                                                        
        ctx->headers[i].lpData         = av_malloc(AUDIO_BLOCK_SIZE);                                                                                
        ctx->headers[i].dwBufferLength = AUDIO_BLOCK_SIZE;                                                                                           
        ctx->headers[i].dwFlags        = 0;                                                                                                          
                                                                                                                                                     
        if (!ctx->headers[i].lpData) {                                                                                                               
            waveform_read_close(s);                                                                                                                  
            av_log(s, AV_LOG_ERROR, "Could not allocate buffers.\n");                                                                                
            return AVERROR(ENOMEM);                                                                                                                  
        }                                                                                                                                            
                                                                                                                                                     
        result = waveInPrepareHeader(ctx->wi, &ctx->headers[i], sizeof(ctx->headers[i]));                                                            
        if (result != MMSYSERR_NOERROR)                                                                                                              
            return bail_out(s, result, "waveInPrepareHeader");                                                                                       
        result = waveInAddBuffer    (ctx->wi, &ctx->headers[i], sizeof(ctx->headers[i]));                                                            
        if (result != MMSYSERR_NOERROR)                                                                                                              
            return bail_out(s, result, "waveInAddBuffer");                                                                                           
    }                                                                                                                                                
                                                                                                                                                     
    result = waveInStart(ctx->wi);                                                                                                                   
    if (result != MMSYSERR_NOERROR)                                                                                                                  
        return bail_out(s, result, "waveInStart");                                                                                                   
                                                                                                                                                     
    avpriv_set_pts_info(st, 32, 1, 1000);                                                                                                                
                                                                                                                                                     
    return 0;                                                                                                                                        
}                                                                                                                                                    
                                                                                                                                                     
static int waveform_read_packet(AVFormatContext *s, AVPacket *pkt)                                                                                   
{                                                                                                                                                    
    struct waveform_ctx *ctx = s->priv_data;                                                                                                         
    WAVEHDR *header = NULL;                                                                                                                          
                                                                                                                                                     
    while (!header) {                                                                                                                                
        WaitForSingleObject(ctx->mutex, INFINITE);                                                                                                   
        header = ctx->buffer_ready[ctx->next_buffer_read];                                                                                           
        ResetEvent(ctx->event);                                                                                                                      
        ReleaseMutex(ctx->mutex);                                                                                                                    
        if (!header) {                                                                                                                               
            if (s->flags & AVFMT_FLAG_NONBLOCK)                                                                                                      
                return AVERROR(EAGAIN);                                                                                                              
            WaitForSingleObject(ctx->event, INFINITE);                                                                                               
        }                                                                                                                                            
    }                                                                                                                                                
                                                                                                                                                     
    if (av_new_packet(pkt, header->dwBytesRecorded) < 0)                                                                                             
        return AVERROR(ENOMEM);                                                                                                                      
                                                                                                                                                     
    memcpy(pkt->data, header->lpData, header->dwBytesRecorded);                                                                                      
                                                                                                                                                     
    if (ctx->pts[ctx->next_buffer_read]) {                                                                                                           
        pkt->pts = ctx->pts[ctx->next_buffer_read];                                                                                                  
        ctx->pts[ctx->next_buffer_read] = 0;                                                                                                         
    }                                                                                                                                                
                                                                                                                                                     
    ctx->buffer_ready[ctx->next_buffer_read++] = NULL;                                                                                               
    if (ctx->next_buffer_read == AUDIO_BLOCK_COUNT)                                                                                                  
        ctx->next_buffer_read -= AUDIO_BLOCK_COUNT;                                                                                                  
                                                                                                                                                     
    /* Give buffer back to Waveform. */                                                                                                              
    waveInAddBuffer(ctx->wi, header, sizeof(*header));                                                                                               
                                                                                                                                                     
    return pkt->size;                                                                                                                                
}            

#define OFFSET(x) offsetof(struct waveform_ctx, x)
#define DEC AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { "samplerate", "A string describing samplerate.", OFFSET(samplerate), AV_OPT_TYPE_INT, {.i64 = 44100}, 8000, 96000, DEC },
    { "channels",   "A string describing samplerate.", OFFSET(channels), AV_OPT_TYPE_INT, {.i64 = 2}, 1, 2, DEC },
	{ NULL },
};
                                                                                                                                                     
AVInputFormat ff_waveform_demuxer = {                                                                                                                   
    .name ="waveform",                                                                                                                                      
    .long_name=NULL_IF_CONFIG_SMALL("Waveform Audio"),                                                                                                          
    .priv_data_size =sizeof(struct waveform_ctx),                                                                                                                     
    .read_header =waveform_read_header,                                                                                                                            
    .read_packet =waveform_read_packet,                                                                                                                            
    .read_close =waveform_read_close,                                                                                                                             
    .flags = AVFMT_NOFILE,                                                                                                                           
};                                                                                                                                                   

