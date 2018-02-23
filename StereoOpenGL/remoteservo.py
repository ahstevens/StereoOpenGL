# Servo Control
import socket
import time
import wiringpi

TCP_PORT = 5005

minRange = 68
maxRange = 245
servoAngularRange = 160 # degrees

def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

stepSize = float(maxRange - minRange) / servoAngularRange

# use 'GPIO naming'
wiringpi.wiringPiSetupGpio()

# set #18 to be a PWM output
wiringpi.pinMode(18, wiringpi.GPIO.PWM_OUTPUT)

# set the PWM mode to milliseconds stype
wiringpi.pwmSetMode(wiringpi.GPIO.PWM_MODE_MS)

# divide down clock
wiringpi.pwmSetClock(192)
wiringpi.pwmSetRange(2000)

delay_period = 0.01

for pulse in range(minRange, maxRange, 1):
    wiringpi.pwmWrite(18, pulse)
    time.sleep(delay_period)
for pulse in range(maxRange, minRange, -1):
    wiringpi.pwmWrite(18, pulse)
    time.sleep(delay_period)

ip_addr = get_ip_address()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('', TCP_PORT))
sock.listen(1)

print("Starting server on host", ip_addr, "port", TCP_PORT) 

while True:
    print("Listening on ", ip_addr, ":", TCP_PORT, sep="")
    (clientsocket, address) = sock.accept()

    try:
        print("Connection from", address[0], "port", address[1])

        while True:
            data = clientsocket.recv(16)
            if data:
                print("Received data:", data.decode())
                clientsocket.sendall(data)

                if int(data) >= 0 & int(data) <= servoAngularRange:
                    wiringpi.pwmWrite(18, int(minRange + stepSize * int(data)))
            else:
                print("Connection lost from", address[0], "port", address[1])
                break
    finally:
        clientsocket.close()