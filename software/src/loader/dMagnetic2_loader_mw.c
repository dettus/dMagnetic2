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
#define	SIZE_RSC_FILE	362496
#define	FILENAME_LENGTH_MAX	1024

#define	MAXGAMES	4
#define	MAX_NUM_RSC_FILES	9	// actually 7, because there is no ZERO.RSC and wonderland has no ONE.RSC. But it goes up to EIGHT.RSC

typedef	struct _tGameInfo
{
	edMagnetic2_game	game;
	char			prefix1;	// the prefix of the File TWO.RSC can be used to determine the game.
	char			prefix2;	// the prefix of the File two.rsc can be used to determine the game.
	int			rscsize;	// as does the size.
} tGameInfo;

#define	GAME_IDX_WONDERLAND	0		// todo: the game identification should be defaulting to wonderland.
const tGameInfo dMagnetic2_loader_mw_gameinfo[MAXGAMES]=
{
	{	
		.game=DMAGNETIC2_GAME_WONDERLAND,
		.prefix1=0,			// names are TWO.RSC
		.prefix2=0,			// names are TWO.RSC
		.rscsize=362496+362496+362496+362496+362496+362496+332633
	},
	{
		.game=DMAGNETIC2_GAME_GUILD,
		.prefix1='G',			// names are GTWO.RSC or similar
		.prefix2='g',			// names are gtwo.rsc or similar
		.rscsize=4+362496+362496+76800+188726+8394
	},
	{
		.game=DMAGNETIC2_GAME_CORRUPTION,
		.prefix1='C',			// names are CTWO.rsc or similar
		.prefix2='c',			// names are ctwo.rsc or similar
		.rscsize=4+362496+362496+362496+23728+6900
	},
	{
		.game=DMAGNETIC2_GAME_FISH,
		.prefix1='F',			// names are FTWO.rsc or similar
		.prefix2='f',			// names are ftwo.rsc or similar
		.rscsize=4+362496+362496+173056+64175
	}
}

// the purpose of this function is to change the filename from TWO.RSC to THREE.RSC, for example.
int dMagnetic2_loader_mw_substitute_tworsc(char* filename1,char *pFilename,int num,int *gameidx)
{
	int i;
	int l;
	int location;
	int uppercase;

	const char *dMagnetic2_loader_mw_names_upper[MAX_NUM_RSC_FILES+2]={"ZERO","ONE","TWO","THREE","FOUR","FIVE","SIX","SEVEN","EIGHT","TITLE.VGA","TITLE.EGA"};
	const char *dMagnetic2_loader_mw_names_lower[MAX_NUM_RSC_FILES+2]={"zero","one","two","three","four","five","six","seven","eight","title.vga","title.ega"};

	if (num<0 || num>=MAX_NUM_RSC_FILES+2)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}

	l=strlen(filename1);
	location=-1;
	for (i=0;i<l-7;i++)
	{
		// at the same time: check the capitalization
		if (filename1[i+0]=='t' && filename1[i+1]=='w' && filename1[i+2]=='o' && filename1[i+3]=='.')
		{
			location=i;
			uppercase=0;
		}
		if (filename1[i+0]=='T' && filename1[i+1]=='W' && filename1[i+2]=='O' && filename1[i+3]=='.')
		{
			location=i;
			uppercase=1;
		}
	}
	if (location==-1)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	if (gameidx!=NULL)
	{
		*gameidx=GAME_IDX_WONDERLAND;
		if (location>0)
		{
			int i;
			for (i=1;i<MAXGAMES;i++)
			{
				if (filename1[location-1]==dMagnetic2_loader_mw_gameinfo[i].prefix1 || filename1[location-1]==dMagnetic2_loader_mw_gameinfo[i].prefix2)
				{
					*gameidx=i;
				}
			}
		} else {
			*gameidx=GAME_IDX_WONDERLAND;
		}
	}

	strncpy(pFilename,filename,FILENAME_LENGTH_MAX);
	l=strlen(dMagnetic2_loader_mw_names[num]);
	for (i=0;i<l+1;i++)
	{
		int p;
		p=i+location;
		if (p>=FILENAME_LENGTH_MAX)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		pFilename[p]=uppercase?dMagnetic2_loader_mw_names_upper[num][i]:dMagnetic2_loader_mw_names_lower[num][i];
	}

	return DMAGNETIC2_OK;	
	
}

int dMagnetic2_loader_mw_sizes(unsigned char* pTmpBuf,char* filename1,int *pSizes,int *gameidx)
{
	int i;
	int sum;
	int retval;
	int gameidx_int;
	char *pFilename;
	FILE *f;
	pFilename=&pTmpBuf[SIZE_RSC_FILES_MAX];	
	gameidx_int=-1;
	for (i=0;i<MAX_NUM_RSC_FILES;i++)
	{
		int n;
		retval=dMagnetic2_loader_mw_substitute_tworsc(filename1,pFilename,i,&gameidx_int);
		if (retval!=DMAGNETIC2_OK)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	
		f=fopen(pFilename,"rb");
		n=0;
		if (f!=NULL)
		{
			n=fread(pTmpBuf,sizeof(char),SIZE_RSC_FILE+1,f);
			fclose(f);
			if (n>SIZE_RSC_FILE)	// easy sanity check.
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}
		}
		pSizes[i]=n;
		sum+=n;
	}
	if (gameidx_int==-1)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	if (sum!=dMagnetic2_loader_mw_gameinfo[gameidx].rscsize)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	*gameidx=gameidx_int;
	return DMAGNETIC2_OK;
}
int dMagnetic2_loader_mw_readresource(unsigned char* pTmpBuf,char* filename1,int sizes,int offset,unsigned char* pOutput,int length)
{
	int i;
	char *pFilename;
	FILE *f;
	int sum;
	int offset_in_rsc;
	int rsc_file;
	int output_idx;
	pFilename=&pTmpBuf[SIZE_RSC_FILES_MAX];	

	sum=0;
	rsc_file=-1;
	offset_in_rsc=-1;
	for (i=0;i<MAX_NUM_RSC_FILES;i++)
	{
		if (sum<=offset)
		{
			offset_in_rsc=offset-sum;
			rsc_file=i;
		}
	}
	if (rsc_file==-1 || orrset_in_rsc==-1)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	outputidx=0;
	while (length)
	{
		int n;
		retval=dMagnetic2_loader_mw_substitute_tworsc(filename1,pFilename,rsc_file,NULL);
		f=fopen(pFilename,"rb");
		fseek(f,offset_in_rsc,SEEK_SET);
		n=fread(&pOutput[output_idx],sizeof(char),length,f);
		fclose(f);

		// keep on reading in the next file, if necessary
		length=length-n;
		output_idx+=n;
		rsc_file++;
		offset_in_rsc=0;
	}
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_mw(
		char* filename1,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,int* pRealMagSize,
		unsigned char* pGfxBuf,int* pRealGfxSize,
		tdMagnetic2_game_meta *pMeta)
{
	char *pFilename;
	int sizes[MAX_NUM_RSC_FILES];
	int retval;
	int gameidx;

	if (tmpsize<SIZE_RSC_FILE+FILENAME_LENGTH_MAX)
	{
		return DMAGNETIC2_ERROR_BUFFER_TOO_SMALL;
	}
	if (pMeta==NULL || pTmpBuf==NULL)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}

	pMeta->game=DMAGNETIC2_GAME_NONE;
	pMeta->source=DMAGNETIC2_SOURCE_NONE;
	pMeta->gamename[0]=0;
	pMeta->sourcename[0]=0;
	pMeta->version=-1;
	pMeta->real_magsize=0;
	pMeta->real_gfxsize=0;


	// first: identify the game. and collect the size of the .rsc files.
	retval=dMagnetic2_loader_mw_sizes(pTmpBuf,filename1,&sizes,&gameidx);	
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}
	pMeta->game=dMagnetic2_loader_mw_gameinfo[gameidx].game;
	pMeta->version=4;		// magnetic windows has always this verion
	pMeta->source=DMAGNETIC_SOURCE_MW;

	return DMAGNETIC2_OK;
}
