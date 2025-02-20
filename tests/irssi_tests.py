# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    irssi_tests.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/18 22:09:17 by akuburas          #+#    #+#              #
#    Updated: 2025/02/20 18:26:23 by akuburas         ###   ########.fr        #
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


def start_simulation():
	