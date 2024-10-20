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


int dMagnetic2_engine_vm68k_init(tdMagnetic2_engine_vm68k* pVM68k,unsigned char *pMagBuf)
{
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

	// ----------
	// @42   codesize bytes   code
	// @42+codesize  string1size bytes...
	// @...  string2size
	// @...  dictsize
	// @...  undo

	int version;
	int codesize;
	int string1size;
	int string2size;
	int dictsize;
	int decsize;
	int undosize;
	int undopc;
	int idx;
	int i;

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

	idx=42;
	pVM68k->memsize=sizeof(pVM68k->memory);
	memcpy(pVM68k->memory,&pMagBuf[idx],codesize);
	pVM68k->magic=VM68K_MAGIC;
	pVM68k->pcr=0;
	pVM68k->sr=0;
	for (i=0;i<8;i++)
	{
		pVM68k->a[i]=0;
		pVM68k->d[i]=0;
	}
	pVM68k->version=version;
	
	

	return DMAGNETIC2_OK;
}

