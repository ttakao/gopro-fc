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

## III. Antenna

## IV. Raspberry Pi

## V. Ardupilot / Pixhawk

Final working code is [gopro-fc.py](https://github.com/ttakao/gopro-fc/blob/main/gopro-fc.py)
