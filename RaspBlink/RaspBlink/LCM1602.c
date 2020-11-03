/***********************************************************************************
*	{LCM1602_C}
*	LCM1602.c
*		by Austin Atteberry
*
*	PERFORMANCE:
*	Defines functions to interface with the LCM1602 I2C LCD panel
*
************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "LCM1602.h"

/* lcm1602ToggleEnable function
*  Writes a byte with the enable on, then off
*/
int lcm1602ToggleEnable(int fd, int byteVal) {

	/* Delay 500 µs */
	delayMicroseconds(500);

	/* Write with enable ON */
	int enableOn = wiringPiI2CWrite(fd, byteVal | LCD_ENABLE);

	/* Delay 500 µs */
	delayMicroseconds(500);

	/* Write with enable OFF */
	int enableOff = wiringPiI2CWrite(fd, byteVal & ~LCD_ENABLE);

	/* Delay 500 µs */
	delayMicroseconds(500);

	/* Check for error and return */
	if((enableOn != 0) || (enableOff != 0)) {
		return(-1);
	}
	else {
		return(0);
	}
}

/* lcm1602WriteByte function
*  Writes a byte to the LCD
*/
int lcm1602WriteByte(int fd, int byteVal, int mode, int backlight) {

	/* Create two 4-bit commands */
	int upperNibble = mode | (byteVal & 0xF0) | backlight;
	int lowerNibble = mode | ((byteVal<<4) & 0xF0) | backlight;

	/* Send upper nibble */
	int upperStatus = wiringPiI2CWrite(fd, upperNibble);
	int upperToggleStatus = lcm1602ToggleEnable(fd, upperNibble);

	/* Send lower nibble */
	int lowerStatus = wiringPiI2CWrite(fd, lowerNibble);
	int lowerToggleStatus = lcm1602ToggleEnable(fd, lowerNibble);

	/* Check for error and return */
	if((upperStatus != 0) || (upperToggleStatus != 0) || (lowerStatus != 0) || (lowerToggleStatus != 0)) {
		return(-1);
	}
	else {
		return(0);
	}
}

/* lcm1602WriteLine function
*  Writes a line of text to the LCD
*/
void lcm1602WriteLine(int fd, char message[], int line, int backlight) {

	/* Set a line to display the text */
	lcm1602WriteByte(fd, line, LCD_CMD, backlight);

	/* Display the text on the LCD */
	int length = strlen(message);

	if(length > 16) {
		for(int i=0; i<16; i++) {
			lcm1602WriteByte(fd, message[i], LCD_CHR, backlight);
		}
	}
	else {
		for(int i=0; i<length; i++) {
			lcm1602WriteByte(fd, message[i], LCD_CHR, backlight);
		}
	}
}

/* lcm1602Clear function
*  Clears the LCD screen
*/
void lcm1602Clear(int fd, int backlight) {

	/* Clear the display */
	lcm1602WriteByte(fd, 0x01, LCD_CMD, backlight);
}

/* lcm1602FunctionSet function
*  Sets the number of lines and font size
*/
int lcm1602FunctionSet(int fd, int numLines, int fontSize, int backlight) {

	/* Write byte and return */
	if(lcm1602WriteByte(fd, 0x20 | numLines | fontSize, LCD_CMD, backlight) != 0) {
		return(-1);
	}
	else {
		return(0);
	}
}

/* lcm1602DisplaySet function
*  Sets display, cursor, and cursor blink modes
*/
int lcm1602DisplaySet(int fd, int display, int cursor, int blink, int backlight) {

	/* Write byte and return */
	if(lcm1602WriteByte(fd, 0x08 | display | cursor | blink, LCD_CMD, backlight) != 0) {
		return(-1);
	}
	else {
		return(0);
	}
}

/* lcm1602EntryModeSet function
*  Sets the cursor or shift direction
*/
int lcm1602EntryModeSet(int fd, int direction, int displayShift, int backlight) {

	/* Write byte and return */
	if(lcm1602WriteByte(fd, 0x04 | direction | displayShift, LCD_CMD, backlight) != 0) {
		return(-1);
	}
	else {
		return(0);
	}
}

/* lcm1602Initialize function
*  Performs the initialization routine on the LCD
*/
int lcm1602Initialize(int fd, int backlight) {

	/* Write two initialization bytes to the LCD */
	int init_1 = lcm1602WriteByte(fd, 0x33, LCD_CMD, backlight);
	int init_2 = lcm1602WriteByte(fd, 0x32, LCD_CMD, backlight);

	/* Check for error, then return */
	if((init_1 != 0) || (init_2 != 0)) {
		return(-1);
	}
	else {
		return(0);
	}
}

/* lcm1602Setup function
*  Sets up the LCD
*/
int lcm1602Setup(const int devId, int direction, int displayShift, int display, int cursor, int blink, int numLines, int fontSize, int backlight) {

	/* Setup wiringPi I2C communication */
	int fd = wiringPiI2CSetup(devId);

	/* Initilize LCD */
	int initStatus = lcm1602Initialize(fd, backlight);

	/* Set cursor/shift direction */
	int entryStatus = lcm1602EntryModeSet(fd, direction, displayShift, backlight);

	/* Set Display, Cursor, and Cursor Blink ON/OFF */
	int displayStatus = lcm1602DisplaySet(fd, display, cursor, blink, backlight);

	/* Set number of lines and font size */
	int functionStatus = lcm1602FunctionSet(fd, numLines, fontSize, backlight);

	/* Clear the display */
	lcm1602Clear(fd, backlight);

	return(fd);
}