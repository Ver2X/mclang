#!/usr/bin/env python3
import subprocess
import sys
import os
import glob
import argparse

# Create an argument parser
parser = argparse.ArgumentParser()
parser.add_argument("--asan", action="store_true", help="Enable ASan")

# Parse the command-line arguments
args = parser.parse_args()

# Set ENABLE_ASAN based on the command-line argument
ENABLE_ASAN = args.asan

# Check if CMake is installed
if subprocess.call(["cmake", "--version"], stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL) != 0:
    print("CMake is not installed")
    sys.exit(1)

# Check if Ninja is installed
if subprocess.call(["ninja", "--version"], stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL) != 0:
    print("Ninja is not installed")
    sys.exit(1)
    
# Remove core files if they exist
core_files = glob.glob("core*")
for file in core_files:
    if os.path.exists(file):
        os.remove(file)

# Remove old build directory
if not os.path.exists("build"):
    os.makedirs("build")
# else:
    # os.system("rm -rf build")
    # os.makedirs("build")

# Enter the build directory
os.chdir("build")

# Build the project using CMake and set compilation options based on ENABLE_ASAN
cmake_cmd = ["cmake", f"-DENABLE_ASAN={str(ENABLE_ASAN).lower()}", "-G", "Ninja", ".."]
subprocess.run(cmake_cmd, check=True)

# Compile the project
subprocess.run(["cmake", "--build", "."], check=True)

# Go back to the parent directory
os.chdir("..")

# Clean previous build results
# subprocess.run(["make", "clean"])

# Run the compiled executable
# subprocess.run(["./build/mclang", "std/loop.c"], stdout=subprocess.DEVNULL)
# subprocess.run(["./build/mclang", "std/if.c"], stdout=subprocess.DEVNULL)