# gopro-fc
 Gopro control from FC with companion computer

 ## Introduction
 I am making some drone boats.
 As usual drone users, I also want to take pictures on the boat.
 I think GoPro camera is very popular.
 But I have never seen using Gopro on the drone boat.
 Then I make a solution. 
 Building this system is not easy for me. Then I'd like to share my experiences with Ardupilot folks.

 ## Basic concept
 Please see following high level chart.
 ![](/images/gopro-fc.png)

1. GCS or Transmitter send "camera trigger". Flight Controller raise signal at AUX OUT pin.
1. Companion Computer(Raspberry Pi) watch AUX OUT. When getting signal, Raspberry pi send a shutter command to GoPro via Wifi.
1. WiFi antenna is reached to GoPro. GoPro can get signal even if in the sea.

## I. GoPro
GoPro is not only very popular but program interface via USB, WiFi and Bluetooth. 
We can find GoPro API at [here](https://gopro.github.io/OpenGoPro/) 

When thinking under the water environment, we can use great GoPro protection case. Then we don't want to use USB cable nor don't want to use weak Bluetooth.
The following discussion is just only focus on WiFi.

## II. WiFi
What we want to do is like this illustration.
{: align="center"}
![](/images/camera_raspi.png)

Usually, we say that radio waves cannot transmit through water.
It is not strictly correct. radio waves decay rapidly in water.
When you attach antenna closely GoPro, you can use wifi.

### USB WiFi dongle and cable
First of all, we must find WiFi USB dongle with antenna connector. Because it is very hard to add antenna on the Raspberry Pi board and that WiFi power is weak.
I find USB WiFi dongle with screw antenna.
And you find 1m coaxial cable with SMA-P connector.
![](/images/cable.jpg)
![](/images/wifi_dongle.jpg)

Depending on cable connector, you may need gender change connector.

### Antenna
<table>
<tbody><tr><td>
Note: If you use this system on air craft. You don't need to make Antenna. Maybe you can use Raspberry Pi on board WiFi.
</td></tr></tbody>
</table>

At the cable opposite side, make an antenna for wifi.
Wifi 2.5GHz wave length is about 12 cm. Then the antenna length should be 6cm (1/2 wave length) or 3cm (1/4 wave length)
Now strip off approx. 3cm.
{: align="center"}
![](/images/antenna2.jpg)

I put glue and cover by heat shrink tube for water resistant.

### USB device Driver
THe following process is supported by Raspbian OS **64bit**

First of all, just connect USB WiFi dongle and see 'ifconfig' or 'ip a' command response. If your WiFi dongle is recognized as 'wlanx'. You are so lucky. Raspbian OS has adequate device driver.
If your WiFi dongle does not work, you need to install device driver.
You enter the command 'lsusb', you can find connected device characteristics. In my case, that is "Realtek Semiconductor Corp. RTL88x2bu".

Then I need to install RTL88x2bu device driver.
(Unfortunately, I am not familiar with kernel compile. I introduce just only install scenario step by step)

1. sudo apt update
1. sudo apt upgrade

1. sudo apt install git bc dkms raspberrypi-kernel-headers
1. sudo apt update
1. sudo reboot

1. git clone https://github.com/cilynx/rtl88x2bu
1. cd  rtl88x2bu/

1. sed -i 's/I386_PC = y/I386_PC = n/' Makefile
1. sed -i 's/ARM_RPI = n/ARM_RPI = y/' Makefile

1. VER=$(sed -n 's/\PACKAGE_VERSION="\(.*\)"/\1/p' dkms.conf)
1. sudo rsync -rvhP ./ /usr/src/rtl88x2bu-${VER}
1. sudo dkms add -m rtl88x2bu -v ${VER}
1. sudo dkms build -m rtl88x2bu -v ${VER}
1. sudo dkms install -m rtl88x2bu -v ${VER}

After finishing these process, you need reboot.
You will find wlan0 and wlan1 

To erase wlan0 (on board wifi) you make the following file.

filename: /etc/modprobe.d/raspi-blacklist.conf
~~~
#wifi
blacklist brcmfmac
blacklist brcmutil
~~~
And reboot.

Now you use USB WiFi dongle.

## III. Open GoPro
GoPro has API interface from 2021.
It's called [Open GoPro](https://gopro.github.io/OpenGoPro/)

When you use GoPro Wifi, first of all you start and connect QUIK from your smartphone and push "Preview" button. QUIK shows the window connect to GoPro with Access Point.
You can get AP name and password from "Camera info" in "connect" menu.
The most important thing is GoPro is Access Point. The Raspberry Pi should be client.
Then you prepare GoPro AP in the Raspberry Pi.

### Open GoPro installation
pip is required.
~~~
sudo apt -y install python3-pip
~~~
And install
~~~
pip install open-gopro
~~~

### Before setting WiFi
Maybe you connect your Raspberry PI to your home WiFi router.
The following steps connect your Raspberry pi to GoPro.
You need to change WiFi connection.
It means if you use SSH usually, it cannot be used.
There are two solutions.
One way is preparing physical HDMI display and wireless keyboard.
Another way is using Ethernet adapter. If your Windows or Mac does not have Ethernet connector, you can prepare USB-Ethernet connector. It is relatively cheap and some connector supports Mac environment and Windows one.

I am using Ether net connector and both side IP address are static like 192.168.300.x. Setting is easy and you can find the way anywhere.


### WiFi setting in Raspberry Pi
Please check contents in 

/etc/wpa_supplicant/wpa_supplicant.conf

if you setup SSH, you will see the definition of current WiFi access point name.
Anyway you add
~~~
network={
    ssid="Gopro Access Point Name"
    psk="password"
}
~~~

Reboot is required.
After reboot finished, you can test wpa_cli command.
wpa_cli has rich functions, but now we just only test list and set AP.
When you want to list AP
~~~
wpa_cli -i wlan- list_networks
~~~

if you choose AP you want to connect
~~~
wpa_cli -i wlan- select_network x
~~~
x is the list number.

Of course this command can change connection, but if choosing AP can use or not is other discussion.
After changing AP connection, please wait a few minutes.
Check 'ifconfig' command if your Raspi is assigned ip address or not.
GoPro will give you 10.5.5.x local ip address.

## IV. Python test
Try following simple program.
~~~
import requests
import time
import json

GOPRO_BASE_URL = "http://10.5.5.9:8080/"
cmd = GOPRO_BASE_URL+"/gopro/camera/presets/set_group?id=1001"
response = requests.get(cmd,timeout=10)
response_raise_for_status()
resp = response.json()
print( json.dumps(resp))
~~~
THis command make gopro enter into camera mode.
When run this program and GoPro makes some noise.
Maybe OK.

## V. Ardupilot / Pixhawk
Now we set up Ardupilot.
For sending shutter signal. You need to change parameter
~~~
CAM_TRIGG_TYPE=1
~~~
THis is in other words "Relay mode"

### AUX OUT
WHich pins should be shutter ?
Usually, AUX OUT 5 or AUX OUT 6 are chosen.

![](/images/pixhawk_pins.jpg)

These are SERVO13 or SERVO14.
I choose AUX OUT5.
Then parameter set as 
~~~
RELAY_PIN=54
~~~
And Servo13 should be GPIO
~~~
SERVO13_FUNCTION=-1
~~~
Finally don't forget "Write parameters"

You can change shutter push duration.
But we will pick up GPIO voltage up edge, duration is not so important.

FInally set Transmitter switch
~~~
RC6_OPTION=9
~~~
9 means Camera Trigger.

If you set all following above specifications. GPIO issue pulse when shutter is requested.
![](/images/shutter.jpg)

### Raspberry PI GPIO
We transfer Pixhawk signal to Raspberry pi GPIO.
This time, I choose GPIO26.
You can see GPIO pins number or specification on [http://pinout.xyz/]

IO pin and GND should be connected like this.
![](/images/fc_raspi_con.jpg)

The connection test is using 
[gpiotest3.py](https://github.com/ttakao/gopro-fc/blob/main/gpiotest3.py) program.

THis program is watching GPIO26(TESTIO). If GPIO voltage is high, the interruption is brought up and 'callback1' routine is called.

## VI. Pixhawk - Raspberry PI - GoPro

Finally, the system is built.

### Antenna
Fix the antenna on the GoPro.

![](/images/antenna3.jpg)

(This picture uses see through scotch tape for your understanding. Actually use water resistant tape.)

### Python program
you can try final code [gopro-fc.py](https://github.com/ttakao/gopro-fc/blob/main/gopro-fc.py)

THis program generate another thread for sending KeepAlive signal to GoPro. If you don't send GoPro signal, GoPro might be go down sleep.
If you start this program, first of all mode is changed into Camera and wait for Flight Controller signal.
You can try Transmitter switch (6) or 'Shutter Trigger' in connected Mission Planner.
GoPro camera shutter is triggered.

### Start up procedure
I put here a standard procedure for your understanding.

-[ ] Power on Pixhawk and Raspberry PI.
-[ ] Connect to Mission Planner (Maybe via telemetry)
-[ ] Connect to Transmitter.
-[ ] Connect ether-net cable between PC and Raspi.
-[ ] Start up SSH with ether-net.
-[ ] Make sure boat status.
-[ ] Power on GoPro.
-[ ] Connect to QUIK and push "Preview botton"
-[ ] Connect to GoPro WiFi using wpa_cli command.
-[ ] Check ifconfig if assgined ip address by GoPro
-[ ] Start like gopro-fc.py script.
-[ ] Set Gopro on the boat.
-[ ] Start boat and run Survey mode on the Mission Planner.


