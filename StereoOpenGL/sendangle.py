import sys
import socket

if len(sys.argv) < 4:
    print("Usage:", sys.argv[0], "<host-ip>", "<host-port>", "<servo-angle>")
    sys.exit()

TCP_IP = sys.argv[1]
TCP_PORT = int(sys.argv[2])
MSG = str(sys.argv[3])
BUF_SIZE = 16

print("Host IP:", TCP_IP)
print("Host port:", TCP_PORT)
print("Message:", MSG)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))
sock.send(MSG.encode())
data = sock.recv(BUF_SIZE)
sock.close

print("Received data:", data.decode())