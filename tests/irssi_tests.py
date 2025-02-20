# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    irssi_tests.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/18 22:09:17 by akuburas          #+#    #+#              #
#    Updated: 2025/02/20 18:31:50 by akuburas         ###   ########.fr        #
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

APPROVED_CHANNEL_NAMES = ["#general", "#random", "#test", "#chat", "#fun"]

# tmux simulation parameters
SESSION_NAME = "irssi_sim"
NUM_CLIENTS = 5
NUM_CHANNELS = 1
SERVER_ADDRESS = "localhost"


def start_simulation(server_port, server_password):
	# Create a new tmux session
	subprocess.run(["tmux", "new-session", "-d", "-s", SESSION_NAME])

	# List to keep track of joined channels
	joined_channels = []

	for i in range(NUM_CLIENTS):
		nickname = random.choice(APPROVED_NICKNAMES)
		channel = random.choice(APPROVED_CHANNEL_NAMES)
		
		if channel not in joined_channels:
			joined_channels.append(channel)
		
		# Create a new tmux window for each client
		subprocess.run(["tmux", "new-window", "-t", SESSION_NAME, "-n", f"client_{i}"])
		
		# Send commands to the tmux window to start irssi and connect to the server
		subprocess.run(["tmux", "send-keys", "-t", f"{SESSION_NAME}:client_{i}", 
						f"irssi -c {SERVER_ADDRESS} -p {server_port} -w {server_password} -n {nickname}", "C-m"])
		
		# Join the channel
		subprocess.run(["tmux", "send-keys", "-t", f"{SESSION_NAME}:client_{i}", 
						f"/join {channel}", "C-m"])

	try:
		while True:
			for i in range(NUM_CLIENTS):
				message = random.choice(APPROVED_MESSAGES)
				channel = random.choice(joined_channels)
				
				# Send a random message to a random channel
				subprocess.run(["tmux", "send-keys", "-t", f"{SESSION_NAME}:client_{i}", 
								f"/msg {channel} {message}", "C-m"])
				
				# Wait for a random interval between 1 and 5 seconds
				time.sleep(random.randint(1, 5))
	except KeyboardInterrupt:
		# Stop the simulation when the user presses enter
		print("Stopping simulation...")
		subprocess.run(["tmux", "kill-session", "-t", SESSION_NAME])