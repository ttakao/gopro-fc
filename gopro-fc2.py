"""
gopro-fc2.py
  rebuild gopro-fc. 2024/06
"""
from logging import getLogger, StreamHandler, Formatter,FileHandler,DEBUG
import serial
import time
import sys

import gps
import gopro
from gpiozero import Button

SIGNAL_PORT = 26
global depth  # mm
global arduino # arduino serial port

def winch_ctrl():
    # at first, down winch
    global arduino.write("start")
    
    line = global arduino.readline()
    status = line.strip().decode('UTF-8')
    
    if (status == "END RC=0"){
        gopro.shutter_process()
    } else {
        print("FAIL!!!")
    }
    
    global arduino.write("rewind")
    line = global arduino.readline()
    status = line.strip().decode('UTF-8')
    # Everything is OK


if __name__ == '__main__':
    # 引数　（深度）
    if (len(sys.argv) == 2):
        global depth = int(sys.argv[1])    
    
    # logger 初期化
    log = getLogger("gopro-fc2")
    log.setLevel(DEBUG)
    log_format = Formatter(
        '%(asctime)s [%(levelname)s] %(message)s')
    stream_handler = StreamHandler()
    stream_handler.setLevel(DEBUG)
    stream_handler.setFormatter(log_format)
    log.addHandler(stream_handler)
    file_handler = FileHandler('./gopro-fc2.log')
    file_handler.setLevel(DEBUG)
    file_handler.setFormatter(log_format)
    log.addHandler(file_handler)
     
    gopro.init()
    
    # communicate with Arduino. Wait for until receive data
    global arduino = serial.Serial('/dev/ttyUSB0',9600, timeout=None) 
    global arduino.reset_input_buffer()
    global arduino.write("depth="+str(global depth))
    
    # GPIO event listner
    button = Button(SIGNAL_PORT, pull_up=False)
    button.when_pressed = winch_ctrl
    
    # Loop until raising interruption
    while True:
        time.sleep(10)