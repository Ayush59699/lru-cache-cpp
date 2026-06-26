#!/usr/bin/env bash
set -e

# Run directory setup or navigation if script is invoked outside project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR/.."

echo "Creating build directory..."
mkdir -p build
cd build

echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

echo "Building targets..."
cmake --build . --config Release

echo "===================================================="
echo "RUNNING FUNCTIONAL CORRECTNESS TEST"
echo "===================================================="
./test_lru

echo "===================================================="
echo "RUNNING CONCURRENCY STRESS TEST"
echo "===================================================="
./stress_test

echo "===================================================="
echo "RUNNING HIGH PERFORMANCE BENCHMARK SUITE"
echo "===================================================="
./benchmark

echo "===================================================="
echo "RUNNING BASELINE COMPARISON"
echo "===================================================="
./comparison

echo "===================================================="
echo "All steps executed successfully!"
echo "===================================================="
