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

#ifndef	DMAGNETIC2_LOADER_H
#define	DMAGNETIC2_LOADER_H

typedef	enum _edMagnetic2_game
{
	DMAGNETIC2_GAME_NONE=0,
	DMAGNETIC2_GAME_PAWN,
	DMAGNETIC2_GAME_GUILD,
	DMAGNETIC2_GAME_JINXTER,
	DMAGNETIC2_GAME_CORRUPTION,
	DMAGNETIC2_GAME_MYTH,
	DMAGNETIC2_GAME_FISH,
	DMAGNETIC2_GAME_WONDERLAND,
	DMAGNETIC2_GAME_TODO=0xff
} edMagnetic2_game;

typedef enum _edMagnetic2_source
{
	DMAGNETIC2_SOURCE_NONE=0,
	DMAGNETIC2_SOURCE_MAGGFX,
	DMAGNETIC2_SOURCE_ARCHIMEDES,
	DMAGNETIC2_SOURCE_MSDOS,
	DMAGNETIC2_SOURCE_MW,
	DMAGNETIC2_SOURCE_C64,
	DMAGNETIC2_SOURCE_AMSTRAD_CPC,
	DMAGNETIC2_SOURCE_SPECTRUM,
	DMAGNETIC2_SOURCE_ATARIXL,
	DMAGNETIC2_SOURCE_APPLEII
} edMagnetic2_source;




#define	DMAGNETIC2_MAX_MAGSIZE		(1<<20)		// TODO
#define	DMAGNETIC2_MAX_GFXSIZE		(4<<20)		// TODO

typedef struct _tdMagnetic2_game_meta
{
	edMagnetic2_game game;
	edMagnetic2_source source;
	int version;
	int real_magsize;
	int real_gfxsize;
	char game_name[32];
	char source_name[32];
} tdMagnetic2_game_meta;

int dMagnetic2_loader_getsize(int * pBytes);
int dMagnetic2_loader_init(void *pHandle);

int dMagnetic2_loader(void *pHandle,char* filename1,char* filename2,char* filename3,unsigned char* pMagBuf, unsigned char* pGfxBuf,tdMagnetic2_game_meta *pMeta,int nodoc);
#endif
