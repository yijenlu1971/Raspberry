/***********************************************************************************
*	{LCM1602_H}
*	LCM1602.h
*		by Austin Atteberry
*
*	DEFINITIONS:
*	Definitions and function prototypes for the LCM1602 LCD
*
************************************************************************************/



#ifndef LCD_LINE_1
#define LCD_LINE_1 0x80
#endif

#ifndef LCD_LINE_2
#define LCD_LINE_2 0xC0
#endif

#ifndef LCD_LINE_3
#define LCD_LINE_3 0x94
#endif

#ifndef LCD_LINE_4
#define LCD_LINE_4 0xD4
#endif

#ifndef LCD_ENABLE
#define LCD_ENABLE 0x04
#endif

#ifndef LCD_CMD
#define LCD_CMD 0x00
#endif

#ifndef LCD_CHR
#define LCD_CHR 0x01
#endif

#ifndef CURSOR_RIGHT
#define CURSOR_RIGHT 0x02
#endif

#ifndef CURSOR_LEFT
#define CURSOR_LEFT 0x00
#endif

#ifndef SHIFT_ON
#define SHIFT_ON 0x01
#endif

#ifndef SHIFT_OFF
#define SHIFT_OFF 0x00
#endif

#ifndef SHIFT_RIGHT
#define SHIFT_RIGHT 0x00
#endif

#ifndef SHIFT_LEFT
#define SHIFT_LEFT 0x01
#endif

#ifndef DISPLAY_ON
#define DISPLAY_ON 0x04
#endif

#ifndef DISPLAY_OFF
#define DISPLAY_OFF 0x00
#endif

#ifndef CURSOR_ON
#define CURSOR_ON 0x02
#endif

#ifndef CURSOR_OFF
#define CURSOR_OFF 0x00
#endif

#ifndef CURSOR_BLINK_ON
#define CURSOR_BLINK_ON 0x01
#endif

#ifndef CURSOR_BLINK_OFF
#define CURSOR_BLINK_OFF 0x00
#endif

#ifndef ONE_LINE
#define ONE_LINE 0x00
#endif

#ifndef TWO_LINE
#define TWO_LINE 0x08
#endif

#ifndef SMALL_FONT
#define SMALL_FONT 0x00
#endif

#ifndef LARGE_FONT
#define LARGE_FONT 0x04
#endif

#ifndef LCD_BACKLIGHT_ON
#define LCD_BACKLIGHT_ON 0x08
#endif

#ifndef LCD_BACKLIGHT_OFF
#define LCD_BACKLIGHT_OFF 0x00
#endif

#ifndef _LCM1602_H
#define _LCM1602_H

/* Run lcm1602Setup before using any of the other LCM1602 functions
*  
*  Description of parameters:
*		devId - the LCD's device id (usually 0x27)
*		direction - the direction the cursor will move
*					if displayShift=SHIFT_OFF use CURSOR_RIGHT or CURSOR_LEFT
*					if displayShift=SHIFT_ON use SHIFT_RIGHT or SHIFT_LEFT
*		displayShift - use SHIFT_ON/SHIFT_OFF to enable/disable the display shift
*		display - use DISPLAY_ON/DISPLAY_OFF to turn the display on or off
*		cursor - use CURSOR_ON/CURSOR_OFF to show/hide the cursor
*		blink - use CURSOR_BLINK_ON/CURSOR_BLINK_OFF to turn cursor blinking on/off
*		numLines - use ONE_LINE/TWO_LINE to set one or two line mode
*		fontSize - use SMALL_FONT/LARGE_FONT to set font size small/large
*		backlight - use LCD_BACKLIGHT_ON/LCD_BACKLIGHT_OFF to turn the backlight on/off
*/
extern int lcm1602Setup(const int devId, int direction, int displayShift, int display, int cursor, int blink, int numLines, int fontSize, int backlight);

extern void lcm1602Clear(int fd, int backlight);	// Clears the LCD
extern void lcm1602WriteLine(int fd, char message[], int line, int backlight);	// Writes a line of text to the LCD

#endif
