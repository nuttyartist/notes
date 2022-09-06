# Notes

[![Join the chat at https://gitter.im/nuttyartist/notes](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/nuttyartist/notes?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/nuttyartist/notes.svg?branch=dev)](https://travis-ci.org/nuttyartist/notes)
[![Build status](https://ci.appveyor.com/api/projects/status/rgque4o6x2y0i92i?svg=true)](https://ci.appveyor.com/project/nuttyartist/notes)

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
- Completely private - tracks nothing. Check the code.
- Beautiful and sleek looking, yet still powerful.
- Folders and tags. Organize your ideas hierarchically using nested folders and universally using tags.
- Markdown Support. Format text without lifting your hands from the keyboard.
- Different themes. Switch between Light, Dark, and Sepia.
- Feed View. Select multiple notes to see them all one after another in the editor.
- Always runs in the background. Use the hotkey "Windows" + 'N' to summon Notes. "control" + "N" for macOS.
- Keyboard shortcuts. Meant to have the option to be used solely with a keyboard (but more work needs to be done there).
- What feature will you contribute?

## Support the project

If you use Notes daily, consider donating money so I can pay programmers to develop new features and fix bugs. I partner with [Patreon](https://www.patreon.com/rubymamis) to receive contributions (soon, I'll add Github Sponsors). You can also put a bounty on specific issues using Bountysource. I currently make a living from ads on the website, but I'd like to stop with that.

## How to clone
Use this command to clone the repository:

```shell
$> git clone --recursive  https://github.com/nuttyartist/notes.git
```

## Dependencies
Make sure the Qt (>= 5.3) development libraries are installed:

- Debian/Ubuntu : qt5-default build-essential qtbase5-private-dev sqlite3

## Compiling

```shell
$> mkdir build
$> cd build
$> qmake ../src
$> make -j4
```

## Notes

The notes database and settings file are stored in:

**Windows** : ```C:\Users\user\AppData\Roaming\Awesomeness```  
**Linux** &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: ```/home/user/.config/Awesomeness/``` **or** ```/home/snap/notes/x1/.config/Awesomeness``` **(using snap)**  
**Mac** &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: ```/home/.config/Awesomeness/```  
