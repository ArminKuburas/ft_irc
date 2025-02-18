import subprocess

def run_test(command, expected_output):
    print(f"Testing: {' '.join(command)}")
    
    result = subprocess.run(command, capture_output=True, text=True)
    
    if expected_output in result.stderr or expected_output in result.stdout:
        print("✅ Test passed!")
    else:
        print("❌ Test failed!")
        print(f"Expected: {expected_output}")
        print(f"Got: {result.stderr or result.stdout}")

def run_tests():
    # Case 1: Run without arguments
    run_test(["./ircserv"], "Usage: ./ircserv <port> <password>")

    # Case 2: Run with empty arguments
    run_test(["./ircserv", ""], "Usage: ./ircserv <port> <password>")
    run_test(["./ircserv", "", ""], "Usage: ./ircserv <port> <password>")

    # Case 3: Valid port, missing password
    run_test(["./ircserv", "6667"], "Usage: ./ircserv <port> <password>")

    # Case 4: Invalid port (e.g., 80 is a common web port)
    run_test(["./ircserv", "80", "password123"], "Error: Invalid port")

    # Case 5: Commonly used port (e.g., 22 for SSH)
    run_test(["./ircserv", "22", "password123"], "Error: Invalid port")

    print("\nAll tests completed.\n")
