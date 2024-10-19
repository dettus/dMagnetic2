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
./loader_mkmaggfx.app games/jinxter.mag games/jinxter.gfx	
mv test.mag test_maggfx.mag
mv test.gfx test_maggfx.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_maggfx.mag


echo ">>> amstradcpc <<<"
./loader_mkmaggfx.app games/amstradcpc/JINXTER1.DSK games/amstradcpc/JINXTER2.DSK
mv test.mag test_amstradcpc.mag
mv test.gfx test_amstradcpc.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_amstradcpc.mag


echo ">>> atarixl <<<"
./loader_mkmaggfx.app games/atarixl/Jinxter_a.atr games/atarixl/Jinxter_b.atr
mv test.mag test_atarixl.mag
mv test.gfx test_atarixl.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_atarixl.mag


echo ">>> apple ii (nib) <<<"
./loader_mkmaggfx.app games/appleii/JINXTER1.NIB games/appleii/JINXTER2.NIB 
mv test.mag test_appleii_nib.mag
mv test.gfx test_appleii_nib.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_appleii_nib.mag




echo ">>> apple ii (2mg) <<<"
./loader_mkmaggfx.app "games/appleii/CorruptionA(dosVol114).2mg" "games/appleii/CorruptionB(dosVol115).2mg" "games/appleii/CorruptionC(dosVol116).2mg"
mv test.mag test_appleii_2mg.mag
mv test.gfx test_appleii_2mg.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_appleii_2mg.mag



echo ">>> apple ii (woz) <<<"
./loader_mkmaggfx.app "games/appleii/Corruption disk 1.woz" "games/appleii/Corruption disk 2.woz" "games/appleii/Corruption disk 3.woz"
mv test.mag test_appleii_woz.mag
mv test.gfx test_appleii_woz.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_appleii_woz.mag




echo ">>> archimedes (adf) <<<"
./loader_mkmaggfx.app games/archimedes/jinxter.adf
mv test.mag test_archimedes_adf.mag
mv test.gfx test_archimedes_adf.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_archimedes_adf.mag

echo ">>> archimedes (adl) <<<"
./loader_mkmaggfx.app games/archimedes/jinxter_im_arc_800.adl
mv test.mag test_archimedes_adl.mag
mv test.gfx test_archimedes_adl.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_archimedes_adl.mag



echo ">>> Commodore C64 <<<"
./loader_mkmaggfx.app games/d64/jinxter1.d64 games/d64/jinxter2.d64
mv test.mag test_c64.mag
mv test.gfx test_c64.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_c64.mag


echo ">>> Magnetic Windows <<<"
./loader_mkmaggfx.app games/magneticwindows/Wonder/TWO.RSC
mv test.mag test_mw.mag
mv test.gfx test_mw.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_mw.mag


echo ">>> MSDOS <<<"
./loader_mkmaggfx.app games/msdos/jinxter/
mv test.mag test_msdos.mag
mv test.gfx test_msdos.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_msdos.mag

echo ">>> spectrum +3 <<<"
./loader_mkmaggfx.app games/spectrum/Jinxter.dsk
mv test.mag test_spectrum.mag
mv test.gfx test_spectrum.gfx
dMagnetic -vrows 2 -vcols 80 -mag test_spectrum.mag

