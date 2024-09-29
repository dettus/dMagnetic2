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
#include <string.h>

#define	MAGICNUM	0xbc412ef9


#define	MAX_GFX_SIZE	2534116		// wonder.gfx is 2534113 bytes large
typedef	struct _tHandle_graphics
{
	unsigned int	magic;

	int 		gfxsize;
	unsigned char	gfxbuf[MAX_GFX_SIZE];
// internal canvas
	tdMagnetic2_canvas_small	canvas_small;
// configurations
	int		vga0ega1;
} tHandle_graphics;


int dMagnetic2_graphics_getSize(int *pBytes)
{
	int size;
	size=sizeof(tHandle_graphics);
	return DMAGNETIC2_OK;
}

int dMagnetic2_graphics_init(void *pHandle)
{
	tHandle_graphics* pThis=(tHandle_graphics*)pThis;
	memset(pThis,0,sizeof(tHandle_graphics));
	pThis->magic=MAGICNUM;
	return DMAGNETIC2_OK;
}

int dMagnetic2_graphics_check_handle(tHandle_graphics* pThis)
{
	int retval;

	retval=DMAGNETIC2_OK;
	if (pThis->magic!=MAGICNUM)
	{
		retval=DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	return retval;
}

int dMagnetic2_graphics_set_gfx(void *pHandle,int size,unsigned char* pGfx,int vga0ega1)
{
	tHandle_graphics* pThis=(tHandle_graphics*)pThis;
	int retval;
	retval=dMagnetic2_graphics_check_handle(pThis);
	if (retval==DMAGNETIC2_OK)
	{
		if (size<=MAX_GFX_SIZE)
		{
			memcpy(pThis->gfxbuf,pGfx,size);
			pThis->gfxsize=size;
			pThis->vga0ega1=vga0ega1;
		} else {
			retval=DMAGNETIC2_ERROR_BUFFER_TOO_SMALL;
		}
	}
	return retval;
}
