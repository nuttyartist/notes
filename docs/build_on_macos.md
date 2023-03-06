These are common steps to build Notes from source on macOS.

### Requirements

macOS Catalina 10.15 (or newer) is required to build Notes.

This guide assumes you already have Xcode Command Line Tools (version 11 or newer) and [Homebrew](https://brew.sh/) both installed.

Required Homebrew packages:

- `git`
- `qt@6` *(`qt@5` is also supported, but not recommended)*
- `cmake`
- `ninja`

### Build options

Please refer to [build_options.md](build_options.md).

### Building

First, use `git` to clone the project and its components, and then navigate into it:

```shell
git clone https://github.com/nuttyartist/notes.git --recurse-submodules
cd notes
```

Optionally, if you want to dedicate all cores of your CPU to build Notes much faster, set this environment variable:

```shell
export CMAKE_BUILD_PARALLEL_LEVEL=$(sysctl -n hw.logicalcpu)
```

After that, we're ready to build Notes!

Invoke CMake to build the project into a folder called `build`, in [`Release` mode](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html):

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release'
```

Alternatively, if you want to target both `x86_64` and `arm64` (Apple Silicon) architectures in a single, universal binary, invoke CMake like this instead *(please note that this might be unsupported on Qt versions older than 6)*:

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES='x86_64;arm64'
```

After Notes is built, you can use the `macdeployqt` tool from Qt to include the required libraries to your app bundle:

```shell
macdeployqt 'build/Notes.app' -appstore-compliant
```

You can now install Notes by either dragging the `Notes.app` folder (found in `build/bin`), into the `Applications` folder.

Alternatively, you can run Notes directly by executing `build/bin/Notes.app/Contents/MacOS/Notes`.
