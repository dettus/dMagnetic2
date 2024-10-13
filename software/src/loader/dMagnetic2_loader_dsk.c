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


// the purpose of this loader is to read disks from the Amstrad or Spectrum releases.
// Those were stored on Floppies with the CP/M file system.

#define	DSK_MIN_IMAGESIZE	194816	// disksize without any extended meta data
#define	DSK_MAX_IMAGESIZE	195635	// TODO: check	

#define	MAXFILENAMELEN	(8+1)	// filenames in CPM are 8 bytes long
#define	EXTENDLEN	4	// extension is 4 bytes long
#define	MAXBLOCKS	16	// 16 pointers per directory entry

#define	MAXBLOCKSIZE	1024	// one pointer is pointing towards 1 kByte
#define	MINSECTORSIZE	128	// the smallest number of bytes in a sector
#define	MAXOFFSETSPERENTRY	((MAXBLOCKSIZE/MINSECTORSIZE)*MAXBLOCKS)

#define	SIZE_FILEHEADER	256
#define	SIZE_TRACKHEADER	256
#define	SIZE_SECTORHEADER	8
#define	SIZE_DIRENTRY		32
#define	MAX_DIRENTRIES		32	// TODO???
#define	MAX_SECTORNUMPERTRACK	64
#define	MAX_SECTORNUMPERDISK	(DSK_MIN_IMAGESIZE/MINSECTORSIZE)	
#define	MAX_DISKS		2
#define	NUM_GAMES		6
#define	MAX_TRACKNUM		128	// TODO??

#define	FILESUFFIX1		1
#define	FILESUFFIX2		2
#define	FILESUFFIX3		3
#define	FILESUFFIX4		4
#define	FILESUFFIX5		5
#define	FILESUFFIX6		6
#define	FILESUFFIX7		7
#define	FILESUFFIX8		8


// entries in the CP/M file system
typedef struct _tDirEntry
{
	unsigned char userID;
	char name[MAXFILENAMELEN+1];
	unsigned char attrs;
	char extend[EXTENDLEN+1];	// extension
	unsigned char blocks[MAXBLOCKS];// block identifier

	int fileID;		// the "filename" without the prefix.
	int offsets[MAXOFFSETSPERENTRY];	// translated offsets
} tDirEntry;
// some information about the disk images
typedef struct _tDskInfo
{
	int size; // between 194816-195635
	int sectorcnt;
	int offsets[MAX_SECTORNUMPERDISK];
	int directory_sector;
	int blocksize;
	int sectorsize;
	int extended1not0;
	int tracknum;
	int sidenum;
	int tracksize[MAX_TRACKNUM];

	int entrycnt;
	tDirEntry direntries[MAX_DIRENTRIES];
} tDskInfo;


typedef struct _tGames
{
	edMagnetic2_game	game;	// the enumeration for the game
	char gamefilename[MAXFILENAMELEN+1];
	int version;
	unsigned short expectedsuffixes_amstradcpc;
	unsigned short expectedsuffixes_spectrum;
	
} tGames;

const tGames dMagnetic2_loader_dsk_knownGames[NUM_GAMES]={
	// name,game file names, version number
	{	.game=DMAGNETIC2_GAME_PAWN,
		.gamefilename="PAWN",
		.version=0,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4),
		.expectedsuffixes_spectrum  =(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)
	},
	{	.game=DMAGNETIC2_GAME_GUILD,
		.gamefilename="GUILD",
		.version=1,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7),
		.expectedsuffixes_spectrum  =(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_GAME_JINXTER,
		.gamefilename="JINX",
		.version=2,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum  =(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_GAME_CORRUPTION,
		.gamefilename="CORR",
		.version=3,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_GAME_FISH,		// TODO: COULD ALSO BE MYTH!!!
		.gamefilename="FILE",
		.version=3,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_GAME_MYTH,		// TODO: Could be the same as above
		.gamefilename="????",
		.version=3,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	}
};


int dMagnetic2_loader_dsk_parse_image_header(unsigned char* pDskImage,tDskInfo *pDskInfo,int amstrad0spectrum1)
{
	int i;
	int idx;
	
	// 0x00-0x21: "MV..."                        "Extended"
	// 0x22.0x2f: creator ID
	// 0x30: number of tracks
	// 0x31: number of sides
	// 0x32..33: size of the tracks             unused
	// 0x34..0xff: unused                       track size table

	if (pDskImage[0]=='M')	
	{
		pDskInfo->extended1not0=0;
	} else if (pDskImage[0]=='E')	
	{
		pDskInfo->extended1not0=1;
	} else {
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	pDskInfo->tracknum=pDskImage[0x30];
	pDskInfo->sidenum=pDskImage[0x31];

	if (pDskInfo->tracknum>=MAX_TRACKNUM)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	if (pDskInfo->extended1not0)
	{
		for (i=0;i<pDskInfo->tracknum;i++)
		{
			pDskInfo->tracksize[i]=pDskImage[0x34+i]*256;				
		}
	} else {
		for (i=0;i<pDskInfo->tracknum;i++)
		{
			pDskInfo->tracksize[i]=READ_INT16LE(pDskImage,0x32);
		}
	}
	idx=SIZE_FILEHEADER;
	pDskInfo->sectorcnt=0;
	for (i=0;i<pDskInfo->tracknum*pDskInfo->sidenum;i++)
	{
		int sectorids[MAX_SECTORNUMPERTRACK]={0};
		int order[MAX_SECTORNUMPERTRACK]={0};
		int idx0;
		int track0,side0;
		int j;
		int sectornum;
		int sectorsize;
			
		idx0=idx;
		if (pDskInfo->tracksize[i]!=0)
		{
			// 0x00-0x0b: Magic Word "Track..."
			// 0x0c-0x0f: unused
			if (strncmp("Track",&pDskImage[idx],5)!=0)
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}
			idx+=0x10;
			track0=pDskImage[idx];	idx+=1;	// 0x10: track0
			side0=pDskImage[idx];	idx+=1;	// 0x11: side0
			idx+=2;				// 0x12..0x13: unused

			if ((track0*pDskInfo->sidenum+side0)!=i)	// a little sanity check
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}

			switch((int)pDskImage[idx])	// 0x14: The sectorsize for this track
			{
				case 1: sectorsize= 128;break;		
				case 2: sectorsize= 256;break;
				case 3: sectorsize= 512;break;
				case 4: sectorsize=1024;break;
				default: return DMAGNETIC2_UNKNOWN_SOURCE;
			}
			idx+=1;

			sectornum=pDskImage[idx];	// 0x15: the number of sectors
			idx+=2;				// 0x16: gap3 length, 0x17: filler byte

			if (sectornum>=MAX_SECTORNUMPERTRACK)	// too many sectors
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}

			// after the track header comes the sector header
			// 0x00: track number
			// 0x01: side number
			// 0x02: sector ID
			// 0x03: sector size
			// 0x04..0x05: fdx status
			// 0x06..0x07: unused

			for (j=0;j<sectornum;j++)
			{
				int track1;
				int side1;

				order[j]=j;
				track1=pDskImage[idx];		idx+=1;		// 0x00: track number
				side1=pDskImage[idx];		idx+=1;		// 0x01: sector number
				if (track0!=track1 || side0!=side1)	// sanity check failed
				{
					return DMAGNETIC2_UNKNOWN_SOURCE;
				}
				sectorids[j]=pDskImage[idx];	idx+=1;		// 0x02: sector id. THIS is important
				idx+=5;		// skip over 0x04..0x07
			}
			// sorting the sector ids. bring them in order
			for (j=0;j<sectornum-1;j++)
			{
				int k;
				for (k=j+1;k<sectornum;k++)
				{
					if (sectorids[order[j]]>sectorids[order[k]])
					{
						// swap them
						order[j]^=order[k];
						order[k]^=order[j];
						order[j]^=order[k];
					}
				}
			}
			for (j=0;j<sectornum;j++)
			{
				pDskInfo->offsets[pDskInfo->sectorcnt++]=idx0+SIZE_TRACKHEADER+order[j]*sectorsize;
			}
			idx+=pDskInfo->tracksize[i];
		}
	}

// TODO: auto detection amstrad or spectrum?
	if (amstrad0spectrum1)
	{
		int sectorspertrack;
		int reservedtracks;

		// Within the Spectrum Disks, the first track contains the following information:
		// 
		//Byte 0          Disk type
		//                     0 = Standard PCW range DD SS ST (and +3)
		//                     1 = Standard CPC range DD SS ST system format
		//                     2 = Standard CPC range DD SS ST data only format
		//                     3 = Standard PCW range DD DS DT
		//                     All other values reserved
		//Byte 1          Bits 0...1 Sidedness
		//                     0 = Single sided
		//                     1 = Double sided (alternating sides)
		//                     2 = Double sided (successive sides)
		//                Bits 2...6 Reserved (set to 0)
		//                Bit 7 Double track
		//Byte 2          Number of tracks per side
		//Byte 3          Number of sectors per track
		//Byte 4          Log2(sector size) - 7
		//Byte 5          Number of reserved tracks
		//Byte 6          Log2(block size / 128)
		//Byte 7          Number of directory blocks
		//Byte 8          Gap length (read/write)
		//Byte 9          Gap length (format)
		//Bytes 10...14   Reserved
		//Byte 15         Checksum (used only if disk is bootable)

		// most of this information is redundant/unnecessary.
		// except for the ones needed to find the track containing the directory

		sectorspertrack=pDskImage[pDskInfo->offsets[0]+0x03];
		reservedtracks=pDskImage[pDskInfo->offsets[0]+0x05];
		switch (pDskImage[pDskInfo->offsets[0]+0x06])
		{
			case 1:	pDskInfo->blocksize= 128;break;
			case 2:	pDskInfo->blocksize= 256;break;
			case 3:	pDskInfo->blocksize= 512;break;
			case 4:	pDskInfo->blocksize=1024;break;
			default: break;		// probably not a spectrum image
		}
		pDskInfo->directory_sector=sectorspertrack*reservedtracks;
	} else {
		pDskInfo->directory_sector=0;
		pDskInfo->blocksize=MAXBLOCKSIZE;
	}
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_dsk_directory(unsigned char* pDskImage,tDskInfo *pDskInfo,int *pGameidx)
{
	int i;
	
	for (i=0;i<(pDskInfo->blocksize/pDskInfo->sectorsize)*2;i++)
	{
		int j;
		for (j=0;j<pDskInfo->sectorsize;j+=SIZE_DIRENTRY)
		{
			unsigned char *ptr;
			tDirEntry *pDir;

			ptr=&pDskImage[pDskInfo->offsets[i+pDskInfo->directory_sector]+j];
			pDir=&(pDskInfo->direntries[pDskInfo->entrycnt]);
			if (ptr[0x00]==0)		// userid=0
			{
				int k;
				pDir->userID=ptr[0x00];	// 0x00: userID
							// 0x01...0x08: filename
				for (k=0;k<MAXFILENAMELEN;k++)
				{
					pDir->name[k]=(signed char)(ptr[0x01+k]&0x7f);
				}
				pDir->name[MAXFILENAMELEN-1]=0;
				// voodoo with the attributes in bytes 0x09, 0x0a, 0x0b
				pDir->attrs=(ptr[0x09]>>5)&0x4;	
				pDir->attrs|=(ptr[0x0a]>>6)&0x2;
				pDir->attrs|=(ptr[0x0b]>>7)&0x1;
				// 0x0c..0xf: file extenstion
				for (k=0;k<EXTENDLEN;k++)
				{
					pDir->extend[k]=(signed char)(ptr[0x0c+k]);
				}
				
				// then the offsets for the payload
				for (k=0;k<MAXOFFSETSPERENTRY;k++)
				{
					pDir->offsets[k]=-1;		// invalidate them
				}
				for (k=0;k<MAXBLOCKS;k++)
				{
					int m;
					int n;
					n=pDskInfo->blocksize/pDskInfo->sectorsize;
					pDir->blocks[k]=ptr[0x10+k];
					if (pDir->blocks[k]==0)		// unused
					{
						for (m=0;m<n;m++)
						{
							pDir->offsets[k*n+m]=-1;		// mark as invalid
						}
					} else {
						for (m=0;m<n;m++)
						{
							pDir->offsets[k*n+m]=pDskInfo->offsets[pDir->blocks[k]*n+m+pDskInfo->directory_sector];
						}
					}
				}

				// while we are at it: let's detect the game by the filename
				for (k=0;k<NUM_GAMES;k++)
				{
					if (strncmp(dMagnetic2_loader_dsk_knownGames[k].gamefilename,pDir->name,MAXFILENAMELEN)==0)
					{
						pDir->fileID=pDir->name[strlen(dMagnetic2_loader_dsk_knownGames[k].gamefilename)]-'0';		// the last byte of the filename is the id.
						if (pDskInfo->entrycnt<MAX_DIRENTRIES)
						{
							pDskInfo->entrycnt++;	// only advance the directory entry counter if the filename matches one of the files that needs to be loaded
						}
						*pGameidx=k;
					}
				}
			}
		}
	}
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_dsk_readfile(unsigned char* pTmpBuf,tDskInfo* pDskInfo,int fileID,unsigned char* pOutput)
{
	int i;
	int outputidx;

	outputidx=0;
	// try to find the fileID on all disk images
	for (i=0;i<MAX_DISKS;i++)
	{
		unsigned char* pDskImage;
		tDirEntry *pDir;
		if (pDskInfo[i].size!=0)
		{
			int j;
			pDskImage=&pTmpBuf[i*DSK_MAX_IMAGESIZE];
			pDir=pDskInfo[i].direntries;
			for (j=0;j<pDskInfo[i].entrycnt;j++)
			{
				if (pDir[j].fileID==fileID)
				{
					int k;
					for (k=0;k<MAXOFFSETSPERENTRY;k++)
					{
						if (pDir[j].offsets[k]!=-1)
						{
							memcpy(&pOutput[outputidx],&pDskImage[pDir[j].offsets[k]],pDskInfo->sectorsize);
							outputidx+=pDskInfo->sectorsize;
						}
					}
				}
			}
		}
	}
	return outputidx;
}

int dMagnetic2_loader_dsk_spectrum_mag(unsigned char* pTmpBuf,tDskInfo* pDskInfo,int gameidx,unsigned char* pMagBuf,tdMagnetic2_game_meta *pMeta,int nodoc)
{
	int outputidx;
	int version;
	int codesize;
	int string1size;
	int string2size;
	int dictsize;
	int retval;
	unsigned char *pTmpPtr;


	pTmpPtr=&pTmpBuf[2*DSK_MAX_IMAGESIZE];	// point to after the disk images
	outputidx=42;
	version=dMagnetic2_loader_dsk_knownGames[gameidx].version;

	// start with the code in FILE1, which is huffman encoded
	codesize=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,pTmpPtr);
	if (codesize==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	codesize=dMagnetic2_loader_shared_unhuffer(pTmpBuf,codesize,&pMagBuf[outputidx]);
	outputidx+=codesize;

	// the string1 section is in FILE3
	string1size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX3,&pMagBuf[outputidx]);
	if (string1size==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	outputidx+=string1size;

	// the string2 section is in FILE2, hufmanned
	string2size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX2,pTmpPtr);
	if (string2size==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	string2size=dMagnetic2_loader_shared_unhuffer(pTmpBuf,string2size,&pMagBuf[outputidx]);
	outputidx+=string2size;

	// the dict section is in FILE4, hufmanned
	dictsize=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX4,pTmpPtr);
	if (dictsize==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	dictsize=dMagnetic2_loader_shared_unhuffer(pTmpBuf,dictsize,&pMagBuf[outputidx]);
	outputidx+=dictsize;

	if (nodoc)
	{
		int i;
		unsigned char* ptr=(unsigned char*)&pMagBuf[0];
		for (i=0;i<outputidx-4;i++)
		{
			if (ptr[i+0]==0x62 && ptr[i+1]==0x02 && ptr[i+2]==0xa2 && ptr[i+3]==0x00) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
			if (ptr[i+0]==0xa4 && ptr[i+1]==0x06 && ptr[i+2]==0xaa && ptr[i+3]==0xdf) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
		}
	}
	if (version==3 && pMagBuf[0x2836]==0x66) pMagBuf[0x2836]=0x60;	// final patch for myth
	pMeta->real_magsize=outputidx;

	retval=dMagnetic2_loader_shared_addmagheader(pMagBuf,outputidx,version,codesize,string1size,string2size,dictsize,-1);
	return retval;
}


int dMagnetic2_loader_dsk_amstrad_mag(unsigned char* pTmpBuf,tDskInfo* pDskInfo,int gameidx,unsigned char* pMagBuf,tdMagnetic2_game_meta *pMeta,int nodoc)
{
	int outputidx;
	int code1size;
	int code2size;
	int string1size;
	int string2size;
	int dictsize;
	int retval;
	unsigned char *pTmpPtr;
	edMagnetic2_game game;


	pTmpPtr=&pTmpBuf[2*DSK_MAX_IMAGESIZE];	// point to after the disk images
	outputidx=42;
	game=dMagnetic2_loader_dsk_knownGames[gameidx].game;
	if (game==DMAGNETIC2_GAME_PAWN)
	{
		// in THE PAWN, the code section is packed	
		code1size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,pTmpPtr);
		if (code1size==0)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		code1size=dMagnetic2_loader_shared_unhuffer(pTmpBuf,code1size,&pMagBuf[outputidx]);
		outputidx+=code1size;
		code2size=0;
	} else {
#define	MAGIC_STARTVALUE	0x1803
#define	MAGIC_INCREMENT		0x29
		int i;
		// in other games, it is spread out over two files: FILE1 and FILE6
		code1size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,&pMagBuf[outputidx]);
		retval=dMagnetic2_loader_shared_prbs_descrambler(&pMagBuf[outputidx],code1size,MAGIC_STARTVALUE,MAGIC_INCREMENT);	// the first part is PRBS scrambled different than the second one
		outputidx+=code1size;


		code2size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,&pMagBuf[outputidx]);
		// in the second part, each 128 byte block has its own start value
		for (i=0;i<code2size;i+=128)
		{
			retval=dMagnetic2_loader_shared_prbs_descrambler(&pMagBuf[outputidx+i],128,code1size+i,MAGIC_INCREMENT);
		}
	}
	outputidx+=code2size;
	
	string1size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX3,&pMagBuf[outputidx]);
	outputidx+=string1size;
	if (game==DMAGNETIC2_GAME_PAWN)
	{
		// in THE PAWN, the string2 section is packed
		string2size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX2,pTmpPtr);
		if (string2size==0)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		string2size=dMagnetic2_loader_shared_unhuffer(pTmpBuf,string2size,&pMagBuf[outputidx]);
	} else {
		string2size=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX2,&pMagBuf[outputidx]);
	}
	outputidx+=string2size;
	// some games have a file with the suffix 8, and this is for the dict.
	dictsize=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX8,&pMagBuf[outputidx]);
	retval=dMagnetic2_loader_shared_prbs_descrambler(&pMagBuf[outputidx],dictsize,MAGIC_STARTVALUE,MAGIC_INCREMENT);
	outputidx+=dictsize;
	if (nodoc)
	{
		int i;
		unsigned char* ptr=(unsigned char*)&pMagBuf[0];
		for (i=0;i<outputidx-4;i++)
		{
			if (ptr[i+0]==0x62 && ptr[i+1]==0x02 && ptr[i+2]==0xa2 && ptr[i+3]==0x00) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
			if (ptr[i+0]==0xa4 && ptr[i+1]==0x06 && ptr[i+2]==0xaa && ptr[i+3]==0xdf) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
		}
	}
	pMeta->real_magsize=outputidx;

	retval=dMagnetic2_loader_shared_addmagheader(pMagBuf,outputidx,dMagnetic2_loader_dsk_knownGames[gameidx].version,code1size+code2size,string1size,string2size,dictsize,-1);
	return retval;
}
int dMagnetic2_loader_dsk_amstrad_gfx(unsigned char* pTmpBuf,tDskInfo* pDskInfo,int gameidx,unsigned char* pGfxBuf,tdMagnetic2_game_meta *pMeta)
{
	int i;
	int outputidx;
	int outputidx0;
	edMagnetic2_game game;


	game=dMagnetic2_loader_dsk_knownGames[gameidx].game; 
	outputidx=0;

	// this time, start with the header. MaP6
	pGfxBuf[outputidx]='M';	outputidx+=1;
	pGfxBuf[outputidx]='a';	outputidx+=1;
	pGfxBuf[outputidx]='P';	outputidx+=1;
	pGfxBuf[outputidx]='6';	outputidx+=1;

	outputidx0=outputidx;
	if (game==DMAGNETIC2_GAME_PAWN)
	{
		// THE PAWN uses one single file for the images and the index. 

		// since it is not 32 bit aligned, add 2 extra bytes.
		pGfxBuf[outputidx]='0';	outputidx+=1;
		pGfxBuf[outputidx]='0';	outputidx+=1;
		outputidx+=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX4,&pGfxBuf[outputidx]); 
		// the beginning of the amstrad CPC image file starts with an index
		// this could be used directly, but now the header has to be taken into account.
		for (i=0;i<29;i++)	// go over the index for all 29 images
		{
			unsigned int x;
			x=READ_INT32LE(pGfxBuf,outputidx0+2+i*4);
			x+=6;
			x&=0xffffff;
			WRITE_INT32BE(pGfxBuf,outputidx0+i*4,x);		// make it big endian, move it two bytes forward.
		}
	} else {
		int idxoffs;
		int outputidx1;
		// TODO: check if more than 0 bytes have been read
		dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX4,&pGfxBuf[outputidx]); 	// the index is in this file
		outputidx=4+4*32;		// leave room for the header and the index

		outputidx0=outputidx;	
		outputidx+=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX5,&pGfxBuf[outputidx]); 	// some pictures in this file
		outputidx1=outputidx;	
		outputidx+=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX7,&pGfxBuf[outputidx]); 	// some pictures in that file
		idxoffs=4;

		for (i=0;i<32;i++)	// loop over the whole index
		{
			unsigned int x;
			x=READ_INT32LE(pGfxBuf,idxoffs);
			if (x&0xff000000)	// MSB is set, so the picture is in FILE5
			{
				x&=0xffffff;
				x+=outputidx0;	// the relative offset within the gfx buf
			} else {		// MSB is not set, picture is in FILE7
				x+=outputidx1;	// the relative offset within the gfx buf
			}
			if (x>=outputidx)	//  or it is not in there!
			{
				x=0;
			}
			WRITE_INT32BE(pGfxBuf,idxoffs,x);
			idxoffs+=4;	
		}
	}
	pMeta->real_gfxsize=outputidx;
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_dsk(
		char* filename1,char* filename2,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,int* pRealMagSize,
		unsigned char* pGfxBuf,int* pRealGfxSize,
		tdMagnetic2_game_meta *pMeta,
		int amstrad0spectrum1,
		int nodoc)
{

// TODO: find out, if the disk images come in in the correct order
// TODO: autodetection, if amstrad or spectrum
	int i;
	int n;
	int retval;
	int diskcnt;
	int gameidx;
	tDskInfo	dskInfo[MAX_DISKS];
	
	
	FILE *f;


	memset(dskInfo,0,sizeof(tDskInfo));

	#define	TODOSIZE	65536		// Some files are packed and need to be unhuffed. They are being loaded into the tmpbuffer
	// check the important output buffers
	if (tmpsize<2*DSK_MAX_IMAGESIZE+TODOSIZE)	// should be large enough for two disk images. and a spare byte for a trick to determine the correct file size
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
	diskcnt=0;

	if (filename1==NULL)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	f=fopen(filename1,"rb");
	if (f==NULL)
	{
		return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
	}
	n=fread(&pTmpBuf[0],sizeof(char),DSK_MAX_IMAGESIZE+1,f);
	fclose(f);
	dskInfo[diskcnt].size=n;	
	diskcnt++;
	if (n>DSK_MAX_IMAGESIZE || n<DSK_MIN_IMAGESIZE)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	if (filename2!=NULL)
	{
		f=fopen(filename1,"rb");
		if (f==NULL)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(&pTmpBuf[DSK_MAX_IMAGESIZE],sizeof(char),DSK_MAX_IMAGESIZE+1,f);
		fclose(f);
		dskInfo[diskcnt].size=n;	
		diskcnt++;
		if (n>DSK_MAX_IMAGESIZE || n<DSK_MIN_IMAGESIZE)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}
	// at this point, either one or both of the image files are in memory
	for (i=0;i<diskcnt;i++)
	{
		retval=dMagnetic2_loader_dsk_parse_image_header(&pTmpBuf[i*DSK_MAX_IMAGESIZE],&dskInfo[i],amstrad0spectrum1);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
		if (dskInfo[i].sectorsize==0)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		retval=dMagnetic2_loader_dsk_directory(&pTmpBuf[i*DSK_MAX_IMAGESIZE],&dskInfo[i],&gameidx);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}
	if (gameidx>=0 && gameidx<NUM_GAMES)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	pMeta->game=dMagnetic2_loader_dsk_knownGames[gameidx].game;		// TODO: Myth/Fish detection
	pMeta->version=dMagnetic2_loader_dsk_knownGames[gameidx].version;
	pMeta->source=amstrad0spectrum1?DMAGNETIC2_SOURCE_SPECTRUM:DMAGNETIC2_SOURCE_AMSTRAD_CPC;
	if (pMagBuf!=NULL)
	{
		if (amstrad0spectrum1)
		{
			retval=dMagnetic2_loader_dsk_spectrum_mag(pTmpBuf,dskInfo,gameidx,pMagBuf,pMeta,nodoc);
		} else {
			retval=dMagnetic2_loader_dsk_amstrad_mag(pTmpBuf,dskInfo,gameidx,pMagBuf,pMeta,nodoc);
		}
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}
	if (pGfxBuf!=NULL)
	{
		if (amstrad0spectrum1)
		{
			retval=DMAGNETIC2_OK;		// no pictures. sorry!
		} else {
			retval=dMagnetic2_loader_dsk_amstrad_gfx(pTmpBuf,dskInfo,gameidx,pGfxBuf,pMeta);

		}
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}

	

	return DMAGNETIC2_OK;
}

