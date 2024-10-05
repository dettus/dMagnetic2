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


#define	MAX_FILENAME_LEN	1024
#define	MAX_SIZE_DISK1		209529
#define	MAX_SIZE_DISK2		359780
#define	MAX_GAMEFILE_SIZE	209529
#define	KNOWN_GAMES		6


#define	MAX_SIZE_CODE		65536	// actually, jinxter has 65472 bytes
#define	MAX_SIZE_STRING1	91000	// actually, corruption has 90307 bytes
#define	MAX_SIZE_STRING2	4000	// actually, corruption has 3900 bytes
#define	MAX_SIZE_DICT		8000	// actually, jinxter has 7281 bytes

#define	MAX_SIZE_FILE5		28000	// actually, myth has 27205 bytes
#define	MAX_SIZE_FILE6		25000	// actually, guild has 24116 bytes

typedef struct _tGameInfo
{
	edMagnetic2_game	game;	// the enumeration for the game
	char prefix[8];	// the prefix for the game's binaries.
	int disk1size;	// the size of the DISK1.PIX file is in an indicator for the game being used.
	int disk2size;	// the size of the DISK1.PIX file is in an indicator for the game being used.
	int version;	// the interpreter version.
} tGameInfo;


// The others have some opcodes which I can not decode (yet)
#define	KNOWN_GAMES	6
const tGameInfo dMagnetic2_loader_msdos_gameInfo[KNOWN_GAMES]={
	{
		.game=DMAGNETIC2_GAME_PAWN,
		.version=0,
		.prefix="PAWN",
		.disk1size=209529,
		.disk2size=359780,
	},	// THE PAWN
	{
		.game=DMAGNETIC2_GAME_GUILD,
		.version=1,
		.prefix="GUILD",
		.disk1size=185296,
		.disk2size=355797,
	},	// THE GUILD OF THIEVES
	{
		.game=DMAGNETIC2_GAME_JINXTER,
		.version=2,
		.prefix="JINX",
		.disk1size=159027,
		.disk2size=359251,
	},	// JINXTER
	{
		.game=DMAGNETIC2_GAME_CORRUPTION,
		.version=3,
		.prefix="CORR",
		.disk1size=160678,
		.disk2size=359633,
	},	// CORRUPTION
	{
		.game=DMAGNETIC2_GAME_FISH,
		.version=3,
		.prefix="FILE",
		.disk1size=162541,
		.disk2size=352510,
	},	// FISH
	{
		.game=DMAGNETIC2_GAME_MYTH,
		.version=3,
		.prefix="FILE",
		.disk1size=67512,
		.disk2size=0
	}	// MYTH
};



int dMagnetic2_loader_msdos_identify_game(int disk1size)
{
	int i;
	int gameidx;
	// detecting the game is pretty straightforward:
	// the pictures are stored in a file called "DISK1.PIX".
	// the size of this file varies with the game.

	gameidx=-1;
	for (i=0;i<KNOWN_GAMES;i++)
	{
		if (disk1size==dMagnetic2_loader_msdos_gameInfo[i].disk1size) gameidx=i;
	}
	return gameidx;
}

int dMagnetic2_loader_msdos_mkmag(char* filename1,unsigned char* pTmpBuf,unsigned char* pMagBuf,tdMagnetic2_game_meta *pMeta,int gameidx,char filename_postfix,int nodoc)
{
	int size_code=0;
	int size_dict=0;
	int size_string1=0;
	int size_string2=0;
	int idx;
	int n;
	FILE *f;

	idx=42;		// leave some room for the header


	// start by reading the code section in the file ending with 1
	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s1%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
	// start by reading the index
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
	}
	n=fread(pTmpBuf,sizeof(char),MAX_SIZE_CODE,f);
	fclose(f);
	// sometimes this file is huffed. if not, it starts with 0x49 0xfa
	if (pTmpBuf[0]==0x49 && pTmpBuf[1]==0xfa)
	{
		memcpy(&pMagBuf[idx],pTmpBuf,n);
		size_code=n;	
	} else {
		size_code=dMagnetic2_loader_shared_unhuffer(pTmpBuf,n,&pMagBuf[idx]);
	}
	idx+=size_code;

	// the strings are stores in the files ending with 3 and 2
	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s3%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
	// start by reading the index
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
	}
	size_string1=fread(&pMagBuf[idx],sizeof(char),MAX_SIZE_STRING1,f);
	fclose(f);
	idx+=size_string1;

	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s2%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
	// start by reading the index
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
	}
	size_string2=fread(&pMagBuf[idx],sizeof(char),MAX_SIZE_STRING2,f);
	fclose(f);
	idx+=size_string2;

	size_dict=0;
	if (dMagnetic2_loader_msdos_gameInfo[gameidx].version>=2)	// directories for those games are huffed
	{
		snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s0%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
		// start by reading the index
		f=fopen(pTmpBuf,"rb");
		if (f==NULL)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(pTmpBuf,sizeof(char),MAX_SIZE_CODE,f);
		size_dict=dMagnetic2_loader_shared_unhuffer(pTmpBuf,n,&pMagBuf[idx]);
	}	
	idx+=size_dict;

	if (nodoc)
	{
		int i;
		unsigned char* ptr=(unsigned char*)&pMagBuf[0];
		for (i=0;i<idx-4;i++)
		{
			if (ptr[i+0]==0x62 && ptr[i+1]==0x02 && ptr[i+2]==0xa2 && ptr[i+3]==0x00) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
			if (ptr[i+0]==0xa4 && ptr[i+1]==0x06 && ptr[i+2]==0xaa && ptr[i+3]==0xdf) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
		}
	}
	if (dMagnetic2_loader_msdos_gameInfo[gameidx].game==DMAGNETIC2_GAME_MYTH && pMagBuf[0x314a]==0x66) pMagBuf[0x314a]=0x60;	// final touch
	dMagnetic2_loader_shared_addmagheader(pMagBuf,idx,dMagnetic2_loader_msdos_gameInfo[gameidx].version,size_code,size_string1,size_string2,size_dict,-1);
	pMeta->real_magsize=idx;	

	return DMAGNETIC2_OK;
}
	

int dMagnetic2_loader_msdos_mkgfx(char* filename1,unsigned char* pTmpBuf,unsigned char* pGfxBuf,tdMagnetic2_game_meta *pMeta,int gameidx,char filename_postfix)
{
	int n;
	int idx;
	int size_index;
	int size_disk1;
	int size_disk2;
	FILE *f;

	/////////////////////// GFX packing
	// the header of the GFX is always 16 bytes.
	// values are stored as BigEndians
	//  0.. 3 are the magic word 'MaP3'
	//  4.. 7 are the size of the GAME4 (index) file (always 256)
	//  8..11 are the size of the DISK1.PIX file
	// 12..15 are the size of the DISK2.PIX file
	// then the INDEX file (beginning at 16)
	// then the DISK1.PIX file
	// then the DISK2.PIX file

	idx=0;
	pGfxBuf[idx++]='M';pGfxBuf[idx++]='a';pGfxBuf[idx++]='P';pGfxBuf[idx++]='3';
	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s4%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
	// start by reading the index
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
	}
	n=fread(&pGfxBuf[idx],sizeof(char),256+1,f);	
	fclose(f);
	if (n!=256)		// check if the expected filesize matches
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	idx+=n;
	size_index=n;


	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/DISK1.PIX",filename1);
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
	}
	n=fread(&pGfxBuf[idx],sizeof(char),dMagnetic2_loader_msdos_gameInfo[gameidx].disk1size+1,f);
	fclose(f);
	if (n!=dMagnetic2_loader_msdos_gameInfo[gameidx].disk1size+1)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	idx+=n;
	size_disk1=n;


	size_disk2=0;
	if (dMagnetic2_loader_msdos_gameInfo[gameidx].disk2size)	// MYTH stores its pictures in a single file.
	{
		snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/DISK2.PIX",filename1);
		f=fopen(pTmpBuf,"rb");
		if (f==NULL)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(&pGfxBuf[idx],sizeof(char),dMagnetic2_loader_msdos_gameInfo[gameidx].disk2size+1,f);
		fclose(f);
		if (n!=dMagnetic2_loader_msdos_gameInfo[gameidx].disk2size+1)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		idx+=n;
		size_disk2=n;
	}

	// TODO: FIND OUT WHAT THE FILES WITH THE ENDING 5 and 6 ARE. I AM GUESSING THEY ARE THE TITLE IMAGES
	{
		int idx0;
		int size_file5;
		int size_file6;

		idx0=idx;

		size_file5=0;
		size_file6=0;
		
		// leave a little room here
		idx+=16;
		// TODO: the title screen is stored in the file with the ending 5 and 6 (I think...)
		snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s5%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
		// start by reading the index
		f=fopen(pTmpBuf,"rb");
		if (f!=NULL)
		{
			size_file5=fread(&pGfxBuf[idx],sizeof(char),MAX_SIZE_FILE5,f);
			fclose(f);
		}
		idx+=size_file5;

		snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s6%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
		// start by reading the index
		f=fopen(pTmpBuf,"rb");
		if (f!=NULL)
		{
			size_file6=fread(&pGfxBuf[idx],sizeof(char),MAX_SIZE_FILE6,f);	
			fclose(f);
		}
		idx+=size_file6;

		WRITE_INT32BE(pGfxBuf,idx0+0,size_file5);
		WRITE_INT32BE(pGfxBuf,idx0+4,size_file6);
	}

	WRITE_INT32BE(pGfxBuf, 4,size_index);
	WRITE_INT32BE(pGfxBuf, 8,size_disk1);
	WRITE_INT32BE(pGfxBuf,12,size_disk2);

	pMeta->real_gfxsize=idx;

	return DMAGNETIC2_OK;

}

int dMagnetic2_loader_msdos(
		char* filename1,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,int* pRealMagSize,
		unsigned char* pGfxBuf,int* pRealGfxSize,
		tdMagnetic2_game_meta *pMeta,
		int nodoc)
{
	int n;
	int gameidx;
	char filename_postfix;	// some releases of the games have a . at the end of the filename. this can cause problems on some unix systems. 
	FILE *f;
	// check the important output buffers
//	if (tmpsize<MAX_FILENAME_LEN)	
	if (tmpsize<MAX_SIZE_CODE)	
	{
		return DMAGNETIC2_ERROR_BUFFER_TOO_SMALL;
	}
	if (pMeta==NULL || pTmpBuf==NULL)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}

	filename_postfix=0;	// start by assuming that the filenames do not end with a .

	pMeta->game=DMAGNETIC2_GAME_NONE;
	pMeta->source=DMAGNETIC2_SOURCE_NONE;
	pMeta->gamename[0]=0;
	pMeta->sourcename[0]=0;
	pMeta->version=-1;
	pMeta->real_magsize=0;
	pMeta->real_gfxsize=0;

	// for msdos, the game is stored in a directory. 
	// the directory has files. 
	// todo: check if filename1 is a directory.
	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/DISK1.PIX",filename1);
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	fseek(f,0L,SEEK_END);
	n=(int)ftell(f);	// check the filesize
	fclose(f);

	gameidx=dMagnetic2_loader_msdos_identify_game(n);
	if (gameidx==-1)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	// next step: check if the particular release comes with a . at the end of filenames.
	filename_postfix=0;		// lets assume that they do not.
	snprintf(pTmpBuf,MAX_FILENAME_LEN-1,"%s/%s4%c",filename1,dMagnetic2_loader_msdos_gameInfo[gameidx].prefix,filename_postfix);
	// try to open file file
	f=fopen(pTmpBuf,"rb");
	if (f==NULL)	// unable to open the file. must be because it ended with a .
	{
		filename_postfix='.';	// lets assume that all files which need to be read have this postfix.
	} else {
		fclose(f);
	}



	pMeta->game=dMagnetic2_loader_msdos_gameInfo[gameidx].game;
	pMeta->source=DMAGNETIC2_SOURCE_MSDOS;
	pMeta->version=dMagnetic2_loader_msdos_gameInfo[gameidx].version;

	// the game has been identified. The game sections are actually stored in files. 
	// those files can be opened and read.
	if (pMagBuf!=NULL)
	{
		int retval;
		retval=dMagnetic2_loader_msdos_mkmag(filename1,pTmpBuf,pGfxBuf,pMeta,gameidx,filename_postfix,nodoc);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}
	if (pGfxBuf!=NULL)
	{
		int retval;
		retval=dMagnetic2_loader_msdos_mkgfx(filename1,pTmpBuf,pGfxBuf,pMeta,gameidx,filename_postfix);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}
	return DMAGNETIC2_OK;
}
