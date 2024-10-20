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
#include "dMagnetic2_engine.h"
#include "dMagnetic2_shared.h"
#include "dMagnetic2_engine_shared.h"
#include "dMagnetic2_engine_vm68k.h"
#include <string.h>


#define	SIZE_INPUTBUF		256
#define	SIZE_OUTPUTBUF		4096
#define	SIZE_TITLEBUF		128
#define	SIZE_PICNAMEBUF		6
#define	SIZE_FILENAMEBUF	64

#define	MAGIC		0x28844


typedef struct _tdMagnetic2_game_context
{
	tVM68k	vm68k;
} tdMagnetic2_game_context;

typedef struct _tdMagnetic2_engine_handle
{
	unsigned int magic;
	char inputbuf[SIZE_INPUTBUF];
	// add one byte for 0 termination
	char outputbuf[SIZE_OUTPUTBUF+1];
	char titlebuf[SIZE_TITLEBUF+1];
	char picnamebuf[SIZE_PICNAMEBUF+1];
	int picturenum;
	char filenamebuf[SIZE_FILENAMEBUF+1];


	



	int inputlevel;
	int outputlevel;
	int titlelevel;
	int filenamelevel;

	unsigned int status_flags;

	tdMagnetic2_game_context	game_context;
} tdMagnetic2_engine_handle;

int dMagnetic2_engine_get_size(int *pBytes)
{
	if (pBytes==NULL)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}
	*pBytes=sizeof(tdMagnetic2_engine_handle);
	return DMAGNETIC2_OK;
}

int dMagnetic2_engine_init(void *pHandle)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;

	memset(pThis,0,sizeof(tdMagnetic2_engine_handle));
	pThis->magic=MAGIC;

	return DMAGNETIC2_OK;
}

int dMagnetic2_engine_set_mag(void *pHandle,unsigned char* pMagBuf)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;

	dMagnetic2_engine_vm68k_init(&(pThis->game_context.vm68k),pMagBuf);
//	dMagnetic2_linea_init(&pThis->game_context.linea,pMagBuf);
	return DMAGNETIC2_OK;
	
		
}

int dMagnetic2_engine_new_input(void *pHandle,int len,char* pInput,int *pCnt)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;
	int i;
	int cnt;

	if (pThis->magic!=MAGIC)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}

	cnt=0;
	for (i=0;i<len;i++)
	{
		if (pThis->inputlevel<SIZE_INPUTBUF)
		{
			pThis->inputbuf[pThis->inputlevel]=pInput[i];
			pThis->inputlevel++;
			cnt++;
		}
	}

	*pCnt=cnt;	// report back the number of character that have been read
	return DMAGNETIC2_OK;
}

int dMagnetic2_engine_get_text(void *pHandle,char** ppText)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;
	if (pThis->magic!=MAGIC)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}

	*ppText=&(pThis->outputbuf[0]);
	pThis->outputlevel=0;
	pThis->status_flags&=~DMAGNETIC2_ENGINE_STATUS_NEW_TEXT;
	
	return DMAGNETIC2_OK;
	
}

int dMagnetic2_engine_get_title(void *pHandle,char** ppTitle)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;
	if (pThis->magic!=MAGIC)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}

	*ppTitle=&(pThis->titlebuf[0]);
	pThis->titlelevel=0;
	pThis->status_flags&=~DMAGNETIC2_ENGINE_STATUS_NEW_TITLE;
	
	return DMAGNETIC2_OK;
	
}
int dMagnetic2_engine_get_picture(void *pHandle,char** ppPicname,int *pPicnum)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;
	if (pThis->magic!=MAGIC)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}

	*ppPicname=&(pThis->picnamebuf[0]);
	*pPicnum=pThis->picturenum;
	pThis->status_flags&=~DMAGNETIC2_ENGINE_STATUS_NEW_PICTURE;
	
	return DMAGNETIC2_OK;
}	
int dMagnetic2_engine_get_filename(void *pHandle,char** ppFilename)
{
	tdMagnetic2_engine_handle* pThis=(tdMagnetic2_engine_handle*)pHandle;
	if (pThis->magic!=MAGIC)
	{
		return DMAGNETIC2_ERROR_WRONG_HANDLE;
	}

	*ppFilename=&(pThis->filenamebuf[0]);
	pThis->filenamelevel=0;
	
	return DMAGNETIC2_OK;
	
}
