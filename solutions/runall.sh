#!/bin/sh
(
echo ">>>>>>>>>>>>>>> CORRUPTION <<<<<<<<<<<<<<<<<<<"
cat solution_corruption.log	| ../software/dMagnetic corruption	-vmode none -vcols 80	| tail -n 20
echo ">>>>>>>>>>>>>>> FISH <<<<<<<<<<<<<<<<<<<"
cat solution_fish.log		| ../software/dMagnetic fish		-vmode none -vcols 80	| tail -n 20
echo ">>>>>>>>>>>>>>> GUILD <<<<<<<<<<<<<<<<<<<"
cat solution_guild.log		| ../software/dMagnetic guild		-vmode none -vcols 80	| tail -n 20
echo ">>>>>>>>>>>>>>> JINXTER <<<<<<<<<<<<<<<<<<<"
cat solution_jinxter.log	| ../software/dMagnetic jinxter		-vmode none -vcols 80	| tail -n 20
echo ">>>>>>>>>>>>>>> MYTH <<<<<<<<<<<<<<<<<<<"
cat solution_myth.log		| ../software/dMagnetic myth		-vmode none -vcols 80	| tail -n 20
echo ">>>>>>>>>>>>>>> PAWN <<<<<<<<<<<<<<<<<<<"
cat solution_pawn.log		| ../software/dMagnetic pawn		-vmode none -vcols 80	| tail -n 20
echo ">>>>>>>>>>>>>>> WONDERLAND <<<<<<<<<<<<<<<<<<<"
cat solution_wonderland.log	| ../software/dMagnetic wonderland	-vmode none -vcols 80	| tail -n 20
) | tee outcome.txt

diff outcome.txt expectedoutcome.log
md5sum outcome.txt	|| md5 outcome.txt
md5sum expectedoutcome.log	|| md5 expectedoutcome.log

