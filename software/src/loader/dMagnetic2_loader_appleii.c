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

#define	PREAMBLE_SIZE	3
#define	EPILOGUE_SIZE	3
#define	ADDRBUF_SIZE	8
#define	DATABUF_SIZE	343

#define	MAXDISKS	3
#define	MAXTRACKS	35
#define	MAXSECTORS	16
#define	SECTORBYTES	256
#define	DSKTRACKSIZE	(MAXSECTORS*SECTORBYTES)
#define	DSKSIZE		(MAXTRACKS*MAXSECTORS*SECTORBYTES)
#define	NIBTRACKSIZE	(MAXSECTORS*416)	// TODO: why 416?



#define	GAME_PAWN	0
#define	GAME_GUILD	1
#define	GAME_JINXTER	2
#define	GAME_CORRUPTION	3
#define	MAXPICTURES	32




#define	WOZ_MAXQUARTERTRACKS	(4*MAXTRACKS)		// TODO: not sure what to do with this...
#define	WOZ_BLOCKSIZE		512			// the woz format has been designed to work on SD cards. So blocks are aligned to 512 to speed up processing


// the most important information, parsed from the WOZ header
typedef struct _tWozInfo
{
	int quarterTrack[WOZ_MAXQUARTERTRACKS];	// TODO: not sure what to do with this...
	int trackStart[WOZ_MAXQUARTERTRACKS];	// the block offset within the WOZ file
	int trackBits[WOZ_MAXQUARTERTRACKS];	// the number of bits for a track
	unsigned int crc32_expected;
} tWozInfo;

// in the .nib files, data is encoded.
// moreover, since a track is round, a sector could start at the end and continue at the beginnig
int dMagnetic2_loader_appleii_decodenibtrack(unsigned char* pTrackBuf,int track,unsigned char* pTmpBuf)
{
#define	PREAMBLESIZE	3
#define	DECODEROFFS	0x96
#define	SECTORLSB	86
	const	unsigned char dMagnetic2_loader_appleii_addr_preamble[PREAMBLESIZE]={0xD5,0xAA,0x96};
	const	unsigned char dMagnetic2_loader_appleii_data_preamble[PREAMBLESIZE]={0xD5,0xAA,0xAD};
	//const	unsigned char loader_appleii_epilog[PREAMBLESIZE]={0xDE,0xAA,0xEB};

	const	unsigned char dMagnetic2_loader_appleii_translatetab[106]={
		0x00,0x01,0xFF,0xFF,0x02,0x03,0xFF,0x04,
		0x05,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x07,0x08,0xFF,0xFF,0xFF,0x09,0x0A,0x0B,
		0x0C,0x0D,0xFF,0xFF,0x0E,0x0F,0x10,0x11,
		0x12,0x13,0xFF,0x14,0x15,0x16,0x17,0x18,
		0x19,0x1A,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0x1B,0xFF,0x1C,
		0x1D,0x1E,0xFF,0xFF,0xFF,0x1F,0xFF,0xFF,
		0x20,0x21,0xFF,0x22,0x23,0x24,0x25,0x26,
		0x27,0x28,0xFF,0xFF,0xFF,0xFF,0xFF,0x29,
		0x2A,0x2B,0xFF,0x2C,0x2D,0x2E,0x2F,0x30,
		0x31,0x32,0xFF,0xFF,0x33,0x34,0x35,0x36,
		0x37,0x38,0xFF,0x39,0x3A,0x3B,0x3C,0x3D,
		0x3E,0x3F};


	unsigned char addr_track=0;
	unsigned char addr_sector=0;
	unsigned char addr_volume=0;
	unsigned char addr_checksum=0;
	int volumeid;
	int ridx;
	int state;

	volumeid=-1;
	ridx=0;
	state=0;
	while (ridx<(NIBTRACKSIZE+SECTORBYTES+SECTORLSB+1+9*PREAMBLESIZE))
	{
		switch(state)
		{
			case 0:		// find the ADDR preamble
				{
					if ( pTrackBuf[(ridx+0)%NIBTRACKSIZE]==loader_appleii_addr_preamble[0] && pTrackBuf[(ridx+1)%NIBTRACKSIZE]==loader_appleii_addr_preamble[1] && pTrackBuf[(ridx+2)%NIBTRACKSIZE]==loader_appleii_addr_preamble[2])
					{
						ridx+=PREAMBLE_SIZE;
						state=1;
					} else {
						ridx++;
					}
				}
				break;
			case 1:		// decode the ADDR data
				{
					unsigned char addrbuf[ADDRBUF_SIZE];
					int i;
					for (i=0;i<ADDRBUF_SIZE;i++)
					{
						addrbuf[i]=pTrackBuf[(ridx++)%NIBTRACKSIZE];
					}
					loader_appleii_decode_addrbuf(addrbuf,&addr_volume,&addr_track,&addr_sector,&addr_checksum);
					if (volumeid==-1 || volumeid==addr_volume)
					{
						volumeid=addr_volume;
					} else {
//						printf("volumeid mismatch\n");
						return -1;
					}
					if (addr_track!=track)
					{
//						printf("track mismatch %d vs %d\n",addr_track,track);
						return -1;
					}
					ridx+=PREAMBLESIZE;	// skip over the epilogue
					state=2;	// start looking for data
				}
				break;
			case 2:		// find the DATA preamble
				{
					if ( pTrackBuf[(ridx+0)%NIBTRACKSIZE]==loader_appleii_data_preamble[0] && pTrackBuf[(ridx+1)%NIBTRACKSIZE]==loader_appleii_data_preamble[1] && pTrackBuf[(ridx+2)%NIBTRACKSIZE]==loader_appleii_data_preamble[2])
					{
						ridx+=PREAMBLE_SIZE;
						state=3;
					} else ridx++;
				}
				break;
			case 3:		// decode the DATA
				{
					unsigned char lsbbuf[SECTORLSB];
					unsigned char accu;
					int j;
					accu=0;
					for (j=0;j<SECTORLSB;j++)
					{
						accu^=loader_appleii_translatetab[pTrackBuf[ridx%NIBTRACKSIZE]-DECODEROFFS];
						lsbbuf[j]=accu;
						ridx++;
					}
					for (j=0;j<SECTORBYTES;j++)
					{
						int widx;
						accu^=loader_appleii_translatetab[pTrackBuf[ridx%NIBTRACKSIZE]-DECODEROFFS];
						widx=j+SECTORBYTES*addr_sector;
						pDskBuf[widx]=accu;
						pDskBuf[widx]<<=1;pDskBuf[widx]|=(1&lsbbuf[j%SECTORLSB]);lsbbuf[j%SECTORLSB]>>=1;
						pDskBuf[widx]<<=1;pDskBuf[widx]|=(1&lsbbuf[j%SECTORLSB]);lsbbuf[j%SECTORLSB]>>=1;
						ridx++;
					}
					ridx+=PREAMBLESIZE;	// skip over the epilogue
					state=0;		// search for the next ADDR preamble
				}
				break;
		}

	}
	return volumeid;
}



int dMagnetic2_loader_appleii_woz_parseheader(unsigned char* pWozBuf,int wozsize,tWozInfo *pWozInfo)
{
	int idx;
	int len;
	unsigned char donemask;
	idx=0;
	donemask=0;
	while (idx<wozsize && donemask!=3)
	{
		if (memcmp(&pWozBuf[idx],"WOZ2",4)==0)
		{
			if (pWozBuf[idx+4]!=0xff || pWozBuf[idx+5]!=0xa || pWozBuf[idx+6]!=0xd || pWozBuf[idx+7]!=0xa)
			{
//				fprintf(stderr,"       WOZ2 eader corruption? Expected FF 0A 0D 0A, got %02X %02X %02X %02X \n",pWozBuf[idx+4],pWozBuf[idx+5],pWozBuf[idx+6],pWozBuf[idx+7]);
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}
			pWozInfo->crc32_expected=READ_INT32BE(pWozBuf,idx+8);
			idx+=12;
		}
		else if (memcmp(&pWozBuf[idx],"INFO",4)==0)
		{
			len=READ_INT32LE(pWozBuf,idx+4);
			idx+=8;
			// skip the info chunk
			idx+=len;
		}
		else if (memcmp(&pWozBuf[idx],"TMAP",4)==0)
		{
			int i;
			len=READ_INT32LE(pWozBuf,idx+4);
			idx+=8;
			for (i=0;i<WOZ_MAXQUARTERTRACKS;i++)
			{
				unsigned char x;
				x=pWozBuf[idx+i];
				if (x>=0 && x<MAXTRACKS)
				{
					pWozInfo->quarterTrack[i]=x;
				}
			}
			idx+=len;
			donemask|=1;
		}
		else if (memcmp(&pWozBuf[idx],"TRKS",4)==0)
		{
			int i;
			int idx2;
			len=READ_INT32LE(pWozBuf,idx+4);
			idx+=8;
			idx2=idx;
			for (i=0;i<WOZ_MAXQUARTERTRACKS;i++)
			{
				pWozInfo->trackStart[i]=WOZ_BLOCKSIZE*READ_INT16LE(pWozBuf,idx2);idx2+=2;
				idx2+=2;		//skip the block count
				pWozInfo->trackBits[i]=READ_INT32LE(pWozBuf,idx2);idx2+=4;
			}
			idx+=len;
			donemask|=2;

		}
		else if (memcmp(&pWozBuf[idx],"META",4)==0)
		{
			len=READ_INT32LE(pWozBuf,idx+4);
			idx+=8;
			// skip the info chunk
			idx+=len;
		}
		else 
		{
//			fprintf(stderr,"Unknown Tag in WOZ2 detected %02X %02X %02X %02X\n",pWozBuf[idx+0],pWozBuf[idx+1],pWozBuf[idx+2],pWozBuf[idx+3]);
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
	}
	if (donemask!=3)
	{
//		fprintf(stderr," Error parsing the WOZ header\n");
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	return DMAGNETIC_OK;
}

// when the woz bit stream is synchronized, it can be interpreted as a nib stream.
int dMagnetic2_loader_appleii_woz_synchronize(unsigned char* trackbuf,unsigned char* wozbuf,int len)
{
	unsigned char byte;
	unsigned int reg;
	int addrcnt;
	int datacnt;
	int part_cnt;
	int outidx;
	int i;



	reg=0;
	byte=0;
	addrcnt=0;
	datacnt=0;
	part_cnt=0;
	outidx=0;
	for (i=0;i<NIBTRACKSIZE;i++) trackbuf[i]=0xff;	// initialize
	while (outidx<NIBTRACKSIZE && i<(len*2) && (part_cnt!=0 || addrcnt!=MAXSECTORS || datacnt!=MAXSECTORS))
	{
		int wozbyte;
		int wozbit;
		int bit;

		wozbyte=(i%len)/8;
		wozbit=(i%len)%8;
		bit=(wozbuf[wozbyte]>>(7-wozbit))&1;
		byte<<=1;
		byte|=bit;
		if (byte&0x80)	// byte is synchronized when the highest bit is set.
		{
			reg<<=8;
			reg|=((unsigned int)byte)&0xff;
			reg&=0x00ffffff;
			if (part_cnt==0)
			{
				if (reg==0xD5AA96)	// addr preamble found
				{
					addrcnt++;
					trackbuf[outidx++]=0xD5;		// write the preamble
					trackbuf[outidx++]=0xAA;
					trackbuf[outidx++]=0x96;
					part_cnt=ADDRBUF_SIZE+EPILOGUE_SIZE;	// collect 11 bytes 
				}
				if (reg==0xD5AAAD)	// data preamble found
				{
					datacnt++;
					trackbuf[outidx++]=0xD5;		// write the preamble
					trackbuf[outidx++]=0xAA;
					trackbuf[outidx++]=0xAD;
					part_cnt=DATABUF_SIZE+EPILOGUE_SIZE;	// collect 346 bytes
				}
			} else {
				trackbuf[outidx++]=byte;
				part_cnt--;
			}
			byte=0;
		}
		i++;
	}
	// at this point, the trackbuf contains the NIB stream, even though there is no padding between the sectors.
	// the nib decoder will be able to handle it, even though a physical drive might not be able to.
	return DMAGNETIC2_OK;
}


int dMagnetic2_loader_appleii(
		char* filename1,char* filename2,char* filename3,
		unsigned char* pTmpBuf,int tmpsize,
		unsigned char* pMagBuf,int* pRealMagSize,
		unsigned char* pGfxBuf,int* pRealGfxSize,
		tdMagnetic2_game_meta *pMeta,
		int nodoc)
{
#define	SIZE_WOZIMAGE	241435
#define	SIZE_NIBIMAGE	232960
#define	SIZE_2MGIMAGE	143424
#define	SIZE_DSKIMAGE	143360
#define	MAX_IMAGEFILESIZE	(SIZE_WOZIMAGE)
	int diskoffs[MAXDISKS]={0};
	int disklens[MAXDISKS]={0};
	int decodedoffs[MAXDISKS]={0};
	int decodedlens[MAXDISKS]={0};
	int volumeids[MAXDISKS]={0};
	int i;
	int diskcnt;
	int n;
	int offs;
	int magsize;
	int gfxsize;
	FILE *f;
	char *filenames[]={filename1,filename2,filename3};
	unsigned char *pTrackBuf;

	// check the important output buffers
	if (tmpsize<NIBTRACKSIZE+3*MAX_IMAGEFILESIZE+1)	// should be large enough for three disk images. and a spare byte for a trick to determine the correct file size
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


	pTrackBuf=&pTmpBuf[0];	// leave a litte buffer at the beginning of the file for the track buffer
	offs=NIBTRACKSIZE;	// load the images after some space for a track buffer
	diskcnt=0;
	for (i=0;i<MAXDISKS;i++)
	{
		if (filenames[i]!=NULL)
		{
			f=fopen(filenames[i],"rb");
			if (!f)
			{
				return DMAGNETIC2_UNABLE_TO_OPEN_FILE;
			}
			n=fread(&pTmpBuf[offs],sizeof(char),MAX_IMAGEFILESIZE+1,f);
			fclose(f);
			if (n!=SIZE_NIBIMAGE && n!=SIZE_2MGIMAGE && n!=SIZE_DSKIMAGE && n!=SIZE_WOZIMAGE)
			{
//				if (pTmpBuf[offs+0]!='W' || pTmpBuf[offs+1]!='O' || pTmpBuf[offs+2]!='Z' || pTmpBufs[offs+2]=='2')
				{
					return DMAGNETIC2_UNKNOWN_SOURCE;
				}
			}
			diskoffs[i]=offs;
			disklens[i]=n;
			offs+=n;
			diskcnt++;
		}
	}
	// at this point, all the images have been loaded into the tmp buffer.
	// the data is encoded. before it can be used, it needs to be decoded.
	for (i=0;i<diskcnt;i++)
	{
		int lastvolumeid;
		if (disklens[i]==SIZE_NIBIMAGE)		// NIB images are raw, encoded image files.
		{
			int j;
			lastvolumeid=-1;
			
			decodedoffs[i]=diskoffs[i];
			decodedlens[i]=MAXTRACKS*MAXSECTORS*SECTORBYTES;
			offs=diskoffs[i];	
			for (j=0;j<MAXTRACKS;j++)
			{
				int volumeid;
				// to conserve memory, this process overwrites the loaded image. 
				// since a decoded track has less bytes than an encoded one, it can be done in place.
				memcpy(&pTrackBuf,pTmpBuf[diskoffs[i]+j*NIBTRACKSIZE],NIBTRACKSIZE);
				volumeid=dMagnetic2_loader_decodenibtrack(pTrackBuf,j,&pTmpBuf[offs]);
				offs+=MAXSECTORS*SECTORBYTES;
				if (lastvolumeid==-1)
				{
					lastvolumeid=volumeid;
				}
				if (lastvolumeid!=volumeid)
				{
					return DMAGNETIC2_UNKNOWN_SOURCE;
				}
			}
			volumeids[i]=lastvolumeid;
		}
		else if (disklens[i]==SIZE_2MGIMAGE)	// 2MG Files are already decoded. They come with a header.
		{
			// read in the header. https://apple2.org.za/gswv/a2zine/Docs/DiskImage_2MG_Info.txt
			volumeids[i]=pTmpBuf[diskoffs[i]+0x10];	// the volume id is stored in byte 0x10
			decodedoffs[i]=diskoffs[i]+0x40;	// skip the header. 
			decodedlens[i]=SIZE_2MGIMAGE-0x40;	// 
		}
		else 	// must be a .woz file
		{
			offs=diskoffs[i];
			if (pTmpBuf[offs+0]=='W' && pTmpBuf[offs+1]=='O' && pTmpBuf[offs+2]=='Z' && pTmpBuf[offs+3]=='2' )
			{
				tWozInfo wozInfo;
				int lastvolumeid=-1;
				// the file is a .woz file, basically an unsynchronized .nib file with a header.
				// the idea is to synchronize the tracks and treat it as a NIB file.

				// first, the wo header needs to be parsed, to find the tracks within the diskfile
				if (dMagnetic2_loader_woz_parseheader(&pTmpBuf[offs],disklens[i],&wozInfo)!=DMAGNETIC2_OK)
				{
					return DMAGNETIC2_UNKNOWN_SOURCE;
				}
				// at this point, the header information has been read. the tracks can be found, and the WOZ can be decoded		
				offs=diskoffs[i];
				decodedoffs[i]=offs;
				decodedlens[i]=MAXTRACKS*MAXSECTORS*SECTORBYTES;
				for (j=0;j<MAXTRACKS;j++)
				{
					int start;
					int len;
					int quarterTrack;

					quarterTrack=j;
					start=wozInfo.trackStart[quarterTrack];
					len=wozInfo.trackBits[quarterTrack];

					if (start)
					{
						int volumeid;
						// Synchronize the bit stream in the .woz to make it a .nib stream						
						dMagnetic2_loader_appleii_woz_synchronize(pTrackBuf,&pTmpBuf[diskoffs[i]+start],len);
						volumeid=dMagnetic2_loader_decodenibtrack(pTrackBuf,j,&pTmpBuf[offs]);
						offs+=MAXSECTORS*SECTORBYTES;
						if (lastvolumeid==-1)
						{
							lastvolumeid=volumeid;
						}
						if (lastvolumeid!=volumeid)
						{
							return DMAGNETIC2_UNKNOWN_SOURCE;
						}
					}
				}
				volumeids[i]=lastvolumeid;
			} else {
				return DMAGNETIC2_UNKNOWN_SOURCE;
			}
		}
	}
	// at this point, the disk images have been decoded and are available in the tmpbuf.
	// with the volumeid, it is now possible to detect the game which disk it is
	pMeta->game=DMAGNETIC2_GAME_UNKNOWN;
	for (i=0;i<diskcnt;i++)
	{
		int disknum;
		edMagnetic2_game game;
		game=DMAGNETIC2_GAME_UNKNOWN;
		switch (volumeids[i])
		{
			// volumeid 0x68 was for "The Pawn"
			case 0x68:	game=DMAGNETIC2_GAME_PAWN;		disknum=volumeids[i]-0x68;break;
			// volumeid 0x69 was for "The Guild Of Thieves"
			case 0x69:	game=DMAGNETIC2_GAME_GUILD;		disknum=volumeids[i]-0x69;break;
			// volumeid 0x70-0x71 are the two floopy disks for "Jinxter"
			case 0x70:
			case 0x71:	game=DMAGNETIC2_GAME_JINXTER;		disknum=volumeids[i]-0x70;break;	
			// volumeid 0x72-0x74 are the three floppy disks for "Corruption"
			case 0x72:
			case 0x73:
			case 0x74:	game=DMAGNETIC2_GAME_CORRUPTION;	disknum=volumeids[i]-0x72;break;
			default:
				return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		if (pMeta->game==DMAGNETIC2_GAME_UNKNOWN)
		{
			pMeta->game=game;
		}
		if (game!=pMeta->game || disknum<0 || disknum>=MAXDISKS)
		{
			// game detection ambiguous
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		diskoffs[disknum]=decodedoffs[i];
	}
	if (pMeta->game==DMAGNETIC2_GAME_UNKNOWN)
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}
	pMeta->source=DMAGNETIC_SOURCE_APPLEII;
	// at this point, the game is known
	if (pMagBuf!=NULL)
	{
		int magsize;
		if (dMagnetic2_loader_appleiixi_mkmag(pMagBuf,&magsize,pMeta->game,pTmpBuf,diskcnt,diskoffs)!=DMAGNETIC_OK)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		pMeta->real_magsize=magsize;	
	}
	if (pGfxBuf!=NULL)
	{
		int gfxsize;
		if (dMagnetic2_loader_appleii_mkgfx(pGfxBuf,&gfxsize,pMeta->game,pTmpBuf,diskcnt,diskoffs)!=DMAGNETIC_OK)
		{
			return DMAGNETIC2_UNKNOWN_SOURCE;
		}
		pMeta->real_gfxsize=gfxsize;	
	}

	if (nodoc)
	{
		int i;
		unsigned char* ptr=(unsigned char*)&pMagBuf[0];
		for (i=0;i<magsize-4;i++)
		{
			if (ptr[i+0]==0x62 && ptr[i+1]==0x02 && ptr[i+2]==0xa2 && ptr[i+3]==0x00) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
			if (ptr[i+0]==0xa4 && ptr[i+1]==0x06 && ptr[i+2]==0xaa && ptr[i+3]==0xdf) {ptr[i+0]=0x4e;ptr[i+1]=0x71;}
		}
	}

}

