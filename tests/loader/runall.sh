#!/bin/sh

# 
# BSD 2-Clause License
# 
# Copyright (c) 2024, dettus@dettus.net
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 

echo ">>> maggfx <<<"
./loader_mkmaggfx.app games/ccorrupt.mag games/ccorrupt.gfx
./loader_mkmaggfx.app games/cguild2.mag games/cguild2.gfx
./loader_mkmaggfx.app games/corrupt.mag games/corrupt.gfx
./loader_mkmaggfx.app games/fish.mag games/fish.gfx
./loader_mkmaggfx.app games/guild.mag games/guild.gfx
./loader_mkmaggfx.app games/jinxter.mag games/jinxter.gfx
./loader_mkmaggfx.app games/myth.mag games/myth.gfx
./loader_mkmaggfx.app games/pawn.mag games/pawn.gfx
./loader_mkmaggfx.app games/wonder.mag games/wonder.gfx


echo ">>> amstradcpc <<<"
./loader_mkmaggfx.app games/amstradcpc/CORRUPT1.DSK games/amstradcpc/CORRUPT2.DSK
./loader_mkmaggfx.app games/amstradcpc/GUILDOT1.DSK games/amstradcpc/GUILDOT2.DSK
./loader_mkmaggfx.app games/amstradcpc/JINXTER1.DSK games/amstradcpc/JINXTER2.DSK
./loader_mkmaggfx.app games/amstradcpc/PAWN1.DSK games/amstradcpc/PAWN2.DSK
./loader_mkmaggfx.app games/amstradcpc/corruption1.DSK games/amstradcpc/corruption2.DSK
./loader_mkmaggfx.app games/amstradcpc/guild1.DSK games/amstradcpc/guild2.DSK
./loader_mkmaggfx.app games/amstradcpc/jinxter1.DSK games/amstradcpc/jinxter2.DSK
./loader_mkmaggfx.app games/amstradcpc/pawn1.DSK games/amstradcpc/pawn2.DSK
./loader_mkmaggfx.app games/amstradcpc/pawn1_extended.dsk games/amstradcpc/pawn2_extended.dsk


echo ">>> atarixl <<<"
./loader_mkmaggfx.app games/atarixl/Guild_Of_Thieves_1.ATR games/atarixl/Guild_Of_Thieves_2.ATR
./loader_mkmaggfx.app games/atarixl/Jinxter_a.atr games/atarixl/Jinxter_b.atr
./loader_mkmaggfx.app games/atarixl/Pawn_side1.ATR games/atarixl/Pawn_side2.ATR
./loader_mkmaggfx.app games/atarixl/The\ Guild\ of\ Thieves\ -\ Disk\ 1.atr games/atarixl/The\ Guild\ of\ Thieves\ -\ Disk\ 2.atr

echo ">>> apple ii (nib) <<<"
./loader_mkmaggfx.app "games/appleii/CorruptionA(dosVol114).nib" "games/appleii/CorruptionB(dosVol115).nib" "games/appleii/CorruptionC(dosVol116).nib"
./loader_mkmaggfx.app "games/appleii/GUILD.NIB"
./loader_mkmaggfx.app games/appleii/JINXTER1.NIB games/appleii/JINXTER2.NIB 
./loader_mkmaggfx.app games/appleii/PAWN.NIB




echo ">>> apple ii (2mg) <<<"
./loader_mkmaggfx.app "games/appleii/CorruptionA(dosVol114).2mg" "games/appleii/CorruptionB(dosVol115).2mg" "games/appleii/CorruptionC(dosVol116).2mg"



echo ">>> apple ii (woz) <<<"
./loader_mkmaggfx.app "games/appleii/Corruption disk 1.woz" "games/appleii/Corruption disk 2.woz" "games/appleii/Corruption disk 3.woz"




echo ">>> archimedes (adf) <<<"
./loader_mkmaggfx.app games/archimedes/corruption.adf
./loader_mkmaggfx.app games/archimedes/fish.adf
./loader_mkmaggfx.app games/archimedes/guild.adf
./loader_mkmaggfx.app games/archimedes/jinxter.adf
#./loader_mkmaggfx.app games/archimedes/msc1.adf
#./loader_mkmaggfx.app games/archimedes/msc2.adf
#./loader_mkmaggfx.app games/archimedes/msc3.adf
#./loader_mkmaggfx.app games/archimedes/msc4.adf
#./loader_mkmaggfx.app games/archimedes/wonder1_arc.adf
#./loader_mkmaggfx.app games/archimedes/wonder2_arc.adf
#./loader_mkmaggfx.app games/archimedes/wonder3_arc.adf
#./loader_mkmaggfx.app games/archimedes/wonder4_arc.adf


echo ">>> archimedes (adl) <<<"
./loader_mkmaggfx.app games/archimedes/corrupt_im_arc_800.adl
./loader_mkmaggfx.app games/archimedes/corruption.adl
./loader_mkmaggfx.app games/archimedes/fish.adl
./loader_mkmaggfx.app games/archimedes/jinxter_im_arc_800.adl
#./loader_mkmaggfx.app games/archimedes/msc2.adl
#./loader_mkmaggfx.app games/archimedes/msc3.adl
#./loader_mkmaggfx.app games/archimedes/msc4.adl
./loader_mkmaggfx.app games/archimedes/thepawn_arc_800_saveread.adl
#./loader_mkmaggfx.app games/archimedes/wonder1_arc_800.adl
#./loader_mkmaggfx.app games/archimedes/wonder1_arc_800_saveread.adl
#./loader_mkmaggfx.app games/archimedes/wonder2_arc_800.adl
#./loader_mkmaggfx.app games/archimedes/wonder2_arc_800_saveread.adl
#./loader_mkmaggfx.app games/archimedes/wonder3_arc_800.adl
#./loader_mkmaggfx.app games/archimedes/wonder3_arc_800_saveread.adl
#./loader_mkmaggfx.app games/archimedes/wonder4_arc_800.adl
#./loader_mkmaggfx.app games/archimedes/wonder4_arc_800_saveread.adl



echo ">>> Commodore C64 <<<"
./loader_mkmaggfx.app games/d64/corruption1.d64 games/d64/corruption2.d64
./loader_mkmaggfx.app games/d64/fish1.d64 games/d64/fish2.d64
./loader_mkmaggfx.app games/d64/guild1.d64 games/d64/guild2.d64
./loader_mkmaggfx.app games/d64/jinxter1.d64 games/d64/jinxter2.d64
./loader_mkmaggfx.app games/d64/myth.d64
./loader_mkmaggfx.app games/d64/pawn1.d64 games/d64/pawn2.d64

echo ">>> Magnetic Windows <<<"
./loader_mkmaggfx.app games/magneticwindows/MSC/CTWO.RSC
./loader_mkmaggfx.app games/magneticwindows/MSC/FTWO.RSC
./loader_mkmaggfx.app games/magneticwindows/MSC/GTWO.RSC
./loader_mkmaggfx.app games/magneticwindows/Wonder/TWO.RSC


echo ">>> MSDOS <<<"
./loader_mkmaggfx.app games/msdos/corruption/
./loader_mkmaggfx.app games/msdos/fish/
./loader_mkmaggfx.app games/msdos/guild/
./loader_mkmaggfx.app games/msdos/jinxter/
./loader_mkmaggfx.app games/msdos/myth/
./loader_mkmaggfx.app games/msdos/pawn/

echo ">>> spectrum +3 <<<"
./loader_mkmaggfx.app games/spectrum/Corruptn.dsk
./loader_mkmaggfx.app games/spectrum/Fish.dsk
./loader_mkmaggfx.app games/spectrum/Jinxter.dsk
./loader_mkmaggfx.app "games/spectrum/Myth (Rainbird) - Side A.dsk"
./loader_mkmaggfx.app "games/spectrum/Myth (Rainbird) - Side B.dsk"
./loader_mkmaggfx.app games/spectrum/The_pawn.dsk
./loader_mkmaggfx.app games/spectrum/guild.dsk


