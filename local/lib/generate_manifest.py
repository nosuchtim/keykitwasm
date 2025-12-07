#!/usr/bin/env python3
"""
Generate a manifest of all lib files for runtime loading.
"""

import os
import json
import glob

# Change to the script's directory
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# File extensions to include
extensions = ['*.k', '*.K']

# Collect all matching files (exclude backup files)
files = []
for pattern in extensions:
    for f in glob.glob(pattern):
        if not f.endswith('~'):
            files.append(f)

# Remove duplicates and sort
files = sorted(set(files))

# Write JSON manifest
with open('lib_manifest.json', 'w') as f:
    json.dump(files, f, indent=2)

print(f"Generated lib_manifest.json with {len(files)} lib files")
