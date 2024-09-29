#!/bin/sh

cat solution_pawn_dos.log | ../software/dMagnetic -msdosdir /home/games/magneticscrolls/brahman/pawn/ -vcols 80 -vrows 15 -vecho
cat solution_guild_pcversion.log | ../software/dMagnetic -msdosdir /home/games/magneticscrolls/brahman/guild/ -vcols 80 -vrows 15 -vecho

cat solution_corruption_dos.log | dMagnetic -msdosdir /home/games/magneticscrolls/brahman/corruption/ -vcols 80 -vrows 15 -vecho
cat solution_wonderland.log | ../software/dMagnetic -tworsc /home/games/magneticscrolls/brahman/wonderland/TWO.RSC -vcols 80 -vrows 15 -vecho
cat solution_myth_dos.log | dMagnetic -msdosdir /home/games/magneticscrolls/brahman/myth/ -vcols 80 -vrows 15 -vecho
cat solution_jinxter_pcversion.log | dMagnetic -msdosdir /home/games/magneticscrolls/brahman/jinxter/ -vcols 80 -vrows 15 -vecho


