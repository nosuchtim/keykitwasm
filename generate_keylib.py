#!/usr/bin/env python3
"""
Generate keylib.k files by scanning .k files for function and class definitions.
This is a Python replacement for the C keylib.exe utility.

Usage:
    python generate_keylib.py <directory>

Example:
    python generate_keylib.py libcore
    python generate_keylib.py libtools
"""

import os
import sys
import re

def scan_k_file(filepath, filename):
    """Scan a .k file for function and class definitions."""
    entries = []

    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                line = line.strip()

                # Look for function definitions: function name(...) or function name
                if line.startswith('function '):
                    match = re.match(r'^function\s+(\w+)', line)
                    if match:
                        func_name = match.group(1)
                        entries.append(f"#library {filename} {func_name}\n")

                # Look for class definitions: class name(...) or class name
                elif line.startswith('class '):
                    match = re.match(r'^class\s+(\w+)', line)
                    if match:
                        class_name = match.group(1)
                        entries.append(f"#library {filename} {class_name}\n")

    except Exception as e:
        print(f"Warning: Error reading {filepath}: {e}", file=sys.stderr)

    return entries

def generate_keylib(directory):
    """Generate keylib.k file for a directory."""

    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a directory", file=sys.stderr)
        return False

    # Get all .k files in the directory (except keylib.k itself)
    k_files = []
    for filename in sorted(os.listdir(directory)):
        if filename.endswith('.k') and filename.lower() != 'keylib.k':
            k_files.append(filename)

    if not k_files:
        print(f"No .k files found in {directory}")
        return False

    # Scan all files and collect library entries
    all_entries = []
    for filename in k_files:
        filepath = os.path.join(directory, filename)
        print(f"Scanning {filename}...")
        entries = scan_k_file(filepath, filename)
        all_entries.extend(entries)

    # Write keylib.k
    keylib_path = os.path.join(directory, 'keylib.k')
    with open(keylib_path, 'w', encoding='utf-8') as f:
        f.writelines(all_entries)

    print(f"\nGenerated {keylib_path} with {len(all_entries)} entries")
    return True

def main():
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)

    directory = sys.argv[1]
    success = generate_keylib(directory)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
