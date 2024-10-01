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

#include "dMagnetic2_pictures.h"
#include "dMagnetic2_graphics.h"	// for the datatypes
#include "dMagnetic2_shared.h"		// for the macros
#include "dMagnetic2.h"			// for the error codes

#define	PICTURE_MAX_RGB_VALUE		((1<<DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL)-1)

int dMagnetic2_gfxloader_amstrad_cpc(unsigned char* gfxbuf,int gfxsize,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	#define	AMSTRAD_CPC_PICTURE_HEIGHT	152
	#define	AMSTRAD_CPC_PICTURE_WIDTH	160
	#define	AMSTRAD_CPC_PICTURE_PIXEL_NUM	(AMSTRAD_CPC_PICTURE_HEIGHT*AMSTRAD_CPC_PICTURE_WIDTH)
	#define	AMSTRAD_CPC_PICTURE_DESCRAMBLE_LEN	(2*AMSTRAD_CPC_PICTURE_WIDTH)

	const unsigned char gfx6_codebook[16]={0x00,0x40,0x04,0x44,0x10,0x50,0x14,0x54,0x01,0x41,0x05,0x45,0x11,0x51,0x15,0x55};
	// approximation of the fixed CPC palette with 10bit RGB values
	const unsigned int gfx6_rgbvalues[27]={
		0x00000000,0x00000201,0x000003ff,
		0x20100000,0x20100201,0x201003ff,
		0x3ff00000,0x3ff00201,0x3ff003ff,

		0x00080400,0x00080601,0x000807ff,
		0x20180400,0x20180601,0x201807ff,
		0x3ff80400,0x3ff80601,0x3ff807ff,

		0x000ffc00,0x000ffe01,0x000fffff,
		0x201ffc00,0x201ffe01,0x201fffff,
		0x3ffffc00,0x3ffffe01,0x3fffffff
	};

	int i;
	int paletteidx;
	int outidx;
	unsigned char byte;
	unsigned char mask;
	unsigned char symbol;
	unsigned char code;
	int bitidx;
	int toggle;
	int picoffs; 
	int treeidx;
	int retval;
	unsigned char xorbuf[AMSTRAD_CPC_PICTURE_DESCRAMBLE_LEN];	// the pixels are xored over 2 lines
	int xoridx;
	unsigned int rgbs[DMAGNETIC2_GRAPHICS_MAX_COLORS];


	for (i=0;i<AMSTRAD_CPC_PICTURE_DESCRAMBLE_LEN;i++)
	{
		xorbuf[i]=0;
	}
	xoridx=0;



	retval=0;
	// find the index within the gfx buffer
	picoffs=READ_INT32BE(gfxbuf,4+picnum*4);
	if (picoffs==0x00000000 || picoffs>gfxsize)
	{
		retval=-1;	// the picture number was not correct
		return retval;
	} 


	treeidx=0;
	paletteidx=0;
	outidx=0;
	byte=0;
	bitidx=picoffs+1+(gfxbuf[picoffs]+1)*2;
	mask=0;
	rgbs[paletteidx++]=gfx6_rgbvalues[ 0];  // black
	rgbs[paletteidx++]=gfx6_rgbvalues[26];  // bright white
	symbol=0;
	code=0;
	toggle=0;
	while ((outidx<AMSTRAD_CPC_PICTURE_PIXEL_NUM)&& (bitidx<gfxsize||mask))
	{
		unsigned char branch1,branch0;
		unsigned char branch;

		if (mask==0x00)
		{
			byte=gfxbuf[bitidx++];
			mask=0x80;
		}

		branch1=gfxbuf[picoffs+1+2*treeidx];
		branch0=gfxbuf[picoffs+2+2*treeidx];
		branch=(byte&mask)?branch1:branch0;
		mask>>=1;
		if (branch&0x80)
		{
			treeidx=0;
			branch&=0x7f;
			if (paletteidx<16)      // the first two colours are fixed. and the rest comes from the first 14 terminal symbols
			{
				rgbs[paletteidx++]=gfx6_rgbvalues[branch];	// one of them
			} else {
				int loopcnt;
				loopcnt=1;
				if (branch&0x70)	// if bits 6..4 are set, it is a loop. it determines how often the previous code is being repeated
				{
					loopcnt=branch-0x10;
				} else {	// otherwise, it is a code 
					code=gfx6_codebook[branch];
				}
				for (i=0;i<loopcnt && outidx<(AMSTRAD_CPC_PICTURE_PIXEL_NUM);i++)
				{
					// the symbol is being combined from two codes
					symbol<<=1;
					symbol|=code;

					toggle=1-toggle;
					// when the symbol is finished
					if (toggle==0)
					{
						unsigned char p0;
						unsigned char p1;
						// the images are Amstrad Mode 0 pictures. Which means that the pixel bits are being interleaved in one byte.
						p0 =((symbol>>7)&0x1)<<0;
						p0|=((symbol>>3)&0x1)<<1;
						p0|=((symbol>>5)&0x1)<<2;
						p0|=((symbol>>1)&0x1)<<3;

						p1 =((symbol>>6)&0x1)<<0;
						p1|=((symbol>>2)&0x1)<<1;
						p1|=((symbol>>4)&0x1)<<2;
						p1|=((symbol>>0)&0x1)<<3;
						// at this point, the two pixels have been separated
						xorbuf[xoridx+0]^=p0;
						xorbuf[xoridx+1]^=p1;


						if (pSmall!=NULL)
						{
							pSmall->pixels[outidx+0]=xorbuf[xoridx+0];
							pSmall->pixels[outidx+1]=xorbuf[xoridx+1];
						}

						if (pLarge!=NULL)
						{
							pLarge->pixels[outidx+0]=rgbs[xorbuf[xoridx+0]];
							pLarge->pixels[outidx+1]=rgbs[xorbuf[xoridx+1]];
						}

						outidx+=2;
						xoridx=(xoridx+2)%AMSTRAD_CPC_PICTURE_DESCRAMBLE_LEN;

						// prepare the next symbol
						symbol=0;
					}
				}
			}
		} else {
			treeidx=branch;
		}
	}
	if (pSmall!=NULL)
	{
		pSmall->height=AMSTRAD_CPC_PICTURE_HEIGHT;
		pSmall->width =AMSTRAD_CPC_PICTURE_WIDTH;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=rgbs[i];
		}
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}
	if (pLarge!=NULL)
	{
		pLarge->height=AMSTRAD_CPC_PICTURE_HEIGHT;
		pLarge->width =AMSTRAD_CPC_PICTURE_WIDTH;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}
	return retval;
}


