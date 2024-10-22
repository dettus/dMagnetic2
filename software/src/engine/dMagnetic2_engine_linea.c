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

#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_engine_linea.h"
#include "dMagnetic2_engine_vm68k.h"
#include "dMagnetic2_shared.h"
#include <stdio.h>
#include <string.h>

#define	MAGIC	0x42696e61      // ="Lina"
int dMagnetic2_engine_linea_init(tVMLineA* pVMLineA,unsigned char *pMagBuf)
{
	int codesize;
	int string1size;
	int string2size;
	int dictsize;
	int decsize;
	int undosize;
	int undopc;
	int idx;

	memset(pVMLineA,0,sizeof(tVMLineA));
	// lets start with the header.
	// @0   4 bytes "MaSc"
	// @4   9 bytes TODO
	// @13  1 byte version
	// @14  4 bytes codesize
	// @18  4 bytes string1size
	// @22  4 bytes string2size
	// @26  4 bytes dictsize
	// @30  4 bytes decsize
	// @34  4 bytes undosize
	// @38  4 bytes undopc


	if (pMagBuf[0]!='M' || pMagBuf[1]!='a' || pMagBuf[2]!='S' || pMagBuf[3]!='s')
	{
		return DMAGNETIC2_UNKNOWN_SOURCE;
	}

	version=pMagBuf[13];	
	codesize=READ_INT32BE(pMagBuf,14);
	string1size=READ_INT32BE(pMagBuf,18);
	string2size=READ_INT32BE(pMagBuf,22);
	dictsize=READ_INT32BE(pMagBuf,26);
	decsize=READ_INT32BE(pMagBuf,30);
	undosize=READ_INT32BE(pMagBuf,34);
	undopc=READ_INT32BE(pMagBuf,38);

	pVMLineA->version=version;

	idx=42;
	idx+=codesize;
	pVMLineA->pStrings1=&pMagBuf[idx];
	pVMLineA->pStringHuffman=&(pVMLineA->pStrings1[decsize]);
	pVMLineA->string1size=string1size;
	pVMLineA->string2size=string2size;
	idx+=string1size;
	idx+=string2size;
	pVMLineA->pDict=&pMagBuf[idx];
	idx+=dictsize;
	pVMLineA->pUndo=&pMagBuf[idx];
	pVMLineA->undosize=undosize;
	pVMLineA->undopc=undopc;
	pVMLineA->decsize=decsize;

	
}
int dMagnetic2_engine_linea_link_communication(tVMLineA* pVMLineA,
	tVM68k* pVM68k,
	char* inputbuf,int *pInputLevel,
	char* textbuf,int *pTextLevel,
	char* titlebuf,int *pTitleLevel,
	char* picnamebuf,int *pPicnameLevel,int *pPictureNum,
	char* filenamebuf,int *pFilenameLevel,
)
{
	pVMLineA->pVM68k,

	pVMLineA->pInputBuf=inputbuf;
	pVMLineA->pInputLevel=pInputLevel;

	pVMLineA->pTextBuf=textbuf;
	pVMLineA->pTextLevel=pTextLevel;

	pVMLineA->pTitleBuf=titlebuf;
	pVMLineA->pTitleLevel=pTitleLevel;

	pVMLineA->pPicnameBuf=picnamebuf;
	pVMLineA->pPicnameLevel=pPicnameLevel;
	pVMLineA->pPictureNum=pPictureNum;

	pVMLineA->pFilenameBuf=filenamebuf;
	pVMLineA->pFilenameLevel=pFilenameLevel;


	return DMAGNETIC2_OK;
}






