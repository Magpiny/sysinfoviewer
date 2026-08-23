#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
echo "Wanjman build script"
echo "App version 0.3.3"

BUILD_DIR="AppDir"
INSTALL_DIR="AppDir"
EXECUTABLE_NAME="sysinfoviewer"

# Default build type
BUILD_TYPE="Debug"

# --- Functions ---
log_info() {
  echo "INFO: $1"
}

log_error() {
  echo "ERROR: $1" >&2
  exit 1
}

show_usage() {
  echo "Usage: $0 [r|release|d|dev|debug]"
  echo ""
  echo "Options:"
  echo "  r, release    Build in release mode (optimized for production)"
  echo "  d, dev, debug Build in development mode (default, with debug symbols)"
  echo ""
  echo "Examples:"
  echo "  $0 r          # Build release version"
  echo "  $0 dev        # Build development version"
  echo "  $0            # Build development version (default)"
}

# --- Parse Arguments ---
if [ $# -gt 0 ]; then
  case "$1" in
    r|release)
      BUILD_TYPE="Release"
      log_info "Building in RELEASE mode"
      ;;
    d|dev|debug)
      BUILD_TYPE="Debug"
      log_info "Building in DEVELOPMENT mode"
      ;;
    -h|--help)
      show_usage
      exit 0
      ;;
    *)
      log_error "Unknown argument: $1. Use -h for help."
      ;;
  esac
else
  log_info "Building in DEVELOPMENT mode (default)"
fi

# --- Compiler Flags Configuration ---
if [ "$BUILD_TYPE" = "Release" ]; then
  # Release mode: Maximum optimization, small binary, production-ready
  CMAKE_FLAGS=(
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native -mtune=native -DNDEBUG -ffunction-sections -fdata-sections"
    -DCMAKE_C_FLAGS_RELEASE="-O3 -march=native -mtune=native -DNDEBUG -ffunction-sections -fdata-sections"
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -Wl,--strip-all"
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON  # Link-time optimization (LTO)
  )
else
  # Development mode: Debug symbols, no optimization, better for debugging
  CMAKE_FLAGS=(
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -Wall -Wextra"
    -DCMAKE_C_FLAGS_DEBUG="-g -O0 -Wall -Wextra"
  )
fi

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
log_info "Configuring CMake project with ${BUILD_TYPE} settings..."
cmake -B "$BUILD_DIR" \
  -DCMAKE_INSTALL_PREFIX="$(pwd)/$INSTALL_DIR" \
  "${CMAKE_FLAGS[@]}" || log_error "CMake configuration failed."

# 4. Build the project
log_info "Building the project..."
cmake --build "$BUILD_DIR" || log_error "Project build failed."

# 5. Install the project
log_info "Installing the project to ${INSTALL_DIR}..."
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR" || log_error "Project installation failed."

# 6. Display binary size
if [ -f "${INSTALL_DIR}/bin/${EXECUTABLE_NAME}" ]; then
  BINARY_SIZE=$(du -h "${INSTALL_DIR}/bin/${EXECUTABLE_NAME}" | cut -f1)
  log_info "Binary size: ${BINARY_SIZE}"
fi

log_info "Build and installation completed successfully!"
log_info "Build type: ${BUILD_TYPE}"
log_info "Executable can be found at: $(pwd)/${INSTALL_DIR}/bin/${EXECUTABLE_NAME}"
log_info "You can run it using: ./${INSTALL_DIR}/bin/${EXECUTABLE_NAME}"
log_info "$(date +%d/%m/%Y_%H:%M:%S)"
