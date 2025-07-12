cmake_minimum_required(VERSION 3.30)
project(SysInfoViewer VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include ExternalProject module
include(ExternalProject)

# Function to download and install a library
function(download_and_install_library NAME URL)
    ExternalProject_Add(
        ${NAME}
        URL ${URL}        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/${NAME}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )
    set(${NAME}_DIR ${CMAKE_BINARY_DIR}/external/${NAME} PARENT_SCOPE)
endfunction()

# Check and download wxWidgets if not found
find_package(wxWidgets QUIET)
if(NOT wxWidgets_FOUND)
    download_and_install_library(wxWidgets "https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.1/wxWidgets-3.2.1.tar.bz2")
    set(wxWidgets_DIR ${wxWidgets_DIR})
endif()

# Check and download libcurl if not found
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    download_and_install_library(CURL "https://curl.se/download/curl-7.80.0.tar.gz")
    set(CURL_DIR ${CURL_DIR})
endif()

# Check and download ALSA if not found
find_package(ALSA QUIET)
if(NOT ALSA_FOUND)
    download_and_install_library(ALSA "https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.7.tar.bz2")
    set(ALSA_DIR ${ALSA_DIR})
endif()

# Find packages
find_package(wxWidgets REQUIRED COMPONENTS core base)
find_package(CURL REQUIRED)
find_package(ALSA REQUIRED)

# Include directories
include(${wxWidgets_USE_FILE})
include_directories(${CURL_INCLUDE_DIRS})

# Add source files
set(SOURCES
    sysinfo.cpp
    # Add other source files here
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} ${CURL_LIBRARIES} ALSA::ALSA)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT applications)

# Install additional files
install(FILES favicon.ico
    DESTINATION share/${PROJECT_NAME}
    COMPONENT resources)

# Desktop file
set(DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop")
file(WRITE ${DESKTOP_FILE}
"[Desktop Entry]
Name=System Info Viewer
Comment=View system information
Exec=${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}
Icon=${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/favicon.ico
Terminal=false
Type=Application
Categories=System;
")

install(FILES ${DESKTOP_FILE}
    DESTINATION share/applications
    COMPONENT desktop)

# Generate installation script
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/install.sh"
"#!/bin/bash
set -e

# Detect package manager
if command -v pacman &> /dev/null; then
    sudo pacman -S --needed cmake gcc make
elif command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y cmake g++ make
else
    echo 'Unsupported package manager. Please install CMake, GCC, and Make manually.'
    exit 1
fi

# Build and install
mkdir -p build
cd build
cmake ..
make
sudo make install
")

# Generate uninstallation script
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/uninstall.sh"
"#!/bin/bash
set -e

# Uninstall
sudo xargs rm < install_manifest.txt
")

# Make scripts executable
execute_process(COMMAND chmod +x "${CMAKE_CURRENT_BINARY_DIR}/install.sh")
execute_process(COMMAND chmod +x "${CMAKE_CURRENT_BINARY_DIR}/uninstall.sh")

# CPack configuration
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "SysInfoViewer")
set(CPACK_PACKAGE_VENDOR "Wanjare Samuel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System Information Viewer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# DEB-specific configuration
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Wanjare Samuel")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk3.0-gtk3-0v5, libcurl4, libasound2")

# Arch-specific configuration
if(EXISTS "/etc/arch-release")
    set(CPACK_GENERATOR "${CPACK_GENERATOR};TGZ")
    set(CPACK_ARCH_PACKAGE_DEPENDS "wxgtk3 curl alsa-lib")
endif()

include(CPack)