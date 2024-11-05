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

#ifndef	DMAGNETIC2_ENGINE_VM68K_LOADSTORE_H
#define	DMAGNETIC2_ENGINE_VM68K_LOADSTORE_H
int dMagnetic2_engine_vm68k_getbytesize(tVM68k_types size);
int dMagnetic2_engine_vm68k_resolve_ea(tVM68k* pVM68k,tVM68k_next *pNext,tVM68k_types size,
	tVM68k_addrmodes addrmode,tVM68k_ubyte reg,
	tVM68k_uword legal,tVM68k_slong* ea);
int dMagnetic2_engine_vm68k_fetchoperand(tVM68k* pVM68k,tVM68k_bool extendsign,tVM68k_types size,tVM68k_slong ea,tVM68k_ulong* operand);
int dMagnetic2_engine_vm68k_calculateflags(tVM68k_next* pNext,tVM68k_ubyte flagmask,tVM68k_types size,tVM68k_ulong operand1,tVM68k_ulong operand2,tVM68k_uint64 result);
int dMagnetic2_engine_vm68k_calculateflags2(tVM68k_next* pNext,tVM68k_ubyte flagmask,tVM68k_instruction instruction,tVM68k_types datatype,tVM68k_ulong operand1,tVM68k_ulong operand2,tVM68k_uint64 result);
int dMagnetic2_engine_vm68k_storeresult(tVM68k* pVM68k,tVM68k_next* pNext,tVM68k_types size,tVM68k_slong ea,tVM68k_ulong result);

#endif
