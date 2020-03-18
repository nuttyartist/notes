#!/bin/sh

# Variables
license=gpl2
project="notes-1.5.0"
#project="notes-$(git describe --tags HEAD | cut -d- -f1 | sed 's/^v//')~git$(git rev-parse --short HEAD)"
authorEmail="awesomeness.notes@gmail.com"

# Remove old build
if [ -d "$project" ]; then
    printf '%s\n' "Removing old build ($project)"
    rm -rf "$project"
fi

# Generate folders
mkdir -p deb_build/$project
cd deb_build/$project

# Build binary
mkdir build
cd build
qmake ../../../../../src/Notes.pro
make -j4
mv notes ../
cd ..
rm -r build

# Copy icon & desktop file
cp ../../common/LICENSE license.txt
cp -a ../../common/icons .
cp ../../common/notes.desktop notes.desktop
cp ../../debian/copyright copyright
# Copy debian config to build directory
cp -avr ../../debian debian

# Generate source build & debian package"
DEBFULLNAME="Nutty Artist" EMAIL="awesomeness.notes@gmail.com" dh_make -y -s -c $license --createorig
dpkg-buildpackage -us -uc
