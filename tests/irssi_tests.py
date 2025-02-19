# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    irssi_tests.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/18 22:09:17 by akuburas          #+#    #+#              #
#    Updated: 2025/02/19 14:09:06 by akuburas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os
import random
import subprocess
import time

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

# tmux simulation parameters
SESSION_NAME = "irssi_sim"
NUM_CLIENTS = 5

def create_tmux_session(session_name):
	subprocess.run(["tmux", "new-session", "-d", "-s", session_name])
	print(f"[INFO] Created tmux session '{session_name}'.")

def create_tmux_window(session, window_name, command):
	subprocess.run(["tmux", "new-window", "-t", session, "-n", window_name, command])
	print(f"[INFO] Created tmux window '{window_name}' with command: {command}")

def send_tmux_command(target, command_str):
	subprocess.run(["tmux", "send-keys", "-t", target, command_str, "C-m"])

def start_irssi_client(nickname, server_port, server_password):
	# Build the irssi command using provided port and password.
	irssi_command = f"irssi -c 127.0.0.1 -p {server_port} -w {server_password} -n {nickname}"
	create_tmux_window(SESSION_NAME, nickname, irssi_command)
	# Give the client time to start
	time.sleep(1)
	return nickname

def start_simulation(server_port, server_password):
	"""
	Launches an interactive simulation.
	
	All clients will connect to the server at 127.0.0.1 using the given
	server_port and server_password. A random channel name is generated,
	and NUM_CLIENTS irssi clients (each in their own tmux window) join that channel.
	Then, in parallel, each client starts sending random approved messages.
	
	The simulation runs until the user presses Enter.
	"""
	# Generate a random channel name
	channel_name = f"#{random.choice(['general', 'random', 'chat', 'test'])}{random.randint(1, 100)}"
	
	# Create a new tmux session for the simulation.
	create_tmux_session(SESSION_NAME)
	
	# Launch clients with unique random nicknames.
	nicknames = random.sample(APPROVED_NICKNAMES, NUM_CLIENTS)
	clients = []
	for nick in nicknames:
		start_irssi_client(nick, server_port, server_password)
		clients.append(nick)
	
	print(f"[INFO] Simulation started with {NUM_CLIENTS} clients on channel {channel_name}.")
	print(f"[INFO] Server running on port {server_port} with password '{server_password}'.")
	print("\n[INFO] All clients have started. They should join the channel and begin messaging.")
	print("[INFO] Press Enter here to stop the simulation...\n")
	
	# Have each client join the channel.
	for client in clients:
		send_tmux_command(f"{SESSION_NAME}:{client}", f"/join {channel_name}")
		time.sleep(0.5)
	
	# For each client, start a background process that repeatedly sends messages.
	simulation_processes = []
	for client in clients:
		python_cmd = (
			"import random, time, subprocess;"
			"messages = " + str(APPROVED_MESSAGES) + ";"
			"target = '" + f"{SESSION_NAME}:{client}" + "';"
			"while True:"
			"    msg = random.choice(messages);"
			"    subprocess.run(['tmux', 'send-keys', '-t', target, msg, 'C-m']);"
			"    time.sleep(random.uniform(0.5, 3))"
		)
		proc = subprocess.Popen(["python3", "-c", python_cmd])
		simulation_processes.append(proc)
	
	# Wait for user input to stop the simulation.
	input()
	
	# Terminate the background messaging processes.
	for proc in simulation_processes:
		proc.terminate()
	
	# Kill the tmux session (closing all simulation windows).
	subprocess.run(["tmux", "kill-session", "-t", SESSION_NAME])
	print("[INFO] Simulation stopped and tmux session killed.")