# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    script.py                                          :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/10 13:38:42 by akuburas          #+#    #+#              #
#    Updated: 2025/02/11 03:18:28 by akuburas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import subprocess
import sys

def open_irssi(nickname, ip_address, port, password):
    command = (
        f'gnome-terminal -- bash -c "irssi -c {ip_address} -p {port} -w {password} -n {nickname}; exec bash"'
    )
    return subprocess.Popen(command, shell=True)

def mode1(ip_address, port, password):
    base_nickname = input("Enter base nickname: ")
    count = int(input("Enter number of clients: "))
    processes = [open_irssi(f"{base_nickname}{i+1}", ip_address, port, password) for i in range(count)]
    return processes

def mode2(ip_address, port, password):
    nicknames = input("Enter nicknames separated by spaces: ").split()
    processes = [open_irssi(nickname, ip_address, port, password) for nickname in nicknames]
    return processes

def main():
    ip_address = input("Enter IRC server IP address: ")
    port = input("Enter port: ")
    password = input("Enter password: ")
    
    mode = input("Select mode (1 or 2): ")
    processes = []
    
    if mode == "1":
        processes = mode1(ip_address, port, password)
    elif mode == "2":
        processes = mode2(ip_address, port, password)
    else:
        print("Invalid mode selected.")
        sys.exit(1)
    
    input("Press Enter when you want to close all clients...")
    for process in processes:
        process.terminate()
    print("All clients closed.")

if __name__ == "__main__":
    main()

