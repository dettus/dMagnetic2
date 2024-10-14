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
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_loader.h"

unsigned char magbuf[1<<20];
unsigned char gfxbuf[8<<20];


int main(int argc,char** argv)
{
	char* filename1=NULL;
	char* filename2=NULL;
	char* filename3=NULL;
	tdMagnetic2_game_meta	meta;

	void* hLoader;
	int size;
	int retval;
	FILE *f;

	
	if (argc>=2)
	{
		filename1=argv[1];
	}

	if (argc>=3)
	{
		filename2=argv[2];
	}

	if (argc>=4)
	{
		filename3=argv[3];
	}

	printf("  ;Filename1;Filename2;Filename3;");
	printf("malloc_size;");
	printf("retval;game_name;source_name;version;magsize;gfxsize;pass/fail\n");
	printf("CSV;");
	if (filename1!=NULL) printf("%s;",filename1); else printf(";");
	if (filename2!=NULL) printf("%s;",filename2); else printf(";");
	if (filename3!=NULL) printf("%s;",filename3); else printf(";");

	
	retval=dMagnetic2_loader_getsize(&size);
	if (retval!=DMAGNETIC2_OK)
	{
		return 1;
	}
	hLoader=malloc(size);
	printf("%d;",size);

	retval=dMagnetic2_loader_init(hLoader);
	if (retval!=DMAGNETIC2_OK)
	{
		return 1;
	}

	retval=dMagnetic2_loader(hLoader,filename1,filename2,filename3,magbuf,gfxbuf,&meta,0);
	printf("%d;",retval);
	printf("%s;",meta.game_name);
	printf("%s;",meta.source_name);
	printf("%d;",meta.version);
	printf("%d;",meta.real_magsize);
	printf("%d;",meta.real_gfxsize);
	if (retval==DMAGNETIC2_OK)
	{
		printf("PASS;");
	} else {
		printf("FAIL;");
	}
	printf("\n");
	if (retval==DMAGNETIC2_OK)
	{

		
		printf("GAME>     [%s]\n",meta.game_name);
		printf("SOURCE>   [%s]\n",meta.source_name);
		printf("VERSION>  [%d]\n",meta.version);
		printf("MAGSIZE>  [%d]\n",meta.real_magsize);
		printf("GFXSIZE>  [%d]\n",meta.real_gfxsize);

		f=fopen("test.mag","wb");
		fwrite(magbuf,sizeof(char),meta.real_magsize,f);
		fclose(f);

		f=fopen("test.gfx","wb");
		fwrite(gfxbuf,sizeof(char),meta.real_gfxsize,f);
		fclose(f);
	} else {
		printf("loader returned %d\n",retval);
	}
	

	return retval;
}




