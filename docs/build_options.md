These are options you can use to customize your own build of Notes.

You should set those while invoking CMake to build the project. See the [examples section](#examples) below.

### Options

| Name                                       | Default value | Supported values    | Description                                                 |
| ------------------------------------------ | ------------- | ------------------- | ----------------------------------------------------------- |
| `CMAKE_OSX_DEPLOYMENT_TARGET` (macOS-only) | `10.15`       | (any macOS version) | Minimum macOS version to target for deployment              |
| `USE_QT_VERSION`                           | (unset)       | `5` / `6`           | Use a specific version of Qt to build the app               |
| `GIT_REVISION`                             | `OFF`         | `ON` / `OFF`        | Append the current git revision to the app's version string |
| `UPDATE_CHECKER`                           | `ON`          | `ON` / `OFF`        | Enable or disable both the update checker and auto-updater  |

### Examples

To build Notes without any update-checking feature:

```
cmake -B build -DUPDATE_CHECKER=OFF
```

To force CMake to use Qt 5 (useful when you also have Qt 6 installed, which will be chosen by default, if present):

```
cmake -B build -USE_QT_VERSION=5
```

To build Notes in `Release` mode (for other modes, check the [CMake documentation](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)):

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
```
