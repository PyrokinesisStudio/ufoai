# - Locate SDL_mixer library
# This module defines:
#  SDL_MIXER_LIBRARIES, the name of the library to link against
#  SDL_MIXER_INCLUDE_DIRS, where to find the headers
#  SDL_MIXER_FOUND, if false, do not try to link against
#  SDL_MIXER_VERSION_STRING - human-readable string containing the version of SDL_mixer
#
# For backward compatiblity the following variables are also set:
#  SDLMIXER_LIBRARY (same value as SDL_MIXER_LIBRARIES)
#  SDLMIXER_INCLUDE_DIR (same value as SDL_MIXER_INCLUDE_DIRS)
#  SDLMIXER_FOUND (same value as SDL_MIXER_FOUND)
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake
# module, but with modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if(NOT SDL_MIXER_INCLUDE_DIR AND SDLMIXER_INCLUDE_DIR)
  set(SDL_MIXER_INCLUDE_DIR ${SDLMIXER_INCLUDE_DIR} CACHE PATH "directory cache
entry initialized from old variable name")
endif()
find_path(SDL_MIXER_INCLUDE_DIR SDL_mixer.h
  HINTS
    ENV SDLMIXERDIR
    ENV SDLDIR
  PATH_SUFFIXES include/SDL include/SDL12 include/SDL11 include include/SDL2
)

if(NOT SDL_MIXER_LIBRARY AND SDLMIXER_LIBRARY)
  set(SDL_MIXER_LIBRARY ${SDLMIXER_LIBRARY} CACHE FILEPATH "file cache entry
initialized from old variable name")
endif()
find_library(SDL_MIXER_LIBRARY
  NAMES SDL_mixer SDL2_mixer
  HINTS
    ENV SDLMIXERDIR
    ENV SDLDIR
  PATH_SUFFIXES lib
)

if(SDL_MIXER_INCLUDE_DIR AND EXISTS "${SDL_MIXER_INCLUDE_DIR}/SDL_mixer.h")
  file(STRINGS "${SDL_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_MAJOR "${SDL_MIXER_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_MINOR "${SDL_MIXER_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_MIXER_VERSION_PATCH "${SDL_MIXER_VERSION_PATCH_LINE}")
  set(SDL_MIXER_VERSION_STRING ${SDL_MIXER_VERSION_MAJOR}.${SDL_MIXER_VERSION_MINOR}.${SDL_MIXER_VERSION_PATCH})
  unset(SDL_MIXER_VERSION_MAJOR_LINE)
  unset(SDL_MIXER_VERSION_MINOR_LINE)
  unset(SDL_MIXER_VERSION_PATCH_LINE)
  unset(SDL_MIXER_VERSION_MAJOR)
  unset(SDL_MIXER_VERSION_MINOR)
  unset(SDL_MIXER_VERSION_PATCH)
endif()

set(SDL_MIXER_LIBRARIES ${SDL_MIXER_LIBRARY})
set(SDL_MIXER_INCLUDE_DIRS ${SDL_MIXER_INCLUDE_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL_mixer
                                  REQUIRED_VARS SDL_MIXER_LIBRARIES SDL_MIXER_INCLUDE_DIRS
                                  VERSION_VAR SDL_MIXER_VERSION_STRING)

# for backward compatiblity
set(SDLMIXER_LIBRARY ${SDL_MIXER_LIBRARIES})
set(SDLMIXER_INCLUDE_DIR ${SDL_MIXER_INCLUDE_DIRS})
set(SDLMIXER_FOUND ${SDL_MIXER_FOUND})

mark_as_advanced(SDL_MIXER_LIBRARY SDL_MIXER_INCLUDE_DIR)
