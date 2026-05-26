import os
import subprocess
import glob
import sys
import re

# List of source files to compile (in src/ directory)
src_files = [
    "src/main.c", "src/util.c", "src/misc.c", "src/phrase.c", "src/sym.c", "src/keyto.c", "src/yacc.c",
    "src/code.c", "src/code2.c", "src/grid.c", "src/view.c", "src/menu.c", "src/task.c", "src/fifo.c",
    "src/mfin.c", "src/real.c", "src/kwind.c", "src/fsm.c", "src/bltin.c", "src/meth.c", "src/regex.c",
    "src/mdep_wasm.c"
]

def find_emcc():
    """Find emcc executable, checking emsdk directories first, then PATH."""
    candidates = []

    emsdk = os.environ.get("EMSDK")
    if emsdk:
        candidates.append(os.path.join(emsdk, "upstream", "emscripten", "emcc.bat"))
        candidates.append(os.path.join(emsdk, "upstream", "emscripten", "emcc"))

    repo_parent = os.path.abspath(os.path.join(os.getcwd(), ".."))
    candidates.append(os.path.join(repo_parent, "emsdk", "upstream", "emscripten", "emcc.bat"))
    candidates.append(os.path.join(repo_parent, "emsdk", "upstream", "emscripten", "emcc"))

    for name in ("emcc.bat", "emcc"):
        for path_dir in os.environ.get("PATH", "").split(os.pathsep):
            if path_dir:
                candidates.append(os.path.join(path_dir, name))

    for candidate in candidates:
        if os.path.isfile(candidate):
            return candidate

    print("Unable to find emcc. Install/activate emsdk or add emcc to PATH.")
    return None

def read_version():
    """Read the KeyKit version from the VERSION file."""
    try:
        with open("VERSION", "r") as f:
            version = f.read().strip()
    except OSError as e:
        print(f"Unable to read VERSION: {e}")
        sys.exit(1)

    if not version:
        print("VERSION is empty")
        sys.exit(1)

    return version

def replace_in_file(path, pattern, replacement):
    """Replace one required version marker in a source file."""
    with open(path, "r") as f:
        original = f.read()

    updated, count = re.subn(pattern, replacement, original, count=1)
    if count != 1:
        print(f"Unable to update version marker in {path}")
        sys.exit(1)

    if updated != original:
        with open(path, "w") as f:
            f.write(updated)

def sync_version_files():
    """Mirror keykit's VERSION-driven source updates for the WASM build."""
    version = read_version()
    replace_in_file(
        "src/key.h",
        r'#define KEYVERSION "[^"]*"',
        f'#define KEYVERSION "{version}"'
    )
    replace_in_file(
        "src/main.c",
        r'KeyKit [^ ]+ - Copyright',
        f'KeyKit {version} - Copyright'
    )
    replace_in_file(
        "src/key.rc",
        r'"KeyKit \([^)]+\)"',
        f'"KeyKit ({version})"'
    )

def generate_keylib_files():
    """Generate keylib.k files for libcore, libtools, and libextra directories"""
    print("Generating keylib.k files...")

    directories = ['libcore', 'libtools', 'libextra']
    for directory in directories:
        if os.path.isdir(directory):
            print(f"  Generating {directory}/keylib.k...")
            result = subprocess.run(
                [sys.executable, 'generate_keylib.py', directory],
                capture_output=True,
                text=True
            )
            if result.returncode != 0:
                print(f"  Warning: Failed to generate {directory}/keylib.k")
                print(result.stderr)
            else:
                # Print summary line
                for line in result.stdout.split('\n'):
                    if 'Generated' in line:
                        print(f"  {line}")
        else:
            print(f"  Skipping {directory} (not found)")

    print()

def compile_wasm():
    print("Compiling to WASM...")
    
    # Find emcc
    emcc = find_emcc()
    if emcc is None:
        sys.exit(1)
    
    # Output file (in current directory)
    output = "keykit.html"

    # Compiler flags
    flags = [
        emcc,  # Use the found emcc path
        "-Isrc",
        "-o", output,
        "--js-library", "keykit_library.js",  # Include JavaScript library
        "--shell-file", "keykit_shell.html",  # Custom HTML shell
        "-s", "ALLOW_MEMORY_GROWTH=1",
        "-s", "ASYNCIFY=1",  # Important for blocking calls
        "-s", "ASYNCIFY_IMPORTS=['js_sync_from_real']",  # Functions that can suspend
        "-s", "SUPPORT_LONGJMP=emscripten",  # Enable setjmp/longjmp support (JS-based, compatible with ASYNCIFY)
        "-s", "FORCE_FILESYSTEM=1",  # Enable virtual filesystem
        "-s", "EXPORTED_FUNCTIONS=['_main','_mdep_on_midi_message','_mdep_on_mouse_move','_mdep_on_mouse_button','_mdep_on_key_event','_mdep_on_window_resize','_mdep_on_nats_message','_mdep_on_websocket_event']",
        "-s", "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','getValue','setValue','UTF8ToString','FS','IDBFS','HEAPU8']",
        "-lidbfs.js",  # Include IDBFS library
        "-s", "ASSERTIONS=1",  # Enable runtime assertions
        "-D__EMSCRIPTEN__",
        "-Wno-implicit-function-declaration",
        "-Wno-int-conversion",
        "-Wno-incompatible-pointer-types",
        "-Wno-return-type",
        "-lm",  # Link math library
        "-g",   # Debug symbols
        "-O0"   # No optimization for easier debugging (change to -O2 for production)
    ]

    # Optional: Preload files if lib directory exists
    # Uncomment these lines if you have a lib directory to package
    # if os.path.exists("lib"):
    #     flags.extend(["--preload-file", "lib@/keykit/lib"])
    # if os.path.exists("music"):
    #     flags.extend(["--preload-file", "music@/keykit/music"])
    
    cmd = flags + src_files
    
    print(" ".join(cmd))
    print("\nCompiling...")
    try:
        result = subprocess.run(cmd, check=False, capture_output=True, text=True)
        
        # Save both stdout and stderr
        with open("build_log.txt", "w") as f:
            f.write("=== STDOUT ===\n")
            f.write(result.stdout)
            f.write("\n\n=== STDERR ===\n")
            f.write(result.stderr)
        
        print(result.stdout)
        print(result.stderr)
        
        if result.returncode == 0:
            print(f"\nSuccessfully built {output}")
        else:
            print(f"\nCompilation failed with return code {result.returncode}")
            print("Check build_log.txt for details")
            sys.exit(1)
    except Exception as e:
        print(f"Compilation failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    # Keep version-bearing source files in sync with VERSION before compiling.
    sync_version_files()

    # Generate keylib.k files before compiling
    generate_keylib_files()

    # Compile to WebAssembly
    compile_wasm()

