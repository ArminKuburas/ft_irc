# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    script.py                                          :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/10 13:38:42 by akuburas          #+#    #+#              #
#    Updated: 2025/02/10 14:09:20 by pmarkaid         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import subprocess
import sys

def open_irssi(nickname, ip_address):
    command = (
        f'gnome-terminal -- bash -c "irssi -c {ip_address} -p 6697 -w 123 -n {nickname}; exec bash"'
    )
    subprocess.Popen(command, shell=True)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 script.py <nickname1> <nickname2> ...")
        sys.exit(1)
    ip_address = sys.argv[1]
    nicknames = sys.argv[2:]
    for nickname in nicknames:
        open_irssi(nickname, ip_address)
