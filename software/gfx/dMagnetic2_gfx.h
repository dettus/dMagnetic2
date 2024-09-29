//
// BSD 2-Clause License
//
// Copyright (c) 2024, dettus@dettus.net
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	DMAGNETIC2_GFX_H
#define	DMAGNETIC2_GFX_H


#define	DMAGNETIC2_GFX_MAX_COLORS	16
#define	DMAGNETIC2_GFX_MAX_WIDTH	640
#define	DMAGNETIC2_GFX_MAX_HEIGHT	480

// The RGB values are 10 bit wide. 
// red = 29..20
// green=19..10
// blue=  9.. 0

typedef struct _tdMagnetic2_canvas_small
{
	int width;	// width in pixels
	int height;	// height in pixels
	unsigned int rgb[DMAGNETIC2_GFX_MAX_COLORS];	// RGB values. 10 Bit per channel.
	unsigned char pixels[DMAGNETIC2_GFX_MAX_WIDTH*DMAGNETIC2_GFX_MAX_HEIGHT];
} tdMagnetic2_canvas_small;

typedef struct _tdMagnetic2_canvas_large
{
	int width;	// width in pixels
	int height;	// height in pixels
	unsigned int rgbpixels[DMAGNETIC2_GFX_MAX_WIDTH*DMAGNETIC2_GFX_MAX_HEIGHT];	// RGB values. 10 Bit per pixel
} tdMagnetic2_canvas_large;

#define	DMAGNETIC2_GFX_TYPE_NONE	0
#define	DMAGNETIC2_GFX_TYPE_PICTURE	1
#define	DMAGNETIC2_GFX_TYPE_ANIMATION	2
#define	DMAGNETIC2_GFX_TYPE_C64		3
#define	DMAGNETIC2_GFX_TYPE_HALFTONE	4
#define	DMAGNETIC2_GFX_TYPE_VGA		5

// API functions for initialization
int dMagnetic2_gfx_get_size(int *pBytes);
int dMagnetic2_gfx_init(void *pHandle);
int dMagnetic2_gfx_set_gfx(void *pHandle,int size,unsigned char* pGfx);

// API functions for drawing
int dMagnetic2_gfx_set_current_picture(void *pHandle,char* pPicname,int picNum);
int dMagnetic2_gfx_get_current_type(void* pHandle,
int dMagnetic2_gfx_draw_picture(void* pHandle,tdMagnetic2_canvas_small *pCanvas,tdMagnetic2_canvas_large *pCanvas);
int dMagnetic2_gfx_draw_animation(void* pHandle,tdMagnetic2_canvas_small *pCanvas,tdMagnetic2_canvas_large *pCanvas,int *pEnd);

#endif
