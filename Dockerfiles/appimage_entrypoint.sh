#!/bin/bash

# Note: This build script is meant to be used within the container image created by 'appimage-*' Dockerfiles.

set -euo pipefail

# Script options:
#
# -d <dir>: Set build directory (overrides $BUILD_DIR below).
# -t <type>: Set build type (overrides $BUILD_TYPE below).
# -p: Enable 'Pro' features (overrides $PRO_VERSION below).
# -u: Enable the update checker feature (overrides $UPDATE_CHECKER below).
# -n: Do not add the current git revision to the app's version (overrides $GIT_REVISION below).
# -c <options>: Options passed to CMake's configure stage (overrides $CMAKE_CONFIG_OPTIONS below).
# -b <options>: Options passed to CMake's build stage (overrides $CMAKE_BUILD_OPTIONS below).
# -i <options>: Options passed to CMake's install stage (overrides $CMAKE_INSTALL_OPTIONS below).
# -a <dir>: Set linuxdeploy's AppDir directory (overrides $LINUXDEPLOY_APPDIR below).
# -l <options>: Options passed to the first run of linuxdeploy (overrides $LINUXDEPLOY_FIRST_OPTIONS below).
# -f <options>: Options passed to the final run of linuxdeploy (overrides $LINUXDEPLOY_FINAL_OPTIONS below).

# Hint: Pre-existing environment variables with the same name as the variables below will take precedence.
BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-release}"
PRO_VERSION="${PRO_VERSION:-OFF}"
UPDATE_CHECKER="${UPDATE_CHECKER:-ON}"
GIT_REVISION="${GIT_REVISION:-ON}"
CMAKE_CONFIG_OPTIONS="${CMAKE_CONFIG_OPTIONS:---warn-uninitialized -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DGIT_REVISION=${GIT_REVISION} -DCMAKE_INSTALL_PREFIX=/usr -DPRO_VERSION=${PRO_VERSION} -DUPDATE_CHECKER=${UPDATE_CHECKER}}"
CMAKE_BUILD_OPTIONS="${CMAKE_BUILD_OPTIONS:---build ${BUILD_DIR} --parallel $(nproc)}"
CMAKE_INSTALL_OPTIONS="${CMAKE_INSTALL_OPTIONS:---install ${BUILD_DIR}}"
LINUXDEPLOY_APPDIR="${LINUXDEPLOY_APPDIR:-Notes}"
LINUXDEPLOY_FIRST_OPTIONS="${LINUXDEPLOY_FIRST_OPTIONS:---appdir=${LINUXDEPLOY_APPDIR} --desktop-file=${LINUXDEPLOY_APPDIR}/usr/share/applications/${APP_ID}.desktop --library=/lib/$(arch)-linux-gnu/libssl.so.1.1 --plugin=qt}"
LINUXDEPLOY_FINAL_OPTIONS="${LINUXDEPLOY_FINAL_OPTIONS:-${LINUXDEPLOY_FIRST_OPTIONS} --output=appimage}"

SCRIPT_NAME="$(basename "${0}")"

function msg() {
  echo -e "\033[1m[${SCRIPT_NAME}] ${1}\033[0m"
}

while getopts 'd:t:punc:b:i:a:l:f:' OPTION; do
  case "${OPTION}" in
  d)
    msg "Note: Overriding build directory: '${BUILD_DIR}' -> '${OPTARG}'"
    BUILD_DIR="${OPTARG}"
    ;;
  t)
    msg "Note: Overriding build type: '${BUILD_TYPE}' -> '${OPTARG}'"
    BUILD_TYPE="${OPTARG}"
    ;;
  p)
    msg "Note: Enabling 'Pro' features."
    PRO_FEATURES='ON'
    ;;
  u)
    msg "Note: Enabling the update checker feature."
    UPDATE_CHECKER='ON'
    ;;
  n)
    msg "Note: Not adding git revision to the app's version."
    GIT_REVISION='OFF'
    ;;
  c)
    msg "Note: Overriding CMake configure options: '${CMAKE_CONFIG_OPTIONS}' -> '${OPTARG}'"
    CMAKE_CONFIG_OPTIONS="${OPTARG}"
    ;;
  b)
    msg "Note: Overriding CMake build options: '${CMAKE_BUILD_OPTIONS}' -> '${OPTARG}'"
    CMAKE_BUILD_OPTIONS="${OPTARG}"
    ;;
  i)
    msg "Note: Overriding CMake install options: '${CMAKE_INSTALL_OPTIONS}' -> '${OPTARG}'"
    CMAKE_INSTALL_OPTIONS="${OPTARG}"
    ;;
  a)
    msg "Note: Overriding AppDir directory: '${LINUXDEPLOY_APPDIR}' -> '${OPTARG}'"
    LINUXDEPLOY_APPDIR="${OPTARG}"
    ;;
  l)
    msg "Note: Overriding linuxdeploy's first run options: '${LINUXDEPLOY_FIRST_OPTIONS}' -> '${OPTARG}'"
    LINUXDEPLOY_FIRST_OPTIONS="${OPTARG}"
    ;;
  f)
    msg "Note: Overriding linuxdeploy's final run options: '${LINUXDEPLOY_FINAL_OPTIONS}' -> '${OPTARG}'"
    LINUXDEPLOY_FINAL_OPTIONS="${OPTARG}"
    ;;
  esac
done

set -x

msg 'Running CMake (configure)...'
cmake ${CMAKE_CONFIG_OPTIONS}

msg 'Running CMake (build)...'
cmake ${CMAKE_BUILD_OPTIONS}

msg 'Running CMake (install)...'
DESTDIR="${BUILD_DIR}/${LINUXDEPLOY_APPDIR}" cmake ${CMAKE_INSTALL_OPTIONS}

msg 'Running linuxdeploy (first run)...'
(cd "${BUILD_DIR}" && linuxdeploy ${LINUXDEPLOY_FIRST_OPTIONS})

msg 'Removing unused libraries in AppDir...'
# We only use the SQLite Qt driver, so it's fine to delete others.
rm -fv "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/plugins/sqldrivers/libqsqlmimer.so" # Qt 6.6+
rm -fv "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/plugins/sqldrivers/libqsqlmysql.so" # Qt 6+
rm -fv "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/plugins/sqldrivers/libqsqlodbc.so"  # Qt 5+
rm -fv "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/plugins/sqldrivers/libqsqlpsql.so"  # Qt 5+
# The bearer plugin has caused problems for us in the past. Plus, it was removed altogether in Qt 6.
rm -frv "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/plugins/bearer"

# Avoid running appstreamcli on Ubuntu 20.04 because it exits with non-zero code on just validation warnings.
if ! grep -q 'VERSION_ID="20.04"' /etc/os-release; then
  msg 'Validating AppStream metadata...'
  appstreamcli validate --verbose "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/share/metainfo/${APP_ID}.metainfo.xml"
fi

msg 'Validating desktop file...'
desktop-file-validate "${BUILD_DIR}/${LINUXDEPLOY_APPDIR}/usr/share/applications/${APP_ID}.desktop"

msg 'Determining app version...'
app_version=$(grep -oPm1 '\bAPP_VERSION +\K[^)]+' CMakeLists.txt)
if [ -z "${app_version}" ]; then
  msg 'Fatal: Failed to extract app version from CMakeLists.txt.'
  exit 1
fi
shopt -s nocasematch
if [[ "${GIT_REVISION}" == 'ON' ]]; then
  app_version="${app_version}+g$(git rev-parse --short HEAD)"
fi
shopt -u nocasematch
artifact_name="Notes_${app_version}-Qt${QT_VERSION}-$(arch)"
appimage_name="${artifact_name}.AppImage"

msg 'Running linuxdeploy (final run)...'
export LINUXDEPLOY_OUTPUT_VERSION="${app_version}"
export OUTPUT="${appimage_name}"
(cd "${BUILD_DIR}" && linuxdeploy ${LINUXDEPLOY_FINAL_OPTIONS})

# Needed for GitHub Actions.
if [ -f /GITHUB_OUTPUT ]; then
  msg "Note: File '/GITHUB_OUTPUT' exists, assuming we're running on GitHub Actions."
  echo "artifact_name=${artifact_name}" >>'/GITHUB_OUTPUT'
  echo "appimage_path=${BUILD_DIR}/${appimage_name}" >>'/GITHUB_OUTPUT'
fi
