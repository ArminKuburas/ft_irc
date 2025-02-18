import subprocess
import sys
from test_cases import run_tests

def compile_server():
    print("Compiling server...")
    result = subprocess.run(["make", "re"], capture_output=True, text=True)

    if result.returncode != 0:
        print("❌ Compilation failed!")
        print(result.stderr)
        sys.exit(1)
    print("✅ Compilation successful!")

def cleanup():
    print("Cleaning up...")
    subprocess.run(["make", "fclean"], capture_output=True, text=True)
    print("Cleanup complete.")

def main():
    compile_server()

    print("\nStarting tests...\n")
    run_tests()

    cleanup()

if __name__ == "__main__":
    main()
