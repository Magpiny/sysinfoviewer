#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
BUILD_DIR="build_local"
APP_DIR="app"
EXECUTABLE_NAME="sysvnfoviewer"

# --- Functions ---
log_info() {
  echo "INFO: $1"
}

log_error() {
  echo "ERROR: $1" >&2
  exit 1
}

# --- Main Script ---

log_info "SystemInfoViewer: Is a system monitoring tool"
log_info "AUTHOR: Magpiny BO"
log_info "..........................................................."
log_info " "
log_info "Version: 0.2.3"
log_info "$(date +%d/%m/%Y_%H:%M:%S)"
log_info "Starting script to create local app in '${APP_DIR}'..."

# 1. Clean previous build and app directories
if [ -d "$BUILD_DIR" ]; then
  log_info "Removing existing build directory: ${BUILD_DIR}"
  rm -rf "$BUILD_DIR" || log_error "Failed to remove build directory."
fi

if [ -d "$APP_DIR" ]; then
  log_info "Removing existing app directory: ${APP_DIR}"
  rm -rf "$APP_DIR" || log_error "Failed to remove app directory."
fi

# 2. Create necessary directories
log_info "Creating build directory: ${BUILD_DIR}"
mkdir -p "$BUILD_DIR" || log_error "Failed to create build directory."

# The APP_DIR will be created by cmake --install

# 3. Configure CMake
log_info "Configuring CMake project for local installation..."
cmake -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX="$(pwd)/$APP_DIR" || log_error "CMake configuration failed."

# 4. Build the project
log_info "Building the project..."
cmake --build "$BUILD_DIR" || log_error "Project build failed."

# 5. Install the project into the app directory
log_info "Installing the project to ${APP_DIR}..."
cmake --install "$BUILD_DIR" || log_error "Project installation failed."

log_info "Local app creation completed successfully!"
log_info "You can run the app using: ./${APP_DIR}/bin/${EXECUTABLE_NAME}"
log_info "The desktop file is located at: ./${APP_DIR}/share/applications/${EXECUTABLE_NAME}.desktop"
log_info "The icon is located at: ./${APP_DIR}/share/${EXECUTABLE_NAME}/favicon.ico"
