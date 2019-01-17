//============================================================================
// Name        : DisplayNixie.cpp
// Author      : GRA&AFCH
// Version     : v1.2
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
	printf("Display Nixie CLI Tool v1.2\n\r");
	//printf("Arg len: %d \n\r", strlen(argv[1]));
	if (argc < 2)
	{
		printf("Enter digits to display... or commands: now - present time, clock - loop program");
		return 0;
	}
	wiringPiSetup ();

	if (wiringPiSPISetupMode (0, 2000000, 2)) printf("SPI ok\n\r");
			else {printf("SPI NOT ok\n\r"); return 0;}
	//char* _argv;
	//String _stringToDisplay=argv[1];
	char _stringToDisplay[10];
	bool clock=0;
	int tubes_qty=0;

	do {
	if (!strcmp(argv[1],"now")) //printf("NOWW"); else printf("NOTTT NOWW");
	{
		time_t seconds=time(NULL);
		tm* timeinfo=localtime(&seconds);
		char* format="%H%M%S";
		strftime(_stringToDisplay, 8, format, timeinfo);
	}

	else if (!strcmp(argv[1],"clock")) //printf("NOWW"); else printf("NOTTT NOWW");
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
		tubes_qty=strlen(argv[1]);
		if ((tubes_qty != 6) && (tubes_qty != 9))
		{
			puts("Wrong length: must be 6 or 9 digits");
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

	/*Var32=0;

	Var32= (SymbolArray[_stringToDisplay[5]-0x30])<<20;
	Var32|=(SymbolArray[_stringToDisplay[4]-0x30])<<10;
	Var32|=(SymbolArray[_stringToDisplay[3]-0x30]);

	buff[0]=Var32>>24;
	buff[1]=Var32>>16;
	buff[2]=Var32>>8;
	buff[3]=Var32;
	// HV #2
	Var32=0;

	Var32= (SymbolArray[_stringToDisplay[2]-0x30])<<20;
	Var32|=(SymbolArray[_stringToDisplay[1]-0x30])<<10;
	Var32|=(SymbolArray[_stringToDisplay[0]-0x30]);

	buff[4]=Var32>>24;
	buff[5]=Var32>>16;
	buff[6]=Var32>>8;
	buff[7]=Var32;
	//HV#1 */

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
	} while (clock);
	return 0;
}
