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

#include "dMagnetic2_loader.h"
#include "dMagnetic2_loader_shared.h"

const unsigned char *dMagnetic2_game_names[8]={
	"unknown",
	"The Pawn",
	"The Guild Of Thieves",
	"Jinxter",
	"Corruption",
	"Myth",
	"Fish!",
	"Wonderland"
};

const unsigned char *dMagnetic2_game_sources[10]={
	"unknown",
	".mag/.gfx",
	"Acron Archimedes",
	"MSDOS",
	"Magnetic Windows Resource Files",
	"Commodore C64",
	"Amstrad CPC",
	"Spectrum +3",
	"Atari XL",
	"Apple II"
};

#define	MAX_TMP_SIZE	(1<<20)		// TODO. c64: 2*174848 bytes+1. archimedes: 819200+1. appleii: 3*232960+1

#define	MAGIC		0xfb328c12

typedef struct _tdMagnetic2_loader_handle
{
	unsigned int magic;
	unsigned char tmpbuf[MAX_TMP_SIZE];
} tdMagnetic2_loader_handle;

#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_loader.h"

int dMagnetic2_loader_getsize(int *pBytes)
{
	*pBytes=sizeof(tdMagnetic2_loader_handle);
	return DMAGNETIC2_OK;
}
int dMagnetic2_loader_init(void *pHandle)
{
	tdMagnetic2_loader_handle* pThis=(tdMagnetic2_loader_handle*)pHandle;
	if (pThis==NULL)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	memset(pThis,0,sizeof(tdMagnetic2_loader_handle));
	pThis->magic=MAGIC;

	return DMAGNETIC2_OK;
}
int dMagnetic2_loader(void *pHandle,char* filename1,char* filename2,char* filename3,unsigned char* pMagBuf,unsigned char* pGfxBuf,tdMagnetic2_game_meta *pMeta)
{
	tdMagnetic2_loader_handle* pThis=(tdMagnetic2_loader_handle*)pHandle;
	if (pThis==NULL)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	if (pThis->magic!=MAGIC)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	// the idea here is some sort of autodetection.
	// there are several loaders. each one is tried, and one of them should return with a valid loaded game.
	
	return DMAGNETIC2_UNKNOWN_SOURCE;

}
