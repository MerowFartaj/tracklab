#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

build_dir="build"
app_path="${build_dir}/tracklab_app_artefacts/Debug/Tracklab.app"
generator="Xcode"
cmake_config_args=()

if ! xcodebuild -version >/dev/null 2>&1; then
    echo "Full Xcode is not available. Falling back to Unix Makefiles for this local build."
    generator="Unix Makefiles"
    cmake_config_args=(-DCMAKE_BUILD_TYPE=Debug)
fi

if [[ -f "${build_dir}/CMakeCache.txt" ]] \
    && ! grep -q "CMAKE_GENERATOR:INTERNAL=${generator}" "${build_dir}/CMakeCache.txt"; then
    echo "Removing ${build_dir} because it was configured with a different generator."
    rm -rf "${build_dir}"
fi

cmake -B "${build_dir}" -G "${generator}" "${cmake_config_args[@]}"
cmake --build build --config Debug

echo "Build succeeded: ${app_path}"
