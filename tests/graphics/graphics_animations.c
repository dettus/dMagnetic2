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
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_graphics.h"	// for the xpm function


unsigned char gfxbuf[1<<22];
char xpmbuf[1<<20];

int main(int argc,char** argv)
{
	int size_handle;
	int size_tmpbuf;
	void *handle;
	void *tmpbuf;
	int gfxsize;
	int retval;
	int picnum;
	int isAnimation;
	tdMagnetic2_canvas_small canvas_small;
	tdMagnetic2_canvas_large canvas_large;
	FILE *f;
	int framenum;

	if (argc!=4)
	{
		printf("PLEASE RUN WITH %s INPUT.gfx PREFIX FRAMENUM\n",argv[0]);
		printf("   which will generate files such as PREFIX_frog_00.xpm\n");
		return 1;
	}
	framenum=atoi(argv[3]);

	retval=dMagnetic2_graphics_getsize(&size_handle,&size_tmpbuf);
	if (retval!=DMAGNETIC2_OK)
	{
		return retval;
	}
	printf("allocating %d bytes for the handle and %d bytes for the tmpbuf\n",size_handle,size_tmpbuf);
	handle=malloc(size_handle);
	tmpbuf=malloc(size_tmpbuf);

	printf("initializing backend\n");
	retval=dMagnetic2_graphics_init(handle,tmpbuf);
	if (retval!=DMAGNETIC2_OK)
	{
		printf("FAIL!");
		return retval;
	}
	
	printf("loading .gfx file\n");	
	f=fopen(argv[1],"rb");
	gfxsize=fread(gfxbuf,sizeof(char),sizeof(gfxbuf),f);
	fclose(f);
	printf("read %d bytes\n",gfxsize);

	retval=dMagnetic2_graphics_set_gfx(handle,gfxbuf,gfxsize);
	if (retval!=DMAGNETIC2_OK)
	{
		printf("FAIL!");
		return retval;
	}
		

	
	for (picnum=0;picnum<128;picnum++)
	{
		char picname[7];

		retval=dMagnetic2_graphics_getpicname(handle,picname,picnum);
		if (picname[0])
		{	
			retval=dMagnetic2_graphics_decode_by_picname(handle,picname,&canvas_small,&canvas_large,&isAnimation);
			if (isAnimation)
			{
				int isLast;
				int framecnt;
				printf("%3d> %-6s %s\n",picnum,picname,isAnimation?"animation":"picture");

				isLast=0;
				framecnt=0;
				do
				{
					retval=dMagnetic2_graphics_animation_nxtframe(handle,&isLast,&canvas_small,&canvas_large);
					if (retval==DMAGNETIC2_OK)
					{
						char filename[128];
						snprintf(filename,128,"%s_%s_%04d.xpm",argv[2],picname,framecnt);
						dMagnetic2_graphics_canvas_small_to_xpm(&canvas_small,xpmbuf,sizeof(xpmbuf));

						f=fopen(filename,"w");
						fprintf(f,"%s",xpmbuf);
						fclose(f);
					}

					framecnt++;
				} while (!isLast && framecnt<framenum);	
			}
		}
	}
	
	return 0;	

		
}
