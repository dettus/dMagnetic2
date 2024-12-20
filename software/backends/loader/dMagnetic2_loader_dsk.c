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


// entries in the CP/M file system (According to https://www.seasip.info/Cpm/format41.html
typedef struct _tDirEntry
{
	unsigned char userID;
	char name[MAXFILENAMELEN+1];
	unsigned char attrs;
	unsigned short extent;	// essentially: How many bytes are in the entry?
	unsigned char last_record_bytes;	// How many bytes in the last record? =0 means 128
	int number_of_records;		
	unsigned char blocks[MAXBLOCKS];// block identifier

	int fileID;		// the "filename" without the prefix.
	int offsetscnt;
	int offsets[MAXOFFSETSPERENTRY];	// translated offsets
} tDirEntry;

typedef struct _tNewDskInfo
{
	int baseoffset;		// actually the offset inside the disk image in memory
	int sectorsize;
	int size;
	int offsets[MAX_SECTORNUMPERDISK];

	int entrycnt;
	tDirEntry direntries[MAX_DIRENTRIES];
} tNewDskInfo;


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

int dMagnetic2_loader_dsk_readfile(unsigned char* pTmpBuf,tNewDskInfo* pDskInfo,int fileID,unsigned char* pOutput)
{
	int i;
	int outputidx;

	outputidx=0;
	for (i=0;i<MAX_DISKS;i++)	// look on all the disks
	{
		int j;
		int sectorsize;
		sectorsize=pDskInfo[i].sectorsize;
		for (j=0;j<pDskInfo[i].entrycnt;j++)	// in all the directories
		{
			tDirEntry *pDir;
			pDir=&(pDskInfo[i].direntries[j]);
			if (pDir->fileID==fileID)	// for the matching fileid
			{
				int k;
				for (k=0;k<pDir->offsetscnt;k++)	// copy all the sectors from this entry
				{
					int offset;
					offset=pDir->offsets[k];
					if (offset!=-1)		// only copy the valid ones
					{
						memcpy(&pOutput[outputidx],&pTmpBuf[offset],sectorsize);
						outputidx+=sectorsize;
					}
				}
			}
		}
	}
	return outputidx;
}

int dMagnetic2_loader_dsk_spectrum_mag(unsigned char* pTmpBuf,tNewDskInfo* pDskInfo,int gameidx,unsigned char* pMagBuf,tdMagnetic2_game_meta *pMeta,int nodoc)
{
	int outputidx;
	int version;
	int size_code;
	int size_string1;
	int size_string2;
	int size_dict;
	int retval;
	unsigned char *pTmpPtr;


	pTmpPtr=&pTmpBuf[2*DSK_MAX_IMAGESIZE];	// point to after the disk images
	outputidx=42;
	version=dMagnetic2_loader_dsk_knownGames[gameidx].version;

	// start with the code in FILE1, which is huffman encoded
	size_code=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,pTmpPtr);
	if (size_code==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	size_code=dMagnetic2_loader_shared_unhuffer(pTmpPtr,size_code,&pMagBuf[outputidx]);
	outputidx+=size_code;
	

	// the string1 section is in FILE3
	size_string1=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX3,&pMagBuf[outputidx]);
	if (size_string1==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	outputidx+=size_string1;

	// the string2 section is in FILE2, hufmanned
	size_string2=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX2,pTmpPtr);
	if (size_string2==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	size_string2=dMagnetic2_loader_shared_unhuffer(pTmpPtr,size_string2,&pMagBuf[outputidx]);
	outputidx+=size_string2;

	// the dict section is in FILE4, hufmanned
	size_dict=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX4,pTmpPtr);
	if (size_dict==0)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	size_dict=dMagnetic2_loader_shared_unhuffer(pTmpPtr,size_dict,&pMagBuf[outputidx]);
	outputidx+=size_dict;

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

	retval=dMagnetic2_loader_shared_addmagheader(pMagBuf,outputidx,version,size_code,size_string1,size_string2,size_dict,-1);
	return retval;
}


int dMagnetic2_loader_dsk_amstrad_mag(unsigned char* pTmpBuf,tNewDskInfo* pDskInfo,int gameidx,unsigned char* pMagBuf,tdMagnetic2_game_meta *pMeta,int nodoc)
{
	int outputidx;
	int size_code1;
	int size_code2;
	int size_string1;
	int size_string2;
	int size_dict;
	int retval;
	unsigned char *pTmpPtr;
	edMagnetic2_game game;


	pTmpPtr=&pTmpBuf[2*DSK_MAX_IMAGESIZE];	// point to after the disk images
	outputidx=42;
	game=dMagnetic2_loader_dsk_knownGames[gameidx].game;
	if (game==DMAGNETIC2_GAME_PAWN)
	{
		// in THE PAWN, the code section is packed	
		size_code1=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,pTmpPtr);
		if (size_code1==0)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		size_code1=dMagnetic2_loader_shared_unhuffer(pTmpBuf,size_code1,&pMagBuf[outputidx]);
		outputidx+=size_code1;
		size_code2=0;
	} else {
#define	MAGIC_STARTVALUE	0x1803
#define	MAGIC_INCREMENT		0x29
		int i;
		// in other games, it is spread out over two files: FILE1 and FILE6
		size_code1=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX1,&pMagBuf[outputidx]);
		retval=dMagnetic2_loader_shared_prbs_descrambler(&pMagBuf[outputidx],size_code1,MAGIC_STARTVALUE,MAGIC_INCREMENT);	// the first part is PRBS scrambled different than the second one
		outputidx+=size_code1;


		size_code2=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX6,&pMagBuf[outputidx]);
		// in the second part, each 128 byte block has its own start value
		for (i=0;i<size_code2;i+=128)
		{
			retval=dMagnetic2_loader_shared_prbs_descrambler(&pMagBuf[outputidx+i],128,size_code1+i,MAGIC_INCREMENT);
		}
	}
	outputidx+=size_code2;
	
	size_string1=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX3,&pMagBuf[outputidx]);
	outputidx+=size_string1;
	if (game==DMAGNETIC2_GAME_PAWN)
	{
		// in THE PAWN, the string2 section is packed
		size_string2=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX2,pTmpPtr);
		if (size_string2==0)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		size_string2=dMagnetic2_loader_shared_unhuffer(pTmpBuf,size_string2,&pMagBuf[outputidx]);
	} else {
		size_string2=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX2,&pMagBuf[outputidx]);
	}
	outputidx+=size_string2;
	// some games have a file with the suffix 8, and this is for the dict.
	size_dict=dMagnetic2_loader_dsk_readfile(pTmpBuf,pDskInfo,FILESUFFIX8,&pMagBuf[outputidx]);
	retval=dMagnetic2_loader_shared_prbs_descrambler(&pMagBuf[outputidx],size_dict,MAGIC_STARTVALUE,MAGIC_INCREMENT);
	outputidx+=size_dict;
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

	retval=dMagnetic2_loader_shared_addmagheader(pMagBuf,outputidx,dMagnetic2_loader_dsk_knownGames[gameidx].version,size_code1+size_code2,size_string1,size_string2,size_dict,-1);
	return retval;
}
int dMagnetic2_loader_dsk_amstrad_gfx(unsigned char* pTmpBuf,tNewDskInfo* pDskInfo,int gameidx,unsigned char* pGfxBuf,tdMagnetic2_game_meta *pMeta)
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








///////////////////////////////
int dMagnetic2_loader_dsk_find_sector_offsets(unsigned char* pImage,tNewDskInfo* pDskInfo)
{
	int i;
	int tracknum;
	int sidenum;
	int tracksize;
	int sectorcnt;
	int idx;
	int j;
	int extendedornot=0;

	// 0x00-0x21: "MV..."                        "Extended"
	// 0x22.0x2f: creator ID
	// 0x30: number of tracks
	// 0x31: number of sides
	// 0x32..33: size of the tracks             unused
	// 0x34..0xff: unused                       track size table


	switch ((char)pImage[0])
	{
		case 'M': extendedornot=0;break;
		case 'E': extendedornot=1;break;
		default: return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	tracknum=pImage[0x30];
	sidenum=pImage[0x31];

	sectorcnt=0;
	idx=SIZE_FILEHEADER;

	
	tracksize=READ_INT16LE(pImage,0x32);
	for (i=0;i<tracknum*sidenum;i++)
	{
		int idx0;

		idx0=idx;	
		if (extendedornot)
		{
			tracksize=pImage[0x34+i]*256;		// FIXME: or is this correct?
		}
		if (tracksize)
		{
			int track0;
			int side0;
			int sectorsize;
			int sectornum;

			int sectorIds[MAX_SECTORNUMPERTRACK]={0};
			int order[MAX_SECTORNUMPERTRACK]={0};
			// 0x00... 0x0b: Magic Word "Track"
			// 0x0c... 0x0f: unused
			idx+=0xc;	// skip the magic word
			idx+=0x4;
			
			track0=pImage[idx];	idx+=1;	
			side0=pImage[idx];	idx+=1;	
			idx+=2;			// 2 unused bytes


			if ((track0*sidenum+side0)!=i)	// sanity check failed
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}
			switch(pImage[idx])
			{
				case 0:	sectorsize= 128; break;
				case 1:	sectorsize= 256; break;
				case 2:	sectorsize= 512; break;
				case 3:	sectorsize=1024; break;
				case 4:	sectorsize=2048; break;
				default: return DMAGNETIC2_UNKNOWN_SOURCE;break; // sanity check failed
			}
			pDskInfo->sectorsize=sectorsize;
			idx+=1;

			sectornum=pImage[idx];	idx+=1;// number of sectors in this track
			idx+=2;			// skip over the gap3 length and the fillter type
			if (sectornum>=MAX_SECTORNUMPERTRACK)
			{
				return DMAGNETIC2_UNKNOWN_SOURCE;	// sanity check failed
			}
			/// this concludes the track header. now come the sector header(s)
			for (j=0;j<sectornum;j++)
			{
				int track1;
				int side1;

				track1=pImage[idx];	idx+=1;
				side1=pImage[idx];	idx+=1;

				if (track0!=track1 || side0!=side1)
				{
					return DMAGNETIC2_UNKNOWN_SOURCE;	// sanity check failed
				}


				order[j]=j;
				sectorIds[j]=pImage[idx];	idx+=1;
				idx+=3;		// skip sector size and fdc status
				idx+=2;		// skip over unused bytes
			}

			// now that the sectorids are known, it is important to sort them
			for (j=0;j<sectornum-1;j++)
			{
				int k;
				for (k=j+1;k<sectornum;k++)
				{
					if (sectorIds[order[j]]>sectorIds[order[k]])
					{
						order[j]^=order[k];
						order[k]^=order[j];
						order[j]^=order[k];
					}
				}
			}
			for (j=0;j<sectornum;j++)
			{
				pDskInfo->offsets[sectorcnt]=idx0+pDskInfo->baseoffset+SIZE_TRACKHEADER+order[j]*sectorsize;
				sectorcnt++;
			}
			idx=idx0+tracksize;	// advance to the next track
		}

	}
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_dsk_directory_new(unsigned char* pImage,tNewDskInfo* pDskInfo,int amstrad0spectrum1,int *pGameidx)
{
	int directorysector;
	int blocksize;
	int i;
	int idx;
	pDskInfo->entrycnt=0;
	*pGameidx=-1;

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

		idx=pDskInfo->offsets[0];
		sectorspertrack=pImage[idx+3];
		reservedtracks=pImage[idx+5];
		switch(pImage[idx+6])
		{
			case 0:	blocksize= 128;	break;
			case 1: blocksize= 256;	break;
			case 2: blocksize= 512;	break;
			case 3: blocksize=1024;	break;
			default: return DMAGNETIC2_UNKNOWN_SOURCE; break;
		}
		directorysector=sectorspertrack*reservedtracks;
	} else {	// amstrad just starts at the beginning
		directorysector=0;
		blocksize=MAXBLOCKSIZE;
	}

	pDskInfo->entrycnt=0;
	// at this point, the location of the directory is known. it can be read.
	for (i=0;i<(blocksize/pDskInfo->sectorsize)*2;i++)
	{	
		int j;
		
		for (j=0;j<pDskInfo->sectorsize;j+=SIZE_DIRENTRY)
		{
			int k;
			int validfilename;
			tDirEntry *pDir;

			validfilename=0;
			idx=pDskInfo->offsets[directorysector+i]+j-pDskInfo->baseoffset;
			pDir=&pDskInfo->direntries[pDskInfo->entrycnt];
			pDir->userID=pImage[idx];
			if (pDir->userID<=15)		// entry is a filename/pointer
			{
				for (k=0;k<MAXFILENAMELEN;k++)
				{
					pDir->name[k]=pImage[idx+1+k]&0x7f;
				}
				pDir->fileID=-1;
				pDir->attrs =(pImage[idx+ 9]>>7)&0x1;		// read only bit
				pDir->attrs|=(pImage[idx+10]>>6)&0x2;		// system file/hidden
				pDir->attrs|=(pImage[idx+11]>>5)&0x4;		// file has been backed up

				pDir->extent =(pImage[idx+12]);
				pDir->extent+=(pImage[idx+13])*32;		// what is missing is the exm mask.
				pDir->last_record_bytes=pImage[idx+14];
				if (pDir->last_record_bytes==0)
				{
					pDir->last_record_bytes=128;
				}
				pDir->number_of_records=pImage[idx+15];

				for (k=0;k<MAXBLOCKS;k++)
				{
					pDir->blocks[k]=pImage[idx+16+k];
				}
				for (k=0;k<NUM_GAMES;k++)
				{
					int l;
					l=strlen(dMagnetic2_loader_dsk_knownGames[k].gamefilename);
					if (strncmp(pDir->name,dMagnetic2_loader_dsk_knownGames[k].gamefilename,l)==0 &&
							pDir->name[l]>='0' && pDir->name[l]<='8')
					{
						*pGameidx=k;
						validfilename=1;

						pDir->fileID=pDir->name[l]-'0';
					}
				}
				if (validfilename)	// when the name matches the game
				{
					int n;
					// one "block" from the directory entry contains n sectors
					n=blocksize/pDskInfo->sectorsize;
					pDir->offsetscnt=MAXBLOCKS*n;
					for (k=0;k<MAXBLOCKS;k++)
					{
						int m;
						if (pDir->blocks[k]==0)
						{
							for (m=0;m<n;m++)
							{
								pDir->offsets[k*n+m]=-1;		// mark as invalid
							}
						} else {
							for (m=0;m<n;m++)
							{
								int sector;
								sector=pDir->blocks[k]*n+m+directorysector;
								pDir->offsets[k*n+m]=pDskInfo->offsets[sector];		// calculate the "real" offset
							}
						}							
					}
					pDskInfo->entrycnt++;	// only count the filenames which are part of the game
				}
			}
		}
	}
	if (*pGameidx==-1)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	return DMAGNETIC2_OK;	
}
#define	TODOSIZE	65536		// Some files are packed and need to be unhuffed. They are being loaded into the tmpbuffer

int dMagnetic2_loader_dsk_getsize(int *pBytes)
{
// should be large enough for a disk image. and a spare byte for a trick to determine the correct file size	
	*pBytes=2*DSK_MAX_IMAGESIZE+TODOSIZE;// should be large enough for two disk images. and a spare byte for a trick to determine the correct file size
	return DMAGNETIC2_OK;
}


int dMagnetic2_loader_dsk(
		char* filename1,char* filename2,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,
		unsigned char* pGfxBuf,
		tdMagnetic2_game_meta *pMeta,
		int amstrad0spectrum1,
		int nodoc)
{

// TODO: find out, if the disk images come in in the correct order (is it important??)
// TODO: autodetection, if amstrad or spectrum
	int i;
	int n;
	int retval;
	int diskcnt;
	int gameidx;
	tNewDskInfo	dskInfo[MAX_DISKS];
	
	FILE *f;


	memset(dskInfo,0,sizeof(tNewDskInfo));

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
	dskInfo[diskcnt].baseoffset=0;
	dskInfo[diskcnt].size=n;
	diskcnt++;
	if (n>DSK_MAX_IMAGESIZE || n<DSK_MIN_IMAGESIZE)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	if (filename2!=NULL)
	{
		f=fopen(filename2,"rb");
		if (f==NULL)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(&pTmpBuf[DSK_MAX_IMAGESIZE],sizeof(char),DSK_MAX_IMAGESIZE+1,f);
		fclose(f);
		dskInfo[diskcnt].size=n;
		dskInfo[diskcnt].baseoffset=DSK_MAX_IMAGESIZE;
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
		retval=dMagnetic2_loader_dsk_find_sector_offsets(&pTmpBuf[i*DSK_MAX_IMAGESIZE],&dskInfo[i]);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}

		retval=dMagnetic2_loader_dsk_directory_new(&pTmpBuf[i*DSK_MAX_IMAGESIZE],&dskInfo[i],amstrad0spectrum1,&gameidx);
		if (retval!=DMAGNETIC2_OK)
		{
			return retval;
		}
	}

	if (gameidx<0 || gameidx>=NUM_GAMES)
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

