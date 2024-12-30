#!/usr/bin/python3

import requests
import time
import json
import threading
import logging as log
from logging import getLogger
import cgi
import cgitb
import os

cgitb.enable()

logformat = "%(asctime)s %(levelname)s %(name)s :%(message)s"
log.basicConfig(filename="/home/tsukasa/www/cgi-bin/cgi.log", 
                format=logformat, level=log.DEBUG)

GOPRO_BASE_URL = "http://10.5.5.9:8080"
WRITE_PATH = "/home/tsukasa/www/snapshot.jpg"

def init():    
    response = requests.get(GOPRO_BASE_URL + 
                            "/gopro/camera/presets/set_group?id=1001",
                            timeout=10)
    time.sleep(1)
    response.raise_for_status()

    log.info("GoPro in picture mode.")

def send_shutter():
    log.info("Shutter Requested.")
    response = requests.get(GOPRO_BASE_URL + 
                            "/gopro/camera/shutter/start",
                            timeout=10)
    time.sleep(1)
    response.raise_for_status()

    response = requests.get(GOPRO_BASE_URL +
                            "/gopro/camera/shutter/stop",
                            timeout=10)
    time.sleep(1)
    response.raise_for_status()

def get_filename():

    log.info("get picture filename.")

    response = requests.get(GOPRO_BASE_URL + 
                            "/gopro/media/last_captured",
                            timeout=10)
    time.sleep(1)
    response.raise_for_status()

    path = json.loads(response.text)
    
    return path

def get_picture(path):
    time.sleep(1)
    filename = path["file"]
    foldername = path["folder"]
    url = GOPRO_BASE_URL + f"/videos/DCIM/{foldername}/{filename}"

    log.info("Download picture.")
    response = requests.get(url, stream=True, timeout=10)
    time.sleep(2)
    response.raise_for_status()
    
    
    with open(WRITE_PATH, "wb") as f:
        log.info(f"Receiving stream {filename} to {WRITE_PATH}")
        for chunk in response.iter_content(chunk_size=1024):
            f.write(chunk)
    time.sleep(1)
#------------

init()
send_shutter()
path = get_filename()
get_picture(path)

form = cgi.FieldStorage()
calledURL = form.getvalue('currentURL')
log.info(f"Called URL= {calledURL}")

print(f"Location: http://{calledURL}\n\n")
exit()