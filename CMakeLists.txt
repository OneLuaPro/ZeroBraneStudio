# MIT License
#
# ZeroBraneStudio   Copyright (c) 2011-2024 Paul Kulchenko (paul@kulchenko.com)
# CMake Integration Copyright (c) 2024 Kritzel Kratzel
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in 
# the Software without restriction, including without limitation the rights to 
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all 
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# ------------------------------------------------------------------------------
# CMake for ZeroBraneStudio
# ------------------------------------------------------------------------------
# Setup with Visual Studio 17 2022 generator for x64
# ------------------------------------------------------------------------------
# Visual Studio is a mutlti-configuration generator
# https://stackoverflow.com/questions/24460486/
#
# Basic instructions for out-of-source build
# ------------------------------------------
# mkdir build && cd build
# cmake .. -G "Visual Studio 17 2022" -A x64
# cmake --build . --config Release
# cmake --install . --config Release
# 
# Available architectures (-A ...) are: Win32, x64, ARM, ARM64
#
# Note: As ZeroBraneStudio does not comprise any files to be compiled and linked (just
# files to be installed properly) the mention of architecture or config is
# somewhat arbitrary.

# ------------------------------------------------------------------------------
# General definitions
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

# ------------------------------------------------------------------------------
# find liblua installation and version info
if(NOT LUA_HINTS)
  if(WIN32)
    set(LUA_HINTS "c:/Apps")
  endif()
endif()
find_package(liblua REQUIRED CONFIG HINTS ${LUA_HINTS})
if(liblua_FOUND)
  message(STATUS "liblua version        : ${liblua_VERSION}")
  message(STATUS "liblua install prefix : ${LIBLUA_INSTALLDIR}")
  message(STATUS "liblua include dir    : ${LIBLUA_INCLUDEDIR}")
  message(STATUS "liblua lib dir        : ${LIBLUA_LIBDIR}")
else()
  message(FATAL_ERROR "Unable to find liblua version ${liblua_VERSION}.")
endif()
# Note: liblua_VERSION is set by find_package() directly. LIBLUA_INSTALLDIR,
# LIBLUA_INCLUDEDIR and LIBLUA_LIBDIR are set by libluaConfigVersion.cmake in
# <prefix>/share/cmake/liblua.

# ------------------------------------------------------------------------------
# Installation prefix directory - automatically set from find_package()
# Needs to be defined before project definition statement - for whatever reason
if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${LIBLUA_INSTALLDIR})
endif()

# ------------------------------------------------------------------------------
# Project defintion
project(ZeroBraneStudio)

# ------------------------------------------------------------------------------
# Other settings
set(CMAKE_VERBOSE_MAKEFILE OFF)
# ZeroBraneStudio has nothing to build, only some files to install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

# ------------------------------------------------------------------------------
# Setup GNU-alike installatin directories
include (GNUInstallDirs)
set(INSTALL_APPDIR
  "opt/${CMAKE_PROJECT_NAME}" CACHE PATH "Installation directory for app")
# set(INSTALL_BINDIR
#   ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
# set(INSTALL_DOCDIR
#   ${CMAKE_INSTALL_DOCDIR} CACHE PATH "Installation directory for documentation")
# set(INSTALL_DATAROOTDIR
#   ${CMAKE_INSTALL_DATAROOTDIR} CACHE PATH "Installation directory for data")
# # Lua-specific installation dirs
# set(INSTALL_TOP_LDIR
#   ${INSTALL_DATAROOTDIR}/lua/${liblua_VERSION_MAJOR}.${liblua_VERSION_MINOR})

# ------------------------------------------------------------------------------
# Install ZeroBraneStudio files and docs
install(FILES
  CHANGELOG.md
  README.md
  CONTRIBUTING.md
  ZeroBraneStudio.bat
  LICENSE
  DESTINATION ${INSTALL_APPDIR}
)
install(DIRECTORY
  cfg
  packages
  spec
  src
  DESTINATION ${INSTALL_APPDIR}
)
install(FILES
  interpreters/luabase.lua
  interpreters/luadeb.lua  
  DESTINATION ${INSTALL_APPDIR}/interpreters
)
install(FILES
  lualibs/lua_lexer_loose.lua
  lualibs/lua_parser_loose.lua
  DESTINATION ${INSTALL_APPDIR}/lualibs
)
install(DIRECTORY
  lualibs/copas
  lualibs/coxpcall
  lualibs/mobdebug
  DESTINATION ${INSTALL_APPDIR}/lualibs
)
install(DIRECTORY
  zbstudio/res
  DESTINATION ${INSTALL_APPDIR}/zbstudio
)
install(FILES
  zbstudio/app.lua
  zbstudio/config.lua
  DESTINATION ${INSTALL_APPDIR}/zbstudio
)
#install(DIRECTORY lualibs/coxpcall DESTINATION ${INSTALL_APPDIR}/lualibs)
#install(DIRECTORY lualibs/momdebug DESTINATION ${INSTALL_APPDIR}/lualibs)

# if(WIN32)
#   # *.bat-launcher for Windows
#   install(FILES bin/luacheck.bat DESTINATION ${INSTALL_BINDIR})
# endif()
# install(DIRECTORY src/ DESTINATION ${INSTALL_TOP_LDIR}
#   FILES_MATCHING PATTERN "*.lua"
# )
# install(DIRECTORY doc-html/ DESTINATION ${INSTALL_DOCDIR})


# "/" omits leading dir
