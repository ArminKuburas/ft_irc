# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    script.py                                          :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/10 13:38:42 by akuburas          #+#    #+#              #
#    Updated: 2025/02/10 13:50:02 by akuburas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import subprocess
import sys

def open_irssi(nickname):
    command = (
        f'gnome-terminal -- bash -c "irssi -c 127.0.0.1 -p 6697 -w 123 -n {nickname}; exec bash"'
    )
    subprocess.Popen(command, shell=True)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <nickname1> <nickname2> ...")
        sys.exit(1)
    
    nicknames = sys.argv[1:]
    for nickname in nicknames:
        open_irssi(nickname)