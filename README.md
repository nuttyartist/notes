# Notes

[![Discord](https://dcbadge.vercel.app/api/server/RP6ReXRn5j?style=flat)](https://discord.gg/RP6ReXRn5j)
[![GitHub Actions status on Linux](https://github.com/nuttyartist/notes/actions/workflows/linux.yml/badge.svg?branch=dev)](https://github.com/nuttyartist/notes/actions/workflows/linux.yml?query=branch%3Adev)
[![GitHub Actions status on macOS](https://github.com/nuttyartist/notes/actions/workflows/macos.yml/badge.svg?branch=dev)](https://github.com/nuttyartist/notes/actions/workflows/macos.yml?query=branch%3Adev)
[![GitHub Actions status on Windows](https://github.com/nuttyartist/notes/actions/workflows/windows.yml/badge.svg?branch=dev)](https://github.com/nuttyartist/notes/actions/workflows/windows.yml?query=branch%3Adev)
[![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/github/nuttyartist/notes?branch=dev&svg=true)](https://ci.appveyor.com/project/nuttyartist/notes)

Notes is an open source and cross-platform note-taking app that is both beautiful and powerful.

Website: <a href="http://get-notes.com" target="_blank">get-notes.com</a>  
Our vision for the future on our [Wiki](https://github.com/nuttyartist/notes/wiki/Vision).

## Screenshots

![notes_screenshot_1](https://user-images.githubusercontent.com/16375940/188721143-df0a3584-011f-4ef0-a185-82066f9eb671.gif)
![notes_screenshot_2](https://user-images.githubusercontent.com/16375940/188721215-943dff96-fd61-48ad-a2c0-fa059db72152.gif)

## Features Overview

- Native app (written in C++ with Qt).
- Fast with a low memory footprint.
- Fully open source and cross-platform (Linux, macOS, Windows).
- Completely private - tracks nothing.
- Beautiful and sleek looking, yet still powerful.
- Folders and tags. Organize your ideas hierarchically using nested folders and universally using tags.
- Markdown Support. Format text without lifting your hands from the keyboard.
- Different themes. Switch between Light, Dark, and Sepia.
- Feed View. Select multiple notes to see them all one after another in the editor.
- Always runs in the background. Use the hotkey "Windows" + 'N' to summon Notes. "control" + "N" for macOS.
- Keyboard shortcuts. Meant to have the option to be used solely with a keyboard (but more work needs to be done there).
- What feature will you contribute?

## Support the project

If you use Notes daily, consider donating money so I can pay programmers to develop new features and fix bugs. I partner with [Github Sponsors](https://github.com/sponsors/nuttyartist) and [Patreon](https://www.patreon.com/rubymamis) to receive contributions. You can also put a bounty on specific issues using Bountysource. I currently make a living from ads on the website, but I'd like to stop with that.

## How to clone

Use this command to clone the repository:

```shell
$> git clone --recursive  https://github.com/nuttyartist/notes.git
```

## Dependencies

Make sure the Qt (>= 5.9) development libraries are installed:

- Debian/Ubuntu : qt5-default build-essential qtbase5-private-dev sqlite3

## Compiling

```shell
$> mkdir build
$> cd build
$> cmake .. -DCMAKE_BUILD_TYPE=Release
$> make -j4
```

## Database path

The notes database and settings file are stored in:

**Windows** : `C:\Users\user\AppData\Roaming\Awesomeness`  
**Linux** &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: `/home/user/.config/Awesomeness/` **or** `/home/snap/notes/x1/.config/Awesomeness` **(using snap)**  
**Mac** &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: `/home/user/.config/Awesomeness/`

## Contributers

### Developers:
Alex Spataru  
Ali Diouri  
David Planella  
Diep Ngoc  
Guilherme Silva  
Thorbjørn Lindeijer  
Tuur Vanhoutte  
Waqar Ahmed  

### Designers:
Kevin Doyle

And to the many of our beloved users who keep sending us feedback, you are an essential force in helping us improve, thank you!  

## Notes makes use of the following third-party libraries:

* [QMarkdownTextEdit](https://github.com/pbek/qmarkdowntextedit)
* [QSimpleUpdater](https://github.com/alex-spataru/QSimpleUpdater)
* [QAutostart](https://github.com/b00f/qautostart)
* [Qxt](https://bitbucket.org/libqxt/libqxt/src/master/)

## Notes makes use of the following open source fonts:

* Roboto
* Source Sans Pro
* Trykker
* Mate
* iA Writer Mono
* iA Writer Duo
* iA Writer Quattro
