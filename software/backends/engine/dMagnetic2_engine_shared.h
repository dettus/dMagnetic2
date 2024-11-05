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

#ifndef	DMAGNETIC2_ENGINE_SHARED_H
#define	DMAGNETIC2_ENGINE_SHARED_H
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

// then a couple of enumerations. to make the sourcecode a little bit easier to read.
typedef enum _tVM68k_types {VM68K_BYTE=0,VM68K_WORD=1,VM68K_LONG=2,VM68K_UNKNOWN=3} tVM68k_types;
typedef enum _tVM68k_addrmodes {	VM68K_AM_DATAREG=0,		// Dn
					VM68K_AM_ADDRREG=1,		// An
					VM68K_AM_INDIR=2,		// (An)
					VM68K_AM_POSTINC=3,		// (An)+
					VM68K_AM_PREDEC=4,		// -(An)
					VM68K_AM_DISP16=5,		// (d16,An)
					VM68K_AM_INDEX=6,		// (d8,An,Xn)
					VM68K_AM_EXT=7} 
		tVM68k_addrmodes;
typedef	enum _tVM68k_addrmode_ext {	VM68K_AMX_W=0,			// (xxx),W
					VM68K_AMX_L=1,			// (xxx),L
					VM68K_AMX_data=4,		// #<data>
					VM68K_AMX_PC=2,			// (d16,PC)
					VM68K_AMX_INDEX_PC=3} 		// (d8,PC,Xn)
					tVM68k_addrmode_ext;

// the virtual machine state. 
// the idea is, that this whole struct is self contained (no pointers), so that it can be used as a savegame.
#define	VM68K_MAGIC		0x38366d76	// "vm68", little endian
typedef struct _tVM68k
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
} tVM68k;



////// this structure holds the state after the instruction has been decoded.
typedef	struct _tVM68k_next
{
	tVM68k_ulong	pcr;	// program counter
	tVM68k_bool	override_sr;
	tVM68k_uword	sr;
	tVM68k_bool	cflag;
	tVM68k_bool	vflag;
	tVM68k_bool	zflag;
	tVM68k_bool	nflag;
	tVM68k_bool	xflag;
					// bit 0..4: CVZNX
	tVM68k_ulong	a[8];	// address register
	tVM68k_ulong	d[8];	// data register

	////// memory queue
	tVM68k_types	mem_size;
	tVM68k_ulong	mem_addr[16];
	tVM68k_ulong	mem_value[16];
	tVM68k_ubyte	mem_we;

} tVM68k_next;


typedef	enum _tVM68k_instruction
{
VM68K_INST_UNKNOWN=0,
VM68K_INST_ABCD,	//1100xxx10000myyy
 VM68K_INST_ADD,	//1101 rrro oomm myyy
 VM68K_INST_ADDA,	//1101 rrro oomm myyy
 VM68K_INST_ADDI,	//0000 0110 ssmm myyy
 VM68K_INST_ADDQ,	//0101 ddd0 ssmm myyy
 VM68K_INST_ADDX,	//1101 xxx1 ss00 myyy
 VM68K_INST_AND,	//1100 xxxo oomm myyy
 VM68K_INST_ANDI,	//0000 0010 ssmm myyy
 VM68K_INST_ANDItoCCR,	//0000 0010 0011 1100 00000000dddddddd
 VM68K_INST_ANDItoSR,	//0000 0010 0111 1100 dddddddddddddddd
 VM68K_INST_ASL_ASR,	//1110 cccd ssl0 0yyy
 VM68K_INST_BCC,	//0110 CCCC dddd dddd
 VM68K_INST_BCHG,	//0000 xxx1 01mm myyy
 VM68K_INST_BCHGB,	//0000 1000 10mm myyy
 VM68K_INST_BCLR,	//0000 xxx1 10mm myyy
 VM68K_INST_BCLRI,	//0000xxx110mmmyyy
 VM68K_INST_BRA,	//0110 0000 dddd dddd
 VM68K_INST_BSET,	//0000 xxx1 11mm myyy
 VM68K_INST_BSETB,	//0000 1000 11mm myyy
// VM68K_INST_BSR,		//01100001dddddddd
 VM68K_INST_BTST,	//0000 xxx1 00mm myyy
 VM68K_INST_BTSTB,	//0000 1000 00mm myyy
VM68K_INST_CHK,		//0100xxxss0mmmyyy
 VM68K_INST_CLR,	//0100 0010 ssmm myyy
 VM68K_INST_CMP,	//1011 xxxo oomm myyy
 VM68K_INST_CMPA,	//1011 xxxo oomm myyy
 VM68K_INST_CMPI,	//0000 1100 ssmm myyy
 VM68K_INST_CMPM,	//1011 xxx1 ss00 1yyy
 VM68K_INST_DBcc,	//0101 CCCC 1100 1yyy
VM68K_INST_DIVS,	//1000xxx111mmmyyy
VM68K_INST_DIVU,	//1000xxx011mmmyyy
 VM68K_INST_EOR,	//1011 xxxo oomm myyy
 VM68K_INST_EORI,	//0000 1010 ssmm myyy
 VM68K_INST_EORItoCCR,	//0000 1010 0011 1100 00000000dddddddd
 VM68K_INST_EORItoSR,	//0000 1010 0111 1100 dddddddddddddddd
 VM68K_INST_EXG,	//1100 xxx1 oooo oyyy
 VM68K_INST_EXT,	//0100 100o oo00 0yyy
VM68K_INST_ILLEGAL,	//0100101011111100
 VM68K_INST_JMP,	//0100 1110 11mm myyy
 VM68K_INST_JSR,	//0100 1110 10mm myyy
 VM68K_INST_LEA,	//0100 xxx1 11mm myyy
VM68K_INST_LINK,	//0100111001010yyydddddddddddddddd
 VM68K_INST_LSL_LSR,	//1110 cccd ssl0 1yyy
 VM68K_INST_MOVE,	//00ss xxxm mmMM Myyy
 VM68K_INST_MOVEA,	//00ss xxx0 01mm myyy
 VM68K_INST_MOVEtoCCR,	//0100010011mmmyyy
 VM68K_INST_MOVEfromSR,	//0100000011mmmyyy
 VM68K_INST_MOVEtoSR,	//0100011011mmmyyy
VM68K_INST_MOVEUSP,	//010011100110dyyy
 VM68K_INST_MOVEMregtomem,	//0100 1d00 1smm myyy
 VM68K_INST_MOVEMmemtoreg,	//0100 1d00 1smm myyy
VM68K_INST_MOVEP,	//0000xxxooo001yyydddddddddddddddd
 VM68K_INST_MOVEQ,	//0111xxx0dddddddd
VM68K_INST_MULS,	//1100xxx111mmmyyy
VM68K_INST_MULU,	//1100xxx011mmmyyy!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
VM68K_INST_NBCD,	//1100xxx011mmmyyy!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 VM68K_INST_NEG,	//0100 0100 ssmm myyy
 VM68K_INST_NEGX,	//0100 0000 ssmm myyy
 VM68K_INST_NOP,	//0100 1110 0111 0001
 VM68K_INST_NOT,	//0100 0110 ssmm myyy
 VM68K_INST_OR,		//1000 xxxo oomm myyy
 VM68K_INST_ORI,	//0000 0000 ssmm myyy
 VM68K_INST_ORItoCCR,	//0000 0000 0011 1100 00000000dddddddd
 VM68K_INST_ORItoSR,	//0000 0000 0111 1100 dddddddddddddddd
 VM68K_INST_PEA,	//0100 1000 01mm myyy
VM68K_INST_RESET,	//0100111001110000
 VM68K_INST_ROL_ROR,	//1110 cccd ssl1 1yyy
 VM68K_INST_ROXL_ROXR,	//1110 cccd ssl1 0yyy
VM68K_INST_RTE,		//0100 1110 0111 0011
VM68K_INST_RTR,		//0100 1110 0111 0111
 VM68K_INST_RTS,	//0100 1110 0111 0101
VM68K_INST_SBCD,	//1000xxx10000ryyy
 VM68K_INST_SCC,	//0101 CCCC 11mm myyy
VM68K_INST_STOP,	//0100111001110010iiiiiiiiiiiiiiii
 VM68K_INST_SUB,	//1001 xxxo oomm myyy
 VM68K_INST_SUBA,	//1001 xxxo oomm myyy
 VM68K_INST_SUBI,	//0000 0100 ssmm myyy
 VM68K_INST_SUBQ,	//0101 ddd1 ssmm myyy
 VM68K_INST_SUBX,	//1001 yyy1 ss00 ryyy
VM68K_INST_SWAP,	//0100100001000yyy
VM68K_INST_TAS,		//0100101011mmmyyy
VM68K_INST_TRAP,	//010011100100vvvv
VM68K_INST_TRAPV,	//0100111001110110
 VM68K_INST_TST,	//0100 1010 ssmm myyy
VM68K_INST_UNLK,	//0100111001011yyy
} tVM68k_instruction;







#define	INITNEXT(pVM68k,next)	\
	(next).pcr=(pVM68k)->pcr;	\
	(next).a[0]=(pVM68k)->a[0];	\
	(next).a[1]=(pVM68k)->a[1];	\
	(next).a[2]=(pVM68k)->a[2];	\
	(next).a[3]=(pVM68k)->a[3];	\
	(next).a[4]=(pVM68k)->a[4];	\
	(next).a[5]=(pVM68k)->a[5];	\
	(next).a[6]=(pVM68k)->a[6];	\
	(next).a[7]=(pVM68k)->a[7];	\
	(next).d[0]=(pVM68k)->d[0];	\
	(next).d[1]=(pVM68k)->d[1];	\
	(next).d[2]=(pVM68k)->d[2];	\
	(next).d[3]=(pVM68k)->d[3];	\
	(next).d[4]=(pVM68k)->d[4];	\
	(next).d[5]=(pVM68k)->d[5];	\
	(next).d[6]=(pVM68k)->d[6];	\
	(next).d[7]=(pVM68k)->d[7];	\
	(next).override_sr=0;		\
	(next).sr=((pVM68k)->sr);	\
	(next).cflag=((pVM68k)->sr>>0)&1;	\
	(next).vflag=((pVM68k)->sr>>1)&1;	\
	(next).zflag=((pVM68k)->sr>>2)&1;	\
	(next).nflag=((pVM68k)->sr>>3)&1;	\
	(next).xflag=((pVM68k)->sr>>4)&1;	\
	(next).mem_we=0;	\
	(next).mem_addr[0]=0;	\
	(next).mem_size=0;	\
	(next).mem_value[0]=0;


#define	WRITEFLAGS(pVM68k,transaction)	\
	(pVM68k)->sr|=(tVM68k_uword)((transaction).cflag)<<0;	\
	(pVM68k)->sr|=(tVM68k_uword)((transaction).vflag)<<1;	\
	(pVM68k)->sr|=(tVM68k_uword)((transaction).zflag)<<2;	\
	(pVM68k)->sr|=(tVM68k_uword)((transaction).nflag)<<3;	\
	(pVM68k)->sr|=(tVM68k_uword)((transaction).xflag)<<4;

#define	READEXTENSIONBYTE(pVM68k,pNext)	READ_INT8BE((pVM68k)->memory,(pNext)->pcr+1);(pNext)->pcr+=2;
#define	READEXTENSIONWORD(pVM68k,pNext)	READ_INT16BE((pVM68k)->memory,(pNext)->pcr);(pNext)->pcr+=2;
#define	READEXTENSIONLONG(pVM68k,pNext)	READ_INT32BE((pVM68k)->memory,(pNext)->pcr);(pNext)->pcr+=4;

#define	READEXTENSION(pVM68k,pNext,datatype,operand)	\
	switch (datatype)	\
{	\
	case VM68K_BYTE:	operand=READEXTENSIONBYTE(pVM68k,pNext);break;	\
	case VM68K_WORD:	operand=READEXTENSIONWORD(pVM68k,pNext);break;	\
	case VM68K_LONG:	operand=READEXTENSIONLONG(pVM68k,pNext);break;	\
	default:		operand=0;break;	\
}

#define	READSIGNEDEXTENSION(pVM68k,pNext,datatype,operand)	\
	switch (datatype)	\
{	\
	case VM68K_BYTE:	operand=READEXTENSIONBYTE(pVM68k,pNext);operand=(tVM68k_slong)((tVM68k_sbyte)((operand)&  0xff));break;	\
	case VM68K_WORD:	operand=READEXTENSIONBYTE(pVM68k,pNext);operand=(tVM68k_slong)((tVM68k_sword)((operand)&0xffff));break;	\
	case VM68K_LONG:	operand=READEXTENSIONLONG(pVM68k,pNext);break;	\
}

#define	PUSHWORDTOSTACK(pVM68k,pNext,x)	{(pNext)->a[7]-=2;(pNext)->mem_addr[(pNext)->mem_we]=(pNext)->a[7];(pNext)->mem_size=VM68K_WORD;(pNext)->mem_value[(pNext)->mem_we]=x;(pNext)->mem_we++;}
#define	PUSHLONGTOSTACK(pVM68k,pNext,x)	{(pNext)->a[7]-=4;(pNext)->mem_addr[(pNext)->mem_we]=(pNext)->a[7];(pNext)->mem_size=VM68K_LONG;(pNext)->mem_value[(pNext)->mem_we]=x;(pNext)->mem_we++;}

#define	POPWORDFROMSTACK(pVM68k,pNext,x)	{tVM68k_uword y;y=READ_INT16BE((pVM68k)->memory,(pNext)->a[7]);(pNext)->a[7]+=2;x=((x)&0xffff0000)|(y&0xffff);}
#define	POPLONGFROMSTACK(pVM68k,pNext,x)	{x=READ_INT32BE((pVM68k)->memory,(pNext)->a[7]);(pNext)->a[7]+=4;}


#define DATAREGADDR(addr)	(-((addr)+ 1))
#define ADDRREGADDR(addr)	(-((addr)+10))

#define	VM68K_LEGAL_AM_DATAREG	(1<< 1)
#define	VM68K_LEGAL_AM_ADDRREG	(1<< 2)
#define	VM68K_LEGAL_AM_INDIR	(1<< 3)
#define	VM68K_LEGAL_AM_POSTINC	(1<< 4)
#define	VM68K_LEGAL_AM_PREDEC	(1<< 5)
#define	VM68K_LEGAL_AM_DISP16	(1<< 6)
#define	VM68K_LEGAL_AM_INDEX	(1<< 7)

#define	VM68K_LEGAL_AMX_W	(1<< 8)
#define	VM68K_LEGAL_AMX_L	(1<< 9)
#define	VM68K_LEGAL_AMX_DATA	(1<<10)
#define	VM68K_LEGAL_AMX_PC	(1<<11)
#define	VM68K_LEGAL_AMX_INDEX_PC (1<<12)

#define	VM68K_LEGAL_CONTROLALTERATEADDRESSING		(VM68K_LEGAL_AM_INDIR|VM68K_LEGAL_AM_DISP16|VM68K_LEGAL_AM_INDEX|VM68K_LEGAL_AMX_W|VM68K_LEGAL_AMX_L|VM68K_LEGAL_AMX_PC|VM68K_LEGAL_AMX_INDEX_PC)
#define	VM68K_LEGAL_CONTROLADDRESSING			(VM68K_LEGAL_AM_INDIR|VM68K_LEGAL_AM_DISP16|VM68K_LEGAL_AM_INDEX|VM68K_LEGAL_AMX_W|VM68K_LEGAL_AMX_L|VM68K_LEGAL_AMX_PC|VM68K_LEGAL_AMX_INDEX_PC)
#define	VM68K_LEGAL_DATAADDRESSING ( VM68K_LEGAL_AM_DATAREG| VM68K_LEGAL_AM_INDIR|	VM68K_LEGAL_AM_POSTINC| VM68K_LEGAL_AM_PREDEC| VM68K_LEGAL_AM_DISP16| VM68K_LEGAL_AM_INDEX| VM68K_LEGAL_AMX_W| VM68K_LEGAL_AMX_L| VM68K_LEGAL_AMX_DATA| VM68K_LEGAL_AMX_PC| VM68K_LEGAL_AMX_INDEX_PC )
#define	VM68K_LEGAL_ALL ( VM68K_LEGAL_AM_DATAREG| VM68K_LEGAL_AM_ADDRREG| VM68K_LEGAL_AM_INDIR|	VM68K_LEGAL_AM_POSTINC| VM68K_LEGAL_AM_PREDEC| VM68K_LEGAL_AM_DISP16| VM68K_LEGAL_AM_INDEX| VM68K_LEGAL_AMX_W| VM68K_LEGAL_AMX_L| VM68K_LEGAL_AMX_DATA| VM68K_LEGAL_AMX_PC| VM68K_LEGAL_AMX_INDEX_PC )
#define	VM68K_LEGAL_DATAALTERATE ( VM68K_LEGAL_AM_DATAREG|VM68K_LEGAL_AM_INDIR|VM68K_LEGAL_AM_POSTINC| VM68K_LEGAL_AM_PREDEC| VM68K_LEGAL_AM_DISP16| VM68K_LEGAL_AM_INDEX| VM68K_LEGAL_AMX_W| VM68K_LEGAL_AMX_L)
#define	VM68K_LEGAL_MEMORYALTERATE ( VM68K_LEGAL_AM_INDIR|VM68K_LEGAL_AM_POSTINC| VM68K_LEGAL_AM_PREDEC| VM68K_LEGAL_AM_DISP16| VM68K_LEGAL_AM_INDEX| VM68K_LEGAL_AMX_W| VM68K_LEGAL_AMX_L)
#define	VM68K_LEGAL_ALTERABLEADRESSING (VM68K_LEGAL_AM_DATAREG|VM68K_LEGAL_AM_ADDRREG|VM68K_LEGAL_AM_INDIR|VM68K_LEGAL_AM_POSTINC|VM68K_LEGAL_AM_PREDEC|VM68K_LEGAL_AM_DISP16|VM68K_LEGAL_AM_INDEX| VM68K_LEGAL_AMX_W|VM68K_LEGAL_AMX_L)



#define	FLAGC	(1<<0)
#define	FLAGV	(1<<1)
#define	FLAGZ	(1<<2)
#define	FLAGN	(1<<3)
#define	FLAGX	(1<<4)
#define	FLAGCZCLR	(1<<5)	// when set, the c and z flag are always cleared.

#define	FLAGS_ALL	(FLAGC|FLAGV|FLAGZ|FLAGN|FLAGX)
#define	FLAGS_LOGIC	(FLAGZ|FLAGN|FLAGCZCLR)



// some special return codes

#define	VM68K_OK		DMAGNETIC2_OK
#define	VM68K_NOK_UNKNOWN_INSTRUCTION	-1
#define	VM68K_NOK_INVALID_PTR		-2




#endif
