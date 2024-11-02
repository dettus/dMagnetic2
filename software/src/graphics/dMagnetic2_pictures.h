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

#ifndef	DMAGNETIC2_PICTURES_H
#define	DMAGNETIC2_PICTURES_H

#include "dMagnetic2_graphics.h"	// for the datatypes

typedef	enum _tGraphic_format
{
	DMAGNETIC2_FORMAT_NONE=0,
	DMAGNETIC2_FORMAT_GFX1,	// MaPi
	DMAGNETIC2_FORMAT_GFX2,	// MaP2
	DMAGNETIC2_FORMAT_MSDOS,	// MaP3
	DMAGNETIC2_FORMAT_MAGWIN,	// MaP4
	DMAGNETIC2_FORMAT_C64,		// MaP5
	DMAGNETIC2_FORMAT_AMSTRAD_CPC,	// MaP6
	DMAGNETIC2_FORMAT_ATARI_XL,	// MaP7
	DMAGNETIC2_FORMAT_APPLE_II	// MaP8
} tGraphic_format;
typedef	struct _tdMagnetic2_picture_handle
{
	unsigned int magic;
	unsigned char *pTmpBuf;
	unsigned char *pGfxBuf;
	int gfxsize;
	tGraphic_format format;
} tdMagnetic2_picture_handle;


int dMagnetic2_pictures_init(tdMagnetic2_picture_handle *pThis,unsigned char* pTmpBuf);
int dMagnetic2_pictures_set_gfx(tdMagnetic2_picture_handle *pThis,unsigned char* pGfxBuf,int gfxsize);
int dMagnetic2_pictures_decode_by_picnum(tdMagnetic2_picture_handle *pThis,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge);
int dMagnetic2_pictures_decode_by_picname(tdMagnetic2_picture_handle *pThis,char* picname,int vga0ega1,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge);

#endif


