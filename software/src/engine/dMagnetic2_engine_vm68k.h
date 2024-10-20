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

#ifndef	DMAGNETIC2_ENGINE_VM68K_H
#define	DMAGNETIC2_ENGINE_VM68K_H

// the purpose of this file is to provide the shared datatypes needed for the virtual machine
#ifdef __sgi__
typedef unsigned char           tVM68k_bool;
typedef unsigned char           tVM68k_ubyte;
typedef unsigned short          tVM68k_uword;
typedef unsigned int            tVM68k_ulong;
typedef unsigned long long      tVM68k_uint64;
        
typedef signed char             tVM68k_sbyte;
typedef signed short            tVM68k_sword;
typedef signed int              tVM68k_slong;
typedef signed long long        tVM68k_sint64;
        
        
#else   
#include <stdint.h>
        
        
// first of all: the standard data types.
typedef uint_least8_t   tVM68k_bool;
typedef uint_least8_t   tVM68k_ubyte;
typedef uint_least16_t  tVM68k_uword;
typedef uint_least32_t  tVM68k_ulong;
typedef uint_least64_t  tVM68k_uint64;


typedef int_least8_t    tVM68k_sbyte;
typedef int_least16_t   tVM68k_sword;
typedef int_least32_t   tVM68k_slong;
typedef int_least64_t   tVM68k_sint64;
#endif


// the virtual machine state. 
// the idea is, that this whole struct is self contained (no pointers), so that it can be used as a savegame.
#define	VM68K_MAGIC		0x38366d76	// "vm68", little endian
typedef struct _tdMagnetic2_engine_vm68k
{
	tVM68k_ulong    magic;  // just so that the functions can identify a handle as this particular data structure
	tVM68k_ulong    pcr;    // program counter
	tVM68k_uword    sr;     // status register.
				// bit 0..4: CVZNX
	tVM68k_ulong    a[8];   // address register
	tVM68k_ulong    d[8];   // data register
	tVM68k_ubyte    memory[98304];
	tVM68k_ulong    memsize;        // TODO: check for violations.

	/////// VERSION PATCH
	tVM68k_ubyte    version;        // game version. not the interpreter version
} tdMagnetic2_engine_vm68k;

int dMagnetic2_engine_vm68k_init(tdMagnetic2_engine_vm68k* pVM68k,unsigned char *pMagBuf);

#endif


