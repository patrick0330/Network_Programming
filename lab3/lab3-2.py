#!/usr/bin/python3
# -*- coding: utf-8 -*-
import socket
import time 
import sys
HOST = 'inp111.zoolab.org'
PORT = 10003

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)
s.connect((HOST, PORT))


# print(start_time)
total_bytes = 0
packet_num = 0
header_offset = 54
mega_bytes = float(sys.argv[1]) * 1000000
print(mega_bytes)

data = '0'
start_time = time.time() 
while True:
    delta = time.time() - start_time
    
    data_to_send = round(mega_bytes * delta) - total_bytes - header_offset
    if data_to_send <= 0:
        # print("dont send")
        continue
    s.send((data * data_to_send).encode())
    time.sleep(0.0002)
    total_bytes += data_to_send + header_offset
    packet_num += 1
    print(packet_num,total_bytes)
