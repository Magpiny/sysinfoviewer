cmake_minimum_required(VERSION 3.2.0)
project(sysinfoviewer VERSION 0.2.3 LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix")

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS $<CONFIG:Debug>)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Find libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Find ALSA
find_package(ALSA REQUIRED)

# Find libdrm for display info
find_package(PkgConfig REQUIRED)
pkg_check_modules(DRM REQUIRED libdrm)

# Add source files
set(SOURCES
    sysinfo.cpp
    # Add other source files here
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link wxWidgets libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES} ${CURL_LIBRARIES})

# Link ALSA library
target_link_libraries(${PROJECT_NAME} PRIVATE ALSA::ALSA)

# Link libdrm
if(DRM_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${DRM_LIBRARIES})
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIRS}
)

# Modify your install commands to be more AppImage-friendly
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT applications)

# Install icon (convert .ico to .png for better Linux compatibility)
install(FILES favicon.ico
    DESTINATION share/icons/hicolor/256x256/apps
    RENAME sysinfoviewer.png  # AppImage prefers PNG
    COMPONENT resources)



# CPack configuration
set(CPACK_GENERATOR "TGZ;DEB;RPM")
set(CPACK_PACKAGE_NAME "SysInfoViewer")
set(CPACK_PACKAGE_VENDOR "Wanjare Samuel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "System Information Viewer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# Desktop file
set(DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop")
file(WRITE ${DESKTOP_FILE}
"[Desktop Entry]
Name=System Info Viewer
Comment=Monitor System Resources
Exec=sysinfoviewer
Icon=sysinfoviewer
Terminal=false
Type=Application
Categories=Utility;
")

install(FILES ${DESKTOP_FILE}
    DESTINATION share/applications
    COMPONENT desktop)

include(CPack)
