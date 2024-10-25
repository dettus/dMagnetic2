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

int dMagnetic2_engine_linea_istrap(tVM68k_ushort *pOpcode)
{
	tVM68k_uword inst;

// sometimes the opcode is actually just a substitute
	inst=*pOpcode;
	if ((inst&0xfe00)==0xA400) {inst&=0x01ff;inst|=0x6100;}	// BSR
	if ((inst&0xfe00)==0xA200) {inst=0x4e75;}	// RTS
	if ((inst&0xfe00)==0xA600) {inst&=0x01ff;inst|=0x4a00;}	// TST
	if ((inst&0xfe00)==0xA800) {inst&=0x01ff;inst|=0x4800;}	// MOVEM, register to memory (=0x4800)
	if ((inst&0xfe00)==0xAA00) {inst&=0x01ff;inst|=0x4C00;}	// MOVEM, memory to register (=0x4C00)

	*pOpcode=inst;

	// check if the highest 4 bits are =0xA (TrapA) or =0xF (TrapF)
	return ((inst&0xf000)==0xa000) || ((inst&0xf000)==0xf000);
}


int dMagnetic2_engine_linea_trapa(tVMLineA* pVMLineA,tVM68k_ushort opcode,unsigned int *pStatus)
{
	int version;
	int retval;
	
	tVM68k* pVM68k=pVMLineA->pVM68k;
	version=pVMLineA->version;

	retval=DMAGNETIC2_OK;

	switch(opcode)
	{
		case 0xa000:	// getchar
			{
				if (*(pVMLineA->pInputLevel)==0)	// the input buffer is empty
				{
					*pStatus|=(DMAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT);	// set the status flag
					pVMLineA->input_level=0;	// prepare the read from the buffer
					pVMLineA->input_used=0;
				} else { 	// read from the input buffer
					// take one byte from the input buffer, and send it to the CPU
					pVMLineA->input_level=*(pVMLineA->pInputLevel);
					pVM68k->d[1]=pVMLineA->pInputBuf[pVMLineA->input_used];
					pVM68k->input_used++;
				}
				if (pVMLineA->input_level==pVMLineA_used && pVMLineA->*(pVMLineA->pInputLevel)) // the buffer has been fully read
				{
					pVMLineA->input_level=0;	// prepare the next read
					pVMLineA->input_used=0;
					*pStatus&=~(DMAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT);// remove the status flag
				}
			}
			break;


// historically, the lineA trap instructions were defined backwards. they started with 0xa0ff and grew towards smaller numbers
		case 0xa0ff:	// read from the dictionary
			{
				// so, here's what i know: (version 0)
				// the dictonary is stored at A3. 
				// the word entered at A6
				// there is a "bank" in register D6
				//
				// the data structure is more or less plain. but the last char of each word has bit 7 set. 
				// special characters 0x81=ENDOFDICT 0x82=BANKSEPARATOR are used.
				//
				// input: (A6)
				// output: (A2)
				// dict: (A3)
				// objects: (A1)

				{
					tVM68k_ubyte* dtabptr;
					tVM68k_ubyte* inputptr;
					tVM68k_ubyte* outputptr;
					tVM68k_ubyte* dictptr;
					tVM68k_ubyte* objectptr;
					tVM68k_ubyte* adjptr;

					tVM68k_uword	inputidx;
					tVM68k_uword	outputidx;
					tVM68k_uword	outputidx2;
					tVM68k_uword	dictidx;
					tVM68k_uword	adjidx;

					tVM68k_uword	wordidx;
					tVM68k_ubyte	bank;
					tVM68k_ubyte	flag;
					tVM68k_bool	matching;

					tVM68k_ubyte	cdict;
					tVM68k_bool	matchfound;
					tVM68k_ulong	wordmatch;
					tVM68k_uword	longestmatch;

					tVM68k_ubyte	flag2;

					int i,j;
					longestmatch=0;
					flag2=0;

					inputptr  =&pVM68k->memory[pVM68k->a[6]];
					if (version==0 || pVMLineA->pDict==NULL || pVMLineA->dictsize==0) 
					{
						dictptr=&pVM68k->memory[pVM68k->a[3]&0xffff];
						dtabptr=&pVM68k->memory[pVM68k->a[5]&0xffff];	// version>0
					} else {
						dictptr=&pLineA->pDict[pVM68k->a[3]&0xffff];
						dtabptr=&pLineA->pDict[pVM68k->a[5]&0xffff];	// version>0

					}
					outputptr =&pVM68k->memory[pVM68k->a[2]];
					objectptr =&pVM68k->memory[pVM68k->a[1]];
					adjptr    =&pVM68k->memory[pVM68k->a[0]];
					inputidx=dictidx=outputidx=0;
					pVM68k->d[0]&=0xffff0000;		// this regsiter was used during the adjective search.
					pVM68k->d[1]&=0xffff0000;		// this regsiter was used during the adjective search.

					flag=0;
					bank=(pVM68k->d[6]&0xff);
					wordidx=0;
					cdict=0;
					matching=1;
					matchfound=0;
					pVM68k->d[0]&=0xffff0000;
					// the way the first loop works is this:
					// character by character, a word from the dictionary is compared to the input.
					// when a mismatch happens, the beginning of the next word is searched. -> matching=0;
					// 
					while (cdict!=0x81)	// 0x81 is the end marker of the dictionary
					{
						cdict=dictptr[dictidx++];
						if (cdict==0x82)	// bank separator
						{
							flag=0;
							inputidx=0;
							wordidx=0;
							bank++;
							matching=1;
						} else if (matching) {	// actively comparing
							tVM68k_ubyte	cinput1,cinput2;
							cinput1=inputptr[inputidx++];	// the current character
							cinput2=inputptr[inputidx];	// and the next onea
							if (version!=0)
							{
								if (cdict==0x5f && (cinput2!=0 || cinput1==' '))	// uppercase
								{
									flag=0x80;	// the dictionary uses _ to signal objects that consist of longer words. "can of worms" thus becomes "can_of_worms". the matcher has to find it.
									cinput1='_';	// replace the space from the input with an _ to see if there is a match.
								}

							}
							if (cdict&0x80)	// the end of an entry in the dictionary is marked by bit 7 being set.
							{
								matchfound=0;
								if ((cinput1&0x5f)==(cdict&0x5f)) 	// still a match. wonderful.
								{
									if (cinput2==0x27)	// rabbit's (Wonderland)
									{
										tVM68k_ubyte cinput3;
										inputidx++;
										cinput3=inputptr[inputidx];	// store the letter after the ' into register D0. for example: rabbit's -> store the S
										pVM68k->d[0]&=0xffff0000;
										pVM68k->d[0]|=(cinput3)&0xff;
										pVM68k->d[0]|=0x200;
									}
									if (cdict!=0xa0 || version<4)		// corruption started using " " as word separator for multi-word objects
									{
										if (cinput2==0 || cinput2==0x20 || cinput2==0x27) matchfound=1;	// and the input word ends as well. perfect match.
									}
								} else {
									if (version==0 && inputidx>7) matchfound=1;	// the first 7 characters matched. good enough.
									matching=0;
								}
							} else {	// keep comparing.
								if (version!=0)	// version 1 introduced objects with multiple words.
								{
									if (cinput1==' ' && cdict==0x5f) // multiple word entry found
									{
										flag=1;
										cinput1=0x5f;	// multiple word entries are marked by a _ instead of a space. this one makes sure that the next if() will work.
									}
								}
								if ((cinput1&0x5f)!=(cdict&0x5f) 
										|| (cdict&0x5f)==0x00 	// FIXME
								   )
								{
									if (cinput2==' ' && version==0 && inputidx>=7) matchfound=1;	// the first 7 characters matched. good enough.
									matching=0;	// there was a mismatch.
								}
							}
						}
						if (matchfound)
						{
							// the matches are stored in the following format:
							// bit 31..24 are a flag, which is =0 in version 0.
							// bit 23..16 is the bank.
							// bit 15..0 contain the matched word number in the bank.
							wordmatch =(((tVM68k_ulong)flag)<<24);
							wordmatch|=(((tVM68k_ulong)bank)<<16);
							wordmatch|=((tVM68k_ulong)wordidx);
							if (inputidx>=longestmatch) longestmatch=inputidx;
							WRITE_INT32BE(outputptr,outputidx,wordmatch);	// store the candidates in the output location.
							outputidx+=4;	// length of the result: 4 bytes.
							matchfound=0;
						}
						if (cdict&0x80 && cdict!=0x82 && !(version>4 && cdict==0xa0))	// when the end of the word is reached. bit 7 is set.	// FIXME: There is no version >4
						{
							wordidx++;
							matching=1;	// start over
							inputidx=0;	// start over.
							flag=0;
						}
					}

					WRITE_INT16BE(outputptr,outputidx,0xffff);// the end marker in the buffer is a 0xffff.
										  // the output buffer holds outputidx/4 many results.
					if (version!=0)	// version 1 introduced synonyms.
					{
						// search the list of output words
						for (i=0;i<outputidx;i+=4)
						{
							wordmatch=READ_INT32BE(outputptr,i);
							flag=(wordmatch>>24)&0xff;
							bank=(wordmatch>>16)&0xff;
							wordidx=wordmatch&0xffff;
							if (bank==0x0b)
							{
								tVM68k_uword substword;
								substword=READ_INT16BE(dtabptr,wordidx*2);		// TODO: version >1???

								// the lower 5 bits are the bank.
								// the upper 11 bits in the substitute database are the actual word index.
								bank=substword&0x1f;
								wordidx=substword>>5;
								wordmatch=flag;wordmatch<<=8;
								wordmatch|=(bank&0xff);wordmatch<<=16;
								wordmatch|=wordidx&0xffff;
								WRITE_INT32BE(outputptr,i,wordmatch);

							}

						} 
					}

					outputidx2=0;
					adjidx=0;
					for (i=0;i<outputidx;i+=4)
					{
						tVM68k_uword	objectidx;
						tVM68k_uword	adjidx_base;
						tVM68k_uword	obj;
						tVM68k_bool	mismatch;
						wordmatch=READ_INT32BE(outputptr,i);
						objectidx=0;
						flag=(wordmatch>>24)&0xff;
						bank=(wordmatch>>16)&0xff;
						wordidx=wordmatch&0xffff;

						mismatch=0;
						obj=READ_INT16BE(objectptr,objectidx);
						if (obj && bank==6)
						{
							// first step: skip the adjectives that are not meant for this word. each adjective list is separated by a 0.
							for (j=0;j<wordidx;j++)
							{
								do
								{
									cdict=READ_INT8BE(adjptr,adjidx++);
								} while (cdict!=0);
							}
							adjidx_base=adjidx;	// remeber the beginning of the list of adjectives for this word.

							do
							{
								tVM68k_ubyte cinput2;
								adjidx=adjidx_base;
								cinput2=READ_INT8BE(objectptr,objectidx+1);
								obj=READ_INT16BE(objectptr,objectidx);
								if (obj)
								{
									objectidx+=2;
									do
									{
										cdict=READ_INT8BE(adjptr,adjidx++);
									} while (cdict && ((cdict-3)!=cinput2));
									if ((cdict-3)!=cinput2) mismatch=1;

								}
							}
							while (obj && !mismatch);
							adjidx=0;
						}

						pVM68k->d[1]&=0xffff0000;
						if (mismatch==0) 
						{
							flag2|=flag;
							wordmatch =flag2&0xff;wordmatch<<=8;
							wordmatch|=bank&0xff;wordmatch<<=16;
							wordmatch|=wordidx&0xffff;
							WRITE_INT32BE(outputptr,outputidx2,wordmatch);
							outputidx2+=4;
						} else {

							pVM68k->d[1]|=1;
						}
					}
					pVM68k->a[5]=pVM68k->a[6];
					// flag2 being set denotes that there has been an object that is occupying multiple words. 
					if (flag2 && outputidx)	// that match is probably a better one, so move it to the front of the output word list.
					{
						for (i=0;i<outputidx && flag2;i+=4)
						{

							wordmatch=READ_INT32BE(outputptr,i);	// find the wordmatch with the flag set.
							if (wordmatch&0x80000000)
							{
								wordmatch&=0x7fffffff;
								WRITE_INT32BE(outputptr,0,wordmatch);	// move it to the front.
								flag2=0;
							}
						}
						outputidx2=4;
						if (longestmatch)
						{
							pVM68k->a[5]=pVM68k->a[6]+(longestmatch-3);
						}

					}
					pVM68k->a[2]+=outputidx2;
					//						pVM68k->d[0]=0;
					pVM68k->a[6]=pVM68k->a[5]+1;

				}
			}
			break;
		case 0xa0fe:
			{
				// register D0 conatins an object number. calculate the address in memory
				tVM68k_sword objectnum;
				tVM68k_ulong objectidx;

				if (version>2 && (pVM68k->d[0]&0x3fff)>pLineA->properties_size)
				{
					pVM68k->d[0]&=0xffff7fff;
					//						objectidx=((pLineA->properties_size-(pVM68k->d[0]&0x3fff))^0xffff);	// TODO: I THINK THIS IS JUST A MODULO!!!
					objectidx=((pVM68k->d[0]&0x3fff)-pLineA->properties_size)-1;
					objectnum=READ_INT16BE(pVM68k->memory,pLineA->properties_tab+objectidx*2);
				} else {
					if (version>=2) 
					{
						pVM68k->d[0]&=0xffff7fff;
					}
					else 
					{
						pVM68k->d[0]&=0x00007fff;
					}
					objectnum=pVM68k->d[0]&0x7fff;

				}
				objectnum&=0x3fff;
				pVM68k->a[0]=pLineA->properties_offset+objectnum*14;

			}
			break;
		case 0xa0fd:	// configure the communication between CPU and lineA 
			{
				pVMLineA->properties_offset=pVM68k->a[0];		// save the pointer
				if (version!=0)
				{
					// version 1 introduced line F instructions
					pVMLineA->linef_subroutine=(pVM68k->a[3]&0xffff);
					if (version>1)
					{
						// version 2 instruduced programmable instructions
						pVMLineA->linef_tab=(pVM68k->a[5])&0xffff;
						pVMLineA->linef_tabsize=(pVM68k->d[7]+1)&0xffff;
					}
					if (version>2)
					{
						pVMLineA->properties_tab=(pVM68k->a[6])&0xffff;
						pVMLineA->properties_size=(pVM68k->d[6]);
					}
				}
			}
			break;

	}
	return retval;


}
int dMagnetic2_engine_linea_trapf(tVMLineA* pVMLineA,tVM68k_ushort opcode)
{
	// version 1 introduced configurable subroutines.
	// in version 2, the became programmable
	int version;
	tVM68k* pVM68k=pVMLineA->pVM68k;


	version=pVMLineA->version;
	if (version==0)		// for this version of the game, the instruction has not been defined
	{
		return DMAGNETIC2_UNKNOWN_OPCODE;
	} else if (version==1) {
		// push the PCR to the the stack
		pVM68k->a[7]-=4;
		WRITE_INT32BE(pVM68k->memory,pVM68k->a[7],pVM68k->pcr);

		// jump to the preconfigured address
		pVM68k->pcr=(pVMLineA->linef_subroutines)&pVM68k->memsize;
	} else {
		int idx;
		int base;
		
		idx=opcode&0x7ff;
		if (idx>=pLineA->linef_tabsize)
		{
			if (!(opcode&0x0800))	// 0xf800 : jump. if not, this is a call to a subroutine.
			{
				// push the PCR to the the stack
				pVM68k->a[7]-=4;
				WRITE_INT32BE(pVM68k->memory,pVM68k->a[7],pVM68k->pcr);
			}
			idx=(opcode|0x0800);
			idx^=0xffff;
			base=READ_INT16BE(pVM68k->memory,(pLineA->linef_tab+2*idx));
			pVM68k->pcr(pLineA->linef_tab+2*idx+base)%pVM68k->memsize;	// weird, but it works.
		} else {
			// push the PCR to the the stack
			pVM68k->a[7]-=4;
			WRITE_INT32BE(pVM68k->memory,pVM68k->a[7],pVM68k->pcr);

			// jump to the preconfigured address
			pVM68k->pcr=(pVMLineA->linef_subroutines)&pVM68k->memsize;
		}
	}
	return DMAGNETIC2_OK;
}

tVM68k_ulong dMagnetic2_engine_linea_random(tVMLineA* pVMLineA)
{
	// if random mode is PRBS
	pVMLineA->random_state*=1103515245ull;
	pVMLineA->random_state+=12345ull;
	
	return (pVMLineA->random_state&0x7fffffff);
}
int dMagnetic2_engine_linea_singlestep(tVMLineA* pVMLineA,tVM68k_ushort opcode,unsigned int *pStatus)
{
	int retval;
	int version;
	tVM68k* pVM68k=pVMLineA->pVM68k;


	version=pVMLineA->version;

	// check if the machine is actively waiting for input
	if ((*pStatus)&MAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT && (*(pVMLineA->pInputLevel==0)))
	{
		return DMAGNETIC2_OK;		// in that case: there is nothing to do
	}

	retval=DMAGNETIC2_UNKNOWN_OPCODE;

	if (opcode&0xf000==0xa000)
	{
		// first: advance the random generator. but only for certain opcodes
		if (
			(opcode&0xff)<0xdd
			||(version<4 && (opcode&0xff)<0xe4)
			||(version<2 && (opcode&0xff)<0xed)
		)
		{
			(void)dMagnetic2_engine_linea_getrandom(pVMLineA);
		} 
		retval=dMagnetic2_engine_linea_trapa(tVMLineA* pVMLineA,tVM68k_ushort opcode);
	} else if (opcode&0xf000==0xf000) {
		retval=dMagnetic2_engine_linea_trapf(tVMLineA* pVMLineA,tVM68k_ushort opcode);
	}
	return retval;

}

