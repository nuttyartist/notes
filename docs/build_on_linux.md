These are common steps to build Notes from source on Linux distributions.

### Requirements

It's impossible to create a guide that will work for all Linux distros out there, but thankfully the only major difference between all of them will be package names, so feel free to add the appropriate package names for your favorite distro down here *(alphabetically, please)*.

| Distro                       | Build dependencies[^1]                                                                        | Runtime dependencies[^2]                                                                                                                                                                                                                                    |
| ---------------------------- | --------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Arch Linux[^3]               | `cmake` `gcc` `git` `qt6-base` `qt6-declarative`                                              | `hicolor-icon-theme` `qt6-base` `qt6-declarative`                                                                                                                                                                                                           |
| Fedora 37 - 39               | `cmake` `gcc` `git` `libxkbcommon-devel` `qt6-qtbase-private-devel` `qt6-qtdeclarative-devel` | `qt6-qtbase-gui` `qt6-qtdeclarative`                                                                                                                                                                                                                        |
| openSUSE Leap 15[^4]         | `cmake` `gcc` `git` `qt6-base-private-devel` `qt6-declarative-devel`                          | `libQt6Concurrent6` `libQt6Gui6` `qt6-sql-sqlite`                                                                                                                                                                                                           |
| Ubuntu 20.04[^5]             | `cmake` `gcc` `git` `qtbase5-private-dev` `qt5qtdeclarative5-dev`                             | `libqt5network5` `libqt5sql5` `libqt5widgets5` `qml-module-qtquick2` `qml-module-qtquick-controls2` `qml-module-qtquick-window2`                                                                                                                            |
| Ubuntu 22.04 - 23.04         | `cmake` `gcc` `git` `qt6-base-private-dev` `qt6-declarative-dev` `libgl-dev`                  | `libqt6network6` `libqt6sql6` `libqt6widgets6` `qml6-module-qtqml-workerscript` `qml6-module-qtquick-controls` `qml6-module-qtquick-layouts` `qml6-module-qtquick-particles` `qml6-module-qtquick-templates` `qml6-module-qtquick-window` `qt6-qpa-plugins` |

[^1]: These packages are only required to build Notes, meaning you can remove all of them (or some of them) afterward.
[^2]: These packages are required to actually run Notes.
[^3]: We recommend building and installing through the [official AUR package](https://aur.archlinux.org/packages/notes).
[^4]: You may need to tell `cmake` to use use GCC 8 (or newer), e.g. run `export CXX=g++-10` before you invoke `cmake`.
[^5]: This distro can only build Notes with Qt 5.

### Build options

Please refer to [build_options.md](build_options.md).

### Building

First, use `git` to clone the project and its components, and then navigate into it:

```shell
git clone https://github.com/nuttyartist/notes.git --recurse-submodules
cd notes
```

Let's create a build folder called `build`:

```shell
mkdir build
cd build
```

After that, we're ready to build Notes!

Invoke CMake to configure and build the project into a folder called `build`, in [`Release` mode](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html):

```shell
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

To run the binary you just built (e.g. for testing purposes), you can simply execute the `notes` binary in the `build` folder:

```shell
./notes
```

If you want to install Notes like a regular, Linux desktop application (with its own desktop file and icons), you can simply run (as root):

```shell
make install
```

Alternatively, you can also create DEB or RPM packages (see the [Packaging section](#Packaging) below).

### Packaging

After building, you could also easily create DEB or RPM packages using [CPack](https://cmake.org/cmake/help/latest/manual/cpack.1.html):

```shell
# Create a DEB package
cpack -G DEB
# Create a RPM package
cpack -G RPM
```

Please note that it only makes sense to create DEB or RPM packages on the same distro you intend to install and run Notes on.
