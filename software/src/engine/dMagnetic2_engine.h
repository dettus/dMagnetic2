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

#ifndef	DMAGNETIC2_ENGINE_H
#define	DMAGNETIC2_ENGINE_H

#define	DMAGNETIC2_ENGINE_STATUS_NONE			0
#define	DMAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT	(1<<0)
#define	DMAGNETIC2_ENGINE_STATUS_NEW_TEXT		(1<<1)
#define	DMAGNETIC2_ENGINE_STATUS_NEW_TITLE		(1<<2)
#define	DMAGNETIC2_ENGINE_STATUS_NEW_PICTURE		(1<<3)
#define	DMAGNETIC2_ENGINE_STATUS_SAVE			(1<<4)
#define	DMAGNETIC2_ENGINE_STATUS_LOAD			(1<<5)
#define	DMAGNETIC2_ENGINE_STATUS_RESTART		(1<<6)
#define	DMAGNETIC2_ENGINE_STATUS_QUIT			(1<<7)


// API functions for initialization

int dMagnetic2_engine_get_size(int *pBytes);
int dMagnetic2_engine_init(void *pHandle);
int dMagnetic2_engine_set_mag(void *pHandle,int size,unsigned char* pMag);
//int dMagnetic2_engine_set_sections(void* pHandle,int memsize,unsigned char *pMem,int dictsize,unsigned char *pDict, int string1size,unsigned char *pString1,int string2size,unsigned char* pString2);


// API functions for running the game
int dMagnetic2_engine_process(void *pHandle,int singlestep,int* pStatus);	// singlestep=0: run until input is required
int dMagnetic2_engine_set_input(void *pHandle,int len,char* pInput);
int dMagnetic2_engine_get_text(void* pHandle,char** ppText);
int dMagnetic2_engine_get_title(void* pHandle,char** ppTitle);
int dMagnetic2_engine_get_picture(void* pHandle,char** ppPicname,int *pPicnum);
int dMagnetic2_engine_get_filename(void* pHandle,char** ppFilename);
int dMagnetic2_engine_save_game(void* pHandle,int *pSize,unsigned char* pContext);
int dMagnetic2_engine_load_game(void* pHandle,int pSize,unsigned char* pContext);

// API functions for configuration
int dMagnetic2_engine_configure(void* pHandle,int* todo);

#endif
