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

#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_pictures_msdos.h"
#include "dMagnetic2_shared.h"	// for the macros
#include "dMagnetic2_graphics.h"	// for the datatypes
#include "dMagnetic2_errorcodes.h"

#define	PICTURE_MAX_RGB_VALUE		((1<<DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL)-1)

int dMagnetic2_gfxloader_msdos(unsigned char* gfxbuf,int gfxsize,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	//  0.. 3: 4 bytes "MaP3"
	//  4.. 8: 4 bytes length of index
	//  8..11: 4 bytes length of disk1.pix
	// 11..15: 4 bytes length of disk2.pix
	// then the index
	// then the disk1.pix data
	// then the disk2.pix data
	int retval;
	int offs1;
	int offs2;
	int offset;
	int indexoffs,indexlen;
	int disk1offs,disk1len;
	int disk2offs;
	int i,n;

	int huffsize;
	int treeidx;
	int byteidx;
	int unhufcnt;
	int pixelcnt;
	int state;

	unsigned char mask;
	unsigned char byte;
	unsigned int unpackedsize;
	int max_stipple;
	unsigned char pl_lut[128]={0};	// lookup table for left pixels
	unsigned char pr_lut[128]={0};	// lookup table for right pixels
	unsigned char xorbuf[DMAGNETIC2_GRAPHICS_MAX_WIDTH*2]={0};	// ring buffer, to perform an XOR over two lines of stipples
	unsigned char last_stipple;
	int state_cnt;
	int height,width;
	unsigned int rgbs[DMAGNETIC2_GRAPHICS_MAX_COLORS];
	if (!gfxsize) return 0;		// there is no picture data available. nothing to do.

	picnum&=0xffff;
//	pPicture->pictureType=PICTURE_HALFTONE;	// this format offers half tones.

	indexlen=READ_INT32BE(gfxbuf, 4);
	disk1len=READ_INT32BE(gfxbuf, 8);
	//	disk2len=READ_INT32BE(gfxbuf,12);

	indexoffs=16;
	disk1offs=indexoffs+indexlen;
	disk2offs=disk1offs+disk1len;

	retval=0;

	// step 1: find the offset of the picture within the index.
	// the way it is stored is that the offsets within disk1 are stored in the first half,
	// and the offsets for disk2 are in the second half.
	// in case the offset is -1, it must be in the other one.
	offs1=(int)READ_INT32LE(gfxbuf,indexoffs+picnum*4);
	offs2=(int)READ_INT32LE(gfxbuf,indexoffs+indexlen/2+picnum*4);
	if (picnum!=30 && offs1!=-1 && offs2!=-1) offs2=-1;	// in case one picture is stored on both disks, prefer the first one.

	if (picnum==30 && offs1==-1 && offs2==-1) offs1=0;	// special case: the title screen for the GUILD of thieves is the first picture in DISK1.PIX
	if (offs1!=-1) offset=offs1+disk1offs;			// in case the index was found in the first half, use disk1
	else if (offs2!=-1) offset=offs2+disk2offs;		// in case the index was found in the second half, use disk2
	else return DMAGNETIC2_ERROR_WRONG_PICTUREFORMAT;	///  otherwise: ERROR



	if (offset>gfxsize) 	// this is MYTH: there is only a single image file.
	{
		offset=offs1;
	}

	// the picture is stored in layers.
	// the first layer is a huffmann table.
	// this unpacks the second layer, which contains repitions
	// and a "stipple" table. from this, the actual pixels are being 
	// calculated.

	huffsize=gfxbuf[offset+0];
	unpackedsize=READ_INT16BE(gfxbuf,offset+huffsize+1);
	unpackedsize*=4;
	unpackedsize+=3;
	unpackedsize&=0xffff;	// it was designed for 16 bit machines.

	pixelcnt=-1;
	unhufcnt=0;
	state=0;
	treeidx=0;
	mask=0;
	byteidx=offset+huffsize+2+1;	// the beginning of the bitstream starts after the huffmann table and the unpackedsize
	byte=0;
	width=0;
	height=0;
	memset(xorbuf,0,sizeof(xorbuf));	// initialize the xor buffer with 0
	state_cnt=0;
	max_stipple=last_stipple=0;

	while (unhufcnt<unpackedsize && (pixelcnt<(2*width*height)) && byteidx<gfxsize)
	{
		// first layer: the bytes for the unhuf buf are stored as a bitstream, which are used to traverse a huffmann table.
		unsigned char branch1,branch0,branch;
		if (mask==0)
		{
			byte=gfxbuf[byteidx];
			byteidx++;
			mask=0x80;			// MSB first
		}
		branch1=gfxbuf[offset+1+treeidx*2+0];
		branch0=gfxbuf[offset+1+treeidx*2+1];
		branch=(byte&mask)?branch1:branch0;
		mask>>=1;				// MSB first.
		if (branch&0x80)		// leaves have the highest bit set. terminal symbols only have 7 bit.
		{
			treeidx=0;
			branch&=0x7f;	// terminal symbols have 7 bit
					//
					//
					// the second layer begins here
			switch (state)
			{
				case 0:	// first state: the ID should be "0x77"
					if (branch!=0x77)	return -1;	// illegal format
					state=1;
					break;
				case 1: // second byte is the number of "stipples"
					max_stipple=branch;
					state=2;
					break;
				case 2:	// width, stored as 2*6 bit Big Endian
					width<<=6;	// 2*6 bit. big endian;
					width|=branch&0x3f;
					state_cnt++;
					if (state_cnt==2)	state=3;
					break;
				case 3:	// height, stored as 2*6 bit Big Endian
					height<<=6;	// 2*6 bit. big endian;
					height|=branch&0x3f;
					state_cnt++;
					if (state_cnt==4)
					{
						if (height<=0 || width<=0) 	return DMAGNETIC2_ERROR_WRONG_PICTUREFORMAT;	// error in decoding the height and the width
						pixelcnt=0;
						state_cnt=0;
						state=4;
					}
					break;
				case 4:	// rgb values
					{
						unsigned char halftonelut[4]={0,2,5,7};
						unsigned int red,green,blue;


						red  =(branch>>4)&0x3;
						green=(branch>>2)&0x3;
						blue =(branch>>0)&0x3;

						red	=(halftonelut[red]*PICTURE_MAX_RGB_VALUE)/7;
						green	=(halftonelut[green]*PICTURE_MAX_RGB_VALUE)/7;
						blue	=(halftonelut[blue]*PICTURE_MAX_RGB_VALUE)/7;


						rgbs[state_cnt++]=(red<<(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|(green<<(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|blue;
						
					}
					if (state_cnt==16) 
					{
						state_cnt=0;
						state=5;
					}
					break;
				case 5:	// lookup-table to retrieve the left pixel value from the stipple
					pl_lut[state_cnt++]=branch;
					if (state_cnt==max_stipple)
					{
						state_cnt=0;
						state=6;
					}
					break;
				case 6:	// lookup-table to retrieve the right pixel value from the stipple
					pr_lut[state_cnt++]=branch;
					if (state_cnt==max_stipple)
					{
						last_stipple=0;
						state_cnt=0;
						state=7;
					}
					break;
				case 7:
				case 8:
					// now for the stipple table
					// this is actually a third layer of encoding.
					// it contains terminal symbols [0... max_stipple)
					// 
					// if the symbol is <max_stipple, it is a terminal symbol, a stipple
					// if the symbol is =max_stipple, it means that the next byte is being escaped
					// if the symbol is >max_stipple, it means that the previous symbol is being repeated.
					n=0;
					if (state==8)	// this character has been "escaped"
					{
						state=7;
						n=1;
						last_stipple=branch;
					}
					else if (branch<max_stipple)
					{
						last_stipple=branch;	// store this symbol for the next repeat instruction
						n=1;
					} else {
						if (branch==max_stipple)	// "escape" the NEXT symbol. use it, even though it might be >=max_stipple.
						{			// this is necessary for the XOR operation.
							state=8;
							n=0;
						} else if (branch>max_stipple) 
						{
							n=branch-max_stipple;	// repeat the previous stipple
							branch=last_stipple;
						}
					}
					for (i=0;i<n;i++)
					{
						unsigned char x;
						xorbuf[state_cnt]^=branch;			// descramble the symbols
						x=xorbuf[state_cnt];
						state_cnt=(state_cnt+1)%(2*width);
						if (pSmall!=NULL)
						{
							pSmall->pixels[pixelcnt+0]=pl_lut[x];
							pSmall->pixels[pixelcnt+1]=pr_lut[x];
						}
						if (pLarge!=NULL)
						{
							pLarge->rgbpixels[pixelcnt+0]=rgbs[pl_lut[x]];
							pLarge->rgbpixels[pixelcnt+1]=rgbs[pr_lut[x]];
						}
						pixelcnt+=2;
					}
					break;
			}
		} else {
			treeidx=branch;	// non terminal -> traverse the tree further down
		}
	}
	if (pSmall!=NULL)
	{
		pSmall->height=height;
		pSmall->width=width*2;
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_HALFTONE;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=rgbs[i];
		}
	}
	if (pLarge!=NULL)
	{
		pLarge->height=height;
		pLarge->width=width*2;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_HALFTONE;
	}
		

	return retval;
}


