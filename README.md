# NixieClockRaspberryPi
Simple Nixie Tubes Clock Based on RaspberryPi, adapter and NCS314 shield for Arduino (by GRA and AFCH).

A simple program that show the current time (stored in shield's onboard RTC) at Nixie tubes.

(Enhanced mod by Leon Shaner available here: https://github.com/UberEclectic/NixieClockRaspberryPi/tree/master/DisplayNixie)

    Requared hardware:
  1) Raspberry Pi any models with 40-pin GPIO connector (Except A and B models).
  2) Arduino to Raspberry Pi adapter (by GRA & AFCH): https://gra-afch.com/catalog/shield-nixie-clock-for-arduino/raspberry-pi-shield-nixie-tubes-clock-ncs314-for-in-14-nixie-tubes-options-tubes-gps-remote-arduino-columns-copy/
  3) Arduino Nixie Clock Shield NCS314 (v1.2, v2.x): https://gra-afch.com/catalog/shield-nixie-clock-for-arduino/nixie-tubes-clock-arduino-shield-ncs314-for-xussr-in-14-nixie-tubes/

    How to use binaries:

1) Download and unzip.
2) Set execute permission "Anyone" to "DisplayNixie" file: https://photos.app.goo.gl/yVWtZkcIU9afhsvI2
3) Enable SPI and I2C modules in the "Raspberry Pi Configuration": https://photos.app.goo.gl/vH7DtG9nwMzJHwvP2
  3.1) or CLI way "sudo raspi-config": https://photos.app.goo.gl/wfoPd8CNLSlJ0bF83
4) Run the program "sudo /home/pi/Downloads/NixieClockRaspberryPi-master/Firmware/Debug/DisplayNixie": 
https://photos.google.com/album/AF1QipPrdCjy8WkSl1f51m8EP9go7VUFAiGsy_bjW9pH/photo/AF1QipPch7Yz6RWZKBhiU_U8SIZtXHFvxyfVzh27Byjp

    Step-by-Step video: https://youtu.be/-58clsFwA3I

Сommand line options available for CLITool (/home/pi/Downloads/NixieClockRaspberryPi-master/CLITool/Debug/CLITool):
1) 123456 - any 6 digits. Shield will display these numbers.
2) now - Shield will display current system time at the time the command is executed.
3) clock - the program will loop and will update every second the time from Real Time Clock (RTC on shield board) on tubes.
4) settime x - set time, where x time in format [hh:mm:ss].
5) setsystime - set current time from OS.
Example: "sudo /home/pi/Downloads/NixieClockRaspberryPi-master/CLITool/Debug/CLITool 123456" - will show 123456 on tubes.

        To compile the source: 
        
Add folder "DisplayNixie" into the workspace of Eclipse:
"File"->"Import", "Genral"->"Existing Project into Workspce".
p.s. The project uses the "wiringPi" library: http://wiringpi.com/download-and-install/

        Autorun the program at startup: 
Edit /home/pi/.bashrc file - add string like this: 
sudo "/home/pi/Documents/Eclipse Projects/DisplayNixie/Debug/DisplayNixie" clock
