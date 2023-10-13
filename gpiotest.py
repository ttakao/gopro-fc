"""
GPIOTEST3
"""
import RPi.GPIO as GPIO
from time import sleep

TESTIO = 26

def main():
    GPIO.setmode(GPIO.BCM) #ピン番号指定
    GPIO.setup(TESTIO, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    GPIO.add_event_detect(TESTIO, GPIO.RISING, 
                       callback=callback1, bouncetime=300)

    try:
        while True:
            sleep(1)

    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()
        print("The end of program")

def callback1(number):
    print("%s is High "%number)
    
if __name__ == "__main__":
    main()