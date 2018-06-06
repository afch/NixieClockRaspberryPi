#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <ctime>
#include <string.h>
#include <wiringPiI2C.h>

using namespace std;
#define LEpin 3
#define UpButtonPin 1
#define DownButtonPin 4
#define ModeButtonPin 5
#define I2CAdress 0x68
#define I2CFlush 0
#define DateLength 7

#define SECOND_REGISTER 0x0
#define MINUTE_REGISTER 0x1
#define HOUR_REGISTER 0x2
#define WEEK_REGISTER 0x3
#define DAY_REGISTER 0x4
#define MONTH_REGISTER 0x5
#define YEAR_REGISTER 0x6

uint16_t SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

int bcdToDec(int val) {
	return ((val / 16  * 10) + (val % 16));
}

int decToBcd(int val) {
	return ((val / 10  * 16) + (val % 10));
}

struct Date {
	int second;
	int minute;
	int hour;
	int weekDay;
	int day;
	int month;
	int year;
};

Date getRTCDate(int fileDesc) {
	wiringPiI2CWrite(fileDesc, I2CFlush);
	Date date;
	date.second = bcdToDec(wiringPiI2CRead(fileDesc));
	date.minute = bcdToDec(wiringPiI2CRead(fileDesc));
	date.hour = bcdToDec(wiringPiI2CRead(fileDesc));
	date.weekDay = bcdToDec(wiringPiI2CRead(fileDesc));
	date.day = bcdToDec(wiringPiI2CRead(fileDesc));
	date.month = bcdToDec(wiringPiI2CRead(fileDesc));
	date.year = bcdToDec(wiringPiI2CRead(fileDesc));
	return date;
}

void writeRTCDate(int description,Date date) {
	wiringPiI2CWrite(description, I2CFlush);
	wiringPiI2CWriteReg8(description,SECOND_REGISTER,decToBcd(date.second));
	wiringPiI2CWriteReg8(description,MINUTE_REGISTER,decToBcd(date.minute));
	wiringPiI2CWriteReg8(description,HOUR_REGISTER,decToBcd(date.hour));
	wiringPiI2CWriteReg8(description,WEEK_REGISTER,decToBcd(date.weekDay));
	wiringPiI2CWriteReg8(description,DAY_REGISTER,decToBcd(date.day));
	wiringPiI2CWriteReg8(description,MONTH_REGISTER,decToBcd(date.month));
	wiringPiI2CWriteReg8(description,YEAR_REGISTER,decToBcd(date.year));
	wiringPiI2CWrite(description, I2CFlush);
}


void initPin(int pin) {
	pinMode(pin, INPUT);
	pullUpDnControl(pin, PUD_UP);
}
int main(int argc, char* argv[]) {
	if (argc < 2)
	{
		printf("Enter digits to display... or commands: now - present time, clock - loop program");
		return 0;
	}
	wiringPiSetup();
	initPin(UpButtonPin);
	initPin(DownButtonPin);
	initPin(ModeButtonPin);
	int fileDesc = wiringPiI2CSetup(I2CAdress);
	Date date;
	date.second = 31;
	date.minute = 46;
	date.hour = 19;
	date.weekDay = 4;
	date.day = 15;
	date.month = 6;
	date.year = 19;
	writeRTCDate(fileDesc,date);
	Date date2 = getRTCDate(fileDesc);
    printf("Up: %d\n", date2.second);
    printf("Up: %d\n", date2.minute);
    printf("Up: %d\n", date2.hour);
    printf("Up: %d\n", date2.weekDay);
    printf("Up: %d\n", date2.day);
    printf("Up: %d\n", date2.month);
    printf("Up: %d\n", date2.year);

	if (wiringPiSPISetupMode (0, 2000000, 2)) printf("SPI ok");
			else {printf("SPI NOT ok"); return 0;}
	//char* _argv;
	//String _stringToDisplay=argv[1];
	char _stringToDisplay[8];
	bool clock=0;

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
	} 
	while (clock);
	return 0;
}
