cmake_minimum_required(VERSION 3.10)

set(APP_VERSION 2.2.1)
set(APP_ID "io.github.nuttyartist.notes")
set(APP_AUTHOR "Ruby Mamistvalove")
set(APP_AUTHOR_EMAIL "ruby.mamistvalove@gmail.com")
set(APP_WEBSITE "https://www.get-notes.com/")
set(APP_REPOSITORY "https://github.com/nuttyartist/notes")
string(TIMESTAMP CURRENT_YEAR "%Y")

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
option(PRO_VERSION "Enable or disable Notes Pro features" ON)

project(
  Notes
  VERSION ${APP_VERSION}
  DESCRIPTION "Note-taking app. Write down your thoughts"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
  include(CheckPIESupported)
  check_pie_supported()
endif()

if(PRO_VERSION)
  add_definitions(-DPRO_VERSION)
endif()

# List of Qt components required to build the app.
set(QT_COMPONENTS
    Core
    Concurrent
    Gui
    Network
    Sql
    Test
    Widgets
    Quick)

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
message(STATUS "Pro Version: ${PRO_VERSION}")
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
      string(PREPEND GIT_REV "+g")
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
    ${PROJECT_SOURCE_DIR}/src/editorsettingsoptions.h
    ${PROJECT_SOURCE_DIR}/src/editorsettingsoptions.cpp
    ${PROJECT_SOURCE_DIR}/src/elidedlabel.cpp
    ${PROJECT_SOURCE_DIR}/src/elidedlabel.h
    ${PROJECT_SOURCE_DIR}/src/foldertreedelegateeditor.cpp
    ${PROJECT_SOURCE_DIR}/src/foldertreedelegateeditor.h
    ${PROJECT_SOURCE_DIR}/src/fontloader.cpp
    ${PROJECT_SOURCE_DIR}/src/fontloader.h
    ${PROJECT_SOURCE_DIR}/src/labeledittype.cpp
    ${PROJECT_SOURCE_DIR}/src/labeledittype.h
    ${PROJECT_SOURCE_DIR}/src/listviewlogic.cpp
    ${PROJECT_SOURCE_DIR}/src/listviewlogic.h
    ${PROJECT_SOURCE_DIR}/src/lqtutils_enum.h
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
    # qrc files
    ${PROJECT_SOURCE_DIR}/src/fonts.qrc
    ${PROJECT_SOURCE_DIR}/src/images.qrc
    ${PROJECT_SOURCE_DIR}/src/styles.qrc)

if(QT_VERSION VERSION_LESS 6.0)
  list(APPEND SOURCES ${PROJECT_SOURCE_DIR}/src/qml.qrc)
endif()

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
  PUBLIC APP_VERSION="${PROJECT_VERSION}${GIT_REV}" APP_ID="${APP_ID}"
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

if(QT_VERSION VERSION_GREATER_EQUAL 6.2)
  set(QML_FILES
      src/qml/CheckBoxMaterial.qml
      src/qml/CustomHorizontalScrollBar.qml
      src/qml/CustomTextArea.qml
      src/qml/CustomTextField.qml
      src/qml/CustomVerticalScrollBar.qml
      src/qml/EditorSettings.qml
      src/qml/FontChooserButton.qml
      src/qml/FontIconLoader.qml
      src/qml/FontIconsCodes.qml
      src/qml/IconButton.qml
      src/qml/OptionItemButton.qml
      src/qml/SubscriptionWindow.qml
      src/qml/SwitchButton.qml
      src/qml/TextButton.qml
      src/qml/ThemeChooserButton.qml
      src/qml/TodoColumnDelegate.qml
      src/qml/TodoTaskDelegate.qml
      src/qml/Utilities.js
      src/qml/ViewChooserButton.qml
      src/qml/kanbanMain.qml)

  foreach(file IN LISTS QML_FILES)
    get_filename_component(fileName ${file} NAME)
    set_source_files_properties(${file} PROPERTIES QT_RESOURCE_ALIAS
                                                   ${fileName})

    # This whole shenanigans is required to silence compiler warnings on files auto-generated by rcc.
    # It isn't pretty, but unfortunately there's no nicer way to do this yet:
    # https://discourse.cmake.org/t/qt-autogen-source-files-custom-compilation-flags/3002
    string(REPLACE "." "_" file_qml_cpp ${fileName})

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                               "Clang")
      set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/${PROJECT_NAME}_src/qml/${file_qml_cpp}.cpp
        PROPERTIES COMPILE_FLAGS -w)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION GREATER_EQUAL
                                                     1920)
      set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/${PROJECT_NAME}_src/qml/${file_qml_cpp}.cpp
        PROPERTIES COMPILE_FLAGS /w)
    endif()
  endforeach()

  # Silence an extra warning given by Clang:
  # build/.rcc/qmlcache/Notes_qmlcache_loader.cpp#L46
  # must specify at least one argument for '...' parameter of variadic macro [-Wgnu-zero-variadic-macro-arguments]
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/${PROJECT_NAME}_qmlcache_loader.cpp
      PROPERTIES COMPILE_FLAGS -w)
  endif()

  qt_add_qml_module(
    ${PROJECT_NAME}
    URI
    ${APP_ID}
    VERSION
    ${PROJECT_VERSION}
    NO_PLUGIN
    RESOURCE_PREFIX
    "/qt/qml"
    QML_FILES
    ${QML_FILES}
    NO_RESOURCE_TARGET_PATH)
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
         Qt${QT_VERSION_MAJOR}::WidgetsPrivate
         Qt${QT_VERSION_MAJOR}::Quick)

if(APPLE)
  set(COPYRIGHT_TEXT
      "Copyright (c) 2015-${CURRENT_YEAR} ${APP_AUTHOR} and contributors.")

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
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME
                                                   ${PROJECT_NAME_LOWERCASE})

  find_package(X11 REQUIRED)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES})

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

  foreach(ICON_SIZE 128 256 512)
    install(
      FILES
        ${PROJECT_SOURCE_DIR}/packaging/linux/common/icons/${ICON_SIZE}x${ICON_SIZE}/notes.png
      DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps
      RENAME ${APP_ID}.png)
  endforeach()

  install(
    FILES ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME_LOWERCASE})

  # On Debian-based distros, compress the changelog file and install the copyright file.
  # FIXME: Find out if it's possible to only do this when creating a deb package with CPack.
  if(EXISTS /etc/debian_version)
    string(TIMESTAMP DATE_NOW_UTC "%a, %d %b %Y %H:%M:%S +0000")

    configure_file(${PROJECT_SOURCE_DIR}/packaging/linux/debian/changelog.in
                   ${CMAKE_BINARY_DIR}/changelog.Debian)

    add_custom_command(
      COMMAND gzip ARGS -9 -c -f -k -n ${CMAKE_BINARY_DIR}/changelog.Debian >
              ${CMAKE_BINARY_DIR}/changelog.gz
      OUTPUT ${CMAKE_BINARY_DIR}/changelog.gz)

    add_custom_target(changelog ALL DEPENDS ${CMAKE_BINARY_DIR}/changelog.gz)

    install(
      FILES ${CMAKE_BINARY_DIR}/changelog.gz
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME_LOWERCASE})

    configure_file(${PROJECT_SOURCE_DIR}/packaging/linux/debian/copyright.in
                   ${CMAKE_BINARY_DIR}/copyright)

    install(
      FILES ${CMAKE_BINARY_DIR}/copyright
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME_LOWERCASE})
  endif()

  set(CPACK_PACKAGE_NAME ${PROJECT_NAME_LOWERCASE})
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}${GIT_REV})
  set(CPACK_PACKAGE_VENDOR ${APP_AUTHOR})
  set(CPACK_PACKAGE_CONTACT "${APP_AUTHOR} <${APP_AUTHOR_EMAIL}>")
  set(CPACK_STRIP_FILES TRUE)

  # Package description.
  # Please keep the character count for every line at 80 (max) - it's a lintian rule.
  string(CONCAT CPACK_PACKAGE_DESCRIPTION
                "Note-taking app that is both beautiful and powerful.\n"
                " It is written in C++ using the Qt framework.")

  # Workaround for older CMake versions, where CPACK_PACKAGE_DESCRIPTION didn't exist.
  if(CMAKE_VERSION VERSION_LESS 3.12)
    string(CONCAT CPACK_DEBIAN_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}\n"
                  " ${CPACK_PACKAGE_DESCRIPTION}")
  endif()

  # deb-specific
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-qpa-plugins")
    if(QT_VERSION_MINOR VERSION_GREATER_EQUAL 2)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS
          "${CPACK_DEBIAN_PACKAGE_DEPENDS}, qml6-module-qtqml-workerscript, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-particles, qml6-module-qtquick-templates, qml6-module-qtquick-window"
      )
    endif()
  elseif(QT_VERSION_MAJOR VERSION_EQUAL 5)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "qml-module-qtquick2, qml-module-qtquick-controls2, qml-module-qtquick-window2"
    )
  endif()
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${APP_WEBSITE})
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

  # rpm-specific
  set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
  if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
    if(UNIX
       AND NOT APPLE
       AND EXISTS /etc/os-release)
      file(READ /etc/os-release DISTRO_INFO)
      if(DISTRO_INFO MATCHES "SUSE")
        # Note: Different from Fedora 36, the SQLite library we need is not a dependency
        # of the base Qt 6 package on openSUSE Leap 15.4, so we have to manually include it here.
        set(CPACK_RPM_PACKAGE_REQUIRES "qt6-sql-sqlite")
      endif()
    endif()
  endif()
  set(CPACK_RPM_PACKAGE_LICENSE MPL-2.0)
  set(CPACK_RPM_PACKAGE_URL ${APP_WEBSITE})
  set(CPACK_RPM_PACKAGE_GROUP Applications/Text)
  set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons)
  # Remove any leading whitespace character from the description text we added for the deb package.
  string(REGEX REPLACE "\n +" "\n" CPACK_RPM_PACKAGE_DESCRIPTION
                       ${CPACK_PACKAGE_DESCRIPTION})
  set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons)

  include(CPack)
elseif(WIN32)
  target_compile_definitions(${PROJECT_NAME} PUBLIC QXT_STATIC)

  target_link_libraries(${PROJECT_NAME} PUBLIC user32)
endif()

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION bin
  LIBRARY DESTINATION lib)
