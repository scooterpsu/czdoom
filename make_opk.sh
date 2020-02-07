#!/bin/sh

OPK_NAME=czdoom.opk

echo ${OPK_NAME}

# create default.retrofw.desktop
cat > default.retrofw.desktop <<EOF
[Desktop Entry]
Name=CZDoom
Comment=Limit-removing Doom Engine
Exec=czdoom -iwad \$f
Terminal=false
Type=Application
StartupNotify=true
Icon=czdoom
Categories=games;
X-OD-Selector=/home/retrofw/roms/doom/
X-OD-Filter=.wad,.WAD
X-OD-Manual=czdoom.man.txt
EOF

# create modlaunch.retrofw.desktop
cat > modlaunch.retrofw.desktop <<EOF
[Desktop Entry]
Name=CZDoom Mods
Comment=CZDoom Mod Launcher
Exec=czdoom-modlaunch \$f
Terminal=false
Type=Application
StartupNotify=true
Icon=czdoom-modlaunch
Categories=games;
X-OD-Selector=/home/retrofw/roms/doom/mods/
X-OD-Alias=/home/retrofw/roms/doom/mods/aliases.txt
X-OD-Filter=.doom
EOF

# copy/rename binary
cp src/prboom czdoom

# create opk
FLIST="czdoom"
FLIST="${FLIST} default.retrofw.desktop"
FLIST="${FLIST} modlaunch.retrofw.desktop"
FLIST="${FLIST} src/distrib/data/*"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.retrofw.desktop
rm -f default.retrofw.desktop

cat modlaunch.retrofw.desktop
rm -f modlaunch.retrofw.desktop

# delete copied binary
rm -f czdoom
