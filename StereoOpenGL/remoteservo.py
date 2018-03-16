# Servo Control
import sys
import socket
import time
import servo
import lcddriver

if len(sys.argv) < 2:
    print("Usage:", sys.argv[0], "<host-port>")
    sys.exit()

TCP_PORT = int(sys.argv[1])

def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

try:
    lcd = lcddriver.lcd()
except OSError:
    lcd = None
        
serv = servo.servo()

ip_addr = get_ip_address()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

while True:
    try:
        sock.bind((ip_addr, TCP_PORT))
        break
    except OSError as err:
        print("Error binding ", ip_addr, " port ", TCP_PORT, ": ", err.strerror, " (", err.errno, ")", sep="")
        sleep(5)
        

print("Starting server on host", ip_addr, "port", TCP_PORT)

if lcd:
    lcd.lcd_display_string("====CCOM==VisLab====", 1)
    lcd.lcd_display_string("  IP:" + str(ip_addr), 2)
    lcd.lcd_display_string("Port:" + str(TCP_PORT), 3)
    lcd.lcd_display_string("Awaiting Connection", 4)

clientsocket = None

try:
    sock.listen(1)
    
    while True:
        print("Listening on ", ip_addr, ":", TCP_PORT, sep="")

        try:
            (clientsocket, address) = sock.accept()
            print("Connection from", address[0], "port", address[1])
        
            if lcd:
                lcd.lcd_display_string(str(address[0]) + ":" + str(address[1]), 1)
                lcd.lcd_display_string(time.strftime("%m/%d/%Y %H:%M:%S"), 2)
                lcd.lcd_display_string("<Connected>", 3)

            while True:
                data = clientsocket.recv(4096)
                if data:
                    ts = time.localtime(time.time())
                    print("Received data:", data.decode())
                    if lcd:
                        lcd.lcd_display_string(time.strftime("%m/%d/%Y %H:%M:%S"), 2)
                        lcd.lcd_display_string("Angle: " + str(data.decode()), 4)
                
                    clientsocket.sendall(data)
                
                    angletime = data.decode().split(",")
                    if len(angletime) == 2:
                        serv.set(float(angletime[0]), float(angletime[1]))    
                    else:
                        serv.set(float(angletime[0]))
                
                else:
                    print("Connection lost from", address[0], "port", address[1])
                    if lcd:
                        lcd.lcd_display_string(time.strftime("%m/%d/%Y %H:%M:%S"), 2)
                        lcd.lcd_display_string("<Connection lost>", 3)
                    break
        except KeyboardInterrupt:
            print("\nShutting down...")
            break
finally:
    if sock:
        sock.close()
    if clientsocket:
        clientsocket.close()
            