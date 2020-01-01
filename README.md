# Notes

[![Join the chat at https://gitter.im/nuttyartist/notes](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/nuttyartist/notes?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/nuttyartist/notes.svg?branch=dev)](https://travis-ci.org/nuttyartist/notes)
[![Build status](https://ci.appveyor.com/api/projects/status/rgque4o6x2y0i92i?svg=true)](https://ci.appveyor.com/project/nuttyartist/notes)

Notes is an open source, cross-platform note taking app which has the potential to be something much bigger, and much better at empowering the people using it.
This is where you come into play. Be part of making Notes better.

Next version: [1.5.0 (see project)](https://github.com/nuttyartist/notes/projects/7)  
Future versions mockups can be seen on our [Vision](https://github.com/nuttyartist/notes/wiki/Vision) page in our wiki.

## Screenshot
![notes_screenshot](https://user-images.githubusercontent.com/16375940/29837038-bc4e58a4-8cff-11e7-9fb1-692e9948c33c.png)


## How to clone
Use this command to clone the repository:

```shell
$> git clone --recursive  https://github.com/nuttyartist/notes.git
```

## Dependencies
Make sure the Qt (>= 5.2.1) development libraries are installed:

- Debian/Ubuntu : qt5-default build-essential qtbase5-private-dev sqlite3

## Compiling

```shell
$> mkdir build
$> cd build
$> qmake ../src
$> make -j4
```

## Notes

The notes database and settings file are stored:

**Windows** : ```C:\Users\user\AppData\Roaming\Awesomeness```  
**Linux** &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: ```/home/user/.config/Awesomeness/``` **or** ```/home/snap/notes/x1/.config/Awesomeness``` **(using snap)**  
**Mac** &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: ```/home/.config/Awesomeness/```  

## Information for Contributors

Check out our [wiki](https://github.com/nuttyartist/notes/wiki), we discuss there our philosophy, our current state and the future version of Notes.

## Information for End Users

You can find more info about Notes and how to download it here: <a href="http://get-notes.com" target="_blank">get-notes.com</a>
