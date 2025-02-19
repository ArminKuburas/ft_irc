# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    irssi_tests.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/18 22:09:17 by akuburas          #+#    #+#              #
#    Updated: 2025/02/19 13:09:24 by akuburas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os
import random
import subprocess
import sys
import time

# Define the approved lists
APPROVED_NICKNAMES = ["Alice", "Bob", "Charlie", "Dave", "Eve", "Frank", "Grace", "Heidi", "Ivan", "Judy"]
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

# List of valid server configurations (port, password)
VALID_SERVER_CONFIGS = [
	("6667", "pass123"),
	("6697", "secret"),
	("7000", "letmein"),
	("7070", "password"),
]

# Choose one configuration at random
SERVER_PORT, SERVER_PASSWORD = random.choice(VALID_SERVER_CONFIGS)

# A random channel name
CHANNEL_NAME = f"#{random.choice(['general', 'random', 'chat', 'test'])}{random.randint(1, 100)}"

# tmux session name for simulation
SESSION_NAME = "irssi_sim"

# Number of simulated clients (for now, 5)
NUM_CLIENTS = 5

def create_tmux_session(session_name):
	subprocess.run(["tmux", "new-session", "-d", "-s", session_name])
	print(f"[INFO] Created tmux session '{session_name}'.")

def create_tmux_window(session, window_name, command):
	subprocess.run(["tmux", "new-window", "-t", session, "-n", window_name, command])
	print(f"[INFO] Created tmux window '{window_name}' with command: {command}")

def send_tmux_command(target, command_str):
	"""
	Sends a command string (simulate keystrokes) to a tmux target.
	`target` can be of the form session:window (or session:window.pane).
	"""
	subprocess.run(["tmux", "send-keys", "-t", target, command_str, "C-m"])

def start_irssi_client(nickname):
	"""
	Launches an irssi client in a new tmux window. The client connects to the server using
	the randomly selected SERVER_PORT and SERVER_PASSWORD.
	"""
	# Build the irssi command.
	# Adjust this command as needed for your actual irssi or simulated client.
	irssi_command = f"irssi -c 127.0.0.1 -p {SERVER_PORT} -w {SERVER_PASSWORD} -n {nickname}"
	create_tmux_window(SESSION_NAME, nickname, irssi_command)
	# Give the client a moment to start up before sending further commands.
	time.sleep(1)
	return nickname  # returning the window name (nickname) as the target identifier

def simulate_activity(target):

	# Join the channel. In irssi, the command is typically: /join #channel
	send_tmux_command(f"{SESSION_NAME}:{target}", f"/join {CHANNEL_NAME}")
	time.sleep(1)
	
	# Loop to send random messages.
	# We will run this loop until the simulation is stopped.
	while True:
		# Choose a random message
		msg = random.choice(APPROVED_MESSAGES)
		# Send the message
		send_tmux_command(f"{SESSION_NAME}:{target}", msg)
		# Sleep for a random interval between messages (0.5 to 3 seconds)
		time.sleep(random.uniform(0.5, 3))

def start_simulation():

	create_tmux_session(SESSION_NAME)
	
	# Launch clients with random nicknames (select from approved list without duplicates)
	nicknames = random.sample(APPROVED_NICKNAMES, NUM_CLIENTS)
	clients = []
	
	for nick in nicknames:
		start_irssi_client(nick)
		clients.append(nick)
	
	print(f"[INFO] Simulation started with {NUM_CLIENTS} clients on channel {CHANNEL_NAME}.")
	print(f"[INFO] Server running on port {SERVER_PORT} with password '{SERVER_PASSWORD}'.")
	print("\n[INFO] Press Enter here to stop the simulation...\n")
	
	# In parallel, we need to simulate message activity in each window.
	# For simplicity in this example, we'll start separate subprocesses that run a Python one-liner loop.
	# (Alternatively, you could use threading or asyncio.)
	simulation_processes = []
	for client in clients:
		# Build a shell command that repeatedly sends random messages to the tmux window.
		# We use a Python one-liner to do this.
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
	
	# Wait for user input to stop simulation.
	input()
	
	# Terminate the simulation processes (if they are still running)
	for proc in simulation_processes:
		proc.terminate()
	
	# Kill the tmux session to close all client windows.
	subprocess.run(["tmux", "kill-session", "-t", SESSION_NAME])
	print("[INFO] Simulation stopped and tmux session killed.")