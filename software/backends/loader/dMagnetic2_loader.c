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


#include "dMagnetic2_loader_appleii.h"
#include "dMagnetic2_loader_archimedes.h"
#include "dMagnetic2_loader_atarixl.h"
#include "dMagnetic2_loader_c64.h"
#include "dMagnetic2_loader_dsk.h"
#include "dMagnetic2_loader_maggfx.h"
#include "dMagnetic2_loader_msdos.h"
#include "dMagnetic2_loader_mw.h"

const char *dMagnetic2_game_names[8]={
	"unknown",
	"The Pawn",
	"The Guild Of Thieves",
	"Jinxter",
	"Corruption",
	"Myth",
	"Fish!",
	"Wonderland"
};

const char *dMagnetic2_game_sources[10]={
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
	unsigned char *pTmpBuf;
} tdMagnetic2_loader_handle;

#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_loader.h"

int dMagnetic2_loader_getsize(int *size_handle,int *size_tmpbuf)
{
	int max;
	int size_tmp;
	*size_handle=sizeof(tdMagnetic2_loader_handle);
	max=0;
	dMagnetic2_loader_appleii_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_archimedes_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_atarixl_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_c64_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_dsk_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_maggfx_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_msdos_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	dMagnetic2_loader_mw_getsize(&size_tmp);
	if (max<size_tmp)
	{
		max=size_tmp;
	}
	*size_tmpbuf=max;
	
	return DMAGNETIC2_OK;
}
int dMagnetic2_loader_init(void *pHandle,void* pTmpBuf)
{
	tdMagnetic2_loader_handle* pThis=(tdMagnetic2_loader_handle*)pHandle;
	if (pThis==NULL)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}
	memset(pThis,0,sizeof(tdMagnetic2_loader_handle));
	pThis->magic=MAGIC;
	pThis->pTmpBuf=(unsigned char*)pTmpBuf;

	return DMAGNETIC2_OK;
}
int dMagnetic2_loader(void *pHandle,char* filename1,char* filename2,char* filename3,unsigned char* pMagBuf,unsigned char* pGfxBuf,tdMagnetic2_game_meta *pMeta,int nodoc)
{
	int retval;
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


	retval=DMAGNETIC2_UNKNOWN_SOURCE;

	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_appleii(filename1,filename2,filename3,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_archimedes(filename1,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_atarixl(filename1,filename2,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_c64(filename1,filename2,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_dsk(filename1,filename2,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,0,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_dsk(filename1,filename2,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,1,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_maggfx(filename1,filename2,pMagBuf,pGfxBuf,pMeta);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_msdos(filename1,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta,nodoc);
	}
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		retval=dMagnetic2_loader_mw(filename1,pThis->pTmpBuf,MAX_TMP_SIZE,pMagBuf,pGfxBuf,pMeta);
	}
	switch(pMeta->game)
	{
		case DMAGNETIC2_GAME_NONE:		strncpy(pMeta->game_name,"UNKNOWN",32);break;
		case DMAGNETIC2_GAME_PAWN:		strncpy(pMeta->game_name,"The Pawn",32);break;
		case DMAGNETIC2_GAME_GUILD:		strncpy(pMeta->game_name,"The Guild Of Thieves",32);break;
		case DMAGNETIC2_GAME_JINXTER:		strncpy(pMeta->game_name,"Jinxter",32);break;
		case DMAGNETIC2_GAME_CORRUPTION:	strncpy(pMeta->game_name,"Corruption",32);break;
		case DMAGNETIC2_GAME_MYTH:		strncpy(pMeta->game_name,"Myth",32);break;
		case DMAGNETIC2_GAME_FISH:		strncpy(pMeta->game_name,"Fish!",32);break;
		case DMAGNETIC2_GAME_WONDERLAND:	strncpy(pMeta->game_name,"Wonderland",32);break;
		default:				strncpy(pMeta->game_name,"TODO",32);break;
	}
	switch (pMeta->source)
	{
		case DMAGNETIC2_SOURCE_NONE:		strncpy(pMeta->source_name,"UNKNOWN",32);break;
		case DMAGNETIC2_SOURCE_MAGGFX:		strncpy(pMeta->source_name,".mag/.gfx",32);break;
		case DMAGNETIC2_SOURCE_ARCHIMEDES:	strncpy(pMeta->source_name,"Acron Archimedes",32);break;
		case DMAGNETIC2_SOURCE_MSDOS:		strncpy(pMeta->source_name,"MS-DOS",32);break;
		case DMAGNETIC2_SOURCE_MW:		strncpy(pMeta->source_name,"Magnetic Windows Resource File",32);break;
		case DMAGNETIC2_SOURCE_C64:		strncpy(pMeta->source_name,"Commodore 64",32);break;
		case DMAGNETIC2_SOURCE_AMSTRAD_CPC:	strncpy(pMeta->source_name,"Amstrad CPC",32);break;
		case DMAGNETIC2_SOURCE_SPECTRUM:	strncpy(pMeta->source_name,"Spectrum +3",32);break;
		case DMAGNETIC2_SOURCE_ATARIXL:		strncpy(pMeta->source_name,"Atari XL",32);break;
		case DMAGNETIC2_SOURCE_APPLEII:		strncpy(pMeta->source_name,"Apple II",32);break;
		default:				strncpy(pMeta->source_name,"TODO",32);break;
	}
	
	return retval;

}
