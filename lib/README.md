# KeyKit Library Files

This directory contains KeyKit library files that are loaded at runtime in the WebAssembly build.

## Files

- **`*.k`** - KeyKit source files (main library code)
- **`*.kc`** - KeyKit compiled files
- **`*.kb`** - KeyKit binary files
- **`*.kbm`** - KeyKit binary map files
- **`*.exp`** - Export/expression files
- **`*.txt`** - Text/documentation files
- **`*.ppm`** - PPM image files

## Manifest

The **`lib_manifest.json`** file contains a JSON array of all library files that should be loaded in the WebAssembly build.

### Regenerating the Manifest

When you add or remove files in this directory, regenerate the manifest:

```bash
bash generate_manifest.sh
```

This will update `lib_manifest.json` with the current list of files.

## How Library Files Are Used

In the WebAssembly build:

1. The manifest is fetched by the browser
2. Each file listed is downloaded and written to a virtual filesystem at `/keykit/lib/`
3. C code can access files using standard I/O: `fopen("/keykit/lib/filename.k", "r")`

See `../RUNTIME_LIBRARY_LOADING.md` for complete details.

## File Count

Currently contains **374 library files**.
