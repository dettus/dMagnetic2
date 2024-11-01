/*

Copyright 2024, dettus@dettus.net

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <string.h>
#include <stdlib.h>
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_graphics.h"

#define	MAGIC		0x28816

#define	MAXPICWIDTH	512
#define	NUM_COLORS	16
#define	TREESIZE	609		// 

#define	PICTURE_MAX_RGB_VALUE	1023
#define	DMAGNETIC2_GRAPHICS_WRONG_CEL	-17

typedef struct _tdMagnetic2_animation_handle
{
	int magic;
	// the information about the current animation
	unsigned char linebuf[MAXPICWIDTH];
	unsigned int rgbs[NUM_COLORS];
	int width;
	int height;
	int celnum;
	unsigned char* pGfxBuf;
	int gfxsize;
} tdMagnetic2_animation_handle;


int dMagnetic2_animation_magwin_addcel(tdMagnetic2_animation_handle *pThis,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge,
	int offset,int celidx,int xpos,int ypos,int magic)
{
	int width;
	int height;
	int transparency;
	int animidx;
	int j;
	int blocksize;
	int celnum;
	int x;
	unsigned char linebuf[MAXPICWIDTH];
	unsigned int rgbs[NUM_COLORS];

	// the header of a picture is
	// 609 bytes Tree
	// 16*2 bytes RGB values
	// 4 bytes??
	// 2 bytes width
	// 2 bytes height

	celnum=0;
	animidx=offset+TREESIZE+(NUM_COLORS*2)+4;	// skip most of the header
	memset(pThis->linebuf,0,sizeof(pThis->linebuf));

	blocksize=READ_INT16LE(pThis->pGfxBuf,animidx+6);

	if (celidx==0)	// this is the background cel. initialize the picture
	{
		for (j=0;j<NUM_COLORS;j++)
		{
			int red,green,blue;
			unsigned short rgb;
			rgb=READ_INT16LE(pThis->pGfxBuf,offset+TREESIZE+j*2+4);
			red=	(rgb>>8)&0xf;
			green=	(rgb>>4)&0xf;
			blue=	(rgb>>0)&0xf;

			// convert to 10 bit
			red*=	PICTURE_MAX_RGB_VALUE;
			green*=	PICTURE_MAX_RGB_VALUE;
			blue*=	PICTURE_MAX_RGB_VALUE;

			red/=	7;
			green/=	7;
			blue/=	7;

			pThis->rgbs[j]=(red<<20)|(green<<10)|blue;
		}
		pThis->width=	READ_INT16LE(pThis->pGfxBuf,animidx+0);
		pThis->height=	READ_INT16LE(pThis->pGfxBuf,animidx+2);
		pThis->celnum=	READ_INT16LE(pTHis->pGfxBuf,animidx+blocksize+8);
	}

	if (celidx>=pThis->celnum)
	{
		return DMAGNETIC2_GRAPHICS_WRONG_CEL;
	}

	animidx+=4;	// the first cel has some information
	animidx+=(celidx*(blocksize+8));	// the other information is stored in blocks with some heaeders

	width=	READ_INT16LE(pThis->pGfxBuf,animidx+0);
	height= READ_INT16LE(pThis->pGfxBuf,animidx+2);
	transparency=	READ_INT16LE(pThis->pGfxBuf,animidx+4);
	blocksize=	READ_INT16LE(pThis->pGfxBuf,animidx+6);

	if (pSmall!=NULL)
	{
		pSmall->width=width;
		pSmall->height=height;
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_NONE;
		for (i=0;i<MAX_COLORS;i++)
		{
			pSmall->rgb[i]=pThis->rgbs[i];	
		}
	}
	if (pLarge!=NULL)
	{
		pLarge->width=width;
		pLarge->height=height;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_NONE;
	}

	// patch away some transparency encoding i have not understood.
	if (transparency==21 && magic==1) transparency==65536;
	if (transparency==21) transparency=0;

	// at this point, the transparency denotes the color which is not drawn

#define	TWOSCOMPLEMENT	65536
	{
		unsigned char rlechar;
		int pixcnt;
		int rlestate;
		int rlenum;
		unsigned char byte;
		unsigned char mask;
		int treeidx;
		int bitidx;

		bitidx=picidx+8;
		pixcnt=0;
		rlestate=0;
		byte=0;
		mask=0;
		treeidx=0;
		rlenum=1;

		x=0;
		if (xpos>=(TWOSCOMPLEMENT-pPicture->width))
		{
			xpos=xpos-TWOSCOMPLEMENT;
		}
		if (ypos>=(TWOSCOMPLEMENT-pPicture->height))
		{
			ypos=ypos-TWOSCOMPLEMENT;
		}
		rlechar=gfxbuf[offset+0x240];
		// draw until the picture is full, the bit stream is over or the next lines would be drawn outside of the frame
		while (pixcnt<(width*height) && (bitidx<gfxsize || mask) && ypos<pThis->height)
		{
			unsigned char branch0,branch1;
			unsigned char termbyte0,termbyte1;
			unsigned char branch;
			unsigned char termbyte;

			termbyte0=pThis->pGfxBuf[offset+ 0x00+treeidx/8];
			termbyte1=pThis->pGfxBuf[offset+0x120+treeidx/8];
			branch0=  pThis->pGfxBuf[offset+ 0x20+treeidx];
			branch1=  pThis->pGfxBuf[offset+0x140+treeidx];
			if (mask==0)
			{
				byte=gfxbuf[bitidx++];
				mask=0x80;
			}
			branch  =(byte&mask)?  branch1:  branch0;
			termbyte=(byte&mask)?termbyte1:termbyte0;
			termbyte&=(0x80)>>(treeidx&7);
			mask>>=1;

			if (termbyte)
			{
				switch(rlestate)
				{
					case 0: // check for the rle character
						rlenum=1;
						if (branch==rlechar)
						{
							rlestate=1;
						}
						break;
					case 1:
						if (branch==0xff)       // check if the rle character was escaped
						{
							rlestate=0;
							branch=rlechar; // yes. use the rle character
							rlenum=1;
						} else {
							rlenum=4+branch;//no. repeat this many times
							rlestate=2;
						}
						break;
					default:// at this point, the rlenum contains the number of times the current branch should be repeated
						rlestate=0;
						break;
				}
				while (rlestate==0 && rlenum && pixcnt<(width*height))
				{
					unsigned char p1;
					unsigned char p2;
					p1=branch&0xf;p2=(branch>>4)&0xf;
					pThis->linebuf[x]^=p1;
					x++;
					pixcnt++;
					pThis->linebuf[x]^=p2;
					if (x!=width)
					{
						pixcnt++;
					}
					x++;
					if (x>=width)
					{
						int i;

						for (i=0;i<width;i++)
						{
							int pnew;
							pnew=linebuf[i];
							// do not draw "transparent" pixels.
							// and only draw, if the pixel would be inside the picture
							if (ypos>=0 && ypos<=pThis->height && (i+xpos)>=0 && (i+xpos)<pThis->width && pnew!=transparency)
							{
								int c;
								c=xpos+i+ypos*pThis->width;
								if (pSmall!=NULL)
								{
									pSmall->pixels[c]=pnew;
								} 
								if (pLarge!=NULL)
								{
									pLarge->rgbpixels[c]=rgbs[pnew];
								}
							}
						}

						x=0;
						ypos++;
					}
					rlenum--;
				}
				treeidx=0;
			} else {
				treeidx=branch;
			}
		}
	}	
	return DMAGNETIC2_OK;
}
