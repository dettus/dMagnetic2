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

#include "dMagnetic2_pictures.h"
#include "dMagnetic2_graphics.h"		// for the datatypes
#include "dMagnetic2_shared.h"			// for the macros
#include "dMagnetic2_errorcodes.h"		// for the error codes

#include "dMagnetic2_pictures_amstrad_cpc.h"	// Header MaP6
#include "dMagnetic2_pictures_appleii.h"	// Header MaP8
#include "dMagnetic2_pictures_atarixl.h"	// Header MaP7
#include "dMagnetic2_pictures_c64.h"		// Header MaP5
#include "dMagnetic2_pictures_gfx.h"		// Header MaPi and Map2
#include "dMagnetic2_pictures_magwin.h"		// Header MaP4
#include "dMagnetic2_pictures_msdos.h"		// Header MaP3


#define	MAGICNUM	0x1345abdf

typedef	struct _tdMagnetic2_picture_handle
{
	unsigned int magic;
	unsigned char tmpbuf[65536];		// TODO: how much is needed?
	unsigned char *pGfxbuf;
	int gfxsize;
	int version;
	int vga0ega1;
} tdMagnetic2_picture_handle;


int dMagnetic2_pictures_getsize(int *pBytes)
{
	*pBytes=sizeof(tdMagnetic2_picture_handle);
	return DMAGNETIC2_OK;
}

int dMagnetic2_pictures_init(void *pHandle,unsigned char* pGfxbuf,int gfxsize,int version,int vga0ega1)
{
	tdMagnetic2_picture_handle* pThis=(tdMagnetic2_picture_handle*)pHandle;
	pThis->magic=MAGICNUM;
	pThis->pGfxbuf=pGfxbuf;
	pThis->gfxsize=gfxsize;
	pThis->version=version;
	pThis->vga0ega1=vga0ega1;
	return DMAGNETIC2_OK;
}


int dMagnetic2_pictures_decode(void *pHandle,char* picname,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	tdMagnetic2_picture_handle* pThis=(tdMagnetic2_picture_handle*)pHandle;
	return DMAGNETIC2_OK;
}



