#!/bin/sh

OPK_NAME=czdoom.opk

echo ${OPK_NAME}

# create default.retrofw.desktop
cat > default.retrofw.desktop <<EOF
[Desktop Entry]
Name=CZDoom
Comment=Limit-removing Doom Engine
Exec=czdoom.dge \$f
Icon=czdoom
Categories=games;
X-OD-Alias=aliases.txt
X-OD-Manual=czdoom.man.txt
X-OD-Selector=/home/retrofw/roms/doom
X-OD-Filter=.wad,.WAD,.doom,.DOOM
Terminal=false
Type=Application
StartupNotify=true
EOF

# create opk
FLIST="src/prboom"
FLIST="${FLIST} default.retrofw.desktop"
FLIST="${FLIST} data/prboom.wad"
FLIST="${FLIST} opk/*"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.retrofw.desktop
rm -f default.retrofw.desktop
