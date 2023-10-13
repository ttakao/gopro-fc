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
At the cable opposite side, make an antenna for wifi.
Wifi 2.5GHz wave length is about 12 cm. Then the antenna length should be 6cm (1/2 wave length) or 3cm (1/4 wave length)
Now strip off approx. 3cm.
![](/images/antenna2.jpg)

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
‘‘‘#wifi
blacklist brcmfmac
blacklist brcmutil
‘‘‘
And reboot.

Now you use USB WiFi dongle.

## IV. Raspberry Pi

## V. Ardupilot / Pixhawk

Final working code is [gopro-fc.py](https://github.com/ttakao/gopro-fc/blob/main/gopro-fc.py)
