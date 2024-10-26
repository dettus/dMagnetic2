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


typedef struct _tProperties
{
	tVM68k_ubyte unknown1[5];
	tVM68k_ubyte flags1;
	tVM68k_ubyte flags2;
	tVM68k_ubyte unknown2;
	tVM68k_uword parentobject;
	tVM68k_ubyte unknown3[2];
	tVM68k_uword endflags;
} tProperties;

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

int dMagnetic2_engine_linea_istrap(tVM68k_uword *pOpcode)
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
// the purpose of this function is to load the properties for a specific object.
int dMagnetic2_engine_linea_loadproperties(tVMLineA* pVMLineA,tVM68k_uword objectnum,tVM68k_ulong* retaddr,tProperties* pProperties)
{
	tVM68k_ulong addr;
	tVM68k* pVM68k=(pVMLineA->pVM68k);
	int version;
	int i;

	version=pVMLineA->version;

	if (version>2 && (objectnum>pVMLineA->properties_size))
	{
		addr=(pVMLineA->properties_size-objectnum)^0xffff;	// TODO: WTF?
		addr*=2;
		addr+=pLineA->properties_tab;
		objectnum=READ_INT16BE(pVM68k->memory,addr);
	}
	addr=pVMLineA->properties_offset+14*objectnum;

	for (i=0;i<5;i++)
	{
		pProperties->unknown1[i]=pVM68k->memory[addr+i];
	}
	pProperties->flags1=pVM68k->memory[addr+5];
	pProperties->flags2=pVM68k->memory[addr+6];
	pProperties->unknown2=pVM68k->memory[addr+7];
	pProperties->parentobject=READ_INT16BE(pVM68k->memory,addr+8);
	for (i=0;i<2;i++)
	{
		pProperties->unknown3[i]=pVM68k->memory[addr+i+10];
	}
	pProperties->endflags=READ_INT16BE(pVM68k->memory,addr+12);
	if (retaddr!=NULL) *retaddr=addr;
	return LINEA_OK;
}


tVM68k_ulong dMagnetic2_engine_linea_getrandom(tVMLineA* pVMLineA)
{
	// if random mode is PRBS
	pVMLineA->random_state*=1103515245ull;
	pVMLineA->random_state+=12345ull;
	
	return (pVMLineA->random_state&0x7fffffff);
}


int dMagnetic2_engine_linea_trapa(tVMLineA* pVMLineA,tVM68k_uword opcode,unsigned int *pStatus)
{
	int version;
	int retval;
	
	tVM68k* pVM68k=pVMLineA->pVM68k;
	version=pVMLineA->version;

	retval=DMAGNETIC2_OK;

	switch(opcode)
	{
// lets start with the input/output traps
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

		case 0xa0df:	// new picture (by name)
			{
				tVM68k_ubyte	datatype;
				int i;
				datatype=READ_INT8BE(pVM68k->memory,pVM68k->a[1]+2);

				for (i=0;i<DMAGNETIC2_SIZE_PICTUREBUF;i++)
				{
					pVMLineA->pPictureBuf[i]=READ_INT8BE(pVM68k->memory,pVM68k->a[1]+3+i);
				}
				pVMLineA->pPictureBuf[DMAGNETIC2_SIZE_PICTUREBUF-1]=0;
				*(pVMLineA->pPictureNum)=DMAGNETIC2_LINEA_PICTURE_NAME;
				if (datatype==7)	
				{
					*pStatus|=DMAGNETIC2_ENGINE_STATUS_NEW_PICTURE;	// report the picture
				}
			}
			break;
		case 0xa0e1:	// getstring, new feature by corruption! (version4)
			{
				int i;
				int level;
				int used;
				if (*(pVMLineA->pInputLevel)==0)	// the input buffer is empty
				{
					*pStatus|=(DMAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT);	// set the status flag
				} else {
					// when this instruction is being called, the argument is the output pointer in A1.
					// up to 256 bytes may be written there. A1 itself is being incremented, until
					// the end of the output is being reached.
					//
					// when the amount of bytes is either 256 or 1, D1 is set to 1. (TODO: why?)
					// 
					// when the buffer is empty, the callback function for inputs is called first.
					pVMLineA->level=*(pVMLineA->pInputLevel);
					i=0;
					if (pVMLineA->level>pVMLineA->used)	// still characters in the buffer?
					{
						tVM68k_ubyte c;
						do
						{
							c=pVMLineA->pInputbuf[pVMLineA->used];
							if (c==0)
							{
								c='\n';	// apparently, the virtual machine wants its strings CR terminated.
							}
							WRITE_INT8BE(pVM68k->memory,(pVM68k->a[1]+i),c);
							pLineA->used++;					// increase the read pointer for the next time.
							i++;
						} while (i<MAXINPUTBUFFER  && pVMLineA->level>pLineA->used && c!='\n');
					}
					if (pVMLineA->level==pVMLineA->used) 	// the input buffer has been fully read
					{
						dMagnetic2_engine_linea_getrandom(pVMLineA);	// advance the random generator
						pVMLineA->level=0;
						pVMLineA->used=0;
						*pStatus&=~(DMAGNETIC2_ENGINE_STATUS_WAITING_FOR_INPUT);	// clear the status flag
						*(pVMLineA->pInputLevel)=0;					// clear the input buffer

						//						dMagnetic2_engine_linea_flush(pVMLineA);
					}
					pVM68k->a[1]+=(i-1);
					pVM68k->d[1]&=0xffff0000;
					if (i==MAXINPUTBUFFER || i==1)
					{
						pVM68k->d[1]|=1;
					}
				}
			}
			break;
		case 0xa0e3:	// this one apparently erases the picture
			if (pVM68k->d[1]==0)
			{
				if (pVMLineA->version<4 || pVM68k->d[6]==0)
				{
					*pStatus|=DMAGNETIC2_ENGINE_STATUS_NEW_PICTURE;
					*(pVMLineA->pPictureNum)=DMAGNETIC2_LINEA_NO_PICTURE
					pVMLineA->pPictureBuf[0]=0;	
				}
			}
			break;
		case 0xa0ea:	// print a word from the dictionary. the beginning INDEX is stored in A1. the headline flag is signalled in D1.
			{
				unsigned char c;
				tVM68k_ubyte*	dictptr;
				tVM68k_uword	dictidx;

				dictptr=pVMLineA->pDict;
				dictidx=pVM68k->a[1]&0xffff;
				do
				{
					c=dictptr[dictidx++];
					dMagnetic2_engine_linea_newchar(pVMLineA,c,pVM68k->d[2]&0xff,pVM68k->d[1]&0xff,pStatus);
				} while (!(c&0x80));
				pVM68k->a[1]&=0xffff0000;
				pVM68k->a[1]|=dictidx;
			}
			break;

		// the restart and the quit
		case 0xa0ed:	// quit
			{
				*pStatus|=DMAGNETIC2_ENGINE_STATUS_QUIT;
			}
			break;	
		case 0xa0ee:	// restart
			{
				*pStatus|=DMAGNETIC2_ENGINE_STATUS_RESTART;
			}
			break;	
		case 0xa0f0:	// show picture
			{
				int picnum;
				int picmode;
				picnum=(pVM68k->d[0]&0x1f);	// there are no more than 30 pictures in any of the games.	at least not NUMBERED ones.
				picmode=pVM68k->d[1];
				if (picmode)
				{
					*pStatus|=DMAGNETIC2_ENGINE_STATUS_NEW_PICTURE;
					*(pVMLineA->pPictureNum)=picnum;
					pVMLineA->pPictureBuf[0]=0;	// the picture has a number, not a name
			//		snprintf(pVMLineA->pPicnameBuf,DMAGNETIC2_SIZE_PICNAMEBUF,"%02d",picnum);
				}
			}
			break;
		case 0xa0f3:	// new character
			{
				retval=dMagnetic2_engine_linea_newchar(pVMLineA,pVM68k->d[1],pVM68k->d[2]&0xff,pVM68k->d[3]&0xff,pStatus);
			}			
			break;	
		case 0xa0f4:	// save game
			{
				int nameptr;
				int namelen;
//				int dataptr;
//				int datalen;
				*pStatus|=DMAGNETIC2_ENGINE_STATUS_SAVE;
				// the filename is stored at a[0]
				nameptr=pVM68k->a[0]%pVM68k->memsize; // where in the memory is the filename?
//				namelen=pVM68k->d[0];		// PROBABLY the filename is this long.
				namelen=DMAGNETIC2_SIZE_FILENAMEBUF;	// but i am not sure
				if (namelen>=DMAGNETIC2_SIZE_FILENAMEBUF) namelen=DMAGNETIC2_SIZE_FILENAMEBUF-1;
				memcpy(pVMLineA->pFilenameBuf,&(pVM68k->memory[nameidx]),namelen);
				pVMLineA->pFilenameBuf[namelen]=0;	// 0 terminate the name

//				dataptr=pVM68k->a[1]%pVM68k->memsize; // where in the memory is the filedata?
//				datalen=pVM68k->d[1];		// PROBABLY the filedata is this long.
			}
			break;
		case 0xa0f5:	// load game
			{
				int nameptr;
//				int namelen;
//				int dataptr;
//				int datalen;
				*pStatus|=DMAGNETIC2_ENGINE_STATUS_LOAD;
				// the filename is stored at a[0]
				nameptr=pVM68k->a[0]%pVM68k->memsize; // where in the memory is the filename?
				//namelen=pVM68k->d[0];		// PROBABLY the filename is this long.
				namelen=DMAGNETIC2_SIZE_FILENAMEBUF;	// but i am not sure
				if (namelen>=DMAGNETIC2_SIZE_FILENAMEBUF) namelen=DMAGNETIC2_SIZE_FILENAMEBUF-1;
				memcpy(pVMLineA->pFilenameBuf,&(pVM68k->memory[nameidx]),namelen);

//				dataptr=pVM68k->a[1]%pVM68k->memsize; // where in the memory is the filedata?
//				datalen=pVM68k->d[1];		// PROBABLY the filedata is this long.
			}
			break;
		case 0xa0f8:	// write string
			{
				// strings are huffman-coded.
				// version 0: 'string2' holds the decoding tree in the first 256 bytes.
				// and the offset addresses for the bit streams in string1.
				// modes have bit 7 set.
				//
				// when the string is terminated with a \0 it ends.
				// when the string terminates with the sequence " @", it will be
				// extended. 
				//
				// the extension will have the cflag set.
				//
				tVM68k_ulong idx;
				tVM68k_uword tmp;
				tVM68k_ubyte val;
				tVM68k_ubyte prevval;
				tVM68k_ulong byteidx;
				tVM68k_ubyte bitidx;
				int retval;

				char c;
				if (!(pVM68k->sr&(1<<0)))	// cflag is in bit 0.
				{
					bitidx=0;
					idx=pVM68k->d[0]&0xffff;
					if (idx==0) byteidx=idx;
					// version 0: string 2 holds the table to decode the strings.
					// the decoder table is 256 bytes long. afterwards, a bunch of pointers
					// to bit indexes follow.
					else byteidx=READ_INT16BE(pLineA->pStringHuffman,(0x100+2*idx));
					tmp=READ_INT16BE(pLineA->pStringHuffman,0x100);
					if (tmp && idx>=tmp)
					{
						byteidx+=pLineA->string1size;
					}
				} else {
					byteidx=pLineA->interrupted_byteidx;
					bitidx=pLineA->interrupted_bitidx;

				}
				val=0;
				do
				{
					prevval=val;
					val=0;
					while (!(val&0x80))	// terminal symbols have bit 7 set.
					{
						tVM68k_ubyte bit;
						bit=pVMLineA->pStrings1[byteidx];
						if (bit>>(bitidx)&1)
						{
							val=pVMLineA->pStringHuffman[0x80+val];	// =1 -> go to the right
						} else {
							val=pVMLineA->pStringHuffman[     val];	// =0 -> go to the left
						}
						bitidx++;
						if (bitidx==8)
						{
							bitidx=0;
							byteidx++;
						}
					}
					val&=0x7f;	// remove bit 7.
					c=val;


					retval=dMagnetic2_engine_linea_newchar(pVMLineA,c,pVM68k->d[2]&0xff,pVM68k->d[3]&0xff,pStatus);
					if (retval!=DMAGNETIC2_OK)
					{
						return retval;
					}

				}
				while (val!=0 && !(prevval==' ' && val=='@'));	// end markers for the string are \0 and " @"
				if (prevval==' ' && val=='@')		// extend the string next time this function is being called.
				{
					pVM68k->sr|=(1<<0);	// set the cflag. cflag=bit 0.
					pVMLineA->interrupted_byteidx=byteidx;
					pVMLineA->interrupted_bitidx=bitidx;
				} else {
					pVM68k->sr&=~(1<<0);	// clear the cflag. cflag=bit 0.
				}
				
			}
			break;






// historically, the lineA trap instructions were defined backwards. they started with 0xa0ff and grew towards smaller numbers
// i tried implementing this, but it got confusing.
		case 0xa0de:	// version 3 (corruption) introduced this. the other implementation wrote a 1 into D1.
			{
				pVM68k->d[1]&=0xffffff00;
				pVM68k->d[1]|=0x01;
			}
			break;
		case 0xa0e0:	// unknown, PROMPT_EV
			break;
		case 0xa0e4:
			{
					pVM68k->a[7]+=4;	// increase the stack pointer? maybe skip an entry or something?
					pVM68k->pcr=READ_INT32BE(pVM68k->memory,pVM68k->a[7])%pVM68k->memsize;
					pVM68k->a[7]+=4;
			}
			break;
		case 0xa0e5:	// set the Z-flag, RTS, introduced with jinxter.
		case 0xa0e6:	// clear the Z-flag, RTS, introduced with jinxter.
		case 0xa0e7:	// set the Z-flag, introduced with jinxter.
		case 0xa0e8:	// clear the Z-flag, introduced with jinxter.
			{
				if (opcode==0xa0e5 || opcode==0xa0e7)	// set zflag
				{
					pVM68k->sr|=(1<<2);		// BIT 2 is the Z-flag
				} else {	// clear z-flag
					pVM68k->sr&=~(1<<2);		// BIT 2 is the Z-flag
				}
				if (opcode==0xa0e4 || opcode==0xa0e5 || opcode==0xa0e6)
				{
					// RTS: poplongfromstack(pcr);
					pVM68k->pcr=READ_INT32BE(pVM68k->memory,pVM68k->a[7])%pVM68k->memsize;
					pVM68k->a[7]+=4;
				}
			}
			break;
		case 0xa0e9:
			{	// strcpy a word from the dictionary into the memory.
				// source is in A1
				// destination is A0
				tVM68k_ubyte tmp;
				do
				{
					tmp=pVMLineA->pDict[pVM68k->a[1]++];
					pVM68k->memory[pVM68k->a[0]++]=tmp;
				} while (!(tmp&0x80));
			}
			break;
		case 0xa0eb:	// write the byte stored in D1 into the dictionary at index A1
			{
				pLineA->pDict[pVM68k->a[1]&0xffff]=pVM68k->d[1]&0xff;
			}
			break;
		case 0xa0ec:	// read one byte stored @A1 from the dictionary. write it into register D0.	(jinxter)
			{
				pVM68k->d[1]&=0xffffff00;
				pVM68k->d[1]|=pLineA->pDict[pVM68k->a[1]&0xffff]&0xff;
			}
			break;
		case 0xa0f1:
			{	// skip some words in the input buffer
				tVM68k_ubyte*	inputptr;
				tVM68k_uword	inputidx;
				tVM68k_ubyte	cinput;
				int i,n;
				inputptr=&pVM68k->memory[pVM68k->a[1]&0xffff];
				inputidx=0;
				n=(pVM68k->d[0])&0xffff;
				for (i=0;i<n;i++)
				{
					do
					{
						cinput= READ_INT8BE(inputptr,inputidx++);
					} while (cinput);	// words are zero-terminated
				}
				pVM68k->a[1]+=inputidx;
			}
			break;
		case 0xa0f2:
			{
				tVM68k_uword objectnum;
				tProperties properties;
				int n;
				tVM68k_bool found;
				objectnum=(pVM68k->d[2])&0x7fff;
				n=pVM68k->d[4]&0x7fff;
				pVM68k->d[0]&=0xffff0000;
				pVM68k->d[0]|=pVM68k->d[2]&0xffff;
				found=0;
				retval=dMagnetic2_engine_linea_loadproperties(pVMLineA,objectnum,&pVM68k->a[0],&properties);
				do
				{
					if (properties.endflags&0x3fff) 
					{
						found=1;
					} else {
						retval=dMagnetic2_engine_linea_loadproperties(pVMLineA,pVM68k,objectnum-1,NULL,&properties);
						if (objectnum==n) found=1;
						else objectnum--;
					}
				} while ((objectnum!=0) && !found);
				if (found) pVM68k->sr|=(1<<0);            // bit 0 is the cflag
				pVM68k->d[2]&=0xffff0000;
				pVM68k->d[2]|=objectnum&0xffff;
			}
			break;
		case 0xa0f6:	// get random number (word), modulo D1.
			{
				tVM68k_ulong rand;
				tVM68k_uword limit;
				rand=dMagnetic2_engine_linea_getrandom(pVMLineA);	// advance the random generator
				limit=(pVM68k->d[1])&0xff;
				if (limit==0) limit=1;
				rand%=limit;
				pVM68k->d[1]&=0xffff0000;
				pVM68k->d[1]|=(rand&0xffff);
			}
			break;
		case 0xa0f7:
			{	// get a random value between 0 and 255, and write it to D0.
				tVM68k_ulong rand;
				rand=dMagnetic2_engine_linea_getrandom(pVMLineA);	// advance the random generator

				pVM68k->d[0]&=0xffffff00;
				pVM68k->d[0]|=((rand+(rand>>8))&0xff);
			}
			break;
		case 0xa0f9:	//get inventory item(d0)
			{
					// there is a list of parent objects
					//
					// apparently, the structure of the properties is as followed:
					// byte 0..4: UNKNOWN
					// byte 5: Flags. 
					//		bit 0: is_described
					// byte 6: some flags
					//		=bit 7: worn
					//		=bit 6: bodypart
					//		=bit 3: room
					//		=bit 2: hidden
					// byte 8/9: parent object. the player is =0x0000
					// byte 10..13: UNKNOWN
					// the data structure is a list.
				tVM68k_bool found;
				tVM68k_uword objectnum1;
				tVM68k_uword objectnum2;
				tProperties properties;

				found=0;
				// go backwards from the objectnumber
				for (objectnum1=pVM68k->d[0];objectnum1>0 && !found;objectnum1--)
				{
					objectnum2=objectnum1;
					do
					{
						// search for the parent
						retval=dMagnetic2_engine_linea_loadproperties(pVMLineA,objectnum2,&pVM68k->a[0],&properties);
						objectnum2=properties.parentobject;
						if ((properties.flags1&1) //is described
							|| (properties.flags2&0xcc))	// worn, bodypart, room or hidden
						{
							objectnum2=0;	// break the loop
						}
						else if (properties.parentobject==0) found=1;
						if (!(properties.flags2&1))
						{
							objectnum2=0;	// break the loop
						}
					} while (objectnum2);
				}
				// set the z-flag when the object was found. otherwise clear it.
				pVM68k->sr&=~(1<<2);	// zflag is bit 2
				if (found) pVM68k->sr|=(1<<2);
				pVM68k->d[0]&=0xffff0000;
				pVM68k->d[0]|=(objectnum1+1)&0xffff;	// return value
			}
			break;
		case 0xa0fa:
			{
				// search the properties database for a match with the entry in D2.
				// starting adress is stored in A0. D3 is the variable counter.
				// d4 is the limit. for (;D3<D4;D3++) {}
				// d5 =0 is a byte search. D5=1 is a word search.
				// set cflag when the entry is found.

				tVM68k_uword i;
				tVM68k_bool found;
				tVM68k_ulong addr;
				tVM68k_uword pattern;
				tVM68k_uword value;
				tVM68k_bool byte0word1;

				found=0;
				addr=pVM68k->a[0];
				pattern=pVM68k->d[2];
				byte0word1=pVM68k->d[5];
				pVM68k->sr&=~(1<<0);	// cflag is bit 0;
				for (i=(pVM68k->d[3]&0xffff);i<(pVM68k->d[4]&0xffff) && !found;i++)
				{
					if (byte0word1)
					{
						value=READ_INT16BE(pVM68k->memory,addr);
						value&=0x3fff;
					} else {
						value= READ_INT8BE(pVM68k->memory,addr);
						value&=0xff;
					}
					addr+=14;
					if (value==pattern)
					{
						found=1;
						pVM68k->a[0]=addr;
						pVM68k->sr|=(1<<0);	// cflag is bit 0.
					}
				}
				pVM68k->d[3]=i;

			}
			break;
		case 0xa0fb:
			{	// skip D2 many words in the dictionary, that is pointed at by A1
				tVM68k_ubyte*	dictptr;
				tVM68k_uword	dictidx;
				tVM68k_ubyte	cdict;
				int i;
				int n;


				dictidx=0;
				if (version==0 || pVMLineA->pDict==NULL || pVMLineA->dictsize==0) 
				{
					dictptr=&pVM68k->memory[pVM68k->a[1]&0xffff];
				} else {
					//dictptr=pLineA->pDict;
					dictptr=&pVMLineA->pDict[pVM68k->a[1]&0xffff];
				}
				n=(pVM68k->d[2]&0xffff);
				for (i=0;i<n;i++)
				{
					do
					{
						cdict= READ_INT8BE(dictptr,dictidx++);
					} while (!(cdict&0x80));	// until the end marker
				}
				pVM68k->d[2]&=0xffff0000;	// that was a counter
				pVM68k->a[1]+=dictidx;
			}
			break;
		case 0xa0fc:	// skip D0 many words in the input buffer, as well as the dictionary.
			{
				tVM68k_ubyte*	dictptr;
				tVM68k_ubyte*	inputptr;
				tVM68k_uword	dictidx;
				tVM68k_uword	inputidx;
				int i,n;
				dictidx=0;
				inputidx=0;
				if (version==0 || pVMLineA->pDict==NULL || pVMLineA->dictsize==0) 
				{
					dictptr=&pVM68k->memory[pVM68k->a[0]&0xffff];	// TODO: version 0. 
				} else {
					dictptr=&pLineA->pDict[pVM68k->a[0]&0xffff];
				}
				inputptr=&pVM68k->memory[pVM68k->a[1]&0xffff];
				n=(pVM68k->d[0])&0xffff;
				for (i=0;i<n;i++)
				{
					tVM68k_ubyte cdebug;
					do
					{
						cdebug=dictptr[dictidx++];
					}
					while (!(cdebug&0x80));	// in the dictionary, the end marker is bit 7 being set.
					do
					{
						cdebug=inputptr[inputidx++];
					}
					while (cdebug!=0x00);	// search for the end of the input.
				}
				pVM68k->d[0]&=0xffff0000;	// d0 was used as a counter
				pVM68k->a[0]+=dictidx;
				pVM68k->a[1]+=inputidx;
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
		
















	}
	return retval;


}
int dMagnetic2_engine_linea_trapf(tVMLineA* pVMLineA,tVM68k_uword opcode)
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

int dMagnetic2_engine_linea_singlestep(tVMLineA* pVMLineA,tVM68k_uword opcode,unsigned int *pStatus)
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
		retval=dMagnetic2_engine_linea_trapa(tVMLineA* pVMLineA,tVM68k_uword opcode);
	} else if (opcode&0xf000==0xf000) {
		retval=dMagnetic2_engine_linea_trapf(tVMLineA* pVMLineA,tVM68k_uword opcode);
	}
	return retval;

}

