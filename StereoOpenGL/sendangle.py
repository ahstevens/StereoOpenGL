import sys
import socket

if len(sys.argv) < 3:
    print("Usage:", sys.argv[0], "<host-ip>", "<host-port>", "[servo-angle]")
    sys.exit()

TCP_IP = sys.argv[1]
TCP_PORT = int(sys.argv[2])
BUF_SIZE = 16

print("Host IP:", TCP_IP)
print("Host port:", TCP_PORT)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))

if len(sys.argv) == 4:
    sock.send(sys.argv[3].encode())
    print("Message:", sys.argv[3])

    data = sock.recv(BUF_SIZE)
    print("Response:", data.decode())
else:
    angle = input("Angle(, Time): ")
    while float(angle.split(',')[0]) >= -90:
        sock.send(angle.encode())
        data = sock.recv(BUF_SIZE)
        print("Response:", data.decode())
        angle = input("Angle(, Time): ")

sock.close()