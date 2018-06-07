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
#include <wiringPiI2C.h>


using namespace std;
#define LEpin 3
#define UP_BUTTON_PIN 1
#define DOWN_BUTTON_PIN 4
#define MODE_BUTTON_PIN 5
#define I2CAdress 0x68
#define I2CFlush 0


#define SECOND_REGISTER 0x0
#define MINUTE_REGISTER 0x1
#define HOUR_REGISTER 0x2
#define WEEK_REGISTER 0x3
#define DAY_REGISTER 0x4
#define MONTH_REGISTER 0x5
#define YEAR_REGISTER 0x6


#define SECOND_IN_SECONDS 1
#define MINUTE_IN_SECONDS 60
#define HOUR_IN_SECONDS 3600
#define DELAY_TIME 17
uint16_t SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

tm date;
int fileDesc;

int bcdToDec(int val) {
	return ((val / 16  * 10) + (val % 16));
}

int decToBcd(int val) {
	return ((val / 10  * 16) + (val % 10));
}

tm addSecondsToDate(tm date, int seconds) {
	time_t timer = mktime(&date);
	timer = timer + seconds;
	return *(localtime(&timer));
}

tm getRTCDate() {
	wiringPiI2CWrite(fileDesc, I2CFlush);
	tm date;
	date.tm_sec =  bcdToDec(wiringPiI2CReadReg8(fileDesc,SECOND_REGISTER));
	date.tm_min =  bcdToDec(wiringPiI2CReadReg8(fileDesc,MINUTE_REGISTER));
	date.tm_hour = bcdToDec(wiringPiI2CReadReg8(fileDesc,HOUR_REGISTER));
	date.tm_wday = bcdToDec(wiringPiI2CReadReg8(fileDesc,WEEK_REGISTER));
	date.tm_mday = bcdToDec(wiringPiI2CReadReg8(fileDesc,DAY_REGISTER));
	date.tm_mon =  bcdToDec(wiringPiI2CReadReg8(fileDesc,MONTH_REGISTER));
	date.tm_year = bcdToDec(wiringPiI2CReadReg8(fileDesc,YEAR_REGISTER));
	return date;
}


void writeRTCDate(tm date) {
	wiringPiI2CWrite(fileDesc, I2CFlush);
	wiringPiI2CWriteReg8(fileDesc,SECOND_REGISTER,decToBcd(date.tm_sec));
	wiringPiI2CWriteReg8(fileDesc,MINUTE_REGISTER,decToBcd(date.tm_min));
	wiringPiI2CWriteReg8(fileDesc,HOUR_REGISTER,decToBcd(date.tm_hour));
	wiringPiI2CWriteReg8(fileDesc,WEEK_REGISTER,decToBcd(date.tm_wday));
	wiringPiI2CWriteReg8(fileDesc,DAY_REGISTER,decToBcd(date.tm_mday));
	wiringPiI2CWriteReg8(fileDesc,MONTH_REGISTER,decToBcd(date.tm_mon));
	wiringPiI2CWriteReg8(fileDesc,YEAR_REGISTER,decToBcd(date.tm_year));
	wiringPiI2CWrite(fileDesc, I2CFlush);
}

void printTm (tm date) {
    printf("Seconds: %d\n", date.tm_sec);
    printf("Minutes: %d\n", date.tm_min);
    printf("Hours: %d\n", date.tm_hour);
    printf("Week day: %d\n", date.tm_wday);
    printf("Day: %d\n", date.tm_mday);
    printf("Month: %d\n", date.tm_mon);
    printf("Year: %d\n", date.tm_year);
}


void initPin(int pin, void (*function)(void)) {
	pinMode(pin, INPUT);
	pullUpDnControl(pin, PUD_UP);
    wiringPiISR(pin,INT_EDGE_RISING,function);
}

void funcUp(void) {
	static unsigned long upTime = 0;
	static unsigned long debounceDelay = 150;
	if ((millis() - upTime) > debounceDelay) {
		piLock(0);
		date = addSecondsToDate(getRTCDate(), HOUR_IN_SECONDS);
		writeRTCDate(date);
		piUnlock(0);
		upTime = millis();
	}
}

void funcDown(void) {
	static unsigned long downTime = 0;
	static unsigned long debounceDelay = 150;
	if ((millis() - downTime) > debounceDelay) {
		piLock(0);
		date = addSecondsToDate(getRTCDate(), MINUTE_IN_SECONDS);
		writeRTCDate(date);
		piUnlock(0);
		downTime = millis();
	}
}
void funcMode(void) {
	static unsigned long modeTime = 0;
	static unsigned long debounceDelay = 150;
	if ((millis() - modeTime) > debounceDelay) {
		piLock(0);
		date = addSecondsToDate(getRTCDate(), -date.tm_sec);
		writeRTCDate(date);
		piUnlock(0);
		modeTime = millis();
	}
}
uint32_t get32Rep(char * _stringToDisplay) {
	uint32_t Var32=0;

	Var32= (SymbolArray[_stringToDisplay[5]-0x30])<<20;
	Var32|=(SymbolArray[_stringToDisplay[4]-0x30])<<10;
	Var32|=(SymbolArray[_stringToDisplay[3]-0x30]);
	return Var32;
}
int main(int argc, char* argv[]) {
	if (argc < 2)
	{
		printf("Enter digits to display... or commands: now - present time, clock - loop program");
		return 0;
	}
	wiringPiSetup();
	initPin(UP_BUTTON_PIN,&funcUp);
	initPin(DOWN_BUTTON_PIN,&funcDown);
	initPin(MODE_BUTTON_PIN,&funcMode);
	fileDesc = wiringPiI2CSetup(I2CAdress);
	date = getRTCDate();
	if (wiringPiSPISetupMode (0, 2000000, 2)) printf("SPI ok");
			else {printf("SPI NOT ok"); return 0;}
	do {
		char _stringToDisplay[8];
		piLock(0);
		date = getRTCDate();
		piUnlock(0);
		char* format ="%H%M%S";
		strftime(_stringToDisplay, 8, format, &date);


		pinMode(LEpin, OUTPUT);

		uint32_t Var32 = get32Rep(_stringToDisplay);

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
		delay (DELAY_TIME);
	}
	while (true);
	return 0;
}
