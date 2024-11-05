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
#ifndef	DMAGNETIC2_ANIMATIONS_MAGWIN_H
#define	DMAGNETIC2_ANIMATIONS_MAGWIN_H

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



int dMagnetic2_animations_magwin_init(tdMagnetic2_animations_handle *pThis);
int dMagnetic2_animations_magwin_set_gfx(tdMagnetic2_animations_handle *pThis,unsigned char *pGfxBuf,int gfxsize);
int dMagnetic2_animations_magwin_start(tdMagnetic2_animations_handle *pThis,char *picname,int *pIsAnimation);
int dMagnetic2_animations_magwin_render_frame(tdMagnetic2_animations_handle *pThis,int *pIsLast,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge);


#endif

