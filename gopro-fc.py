import requests
import time
import datetime
import json
import jmespath
import threading
import gps
from gpiozero import Button

    """_summary_ This program runs on Raspberry pi.
    Watching GPIO 26. if short, start shutter process.
    """
SIGNAL_PORT = 26
GOPRO_BASE_URL = "http://10.5.5.9:8080/"

from logging import getLogger

def init():
    log = getLogger("gopro-fc2")
     # Try photo mode.
    response = requests.get(GOPRO_BASE_URL+
                            "/gopro/camera/presets/set_group?id=1001",
                            timeout=3)
    # request_cmd("/gopro/camera/shutter/stop") # release
    # if not good, throw error.
    response.raise_for_status()

    log.info("GoPro in picture mode.")

    # Generate keep alive thread
    thread_A = threading.Thread(target=request_alive)
    thread_A.start()    

    # thread_A.join() # wait until thread_A

def shutter_process():
    global listname

    log = getLogger("gopro-fc2")

    log.info("Shutter requested")

    #first media list
    response = requests.get(GOPRO_BASE_URL
                            + "/gopro/media/list", timeout=4)
    response.raise_for_status()

    jsonlist = json.loads(response.text)
    filelist_before = jmespath.search("media[].fs[].n[]", jsonlist)

    response = requests.get(GOPRO_BASE_URL
                            +"/gopro/camera/shutter/start",
                            timeout=3)
    # request_cmd("/gopro/camera/shutter/stop") # release

    time.sleep(1)
    response.raise_for_status()

    response = requests.get(GOPRO_BASE_URL
                            + "/gopro/camera/shutter/stop",
                            timeout=3)
    time.sleep(1)
    resp=response.raise_for_status()
    
    # second media list
    response = requests.get(GOPRO_BASE_URL
                            + "/gopro/media/list", timeout=4)

    time.sleep(1)    
    response.raise_for_status()
    
    jsonlist = json.loads(response.text)
    filelist_after = jmespath.search("media[].fs[].n[]", jsonlist)
    
    # find added file name.
    difflist = list(set(filelist_after) - set(filelist_before))
    diffname = difflist[0]

    log.info("file name :"+diffname+" Generated.")
    # get GPS location
    loc = gps.getlocation()
    lat = str(loc[0])
    lon = str(loc[1])
    alt = str(loc[2])
    heading = str(loc[3])
    # picture download needs a few minutes. we cannot wait.
    # then we create location file.
    
    # picture list name
    now = datetime.datetime.now()
    listname = './piclist/' + now.strftime("%Y%m%d")+'.txt'
    
    with open(listname,'a') as fs:
        print(f"{diffname}:{lat}:{lon}:{alt}:{heading}", file=fs)
        

def request_alive(): # loop as other thread
    log = getLogger("gopro-fc2")

    while True:
        response = requests.get(GOPRO_BASE_URL
                                +"/gopro/camera/keep_alive",
                                timeout=3)
        response.raise_for_status()
        resp = response.json()

        log.info("Keep Alive")

        time.sleep(30)
        
if __name__ == '__main__':
    init()

    # GPIO event listner
    button = Button(SIGNAL_PORT, pull_up=False)
    button.when_pressed = shutter_process

    # Loop
    while True:
        time.sleep(10)

    thread_A.join() # wait until thread_A
