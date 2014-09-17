/*
 * Copyright (C) 2013 Xiaolei Yu <dreifachstein@gmail.com>
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

#include "config.h"
#include "libswscale/swscale.h"
#include "libswscale/swscale_internal.h"
#include "libavutil/arm/cpu.h"

extern void rgbx_to_nv12_neon_32(const uint8_t *src, uint8_t *y, uint8_t *chroma,
                int width, int height,
                int y_stride, int c_stride, int src_stride,
                int32_t coeff_tbl[9]);

extern void rgbx_to_nv12_neon_16(const uint8_t *src, uint8_t *y, uint8_t *chroma,
                int width, int height,
                int y_stride, int c_stride, int src_stride,
                int32_t coeff_tbl[9]);
extern const uint32_t yuv2rgb565_table[];
struct yuv_pack
{
    uint8_t *yuv;
    size_t pitch;
};

struct yuv_planes
{
    uint8_t *y, *u, *v;
    size_t pitch;
};

extern void i420_rgb_neon (struct yuv_pack *const out,
                    const struct yuv_planes *const in,
                    int width, int height);// asm("i420_rgb_neon");
extern void nv12_rgb_neon (struct yuv_pack *const out,
                    const struct yuv_planes *const in,
                    int width, int height);// asm("nv12_rgb_neon");
static int CoefY[256];
static int CoefRV[256];
static int CoefGU[256];
static int CoefGV[256];
static int CoefBU[256];
#   define unlikely(p) __builtin_expect(!!(p), 0)

static void I420_RGBA_C (struct yuv_pack* pOut, struct yuv_planes* pIn, size_t width, size_t height)
{
    const uint8_t *const out = pOut->yuv;

    const int ypitch = pIn->pitch;
    const int uvpitch = pIn->pitch >> 1 ;
    const int dpitch = pOut->pitch / 4;//dst->p->i_pixel_pitch;

    for (size_t j = 0; j <  height; ++j)
    {
        const int y = j * ypitch;
        const int u = (j>>1) * uvpitch;
        const int d = j * dpitch;

        for (size_t i = 0; i < width; ++i)
        {
            uint8_t Y = pIn->y[y + i];
            uint8_t U = pIn->u[u + (i>>1)];
            uint8_t V = pIn->v[u + (i>>1)];

            //coef = float * Precision + .5 (Precision=32768)
            int R = CoefY[Y] + CoefRV[V];
            int G = CoefY[Y] + CoefGU[U] + CoefGV[V];
            int B = CoefY[Y] + CoefBU[U];

            //rgb = (rgb+Precision/2) / Precision (Precision=32768)
            R = R >> 15;
            G = G >> 15;
            B = B >> 15;

            if (unlikely(R < 0)) R = 0;
            if (unlikely(G < 0)) G = 0;
            if (unlikely(B < 0)) B = 0;
            if (unlikely(R > 255)) R = 255;
            if (unlikely(G > 255)) G = 255;
            if (unlikely(B > 255)) B = 255;

            ((uint32_t*)out)[d + i] = R | (G<<8) | (B<<16) | (0xff<<24);
        }
    }
}

static int nv12_to_rgb32_neon_wrapper(SwsContext *context, const uint8_t *src[],
                        int srcStride[], int srcSliceY, int srcSliceH,
                        uint8_t *dst[], int dstStride[]) {
	struct yuv_pack out = { dst[0]+srcSliceY * dstStride[0], dstStride[0] };
    struct yuv_planes in = { src[0]+srcSliceY * srcStride[0],  
		src[1]+(srcSliceY/2) * srcStride[1], 
		src[2]+(srcSliceY/2) * srcStride[2],
		srcStride[0] };
    nv12_rgb_neon (&out, &in, context->srcW, srcSliceH);
	return 0;
}

static int yuv420p_to_rgb32_neon_wrapper(SwsContext *context, const uint8_t *src[],
                        int srcStride[], int srcSliceY, int srcSliceH,
                        uint8_t *dst[], int dstStride[]) {
	struct yuv_pack out = { dst[0]+srcSliceY * dstStride[0], dstStride[0] };
	struct yuv_planes in = { src[0]+srcSliceY * srcStride[0],  
		src[1]+(srcSliceY/2) * srcStride[1], 
		src[2]+(srcSliceY/2) * srcStride[2],
		srcStride[0] };
	i420_rgb_neon (&out, &in, context->srcW, srcSliceH);

    //I420_RGBA_C(&out, &in, context->srcW, srcSliceH);
    return 0;
}

static int rgbx_to_nv12_neon_32_wrapper(SwsContext *context, const uint8_t *src[],
                        int srcStride[], int srcSliceY, int srcSliceH,
                        uint8_t *dst[], int dstStride[]) {

    rgbx_to_nv12_neon_32(src[0] + srcSliceY * srcStride[0],
            dst[0] + srcSliceY * dstStride[0],
            dst[1] + (srcSliceY / 2) * dstStride[1],
            context->srcW, srcSliceH,
            dstStride[0], dstStride[1], srcStride[0],
            context->input_rgb2yuv_table);

    return 0;
}

static int rgbx_to_nv12_neon_16_wrapper(SwsContext *context, const uint8_t *src[],
                        int srcStride[], int srcSliceY, int srcSliceH,
                        uint8_t *dst[], int dstStride[]) {

    rgbx_to_nv12_neon_16(src[0] + srcSliceY * srcStride[0],
            dst[0] + srcSliceY * dstStride[0],
            dst[1] + (srcSliceY / 2) * dstStride[1],
            context->srcW, srcSliceH,
            dstStride[0], dstStride[1], srcStride[0],
            context->input_rgb2yuv_table);

    return 0;
}

static void get_unscaled_swscale_neon(SwsContext *c) {
    int accurate_rnd = c->flags & SWS_ACCURATE_RND;
    if (c->srcFormat == AV_PIX_FMT_RGBA
            && c->dstFormat == AV_PIX_FMT_NV12
            && (c->srcW >= 16)) {
        c->swscale = accurate_rnd ? rgbx_to_nv12_neon_32_wrapper
                        : rgbx_to_nv12_neon_16_wrapper;
    }
	else if (c->srcFormat == AV_PIX_FMT_YUV420P
			&& c->dstFormat == AV_PIX_FMT_RGBA)
	{
		c->swscale =yuv420p_to_rgb32_neon_wrapper;
	}
	else if (c->srcFormat == AV_PIX_FMT_NV12
			&& c->dstFormat == AV_PIX_FMT_RGBA)
	{
		//c->swscale =nv12_to_rgb32_neon_wrapper;
	}
}

void ff_get_unscaled_swscale_arm(SwsContext *c)
{
    int cpu_flags = av_get_cpu_flags();
    if (have_neon(cpu_flags))
        get_unscaled_swscale_neon(c);
    //precompute some values for the C version
    const int coefY  = (int)(1.164 * 32768 + 0.5);
    const int coefRV = (int)(1.793 * 32768 + 0.5);
    const int coefGU = (int)(0.213 * 32768 + 0.5);
    const int coefGV = (int)(0.533 * 32768 + 0.5);
    const int coefBU = (int)(2.113 * 32768 + 0.5);
    for (int i=0; i<256; ++i)
    {
        CoefY[i] = coefY * (i-16) + 16384;
        CoefRV[i] = coefRV*(i-128);
        CoefGU[i] = -coefGU*(i-128);
        CoefGV[i] = -coefGV*(i-128);
        CoefBU[i] = coefBU*(i-128);
    }
}
