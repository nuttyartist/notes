cmake_minimum_required(VERSION 3.10)

set(APP_VERSION 2.1.0)
set(APP_ID "io.github.nuttyartist.notes")

# Any of the options below can be changed by passing -D<option name>=<value> to CMake.
set(CMAKE_OSX_DEPLOYMENT_TARGET
    "10.15"
    CACHE STRING "Minimum macOS version to target for deployment")
set(USE_QT_VERSION
    ""
    CACHE STRING "Use a specific version of Qt to build the app")
option(GIT_REVISION
       "Append the current git revision to the app's version string" OFF)
option(UPDATE_CHECKER
       "Enable or disable both the update checker and auto-updater" ON)

project(
  Notes
  VERSION ${APP_VERSION}
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# List of Qt components required to build the app.
set(QT_COMPONENTS
    Core
    Concurrent
    Gui
    Network
    Sql
    Test
    Widgets)

# No Qt version was specified, try to use either Qt 6 (preferred) or Qt 5.
if(USE_QT_VERSION STREQUAL "")
  # This can probably be simplified once the following bug gets resolved:
  # https://gitlab.kitware.com/cmake/cmake/-/issues/23575
  message(STATUS "Trying to find Qt 6...")

  find_package(QT NAMES Qt6 QUIET)

  if(NOT DEFINED QT_VERSION_MAJOR)
    message(STATUS "Could not find Qt 6, will try Qt 5 next...")

    find_package(QT NAMES Qt5 QUIET)

    if(NOT DEFINED QT_VERSION_MAJOR)
      message(FATAL_ERROR "Could not find Qt 5, aborting compilation.")
    endif()
  endif()

  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})
else()
  if(NOT USE_QT_VERSION STREQUAL "5" AND NOT USE_QT_VERSION STREQUAL "6")
    message(
      FATAL_ERROR
        "An unsupported version of Qt was specified: '${USE_QT_VERSION}'. Valid values are: 5 or 6."
    )
  endif()
  message(STATUS "Trying to find Qt ${USE_QT_VERSION} (manually specified)...")

  find_package(QT NAMES Qt${USE_QT_VERSION} QUIET)

  if(NOT DEFINED QT_VERSION_MAJOR)
    message(
      FATAL_ERROR "Could not find Qt ${USE_QT_VERSION}, aborting compilation.")
  endif()

  find_package(Qt${USE_QT_VERSION} REQUIRED COMPONENTS ${QT_COMPONENTS})
endif()

message(STATUS "Success! Configuration details:")
message(STATUS "App name: ${PROJECT_NAME}")
message(STATUS "App version: ${PROJECT_VERSION}")
message(STATUS "Qt version: ${QT_VERSION}")
message(STATUS "Update checker: ${UPDATE_CHECKER}")
if(CMAKE_BUILD_TYPE STREQUAL "")
  message(STATUS "Build type: (not set)")
else()
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(GIT_REV "")
if(GIT_REVISION)
  if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
      COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_REV
      ERROR_VARIABLE GIT_REV_ERR
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
    if(GIT_REV)
      message(STATUS "Git revision: ${GIT_REV}")
      string(PREPEND GIT_REV "-g")
    else()
      message(FATAL_ERROR "Failed to retrieve git revision: ${GIT_REV_ERR}")
    endif()
  else()
    message(
      WARNING
        "The GIT_REVISION option was set, but this is not a git repository.")
  endif()
endif()

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/aboutwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/aboutwindow.h
    ${PROJECT_SOURCE_DIR}/src/allnotebuttontreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/allnotebuttontreedelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/customapplicationstyle.cpp
    ${PROJECT_SOURCE_DIR}/src/customapplicationstyle.h
    ${PROJECT_SOURCE_DIR}/src/customdocument.cpp
    ${PROJECT_SOURCE_DIR}/src/customDocument.h
    ${PROJECT_SOURCE_DIR}/src/customMarkdownHighlighter.cpp
    ${PROJECT_SOURCE_DIR}/src/customMarkdownHighlighter.h
    ${PROJECT_SOURCE_DIR}/src/dbmanager.cpp
    ${PROJECT_SOURCE_DIR}/src/dbmanager.h
    ${PROJECT_SOURCE_DIR}/src/defaultnotefolderdelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/defaultnotefolderdelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/editorsettingsbutton.cpp
    ${PROJECT_SOURCE_DIR}/src/editorsettingsbutton.h
    ${PROJECT_SOURCE_DIR}/src/elidedlabel.cpp
    ${PROJECT_SOURCE_DIR}/src/elidedlabel.h
    ${PROJECT_SOURCE_DIR}/src/foldertreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/foldertreedelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/labeledittype.cpp
    ${PROJECT_SOURCE_DIR}/src/labeledittype.h
    ${PROJECT_SOURCE_DIR}/src/listviewlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/listviewlogic.h
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/mainwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/mainwindow.h
    ${PROJECT_SOURCE_DIR}/src/nodedata.cpp
    ${PROJECT_SOURCE_DIR}/src/nodedata.h
    ${PROJECT_SOURCE_DIR}/src/nodepath.cpp
    ${PROJECT_SOURCE_DIR}/src/nodepath.h
    ${PROJECT_SOURCE_DIR}/src/nodetreedelegate.cpp
    ${PROJECT_SOURCE_DIR}/src/nodetreedelegate.h
    ${PROJECT_SOURCE_DIR}/src/nodetreemodel.cpp
    ${PROJECT_SOURCE_DIR}/src/nodetreemodel.h
    ${PROJECT_SOURCE_DIR}/src/nodetreeview.cpp
    ${PROJECT_SOURCE_DIR}/src/nodetreeview.h
    ${PROJECT_SOURCE_DIR}/src/nodetreeview_p.h
    ${PROJECT_SOURCE_DIR}/src/noteeditorlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/noteeditorlogic.h
    ${PROJECT_SOURCE_DIR}/src/notelistdelegate.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistdelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistdelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/notelistdelegate.h
    ${PROJECT_SOURCE_DIR}/src/notelistmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistmodel.h
    ${PROJECT_SOURCE_DIR}/src/notelistview.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistview.h
    ${PROJECT_SOURCE_DIR}/src/notelistview_p.h
    ${PROJECT_SOURCE_DIR}/src/pushbuttontype.cpp
    ${PROJECT_SOURCE_DIR}/src/pushbuttontype.h
    ${PROJECT_SOURCE_DIR}/src/singleinstance.cpp
    ${PROJECT_SOURCE_DIR}/src/singleinstance.h
    ${PROJECT_SOURCE_DIR}/src/splitterstyle.cpp
    ${PROJECT_SOURCE_DIR}/src/splitterstyle.h
    ${PROJECT_SOURCE_DIR}/src/styleeditorwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/styleeditorwindow.h
    ${PROJECT_SOURCE_DIR}/src/tagdata.cpp
    ${PROJECT_SOURCE_DIR}/src/tagdata.h
    ${PROJECT_SOURCE_DIR}/src/taglistdelegate.cpp
    ${PROJECT_SOURCE_DIR}/src/taglistdelegate.h
    ${PROJECT_SOURCE_DIR}/src/taglistmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/taglistmodel.h
    ${PROJECT_SOURCE_DIR}/src/taglistview.cpp
    ${PROJECT_SOURCE_DIR}/src/taglistview.h
    ${PROJECT_SOURCE_DIR}/src/tagpool.cpp
    ${PROJECT_SOURCE_DIR}/src/tagpool.h
    ${PROJECT_SOURCE_DIR}/src/tagtreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/tagtreedelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/trashbuttondelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/trashbuttondelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/treeviewlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/treeviewlogic.h
    # ui
    ${PROJECT_SOURCE_DIR}/src/aboutwindow.ui
    ${PROJECT_SOURCE_DIR}/src/mainwindow.ui
    ${PROJECT_SOURCE_DIR}/src/styleeditorwindow.ui
    # qrc files
    ${PROJECT_SOURCE_DIR}/src/fonts.qrc
    ${PROJECT_SOURCE_DIR}/src/images.qrc
    ${PROJECT_SOURCE_DIR}/src/styles.qrc)

# Third-party source files.
set(SOURCES_3RD_PARTY
    ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/src/qautostart.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/src/qautostart.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/linenumberarea.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/markdownhighlighter.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/markdownhighlighter.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qmarkdowntextedit.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qmarkdowntextedit.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qownlanguagedata.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qownlanguagedata.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qplaintexteditsearchwidget.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qplaintexteditsearchwidget.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobal.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobal.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut.h
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_p.h)

if(UPDATE_CHECKER)
  list(APPEND SOURCES ${PROJECT_SOURCE_DIR}/src/updaterwindow.cpp
       ${PROJECT_SOURCE_DIR}/src/updaterwindow.h
       ${PROJECT_SOURCE_DIR}/src/updaterwindow.ui)

  list(
    APPEND
    SOURCES_3RD_PARTY
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/include/QSimpleUpdater.h
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Downloader.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Downloader.h
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/QSimpleUpdater.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Updater.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Updater.h)
endif()

if(APPLE)
  list(APPEND SOURCES ${PROJECT_SOURCE_DIR}/src/framelesswindow.mm
       ${PROJECT_SOURCE_DIR}/src/framelesswindow.h
       ${PROJECT_SOURCE_DIR}/src/images/notes_icon.icns)

  list(APPEND SOURCES_3RD_PARTY
       ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_mac.cpp)
elseif(UNIX)
  list(APPEND SOURCES_3RD_PARTY
       ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_x11.cpp)
elseif(WIN32)
  list(APPEND SOURCES ${PROJECT_SOURCE_DIR}/src/framelesswindow.cpp
       ${PROJECT_SOURCE_DIR}/src/framelesswindow.h
       ${PROJECT_SOURCE_DIR}/src/images/notes.rc)

  list(APPEND SOURCES_3RD_PARTY
       ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_win.cpp)
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES}
                                                   ${SOURCES_3RD_PARTY})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

# Include third-party headers as 'system' files, in order to silence compiler warnings on them.
target_include_directories(
  ${PROJECT_NAME} SYSTEM
  PUBLIC ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/src
         ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit
         ${PROJECT_SOURCE_DIR}/3rdParty/qxt)

if(UPDATE_CHECKER)
  target_include_directories(
    ${PROJECT_NAME} SYSTEM
    PUBLIC ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/include)

  add_definitions(-DUPDATE_CHECKER)
endif()

target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC APP_VERSION="${PROJECT_VERSION}${GIT_REV}"
         QT_DISABLE_DEPRECATED_BEFORE=0x050900 QSU_INCLUDE_MOC=1)

# Enable useful compiler warnings (except on third-party code).
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                           "Clang")
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic
                                                -Wshadow)
  set_source_files_properties(${SOURCES_3RD_PARTY} PROPERTIES COMPILE_FLAGS -w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION GREATER_EQUAL
                                                 1920)
  target_compile_options(${PROJECT_NAME} PUBLIC /W4)
  set_source_files_properties(${SOURCES_3RD_PARTY} PROPERTIES COMPILE_FLAGS /w)
endif()

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC Qt${QT_VERSION_MAJOR}::Concurrent
         Qt${QT_VERSION_MAJOR}::Core
         Qt${QT_VERSION_MAJOR}::CorePrivate
         Qt${QT_VERSION_MAJOR}::Gui
         Qt${QT_VERSION_MAJOR}::GuiPrivate
         Qt${QT_VERSION_MAJOR}::Network
         Qt${QT_VERSION_MAJOR}::Sql
         Qt${QT_VERSION_MAJOR}::Widgets
         Qt${QT_VERSION_MAJOR}::WidgetsPrivate)

if(APPLE)
  string(TIMESTAMP CURRENT_YEAR "%Y")
  set(COPYRIGHT_TEXT
      "Copyright (c) 2015-${CURRENT_YEAR} Ruby Mamistvalove and contributors.")

  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
               MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_ID}
               MACOSX_BUNDLE_ICON_FILE "notes_icon.icns"
               MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
               MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
               MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
               MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT_TEXT}
               MACOSX_BUNDLE_INFO_STRING ${COPYRIGHT_TEXT})

  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/images/notes_icon.icns
                              PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  target_link_libraries(${PROJECT_NAME} PUBLIC "-framework Carbon"
                                               "-framework Cocoa")
elseif(UNIX)
  # On Linux, it's conventional for executables to be all lowercase.
  string(TOLOWER ${PROJECT_NAME} BINARY_NAME)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${BINARY_NAME})

  target_link_libraries(${PROJECT_NAME} PUBLIC X11)

  include(GNUInstallDirs)

  configure_file(${PROJECT_SOURCE_DIR}/packaging/linux/common/notes.desktop.in
                 ${CMAKE_BINARY_DIR}/${APP_ID}.desktop)
  install(FILES ${CMAKE_BINARY_DIR}/${APP_ID}.desktop
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

  configure_file(
    ${PROJECT_SOURCE_DIR}/packaging/linux/common/notes.appdata.xml.in
    ${CMAKE_BINARY_DIR}/${APP_ID}.metainfo.xml)
  install(FILES ${CMAKE_BINARY_DIR}/${APP_ID}.metainfo.xml
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)

  foreach(
    ICON_SIZE
    16
    22
    24
    32
    48
    64
    128
    256
    512)
    install(
      FILES
        ${PROJECT_SOURCE_DIR}/packaging/linux/common/icons/${ICON_SIZE}x${ICON_SIZE}/notes.png
      DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps
      RENAME ${APP_ID}.png)
  endforeach()

  install(
    FILES ${PROJECT_SOURCE_DIR}/packaging/linux/common/icons/scalable/notes.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
    RENAME ${APP_ID}.svg)
elseif(WIN32)
  target_compile_definitions(${PROJECT_NAME} PUBLIC QXT_STATIC)

  target_link_libraries(${PROJECT_NAME} PUBLIC user32)
endif()

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION bin
  LIBRARY DESTINATION lib)
