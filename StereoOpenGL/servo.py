import wiringpi
from time import sleep

class servo:
    def __init__(self):
        minRange = 50
        self.maxRange = 257
        self.minStop = 60 # for -90 degree rotation
        self.maxStop = 249 # for +90 degree rotation
        self.origin = 154
        self.servoAngularRange = 180 # degrees

        self.stepSize = float(self.maxStop - self.minStop) / self.servoAngularRange
    
        self.angle = 0
    
        self.setupGPIO()
    
        wiringpi.pwmWrite(18, self.origin)
    
    def setupGPIO(self):
        # use 'GPIO naming'
        wiringpi.wiringPiSetupGpio()

        # set #18 to be a PWM output
        wiringpi.pinMode(18, wiringpi.GPIO.PWM_OUTPUT)

        # set the PWM mode to milliseconds stype
        wiringpi.pwmSetMode(wiringpi.GPIO.PWM_MODE_MS)

        # divide down clock
        wiringpi.pwmSetClock(192)
        wiringpi.pwmSetRange(2000)
        
    def set(self, angle, time = 2.5):
        #servo turns clockwise, so change sign of angle
        angle = -angle;
        if self.angle == angle:
            return
        if angle < -90:
            angle = -90
        if angle > 90:
            angle = 90
            
        delta = angle - self.angle
        nsteps = delta * self.stepSize
        sleeptime = abs(time / nsteps)
        
        start = int(self.origin + self.angle * self.stepSize)
        stop = int(self.origin + angle * self.stepSize)
        
        if start - stop > 0:
            movement = range(start, stop - 1, -1)
        else:
            if start - stop < 0:
                movement = range(start, stop + 1, 1)
        
        if sleeptime == 0 & len(movement) > 0:
            wiringpi.pwmWrite(18, movement[-1])
        else:
            for step in movement:
                wiringpi.pwmWrite(18, step)
                sleep(sleeptime)
                
        self.angle = angle