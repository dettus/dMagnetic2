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
#include "dMagnetic2_engine.h"


unsigned char magbuf[1<<20];

int main(int argc,char** argv)
{
	FILE *f;
	char inputbuf[256];
	void *handle;
	unsigned int status;
	int i;
	int retval;
	int n;
	if (argc!=2)
	{
		fprintf(stderr,"please run with %s INPUT.mag\n",argv[0]);
		return 1;
	}

	f=fopen(argv[1],"rb");
	n=fread(magbuf,sizeof(char),sizeof(magbuf),f);
	fclose(f);
	printf("read %d bytes\n",n);


	dMagnetic2_engine_get_size(&n);
	printf("allocating %d bytes\n",n);
	handle=malloc(n);

	printf("initializing\n");fflush(stdout);
	dMagnetic2_engine_init(handle);
	
	printf("loading .mag\n");fflush(stdout);
	dMagnetic2_engine_set_mag(handle,magbuf);
	

	printf("=[ single step ]================================================================\n");
	for (i=0;i<10;i++)
	{
		retval=dMagnetic2_engine_process(handle,1,&status);
		printf("step %2d --> status %02X  retval:%d\n",i,status,retval);
		fflush(stdout);
	}
	printf("=[ running ]====================================================================\n");
	do
	{
		retval=dMagnetic2_engine_process(handle,0,&status);
		printf("       --> status %02X  retval:%d\n",status,retval);
		fflush(stdout);
		if (status&DMAGNETIC2_ENGINE_STATUS_NEW_TITLE)
		{
			char *pTitle;
			printf("\x1b[1;37;41mTITLE");
			retval=dMagnetic2_engine_get_title(handle,&pTitle);
			printf("[%s]\x1b[0m retval:%d\n",pTitle,retval);	
		}
		if (status&DMAGNETIC2_ENGINE_STATUS_NEW_TEXT)
		{
			char *pText;
			printf("\x1b[1;37;42mNEW TEXT");
			retval=dMagnetic2_engine_get_text(handle,&pText);
			printf("[%s]\x1b[0m retval:%d\n",pText,retval);	
		}
		if (status&DMAGNETIC2_ENGINE_STATUS_NEW_PICTURE)
		{
			char *pPicname;
			int picnum;
			printf("\x1b[1;37;43mNEW PICTURE");
			retval=dMagnetic2_engine_get_picture(handle,&pPicname,&picnum);
			printf("[%s]/%d\x1b[0m retval:%d\n",pPicname,picnum,retval);
		}
		if (status&DMAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT)
		{
			int cnt;
			printf("\x1b[1;37;44mWAITING FOR INPUT\x1b[0m");
			fgets(inputbuf,sizeof(inputbuf),stdin);
			retval=dMagnetic2_engine_new_input(handle,strlen(inputbuf),inputbuf,&cnt);
			printf("--> retval:%d cnt:%d\n",retval,cnt);
		}
	} while (!feof(stdin) && !(status&(DMAGNETIC2_ENGINE_STATUS_QUIT|DMAGNETIC2_ENGINE_STATUS_RESTART)));	
	return 0;
}

