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
// 

#include <stdio.h>
#include <stdlib.h>

#include "dMagnetic2_loader_shared.h"


unsigned char inbuf[1<<20];
unsigned char outbuf[1<<20];
int main(int argc,char** argv)
{
	FILE *f;
	int n,m;
	if (argc!=3)
	{
		fprintf(stderr,"Please run with %s INPUT.bin OUTPUT.bin\n",argv[0]);
		return 1;
	}
	f=fopen(argv[1],"rb");
	n=fread(inbuf,sizeof(char),sizeof(inbuf),f);
	fclose(f);
	printf("%d bytes read\n",n);
	
	m=dMagnetic2_loader_shared_unhuffer(inbuf,n,outbuf);
	printf("%d bytes unhuffed\n",m);
	f=fopen(argv[2],"wb");
	n=fwrite(outbuf,sizeof(char),m,f);
	fclose(f);
	printf("%d bytes written\n",n);
	
}
