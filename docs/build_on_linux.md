These are common steps to build Notes from source on Linux distributions.

### Requirements

It's impossible to create a guide that will work for all Linux distros out there, but thankfully the only major difference between all of them will be package names, so feel free to add the appropriate package names for your favorite distro down here *(alphabetically, please)*.

| Distro       | Packages                                                     |
| ------------ | ------------------------------------------------------------ |
| Arch Linux   | `base-devel` `cmake` `git` `qt6-base` `sqlite`               |
| Ubuntu 18.04 | `build-essential` `cmake` `qtbase5-private-dev` `sqlite3`    |
| Ubuntu 22.04 | `build-essential` `cmake` `qt6-base-private-dev` `libgl-dev` |

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

Invoke CMake to build the project into a folder called `build`, in [`Release` mode](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html):

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
