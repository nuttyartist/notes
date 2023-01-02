cmake_minimum_required(VERSION 3.10)

set(APP_VERSION 2.0.0)

# Any of the options below can be changed by passing -D<option name>=<value> to CMake.
set(CMAKE_OSX_DEPLOYMENT_TARGET
    "10.15"
    CACHE STRING "Minimum macOS version to target for deployment")

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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

if(NOT DEFINED QT_VERSION_MAJOR)
  message(FATAL_ERROR "Qt not found.")
else()
  message("Using QT_VERSION_MAJOR: ${QT_VERSION_MAJOR}")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent Gui Network
                                                       Sql Widgets Test)

set(GENERAL_PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/aboutwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/allnotebuttontreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/customapplicationstyle.cpp
    ${PROJECT_SOURCE_DIR}/src/customdocument.cpp
    ${PROJECT_SOURCE_DIR}/src/dbmanager.cpp
    ${PROJECT_SOURCE_DIR}/src/defaultnotefolderdelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/editorsettingsbutton.cpp
    ${PROJECT_SOURCE_DIR}/src/elidedlabel.cpp
    ${PROJECT_SOURCE_DIR}/src/foldertreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/labeledittype.cpp
    ${PROJECT_SOURCE_DIR}/src/listviewlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/mainwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/nodedata.cpp
    ${PROJECT_SOURCE_DIR}/src/nodepath.cpp
    ${PROJECT_SOURCE_DIR}/src/nodetreedelegate.cpp
    ${PROJECT_SOURCE_DIR}/src/nodetreemodel.cpp
    ${PROJECT_SOURCE_DIR}/src/nodetreeview.cpp
    ${PROJECT_SOURCE_DIR}/src/noteeditorlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistdelegate.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistdelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/notelistview.cpp
    ${PROJECT_SOURCE_DIR}/src/pushbuttontype.cpp
    ${PROJECT_SOURCE_DIR}/src/singleinstance.cpp
    ${PROJECT_SOURCE_DIR}/src/splitterstyle.cpp
    ${PROJECT_SOURCE_DIR}/src/styleeditorwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/tagdata.cpp
    ${PROJECT_SOURCE_DIR}/src/taglistdelegate.cpp
    ${PROJECT_SOURCE_DIR}/src/taglistmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/taglistview.cpp
    ${PROJECT_SOURCE_DIR}/src/tagpool.cpp
    ${PROJECT_SOURCE_DIR}/src/tagtreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/trashbuttondelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/treeviewlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/updaterwindow.cpp
    # ui
    ${PROJECT_SOURCE_DIR}/src/aboutwindow.ui
    ${PROJECT_SOURCE_DIR}/src/mainwindow.ui
    ${PROJECT_SOURCE_DIR}/src/styleeditorwindow.ui
    ${PROJECT_SOURCE_DIR}/src/updaterwindow.ui
    # 3rdParty ui
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qplaintexteditsearchwidget.ui
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Downloader.ui
    # 3rdParty source files
    ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/src/qautostart.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/src/qautostart.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/markdownhighlighter.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qmarkdowntextedit.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qownlanguagedata.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qplaintexteditsearchwidget.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Downloader.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/QSimpleUpdater.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/Updater.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobal.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut.cpp
    # qrc files
    ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/media.qrc
    ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/etc/resources/qsimpleupdater.qrc
    ${PROJECT_SOURCE_DIR}/src/fonts.qrc
    ${PROJECT_SOURCE_DIR}/src/images.qrc
    ${PROJECT_SOURCE_DIR}/src/styles.qrc)

file(
  GLOB_RECURSE
  HEADERS
  ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/qautostart.h
  ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/linenumberarea.h
  ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/markdownhighlighter.h
  ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qmarkdowntextedit.h
  ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qownlanguagedata.h
  ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit/qplaintexteditsearchwidget.h
  ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/include/*.h
  ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/src/*.h
  ${PROJECT_SOURCE_DIR}/3rdParty/qxt/*.h
  ${PROJECT_SOURCE_DIR}/src/*.h)

set(MACX_PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_mac.cpp
    ${PROJECT_SOURCE_DIR}/src/framelesswindow.mm
    ${PROJECT_SOURCE_DIR}/src/images/notes_icon.icns)

set(WIN32_PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_win.cpp
    ${PROJECT_SOURCE_DIR}/src/framelesswindow.cpp
    ${PROJECT_SOURCE_DIR}/src/images/notes.rc)

set(LINUX_PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/3rdParty/qxt/qxtglobalshortcut_x11.cpp)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${GENERAL_PROJECT_SOURCES}
                                                   ${HEADERS})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR}/src/
         ${PROJECT_SOURCE_DIR}/3rdParty/QSimpleUpdater/include
         ${PROJECT_SOURCE_DIR}/3rdParty/qautostart/src
         ${PROJECT_SOURCE_DIR}/3rdParty/qmarkdowntextedit
         ${PROJECT_SOURCE_DIR}/3rdParty/qxt)

target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC APP_VERSION="${PROJECT_VERSION}" QT_DISABLE_DEPRECATED_BEFORE=0x050900
         QSU_INCLUDE_MOC=1)

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX_BUNDLE_ICON_FILE notes_icon.icns)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/images/notes_icon.icns
                              PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  target_sources(${PROJECT_NAME} PUBLIC ${MACX_PROJECT_SOURCES})

  target_link_libraries(${PROJECT_NAME} PUBLIC "-framework Carbon"
                                               "-framework Cocoa")
elseif(UNIX)
  # On Linux, it's conventional for executables to be all lowercase.
  string(TOLOWER ${PROJECT_NAME} BINARY_NAME)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${BINARY_NAME})

  target_sources(${PROJECT_NAME} PUBLIC ${LINUX_PROJECT_SOURCES})

  target_link_libraries(${PROJECT_NAME} PUBLIC X11)
elseif(WIN32)
  target_sources(${PROJECT_NAME} PUBLIC ${WIN32_PROJECT_SOURCES})

  target_compile_definitions(${PROJECT_NAME} PUBLIC QXT_STATIC)

  target_link_libraries(${PROJECT_NAME} PUBLIC user32)
endif()

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION bin
  LIBRARY DESTINATION lib)
