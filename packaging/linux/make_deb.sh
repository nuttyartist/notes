#!/bin/sh

# Variables
license=custom
project="notes-0.9.0"
authorEmail="awesomeness.notes@gmail.com"

# Remove old build
if [ -d "$project" ]; then
    printf '%s\n' "Removing old build ($project)"
    rm -rf "$project"
fi

# Generate folders
mkdir deb_build
cd deb_build
mkdir $project
cd $project

# Build binary
mkdir build
cd build
qmake -qt5 ../../../../../src/Notes.pro
make -j4
mv notes ../
cd ..
rm -r build

#

# Copy icon & desktop file
cp ../../common/LICENSE license.txt
cp ../../common/notes.png notes.png
cp ../../common/notes.desktop notes.desktop

# Copy debian config to build directory
cp -avr ../../debian debian

# Generate source build & debian package"
dh_make -s -c $license --copyrightfile license.txt -e $authorEmail --createorig
dpkg-buildpackage
