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

#define	DSK_MIN_IMAGESIZE	194816
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
#define	MAX_SECTORNUMPERDISK	(DSK_IMAGESIZE/MINSECTORSIZE)
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
	{	.game=DMAGNETIC2_PAWN,
		.gamefilename="PAWN",
		.version=0,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4),
		.expectedsuffixes_spectrum  =(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)
	},
	{	.game=DMAGNETIC2_GUILD,
		.gamefilename="GUILD",
		.version=1,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7),
		.expectedsuffixes_spectrum  =(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_JINXTER,
		.gamefilename="JINX",
		.version=2,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum  =(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_CORRUPTION,
		.gamefilename="CORR",
		.version=3,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC2_FISH,		// TODO: COULD ALSO BE MYTH!!!
		.gamefilename="FILE",
		.version=3,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	},
	{	.game=DMAGNETIC_MYTH,		// TODO: Could be the same as above
		.gamefilename="????",
		.version=3,
		.expectedsuffixes_amstradcpc=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8),
		.expectedsuffixes_spectrum=(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)
	}
};
void dMagnetic2_loader_dsk_descrambler(unsigned char* outputbuf,int len,unsigned short startvalue)
{
	unsigned short value;
	unsigned char key;
	int i;

	value=startvalue;
	for (i=0;i<len;i++)
	{
		value=(value+((value<<8)+0x29))&0xffff;
		key=(value^(value>>8))&0xff;
		outputbuf[i]^=key;
	}
}
int dMagnetic2_loader_dsk_readfile(unsigned char* inputbuf,unsigned char* outputbuf,int fileID,tDirEntry* pDirEntries,int entrycnt,int sectorsize)
{
	int i;
	int outputidx;
	outputidx=0;
	for (i=0;i<entrycnt;i++)
	{
		int j;
		if (pDirEntries[i].fileID==fileID)
		{
			for (j=0;j<MAXOFFSETSPERENTRY;j++)
			{
				if (pDirEntries[i].offsets[j]!=-1)
				{
					memcpy(&outputbuf[outputidx],&inputbuf[pDirEntries[i].offsets[j]],sectorsize);
					outputidx+=sectorsize;
				}
			}
		}
	}
	return outputidx;
}

int dMagnetic2_loader_dsk_parse_image_header(unsigned char* pDskImage,tDskInfo *pDskInfo,int amstrad0spectrum1)
{
	int i;
	
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
	ddP	return DMAGNETIC2_UNKNOWN_SOURCE;
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
			idx+=0xf;
			track0=pDskImage[idx];	idx+=1;	// 0x10: track0
			side0=pDskImage[idx];	idx+=1;	// 0x11: side0
			idx+=2;				// 0x12..0x13: unused

			if ((track0*sidenum+side0)!=i)	// a little sanity check
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}

			switch((int)pDskInfo[idx])	// 0x14: The sectorsize for this track
			{
				case 1: sectorsize= 128;break;		
				case 2: sectorsize= 256;break;
				case 3: sectorsize= 512;break;
				case 4: sectorsize=1024;break;
				default: return DMAGNETIC2_UNKNOWN_SOURCE;
			}
			idx+=1;

			sectornum=pDskInfo[idx];	// 0x15: the number of sectors
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
				track1=pDskInfo[idx];		idx+=1;		// 0x00: track number
				side1=pDskInfo[idx];		idx+=1;		// 0x01: sector number
				if (track0!=track1 || side0!=side1)	// sanity check failed
				{
					return DMAGNETIC2_UNKNOWN_SOURCE;
				}
				sectorids[j]=pDskInfo[idx];	idx+=1;		// 0x02: sector id. THIS is important
				idx+=5;		// skip over 0x04..0x07
			}
			// sorting the sector ids. bring them in order
			for (j=0;j<sectornum-1;j++)
			{
				int k;
				for (k=j+1;k<sectornum;k++)
				{
					if (sectorids[order[l]]>sectorids[order[k]])
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
			idx+=pDskInfo->tracksize;
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
		reservedtrack=pDskInfo[pDskInfo->offsets[0]+0x05];
		switch (pDskImage[pDskInfo->offsets[0]+0x06])
		{
			case 1:	pDskInfo->blocksize= 128;break;
			case 2:	pDskInfo->blocksize= 256;break;
			case 3:	pDskInfo->blocksize= 512;break;
			case 4:	pDskInfo->blocksize=1024;break;
			default: break;		// probably not a spectrum image
		}
		pDskInfo->directorysector=sectorspertrack*reservedtracks;
	} else {
		pDskInfo->directorysector=0;
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

			ptr=&pDskImage[pDskInfo->offsets[i+pDskInfo->directorysector]+k];
			pDir=pDskInfo->direntries[pDskInfo->entrycnt];
			if (ptr[0x00]==0)		// userid=0
			{
				int k;
				pDir->userID=ptr[0x00];	// 0x00: userID
							// 0x01...0x08: filename
				for (k=0;k<MAXFILENAMELEN;k++)
				{
					pDir->name[k]=(signed char)(ptr[0x01+k]&0x7f);
				}
				pDir->name[k][MAXFILENAMELEN-1]=0;
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
							pDir->offsets[k*n+m]=pDskInfo->offsets[pDir->blocks[k]*n+m+pDskInfo->directorysector];
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


int dMagnetic2_loader_dsk(
		char* filename1,char* filename2,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,int* pRealMagSize,
		unsigned char* pGfxBuf,int* pRealGfxSize,
		tdmagnetic2_game_meta *pMeta,
		int amstrad0spectrum1,
		int nodoc)
{

// TODO: find out, if the disk images come in in the correct order
// TODO: autodetection, if amstrad or spectrum
	int i;
	int l;
	int sidecnt_is;
	int sidecnt_expected;
	int entrynum;
	int retval;
	int diskcnt;
	int sectorspertrack;
	int reservedtracks;
	int blocksize;
	int gameidx;
	tDskInfo	dskInfo[MAX_DISKS];
	
	
	FILE *f;


	memset(dskInfo,0,sizeof(tDskInfo));


	// check the important output buffers
	if (tmpsize<2*DSK_IMAGESIZE+1)	// should be large enough for two disk images. and a spare byte for a trick to determine the correct file size
	{
		return DMAGNETIC2_ERROR_BUFFER_TOO_SMALL;
	}
	if (pmeta==null || ptmpbuf==null)
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
	n=fread(&pTmpBuf[0],sizeof(char),DSK_IMAGESIZE+1,f);
	fclose(f);
	dskInfo[diskcnt]->size=n;	
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
		n=fread(&pTmpBuf[DSK_IMAGESIZE],sizeof(char),DSK_IMAGESIZE+1,f);
		fclose(f);
		dskInfo[diskcnt]->size=n;	
		diskcnt++;
		if (n>DSK_MAX_IMAGESIZE || n<DSK_MIN_IMAGESIZE)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}
	// at this point, either one or both of the image files are in memory
	for (i=0;i<diskcnt;i++)
	{
		retval=dMagnetic2_loader_dsk_parse_image_header(&pTmpBuf[i*DSK_IMAGESIZE],&dskInfo[i]);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
		if (dskInfo[i].sectorsize==0)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		retval=dMagnetic2_loader_dsk_directory(&pTmpBuf[i*DSK_IMAGESIZE],&dskInfo[i],&gameidx);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}


	

	return DMAGNETIC2_OK;
}

