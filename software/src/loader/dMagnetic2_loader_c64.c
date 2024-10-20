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


// the purpose of this loader is to load the game from .D64 image files.
#define D64_IMAGESIZE   174848
#define D64_TRACKNUM    40
#define D64_SECTORSIZE  256
#define D64_MAXENTRIES  64
#define D64_BITMASKSIZE 6080    // pictures have a resolution of 160x152 pixels


// some information about the .d64 image format
const unsigned char dMagnetic2_loader_c64_sectorcnt[D64_TRACKNUM]=
{
	21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,     // track 1-17
	19,19,19,19,19,19,19,   // track 18-24
	18,18,18,18,18,18,      // track 25-30
	17,17,17,17,17,17,17,17,17,17   // track 31-40
};


typedef enum _eFileType
{
	TYPE_UNKNOWN,
	TYPE_CODE1,
	TYPE_CODE2,
	TYPE_CODE1_ENCRYPTED,
	TYPE_CODE2_ENCRYPTED,
	TYPE_STRING1,
	TYPE_STRING2,
	TYPE_DICTIONARY,
	TYPE_PICTURE,
	TYPE_CAMEO
} eFileType;

typedef struct _tFileEntry
{
	unsigned int offset;    // where in the image is the file?
	unsigned char track;    // track
	unsigned char sector;   // sector
	int len;                // do not trust this one
	int side;               // A, B or both?
	eFileType	fileType;
} tFileEntry;


typedef struct _tGameInfo
{
	edMagnetic2_game	game;	// the enumeration for the game
	int sides;			// how many floppy sides did this game occupy?
	char magicword[5];		// the word, that is hidden in the second sector of the disk image
	int version;			// the virtual machine's version
	int pictureorder[32];		// the order in which the pictures can be found in the images are not the same as in other releases.
} tGameInfo;

const tGameInfo dMagnetic2_loader_c64_gameinfo[6]=
{
								// the pictures are ordered different from the other releases. there, this list would have been 0 1 2 3 4...
								// but the C64 had only a limited amount of floppy disk space.
	{		// gamename="The Pawn",
		.version=0,
		.game=DMAGNETIC2_GAME_PAWN,
		.sides=2,
		.magicword="PAWN",
		.pictureorder={ 4,26,13,23, 0, 8,29, 5, 18,19, 3, 9, 12,11,16,22, 17,21,28, 6, 27,25,24, 2, 1,20,14, 7, 15,10,-1,-1 }
	},
	{		// gamename="The Guild of Thieves",
		.version=1,
		.game=DMAGNETIC2_GAME_GUILD,
		.sides=2,
		.magicword="SWAG",
		.pictureorder={ 9,17,20, 0,26,19,11,12, 4, 5, 2,13,14, 8, 6, 1,15,16, 3,24,21,28,22,25,18,23, 7,10,27,-1,-1,-1}
	},
	{		// gamename="Jinxter",			 
		.version=2,
		.game=DMAGNETIC2_GAME_JINXTER,
		.sides=2,
		.magicword="ARSE",
		.pictureorder={ 4, 0, 5, 6, 7,-1, 8, 1, 9,10,11,12, 13,14,15,16, 17, 2, 3,27, 18,19,20,21, 22,23,24,25, 26,27,26,26}
	},
	{		// gamename="Corruption",
		.version=3,
		.game=DMAGNETIC2_GAME_CORRUPTION,
		.sides=2,
		.magicword="COKE",
		.pictureorder={ 24, 8, 9,25, 10,13,15,16, 17, 1,18,23, 21, 6, 5, 4, 12,14, 2, 3, 11,20, 7,22, 19, 0,-1,-1, -1,-1,-1,-1 }
	},
	{		// gamename="Fish!",
		.version=3,
		.game=DMAGNETIC2_GAME_FISH,
		.sides=2,
		.magicword="GLUG",
		.pictureorder={ 3,21, 8,11, 18,16,17, 4, 2, 5, 1, 6, 9,10,14,20, 22,24,25, 0, 15,23, 7,19, 13,12,26,-1, -1,-1,-1,-1 }
	},
	{		//gamename="Myth",
		.version=3,
		.game=DMAGNETIC2_GAME_MYTH,
		.sides=1,
		.magicword="GODS",
		.pictureorder={ 0, 1, 2, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
	}
};

edMagnetic2_game dMagnetic2_loader_c64_detectgame(unsigned char* diskram,tdMagnetic2_game_meta *pMeta,int *pSidecnt_expected,const int **ppPictureorder)
{

	// find the magic word
	int i;
	char tmp[5];
#define	MAGIC_WORD_LOCATION	0x1eb
	// at those positions in the image, a magic word is hidden.
	// it can be used to detect the game
	tmp[0]=diskram[MAGIC_WORD_LOCATION+0];
	tmp[1]=diskram[MAGIC_WORD_LOCATION+1];
	tmp[2]=diskram[MAGIC_WORD_LOCATION+2];
	tmp[3]=diskram[MAGIC_WORD_LOCATION+3];
	tmp[4]=0;

	for (i=0;i<6;i++)
	{
		if (strncmp(tmp,dMagnetic2_loader_c64_gameinfo[i].magicword,4)==0) 
		{
			pMeta->game=dMagnetic2_loader_c64_gameinfo[i].game;
			pMeta->source=DMAGNETIC2_SOURCE_C64;
			pMeta->version=dMagnetic2_loader_c64_gameinfo[i].version;
			*pSidecnt_expected=dMagnetic2_loader_c64_gameinfo[i].sides;
			*ppPictureorder=dMagnetic2_loader_c64_gameinfo[i].pictureorder;
			return DMAGNETIC2_OK;
		}
	}

	return DMAGNETIC2_UNKNOWN_SOURCE;
}
int dMagnetic2_loader_c64_readEntries(unsigned char* d64image,int d64size,tFileEntry* pEntries,int* pEntrynum)
{
// this function is reading the "directory" 
	int i;
	int offsets[D64_TRACKNUM];
	int cnt;
#define	DIRECTORY_START		(2*D64_SECTORSIZE)	// the "directory" is in the first track, in sector 2

	// first step: calculate the offsets for the tracks
	offsets[0]=0;
	for (i=1;i<D64_TRACKNUM;i++)
	{
		offsets[i]=offsets[i-1]+dMagnetic2_loader_c64_sectorcnt[i-1]*D64_SECTORSIZE;
	}
	cnt=0;
	// read the entries, and translate it into offsets within the d64 image
	for (i=0;i<D64_MAXENTRIES;i++)
	{
		unsigned char track;
		unsigned char sect;
		unsigned char side;
		unsigned char len;
		int newoffs;

		// 1, entry, 4 bytes. in sector 2.
		track=d64image[2*D64_SECTORSIZE+i*4+0];
		sect =d64image[2*D64_SECTORSIZE+i*4+1];
		len  =d64image[2*D64_SECTORSIZE+i*4+2];
		side =d64image[2*D64_SECTORSIZE+i*4+3]&3;
	
		// if the "track" is after the end of the image, this image was not a proper image	
		if (track>=D64_TRACKNUM)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		
		if (len)
		{
			newoffs=offsets[track]+sect*D64_SECTORSIZE;
			pEntries[cnt].offset=newoffs;
			pEntries[cnt].track=track;
			pEntries[cnt].sector=sect;
			pEntries[cnt].len=len;
			pEntries[cnt].side=side;
			pEntries[cnt].fileType=TYPE_UNKNOWN;

			cnt++;
		}
	}
	*pEntrynum=cnt;
	return DMAGNETIC2_OK;
}
void dMagnetic2_loader_c64_readSector(unsigned char* d64image,int track,int sect,unsigned char* buf)
{
	int i;
	int offset;

	// first step: calculate the offsets for the tracks
	offset=sect*D64_SECTORSIZE;
	for (i=1;i<track;i++)
	{
		offset+=dMagnetic2_loader_c64_sectorcnt[i-1]*D64_SECTORSIZE;
	}
	if ((offset+256)>D64_IMAGESIZE)
	{
		return;
	}
	for (i=0;i<256;i++) buf[i]=d64image[offset++];
}



void dMagnetic2_loader_c64_identifyEntries(unsigned char* d64image,tFileEntry* pEntries,int entryNum,int diskcnt)
{
	int i;
	unsigned char tmp1[256]={0};
	unsigned char tmp2[256]={0};
	int sideoffsets[4]={-1,-1,-1,-1};

	// some file types have a fixed position in the list
	pEntries[2].fileType=TYPE_STRING1;
	pEntries[3].fileType=TYPE_STRING2;
	pEntries[4].fileType=TYPE_CAMEO;
	// first: find the code block
	for (i=0;i<entryNum;i++)
	{
		dMagnetic2_loader_c64_readSector(&d64image[0],pEntries[i].track,pEntries[i].sector,tmp1);		// read from the first image
		if (diskcnt==2)
		{
			dMagnetic2_loader_c64_readSector(&d64image[D64_IMAGESIZE],pEntries[i].track,pEntries[i].sector,tmp2);	// and from the second one
		}

		if (tmp1[0]==0x49 && tmp1[1]==0xfa)	// 0x49fa is ALWAYS the first instruction.
		{
			pEntries[i].fileType=TYPE_CODE1;
			pEntries[1].fileType=TYPE_CODE2;
			pEntries[i+1].fileType=TYPE_DICTIONARY;
			sideoffsets[pEntries[i].side]=0;
		} 
		else if (tmp2[0]==0x49 && tmp2[1]==0xfa)	// 0x49fa is ALWAYS the first instruction.
		{
			pEntries[i].fileType=TYPE_CODE1;
			pEntries[1].fileType=TYPE_CODE2;
			pEntries[i+1].fileType=TYPE_DICTIONARY;
			sideoffsets[pEntries[i].side]=D64_IMAGESIZE;
		} else {
			dMagnetic2_loader_shared_descramble(tmp1,tmp1,0,NULL,0);
			dMagnetic2_loader_shared_descramble(tmp2,tmp2,0,NULL,0);
			if ((tmp1[0]==0x49 || tmp1[2]==0x49) && (tmp1[1]==0xfa || tmp1[3]==0xfa))	// 0x49fa is ALWAYS the first instruction. run level encoded files start with a 2 byte header.
			{
				pEntries[i].fileType=TYPE_CODE1_ENCRYPTED;
				pEntries[1].fileType=TYPE_CODE2_ENCRYPTED;
				pEntries[i+1].fileType=TYPE_DICTIONARY;
				sideoffsets[pEntries[i].side]=0;
			} 
			if ((tmp2[0]==0x49 || tmp2[2]==0x49) && (tmp2[1]==0xfa || tmp2[3]==0xfa))	// 0x49fa is ALWAYS the first instruction. run level encoded files start with a 2 byte header.
			{
				pEntries[i].fileType=TYPE_CODE1_ENCRYPTED;
				pEntries[1].fileType=TYPE_CODE2_ENCRYPTED;
				pEntries[i+1].fileType=TYPE_DICTIONARY;
				sideoffsets[pEntries[i].side]=D64_IMAGESIZE;
			}
		}
	}
	if (sideoffsets[1]==-1)		sideoffsets[1]=D64_IMAGESIZE-sideoffsets[2];
	else if (sideoffsets[2]==-1)	sideoffsets[2]=D64_IMAGESIZE-sideoffsets[1];
	sideoffsets[0]=0;
	sideoffsets[3]=0;

	// step2: identify all the pictures
	// maybe there is a proper way to determine it from the data. through some form of directory i have not found yet.
	// so here is what i do: I just check if the sector starts with the sequence 3d 82 81... or 3e 82 81, heralding
	// the size of the huffman tree and the first two branches which are never a terminal symbol.
	// (at least in the releases I dealt with)
	for (i=0;i<entryNum;i++)
	{
		pEntries[i].offset+=sideoffsets[pEntries[i].side];
		if (pEntries[i].fileType==TYPE_UNKNOWN)
		{
			dMagnetic2_loader_c64_readSector(&d64image[sideoffsets[pEntries[i].side]],pEntries[i].track,pEntries[i].sector,tmp1);
			if ((tmp1[0]==0x3d || tmp1[0]==0x3e) && tmp1[1]==0x82 && tmp1[2]==0x81) pEntries[i].fileType=TYPE_PICTURE;
		}
	}
}

void dMagnetic2_loader_c64_advanceSector(int *pTrack,int *pSector)
{
	int sect;
	int track;

	track=*pTrack;
	sect=*pSector;
	sect=sect+1;
	if (sect==dMagnetic2_loader_c64_sectorcnt[track-1])
	{
		sect=0;
		track++;
		if (track==18) track+=1;        // skip the directory track
	}
	*pTrack=track;
	*pSector=sect;
}


// The "code" section of the game is broken into two parts on the C64: The first one is being kept in memory.
int dMagnetic2_loader_c64_readCode1(unsigned char* d64image,tFileEntry *pEntries,int entryNum,unsigned char* pCode1Buf,int* pCode1Size)
{
	int i;
	int j;
	int outcnt;
	int rle;
	int inputsize;
	int track;
	int sect;
	int len;
	int offset;
	int encrypted;
	unsigned char tmp[256];

	outcnt=0;
	offset=0;
	encrypted=0;

	track=sect=len=0;
	for (i=0;i<entryNum;i++)
	{
		if (pEntries[i].fileType==TYPE_CODE1 || pEntries[i].fileType==TYPE_CODE1_ENCRYPTED)
		{
			if (pEntries[i].fileType==TYPE_CODE1_ENCRYPTED) encrypted=1;
			if (pEntries[i].offset>=D64_IMAGESIZE) offset=D64_IMAGESIZE;

			track=pEntries[i].track;
			sect =pEntries[i].sector;
			len  =pEntries[i].len;
		}
	}

	rle=0;
	inputsize=len*D64_SECTORSIZE;
	for (i=0;i<len;i++)
	{
		int start;
		dMagnetic2_loader_c64_readSector(&d64image[offset],track,sect,tmp);
		dMagnetic2_loader_c64_advanceSector(&track,&sect);
		start=0;
		if (encrypted)
		{
			dMagnetic2_loader_shared_descramble(tmp,tmp,i,NULL,0);	// the first part of the game is "encrypted"
		}
		if (i==0)
		{
			if (tmp[0]==0x49 && tmp[1]==0xfa) 	// some games are packed. some are not.
			{
				rle=0;
			} else {
				inputsize=READ_INT16BE(tmp,0)-2;	// when the game is packed, the first 16 bit are the size 
				rle=1;
				start=2;
			}
		}
		for (j=start;j<D64_SECTORSIZE && inputsize;j++,inputsize--)
		{
			if (rle==2)
			{
				int k;
				for (k=0;k<tmp[j]-1;k++) pCode1Buf[outcnt++]=0;
				rle=1;
			} else {
				pCode1Buf[outcnt++]=tmp[j];
			}
			if (tmp[j]==0x00 && rle==1)
			{
				rle=2;
			}
		}
	}
	*pCode1Size=outcnt;
	return DMAGNETIC2_OK;
}

// The "code" section of the game is broken into two parts on the C64: The second one was "swapped in" when it was needed.
int dMagnetic2_loader_c64_readCode2(unsigned char* d64image,tFileEntry *pEntries,int entryNum,unsigned char* pCode2Buf,int* pCode2Size)
{
	int i;
	int j;
	int outcnt;
	int track,sect,len;
	int offset;
	int encrypted;
	int scrambleoffs;
	unsigned char tmp[256];

	track=sect=len=0;
	offset=0;
	encrypted=0;
	scrambleoffs=0;
	for (i=0;i<entryNum;i++)
	{
		if (pEntries[i].fileType==TYPE_CODE1_ENCRYPTED)
		{
			scrambleoffs=pEntries[i].len;
		}
		if (pEntries[i].fileType==TYPE_CODE2 || pEntries[i].fileType==TYPE_CODE2_ENCRYPTED)
		{
			if (pEntries[i].fileType==TYPE_CODE2_ENCRYPTED) encrypted=1;
			if (pEntries[i].offset>=D64_IMAGESIZE) offset=D64_IMAGESIZE;
			track=pEntries[i].track;
			sect =pEntries[i].sector;
			len  =pEntries[i].len;
		}
	}


	outcnt=0;
	for (i=0;i<len;i++)
	{
		dMagnetic2_loader_c64_readSector(&d64image[offset],track,sect,tmp);
		if (encrypted) dMagnetic2_loader_shared_descramble(tmp,tmp,i+scrambleoffs,NULL,0);
		dMagnetic2_loader_c64_advanceSector(&track,&sect);
		for (j=0;j<256;j++)
		{
			pCode2Buf[outcnt++]=tmp[j];
		}
	}
	*pCode2Size=outcnt;
	return 0;
}


int dMagnetic2_loader_c64_readStrings(unsigned char* d64image,tFileEntry* pEntries,int entryNum,unsigned char* pStringBuf,int* string1size,int* string2size,int* dictsize)
{
	int i;
	int j;
	int k;
	int outcnt;
	unsigned char tmp[256]={0};
	int cnt[3]={0};
	int number=0;
	int encrypted;


	outcnt=0;
	for (i=0;i<entryNum;i++)
	{

		if (pEntries[i].fileType==TYPE_STRING1 || pEntries[i].fileType==TYPE_STRING2 || pEntries[i].fileType==TYPE_DICTIONARY)
		{
			int track,sect,len,offset;
			encrypted=0;
			track=pEntries[i].track;
			sect =pEntries[i].sector;
			len  =pEntries[i].len;
			if (pEntries[i].fileType==TYPE_DICTIONARY)
			{
				encrypted=1;
			}
			if (pEntries[i].offset>=D64_IMAGESIZE) 
			{
				offset=D64_IMAGESIZE;
			} else {
				offset=0;
			}

			for (j=0;j<len;j++)
			{
				dMagnetic2_loader_c64_readSector(&d64image[offset],track,sect,tmp);
				if (encrypted) dMagnetic2_loader_shared_descramble(tmp,tmp,j,NULL,0);
				dMagnetic2_loader_c64_advanceSector(&track,&sect);
				for (k=0;k<256;k++)
				{
					pStringBuf[outcnt++]=tmp[k];
				}
				cnt[number]+=256;
			}
			number++;
		}
	}
	*string1size	=cnt[0];
	*string2size	=cnt[1];
	*dictsize	=cnt[2];
	return DMAGNETIC2_OK;
}
int dMagnetic2_loader_c64_getsize(int *pBytes)
{
// should be large enough for a disk image. and a spare byte for a trick to determine the correct file size	
	*pBytes=2*D64_IMAGESIZE+1;// should be large enough for two disk images. and a spare byte for a trick to determine the correct file size
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_c64(
		char* filename1,char* filename2,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,
		unsigned char* pGfxBuf,
		tdMagnetic2_game_meta *pMeta,
		int nodoc)
{
	int i;
	int l;
	unsigned char *d64image;
	int sidecnt_is;
	int sidecnt_expected;
	int entryNum;
	int retval;
	tFileEntry entries[D64_MAXENTRIES];
	FILE *f;
	int code1size,code2size,string1size,string2size,dictsize;
	const int *pPictureorder;


	// check the important output buffers
	if (tmpsize<2*D64_IMAGESIZE+1)	// should be large enough for two disk images. and a spare byte for a trick to determine the correct file size
	{
		return DMAGNETIC2_ERROR_BUFFER_TOO_SMALL;
	}
	if (pMeta==NULL || pTmpBuf==NULL)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}

	pMeta->game=DMAGNETIC2_GAME_NONE;
	pMeta->source=DMAGNETIC2_SOURCE_NONE;
	pMeta->version=-1;
	pMeta->real_magsize=0;
	pMeta->real_gfxsize=0;



	d64image=(unsigned char*)&pTmpBuf[0];
	if (filename1==NULL)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	sidecnt_is=2;		// most games have two sides
	// load the first image
	{
		f=fopen(filename1,"rb");
		if (!f)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		l=fread(&d64image[0*D64_IMAGESIZE],sizeof(char),D64_IMAGESIZE+1,f);	// this is a trick to read the buffer AND to check if the filesize is correct
		fclose(f);
		if (l!=D64_IMAGESIZE)	// in case there were more or less bytes in the file than the exact size of a .D64 image,
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;	// it cannot be a .D64 file. So not a C64 game.
		}
	}

	// load the second image
	if (filename2==NULL)	
	{
		sidecnt_is=1;// Myth has actually just one disk image.
	} else {
		f=fopen(filename2,"rb");
		if (!f)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		l=fread(&d64image[1*D64_IMAGESIZE],sizeof(char),D64_IMAGESIZE+1,f);	// this is a trick to read the buffer AND to check if the filesize is correct
		fclose(f);
		if (l!=D64_IMAGESIZE)	// in case there were more or less bytes in the file than the exact size of a .D64 image,
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;	// it cannot be a .D64 file. So not a C64 game.
		}
	}
	// now the images are in memory. now we can parse them.
	// first, find out which game it is
	retval=dMagnetic2_loader_c64_detectgame(d64image,pMeta,&sidecnt_expected,&pPictureorder);
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	if (sidecnt_expected!=sidecnt_is)
	{
		return DMAGNETIC2_MISSING_IMAGE;
	}

	// then, find the entries in the file list
	retval=dMagnetic2_loader_c64_readEntries(d64image,D64_IMAGESIZE,entries,&entryNum);
	if (retval==DMAGNETIC2_UNKNOWN_SOURCE)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	dMagnetic2_loader_c64_identifyEntries(d64image,entries,entryNum,sidecnt_is);	// and figure out if they are code, pictures or something else


	if (pMagBuf!=NULL)
	{
		int huffmantreeidx;
		int magidx;
		////////////////// LOAD THE MAG BUFFER /////////////////
		magidx=42;	// leave some room for the header
		dMagnetic2_loader_c64_readCode1(d64image,entries,entryNum,(unsigned char*)&pMagBuf[magidx],&code1size);
		magidx+=code1size;
		dMagnetic2_loader_c64_readCode2(d64image,entries,entryNum,(unsigned char*)&pMagBuf[magidx],&code2size);
		magidx+=code2size;
		dMagnetic2_loader_c64_readStrings(d64image,entries,entryNum,(unsigned char*)&pMagBuf[magidx],&string1size,&string2size,&dictsize);
		huffmantreeidx=0;

		// within the string buffer, there is the beginning of the huffman tree
		if (pMeta->version<=1) 
		{
			huffmantreeidx=string1size;		// the PAWN and GUILD of Thieves had the beginning of the huffman tree in the second string buffer.
		} else {
			int i;
			// there might be a better way to look for it, but in every C64 game, it starts with 01 02 03.
			// and before that, there are a handful of bytes that are =0.
			// in addition to this, it is aligned to sectors, so its indes has to be a multiple of 256.
			for (i=0x100;i<string1size+string2size;i+=0x100)
			{
				if (	pMagBuf[magidx+i-3]==0x00 && pMagBuf[magidx+i-2]==0x00 && pMagBuf[magidx+i-1]==0x00 &&			// the previous sector ends with 0x00
					pMagBuf[magidx+i+0]==0x01 && pMagBuf[magidx+i+1]==0x02 && pMagBuf[magidx+i+2]==0x03) huffmantreeidx=i;	// the sector with the huffmann tree starts with 0x01 0x02 0x03
			}
		}

		magidx+=string1size+string2size+dictsize;
		dMagnetic2_loader_shared_addmagheader(pMagBuf,magidx,pMeta->version,code1size+code2size,string1size,string2size,dictsize,huffmantreeidx);

		if (nodoc)
		{
			int i;
			unsigned char* ptr=(unsigned char*)&pMagBuf[0];
			for (i=0;i<magidx-4;i++)
			{
				if (ptr[i+0]==0x62 && ptr[i+1]==0x02 && ptr[i+2]==0xa2 && ptr[i+3]==0x00) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
				if (ptr[i+0]==0xa4 && ptr[i+1]==0x06 && ptr[i+2]==0xaa && ptr[i+3]==0xdf) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
			}
		}
		if (pMeta->game==DMAGNETIC2_GAME_MYTH && pMagBuf[0x3080]==0x66) pMagBuf[0x3080]=0x60;	// final touch

		pMeta->real_magsize=magidx;
		/////////// MAG IS FINISHED ////////////////////////////////
	}

	if (pGfxBuf!=NULL)
	{
		unsigned int picoffs[32]={0};
		int side;
		int piccnt;
		int gfxidx;



		side=-1;
		piccnt=0;

		////////////// LOAD THE GFX BUFFER /////////////////
		gfxidx=4+4*32+1;	// leave some room for the magic word, offsets and the game's version

		// just copy the picture data into the GFX buffer. It will be parsed and unpacked later.
		for (i=0;i<entryNum;i++)
		{
			if (entries[i].fileType==TYPE_PICTURE)
			{
				int track,sector,offset,len;
				int j;
				if (side==-1)
				{
					side=entries[i].side;
				}
				else if (side!=entries[i].side)
				{
					side=entries[i].side;
				}
				track	=entries[i].track;
				sector	=entries[i].sector;
				offset	=entries[i].offset;if (offset>=D64_IMAGESIZE) offset=D64_IMAGESIZE; else offset=0;
				len	=entries[i].len;

				picoffs[piccnt]=gfxidx;
				for (j=0;j<len && track<36;j++)
				{
					dMagnetic2_loader_c64_readSector(&d64image[offset],track,sector,(unsigned char*)&pGfxBuf[gfxidx]);
					dMagnetic2_loader_c64_advanceSector(&track,&sector);
					gfxidx+=D64_SECTORSIZE;
				}
				piccnt++;
			}
		}

		pMeta->real_gfxsize=gfxidx;

		gfxidx=0;
		// now the buffer is complete. write the header.
		pGfxBuf[gfxidx++]='M';
		pGfxBuf[gfxidx++]='a';
		pGfxBuf[gfxidx++]='P';
		pGfxBuf[gfxidx++]='5';
		for (i=0;i<32;i++)
		{
			int order;
			order=pPictureorder[i];
			WRITE_INT32BE(pGfxBuf,gfxidx ,(order==-1)?0xffffffff:picoffs[order]);
			gfxidx+=4;
		}
		pGfxBuf[4+4*32]=pMeta->version;
		/////////// GFX is finished ///////////////
	}
	return DMAGNETIC2_OK;
}


