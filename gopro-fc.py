"""
   gopro-fc.py 
   
   This program scenario is;
   1. establish connection to GoPro
   2. wait for GPIO(SIGNAL_PORT) 
   3. Send Shutter command to GoPro
   
   4. Frequently send keep alive command to GoPro
   
   Therefore this program has infinite loop
   
   10/12/2023 First Edition. By Tsukasa Takao
"""

import requests
import time
import json
import threading
import RPi.GPIO as GPIO

SIGNAL_PORT = 26
GOPRO_BASE_URL = "http://10.5.5.9:8080/"

def request_shutter(pin):
    if pin == SIGNAL_PORT:
        response = requests.get(GOPRO_BASE_URL
                            +"/gopro/camera/shutter/start",
                            timeout=3)
    # request_cmd("/gopro/camera/shutter/stop") # release

        response.raise_for_status()
        resp = response.json()
        print( json.dumps(resp) )

    else:
        pass

def request_alive(): # loop as ohter thread

    while True:
        response = requests.get(GOPRO_BASE_URL
                                +"/gopro/camera/keep_alive",
                                timeout=3)
        response.raise_for_status()
        resp = response.json()

        print("Keep Alive")

        time.sleep(3)

if __name__ == '__main__':
    
    # Setup 
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(SIGNAL_PORT, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    # Try photo mode.
    response = requests.get(GOPRO_BASE_URL+
                            "/gopro/camera/presets/set_group?id=1001",
                            timeout=3)
    # request_cmd("/gopro/camera/shutter/stop") # release
    # if not good, throw error.
    response.raise_for_status()
    
    # OK. generate keep alive thread
    thread_A = threading.Thread(target=request_alive)
    thread_A.start()
    
    # GPIO event listner
    GPIO.add_event_detect(SIGNAL_PORT,GPIO.RISING,
        callback=request_shutter, bouncetime=300)
    
    # Loop
    while True:
        sleep(10)

    thread_A.join() # wait until thread_A
