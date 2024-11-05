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

#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_graphics.h"	// for the datatypes
#include "dMagnetic2_shared.h"		// for the macros
#include "dMagnetic2_errorcodes.h"			// for the error codes

#define	PICTURE_MAX_RGB_VALUE		((1<<DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL)-1)
int dMagnetic2_gfxloader_gfx1(unsigned char* gfxbuf,int gfxsize,int picnum,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	int i;
	int retval;

	int picoffs;
	int width;
	int height;
	int tablesize;
	int datasize;

	int treeidx;
	int byteidx;
	unsigned char curpixel;
	unsigned char mask;
	unsigned char byte;
	unsigned char branch;
	int rle;
	unsigned int rgbs[DMAGNETIC2_GRAPHICS_MAX_COLORS];
	unsigned char xorbuf[DMAGNETIC2_GRAPHICS_MAX_WIDTH];
	int lineidx;

	retval=0;
	picnum&=0xffff;
	picoffs=READ_INT32BE(gfxbuf,8+4*picnum);	// the .gfx file starts with the index pointers to the actual picture data.
	if (picoffs==0x00000000 || picoffs>gfxsize)
	{
		retval=DMAGNETIC2_ERROR_WRONG_PICTUREFORMAT;	// the picture number was not correct
		return retval;
	}
	

	// once the offset has been calculated, the actual picture data is as followed:
	// bytes 0..1: UNKNOWN
	// bytes 0x02..0x03: X1
	// bytes 0x04..0x05: X2
	// X2-X1=width
	// bytes 0x06..0x07: height
	// bytes 0x08..0x1b: UNKNOWN
	// bytes 0x1c..0x3b: palette.
	// bytes 0x3c..0x3d: size of the huffmann table
	// bytes 0x3e..0x41: size of the data bit stream
	// 
	// bytes 0x42+0x42+tablesize: huffmann decoding table
	// bytes 0x42+tablesize..0x42+tablesize+datasize: bitstream
	curpixel=0;
	width=READ_INT16BE(gfxbuf,picoffs+4)-READ_INT16BE(gfxbuf,picoffs+2);
	for (i=0;i<width;i++)
	{
		xorbuf[i]=0;
	}
	lineidx=0;
	height=READ_INT16BE(gfxbuf,picoffs+6);
	// this particular graphics format has 3 bits per RGB channel
	for (i=0;i<16;i++)
	{
		unsigned short s;
		unsigned int red,green,blue;
		s=READ_INT16BE(gfxbuf,picoffs+0x1c+2*i);

		red	=(s>>8)&0xf;
		green	=(s>>4)&0xf;
		blue	=(s>>0)&0xf;

		red*=PICTURE_MAX_RGB_VALUE;green*=PICTURE_MAX_RGB_VALUE;blue*=PICTURE_MAX_RGB_VALUE;
		red/=7;green/=7;blue/=7;

		rgbs[i]=(red<<(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|(green<<(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|blue;


	}
	tablesize=READ_INT16BE(gfxbuf,picoffs+0x3c);	// size of the huffmann table
	datasize =READ_INT32BE(gfxbuf,picoffs+0x3e);	// size of the bitstream

	if (datasize>gfxsize)	// probably not correct
	{
		return DMAGNETIC2_ERROR_WRONG_PICTUREFORMAT;
	}


	// the huffmann table contains links. if a bit in the stream is set, the upper 8 bits, otherwise the lower ones.
	// terminal symbols have bit 7 set.
	byteidx=picoffs+0x42+tablesize*2+2;
	mask=0x00;
	byte=0;
	rle=0;
	for (i=0;(i<height*width) && (byteidx<gfxsize);i++)
	{
		if (rle==0)
		{
			treeidx=tablesize;	// start at the end of the huffmann table
			do
			{
				unsigned char branch0;
				unsigned char branch1;
				if (mask==0x00)			// when a full byte has been read
				{
					if (byteidx<gfxsize)
					{
						byte=gfxbuf[byteidx];	// get the next byte
						byteidx++;
					}
					mask=0x80;			// the bitstream is being read MSB first
				}
				branch1=gfxbuf[picoffs+0x42+2*treeidx+0];	// the order of the branches is that first comes the branch when the bit is set =1
				branch0=gfxbuf[picoffs+0x42+2*treeidx+1];	// then comes the branch for when the bit is clear =0
				branch=(byte&mask)?branch1:branch0;		
				mask>>=1;					// the bitstream is being read MSB first
				if (!(branch&0x80))				// if the highest bit is clear
				{
					treeidx=branch;
				}
			}
			while ((branch&0x80)==0x00);	// bit 7 denotes a terminal symbol
			branch&=0x7f;	// remove bit 7.
			if (branch>=0x10)	// it bits 6..4 were set, the previous pixels are being repeated
			{
				rle=branch-0x10;
			} else {	// since there are only 16 possible pixels, this is it.
				curpixel=branch;
				rle=1;	// will become 0 in the next revelation.
			}
		}
		xorbuf[lineidx]^=curpixel;	// each pixel is xored with the one above previous line
		if (pSmall!=NULL)
		{
			pSmall->pixels[i]=xorbuf[lineidx];
		}
		if (pLarge!=NULL)
		{
			pLarge->rgbpixels[i]=rgbs[xorbuf[lineidx]];
		}
		lineidx=(lineidx+1)%width;
		rle--;
	}
	if (pSmall!=NULL)
	{
		pSmall->height=height;
		pSmall->width=width;
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_VALID;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=rgbs[i];
		}
	}
	if (pLarge!=NULL)
	{
		pLarge->height=height;
		pLarge->width=width;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_VALID;
	}
	return retval;
}

int dMagnetic2_gfxloader_gfx2(unsigned char* gfxbuf,int gfxsize,char* picname,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
	int directorysize;
	int offset;
	//int length;
	int retval;
	int i;
	int j;
	int found;
	unsigned int rgbs[DMAGNETIC2_GRAPHICS_MAX_COLORS];
	int height;
	int width;

	

	directorysize=READ_INT16BE(gfxbuf,4);

	retval=DMAGNETIC2_OK;
	// step 1: find the correct filename
	found=0;
	offset=-1;
	for (i=0;i<directorysize && !found;i+=16)
	{
		// each entry in the directory is 16 bytes long. 8 bytes "filename", 4 bytes offset, 4 bytes length. filenames are 0-terminated.
		tVM68k_ubyte c1,c2;
		found=1;
		j=0;
		do
		{
			c1=gfxbuf[6+i+j];
			c2=picname[j];
			if ((c1&0x5f)!=(c2&0x5f)) found=0;	// compare, uppercase
			if ((c1&0x5f)==0) j=8;	// end of the entry reached.
			j++;
		} while (j<8 && found);
		if (found)
		{
			offset=READ_INT32BE(gfxbuf,i+6+8);		// this is the offset from the beginning of the gfxbuf
		}
	}
	// TODO: sanity check. is length-48==height*width/2?
	if (found && offset!=-1)
	{
		// each picture has the following format:
		// @0: 4 bytes UNKNOWN
		// @4..36 16*2 bytes palette
		// @38:      4 bytes data size (MIXED ENDIAN)
		// @42:      2 bytes width
		// @44:      2 bytes height
		// @48:      beginning of the bitmap block.
		// after the bitmap block is a magic field.
		// if it is !=0x5ed0, there is going to be an animation

		// the data format is as such: Each pixel consists of 4 bits:3210. in each line, 
		// the bits are lumped together, beginning with bit 0 of the first pixel, then 
		// bit 0 of the second pixel, then bit 0 of the third pixel and so on. (MSB first)
		// afterwards, the bit lump for bit 1 starts.
		//
		// 00000000111111112222222233333333
		// 00000000111111112222222233333333
		// 00000000111111112222222233333333
		// 00000000111111112222222233333333
		//
		int x,y;
		int idx0,idx1,idx2,idx3;
		int lumpsize;
		int datasize;
		int pixidx;

		for (i=0;i<16;i++)
		{
			unsigned short s;
			unsigned int red,green,blue;
			s=READ_INT16LE(gfxbuf,offset+4+2*i);

			red	=(s>>8)&0xf;
			green	=(s>>4)&0xf;
			blue	=(s>>0)&0xf;

			red*=PICTURE_MAX_RGB_VALUE;green*=PICTURE_MAX_RGB_VALUE;blue*=PICTURE_MAX_RGB_VALUE;
			red/=7;green/=7;blue/=7;

			rgbs[i]=(red<<(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|(green<<(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|blue;
		}
		datasize=READ_INT32ME(gfxbuf,offset+38);
		width=READ_INT16LE(gfxbuf,offset+42);
		height=READ_INT16LE(gfxbuf,offset+44);

		if (width>DMAGNETIC2_GRAPHICS_MAX_WIDTH)
		{
			retval=DMAGNETIC2_ERROR_WRONG_PICTUREFORMAT;
			return retval;
		}

		// animmagic=READ_INT16LE(gfx2buf,offset+48+data_size);
		lumpsize=datasize/(height/4);	// datasize=size of the picture. height: number of lines. thus: datasize/height=number of bytes per line. there are 4 lumps of bits per line.
		pixidx=0;
		for (y=0;y<height;y++)
		{
			tVM68k_ubyte byte0,byte1,byte2,byte3;
			tVM68k_ubyte mask;
			idx0=y*4*lumpsize+offset+48;
			idx1=idx0+1*lumpsize;
			idx2=idx0+2*lumpsize;
			idx3=idx0+3*lumpsize;
			for (x=0;x<width;x+=8)
			{
				tVM68k_ubyte p;
				byte0=gfxbuf[idx0++];
				byte1=gfxbuf[idx1++];
				byte2=gfxbuf[idx2++];
				byte3=gfxbuf[idx3++];
				mask=(1<<7);		// MSB FIRST
				for (i=0;i<8;i++)
				{
					p =(byte0&mask)?0x01:0;
					p|=(byte1&mask)?0x02:0;
					p|=(byte2&mask)?0x04:0;
					p|=(byte3&mask)?0x08:0;
					mask>>=1;
					if ((x+i)<width)
					{
						if (pSmall!=NULL)
						{
							pSmall->pixels[pixidx]=p;
						}
						if (pLarge!=NULL)
						{
							pLarge->rgbpixels[pixidx]=rgbs[p];
						}
						pixidx++;
					}
				}
			}
		}
	} 
	if (pSmall!=NULL)
	{
		pSmall->width=width;
		pSmall->height=height;
		pSmall->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_VALID;
		for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_COLORS;i++)
		{
			pSmall->rgb[i]=rgbs[i];
		}
	}

	if (pLarge!=NULL)
	{
		pLarge->width=width;
		pLarge->height=height;
		pLarge->flags=DMAGNETIC2_GRAPHICS_RENDER_FLAG_VALID;
	}
	return retval;
}

int dMagnetic2_gfxloader_gfx2_getpicname(unsigned char* gfxbuf,char* picname,int picnum)
{
	int idx;
	int entrynum;
	entrynum=READ_INT16BE(gfxbuf,4)/16;
	if (picnum>=entrynum)
	{
		picname[0]=0;
		return DMAGNETIC2_OK;
	}

	idx=4+2+picnum*(6+4+4+2);		// skip the header, the number of entries. and each entry has 6 Bytes name, 4 bytes offset, 4 bytes length, 2 bytes UNKNOWN
	memcpy(picname,&gfxbuf[idx],6);	
	picname[6]=0;			// make sure that it is zero-terminated


	return DMAGNETIC2_OK;

}

