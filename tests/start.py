import subprocess
import sys
import os
from test_cases import run_tests

TESTS_DIR = os.path.dirname(os.path.realpath(__file__))
BASE_DIR = os.path.abspath(os.path.join(TESTS_DIR, ".."))

SERVER_EXECUTABLE = os.path.join(BASE_DIR, "ircserv")

def compile_server():
    print("Compiling server...")
    result = subprocess.run(["make", "re"], capture_output=True, text=True, cwd=BASE_DIR)

    if result.returncode != 0:
        print("❌ Compilation failed!")
        print(result.stderr)
        sys.exit(1)
    print("✅ Compilation successful!")

def cleanup():
    print("Cleaning up...")
    subprocess.run(["make", "fclean"], capture_output=True, text=True, cwd=BASE_DIR)
    print("Cleanup complete.")

def main():
    os.chdir(BASE_DIR)
    compile_server()

    print("\nStarting tests...\n")
    run_tests(BASE_DIR, SERVER_EXECUTABLE)

    cleanup()

if __name__ == "__main__":
    main()
