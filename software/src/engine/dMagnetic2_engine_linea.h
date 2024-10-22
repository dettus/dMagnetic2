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


#ifndef	DMAGNETIC2_ENGINE_LINEA_H
#define	DMAGNETIC2_ENGINE_LINEA_H
#include "dMagnetic2_engine_vm68k.h"
#include "dMagnetic2_shared.h"


typedef	struct _tVMLineA
{
	unsigned int magic;
	int version;

// text conversion data
	int jinxterslide;
	char lastchar;
	int headlineflagged;
	int capital;

	unsigned char *pMagBuf;
// the pointers to the interesting sections inside the mag buf
	tVM68k_ubyte*	pStrings1;
	tVM68k_ulong	string1size;
	tVM68k_ulong	string2size;
	tVM68k_ubyte*	pDict;
	tVM68k_ulong	dictsize;
	tVM68k_ulong	decsize;
	tVM68k_ubyte*	pStringHuffman;
	tVM68k_ubyte*	pUndo;
	tVM68k_ulong	undosize;
	tVM68k_slong	undopc;

///////////// some pointers for the shared communication
	tVM68k	*pVM68k;
	char* pInputBuf;
	int* pInputLevel;

	char* pTextBuf;
	int* pTextLevel;

	char* pTitleBuf;
	int* pTitleLevel;

	char* pPicnameBuf;
	int* pPicnameLevel;
	int* pPictureNum;

	char* pFilenameBuf;
	int* pFilenameLevel;
} tVMLineA;


int dMagnetic2_engine_linea_init(tVMLineA* pVMLineA,unsigned char *pMagBuf);
int dMagnetic2_engine_linea_link_communication(tVMLineA* pVMLineA,
	tVM68k* pVM68k,
	char* inputbuf,int *pInputLevel,
	char* textbuf,int *pTextLevel,
	char* titlebuf,int *pTitleLevel,
	char* picnamebuf,int *pPicnameLevel,int *pPictureNum,
	char* filenamebuf,int *pFilenameLevel,
);

#endif
