cmake_minimum_required(VERSION 4.2.0)
project(sysinfoviewer VERSION 0.2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(wxWidgets REQUIRED COMPONENTS core base adv)
find_package(CURL REQUIRED)
find_package(ALSA REQUIRED)

include(${wxWidgets_USE_FILE})

# Source files
set(SOURCES
    sysinfo.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    ${CURL_LIBRARIES}
    ${ALSA_LIBRARIES}
    drm
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
)

# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install icon if exists
if(EXISTS "${CMAKE_SOURCE_DIR}/favicon.ico")
    install(FILES favicon.ico
        DESTINATION share/pixmaps
        RENAME sysinfoviewer.ico
    )
endif()

# Install desktop file if exists
if(EXISTS "${CMAKE_SOURCE_DIR}/sysinfoviewer.desktop")
    install(FILES sysinfoviewer.desktop
        DESTINATION share/applications
    )
endif()

# Install man page if exists
if(EXISTS "${CMAKE_SOURCE_DIR}/docs/sysinfoviewer.1")
    install(FILES docs/sysinfoviewer.1
        DESTINATION share/man/man1
    )
endif()

# Install license
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    install(FILES LICENSE
        DESTINATION share/licenses/${PROJECT_NAME}
    )
endif()

# Install README
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    install(FILES README.md
        DESTINATION share/doc/${PROJECT_NAME}
    )
endif()

#############################################
# CPack Configuration for Package Generation
#############################################

include(CPack)

# Basic CPack settings
set(CPACK_PACKAGE_NAME "sysinfoviewer")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "3")
set(CPACK_PACKAGE_RELEASE 1)

set(CPACK_PACKAGE_VENDOR "Wanjare Samuel (Magpiny)")
set(CPACK_PACKAGE_CONTACT "magpinyb@proton.me")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Comprehensive system information viewer with real-time monitoring")
set(CPACK_PACKAGE_DESCRIPTION "A comprehensive graphical system information viewer built with wxWidgets for Linux systems. Features real-time CPU, memory, and disk monitoring with visual charts, battery information, process management, network statistics, and detailed hardware information.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Magpiny/sysinfoviewer")

# License
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}-${CMAKE_SYSTEM_PROCESSOR}")

#############################################
# DEB Package Configuration (Debian/Ubuntu)
#############################################
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Wanjare Samuel <magpinyb@proton.me>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk3.2-1, libcurl4, libasound2")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "lm-sensors, smartmontools")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

#############################################
# RPM Package Configuration (Fedora/RHEL)
#############################################
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_RPM_PACKAGE_REQUIRES "wxGTK3 >= 3.2, libcurl, alsa-lib")
set(CPACK_RPM_PACKAGE_SUGGESTS "lm_sensors, smartmontools")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_RELEASE ${CPACK_PACKAGE_RELEASE})
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

#############################################
# Archive Generators (TGZ, TBZ2, TXZ, ZIP)
#############################################
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

#############################################
# Generator Selection
#############################################
# Uncomment the generators you want to use:
set(CPACK_GENERATOR "TGZ;DEB;RPM")
# Available: DEB, RPM, TGZ, TBZ2, TXZ, ZIP, STGZ, NSIS, etc.

# Architecture-specific settings
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "i686")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "aarch64")
endif()

# Post-install scripts (optional)
# set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/scripts/postinst")
# set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/scripts/post_install.sh")

# Components (for fine-grained control)
set(CPACK_COMPONENTS_ALL applications documentation)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "SysInfoViewer Application")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "Main application binary and resources")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "User documentation and man pages")
