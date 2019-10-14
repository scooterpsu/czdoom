#!/bin/sh

OPK_NAME=czdoom.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=CZDoom
Comment=Limit-removing Doom Engine
Exec=prboom -iwad $f
Terminal=false
Type=Application
StartupNotify=true
Icon=czdoom
Categories=games;
EOF

# create opk
FLIST="src/prboom"
FLIST="${FLIST} default.gcw0.desktop"
FLIST="${FLIST} src/distrib/data/*"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop
