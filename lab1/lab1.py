import dpkt
import os

f = open('test.pcap','rb')
pcap = dpkt.pcap.Reader(f)
f2 = open('test.pcap','rb')
pcap2 = dpkt.pcap.Reader(f2)

record = {}
target = 0
for ts, buf in pcap:
    eth = dpkt.ethernet.Ethernet(buf)
    ip = eth.data
    tcp = ip.data
    if(ip.ttl not in record):
        record[ip.ttl] = 1
    else:
        record[ip.ttl] += 1
for i in record:
    if(record[i] == 1):
        target = i
        # print(target)

for ts, buf in pcap2:
    eth = dpkt.ethernet.Ethernet(buf)
    ip = eth.data
    tcp = ip.data
    # print(type(ip.ttl))
    if(ip.ttl == target):
        payload = bytes(tcp)
        # print(payload)
        result = str(payload[32:])
        new_result = result[1:]
        print(new_result)
        os.system(f'echo {new_result} | base64 -d')

    

f.close()
f2.close()