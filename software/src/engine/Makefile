
#BSD 2-Clause License
#
#Copyright (c) 2024, dettus@dettus.net
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
#1. Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#2. Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

CC?=gcc
AR?=ar
CFLAGS=-g -O0

CFLAGS+=-Wall
PROJ_HOME=../../

INCFLAGS=	\
	-I.	\
	-I$(PROJ_HOME)/include	\
	-I$(PROJ_HOME)/src	\
	-I$(PROJ_HOME)/src/shared	\


SOURCEFILES=	\
	dMagnetic2_engine.c	\
	dMagnetic2_engine_vm68k.c	\
	dMagnetic2_engine_vm68k_decode.c	\
	dMagnetic2_engine_vm68k_loadstore.c	\

OBJFILES=${SOURCEFILES:.c=.o}

all: libdmagnetic2_engine.a

clean:
	rm -f $(OBJFILES) libdmagnetic2_engine.a


libdmagnetic2_engine.a:	$(OBJFILES)
	$(AR) rs $@ $(OBJFILES)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_EXTRA) $(INCFLAGS) -c -o $@ $<


