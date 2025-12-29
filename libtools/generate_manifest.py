#!/usr/bin/env python3
"""
Generate libtools_manifest.json for the libtools directory.
Lists all .k, .html, .xml, and .lst files that should be loaded at runtime.
"""

import os
import json

def generate_manifest():
    """Generate manifest of all tool files in libtools directory."""

    # Get current directory (should be libtools)
    libtools_dir = os.path.dirname(os.path.abspath(__file__))

    # Get all files that should be loaded
    files = []
    for filename in sorted(os.listdir(libtools_dir)):
        # Include .k, .html, .xml, and .lst files
        if filename.endswith(('.k', '.html', '.xml', '.lst')):
            files.append(filename)

    # Write manifest
    manifest_path = os.path.join(libtools_dir, 'libtools_manifest.json')
    with open(manifest_path, 'w') as f:
        json.dump(files, f, indent=2)

    print(f"Generated {manifest_path} with {len(files)} entries")
    return len(files)

if __name__ == "__main__":
    generate_manifest()
