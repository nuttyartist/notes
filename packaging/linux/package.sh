#!/bin/sh

# Create Debian package
read -p "Press enter to create Debian package" nothing
clear
sh make_deb.sh
echo "Debian package created"

# Create Snap package
read -p "Press enter to create Snap package" nothing
clear
sh make_snap.sh
echo "Snap package created"