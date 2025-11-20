import os
import subprocess
import glob
import sys

# List of source files to compile
src_files = [
    "main.c", "util.c", "misc.c", "phrase.c", "sym.c", "keyto.c", "yacc.c",
    "code.c", "code2.c", "grid.c", "view.c", "menu.c", "task.c", "fifo.c",
    "mfin.c", "real.c", "kwind.c", "fsm.c", "bltin.c", "meth.c", "regex.c",
    "mdep_wasm.c"
]

def find_emcc():
    """Find emcc executable, checking emsdk directory first, then PATH"""
    
    return "C:\\Users\\tjt\\GitHub\\emsdk\\upstream\\emscripten\\emcc.bat"

def compile_wasm():
    print("Compiling to WASM...")
    
    # Find emcc
    emcc = find_emcc()
    if emcc is None:
        sys.exit(1)
    
    # Output file
    output = "keykit.html"
    
    # Compiler flags
    flags = [
        emcc,  # Use the found emcc path
        "-I.",
        "-o", output,
        "-s", "ALLOW_MEMORY_GROWTH=1",
        "-s", "ASYNCIFY=1",  # Important for blocking calls
        "-s", "EXPORTED_FUNCTIONS=['_main']",
        "-s", "EXPORTED_RUNTIME_METHODS=['ccall','cwrap']",
        "-D__EMSCRIPTEN__",
        "-Wno-implicit-function-declaration",
        "-Wno-int-conversion",
        "-Wno-incompatible-pointer-types",
        "-Wno-return-type",
        "-O2"  # Optimization level
    ]
    
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
            print(f"\n✓ Successfully built {output}")
        else:
            print(f"\n✗ Compilation failed with return code {result.returncode}")
            print("Check build_log.txt for details")
            sys.exit(1)
    except Exception as e:
        print(f"Compilation failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    compile_wasm()

