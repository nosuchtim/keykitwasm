#!/usr/bin/env python3
"""
Generate a manifest of all library files for runtime loading.
This is a cross-platform Python version that works on Windows, Linux, and macOS.
"""

import os
import json
import glob

# Change to the script's directory
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# File extensions to include
extensions = ['*.k', '*.kc', '*.kb', '*.kbm', '*.exp', '*.txt', '*.ppm', '*.lst']

# Collect all matching files
files = []
for pattern in extensions:
    files.extend(glob.glob(pattern))

# Sort the files
files.sort()

# Write JSON manifest
with open('lib_manifest.json', 'w') as f:
    json.dump(files, f, indent=2)

print(f"Generated lib_manifest.json with {len(files)} library files")
