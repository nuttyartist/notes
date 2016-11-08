#!/bin/sh

# Check if snapcraft exists
echo "Checking for snapcraft..."
if type "snapcraft" &> /dev/null ; then
   echo "Snapcraft exists, we're good to go!"
else
   echo "Snapcraft not found, trying to install it..."
   sudo apt-get update
   sudo apt-get install snapcraft
   echo "Snapcraft installed, please re-run this script"
   exit
fi

# Generate directory
mkdir snap_build
cd snap_build

# Copy snapcraft config.
cp -r ../snap/* .

# Copy icons & desktop files
mkdir setup
cd setup
mkdir gui
cd gui
cp ../../../common/icons/scalable/notes.svg notes.svg
cp ../../../common/notes.desktop notes.desktop

# Go to snapcraft directory
cd ..
cd ..

# Clean snapcraft config
snapcraft clean

# Build snap
snapcraft snap

# Clean snapcraft config again
snapcraft clean
