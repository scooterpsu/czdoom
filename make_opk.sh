#!/bin/sh

OPK_NAME=czdoom.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=CZDoom
Comment=Limit-removing Doom Engine
Exec=czdoom -iwad \$f
Terminal=false
Type=Application
StartupNotify=true
Icon=czdoom
Categories=games;
EOF

# create modlaunch.gcw0.desktop
cat > modlaunch.gcw0.desktop <<EOF
[Desktop Entry]
Name=CZDoom Mods
Comment=CZDoom Mod Launcher
Exec=czdoom-modlaunch \$f
Terminal=false
Type=Application
StartupNotify=true
Icon=czdoom-modlaunch
Categories=games;
EOF

# copy/rename binary
cp src/prboom czdoom

# create opk
FLIST="czdoom"
FLIST="${FLIST} default.gcw0.desktop"
FLIST="${FLIST} modlaunch.gcw0.desktop"
FLIST="${FLIST} src/distrib/data/*"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop

cat modlaunch.gcw0.desktop
rm -f modlaunch.gcw0.desktop

# delete copied binary
rm -f czdoom
