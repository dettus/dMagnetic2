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
#include "dMagnetic2_pictures.h"
#include "dMagnetic2_graphics.h"	// for the datatypes
#include "dMagnetic2_shared.h"		// for the macros
#include "dMagnetic2_errorcodes.h"			// for the error codes

#define	PICTURE_MAX_RGB_VALUE		((1<<DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL)-1)




// the gfx4 format is used to handle the pictures from the Magnetic Windows system.
// just like the gfx2, it starts with a directory. 6 byte picname, 4 bytes (little endian) offset, 4 bytes (little endian) length.
// at the offset, the picture is being comprised of the tree (type 7) and image (type 6) from the Magnetic Windows resource files.
// the tree is ALWAYS 609 bytes long. The size of the image varies.
//
// the huffmann tree uses 9 bits to encode 8 bits: the first 32 bytes are a bitmask (MSB first). Then the branches/symbols follow. 
// if the bit from the bitmask is set, it is a terminal symbol.
// 0x00...0x1f:  LEFTMASK		(0)
// 0x20...0x11f: LEFTBRANCH
// 0x120..0x13f: RIGHTMASK		(1)
// 0x140..0x23f: RIGHTBRANCH
// Byte 0x240    escape character (for run level encoding)
// Byte 0x241...0x250: EGA palette
// Byte 0x251...0x260: EGA palette pairs
//
//
// the image data looks like this:
// 0x00..0x03: magic header
// 0x04..0x23: 16x2 bytes RGB values (4 bits per channel, little endian, 0x0rgb)
// 0x24..0x25: width
// 0x26..0x27: height
// 0x28..0x29: transparency placeholder
// 0x2a..0x2b: size
// 0x2c......: bit stream. MSB first. when the bit is set (=1), follow the RIGHT branch.
//
// the run level encoding is signalled by the escape character from the tree (byte 0x240).
// in case the next character is 0xff, it actually an honest escape character.
// if any other value follows, it is the repeat num.
// the character AFTER that one is being repeated (4+repeat) number of times.
//
// each symbol is 8 bits large, 4 bits are the pixel. CAREFUL: bits 0..3 are the left, bits 4..7 are the right pixel
// in case the width is not divisible by 2, bits 4..7 are being ignored.
//
// the xor is being performed line by line.
//
// note that the end of the image comes BEFORE the end of the bitstream. (due to a bug in the encoder)
#include <string.h>

int dMagnetic2_gfxloader_magwin_getpicname(unsigned char* gfxbuf,char* picname,int picnum)
{
	int idx;
	int entrynum;
	entrynum=READ_INT16BE(gfxbuf,4)/14;
	if (picnum>=entrynum)
	{
		picname[0]=0;
		return DMAGNETIC2_OK;
	}

	idx=4+2+picnum*(6+4+4);		// skip the header, the number of entries. and each entry has 6 Bytes name, 4 bytes offset, 4 bytes length
	memcpy(picname,&gfxbuf[idx],6);	
	picname[6]=0;			// make sure that it is zero-terminated


	return DMAGNETIC2_OK;
}
int dMagnetic2_gfxloader_magwin(unsigned char* gfxbuf,int gfxsize,char* picname,int egamode,tdMagnetic2_canvas_small *pSmall,tdMagnetic2_canvas_large *pLarge)
{
#define	SIZEOFTREE	609
	int directorysize;
	int retval;
	int found;
	int i;
	int offset,length;
	int offset_vanilla,length_vanilla;
	int offset_ega,length_ega;
	int offset_anim,length_anim;
	int j;
	int width;
	int height;
	unsigned int rgbs[DMAGNETIC2_GRAPHICS_MAX_COLORS];
	unsigned char xorbuf[DMAGNETIC2_GRAPHICS_MAX_WIDTH/2];
	int xoridx;
	int xorbufsize;

	// the gfx4 buffer starts with the magic value, and then a directory
	retval=0;
	found=0;
	directorysize=READ_INT16BE(gfxbuf,4);
	offset_ega=offset_anim=offset_vanilla=-1;
	length_ega=length_anim=length_vanilla=-1;
	for (i=0;i<DMAGNETIC2_GRAPHICS_MAX_WIDTH/2;i++)
	{
		xorbuf[i]=0;
	}
	xoridx=0;
	for (i=0;i<directorysize && !found;i+=14)
	{
		tVM68k_ubyte c1,c2;
		found=1;
		j=0;
		do
		{
			c1=gfxbuf[6+i+j];
			c2=picname[j];
			if ((c1&0x5f)!=(c2&0x5f)) found=0;
			if ((c1&0x5f)==0) j=6;	// end of entry reached.
			j++;
		} while (j<6 && found);
		if (found)
		{
			int ega;
			int stillimage;
#define	STILLMAGIC	0x00005ed0
			const unsigned short egapalette[16]={0x000,0x005,0x050,0x055, 0x500,0x505,0x550,0x555, 0x222,0x007,0x070,0x077, 0x700,0x707,0x770,0x777};
			offset=READ_INT32LE(gfxbuf,i+6+6);
			length=READ_INT32LE(gfxbuf,i+6+10);

			// check if the image is not the ega image, and it is not an animation background.

			// the EGA images have a specific RGB palette
			// if not, it is not a EGA image
			ega=16;
			for (j=0;j<16;j++)
			{
				if (READ_INT16LE(gfxbuf,offset+SIZEOFTREE+4+2*j)!=egapalette[j]) ega--;
			}
			ega=(ega>=15);

			stillimage=1;
			if (READ_INT32LE(gfxbuf,offset+length-4)!=STILLMAGIC) stillimage=0;		// if they do not, it is an animation
			found=0;
			if (!ega && stillimage)
			{
				offset_vanilla=offset;
				length_vanilla=length;
			}

			if (ega) 
			{
				offset_ega=offset;
				length_ega=length;
			}
			if (!stillimage)
			{
				offset_anim=offset;
				length_anim=length;
			}
		}
	}
	// in case the image was not found
	offset=-1;
	length=-1;
	if (offset==-1)
	{
		offset=offset_vanilla;
		length=length_vanilla;
	}
	if (offset==-1)
	{
		offset=offset_anim;
		length=length_anim;
	}
	if (offset==-1 || egamode)
	{
		offset=offset_ega;
		length=length_ega;
	}
	if (offset!=-1 && length!=-1) found=1;

	if (found)
	{
		int treestart;
		int picstart;
		int size;
		int treeidx;
		int repnum;
		int rlestate;	// for the run length encoding. state 0: if not the escape char, output. otherwise -> state 1. in state 1, if the symbol is 0xff, the output is the escapechar. otherwise, the repition number (sans 4) -> state 4. in state 4, repeat the symbol
		tVM68k_ubyte escapechar;
		tVM68k_ubyte byte;
		tVM68k_ubyte mask;
		treestart=offset+0;
		picstart=offset+SIZEOFTREE;


		// byte 0x240 in the tree is the escape symbol for the run level encoding
		escapechar=gfxbuf[treestart+0x240];
		// bytes 0x04..0x23: RGB values
		for (i=0;i<16;i++)
		{
			unsigned short s;
			unsigned int red,green,blue;
			s=READ_INT16LE(gfxbuf,picstart+0x4+i*2);

			red	=(s>>8)&0xf;
			green	=(s>>4)&0xf;
			blue	=(s>>0)&0xf;

			red*=PICTURE_MAX_RGB_VALUE;green*=PICTURE_MAX_RGB_VALUE;blue*=PICTURE_MAX_RGB_VALUE;
			red/=7;green/=7;blue/=7;




			rgbs[i]=(red<<(2*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|(green<<(1*DMAGNETIC2_PICTURE_BITS_PER_RGB_CHANNEL))|blue;

		}
		// bytes 0x24,0x25= width
		// bytes 0x26,0x27= height
		width=READ_INT16LE(gfxbuf,picstart+0x24);
		height=READ_INT16LE(gfxbuf,picstart+0x26);
		xorbufsize=(width+1)/2;
		if (width>DMAGNETIC2_GRAPHICS_MAX_WIDTH)
		{
			retval=-1;
			return retval;
		}

		// bytes 0x2a,0x2b= size of the bitstream (in bytes)
		size=READ_INT16LE(gfxbuf,picstart+0x2a);
		j=0;
		treeidx=0;
		mask=0;byte=0;
		i=0;
		rlestate=0;
		repnum=1;

		// i is counting up the bytes in the bitstream.
		// j is counting up the pixels of the image
		while (((i<(length-SIZEOFTREE) && i<size ) || mask ) && j<(width*height))
		{
			tVM68k_ubyte term0,term1,term;
			tVM68k_ubyte branch0,branch1,branch;
			// the bitmask is denoting (MSB first) if an entry is a terminal symbol (=1) or a branch (=0)
			term0=gfxbuf[treestart+0x00 +treeidx/8]&(0x80>>(treeidx%8));
			term1=gfxbuf[treestart+0x120+treeidx/8]&(0x80>>(treeidx%8));

			// the entry in the table could either be a branch or a terminal symbol
			branch0=gfxbuf[treestart+0x20 +treeidx];
			branch1=gfxbuf[treestart+0x140+treeidx];


			if (mask==0)
			{
				mask=0x80;
				byte=gfxbuf[picstart+i+0x2c];
				i++;
			}

			term  =(byte&mask)?  term1:term0;
			branch=(byte&mask)?branch1:branch0;
			mask>>=1;


			if (term)
			{
				if (rlestate==0)
				{
					if (branch==escapechar) 
					{
						rlestate=1;
					} else {
						repnum=1;
					}
				} else if (rlestate==1) {
					if (branch==0xff)
					{
						branch=escapechar;
						repnum=1;
						rlestate=0;
					} else {
						repnum=branch+4;	// this form of RLE makes sense when the same byte was repeated 4 or more times in the source picture
						rlestate=2;
					}
				} else if (rlestate==2) {
					rlestate=0;	// the current entry is a terminal symbol, which is going to be repeated
				}
				if (rlestate==0)
				{
					while (repnum && j<(width*height))
					{
						int pl,pr;
						xorbuf[xoridx]^=branch;
						pl=(xorbuf[xoridx]>>0)&0xf;	// left pixel in the lower 4 bits
						pr=(xorbuf[xoridx]>>4)&0xf;	// right pixel in the higher 4 bits
						if (pSmall!=NULL)
						{
							pSmall->pixels[j]=pl;
						}
						if (pLarge!=NULL)
						{
							pLarge->rgbpixels[j]=rgbs[pl];
						}

						j++;
						// if the width of the image is not divisible by 2, ignore the very last pixel
						if (j%width)	// this is actually a clever way to skip the last pixel...
						{
							if (pSmall!=NULL)
							{
								pSmall->pixels[j]=pr;
							}
							if (pLarge!=NULL)
							{
								pLarge->rgbpixels[j]=rgbs[pr];
							}

							j++;
						}
						xoridx=(xoridx+1)%xorbufsize;
						repnum--;
					}
				}
				treeidx=0;	// go back to the start;
			} else {	// not a terminal symbol. keep following the branchesa
				treeidx=branch;
			}
		}
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



