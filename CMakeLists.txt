cmake_minimum_required(VERSION 3.29.6)
project(SysInfoViewer VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Find libcurl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Find ALSA
find_package(ALSA REQUIRED)

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

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_INCLUDE_DIRS})

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT applications)

# Install additional files
install(FILES favicon.ico
    DESTINATION share/${PROJECT_NAME}
    COMPONENT resources)

# CPack configuration
set(CPACK_GENERATOR "TGZ")
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

include(CPack)