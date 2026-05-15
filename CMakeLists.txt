cmake_minimum_required(VERSION 3.5)
project(crossfire-client C)
set(VERSION 1.75.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Build options: feature toggles and GTK version selection.
option(LUA "Lua scripting" OFF)
option(METASERVER2 "Metaserver2 support (requires curl)" ON)
option(SOUND "Sound support (requires sdl_mixer)" ON)
# Set GTK_VERSION to "2" or "3" to select which GTK client to build.
# When set to "both", both gtk-v2 and gtk-v3 targets are built if found.
set(GTK_VERSION "both" CACHE STRING "GTK version to build (2, 3, or both)")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
    # On Linux/other Unix-like, get gio to work by defining _BSD_SOURCE.
    add_definitions(-D_DEFAULT_SOURCE)
elseif(APPLE)
    # macOS: use Homebrew or MacPorts GTK installation.
    add_definitions(-D_DEFAULT_SOURCE)
elseif(MINGW OR WIN32)
    add_definitions(-DWIN32)
    set(CMAKE_INSTALL_PREFIX ".")
endif()

include(GNUInstallDirs)
set(CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_DATAROOTDIR}/crossfire-client)

# -- Required dependencies common to both GTK versions --
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources)
find_package(PkgConfig REQUIRED)
find_package(PNG REQUIRED)
find_package(Perl REQUIRED)
find_package(Vala REQUIRED)

# X11 is only available on Unix/Linux; skip on Windows and macOS.
if(UNIX AND NOT APPLE)
    find_package(X11)
    if(X11_FOUND)
        set(HAVE_X11 TRUE)
    endif()
else()
    set(HAVE_X11 FALSE)
    set(X11_LIBRARIES "")
endif()

# -- GTK2 detection (for gtk-v2 subdir) --
if(GTK_VERSION STREQUAL "2" OR GTK_VERSION STREQUAL "both")
    pkg_check_modules(GTK2 gtk+-2.0 gio-2.0)
    if(GTK2_FOUND)
        set(BUILD_GTK2 TRUE)
    else()
        message(WARNING "gtk+-2.0 not found; skipping gtk-v2 build")
        set(BUILD_GTK2 FALSE)
    endif()
else()
    set(BUILD_GTK2 FALSE)
endif()

# -- GTK3 detection (for gtk-v3 subdir) --
if(GTK_VERSION STREQUAL "3" OR GTK_VERSION STREQUAL "both")
    pkg_check_modules(GTK3 gtk+-3.0 gio-2.0)
    if(GTK3_FOUND)
        set(BUILD_GTK3 TRUE)
    else()
        message(WARNING "gtk+-3.0 not found; skipping gtk-v3 build")
        set(BUILD_GTK3 FALSE)
    endif()
else()
    set(BUILD_GTK3 FALSE)
endif()

# Validate that at least one GTK version is available.
if(NOT BUILD_GTK2 AND NOT BUILD_GTK3)
    message(FATAL_ERROR "No usable GTK version found. "
        "Install gtk+-2.0 and/or gtk+-3.0 development packages.")
endif()

# Use GTK2 or GTK3 variables for shared code (common/) -- prefer GTK3 if both.
if(BUILD_GTK3)
    set(GTK_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS})
    set(GTK_CFLAGS_OTHER ${GTK3_CFLAGS_OTHER})
elseif(BUILD_GTK2)
    set(GTK_INCLUDE_DIRS ${GTK2_INCLUDE_DIRS})
    set(GTK_CFLAGS_OTHER ${GTK2_CFLAGS_OTHER})
endif()

add_definitions(
    ${GTK_CFLAGS_OTHER}
    ${PNG_DEFINITIONS}
)

include(${VALA_USE_FILE})

# Silence deprecated-declaration warnings; GTK2 uses some deprecated APIs.
add_compile_options(-Wno-deprecated-declarations)

# -- Optional dependencies --
if(LUA)
    find_package(Lua51 REQUIRED)
endif()

if(METASERVER2)
    pkg_check_modules(CURL libcurl REQUIRED)
    set(HAVE_CURL_CURL_H ${CURL_FOUND})
endif()

if(SOUND)
    pkg_check_modules(SDLMIXER SDL2_mixer REQUIRED)
    if(EXISTS "${PROJECT_SOURCE_DIR}/sounds")
        install(DIRECTORY sounds DESTINATION ${CMAKE_INSTALL_DATADIR})
    else()
        message(WARNING "SOUND is enabled, but the sound configuration file "
            "was not found in ${PROJECT_SOURCE_DIR}/sounds. "
            "You will need to install sounds yourself.")
    endif()
endif()

include(CheckIncludeFiles)
set(CMAKE_REQUIRED_INCLUDES ${GTK_INCLUDE_DIRS})
check_include_files(gio/gnetworking.h HAVE_GIO_GNETWORKING_H)
check_include_files(sys/capsicum.h HAVE_CAPSICUM)

configure_file("config.h.in" "config.h")

add_subdirectory(common)

if(BUILD_GTK2)
    add_subdirectory(gtk-v2)
endif()

if(BUILD_GTK3)
    add_subdirectory(gtk-v3)
endif()

enable_testing()

# Build an installation package.
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VERSION "${VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/;.git/;.svn/")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "crossfire-client-${CPACK_PACKAGE_VERSION}")
include(CPack)
