#!/bin/bash
set -e

# Default CORES to 8, but allow override from environment
CORES=${CORES:-8}

# Colors for nicer output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No color

# --- IMPORTANT FOR UCRT ---
# This script assumes you are running it from an MSYS2 UCRT64 shell.

if command -v ninja >/dev/null 2>&1; then
    GEN="Ninja"
    BUILD_TOOL="ninja"
    BUILD_PARALLEL_FLAG="-j"
elif command -v mingw32-make >/dev/null 2>&1; then
    GEN="MinGW Makefiles"
    BUILD_TOOL="mingw32-make"
    BUILD_PARALLEL_FLAG="-j"
else
    echo -e "${YELLOW}Warning: Neither 'ninja' nor 'mingw32-make' found. Falling back to 'Unix Makefiles' and 'make'.${NC}"
    GEN="Unix Makefiles"
    BUILD_TOOL="make"
    BUILD_PARALLEL_FLAG="-j"
fi

# Function to get the build directory for a given build type
get_build_dir() {
    local BUILD_TYPE=$1
    echo "build_$BUILD_TYPE"
}

clean() {
    echo -e "${YELLOW}Cleaning all build directories...${NC}"
    rm -rf build_Debug build_Release
    echo -e "${GREEN}All build directories cleaned.${NC}"
}

build() {
    local BUILD_TYPE=$1
    local BUILD_DIR=$(get_build_dir "$BUILD_TYPE")
    local EXTRA_FLAGS=()

    echo -e "${GREEN}Configuring project with generator: $GEN (${BUILD_TYPE})${NC}"
    mkdir -p "$BUILD_DIR"

    if [[ "$BUILD_TYPE" == "Release" ]]; then
        EXTRA_FLAGS+=("-DCMAKE_CXX_FLAGS_RELEASE=-std=c++23 -O3 -march=native -DNDEBUG")
    fi

    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        "${EXTRA_FLAGS[@]}" \
        -S . -B "$BUILD_DIR" -G "$GEN"

    echo -e "${GREEN}Building project using $BUILD_TOOL with $CORES cores in $BUILD_DIR...${NC}"
    cmake --build "$BUILD_DIR" -- ${BUILD_PARALLEL_FLAG}"$CORES"
}

case "$1" in
    "" | build)
        build "Debug"
        ;;
    release)
        build "Release"
        ;;
    clean)
        clean
        ;;
    cleanbuild | "clean build" | rebuild)
        clean
        build "Debug"
        ;;
    cleanrelease | "clean release" | releaserebuild)
        clean
        build "Release"
        ;;
    all) # New option to build both Debug and Release
        build "Debug"
        build "Release"
        ;;
    *)
        echo -e "${RED}Usage: $0 [build | release | clean | cleanbuild | cleanrelease | rebuild | releaserebuild | all]${NC}"
        exit 1
        ;;
esac

echo -e "${GREEN}Build process finished.${NC}"