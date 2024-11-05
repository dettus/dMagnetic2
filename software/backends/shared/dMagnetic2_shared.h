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

#ifndef	DMAGNETIC2_SHARED_H
#define	DMAGNETIC2_SHARED_H

#ifdef __sgi__
typedef unsigned char		tVM68k_bool;
typedef unsigned char		tVM68k_ubyte;
typedef unsigned short		tVM68k_uword;
typedef unsigned int		tVM68k_ulong;
typedef unsigned long long	tVM68k_uint64;

typedef signed char		tVM68k_sbyte;
typedef signed short		tVM68k_sword;
typedef signed int		tVM68k_slong;
typedef signed long long	tVM68k_sint64;


#else
#include <stdint.h>


// first of all: the standard data types.
typedef uint_least8_t	tVM68k_bool;
typedef uint_least8_t	tVM68k_ubyte;
typedef uint_least16_t	tVM68k_uword;
typedef uint_least32_t	tVM68k_ulong;
typedef uint_least64_t	tVM68k_uint64;

typedef int_least8_t	tVM68k_sbyte;
typedef int_least16_t	tVM68k_sword;
typedef int_least32_t	tVM68k_slong;
typedef int_least64_t	tVM68k_sint64;
#endif

#define	MAX_MEMORY		98304		// for wonderland
typedef struct _tdMagnetic2_vmstate
{
	tVM68k_ulong	magic; // just so that the functions can identify a handle as this particular data structure	
	tVM68k_ulong	pcr; // program counter
	tVM68k_uword	sr; // status register.
	tVM68k_ulong	a[8]; // address registers
	tVM68k_ulong	d[8]; // data registers
	tVM68k_ubyte	memory[MAX_MEMORY];
	tVM68k_ubyte	version;
} tdMagnetic2_vmstate;




// helper macros
#define READ_INT32ME(ptr,idx)	(\
		(((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<<24)	|\
		(((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<<16)	|\
		(((tVM68k_ulong)((ptr)[((idx)+3)])&0xff)<< 8)	|\
		(((tVM68k_ulong)((ptr)[((idx)+2)])&0xff)<< 0)	|\
		0)


#define READ_INT32BE(ptr,idx)	(\
		(((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<<24)	|\
		(((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<<16)	|\
		(((tVM68k_ulong)((ptr)[((idx)+2)])&0xff)<< 8)	|\
		(((tVM68k_ulong)((ptr)[((idx)+3)])&0xff)<< 0)	|\
		0)
#define READ_INT16BE(ptr,idx)	(\
		(((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<< 8)	|\
		(((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<< 0)	|\
		0)
#define READ_INT8BE(ptr,idx)	(\
		(((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<< 0)	|\
		0)

#define READ_INT32LE(ptr,idx)	(\
		(((unsigned int)((ptr)[((idx)+3)])&0xff)<<24)	|\
		(((unsigned int)((ptr)[((idx)+2)])&0xff)<<16)	|\
		(((unsigned int)((ptr)[((idx)+1)])&0xff)<< 8)	|\
		(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 0)	|\
		0)
#define READ_INT24LE(ptr,idx)   (\
		(((unsigned int)((ptr)[((idx)+2)])&0xff)<<16)   |\
		(((unsigned int)((ptr)[((idx)+1)])&0xff)<< 8)   |\
		(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 0)   |\
		0)      

#define READ_INT16LE(ptr,idx)	(\
		(((unsigned int)((ptr)[((idx)+1)])&0xff)<< 8)	|\
		(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 0)	|\
		0)
#define READ_INT8LE(ptr,idx)	(\
		(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 0)	|\
		0)


#define WRITE_INT32BE(ptr,idx,val) {\
	(ptr)[(idx)+3]=((tVM68k_ubyte)((val)>> 0)&0xff);	\
	(ptr)[(idx)+2]=((tVM68k_ubyte)((val)>> 8)&0xff);	\
	(ptr)[(idx)+1]=((tVM68k_ubyte)((val)>>16)&0xff);	\
	(ptr)[(idx)+0]=((tVM68k_ubyte)((val)>>24)&0xff);	\
}
#define WRITE_INT16BE(ptr,idx,val) {\
	(ptr)[(idx)+1]=((tVM68k_ubyte)((val)>> 0)&0xff);	\
	(ptr)[(idx)+0]=((tVM68k_ubyte)((val)>> 8)&0xff);	\
}
#define WRITE_INT8BE(ptr,idx,val) {\
	(ptr)[(idx)+0]=((tVM68k_ubyte)((val)>> 0)&0xff);	\
}

#define WRITE_INT32LE(ptr,idx,val) {\
	(ptr)[(idx)+0]=((unsigned char)((val)>> 0)&0xff);	\
	(ptr)[(idx)+1]=((unsigned char)((val)>> 8)&0xff);	\
	(ptr)[(idx)+2]=((unsigned char)((val)>>16)&0xff);	\
	(ptr)[(idx)+3]=((unsigned char)((val)>>24)&0xff);	\
}
#define WRITE_INT16LE(ptr,idx,val) {\
	(ptr)[(idx)+0]=((unsigned char)((val)>> 0)&0xff);	\
	(ptr)[(idx)+1]=((unsigned char)((val)>> 8)&0xff);	\
}
#define WRITE_INT8LE(ptr,idx,val) {\
	(ptr)[(idx)+0]=((unsigned char)((val)>> 0)&0xff);	\
}

#endif
