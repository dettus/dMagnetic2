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

#ifndef	DMAGNETIC2_GRAPHICS_H
#define	DMAGNETIC2_GRAPHICS_H


#define	DMAGNETIC2_GRAPHICS_MAX_COLORS	16
#define	DMAGNETIC2_GRAPHICS_MAX_WIDTH	640
#define	DMAGNETIC2_GRAPHICS_MAX_HEIGHT	480
#define	DMAGNETIC2_GRAPHICS_MAX_PIXELS	(DMAGNETIC2_GRAPHICS_MAX_WIDTH*DMAGNETIC2_GRAPHICS_MAX_HEIGHT)

#define	DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL	10

// The RGB values are 10 bit wide. 
// red = 29..20
// green=19..10
// blue=  9.. 0

typedef struct _tdMagnetic2_canvas_small
{
	int width;	// width in pixels
	int height;	// height in pixels
	unsigned int flags;	// some meta information, for the renderer
	unsigned int rgb[DMAGNETIC2_GRAPHICS_MAX_COLORS];	// RGB values. 10 Bit per channel.
	unsigned char pixels[DMAGNETIC2_GRAPHICS_MAX_PIXELS];
} tdMagnetic2_canvas_small;

typedef struct _tdMagnetic2_canvas_large
{
	int width;	// width in pixels
	int height;	// height in pixels
	unsigned int flags;	// some meta information, for the renderer
	unsigned int rgbpixels[DMAGNETIC2_GRAPHICS_MAX_PIXELS];	// RGB values. 10 Bit per pixel
} tdMagnetic2_canvas_large;


#define	DMAGNETIC2_GRAPHICS_RENDER_FLAG_NONE		0
#define	DMAGNETIC2_GRAPHICS_RENDER_FLAG_VALID		(1<<0)
#define	DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64		(1<<1)
#define	DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN		(1<<2)
#define	DMAGNETIC2_GRAPHICS_RENDER_FLAG_HALFTONE	(1<<3)


// API functions for initialization
int dMagnetic2_graphics_get_size(int *pBytes);
int dMagnetic2_graphics_init(void *pHandle);
int dMagnetic2_graphics_set_gfx(void *pHandle,int size,unsigned char* pGfx,int vga0ega1);

// API functions for drawing
int dMagnetic2_graphics_set_current_picture(void *pHandle,char* pPicname,int picNum);
int dMagnetic2_graphics_get_current_type(void* pHandle,char* todo);
int dMagnetic2_graphics_draw_picture(void* pHandle,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge);
int dMagnetic2_graphics_draw_animation(void* pHandle,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge,int *pEnd);
int dMagnetic2_graphics_canvas_small_to_xpm(tdMagnetic2_canvas_small *pSmall,char* pxpm,int xpmbufsize);

#endif
