//============================================================================
// Name        : DisplayNixie.cpp
// Author      : GRA&AFCH
// Version     :
// Copyright   : Free
// Description : Display digits on shields
//============================================================================

#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <ctime>
#include <string.h>

using namespace std;
int LEpin=3;
uint16_t SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

int main(int argc, char* argv[]) {
	if (argc < 2)
	{
		printf("Enter digits to display... or commands: now - present time, clock - loop program");
		return 0;
	}
	wiringPiSetup ();

	if (wiringPiSPISetupMode (0, 2000000, 2)) printf("SPI ok");
			else {printf("SPI NOT ok"); return 0;}
	char _stringToDisplay[8];
	bool clock=0;

	do {
	if (!strcmp(argv[1],"now")) 
	{
		time_t seconds=time(NULL);
		tm* timeinfo=localtime(&seconds);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, timeinfo);
	}

	else if (!strcmp(argv[1],"clock")) 
	{
		time_t seconds=time(NULL);
		tm* timeinfo=localtime(&seconds);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, timeinfo);
		clock=1;
		delay (1000);
	}

	else
	{
		for (int i=0; i<8; i++)
		{
			_stringToDisplay[i]=argv[1][i];
		}
	}

	pinMode(LEpin, OUTPUT);

	uint32_t Var32=0;

	Var32= (SymbolArray[_stringToDisplay[5]-0x30])<<20;
	Var32|=(SymbolArray[_stringToDisplay[4]-0x30])<<10;
	Var32|=(SymbolArray[_stringToDisplay[3]-0x30]);

	unsigned char buff[8];
	buff[0]=Var32>>24;
	buff[1]=Var32>>16;
	buff[2]=Var32>>8;
	buff[3]=Var32;

	Var32=0;

	Var32= (SymbolArray[_stringToDisplay[2]-0x30])<<20;
	Var32|=(SymbolArray[_stringToDisplay[1]-0x30])<<10;
	Var32|=(SymbolArray[_stringToDisplay[0]-0x30]);

	buff[4]=Var32>>24;
	buff[5]=Var32>>16;
	buff[6]=Var32>>8;
	buff[7]=Var32;

	digitalWrite(LEpin, LOW);
	wiringPiSPIDataRW(0, buff, 8);
	digitalWrite(LEpin, HIGH);
	} while (clock);
	return 0;
}
