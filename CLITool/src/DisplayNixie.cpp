//============================================================================
// Name        : DisplayNixie.cpp
// Author      : GRA&AFCH
// Version     : v1.3
// Copyright   : Free
// Description : Display digits on shields
//============================================================================

#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <ctime>
#include <string.h>
#include <wiringPiI2C.h>
#include <time.h>

#define _VERSION 1.3
#define I2CAdress 0x68
#define I2CFlush 0
#define SECOND_REGISTER 0x0
#define MINUTE_REGISTER 0x1
#define HOUR_REGISTER 0x2

using namespace std;
int LEpin=3;
int I2CFileDesc;
uint16_t SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

int decToBcd(int val) {
	return ((val / 10  * 16) + (val % 10));
}

void writeRTCDate(tm date) {
	wiringPiI2CWrite(I2CFileDesc, I2CFlush);
	wiringPiI2CWriteReg8(I2CFileDesc,HOUR_REGISTER,decToBcd(date.tm_hour));
	wiringPiI2CWriteReg8(I2CFileDesc,MINUTE_REGISTER,decToBcd(date.tm_min));
	wiringPiI2CWriteReg8(I2CFileDesc,SECOND_REGISTER,decToBcd(date.tm_sec));
	wiringPiI2CWrite(I2CFileDesc, I2CFlush);
}

int main(int argc, char* argv[]) {
	printf("Display Nixie CLI Tool v%1.1f \n\r", _VERSION);
	printf("%s", argv[1]);

	if (argc < 2)
	{
		printf("Enter digits to display... or commands: \n now - show current time, \n "
				//"clock - loop the program and update time every second, \n "
				"[digits] - and six or nine digits, \n "
				"settime x - set time, where x time in format [hh:mm:ss], \n "
				"setsystime - set current time from OS, \n "
				//"ledson - turn on RGB LEDs,\n "
				//"ledsoff - turn off RGB LEDs, \n "
				//"setledscolor x - set color of LEDs where x is color in [RRR:GGG:BBB] format, \n "
				//"setledsbright x - [0...255], \n "
				"? - show this help.");
		return 0;
	}
	wiringPiSetup ();

	if (wiringPiSPISetupMode (0, 2000000, 2)) printf("SPI ok\n\r");
			else {printf("SPI NOT ok\n\r"); return 0;}

	I2CFileDesc = wiringPiI2CSetup(I2CAdress);

	char _stringToDisplay[10];
	int tubes_qty=6;

	if (!strcmp(argv[1],"now"))
	{
		time_t seconds=time(NULL);
		tm* timeinfo=localtime(&seconds);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, timeinfo);
	}

	else if (!strcmp(argv[1],"settime"))
		{
		_stringToDisplay[0]=argv[2][0];
		_stringToDisplay[1]=argv[2][1];
		_stringToDisplay[2]=argv[2][3];
		_stringToDisplay[3]=argv[2][4];
		_stringToDisplay[4]=argv[2][6];
		_stringToDisplay[5]=argv[2][7];
		tm time;
		time.tm_hour=10*((int)_stringToDisplay[0]-48)+(int)_stringToDisplay[1]-48;
		time.tm_min=10*((int)_stringToDisplay[2]-48)+(int)_stringToDisplay[3]-48;
		time.tm_sec=10*((int)_stringToDisplay[4]-48)+(int)_stringToDisplay[5]-48;
		writeRTCDate(time);
		}

	else if (!strcmp(argv[1],"setsystime"))
		{
		time_t seconds=time(NULL);
		tm* time=localtime(&seconds);
		writeRTCDate(*time);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, time);
		}

	else
	{
		tubes_qty=strlen(argv[1]);
		if ((tubes_qty != 6) && (tubes_qty != 9))
		{
			puts("Wrong length: must be 6 or 9 digits. \n");
			return 0;
		}
		for (int i=0; i<tubes_qty+1; i++)
		{
			_stringToDisplay[i]=argv[1][i];
		}
	}

	pinMode(LEpin, OUTPUT);

	uint32_t Var32;
	unsigned char buff[12];

	int bufferIndex=0;

	for (int i=0; i<tubes_qty+1; i=i+3)
	{
		Var32=0;

		Var32= (SymbolArray[_stringToDisplay[tubes_qty-i-1]-0x30])<<20;
		Var32|=(SymbolArray[_stringToDisplay[tubes_qty-i-2]-0x30])<<10;
		Var32|=(SymbolArray[_stringToDisplay[tubes_qty-i-3]-0x30]);

		buff[bufferIndex]=Var32>>24;
		bufferIndex++;
		buff[bufferIndex]=Var32>>16;
		bufferIndex++;
		buff[bufferIndex]=Var32>>8;
		bufferIndex++;
		buff[bufferIndex]=Var32;
		bufferIndex++;
	}

	digitalWrite(LEpin, LOW);
	wiringPiSPIDataRW(0, buff, 4*tubes_qty/3);
	digitalWrite(LEpin, HIGH);
	puts("Exit...");
	return 0;
}

