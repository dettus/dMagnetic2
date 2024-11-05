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
#include "dMagnetic2_pictures.h"
#include "dMagnetic2_graphics.h"	// for the datatypes
#include "dMagnetic2_shared.h"		// for the macros
#include "dMagnetic2_errorcodes.h"			// for the error codes

#define	PICTURE_MAX_RGB_VALUE		((1<<DMAGNETIC2_DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL)-1)

int dMagnetic2_gfxloader_atarixl(unsigned char* gfxbuf,int gfxsize,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{

	#define	ATARI_XL_PICTURE_HEIGHT	152
	#define	ATARI_XL_PICTURE_WIDTH	160
	#define	ATARI_XL_PICTURE_PIXEL_NUM	(ATARI_XL_PICTURE_HEIGHT*ATARI_XL_PICTURE_WIDTH)
	#define	ATARI_XL_PICTURE_DESCRAMBLE_LEN	(2*ATARI_XL_PICTURE_WIDTH)


	int retval;
	int picoffs;
	int idx;
	unsigned char mask;
	unsigned char byte;
	unsigned char treesize;
	int treeidx;
	int treeoffs;
	int state;
	int rgbcnt;
	int rlenum;
	int rlecnt;
	unsigned char lc;
	unsigned char rlebuf[256];
	unsigned char threebuf[3];
	unsigned int rgbs[DMAGNETIC2_GRAPHICS_MAX_COLORS];
	int threecnt;
	int pixcnt;
	int rlerep;
	int i;
	int blackcnt;
	unsigned char xorbuf[ATARI_XL_PICTURE_DESCRAMBLE_LEN];
	int xoridx;

	for (i=0;i<ATARI_XL_PICTURE_DESCRAMBLE_LEN;i++)
	{
		xorbuf[i]=0;
	}
	xoridx=0;
	retval=0;
	picoffs=READ_INT32BE(gfxbuf,4+4*picnum);
	if (picoffs==0x00000000 || picoffs>gfxsize)
	{
		retval=-1;
		return retval;
	}
	treesize=gfxbuf[picoffs];
	byte=0;
	mask=0;
	treeidx=0;
	treeoffs=picoffs+1;
	//		idx=idx+treesize*2+3;
	idx=picoffs+treesize*2+3;
	threecnt=0;
	rlenum=0;
	rlecnt=0;
	rgbcnt=0;
	pixcnt=0;
	state=0;


	blackcnt=0;
	lc=0;
	while (idx<gfxsize && pixcnt<ATARI_XL_PICTURE_PIXEL_NUM && state!=3)
	{
		unsigned char branch1,branch0,branch;
		if (mask==0)
		{
			byte=gfxbuf[idx++];
			mask=0x80;
		}
		branch1=gfxbuf[treeoffs+treeidx*2+0];
		branch0=gfxbuf[treeoffs+treeidx*2+1];
		branch=(byte&mask)?branch1:branch0;
		mask>>=1;

		if (branch&0x80)
		{
			treeidx=branch&0x7f;
		} else {
			treeidx=0;
			if (threecnt!=3)
			{
				threebuf[threecnt++]=branch;

			} else {
				int j;
				for (i=0;i<threecnt;i++)
				{
					unsigned char c;
					unsigned int rgb;
					c=threebuf[i]|((branch<<2)&0xc0);branch<<=2;
					switch (state)
					{
						case 0:	// collect rgb values
							{
#define	NUM_BASECOLOURS	16
#define	NUM_BRIGHTNESSLEVELS	16
								// the way atari colors work is by packing a basecolor and the brightness within a byte.
								// the upper 4 bits are the color.
								// the lower 4 bits are the brightness
								// what I am doing is to interpolate between the darkest and the brightest rgb values I found.

								unsigned int red_dark,green_dark,blue_dark;
								unsigned int red_bright,green_bright,blue_bright;
								int r,g,b;
								int basecolor;
								int brightness;
								const unsigned int gfx7_ataripalette[NUM_BASECOLOURS][2]=	// RGB values (10 bit)
								{
									{0x00000000,0x3e6f9be6},
									{0x10420000,0x3ffffeaa},
									{0x11419010,0x3ffe6aae},
									{0x1751f030,0x3ffdab42},
									{0x12817000,0x3ffcab7a},
									{0x124000d8,0x3ffcab7a},
									{0x120031b1,0x39ab6bff},
									{0x0141e205,0x336d3bff},
									{0x02c071e5,0x34ed1bff},
									{0x07429169,0x302ebbff},
									{0x0004b165,0x31ef6bff},
									{0x00048000,0x336fff36},
									{0x05840000,0x2f2ffe69},
									{0x0b035000,0x3caffeae},
									{0x1183a024,0x3f6f3afa},
									{0x1001a008,0x3ffdaa59}
								};
								basecolor=(c>>4)&0xf;
								brightness=(c>>0)&0xf;

								red_dark	=(gfx7_ataripalette[basecolor][0]>>(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))&0x3ff;
								green_dark	=(gfx7_ataripalette[basecolor][0]>>(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))&0x3ff;
								blue_dark	=(gfx7_ataripalette[basecolor][0]>>(0*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))&0x3ff;

								red_bright	=(gfx7_ataripalette[basecolor][1]>>(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))&0x3ff;
								green_bright	=(gfx7_ataripalette[basecolor][1]>>(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))&0x3ff;
								blue_bright	=(gfx7_ataripalette[basecolor][1]>>(0*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))&0x3ff;

								r=red_dark	+((red_bright	-red_dark)*brightness)/NUM_BRIGHTNESSLEVELS;
								g=green_dark	+((green_bright	-green_dark)*brightness)/NUM_BRIGHTNESSLEVELS;
								b=blue_dark	+((blue_bright	-blue_dark)*brightness)/NUM_BRIGHTNESSLEVELS;




								rgb=(r<<(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|(g<<(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|b;


							}
							rgbs[rgbcnt++]=rgb;
							if (c==0) blackcnt++;
							if (rgbcnt==NUM_BASECOLOURS) 
							{
								if (treesize==0x3e) state=1; else state=2;
								if (blackcnt<12) state=3;
							}
							break;
						case 1:	// rle lookup table
							if (rlenum==0) 
							{
								rlenum=c;
								rlecnt=0;
								if (rlenum==128 || rlenum==1) rlenum=0;
							} else {
								rlebuf[rlecnt++]=c;
							}
							if (rlenum==rlecnt) state=2;
							break;
						case 2:	// and the pixel information
							rlerep=0;
							for (j=0;j<rlecnt;j++)
							{
								if (c==rlebuf[j]) rlerep=j+1;
							}
							if (rlerep==0) {lc=c;rlerep=1;}
							for (j=0;j<rlerep;j++)
							{
								unsigned char p0,p1,p2,p3;
								p0=(lc>>6)&0x3;
								p1=(lc>>4)&0x3;
								p2=(lc>>2)&0x3;
								p3=(lc>>0)&0x3;

								xorbuf[(xoridx+0)]^=p0;
								xorbuf[(xoridx+1)]^=p1;
								xorbuf[(xoridx+2)]^=p2;
								xorbuf[(xoridx+3)]^=p3;

								if (pSmall!=NULL)
								{
									if (rlenum==0)
									{
										pSmall->pixels[pixcnt+0]=p0;
										pSmall->pixels[pixcnt+1]=p1;
										pSmall->pixels[pixcnt+2]=p2;
										pSmall->pixels[pixcnt+3]=p3;
									} else {
										pSmall->pixels[pixcnt+0]=xorbuf[(xoridx+0)];
										pSmall->pixels[pixcnt+1]=xorbuf[(xoridx+1)];
										pSmall->pixels[pixcnt+2]=xorbuf[(xoridx+2)];
										pSmall->pixels[pixcnt+3]=xorbuf[(xoridx+3)];
									}
								}
								if (pLarge!=NULL)
								{
									if (rlenum==0)
									{
										pLarge->rgbpixels[pixcnt+0]=rgbs[p0];
										pLarge->rgbpixels[pixcnt+1]=rgbs[p1];
										pLarge->rgbpixels[pixcnt+2]=rgbs[p2];
										pLarge->rgbpixels[pixcnt+3]=rgbs[p3];
									} else {
										pLarge->rgbpixels[pixcnt+0]=rgbs[xorbuf[(xoridx+0)]];
										pLarge->rgbpixels[pixcnt+1]=rgbs[xorbuf[(xoridx+1)]];
										pLarge->rgbpixels[pixcnt+2]=rgbs[xorbuf[(xoridx+2)]];
										pLarge->rgbpixels[pixcnt+3]=rgbs[xorbuf[(xoridx+3)]];
									}
								}
								pixcnt+=4;
								xoridx=(xoridx+4)%ATARI_XL_PICTURE_DESCRAMBLE_LEN;
							}
							break;
					}
				}
				threecnt=0;
			}

		}
	}
	if (pSmall!=NULL)
	{
		pSmall->width=ATARI_XL_PICTURE_WIDTH;
		pSmall->height=ATARI_XL_PICTURE_HEIGHT;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=rgbs[i];
		}
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}
	if (pLarge!=NULL)
	{
		pLarge->width=ATARI_XL_PICTURE_WIDTH;
		pLarge->height=ATARI_XL_PICTURE_HEIGHT;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_WIDEN;
	}

	return retval;
}




