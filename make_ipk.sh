#!/bin/sh

mkdir -p ipkg/home/retrofw/games/czdoom
mkdir -p ipkg/home/retrofw/apps/gmenu2x/sections/games
cp src/prboom ipkg/home/retrofw/games/czdoom/czdoom
cp -R src/distrib/data/* ipkg/home/retrofw/games/czdoom/

cd ipkg

sed -e "s/^Version:.*/Version: $(date +%Y%m%d)/" ../src/distrib/control > control

echo "2.0" > debian-binary

cp ../src/distrib/czdoom home/retrofw/apps/gmenu2x/sections/games/czdoom
cp ../src/distrib/czdoom-modlaunch home/retrofw/apps/gmenu2x/sections/games/czdoom-modlaunch

tar -czvf control.tar.gz control --owner=0 --group=0
tar -czvf data.tar.gz home --owner=0 --group=0
ar rv ../czdoom.ipk control.tar.gz data.tar.gz debian-binary

cd ..
rm -r ipkg