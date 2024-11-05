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
#include "dMagnetic2_engine.h"
#include "dMagnetic2_engine_linea.h"
#include "dMagnetic2_engine_vm68k.h"
#include "dMagnetic2_shared.h"
#include <stdio.h>
#include <string.h>
int dMagnetic2_engine_linea_flush(tVMLineA* pVMLineA)
{
	// TODO
	return DMAGNETIC2_OK;
}

int dMagnetic2_engine_linea_newchar(tVMLineA* pVMLineA,unsigned char c,unsigned char controlD2,unsigned char flag_headline,unsigned int *pStatus)
{
	unsigned char c2;
	int textlevel;
	int titlelevel;


	textlevel=*(pVMLineA->pTextLevel);
	titlelevel=*(pVMLineA->pTitleLevel);
	// one line, ending with a dash -
	// is some kind of code for a "verbatim" mode.
	// it is needed for a sliding puzzle in JINXTER.
	if (pVMLineA->jinxterslide)
	{
		if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c==0x5a && pVMLineA->lastchar=='\n'))
		{
			// The sliding puzzle ends with the phrase "As the blocks slide into their final position". Or with two newlines.
			pVMLineA->jinxterslide=0;
		}
	} else if (c==0x5e && pVMLineA->lastchar==0x2d) {
		pVMLineA->jinxterslide=1;
	}
	
	// prevent double newlines
//	if (pVMLineA->jinxterslide==0 && (c=='\n' || c==0x5e) && (pVMLineA->lastchar==0 || pVMLineA->lastchar=='\n'))
//	{
//		return DMAGNETIC2_OK;
//	}


	if (flag_headline && !pVMLineA->headlineflagged) 	// this starts a headline
	{
		*(pVMLineA->pTextLevel)=textlevel;
		*(pVMLineA->pTitleLevel)=titlelevel;
		dMagnetic2_engine_linea_flush(pVMLineA);	// make sure the output buffers are being flushed
		titlelevel=0;
	}
	if (!flag_headline && pVMLineA->headlineflagged)	// after the headline ends, a new paragraph is beginning
	{
		int i;
		pVMLineA->capital=1;	// obviously, this starts with a capital letter
		for (i=0;i<titlelevel;i++)
		{
			if (pVMLineA->pTitleBuf[i]<' ')
			{
				pVMLineA->pTitleBuf[i]=0;
				titlelevel--;
			}
		}
	}
	pVMLineA->headlineflagged=flag_headline;

	//newline=0;
	if (c==0xff)	// mark the next letter as Capital
	{
		pVMLineA->capital=1;
	} else {
		c2=c&0x7f;	// the highest bit was an end marker for the hufman tree in the dictionary
		// THE RULES FOR THE OUTPUT ARE:
		// replace tabs and _ with space.
		// the headline is printed in upper case letters.
		// after a . there has to be a space.
		// and after a . The next letter has to be uppercase.
		// multiple spaces are to be reduced to a single one.
		// the characters ~ and ^ are to be translated into line feeds.
		// the caracter 0xff makes the next one upper case.
		// after a second newline comes a capital letter.
		// the special marker '@' is either an end marker, or must be substituted by an 's', so that "He thank@ you", and "It contain@ a key" become gramatically correct.
		if (!(pVMLineA->jinxterslide))
		{
			if (c2==9 || c2=='_') c2=' ';
			if (flag_headline && (c2==0x5f || c2==0x40)) c2=' ';	// in a headline, those are the control codes for a space.
			if (c2==0x40) 	// '@' is a special character
			{
				if (controlD2 || pVMLineA->lastchar==' ') return 0;	// When D2 is set, or the last character was a whitespace, it is an end marker
				else c2='s';						// otherwise it must be substituted for an 's'. One example would be "It contain@ a key".
			}
			if (c2==0x5e || c2==0x7e) c2=0x0a;	// ~ or ^ is actually a line feed.
			if (c2==0x0a && pVMLineA->lastchar==0x0a) 	// after two consequitive newlines comes a capital letter.
			{
				pVMLineA->capital=1;
			}
			if (c2=='.' || c2=='!' || c2==':' || c2=='?')	// a sentence is ending.
			{
				pVMLineA->capital=1;
			}
			if (((c2>='a' && c2<='z') || (c2>='A' && c2<='Z')) && (pVMLineA->capital||flag_headline)) 	// the first letter must be written as uppercase. As well as the headline.
			{
				pVMLineA->capital=0;	// ONLY the first character
				c2&=0x5f;	// upper case
			}
			//newline=0;
			if (
					(pVMLineA->lastchar=='.' || pVMLineA->lastchar=='!' || pVMLineA->lastchar==':' || pVMLineA->lastchar=='?'|| pVMLineA->lastchar==',' || pVMLineA->lastchar==';') 	// a sentence as ended
					&&  ((c2>='A' && c2<='Z') ||(c2>='a' && c2<='z') ||(c2>='0' && c2<='9'))) 	// and a new one is beginning.
			{
				// after those letters comes an extra space.otherwise,it would look weird.
				if (flag_headline) 
				{
					if (titlelevel<DMAGNETIC2_SIZE_TITLEBUF-1)
						pVMLineA->pTitleBuf[titlelevel++]=' ';
				} else {
					if (textlevel<DMAGNETIC2_SIZE_OUTPUTBUF-1)
					{
						pVMLineA->pTextBuf[textlevel++]=' ';
					}
				}
			}
			if (textlevel>0 && pVMLineA->lastchar==' ' && (c2==',' || c2==';' || c2=='.' || c2=='!'))	// there have been some glitches with extra spaces, right before a komma. which , as you can see , looks weird.
			{
				textlevel--;
			}
			if (	//allow multiple spaces in certain scenarios
					flag_headline || pVMLineA->lastchar!=' ' || c2!=' ')	// combine multiple spaces into a single one.
			{
				if (c2==0x0a || (c2>=32 && c2<127 && c2!='@')) 
				{
					if (flag_headline) 
					{
						if (titlelevel<DMAGNETIC2_SIZE_TITLEBUF-1)
						{
							if ((c2&0x7f)>=' ')
							{
								pVMLineA->pTitleBuf[titlelevel++]=c2&0x7f;
							} else {
								pVMLineA->pTitleBuf[titlelevel++]=0;
							}
						}
					} else if (textlevel<DMAGNETIC2_SIZE_OUTPUTBUF-1) {
						if (textlevel<DMAGNETIC2_SIZE_OUTPUTBUF-1)
						{
							pVMLineA->pTextBuf[textlevel++]=c2;
						}

						//					if (c2=='\n') newline=1;
					}
					pVMLineA->lastchar=c2;
				}
			}
		} else if (c2) {
			if (c2==0x5e) c2='\n';
			if (c2=='_') c2=' ';
			pVMLineA->pTextBuf[textlevel++]=c2;
			pVMLineA->lastchar=c2;
		}
	}
	// make sure that the buffers are zero-terminated.
	if (titlelevel>=0)
	{
		*pStatus|=DMAGNETIC2_ENGINE_STATUS_NEW_TITLE;
	}
	if (textlevel>=0)
	{
		*pStatus|=DMAGNETIC2_ENGINE_STATUS_NEW_TEXT;
	}
	pVMLineA->pTitleBuf[titlelevel]=0;
	pVMLineA->pTextBuf[textlevel]=0;
	*(pVMLineA->pTextLevel)=textlevel;
	*(pVMLineA->pTitleLevel)=titlelevel;
	if (titlelevel>=(DMAGNETIC2_SIZE_TITLEBUF-1) || textlevel>=(DMAGNETIC2_SIZE_OUTPUTBUF-1))
	{
		dMagnetic2_engine_linea_flush(pVMLineA);	// the buffers are full, and flushing them is required
	}

	return DMAGNETIC2_OK;
}


