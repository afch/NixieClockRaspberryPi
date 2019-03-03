# NixieClockRaspberryPi
## With mods by Leon Shaner

Simple Nixie Tubes Clock Based on RaspberryPi, adapter and NCS314 shield for Arduino (by GRA and AFCH)

A simple program that show the current system time at the Nixie tubes.

### Major features of the Shaner mods:

  1) Use the system RTC by default (e.g. rely on NTP for more accurate time).
  2) Option to display 12-hour or 24-hour for the hour digits.
  3) Repurpose MODE button to toggle fireworks on/on.
  4) Repurpose UP/DOWN buttons to increase/decrease the fireworks cycle speed.
  5) Command-line parsing to choose preferred options without recompile.
  6) Re-wrote makefile for proper handling of binary rebuild any time the sources are modified.

### Required hardware:
  1) Raspberry Pi any models with 40-pin GPIO connector (Except A and B models).
  2) Arduino to Raspberry Pi adapter (by GRA & AFCH): https://gra-afch.com/catalog/shield-nixie-clock-for-arduino/raspberry-pi-shield-nixie-tubes-clock-ncs314-for-in-14-nixie-tubes-options-tubes-gps-remote-arduino-columns-copy/
  3) Arduino Nixie Clock Shield NCS314 (v1.2, v2.x): https://gra-afch.com/catalog/shield-nixie-clock-for-arduino/nixie-tubes-clock-arduino-shield-ncs314-for-xussr-in-14-nixie-tubes/


### How to build:

  1) Download and unzip.
  2) Enable SPI module in the "Raspberry Pi Configuration": https://photos.app.goo.gl/vH7DtG9nwMzJHwvP2
  2.1) or CLI way "sudo raspi-config": https://photos.app.goo.gl/wfoPd8CNLSlJ0bF83

  3) cd into .../DisplayNixie/src
  4) Run 'make'
  5) Binary will be placed at .../DisplayNixie/bin/DisplayNixie, a la:
	/nixie/NixieClockRaspberryPi-shaner/DisplayNixie/bin/DisplayNixie

### Сommand line options (shaner mods):
* USAGE: DisplayNixie     -- Use system clock in 12-hour mode.
* DisplayNixie nosysclock -- use Nixie clock (e.g. not NTP assisted).
* DisplayNixie 24hour     -- use 24-hour mode.
* DisplayNixie fireworks  -- enable fireworks.

NOTE:  Any combination/order of above arguments is allowed.

### Run-time options (shaner mods):

* MODE Button:  Toggle fireworks on/off
* UP Button:    Speed up fireworks color-rotation
* DOWN Button:  Slow down fireworks color-rotation


### Autorun the program at startup: 

1) Edit the provided nixie.service template to reflect the full path down to your binary:
NOTE: Fix the path on the following line in nixie.service to match your binary location.

  ExecStart=/nixie/NixieClockRaspberryPi-shaner/DisplayNixie/bin/DisplayNixie

2) Copy your customized nixie.service into the /etc/systemd/system directory:
*  $ sudo cp nixie.service /etc/systemd/system

3) Reload the systemd state and start the nixie service:
*  $ sudo systemctl daemon-reload
*  $ sudo systemctl start nixie
NOTE:  The systemd will auto-start DisplayNixie whenever the system is booted.

4) To stop and/or restart the nixie binary at any time:
*  $ sudo systemctl stop nixie
*  $ sudo systemctl restart nixie
