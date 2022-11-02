#!/usr/bin/python3
# -*- coding: utf-8 -*-
import socket
import time 
HOST = 'inp111.zoolab.org'
PORT = 10002

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
f = open('res.txt','w')
f2 = open('res2.txt','w')

indata = s.recv(256)
print(indata.decode(),end = "")
out = "GO\n"
s.send(out.encode())
total = 0
data = s.recv(2048) 
txt = ""

while True:
    txt += data.decode()   
    # print(len(data))     
    
    if('==== END DATA ====' in txt):
        # f.write(data.decode())
        # print(data.decode(),end = "")
        total += len(txt.split('==== BEGIN DATA ====\n')[1].split('\n==== END DATA ====')[0])
        break
    # f.write(data.decode())    
    data = s.recv(2048)
    print(data.decode())

ans = "{size}\n".format(size = total)
print("Ans is " + ans)
s.send(ans.encode())
data = s.recv(2048)

print(data.decode(),end = "")

