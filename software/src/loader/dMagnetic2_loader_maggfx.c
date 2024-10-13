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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_shared.h"			// for the macros
#include "dMagnetic2_errorcodes.h"		// for the error codes

#include "dMagnetic2_loader.h"
#include "dMagnetic2_loader_shared.h"

void dMagnetic2_loader_maggfx_detect_game(unsigned char *pMagBuf,tdMagnetic2_game_meta *pMeta)
{
	pMeta->source=DMAGNETIC2_SOURCE_MAGGFX;
	pMeta->version=READ_INT16BE(pMagBuf,12);		// the verion is stored in bytes 12..13 of the header
	pMeta->game=DMAGNETIC2_GAME_TODO;			// not yet implemented
	pMeta->source=DMAGNETIC2_SOURCE_MAGGFX;
}

int dMagnetic2_loader_maggfx(
		char* filename1,char* filename2,
		unsigned char* pMagBuf,
		unsigned char* pGfxBuf,
		tdMagnetic2_game_meta *pMeta)
		
{
	char header[4];			// read the header
	int n;
	int detected_mag;
	int detected_gfx;
	FILE *f;
	// check the important output buffers
	if (pMeta==NULL)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}

	pMeta->game=DMAGNETIC2_GAME_NONE;
	pMeta->source=DMAGNETIC2_SOURCE_NONE;
	pMeta->version=-1;
	pMeta->real_magsize=0;
	pMeta->real_gfxsize=0;


	detected_mag=0;
	detected_gfx=0;


	if (filename1!=NULL)
	{
		// check if the file starts with the correct magic word(s)
		f=fopen(filename1,"rb");
		if (!f)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(header,sizeof(char),4,f);
		fclose(f);
		if (n!=4)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}

		
		if (header[0]=='M' && header[1]=='a' && header[2]=='S' && header[3]=='c') 
		{
			detected_mag=1;	// the first file was detected as .mag
		}
		if (header[0]=='M' && header[1]=='a' && header[2]=='P')
		{
			detected_gfx=1;	// the first file was detected as .gfx
		}
		if (detected_mag==0 && detected_gfx==0)	// the file did not start with the correct magic word.
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	} else {
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	if (filename2!=NULL)
	{
		// check if the file starts with the correct magic word(s)
		f=fopen(filename2,"rb");
		if (!f)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(header,sizeof(char),4,f);
		fclose(f);
		if (n!=4)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		if (header[0]=='M' && header[1]=='a' && header[2]=='S' && header[3]=='c') 
		{
			detected_mag=2;	// the second file was detected as .mag
		}
		if (header[0]=='M' && header[1]=='a' && header[2]=='P')
		{
			detected_gfx=2;	// the second file was detected as .gfx
		}
		if (detected_mag==0 && detected_gfx==0)	// the file did not start with the correct magic word.
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}
	// here, it is established that the file(s) work.
	f=NULL;
	if (detected_mag==1)
	{
		f=fopen(filename1,"rb");
	}
	if (detected_mag==2)
	{
		f=fopen(filename2,"rb");
	}
	if (f)
	{
		n=fread(pMagBuf,sizeof(char),DMAGNETIC2_MAX_MAGSIZE,f);
		fclose(f);
		pMeta->real_magsize=n;
		dMagnetic2_loader_maggfx_detect_game(pMagBuf,pMeta);
	}
	f=NULL;
	if (detected_gfx==1)
	{
		f=fopen(filename1,"rb");
	}
	if (detected_gfx==2)
	{
		f=fopen(filename2,"rb");
	}
	if (f)
	{
		n=fread(pMagBuf,sizeof(char),DMAGNETIC2_MAX_GFXSIZE,f);
		fclose(f);
		pMeta->real_gfxsize=n;
	}
	return DMAGNETIC2_OK;	


}

