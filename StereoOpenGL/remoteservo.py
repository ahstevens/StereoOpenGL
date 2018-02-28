# Servo Control
import socket
import time
import wiringpi
import lcddriver

TCP_PORT = 5005

minRange = 68
maxRange = 245
servoAngularRange = 160 # degrees

def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

stepSize = float(maxRange - minRange) / servoAngularRange

lcd = lcddriver.lcd()

# use 'GPIO naming'
wiringpi.wiringPiSetupGpio()

# set #18 to be a PWM output
wiringpi.pinMode(18, wiringpi.GPIO.PWM_OUTPUT)

# set the PWM mode to milliseconds stype
wiringpi.pwmSetMode(wiringpi.GPIO.PWM_MODE_MS)

# divide down clock
wiringpi.pwmSetClock(192)
wiringpi.pwmSetRange(2000)

wiringpi.pwmWrite(18, minRange)

#delay_period = 0.01

#for pulse in range(minRange, maxRange, 1):
#    wiringpi.pwmWrite(18, pulse)
#    time.sleep(delay_period)
#for pulse in range(maxRange, minRange, -1):
#    wiringpi.pwmWrite(18, pulse)
#    time.sleep(delay_period)

ip_addr = get_ip_address()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((ip_addr, TCP_PORT))
sock.listen(1)

print("Starting server on host", ip_addr, "port", TCP_PORT)
lcd.lcd_display_string("====CCOM==VisLab====", 1)
lcd.lcd_display_string("  IP:" + str(ip_addr), 2)
lcd.lcd_display_string("Port:" + str(TCP_PORT), 3)
lcd.lcd_display_string("Awaiting Connection", 4)

while True:
    print("Listening on ", ip_addr, ":", TCP_PORT, sep="")
    (clientsocket, address) = sock.accept()

    try:
        print("Connection from", address[0], "port", address[1])
        lcd.lcd_display_string(str(address[0]) + ":" + str(address[1]), 1)
        lcd.lcd_display_string(time.strftime("%m/%d/%Y %H:%M:%S"), 2)
        lcd.lcd_display_string("<Connected>", 3)

        while True:
            data = clientsocket.recv(16)
            if data:
                ts = time.localtime(time.time())
                print("Received data:", data.decode())
                lcd.lcd_display_string(time.strftime("%m/%d/%Y %H:%M:%S"), 2)
                lcd.lcd_display_string("Angle: " + str(data.decode()), 4)
                
                clientsocket.sendall(data)

                if int(data) >= 0 & int(data) <= servoAngularRange:
                    wiringpi.pwmWrite(18, int(minRange + stepSize * int(data)))
            else:
                print("Connection lost from", address[0], "port", address[1])
                lcd.lcd_display_string(time.strftime("%m/%d/%Y %H:%M:%S"), 2)
                lcd.lcd_display_string("<Connection lost>", 3)
                break
    finally:
        clientsocket.close()