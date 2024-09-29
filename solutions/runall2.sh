#!/bin/sh
(
echo ">>>>>>>>>>>>>>> CORRUPTION <<<<<<<<<<<<<<<<<<<"
cat solution_corruption.log	| ../software/dMagnetic corruption	-vmode none -vcols 80
echo ">>>>>>>>>>>>>>> FISH <<<<<<<<<<<<<<<<<<<"
cat solution_fish.log		| ../software/dMagnetic fish		-vmode none -vcols 80
echo ">>>>>>>>>>>>>>> GUILD <<<<<<<<<<<<<<<<<<<"
cat solution_guild.log		| ../software/dMagnetic guild		-vmode none -vcols 80
echo ">>>>>>>>>>>>>>> JINXTER <<<<<<<<<<<<<<<<<<<"
cat solution_jinxter.log	| ../software/dMagnetic jinxter		-vmode none -vcols 80
echo ">>>>>>>>>>>>>>> MYTH <<<<<<<<<<<<<<<<<<<"
cat solution_myth.log		| ../software/dMagnetic myth		-vmode none -vcols 80
echo ">>>>>>>>>>>>>>> PAWN <<<<<<<<<<<<<<<<<<<"
cat solution_pawn.log		| ../software/dMagnetic pawn		-vmode none -vcols 80
echo ">>>>>>>>>>>>>>> WONDERLAND <<<<<<<<<<<<<<<<<<<"
cat solution_wonderland.log	| ../software/dMagnetic wonderland	-vmode none -vcols 80
) | tee outcome.txt


