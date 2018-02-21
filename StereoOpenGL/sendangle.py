import sys
import socket

TCP_IP = "192.168.0.116"
TCP_PORT = 5005
BUF_SIZE = 16

print("Host IP:", TCP_IP)
print("Host port:", TCP_PORT)
print("Message:", str(sys.argv[1]))

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))
sock.send(str(sys.argv[1]).encode())
data = sock.recv(BUF_SIZE)
sock.close

print("Received data:", data.decode())