# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    start.py                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/18 22:09:25 by akuburas          #+#    #+#              #
#    Updated: 2025/03/10 08:08:06 by akuburas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os
import random
import subprocess
import sys
import time

from test_cases import run_basic_tests
from irssi_tests import start_simulation

TESTS_DIR = os.path.dirname(os.path.realpath(__file__))
BASE_DIR = os.path.abspath(os.path.join(TESTS_DIR, ".."))
SERVER_EXECUTABLE = os.path.join(BASE_DIR, "ircserv")

# List of valid server configurations (port, password)
VALID_SERVER_CONFIGS = [
    ("6667", "pass123"),
    ("6697", "secret"),
    ("7000", "letmein"),
    ("7070", "password"),
]

def compile_server():
    print("Compiling server...")
    result = subprocess.run(["make", "re"], capture_output=True, text=True, cwd=BASE_DIR)
    if result.returncode != 0:
        print("❌ Compilation failed!")
        print(result.stderr)
        sys.exit(1)
    print("✅ Compilation successful!")

def start_server(server_executable, server_port, server_password):
    tmux_session = "ircserv_sim"
    subprocess.run(["tmux", "new-session", "-t", tmux_session], capture_output=True)
    
    server_cmd = f"{server_executable} {server_port} {server_password}"
    print(f"[INFO] Starting server in tmux session '{tmux_session}' with command: {server_cmd}")
    subprocess.Popen(["tmux", "new-session", "-d", "-s", tmux_session, server_cmd], cwd=BASE_DIR)
    # Allow time for the server to initialize.
    time.sleep(2)
    return tmux_session

def cleanup():
    print("Cleaning up...")
    subprocess.run(["make", "fclean"], capture_output=True, text=True, cwd=BASE_DIR)
    print("Cleanup complete.")

def main():
    os.chdir(BASE_DIR)
    compile_server()
    
    # Choose a random valid configuration for the server.
    server_port, server_password = random.choice(VALID_SERVER_CONFIGS)
    
    # Start the server.
    
    print("\nStarting basic tests...\n")
    run_basic_tests(BASE_DIR, SERVER_EXECUTABLE)
    server_session = start_server(SERVER_EXECUTABLE, server_port, server_password)
    
    print("\nStarting interactive simulation test...\n")
    start_simulation(server_port, server_password)
    
    # After simulation, terminate the server if it is still running.
    print(f"[INFO] Terminating server session '{server_session}'...")
    subprocess.run(["tmux", "kill-session", "-t", server_session], capture_output=True)
    
    cleanup()

if __name__ == "__main__":
    main()