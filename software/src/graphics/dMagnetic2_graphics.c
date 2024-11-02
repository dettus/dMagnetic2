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

#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_graphics.h"
#include "dMagnetic2_pictures.h"
#include "dMagnetic2_animations_magwin.h"

#include <string.h>
#include <stdio.h>

#define	MAGICNUM	0xbc412ef9


#define	MAX_GFX_SIZE	2534116		// wonder.gfx has 2534113 bytes
typedef	struct _tHandle_graphics
{
	unsigned int	magic;
	tdMagnetic2_picture_handle hPicture;
	tdMagnetic2_animations_handle hAnimations;
} tHandle_graphics;


int dMagnetic2_graphics_getsize(int *pSize_handle,int *pSize_tmpbuf)
{
	*pSize_handle=sizeof(tHandle_graphics);
	*pSize_tmpbuf=16384;			// TODO: how much is really needed?
	return DMAGNETIC2_OK;
}

int dMagnetic2_graphics_init(void *pHandle,void *pTmpBuf)
{
	int retval;
	tHandle_graphics* pThis=(tHandle_graphics*)pHandle;
	memset(pThis,0,sizeof(tHandle_graphics));
	pThis->magic=MAGICNUM;
	retval=dMagnetic2_pictures_init(&(pThis->hPicture),pTmpBuf);
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}

	retval=dMagnetic2_animations_magwin_init(&(pThis->hAnimations));
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}

	
	return DMAGNETIC2_OK;
}

int dMagnetic2_graphics_set_gfx(void *pHandle,unsigned char* pGfxBuf,int gfxsize)
{
	tHandle_graphics* pThis=(tHandle_graphics*)pHandle;
	int retval;
	if (pThis->magic!=MAGICNUM)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}

	retval=dMagnetic2_pictures_set_gfx(&(pThis->hPicture),pGfxBuf,gfxsize);
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}
	retval=dMagnetic2_animations_magin_set_gfx(&(pThis->hAnimations),pGfxBuf,gfxsize);
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}

	return retval;
}
int dMagnetic2_graphics_decode_by_picnum(void *pHandle,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	tHandle_graphics* pThis=(tHandle_graphics*)pHandle;
	int retval;
	if (pThis->magic!=MAGICNUM)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	retval=dMagnetic2_pictures_decode_by_picnum(&(pThis->hPicture),picnum,pSmall,pLarge);
	return retval;
}
#define	VGA0EGA1	0		// TODO
int dMagnetic2_graphics_decode_by_picname(void *pHandle,char* picname,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge,int* pIsAnimation)
{
	tHandle_graphics* pThis=(tHandle_graphics*)pHandle;
	int retval;
	if (pThis->magic!=MAGICNUM)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	retval=dMagnetic2_pictures_decode_by_picname(&(pThis->hPicture),picname,VGA0EGA1,pSmall,pLarge);
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}
	// check if there is an animation available under this name
	retval=dMagnetic2_animations_magwin_start(&(pThis->hAnimations),picname,pIsAnimation);
	return retval;
}
int dMagnetic2_graphics_animation_nxtframe(void* pHandle,int *pIsLast,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	tHandle_graphics* pThis=(tHandle_graphics*)pHandle;
	int retval;
	if (pThis->magic!=MAGICNUM)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	retval=dMagnetic2_animations_magwin_render_frame(&(pThis->hAnimations),pIsLast,pSmall,pLarge);
	return retval;
}


//////////////////// some converters /////////////////////////////
int dMagnetic2_graphics_canvas_small_to_xpm(tdMagnetic2_canvas_small *pSmall,char* pxpm,int xpmbufsize)
{
	int i;
	int idx;

	int width;
	int height;
	int cols;

	width=pSmall->width;
	height=pSmall->height;
	cols=16;
	if (pSmall->flags&DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN)
	{
		width*=2;
	}
	if (pSmall->flags&DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64)
	{
		width*=2;
		height*=2;
		cols=17;
	}
	
	idx=0;
	idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"/* XPM */\n");
	idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"static char *xpm[] = {\n");
	idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"/* columns rows colors chars-per-pixel */\n");
	idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\"%d %d %d 1 \",\n",width,height,cols);
	for (i=0;i<16;i++)
	{
		unsigned int rgb;
		unsigned int red,green,blue;

		rgb=pSmall->rgb[i];
		red=(rgb>>20)&0x3ff;	rgb<<=10;	
		green=(rgb>>20)&0x3ff;	rgb<<=10;	
		blue=(rgb>>20)&0x3ff;	rgb<<=10;	

		red*=255;red/=1023;
		green*=255;green/=1023;
		blue*=255;blue/=1023;


		rgb=(red<<16)|(green<<8)|blue;
		idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\"%X c #%06x\",\n",i,rgb);
	}
	if (pSmall->flags&DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64)
	{
		idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\". c #%06x\",\n",0);
	}
	for (i=0;i<pSmall->height;i++)
	{
		int j;
		idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\"");
		for (j=0;j<pSmall->width;j++)
		{
			int c,p;
			p=j+i*(pSmall->width);
			c=pSmall->pixels[p];
			idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"%X",c);
			if (pSmall->flags&DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN)
			{
				idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"%X",c);
				if (pSmall->flags&DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64)
				{
					idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"%X",c);
					idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"%X",c);

				}
			}
		}
		idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\"");
		if (pSmall->flags&DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64)
		{
			idx+=snprintf(&pxpm[idx],xpmbufsize-idx,",\n");
			idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\"");
			for (j=0;j<width;j++)
			{
				idx+=snprintf(&pxpm[idx],xpmbufsize-idx,".");
			}
			idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"\"");
			
		}
		if (i!=(pSmall->height-1)) 
		{
			idx+=snprintf(&pxpm[idx],xpmbufsize-idx,",\n");
		}
	}
	idx+=snprintf(&pxpm[idx],xpmbufsize-idx,"};\n");
	pxpm[idx]=0;
	return DMAGNETIC2_OK;
}

