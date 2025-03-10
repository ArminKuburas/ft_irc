# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    irssi_tests.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/18 22:09:17 by akuburas          #+#    #+#              #
#    Updated: 2025/03/10 10:56:54 by akuburas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os
import random
import subprocess
import time
import threading

# Approved lists
APPROVED_NICKNAMES = ["Alice", "Bob", "Charlie", "Dave", "Eve",
                    "Frank", "Grace", "Heidi", "Ivan", "Judy"]
APPROVED_MESSAGES = [
    "Hello everyone!",
    "How's it going?",
    "Anyone here?",
    "What's up?",
    "I love this channel!",
    "Random message here.",
    "Testing, testing...",
    "This is fun!",
    "Just passing by.",
    "Goodbye for now."
]

APPROVED_CHANNEL_NAMES = ["#general", "#random", "#test", "#chat", "#fun"]

# tmux simulation parameters
SESSION_NAME = "irssi_sim"
NUM_CLIENTS = 5
running_threads = True  # Global flag to stop threads


def create_tmux_session(session_name):
    subprocess.run(["tmux", "new-session", "-d", "-s", session_name])
    print(f"[INFO] Created tmux session '{session_name}'.")


def create_tmux_window(session, window_name, command):
    subprocess.run(["tmux", "new-window", "-t", session, "-n", window_name, command])
    print(f"[INFO] Created tmux window '{window_name}' with command: {command}")


def send_tmux_command(target, command_str):
    subprocess.run(["tmux", "send-keys", "-t", target, command_str, "C-m"])


def start_irssi_client(nickname, server_port, server_password):
    """Starts an irssi client in a tmux window."""
    irssi_command = f"irssi -c 127.0.0.1 -p {server_port} -w {server_password} -n {nickname}"
    create_tmux_window(SESSION_NAME, nickname, irssi_command)
    time.sleep(1)  # Give the client time to start
    return nickname


def message_sender_thread(client):
    """Thread function to send messages periodically."""
    global running_threads
    target = f"{SESSION_NAME}:{client}"
    
    while running_threads:
        msg = random.choice(APPROVED_MESSAGES)
        send_tmux_command(target, msg)
        time.sleep(random.uniform(0.5, 3))  # Random delay between messages


def start_simulation(server_port, server_password):
    """
    Launches an interactive simulation using threads.
    
    - Clients connect to the server at 127.0.0.1 using the given
      server_port and server_password.
    - A random channel name is generated.
    - NUM_CLIENTS irssi clients (each in their own tmux window) join that channel.
    - Each client starts sending random messages in a separate thread.
    
    The simulation runs until the user presses Enter.
    """
    global running_threads
    
    # Generate a random channel name
    channel_name = f"#{random.choice(['general', 'random', 'chat', 'test'])}{random.randint(1, 100)}"
    
    # Create a new tmux session for the simulation
    create_tmux_session(SESSION_NAME)
    
    # Launch clients with unique random nicknames
    nicknames = random.sample(APPROVED_NICKNAMES, NUM_CLIENTS)
    clients = []
    
    for nick in nicknames:
        start_irssi_client(nick, server_port, server_password)
        clients.append(nick)
    
    print(f"[INFO] Simulation started with {NUM_CLIENTS} clients on channel {channel_name}.")
    print(f"[INFO] Server running on port {server_port} with password '{server_password}'.")
    print("\n[INFO] All clients have started. They should join the channel and begin messaging.")
    print("[INFO] Press Enter here to stop the simulation...\n")
    
    # Have each client join the channel
    for client in clients:
        send_tmux_command(f"{SESSION_NAME}:{client}", f"/join {channel_name}")
        time.sleep(0.5)
    
    # Start messaging threads
    threads = []
    
    for client in clients:
        thread = threading.Thread(target=message_sender_thread, args=(client,))
        thread.start()
        threads.append(thread)
    
    # Wait for user input to stop simulation
    print("To check the simulation, you can attach to the tmux session:")
    print(f"  tmux attach-session -t {SESSION_NAME}")
    print("To explore the windows, use Ctrl+b followed by a number (0-9).")
    print("To detach from the session, use Ctrl+b followed by d.")
    print("\n")
    print("To attach to the server session, use:")
    print(f"  tmux attach-session -t ircserv_sim\n")
    input("Press Enter to stop the simulation...\n")
    
    # Stop threads
    running_threads = False
    for thread in threads:
        thread.join()
    
    # Kill the tmux session (closing all simulation windows)
    subprocess.run(["tmux", "kill-session", "-t", SESSION_NAME])
    print("[INFO] Simulation stopped and tmux session killed.")


# Example usage:
# start_simulation(6667, "mypassword")
