#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
BUILD_DIR="build"
INSTALL_DIR="bin" # The user requested building the app into the bin directory
EXECUTABLE_NAME="SysInfoViewer" # From CMakeLists.txt

# --- Functions ---
log_info() {
  echo "INFO: $1"
}

log_error() {
  echo "ERROR: $1" >&2
  exit 1
}

# --- Main Script ---

log_info "Starting robust build script for ${EXECUTABLE_NAME}..."

# 1. Clean previous build artifacts
if [ -d "$BUILD_DIR" ]; then
  log_info "Removing existing build directory: ${BUILD_DIR}"
  rm -rf "$BUILD_DIR" || log_error "Failed to remove build directory."
fi

if [ -d "$INSTALL_DIR" ]; then
  log_info "Removing existing install directory: ${INSTALL_DIR}"
  rm -rf "$INSTALL_DIR" || log_error "Failed to remove install directory."
fi

# 2. Create necessary directories
log_info "Creating build directory: ${BUILD_DIR}"
mkdir -p "$BUILD_DIR" || log_error "Failed to create build directory."

log_info "Creating install directory: ${INSTALL_DIR}"
mkdir -p "$INSTALL_DIR" || log_error "Failed to create install directory."

# 3. Configure CMake
log_info "Configuring CMake project..."
cmake -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX="$(pwd)/$INSTALL_DIR" || log_error "CMake configuration failed."

# 4. Build the project
log_info "Building the project..."
cmake --build "$BUILD_DIR" || log_error "Project build failed."

# 5. Install the project
log_info "Installing the project to ${INSTALL_DIR}..."
cmake --install "$BUILD_DIR" || log_error "Project installation failed."

log_info "Build and installation completed successfully!"
log_info "Executable can be found at: $(pwd)/${INSTALL_DIR}/bin/${EXECUTABLE_NAME}"
log_info "You can run it using: ./${INSTALL_DIR}/bin/${EXECUTABLE_NAME}"
