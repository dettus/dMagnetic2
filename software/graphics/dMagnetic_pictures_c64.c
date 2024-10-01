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

// the Commodore C64 pictures
// The c64 pictures consist of two parts: the bitmap and the colour map. essentially, each 8x8 block can be rendered with 4 colours. 
// 2 of them are fixed, the others are determined by the colourmap.
int dMagnetic2_gfxloader_c64(unsigned char* gfxbuf,int gfxsize,unsigned char* tmpbuf,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
#define	C64_PICWIDTH			160
#define	C64_PICHEIGHT			152
#define	C64_ROWS_PER_BLOCK		8		// on the C64, graphics are rendered by using one of 4 colours in each 8x8 block.
#define	C64_PIXELPERBYTE		(8/2)		// 4 colours require 4 bitmasks, so it is essentally a 4x8 pixel block
#define	C64_BYTES_BITMAP		(C64_PICWIDTH*C64_PICHEIGHT/C64_PIXELPERBYTE)
#define	C64_BYTES_BITMAP_PAWN		((C64_PICWIDTH*C64_PICHEIGHT/C64_PIXELPERBYTE)+64)	// THE Pawn had a little bit padding
#define	C64_BYTES_COLOURMAP_V0		380		// 
#define	C64_BYTES_COLOURMAP_V1		(2*C64_BYTES_COLOURMAP_V0)
#define	C64_BYTES_COLOURMAP_HIGH	760
#define	C64_MAXBYTES_PICTURE_V0		(C64_BYTES_BITMAP+C64_BYTES_COLOURMAP_V0+C64_BYTES_COLOURMAP_HIGH)
#define	C64_MAXBYTES_PICTURE_V1		(C64_BYTES_BITMAP+C64_BYTES_COLOURMAP_V1+C64_BYTES_COLOURMAP_HIGH)
#define	C64_MAXBYTES_PICTURE		C64_MAXBYTES_PICTURE_V1
#define	C64_BYTES_IN_BITMAP		(C64_PICWIDTH*C64_PICHEIGHT/C64_PIXELPERBYTE)
	int blockidx;
	unsigned char colour[4]={0};
	int format;
	int i;
	int picoffs;
	int retval;
	// approximation of the fixed C64 palette with 10bit RGB values
	const unsigned int gfx5_rgbvalues[16]={
		0x00000000,	// black
		0x3fffffff,	// white
		0x205330e0,	// red
		0x1d5ceb22,	// cyan

		0x2393c25d,	// violet
		0x159ac934,	// green
		0x0b82c26d,	// blue
		0x3b6f19c5,	// yellow

		0x239500a4,	// orange
		0x15538000,	// brown
		0x3126c5c5,	// light red
		0x1284a128,	// grey 1

		0x1ed7b5ed,	// grey 2
		0x2a5ffe7d,	// light green
		0x1c16d7ae,	// light blue
		0x2cab2aca};	// grey 3

	blockidx=0;

	retval=0;
	picoffs=READ_INT32BE(gfxbuf,4+4*picnum);
	if (picoffs<=0x00000000 || picoffs>gfxsize)
	{
		retval=-1;	// the picture number was not correct
		return retval;
	} 


	format=0;

	///////////// dehuff /////////////
	{
		int outcnt;
		int expected;
		int treeidx;
		int bitcnt;
		int byteidx;
		int threecnt;
		int rlenum;
		int rlecnt;
		unsigned char rlebuf[256]={0};
		unsigned char threebuf[3]={0};
		unsigned char* ptr;
		unsigned char byte=0;
		unsigned char rlechar;

		treeidx=0;
		expected=C64_BYTES_BITMAP;
		outcnt=-1;
		bitcnt=0;
		ptr=&gfxbuf[picoffs];
		byteidx=1+(ptr[0]+1)*2;	// the first byte is the size of the huffmann tree. After the huffmann tree, the bit stream starts.
		threecnt=0;
		rlechar=0;

		rlenum=0;
		rlecnt=0;

		while (outcnt<expected)
		{
			unsigned char branch1,branch0,branch;
			if (bitcnt==0)
			{
				bitcnt=8;
				byte=ptr[byteidx++];
			}
			branch1=ptr[2*treeidx+1];
			branch0=ptr[2*treeidx+2];
			branch=(byte&0x80)?branch1:branch0;
			byte<<=1;bitcnt--;
			if (branch&0x80)
			{
				treeidx=branch&0x7f;
			} else {
				treeidx=0;
				if (threecnt==3)
				{
					int i;
					threecnt=0;
					for (i=0;i<3;i++) 
					{
						threebuf[i]|=((branch<<2)&0xc0);
						branch<<=2;
					}
					if (outcnt==-1)
					{
						outcnt=0;
						if (version==0) 	// PAWN specific
						{
							colour[0]=threebuf[0]&0xf;	// for when the bitmask is 00
							colour[3]=threebuf[1]&0xf;	// for when the bitmask is 11
							rlenum=threebuf[2];
							expected=C64_BYTES_BITMAP_PAWN+C64_BYTES_COLOURMAP_HIGH;
						} else {
							format=threebuf[0];
							if (threebuf[0]==0x00)
							{
								expected=C64_MAXBYTES_PICTURE_V0;	// after the bitmask comes the colourmap
								rlenum=0;
								rlecnt=0;
								rlechar=tmpbuf[outcnt++]=threebuf[2];
							} else {
								colour[0]=threebuf[1]&0xf;	// for when the bitmask is 00
								colour[3]=threebuf[1]&0xf;	// for when the bitmask is 11
								expected=C64_MAXBYTES_PICTURE_V1;	// after the bitmask comes the colourmap
								rlecnt=0;
								rlenum=threebuf[2];
							}
						}
					} else {
						for (i=0;i<3;i++)
						{
							if (rlecnt<rlenum) 
							{
								rlebuf[rlecnt++]=threebuf[i];
							} else {
								int j;
								int rle;
								rle=0;
								for (j=0;j<rlecnt;j++)
								{
									if (rlebuf[j]==threebuf[i]) rle=(j+1);
								}
								if (rle)
								{
									for (j=0;j<rle;j++)
									{
										if (outcnt<expected) tmpbuf[outcnt++]=rlechar;
									}
								} else {
									if (outcnt<expected) rlechar=tmpbuf[outcnt++]=threebuf[i];
								}
							}
						}
					}
				} else {
					threebuf[threecnt++]=branch;
				}
			}
		}
	}

	///////////// render the picture ///////////////
	// the bitmap consists of 4 pixels per byte.
	// the pixels are ordered as rows, so the 2 bytes A=aabbccdd B=eeffgghh are being rendered as
	// aaee
	// bbff
	// ccgg
	// ddhh
	{
		int colidx;
		int maskidx;
		int x,y;
		int i,j;
		
		int screenram_idx;
		int colourram_idx;

		x=0;
		y=0;

		if (version==0)
		{
			screenram_idx=C64_BYTES_BITMAP_PAWN;
			colourram_idx=-1;
		} else {
			screenram_idx=C64_BYTES_BITMAP+((format==0x00)?C64_BYTES_COLOURMAP_V0:C64_BYTES_COLOURMAP_V1);
			colourram_idx=C64_BYTES_BITMAP;
		}
		for (maskidx=0,colidx=0;maskidx<C64_BYTES_IN_BITMAP;maskidx+=8,colidx++)
		{
			// prepare everything for rendering a 4x8 block
			colour[1]=(tmpbuf[screenram_idx+colidx]>>4)&0xf;
			colour[2]=(tmpbuf[screenram_idx+colidx]>>0)&0xf;
			if (version!=0)
			{
				if (format==0x00)
				{
					colour[3]=tmpbuf[colourram_idx+colidx/2];
					if ((colidx%2)==0)
					{
						colour[3]>>=4;
					}
				} else {
					colour[3]=tmpbuf[colourram_idx+colidx];
				}
				colour[3]&=0xf;
			}


			for (i=0;i<8;i++)
			{
				int y2;
				unsigned char mask;
				y2=y+i;
				mask=tmpbuf[maskidx+i];
				for (j=0;j<4;j++)
				{
					int x2;
					unsigned char col;
					x2=x+j;
					col=colour[(mask>>6)&0x3];
					if (pSmall!=NULL)
					{
						pSmall->pixels[x2+y2*(pPicture->width)]=col;
					}
					if (pLarge!=NULL)
					{
						pLarge->pixels[x2+y2*(pPicture->width)]=gfx5_rgbvalues[col];
					}
					mask<<=2;
				}
			}
			x+=4;
			if (x==C64_PICWIDTH) 
			{
				x=0;
				y+=8;
			}
		}
	}
	if (pSmall!=NULL)
	{
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64|DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=gfx5_rgbvalues[i];
		}
		pSmall->width=C64_PICWIDTH;
		pSmall->height=C64_PICHEIGHT;
	}
	if (pLarge!=NULL)
	{
		pLarge->width=C64_PICWIDTH;
		pLarge->height=C64_PICHEIGHT;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_C64|DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}
	return retval;
}


