//============================================================================
// Name        : DisplayNixie.cpp
// Author      : GRA&AFCH
// Version     : v1.3
// Copyright   : Free
// Description : Display time on shields NCS318 v1.1
//============================================================================

#define _VERSION "1.3"

#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <ctime>
#include <string.h>
#include <wiringPiI2C.h>
#include <softTone.h>
#include <softPwm.h>

using namespace std;
#define LEpin 3
#define UP_BUTTON_PIN 1
#define DOWN_BUTTON_PIN 4
#define MODE_BUTTON_PIN 5
#define BUZZER_PIN 0
#define I2CAdress 0x68
#define I2CFlush 0

#define DEBOUNCE_DELAY 150
#define TOTAL_DELAY 17

#define SECOND_REGISTER 0x0
#define MINUTE_REGISTER 0x1
#define HOUR_REGISTER 0x2
#define WEEK_REGISTER 0x3
#define DAY_REGISTER 0x4
#define MONTH_REGISTER 0x5
#define YEAR_REGISTER 0x6

//#define RED_LIGHT_PIN 28
//#define GREEN_LIGHT_PIN 27
//#define BLUE_LIGHT_PIN 29
#define MAX_POWER 100

#define UPPER_DOTS_MASK 0x80000000
#define LOWER_DOTS_MASK 0x40000000

#define LEFT_REPR_START 5
#define LEFT_BUFFER_START 0
#define RIGHT_REPR_START 2
#define RIGHT_BUFFER_START 4

uint16_t SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

int fileDesc;
int redLight = 100;
int greenLight = 0;
int blueLight = 0;
int lightCycle = 0;
bool dotState = 0;
int rotator = 0;

int bcdToDec(int val) {
	return ((val / 16  * 10) + (val % 16));
}

int decToBcd(int val) {
	return ((val / 10  * 16) + (val % 10));
}

tm addHourToDate(tm date) {
	date.tm_hour += 1;
	mktime(&date);
	return date;
}

tm addMinuteToDate(tm date) {
	date.tm_min += 1;
	mktime(&date);
	return date;
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
	date.tm_isdst = 0;
	return date;
}

void updateRTCHour(tm date) {
	wiringPiI2CWrite(fileDesc, I2CFlush);
	wiringPiI2CWriteReg8(fileDesc,HOUR_REGISTER,decToBcd(date.tm_hour));
	wiringPiI2CWrite(fileDesc, I2CFlush);
}

void updateRTCMinute(tm date) {
	wiringPiI2CWrite(fileDesc, I2CFlush);
	wiringPiI2CWriteReg8(fileDesc,MINUTE_REGISTER,decToBcd(date.tm_min));
	wiringPiI2CWriteReg8(fileDesc,HOUR_REGISTER,decToBcd(date.tm_hour));
	wiringPiI2CWrite(fileDesc, I2CFlush);
}
void resetRTCSecond() {
	wiringPiI2CWrite(fileDesc, I2CFlush);
	wiringPiI2CWriteReg8(fileDesc,SECOND_REGISTER, 0);
	wiringPiI2CWrite(fileDesc, I2CFlush);
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

void initPin(int pin) {
	pinMode(pin, INPUT);
	pullUpDnControl(pin, PUD_UP);

}

void funcMode(void) {
	static unsigned long modeTime = 0;
	if ((millis() - modeTime) > DEBOUNCE_DELAY) {
		puts("MODE button was pressed.");
		resetRTCSecond();
		modeTime = millis();
	}
}



uint32_t get32Rep(char * _stringToDisplay, int start) {
	uint32_t var32 = 0;

	var32= (SymbolArray[_stringToDisplay[start]-0x30])<<20;
	var32|=(SymbolArray[_stringToDisplay[start - 1]-0x30])<<10;
	var32|=(SymbolArray[_stringToDisplay[start - 2]-0x30]);
	return var32;
}

void fillBuffer(uint32_t var32, unsigned char * buffer, int start) {
	buffer[start] = var32>>24;
	buffer[start + 1] = var32>>16;
	buffer[start + 2] = var32>>8;
	buffer[start + 3] = var32;
}

void dotBlink()
{
	static unsigned int lastTimeBlink=millis();

	if ((millis() - lastTimeBlink) > 1000)
	{
		lastTimeBlink=millis();
		dotState = !dotState;
	}
}
/*
void rotateFireWorks() {
	int fireworks[] = {0,0,1,
					  -1,0,0,
			           0,1,0,
					   0,0,-1,
					   1,0,0,
					   0,-1,0
	};
	redLight += fireworks[rotator * 3];
	greenLight += fireworks[rotator * 3 + 1];
	blueLight += fireworks[rotator * 3 + 2];
	softPwmWrite(RED_LIGHT_PIN, redLight);
	softPwmWrite(GREEN_LIGHT_PIN, greenLight);
	softPwmWrite(BLUE_LIGHT_PIN, blueLight);
	lightCycle = lightCycle + 1;
	if (lightCycle == MAX_POWER) {
		rotator = rotator + 1;
		lightCycle  = 0;
	}
	if (rotator > 5)
		rotator = 0;
}*/

uint32_t addBlinkTo32Rep(uint32_t var) {
	if (dotState)
	{
		var &=~LOWER_DOTS_MASK;
		var &=~UPPER_DOTS_MASK;
	}
	else
	{
		var |=LOWER_DOTS_MASK;
		var |=UPPER_DOTS_MASK;
	}
	return var;
}


int main(int argc, char* argv[]) {
	printf("Nixie Clock v%s \n\r", _VERSION);
	wiringPiSetup();
	//softToneCreate (BUZZER_PIN);
	//softToneWrite(BUZZER_PIN, 1000);

	/*softPwmCreate(RED_LIGHT_PIN, 100, MAX_POWER);
	softPwmCreate(GREEN_LIGHT_PIN, 0, MAX_POWER);
	softPwmCreate(BLUE_LIGHT_PIN, 0, MAX_POWER);
	*/
	initPin(UP_BUTTON_PIN);
	initPin(DOWN_BUTTON_PIN);
	initPin(MODE_BUTTON_PIN);
	//wiringPiISR(MODE_BUTTON_PIN,INT_EDGE_RISING,&funcMode);
	fileDesc = wiringPiI2CSetup(I2CAdress);

	tm date = getRTCDate();
	time_t seconds = time(NULL);
	tm* timeinfo = localtime (&seconds);
	date.tm_mday = timeinfo->tm_mday;
	date.tm_wday = timeinfo->tm_wday;
	date.tm_mon =  timeinfo->tm_mon + 1;
	date.tm_year = timeinfo->tm_year - 100;
	writeRTCDate(date);

	if (wiringPiSPISetupMode (0, 2000000, 2)) {
		puts("SPI ok");
	}
	else {
		puts("SPI NOT ok");
		return 0;
	}

	long hourDelay = millis();
	long minuteDelay = hourDelay;
	long modeDelay=hourDelay;
	do {
		char _stringToDisplay[8];
		date = getRTCDate();
		char* format ="%H%M%S";
		strftime(_stringToDisplay, 8, format, &date);


		pinMode(LEpin, OUTPUT);
		dotBlink();

		unsigned char buff[8];

		uint32_t var32 = get32Rep(_stringToDisplay, LEFT_REPR_START);
		var32 = addBlinkTo32Rep(var32);
		fillBuffer(var32, buff , LEFT_BUFFER_START);

		var32 = get32Rep(_stringToDisplay, RIGHT_REPR_START);
		var32 = addBlinkTo32Rep(var32);
		fillBuffer(var32, buff , RIGHT_BUFFER_START);

		if (digitalRead(UP_BUTTON_PIN) == 0 && (millis() - hourDelay) > DEBOUNCE_DELAY) {
			updateRTCHour(addHourToDate(date));
			hourDelay = millis();
		}
		if (digitalRead(DOWN_BUTTON_PIN) == 0 && (millis() - minuteDelay) > DEBOUNCE_DELAY) {
			updateRTCMinute(addMinuteToDate(date));
			minuteDelay = millis();
		}

		if (digitalRead(MODE_BUTTON_PIN) == 0 && (millis() - modeDelay) > DEBOUNCE_DELAY) {
					resetRTCSecond();
					modeDelay = millis();
				}

		//rotateFireWorks();
		digitalWrite(LEpin, LOW);
		wiringPiSPIDataRW(0, buff, 8);
		digitalWrite(LEpin, HIGH);
		delay (TOTAL_DELAY);
	}
	while (true);
	return 0;
}
