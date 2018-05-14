# NixieClockRaspberryPi
Simple Nixie Tubes Clock Based on RaspberryPi, adapter and NCS314 shield for Arduino (by GRA and AFCH)

A simple program that show the current system time at the Nixie tubes.

Requared hardware:
  1) Raspberry Pi any models with 40-pin GPIO connector (Except A and B models).
  2) Arduino to Raspberry Pi adapter (by GRA & AFCH): https://gra-afch.com/catalog/shield-nixie-clock-for-arduino/raspberry-pi-shield-nixie-tubes-clock-ncs314-for-in-14-nixie-tubes-options-tubes-gps-remote-arduino-columns-copy/
  3) Arduino Nixie Clock Shield NCS314 (v1.2, v2.x): https://gra-afch.com/catalog/shield-nixie-clock-for-arduino/nixie-tubes-clock-arduino-shield-ncs314-for-xussr-in-14-nixie-tubes/

How to use binary:

1) Download and unzip.
2) Set execute permission "Anyone" to "DisplayNixie" file: https://photos.app.goo.gl/yVWtZkcIU9afhsvI2
3) Enable SPI module in the "Raspberry Pi Configuration": https://photos.app.goo.gl/vH7DtG9nwMzJHwvP2
  3.1) or CLI way "sudo raspi-config": https://photos.app.goo.gl/wfoPd8CNLSlJ0bF83
4) Run the program "sudo /home/pi/Downloads/NixieClockRaspberryPi-master/Bin/DisplayNixie clock" : 
https://photos.google.com/album/AF1QipPrdCjy8WkSl1f51m8EP9go7VUFAiGsy_bjW9pH/photo/AF1QipPch7Yz6RWZKBhiU_U8SIZtXHFvxyfVzh27Byjp

Step-by-Step video: https://youtu.be/-58clsFwA3I

Сommand line options:
1) 123456 - any 6 digits. Shield will display these numbers.
2) now - Shield will display current time at the time the command is executed.
3) clock - the program will loop and will update every second of the current time on the tubes.
