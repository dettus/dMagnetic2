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

#define	ADFS_IMAGESIZE	819200
#define	ADFS_NICKMAP_OFFSET	0x40
#define	ADFS_NICKMAP_END	0x400
#define	RECURSIVEMAX	5


typedef	struct _tGames
{
	edMagnetic2_game	game;	// the enumeration for the game
	char magicdirname[10];
	char version;
	unsigned int expectedmask;
	int pictureorder[32];
} tGames;
#define	MAXGAMES	5
#define	MAXFILENAMENUM	10
#define	F6CODE		6
#define	F7DICT		7
#define	F8STRING2	8
#define	F9STRING1	9
#define	F10PICTURES	10

const tGames dMagnetic2_loader_archimedes_cGames[MAXGAMES]={
		{
			.game=DMAGNETIC2_GAME_PAWN,
			.version=0,
			.magicdirname="Pawn",
			.expectedmask=(1<<F6CODE)|            (1<<F8STRING2)|(1<<F9STRING1)|(1<<F10PICTURES),
			.pictureorder={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
		},
		{
			.game=DMAGNETIC2_GAME_GUILD,
			.version=1,
			.magicdirname="GUILD",
			.expectedmask=(1<<F6CODE)|(1<<F7DICT)|(1<<F8STRING2)|(1<<F9STRING1)|(1<<F10PICTURES),
			.pictureorder={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
		},
		{
			.game=DMAGNETIC2_GAME_JINXTER,
			.version=2,
			.magicdirname="Jinxter",
			.expectedmask=(1<<F6CODE)|(1<<F7DICT)|(1<<F8STRING2)|(1<<F9STRING1)|(1<<F10PICTURES),
			.pictureorder={16,21,22,11,0,6,3,15,24,7,12,13,1,28,26,17,23,9,4,18,25,20,10,8,19,14,2,    -1,-1,-1,-1,-1}
		},
		{
			.game=DMAGNETIC2_GAME_CORRUPTION,
			.version=3,
			.magicdirname="Corruption",
			.expectedmask=(1<<F6CODE)|(1<<F7DICT)|(1<<F8STRING2)|(1<<F9STRING1)|(1<<F10PICTURES),
			.pictureorder={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
		},
		{
			.game=DMAGNETIC2_GAME_FISH,
			.version=3,
			.magicdirname="Fish!",
			.expectedmask=(1<<F6CODE)|(1<<F7DICT)|(1<<F8STRING2)|(1<<F9STRING1)|(1<<F10PICTURES),
			.pictureorder={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
		}
};
int dMagnetic2_loader_archimedes_findoffset(unsigned char* map,int mapsize,int sectorsize,int indicator,int indlen,int bytespermapbit,int* pStart)
{
	int i;
	int status;
	unsigned int mask;
	unsigned short mapind;
	int sectoroffset;
	int indfound;
	int starti;
	i=0;
	status=0;
	mask=(1<<indlen)-1;

	sectoroffset=0;
	// in case the last 8 bits of the indicator are set, one map entry shares the pointer to several sectors.
	// the last 8 bits are said offset.
	if (indicator&0xff)
	{
		sectoroffset=((indicator&0xff)-1)*sectorsize;
	}
	indicator>>=8;
	indfound=0;
	// go through the allocation map, search for the indicator
	starti=-1;
	while (i<mapsize && indfound==0)
	{
		// TODO: for simplicity reasons, assume that the indicators are byte-aligned within the map.
		// in addition to this, do not think about fragmented files (yet)
		if (status==0)
		{
			mapind=READ_INT16LE(map,i);
			if (mapind)
			{
				if ((mapind&mask)==(indicator))
				{
					indfound=1;
					starti=i;
				}
				if ((mapind&~mask)==0) status=1;
			}
			i+=2;
		} else {
			if (map[i]&0x80)	// byte aligned assumption
			{
				status=0;
			}
			i++;
		}
	}
	if (indfound)
	{
		if (pStart!=NULL)
		{
			*pStart=starti*8*bytespermapbit+sectoroffset;	// this is assuming that the entries are byte-aligned
		}
	}
	return DMAGNETIC2_OK;	// did not find the indicator
}
int dMagnetic2_loader_archimedes_recursivedir(unsigned char* dskimg,int recursivettl,char* dirname,int hugo0nick1,int sectorsize,int indlen,int bytespermapbit,int diridx, int* pGameId,int *pOffsets,int* pLengths)
{
	int dirsize[2]={0x4cc-5,0x7dd-5};	// Hugo/Nick have different sizes for the directory
	int i;
	int done;
	int retval;
	int found;

	if (recursivettl==-1) return DMAGNETIC2_OK;	// too many recursions. something when wrong

	retval=0;
	i=0;
	done=0;
	found=0;
	diridx+=5;	// TODO: check for Hugo/Nick
	while (i<dirsize[hugo0nick1] && !done && retval==0)
	{
		// directories are 16 26 bytes long.
		// 0..9:  10 bytes name
		// 10..13: 4 bytes loadaddr
		// 14..17: 4 bytes execaddr
		// 18..21: 4 bytes length
		// 22..24: 3 bytes indicator
		// 25: 1 byte file type/permissions
		unsigned char name[5];	// looking for filenames F6,F7,F8,F9,F10 requires no more than 4 bytes.
		unsigned int indicator;
		int offset;
		int length;
		unsigned char dirtype;

		name[0]=dskimg[diridx+i+0];	// make upper case
		name[1]=dskimg[diridx+i+1];
		name[2]=dskimg[diridx+i+2];
		name[3]=dskimg[diridx+i+3];
		name[4]=0;

		length=READ_INT32LE(dskimg,diridx+i+18);
		indicator=READ_INT24LE(dskimg,diridx+i+22);
		dirtype=dskimg[diridx+i+25];
		if (name[0]==0) done=1;	// the last entry in the directory starts with 0.
		else {
			offset=0;
			if (hugo0nick1)
			{
				dMagnetic2_loader_archimedes_findoffset(&dskimg[ADFS_NICKMAP_OFFSET],ADFS_NICKMAP_END-ADFS_NICKMAP_OFFSET,sectorsize,indicator,indlen,bytespermapbit,&offset);
			} else {
				offset=indicator<<8;
			}

			if ((dirtype&0x8)==0x8)	// incase it is another directory
			{
				retval=dMagnetic2_loader_archimedes_recursivedir(dskimg,recursivettl-1,(char*)&dskimg[diridx+i+0],hugo0nick1,sectorsize,indlen,bytespermapbit,offset,pGameId,pOffsets,pLengths);
			} else {
				if (name[0]=='F' || name[0]=='f' || name[0]=='c' )	// when the filename starts with an 'f' or 'F'. For "corruption", it is a lower case 'c'. those are the files we are looking for
				{
					int num;	// turn the number behind the F,f, or c into an integer
					num=0;
					if (name[1]>='0' && name[1]<='9') 
					{
						num*=10;
						num+=name[1]-'0';
					} else num=-1;
					if (name[2]>='0' && name[2]<='9') 
					{
						num*=10;
						num+=name[2]-'0';
					} else if (name[2]!=0xd && name[2]!=0x00) num=-1;
					if (num>=0 && num<=MAXFILENAMENUM)
					{
						pOffsets[num]=offset;
						pLengths[num]=length;
						found|=(1<<num);
					}
				}
			}
		}

		i+=26;	// the size of an entry
	}
	if (found && *pGameId==-1)
	{
		int j;
		int k;
		int match;

		for (j=0;j<MAXGAMES && *pGameId==-1;j++)
		{
			match=1;
			for (k=0;k<10 && match;k++)
			{
				if (dMagnetic2_loader_archimedes_cGames[j].magicdirname[k]!=dirname[k] && dirname[k]>' ') match=0;
			}
			if (match) 
			{
				*pGameId=j;
			}
		}
		return found;
	}
	return retval;
}
int dMagnetic2_loader_archimedes_findfiles(unsigned char* dskimg,
	int *pGameId,int* pOffsets,int* pLengths,
	tdMagnetic2_game_meta *pMeta
	)		// pOffsets are pointers to the files F6 (code), F7 (dict), f8 (string2), f9(string1), F10 (pictures). IN THAT ORDER. the same goes for the lengths
{
	int hugo0nick1;
	int sectorsize;
	int indlen;
	int bytespermapbit;
	int diridx;
	unsigned int filemask;

	// step 1: find out, if the disk image contains a file system in the 'Hugo' or 'Nick' format.
	hugo0nick1=-1;
	// do it the lazy way. Just check if at the position where the root directory is expected to be,
	// the magic word "Hugo" or "Nick" appears.
#define	HUGOOFFS 0x400
#define	NICKOFFS 0x800
	// for Hugo disks, this is at the beginning of sector1, so @0x401-0x404
	if (dskimg[HUGOOFFS+1]=='H' && dskimg[HUGOOFFS+2]=='u' && dskimg[HUGOOFFS+3]=='g' && dskimg[HUGOOFFS+4]=='o') hugo0nick1=0;
	// for the Nick disks, this is at the beginning of sector2, so 0x801-0x803
	else if (dskimg[NICKOFFS+1]=='N' && dskimg[NICKOFFS+2]=='i' && dskimg[NICKOFFS+3]=='c' && dskimg[NICKOFFS+4]=='k') hugo0nick1=1;
	if (hugo0nick1==-1)
	{
//		fprintf(stderr,"unable to determine the file system type.\n");
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	sectorsize=1024;
	indlen=-1;
	bytespermapbit=-1;
	diridx=0x400;
	// step 2: if it is the "Nick" format, it comes with a header
	if (hugo0nick1==1)
	{
		int rootind;

		// according to //http://www.riscos.com/support/developers/prm/filecore.html#85862 
		// the header for the "Nick" format is in the first 36 bytes of the dsk image:
		// since the archimedes had an ARM processor, the values are little endian
		// 0..3: 4 bytes UNKNOWN
		// 4:    1 byte log2sector size  *
		// 5:    1 byte sectors/track
		// 6:    1 bytes heads/track
		// 7:    1 byte density
		// 8:    1 byte indlen           *
		// 9:    1 byte byterspermapbit  *
		// 10:   1 byte skey
		// 11:   1 byte boot option
		// 12:   1 byte lowsector
		// 13:   1 byte number of zones
		// 14:   2 byes zone spare
		// 16:   4 bytes root indicator	*
		// 20:   4 bytes disk size
		// 24:   4 bytes disk id
		// 26..35: disk name
		// only the ones with the * are important

		sectorsize=1<<dskimg[4];
		indlen=dskimg[8];
		bytespermapbit=1<<dskimg[9];
		rootind=READ_INT32LE(dskimg,16);


		dMagnetic2_loader_archimedes_findoffset(&dskimg[ADFS_NICKMAP_OFFSET],ADFS_NICKMAP_END-ADFS_NICKMAP_OFFSET,sectorsize,rootind,indlen,bytespermapbit,&diridx);
	}


	// at this point, the location of the root directory is known.
	// to find the files named F6, F7, F8, F9, F10, start a recursive search
	filemask=dMagnetic2_loader_archimedes_recursivedir(dskimg,RECURSIVEMAX,"$",	hugo0nick1,sectorsize,indlen,bytespermapbit,diridx,	pGameId,pOffsets,pLengths);

	if (filemask && *pGameId!=-1)
	{
//		fprintf(stderr,"loader detected %s --> ",loader_archimedes_cGames[*pGameId].gamename);
		if ((filemask&dMagnetic2_loader_archimedes_cGames[*pGameId].expectedmask)!=dMagnetic2_loader_archimedes_cGames[*pGameId].expectedmask)
		{
//			fprintf(stderr,"FAIL\n");
//			fprintf(stderr,"insufficient files on the disk. sorry\n");
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		pMeta->game=dMagnetic2_loader_archimedes_cGames[*pGameId].game;
		pMeta->source=DMAGNETIC2_SOURCE_ARCHIMEDES;
		pMeta->version=dMagnetic2_loader_archimedes_cGames[*pGameId].version;
		
//		fprintf(stderr,"okay\n");
		return DMAGNETIC2_OK;
	}
	return DMAGNETIC2_UNKNOWN_SOURCE;
}
int dMagnetic2_loader_archimedes_mkmag(unsigned char *dskimg,unsigned char* magbuf,int* magsize,
		int gameId,int* offsets,int *lengths,int nodoc)
{
	int magidx;
	int codesize;
	int string1size;
	int string2size;
	int dictsize;

	magidx=42;
	// the game code is stored in F6, it is packed
	codesize=dMagnetic2_loader_shared_unhuffer(&dskimg[offsets[F6CODE]],lengths[F6CODE],&magbuf[magidx]);
	magidx+=codesize;
	// the string1 is stored in F9
	memcpy(&magbuf[magidx],&dskimg[offsets[F9STRING1]],lengths[F9STRING1]);
	string1size=lengths[F9STRING1];
	magidx+=string1size;
	// string2 is in F8, it is packed
	string2size=dMagnetic2_loader_shared_unhuffer(&dskimg[offsets[F8STRING2]],lengths[F8STRING2],&magbuf[magidx]);
	magidx+=string2size;
	if (dMagnetic2_loader_archimedes_cGames[gameId].version)	// the pawn did not have a dictionary file.
	{
		// the dict is stored in F7, it is packed
		dictsize=dMagnetic2_loader_shared_unhuffer(&dskimg[offsets[F7DICT]],lengths[F7DICT],&magbuf[magidx]);
		magidx+=dictsize;
	} else {
		dictsize=0;
	}

	if (nodoc)
	{
		int i;
		unsigned char* ptr=(unsigned char*)&magbuf[0];
		for (i=0;i<magidx-4;i++)
		{
			if (ptr[i+0]==0x62 && ptr[i+1]==0x02 && ptr[i+2]==0xa2 && ptr[i+3]==0x00) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
			if (ptr[i+0]==0xa4 && ptr[i+1]==0x06 && ptr[i+2]==0xaa && ptr[i+3]==0xdf) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
		}
	}

	dMagnetic2_loader_shared_addmagheader(magbuf,magidx,dMagnetic2_loader_archimedes_cGames[gameId].version,codesize,string1size,string2size,dictsize,-1);

	*magsize=magidx;

	return DMAGNETIC2_OK;
}
// the archimedes basically uses the same graphic format as the Amiga and the Atari.
// all that is needed is to find the offsets to the pictures.
int dMagnetic2_loader_archimedes_mkgfx(unsigned char *dskimg,unsigned char* gfxbuf,int* gfxsize,
		int gameId,int* offsets,int *lengths)
{

	int gfxcnt;
	int idx;
	int length;
#define	GFX_HEADER_SIZE	(4+4+32*4)	
#define	PICTURE_HEADER_SIZE	48
	gfxbuf[0]='M';gfxbuf[1]='a';gfxbuf[2]='P';gfxbuf[3]='i';
	gfxcnt=0;

	length=lengths[F10PICTURES]+GFX_HEADER_SIZE;
	// copy the picture "file" from the disk image into the gfx buffer
	// todo: this will leave some junk in the gfx buffer. 
	memcpy(&gfxbuf[GFX_HEADER_SIZE],&dskimg[offsets[F10PICTURES]],lengths[F10PICTURES]);
	*gfxsize=length;
	idx=GFX_HEADER_SIZE;
	while (idx<(length-PICTURE_HEADER_SIZE))
	{
		// i do not know where the index for the pictures is.
		// their format is as followed: there is a 48 byte header. then comes the picture data.
		// luckily, within this header, at position 0x10, there is a magic sequece 00 00 07 77.
		// that is sufficient to find the end of the header, and consequently, the beginning of the picture data.

		if (gfxbuf[idx+0x10]==0x00 && gfxbuf[idx+0x11]==0x00 && gfxbuf[idx+0x12]==0x07 && gfxbuf[idx+0x13]==0x77)
		{
			int idx_dir;
			idx_dir=4*dMagnetic2_loader_archimedes_cGames[gameId].pictureorder[gfxcnt]+8;
			WRITE_INT32BE(gfxbuf,idx_dir,idx+PICTURE_HEADER_SIZE);
			gfxcnt++;
		}
		idx++;
	}
	WRITE_INT32BE(gfxbuf,4,length);
	return DMAGNETIC2_OK;
}


int dMagnetic2_loader_archimedes_getsize(int *pBytes)
{
// should be large enough for a disk image. and a spare byte for a trick to determine the correct file size	
	*pBytes=ADFS_IMAGESIZE+1;
	return DMAGNETIC2_OK;
}

int dMagnetic2_loader_archimedes(
		char* filename1,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,
		unsigned char* pGfxBuf,
		tdMagnetic2_game_meta *pMeta,
		int nodoc)
		
{

	int offsets[MAXFILENAMENUM+1]={0};
	int lengths[MAXFILENAMENUM+1]={0};
	int gameId=-1;

	int n;
	FILE *f;
	// check the important output buffers
	if (pMeta==NULL || pTmpBuf==NULL)
	{
		return DMAGNETIC2_ERROR_NULLPTR;
	}
	if (tmpsize<ADFS_IMAGESIZE+1)	
	{
		return DMAGNETIC2_ERROR_BUFFER_TOO_SMALL;
	}

	pMeta->game=DMAGNETIC2_GAME_NONE;
	pMeta->source=DMAGNETIC2_SOURCE_NONE;
	pMeta->version=-1;
	pMeta->real_magsize=0;
	pMeta->real_gfxsize=0;


	if (filename1!=NULL)
	{
		// check if the file starts with the correct magic word(s)
		f=fopen(filename1,"rb");
		if (!f)
		{
			return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
		}
		n=fread(pTmpBuf,sizeof(char),ADFS_IMAGESIZE+1,f);	// read the image file. plus 1 byte to check if the size will match.
		fclose(f);
		if (n!=ADFS_IMAGESIZE)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}

	// at this point, the diskiamge has been loaded.
	if (dMagnetic2_loader_archimedes_findfiles(pTmpBuf,&gameId,offsets,lengths,pMeta)!=DMAGNETIC2_OK)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	if (pMagBuf!=NULL)
	{
		if (dMagnetic2_loader_archimedes_mkmag(pTmpBuf,pMagBuf,&pMeta->real_magsize,gameId,offsets,lengths,nodoc)!=DMAGNETIC2_OK)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}
	if (pGfxBuf!=NULL)
	{
		if (dMagnetic2_loader_archimedes_mkgfx(pTmpBuf,pGfxBuf,&pMeta->real_gfxsize,gameId,offsets,lengths)!=DMAGNETIC2_OK)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}
	

	return DMAGNETIC2_OK;	


}

