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

int dMagnetic2_gfxloader_appleii(unsigned char* gfxbuf,int gfxsize,unsigned char* tmpbuf,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
#define PICTURE_HOTFIX1         0x80000000
#define PICTURE_HOTFIX2         0x40000000
#define PICTURE_HOTFIX3         0x20000000
#define	APPLE2_COLOURS		16

#define	SIZE_AUX_MEM		8192
#define SIZE_MAIN_MEM		8192
#define	APPLE_II_WIDTH		140
#define	APPLE_II_HEIGHT		(192-32)
#define	BITS_PER_SYMBOL		7
#define	PIXELS_PER_WHATEVER	7	// 4 terminal symbols result in 7 pixels
#define	SYMBOLS_PER_LINE	(APPLE_II_WIDTH/BITS_PER_SYMBOL)
#define	APPLE_II_PIXELS		(APPLE_II_HEIGHT*APPLE_II_WIDTH)


	// approximation of the fixed Apple II palette with 10 bit RGB values
	const unsigned int gfx8_apple2_palette[APPLE2_COLOURS]={// 10 bit per channel
		0x00000000,	// black
		0x1814e2f6,	// dark blue
		0x000a3581,	// dark green
		0x050cfbf6,	// medium blue

		0x1817240c,	// brown
		0x2719c671,	// dark grey
		0x050f58f0,	// light green
		0x1c9fff42,	// aquamarin

		0x38e1e181,	// deep red
		0x3ff443f6,	// purple
		0x2719c671,	// light grey
		0x342c3bff,	// light blue

		0x3ff6a4f0,	// orange
		0x3ffa0742,	// pink
		0x342dda35,	// yellow
		0x3fffffff	// white
	};
	int retval=0;
	int i=0;
	unsigned int picoffs=0;
	unsigned int treeoffs=0;
	int hotfix=0;
	int outidx=0;
	int treeidx=0;
	unsigned char lastterm=0;
	unsigned char mask=0;
	unsigned char byte=0;
	int bitidx=0;
	int oidx=0;
	unsigned int pixreg=0;
	int colcnt=0;
	int linecnt=0;

	retval=0;
	picoffs=READ_INT32BE(gfxbuf,4*picnum+4);
	treeoffs=picoffs;
	if (picoffs==0x0000000)
	{
		retval=-1;
		return retval;
	}


	for (i=0;i<APPLE_II_PIXELS;i++)
	{
		tmpbuf[i]=0;
	}
	hotfix=(treeoffs&0xe0000000);
	if (hotfix==PICTURE_HOTFIX1) hotfix=-1;
	if (hotfix==PICTURE_HOTFIX2) hotfix= 1;
	if (hotfix==PICTURE_HOTFIX3) hotfix= 2;
	treeoffs&=0x1ffffff;
	treeoffs+=1;

	if (treeoffs>gfxsize)
	{
		retval=-1;
		return retval;
	}

	// step 1: unhuffing with the RLE
	outidx=0;
	treeidx=0;
	bitidx=treeoffs+gfxbuf[treeoffs-1]+2+hotfix;

	lastterm=1;
	mask=0;
	byte=0;

	// at the unhuffptr, there is now the content which would have
	// been written into the Apple II Videoram at $2000.
	// the first 8192 bytes are the AUX memory bank.
	// the second 8192 bytes are the MAIN memory bank

	oidx=0;
	colcnt=0;
	linecnt=0;

	while (outidx<(SIZE_AUX_MEM+SIZE_MAIN_MEM) && bitidx<=gfxsize)
	{
		unsigned char branch1,branch0;
		unsigned char branch=0;

		if (mask==0)
		{
			mask=0x80;
			byte=gfxbuf[bitidx++];
		}
		branch1=gfxbuf[treeoffs+0+2*treeidx];
		branch0=gfxbuf[treeoffs+1+2*treeidx];
		branch=(byte&mask)?branch1:branch0;mask>>=1;
		if (branch&0x80)
		{
			unsigned char terminal;
			int n;
			terminal=branch&0x7f;
			if (lastterm==0 && outidx>3)
			{
				n=terminal-1;
				terminal=0;
				lastterm=1;
			} else {
				n=1;
				lastterm=terminal;
			}
			for (i=0;i<n && outidx<(SIZE_AUX_MEM+SIZE_MAIN_MEM);i++)
			{
				// 4*7 bit terminal symbols make up 7 pixels (4 Bit each)
				// however, they are spread out over 2 memory banks: AUX and MAIN memory.
				// mmmmmmmAAAAAAAmmmmmmmAAAAAAA
				if (!(outidx&1))
				{
					pixreg=terminal;
				} else {
					pixreg|=((unsigned int)terminal)<<(2*BITS_PER_SYMBOL);
					if (outidx>=SIZE_AUX_MEM)	// are we already in the MAIN memory?
					{			// the first 8192 output bytes were meant for the AUX memory
						pixreg<<=BITS_PER_SYMBOL;	// in MAIN memory, the other bits are written
					}
					if (colcnt==SYMBOLS_PER_LINE)		// at the end of the line
					{
						colcnt=0;
						linecnt++;
						if ((linecnt&3)==3)	// every 4th line does not exist
						{
							linecnt++;
							colcnt-=4;      // skip 4 terminal words
						}
						// calculate the line address
						oidx =((linecnt>>0)&0x3)<<6;            // bit 0,1 --> bit 6,7
						oidx|=((linecnt>>2)&0x7)<<3;            // bit 2,3,4 --> bit 3,4,5
						oidx|=((linecnt>>5)&0x7)<<0;            // bit 5,6,7 --> bit 0,1,2

						// line->linear buffer
						oidx*=APPLE_II_WIDTH;
					}

					if (colcnt>=0)
					{
						// the pixel information is spread out over AUX and MAIN memory.
						// 4*7=28 bits are used to store 7 pixels.
						// the bits are being read LSB  first, but only 7 bits of a byte are being used.
						// 
						// A0..A6 are the bits from the first byte in the AUX memory. Starting at 0x0000
						// B0..B6 are the bits from the second byte in the AUX memory.
						// M0..M6 are the bits from the first byte in the MAIN memory. Starting at 0x2000
						// N0..N6 are the bits from the second byte in the MAIN memory.
						tmpbuf[oidx+ 0]|=(pixreg&0xf);pixreg>>=4;	// A0 A1 A2 A3
						tmpbuf[oidx+ 1]|=(pixreg&0xf);pixreg>>=4;	// A4 A5 A6 M0
						tmpbuf[oidx+ 2]|=(pixreg&0xf);pixreg>>=4;	// M1 M2 M3 M4
						tmpbuf[oidx+ 3]|=(pixreg&0xf);pixreg>>=4;	// M5 M6 B0 B1
						tmpbuf[oidx+ 4]|=(pixreg&0xf);pixreg>>=4;	// B2 B3 B4 B5
						tmpbuf[oidx+ 5]|=(pixreg&0xf);pixreg>>=4;	// B6 N0 N1 N2
						tmpbuf[oidx+ 6]|=(pixreg&0xf);pixreg>>=4;	// N3 N4 N5 N6

						oidx+=PIXELS_PER_WHATEVER;
					}
					colcnt++;
				}
				outidx++;
				if (outidx==SIZE_AUX_MEM)
				{
					linecnt=0;
					colcnt=0;
					oidx=0;
				}
			}
			treeidx=0;
		} else {
			treeidx=branch;
		}
	}
	if (pSmall!=NULL)
	{
		pSmall->height=APPLE_II_HEIGHT;
		pSmall->width=APPLE_II_WIDTH;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=gfx8_apple2_palette[i];
		}
		for (i=0;i<APPLE_II_PIXELS;i++)
		{
			pSmall->pixels[i]=tmpbuf[i];
		}
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}
	if (pLarge!=NULL)
	{
		pLarge->height=APPLE_II_HEIGHT;
		pLarge->width=APPLE_II_WIDTH;
		for (i=0;i<APPLE_II_PIXELS;i++)
		{
			pLarge->pixels[i]=gfx8_apple2_palette[tmpbuf[i]];
		}
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}
	return retval;

}
