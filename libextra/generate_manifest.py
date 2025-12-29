#!/usr/bin/env python3
"""
Generate libextra_manifest.json for the libextra directory.
Lists all library files that should be loaded at runtime.
"""

import os
import json

def generate_manifest():
    """Generate manifest of all files in libextra directory."""

    # Get current directory (should be libextra)
    libextra_dir = os.path.dirname(os.path.abspath(__file__))

    # Get all library files (.k, .kc, .kb, .kbm, .kp, .km, .tp, .html, .xml, .lst, .txt, .ppm, .exp)
    files = []
    for filename in sorted(os.listdir(libextra_dir)):
        if filename.endswith(('.k', '.kc', '.kb', '.kbm', '.kp', '.km', '.tp', '.html', '.xml', '.lst', '.txt', '.ppm', '.exp')):
            files.append(filename)

    # Write manifest
    manifest_path = os.path.join(libextra_dir, 'libextra_manifest.json')
    with open(manifest_path, 'w') as f:
        json.dump(files, f, indent=2)

    print(f"Generated {manifest_path} with {len(files)} entries")
    return len(files)

if __name__ == "__main__":
    generate_manifest()
