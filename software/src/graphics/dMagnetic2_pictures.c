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
#include <stdlib.h>
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
#define	TMPBUFSIZE	65536		// TODO: how much is needed?

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
	int version;
	int vga0ega1;
	tGraphic_format format;
} tdMagnetic2_picture_handle;


int dMagnetic2_pictures_getsize(int *pBytes,int *size_tmpbuf)
{
	*pBytes=sizeof(tdMagnetic2_picture_handle);
	*size_tmpbuf=TMPBUFSIZE;
	return DMAGNETIC2_OK;
}

int dMagnetic2_pictures_init(void *pHandle,unsigned char* pGfxBuf,unsigned char* pTmpBuf)
{
	tdMagnetic2_picture_handle* pThis=(tdMagnetic2_picture_handle*)pHandle;
	pThis->magic=MAGICNUM;
	pThis->pTmpBuf=pTmpBuf;
	pThis->pGfxBuf=NULL;
	pThis->format=DMAGNETIC2_FORMAT_NONE;
	return DMAGNETIC2_OK;
}
int dMagnetic2_pictures_set_gfx(void* pHandle,unsigned char* pGfxBuf,int gfxsize,int version,int vga0ega1)
{
	tdMagnetic2_picture_handle* pThis=(tdMagnetic2_picture_handle*)pHandle;
	pThis->magic=MAGICNUM;
	pThis->pGfxBuf=pGfxBuf;
	pThis->gfxsize=gfxsize;
	pThis->version=version;
	pThis->vga0ega1=vga0ega1;

	if (pGfxBuf!=NULL)
	{
		if (pGfxBuf[0]=='M' && pGfxBuf[1]=='a' && pGfxBuf[1]=='P')
		{
			switch (pGfxBuf[3])
			{
				case 'i':	pThis->format=DMAGNETIC2_FORMAT_GFX1;			break;
				case '2':	pThis->format=DMAGNETIC2_FORMAT_GFX2;			break;
				case '3':	pThis->format=DMAGNETIC2_FORMAT_MSDOS;			break;
				case '4':	pThis->format=DMAGNETIC2_FORMAT_MAGWIN;			break;
				case '5':	pThis->format=DMAGNETIC2_FORMAT_C64;			break;
				case '6':	pThis->format=DMAGNETIC2_FORMAT_AMSTRAD_CPC;		break;
				case '7':	pThis->format=DMAGNETIC2_FORMAT_ATARI_XL;		break;
				case '8':	pThis->format=DMAGNETIC2_FORMAT_APPLE_II;		break;
				default:	pThis->format=DMAGNETIC2_FORMAT_NONE;			break;
			}
		}
	}

	return DMAGNETIC2_OK;
}


int dMagnetic2_pictures_decode_by_picnum(void *pHandle,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	tdMagnetic2_picture_handle* pThis=(tdMagnetic2_picture_handle*)pHandle;
	int retval;

	retval=DMAGNETIC2_OK;
	switch (pThis->format)
	{
		case DMAGNETIC2_FORMAT_GFX1:		retval=dMagnetic2_gfxloader_gfx1(pThis->pGfxBuf,pThis->gfxsize,picnum,pSmall,pLarge);	break;
		case DMAGNETIC2_FORMAT_MSDOS:		retval=dMagnetic2_gfxloader_msdos(pThis->pGfxBuf,pThis->gfxsize,picnum,pSmall,pLarge);	break;
		case DMAGNETIC2_FORMAT_C64:		retval=dMagnetic2_gfxloader_c64(pThis->pGfxBuf,pThis->gfxsize,pThis->pTmpBuf,picnum,pSmall,pLarge);	break;
		case DMAGNETIC2_FORMAT_AMSTRAD_CPC:	retval=dMagnetic2_gfxloader_amstrad_cpc(pThis->pGfxBuf,pThis->gfxsize,picnum,pSmall,pLarge);	break;
		case DMAGNETIC2_FORMAT_ATARI_XL:	retval=dMagnetic2_gfxloader_atarixl(pThis->pGfxBuf,pThis->gfxsize,picnum,pSmall,pLarge);	break;
		case DMAGNETIC2_FORMAT_APPLE_II:	retval=dMagnetic2_gfxloader_appleii(pThis->pGfxBuf,pThis->gfxsize,pThis->pTmpBuf,picnum,pSmall,pLarge);	break;
		default:
			retval=DMAGNETIC2_OK;
			
	}
	return retval;
}

int dMagnetic2_pictures_decode_by_picname(void *pHandle,char* picname,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	tdMagnetic2_picture_handle* pThis=(tdMagnetic2_picture_handle*)pHandle;
	int retval;
	switch (pThis->format)
	{
		case DMAGNETIC2_FORMAT_GFX2:		retval=dMagnetic2_gfxloader_gfx2(pThis->pGfxBuf,pThis->gfxsize,picname,pSmall,pLarge);	break;
		case DMAGNETIC2_FORMAT_MAGWIN:		retval=dMagnetic2_gfxloader_magwin(pThis->pGfxBuf,pThis->gfxsize,picname,pThis->vga0ega1,pSmall,pLarge);	break;
		default:
			retval=DMAGNETIC2_OK;
			
	}
	return retval;
}



