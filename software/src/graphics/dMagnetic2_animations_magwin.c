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
#include <stdio.h>
#include "dMagnetic2_graphics.h"		// for the datatypes
#include "dMagnetic2_shared.h"			// for the macros
#include "dMagnetic2_errorcodes.h"		// for the error codes
#include "dMagnetic2_graphics.h"

#define	MAGIC		0x28816

#define	MAXPICWIDTH	512
#define	NUM_COLORS	16
#define	TREESIZE	609		// 

#define	PICTURE_MAX_RGB_VALUE	1023
#define	DMAGNETIC2_GRAPHICS_WRONG_CEL	-17

#define	MAXCMDS		256
#define	MAXANIMATIONS	256

typedef struct _tdMagnetic2_animations_drawState
{
	int animationidx;
	int start;
	int count;
	int current;
} tdMagnetic2_animations_drawState;

typedef struct _tdMagnetic2_animations_handle
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

	// pointer to where the actual animation can be found inside the gfx buffer
	int offset;
	int length;

	// states for the animation
	int animation_num;
	int cmd_offset;
	int cmd_num;
	int cmd_ptr[MAXCMDS];
	int cmd_idx;
	int framecnt;
	int animations_offset[MAXANIMATIONS];
	tdMagnetic2_animations_drawState drawChain[MAXANIMATIONS];
	
} tdMagnetic2_animations_handle;

int dMagnetic2_animations_magwin_init(tdMagnetic2_animations_handle *pThis)
{
	memset(pThis,0,sizeof(tdMagnetic2_animations_handle));
	pThis->magic=MAGIC;
	return DMAGNETIC2_OK;
}


// TODO: check if the gfx format matches
int dMagnetic2_animations_magin_set_gfx(tdMagnetic2_animations_handle *pThis,unsigned char *pGfxBuf,int gfxsize)
{
	pThis->pGfxBuf=pGfxBuf;
	pThis->gfxsize=gfxsize;
	return DMAGNETIC2_OK;
}

int dMagnetic2_animations_magwin_addcel(tdMagnetic2_animations_handle *pThis,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge,
	int celidx,int xpos,int ypos,int magic)
{
	int width;
	int height;
	int transparency;
	int animidx;
	int j;
	int blocksize;
	int x;

	// the header of a picture is
	// 609 bytes Tree
	// 16*2 bytes RGB values
	// 4 bytes??
	// 2 bytes width
	// 2 bytes height

	animidx=pThis->offset+TREESIZE+(NUM_COLORS*2)+4;	// skip most of the header
	memset(pThis->linebuf,0,sizeof(pThis->linebuf));

	blocksize=READ_INT16LE(pThis->pGfxBuf,animidx+6);

	if (celidx==0)	// this is the background cel. initialize the picture
	{
		for (j=0;j<NUM_COLORS;j++)
		{
			int red,green,blue;
			unsigned short rgb;
			rgb=READ_INT16LE(pThis->pGfxBuf,pThis->offset+TREESIZE+j*2+4);
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
	}
	// skip the blocks to get to the proper cel

	for (j=0;j<celidx;j++)
	{
		blocksize=READ_INT16LE(pThis->pGfxBuf,animidx+6);
		animidx+=blocksize+8;
		if (j==0)
		{
			pThis->celnum=READ_INT16LE(pThis->pGfxBuf,animidx+2);
			animidx+=4;
			if (pThis->celnum<celidx)
			{
				return DMAGNETIC2_INVALID_ANIMATION;
			}
		}
	}

	width=	READ_INT16LE(pThis->pGfxBuf,animidx+0);
	height= READ_INT16LE(pThis->pGfxBuf,animidx+2);
	transparency=	READ_INT16LE(pThis->pGfxBuf,animidx+4);
	blocksize=	READ_INT16LE(pThis->pGfxBuf,animidx+6);


	if (pSmall!=NULL)
	{
		int i;
		pSmall->width=pThis->width;
		pSmall->height=pThis->height;
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_NONE;
		for (i=0;i<NUM_COLORS;i++)
		{
			pSmall->rgb[i]=pThis->rgbs[i];	
		}
	}
	if (pLarge!=NULL)
	{
		pLarge->width=pThis->width;
		pLarge->height=pThis->height;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_NONE;
	}

	// patch away some transparency encoding i have not understood.
	if (transparency==21 && magic==1) transparency=65536;
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

		bitidx=animidx+8;
		pixcnt=0;
		rlestate=0;
		byte=0;
		mask=0;
		treeidx=0;
		rlenum=1;

		x=0;
		// negative positions are stored as a twos complement
		if (xpos>=(TWOSCOMPLEMENT-pThis->width))
		{
			xpos=xpos-TWOSCOMPLEMENT;
		}
		if (ypos>=(TWOSCOMPLEMENT-pThis->height))
		{
			ypos=ypos-TWOSCOMPLEMENT;
		}
		rlechar=pThis->pGfxBuf[pThis->offset+0x240];
		// draw until the picture is full, the bit stream is over or the next lines would be drawn outside of the frame
		while (pixcnt<(width*height) && (bitidx<pThis->gfxsize || mask) && ypos<pThis->height)
		{
			unsigned char branch0,branch1;
			unsigned char termbyte0,termbyte1;
			unsigned char branch;
			unsigned char termbyte;

			termbyte0=pThis->pGfxBuf[pThis->offset+ 0x00+treeidx/8];
			termbyte1=pThis->pGfxBuf[pThis->offset+0x120+treeidx/8];
			branch0=  pThis->pGfxBuf[pThis->offset+ 0x20+treeidx];
			branch1=  pThis->pGfxBuf[pThis->offset+0x140+treeidx];
			if (mask==0)
			{
				byte=pThis->pGfxBuf[bitidx++];
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
							pnew=pThis->linebuf[i];
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
									pLarge->rgbpixels[c]=pThis->rgbs[pnew];
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

// the purpose of this function is to find the animation in the gfx buffer
// in the directory, animations are marked by having a name that is all lowercase. 
// 
// for example: 
// FROG (vga)
// Frog (ega)
// frog (animation)
int dMagnetic2_animations_magwin_isanimation(tdMagnetic2_animations_handle *pThis,char* picname)
{
	int i;
	int num_entries;
	int idx;
	int match;
#define DIR_BYTES_NAME          6
#define DIR_BYTES_OFFSET        4
#define DIR_BYTES_LENGTH        4
#define DIR_BYTES_ENTRY         (DIR_BYTES_NAME+DIR_BYTES_OFFSET+DIR_BYTES_LENGTH)

#define TO_LOWERCASE(c)         ((c)|0x20)              // lower case characters have bit 5 set.
#define TO_UPPERCASE(c)         ((c)&0x5f)              // upper case characters do not have bit 5 set.

	idx=4;				// 4 bytes header
	num_entries=READ_INT16LE(pThis->pGfxBuf,idx),idx+=2;	// 2 bytes number of entries

	match=0;
	// go through the directory
	for (i=0;i<num_entries && !match;i++)
	{
		int j;
		match=1;
		j=0;
		do
		{
			char c1;
			char c2;

			c1=pThis->pGfxBuf[idx+j];
			c2=TO_LOWERCASE(picname[j]);
			j++;
			if (c1==0 && c2==0x20)
			{
				j=6;		// the end of the name has been reached
			}
			else if (c1!=c2)
			{
				match=0;
			}
		} while (j<6 && match);
		// as long as we are in the directory, we might as well store the offset and the length
		if (match)
		{
			idx+=DIR_BYTES_NAME;
			pThis->offset=READ_INT32LE(pThis->pGfxBuf,idx);	idx+=DIR_BYTES_OFFSET;	
			pThis->length=READ_INT32LE(pThis->pGfxBuf,idx);	idx+=DIR_BYTES_LENGTH;

		} else {
			idx+=DIR_BYTES_ENTRY;
		}
	}
	// double check
	if (match)
	{
		unsigned int magic;
		magic=READ_INT32LE(pThis->pGfxBuf,pThis->offset+pThis->length-4);
		if (magic==0x5ed0)	// static pictures end with this magic value
		{
			match=0;
		}
	}
	return match;
}

int dMagnetic2_animations_magwin_start(tdMagnetic2_animations_handle *pThis,char *picname,int *pIsAnimation)
{
	int isanimation;
	int blocksize;
	int j;
	int cmdsize;
	int idx;
	const int dMagnetic2_animations_cmdlen[7]={	//there are 7 possible commands. each have a different number of bytes 
		1,              // "END MARKER"
		4,              // "ANIMATION", animationidx, start, count 
		2,              // "JUMP", cmdidx
		3,              // "RENDER FRAMES", frames
		3,              // "PAUSE", delay_lsb, delay_msb
		4,              // "CHANCE JUMP", chance, addr_lsb, addr_msb
		3               // "JUMP IF RUNNING", addr_lsb, addr_msb
	};
	
	isanimation=dMagnetic2_animations_magwin_isanimation(pThis,picname);
	*pIsAnimation=isanimation;
	if (!isanimation)		// not an animation
	{
		return DMAGNETIC2_OK;	// nothing to do
	}
	pThis->cmd_idx=0;
	pThis->celnum=0;
	pThis->cmd_num=0;
	pThis->framecnt=0;
	memset(pThis->cmd_ptr,0,sizeof(pThis->cmd_ptr));
	memset(pThis->drawChain,0,sizeof(pThis->drawChain));

	//	
	idx=pThis->offset;      // this has been initialized within the "isanimation()" function
	idx+=TREESIZE;          // skip the tree at the beginning                                       
	idx+=(NUM_COLORS*2)+4;          // skip the RGB values (encoded as 2 bytes), and a magic number 

	blocksize=READ_INT16LE(pThis->pGfxBuf,idx+6);
	idx+=blocksize+8;

	// afterwards, there are n cels
	pThis->celnum=READ_INT16LE(pThis->pGfxBuf,idx+2);
	idx+=4;                      
	for (j=0;j<pThis->celnum;j++)
	{
		blocksize=READ_INT16LE(pThis->pGfxBuf,idx+6);
		idx+=blocksize+8;
	}
	//
	// let the animations begin 
	//
//	pThis->animation_offset=idx;
	pThis->animation_num=READ_INT16LE(pThis->pGfxBuf,idx);
	idx+=4;

	for (j=0;j<pThis->animation_num;j++)
	{
		int steps;
		pThis->animations_offset[j]=idx;
		steps=READ_INT16LE(pThis->pGfxBuf,idx);
		idx+=2;			// TODO: What am I skipping here?
		idx+=2;
		idx+=8*steps;
	}
	idx-=2;	// TODO: I feel like this can be solved differently


	// we have reached the beginning of the list of commands for the animations
	pThis->cmd_offset=idx;
	cmdsize=READ_INT16LE(pThis->pGfxBuf,idx);	idx+=2;
	pThis->cmd_num=0;
	while (cmdsize>0 && pThis->cmd_num<MAXCMDS)
	{
		int cmd;
		pThis->cmd_ptr[pThis->cmd_num]=idx;
		pThis->cmd_num++;
		cmd=pThis->pGfxBuf[idx];
		if (cmd>=0 && cmd<7)
		{
			idx+=dMagnetic2_animations_cmdlen[cmd];
		} else {
			return DMAGNETIC2_INVALID_ANIMATION;	// illegal cmd found: parser error
		}
	}
	pThis->cmd_idx=0;	// when the frames are about to be rendered, start the list of commands at the very beginning


	pThis->framecnt=0;	// 0 frame have been rendered
	return DMAGNETIC2_OK;	
}
// TODO: check if there is a valid animation
int dMagnetic2_animations_magwin_render_frame(tdMagnetic2_animations_handle *pThis,int *pIsLast,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	int done;
	int timeout;
	int i;
	int retval;
	*pIsLast=0;
	done=0;

	// there is a list of commands. go through it, one by one. 
	// until there is the command to render frames
	// since the commands have jumps in them, a faulty gfxbuf could result
	// in an infinite loop.
	// to break it, there is a timeout.
	timeout=50000;

	// one of the commands is "render <n> frames"
	// so there are actually two modes of operation:
	// first, to search for said command, selecting and placing the animations
	// second, to render <n> frames.
	while (!done && timeout>0 && pThis->framecnt==0)
	{
		unsigned char cmd;
		int ptr;
		int tmp;
		timeout--;


		
		if (pThis->framecnt==0)	// no more frames to render. check the commands
		{
#define CMD_END_MARKER          0
#define CMD_SELECT_ANIMATION    1
#define CMD_RENDER_FRAMES       2
#define CMD_JUMP_TO_INSTRUCTION 3
#define CMD_PAUSE               4       // those commands have never been used (as far as i know...)
#define CMD_CHANCE_JUMP         5
#define CMD_JUMP_IF_RUNNING     6
			ptr=pThis->cmd_ptr[pThis->cmd_idx];	pThis->cmd_idx++;
			cmd=pThis->pGfxBuf[ptr+0];

			switch(cmd)
			{
				case CMD_END_MARKER:
					*pIsLast=1;
					return DMAGNETIC2_OK;
				break;
				case CMD_SELECT_ANIMATION:
					// animations are a loop. they have a start, a number of cels, and they start somewhere in this loop
					tmp=pThis->pGfxBuf[ptr+1]-1;
					pThis->drawChain[tmp].animationidx=	pThis->pGfxBuf[ptr+1]-1;	// kind of redundant
					pThis->drawChain[tmp].start=		pThis->pGfxBuf[ptr+2]-1;
					pThis->drawChain[tmp].count=		pThis->pGfxBuf[ptr+3];
					pThis->drawChain[tmp].current=		pThis->drawChain[tmp].start;	// the current animationidx should be the start

				break;
				case CMD_JUMP_TO_INSTRUCTION:
					pThis->cmd_idx=READ_INT16LE(pThis->pGfxBuf,ptr+1);
				break;	
				case CMD_RENDER_FRAMES:	// finally! start rendering frames
					pThis->framecnt=pThis->pGfxBuf[ptr+1];	// this many, please
					done=1;
				break;

				
				// the next commands have NEVER been used
				case CMD_PAUSE:
				case CMD_CHANCE_JUMP:
				case CMD_JUMP_IF_RUNNING:
				default:
					break;
			}
		}
	}

	retval=dMagnetic2_animations_magwin_addcel(pThis,pSmall,pLarge,0,0,0,0);		// draw the background
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}
	// go through all the animations
	// draw them on top of each other: animations which are later in the list are in the foreground
	for (i=0;i<pThis->animation_num;i++)
	{
		if (pThis->drawChain[i].count>0)
		{
			int ptr;
			int steps;
			int xpos,ypos,cel,magic;
			ptr=pThis->animations_offset[pThis->drawChain[i].animationidx];
			steps=READ_INT16LE(pThis->pGfxBuf,ptr);
			if (pThis->drawChain[i].current>=steps)	// the animations are looping
			{
				pThis->drawChain[i].current=0;	// start back at the beginning
			}
			ptr+=8*pThis->drawChain[i].current+2;

			xpos= READ_INT16LE(pThis->pGfxBuf,ptr+0);	// where
			ypos= READ_INT16LE(pThis->pGfxBuf,ptr+2);	// to put
			cel=  READ_INT16LE(pThis->pGfxBuf,ptr+4);	// which cel?
			magic=READ_INT16LE(pThis->pGfxBuf,ptr+6);


			if (cel!=0)
			{
				retval=dMagnetic2_animations_magwin_addcel(pThis,pSmall,pLarge,cel,xpos,ypos,magic);		// draw the background
				if (retval!=DMAGNETIC2_OK)
				{
					return retval;
				}
			}
			pThis->drawChain[i].count--;
			pThis->drawChain[i].current++;
		}
	}
	pThis->framecnt--;	// one frame has been rendered. 
	
	return DMAGNETIC2_OK;
	
}

