//============================================================================
// Name        : DisplayNixie.cpp
// Author      : GRA&AFCH @ Leon Shaner
// Version     : v2.3.1
// Copyright   : Free
// Description : Display time on shields NCS314 v2.x or NCS312
//============================================================================

#define _VERSION "2.3.1 SHANER"

#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <ctime>
#include <string.h>
#include <wiringPiI2C.h>
#include <softTone.h>
#include <softPwm.h>
#include <signal.h>

using namespace std;
#define R5222_PIN 22
bool HV5222;
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

#define RED_LIGHT_PIN 28
#define GREEN_LIGHT_PIN 27
#define BLUE_LIGHT_PIN 29
#define MAX_POWER 100
#define INIT_CYCLE_FREQ 5
#define INIT_CYCLE_BUMP 5

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
int cycleFreq = INIT_CYCLE_FREQ;

// Set initial fireworks mode true=on; false=off
bool doFireworks = false;

// Set default clock mode
// NOTE:  true means rely on system to keep time (e.g. NTP assisted for accuracy).
bool useSystemRTC = true;

// Set the hour mode
// Set use12hour = true for hours 0-12 and 1-11 (e.g. a.m./p.m. implied)
// Set use12hour = false for hours 0-23
bool use12hour = true;

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
	if (use12hour)
	{
		date.tm_hour = bcdToDec(wiringPiI2CReadReg8(fileDesc,HOUR_REGISTER));
		if (date.tm_hour > 12)
			date.tm_hour -= 12;
	}
	else
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

void resetFireWorks() {
	redLight = 0;
	greenLight = 0;
	blueLight = 0; 
	softPwmWrite(RED_LIGHT_PIN, redLight);
	softPwmWrite(GREEN_LIGHT_PIN, greenLight);
	softPwmWrite(BLUE_LIGHT_PIN, blueLight);
}

void initFireWorks() {
	redLight = 5;
	greenLight = 0;
	blueLight = 0; 
	softPwmWrite(RED_LIGHT_PIN, redLight);
	softPwmWrite(GREEN_LIGHT_PIN, greenLight);
	softPwmWrite(BLUE_LIGHT_PIN, blueLight);
}

void funcMode(void) {
	static unsigned long modeTime = 0;
	if ((millis() - modeTime) > DEBOUNCE_DELAY) {
		puts("MODE button was pressed.");
// Mode Switch toggles Fireworks on/off
		doFireworks = !doFireworks;
		if (!doFireworks)
			resetFireWorks();
		else
			initFireWorks();
		modeTime = millis();
	}
}

void funcUp(void) {
	static unsigned long buttonTime = 0;
	if ((millis() - buttonTime) > DEBOUNCE_DELAY) {
// UP button speeds up Fireworks
		cycleFreq = ((cycleFreq + INIT_CYCLE_BUMP) / INIT_CYCLE_BUMP ) * INIT_CYCLE_BUMP;
		if (cycleFreq >= MAX_POWER / 2 )
		{
			cycleFreq = MAX_POWER / 2;
			printf("Up button was pressed. Already at fastest speed, %d; ignoring.\n", cycleFreq);
		}
		else
			printf("UP button was pressed. Frequency=%d\n", cycleFreq);
		buttonTime = millis();
	}
}

void funcDown(void) {
	static unsigned long buttonTime = 0;
	if ((millis() - buttonTime) > DEBOUNCE_DELAY) {
// Down button slows down Fireworks
		cycleFreq = ((cycleFreq - INIT_CYCLE_BUMP) / INIT_CYCLE_BUMP ) * INIT_CYCLE_BUMP;
		if (cycleFreq <= 1 )
		{
			cycleFreq = 1;
			printf("Down button was pressed. Already at slowest speed, %d; ignoring.\n", cycleFreq);
		}
		else
			printf("Down button was pressed. Frequency=%d\n", cycleFreq);
	}
	buttonTime = millis();
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

	if ((millis() - lastTimeBlink) >= 1000)
	{
		lastTimeBlink=millis();
		dotState = !dotState;
	}
}

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
	lightCycle = lightCycle + cycleFreq;
	if (lightCycle >= MAX_POWER) {
		rotator = rotator + 1;
		lightCycle  = 0;
	}
	if (rotator > 5)
	rotator = 0;
}


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

// Interrupt handler to turn off Nixie upon SIGINT(2), SIGQUIT(3), SIGTERM(15), but not SIGKILL(9)
void signal_handler (int sig_received)
{
	printf("Received Signal %d; Exiting.\n", sig_received);
	digitalWrite(RED_LIGHT_PIN, LOW);
	digitalWrite(GREEN_LIGHT_PIN, LOW);
	digitalWrite(BLUE_LIGHT_PIN, LOW);
	digitalWrite(LEpin, LOW);
	exit(sig_received);
}

//uint64_t* reverseBit(uint64_t num);
uint64_t reverseBit(uint64_t num);

int main(int argc, char* argv[]) {

	// Setup signal handlers
  	signal(SIGINT, signal_handler);
  	signal(SIGQUIT, signal_handler);
  	signal(SIGTERM, signal_handler);

	printf("Nixie Clock v%s \n\r", _VERSION);

	wiringPiSetup();

	//softToneCreate (BUZZER_PIN);
	//softToneWrite(BUZZER_PIN, 1000);

	softPwmCreate(GREEN_LIGHT_PIN, 0, MAX_POWER);
	softPwmCreate(BLUE_LIGHT_PIN, 0, MAX_POWER);


// Crude command-line argument handling
	if (argc >= 2)
	{
		int curr_arg = 1;
		while (curr_arg < argc)
		{
			if (!strcmp(argv[curr_arg],"nosysclock"))
				useSystemRTC = false;
			else if (!strcmp(argv[curr_arg],"24hour"))
				use12hour = false;
			else if (!strcmp(argv[curr_arg],"fireworks"))
				doFireworks = true;
			else
			{
				printf("ERROR: %s Unknown argument, \"%s\" on command line.\n\n", argv[0], argv[1]);
				printf("USAGE: %s            -- Use system clock in 12-hour mode.\n", argv[0]);
				printf("       %s nosysclock -- use Nixie clock (e.g. not NTP assisted).\n", argv[0]);
				printf("       %s 24hour     -- use 24-hour mode.\n", argv[0]);
				printf("       %s fireworks  -- enable fireworks.\n", argv[0]);
				puts("\nNOTE:  Any combination/order of above arguments is allowed.\n");
				exit(10);
			}
			curr_arg += 1;
		}
	}
		 

// Tell the user the RTC mode
	if (useSystemRTC)
		puts("Using system RTC (eg. NTP assisted accuracy).");
	else
		puts("Using Nixie embedded RTC (e.g. not NTP assisted).");

// Tell the user the hour mode
	if (use12hour)
		puts("Using 12-hour display (implied a.m./p.m.).");
	else
		puts("Using 24-hour display.");

// Tell the user the fireworks mode
	if (doFireworks)
		puts("Fireworks ENABLED at start.");
	else
		puts("Fireworks DISABLED at start.");

// Further setup...
	initPin(UP_BUTTON_PIN);
	initPin(DOWN_BUTTON_PIN);
	initPin(MODE_BUTTON_PIN);

// Initial setup for multi-color LED's based on default doFirewworks boolean
	if (doFireworks)
		softPwmCreate(RED_LIGHT_PIN, 100, MAX_POWER);
	else
	{
		softPwmCreate(RED_LIGHT_PIN, 0, MAX_POWER);
		resetFireWorks();
	}

// Mode Switch toggles Fireworks on/off
	wiringPiISR(MODE_BUTTON_PIN,INT_EDGE_RISING,&funcMode);

// Open the Nixie device
	fileDesc = wiringPiI2CSetup(I2CAdress);

// Further date setup
	tm date = getRTCDate();
	time_t seconds = time(NULL);
	tm* timeinfo = localtime (&seconds);

// Tell the user the SPI status
	if (wiringPiSPISetupMode (0, 2000000, 2)) {
		puts("SPI ok");
	}
	else {
		puts("SPI NOT ok");
		return 0;
	}

	pinMode(R5222_PIN, INPUT);
	pullUpDnControl(R5222_PIN, PUD_UP);
	HV5222=!digitalRead(R5222_PIN);
	if (HV5222) puts("R52222 resistor detected. HV5222 algorithm is used.");
	uint64_t reverseBuffValue;

// Loop forever displaying the time
	long buttonDelay = millis();

	do {
		char _stringToDisplay[8];


// NOTE:  RTC relies on system to keep time (e.g. NTP assisted for accuracy).
		if (useSystemRTC)
		{
			seconds = time(NULL);
			timeinfo = localtime (&seconds);
			date.tm_mday = timeinfo->tm_mday;
			date.tm_wday = timeinfo->tm_wday;
			date.tm_mon =  timeinfo->tm_mon + 1;
			date.tm_year = timeinfo->tm_year - 100;
			writeRTCDate(*timeinfo);
		}

// NOTE:  RTC relies on Nixie to keep time (e.g. no NTP).
		date = getRTCDate();

		char* format = (char*) "%H%M%S";
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



		if (doFireworks)
		{
			// Handle Fireworks speed UP / Down
			if (digitalRead(UP_BUTTON_PIN) == 0 && (millis() - buttonDelay) > DEBOUNCE_DELAY) {
				funcUp();
				initFireWorks();
				buttonDelay = millis();
			}
			if (digitalRead(DOWN_BUTTON_PIN) == 0 && (millis() - buttonDelay) > DEBOUNCE_DELAY) {
				funcDown();
				initFireWorks();
				buttonDelay = millis();
			}
			rotateFireWorks();
		}

		digitalWrite(LEpin, LOW);

		if (HV5222)
		{
			reverseBuffValue=reverseBit(*(uint64_t*)buff);
			buff[4]=reverseBuffValue;
			buff[5]=reverseBuffValue>>8;
			buff[6]=reverseBuffValue>>16;
			buff[7]=reverseBuffValue>>24;
			buff[0]=reverseBuffValue>>32;
			buff[1]=reverseBuffValue>>40;
			buff[2]=reverseBuffValue>>48;
			buff[3]=reverseBuffValue>>56;
		}

		wiringPiSPIDataRW(0, buff, 8);
		digitalWrite(LEpin, HIGH);
		delay (TOTAL_DELAY);
	}
	while (true);
	return 0;
}

//uint64_t* reverseBit(uint64_t num)
uint64_t reverseBit(uint64_t num)
{
	uint64_t reverse_num=0;
	int i;
	for (i=0; i<64; i++)
	{
		if ((num & ((uint64_t)1<<i)))
			reverse_num = reverse_num | ((uint64_t)1<<(63-i));
	}
	return reverse_num;
}
