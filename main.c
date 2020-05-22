/*
 * CE865 Assignment 1
 * REACTION TIMER
 * Student: Dwivedi Rohan Yogesh
 * Registration No: 1900929
 */

#include <atmel/pit.h>
#include <atmel/aic.h>
#include <board/LED-lib.h>
#include <board/Button-lib.h>
#include <config/AT91SAM7S256.h>
#include <board/SevenSegmentDisplay.h>


#define SECONDS			DISPLAY1
#define RIGHT_DISPLAY	DISPLAY2
#define MIDDLE_DISPLAY	DISPLAY3
#define LEFT_DISPLAY	DISPLAY4
#define PIT_PERIOD      1000 				/* set interrupt period at 1000 milliseconds */
#define LED_CLOCK_LINE  AT91C_PIO_PA24		/* pin 24 is Clock line  */
#define LED_DATA_LINE   AT91C_PIO_PA25      /* pin 25 is data line  */

short Digit[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67 }; /* Hex for {0, 1, 2, ...}      */
short Digit_[] = { 0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,0x87,0xff,0xe7 };			/* Hex for (0. , 1. , 2. ,...} */
int loopcount = 0;
short milli = 0, seconds = 0, min = 0, hour = 0;
short led;
int LEDpos = 8;

void delay_ms(short ms)
{
	/*  provides delay for given milliseconds
	 *  expected arguments: time(ms) for the delay required */

	volatile short loop;
	while (ms-- > 0) for (loop = 0; loop < 2100; loop++);
}

void clear_all_LED(void)
{
	/* sets all LED's off */

	AT91C_BASE_PIOA->PIO_CODR = LED_CLOCK_LINE; //pin 24
	AT91C_BASE_PIOA->PIO_SODR = LED_DATA_LINE;  //pin 25

	for (led = 0; led < 16; led++)

	{
		AT91C_BASE_PIOA->PIO_SODR = LED_CLOCK_LINE; // pin 24
		AT91C_BASE_PIOA->PIO_CODR = LED_CLOCK_LINE; // pin 24
	}
}

void Blink_First_LED(void)
{
	/* Blinks the first led green and red alternatively
	 * until button 1 is pressed*/

	SetLEDcolor(LED1, RED);
	delay_ms(200);
	SetLEDcolor(LED1, OFF);

	SetLEDcolor(LED1, GREEN);
	delay_ms(200);
	SetLEDcolor(LED1, OFF);
}

void ISR_System(void)
{
	/* logic for running the timer*/

	PIT_GetPIVR();

	if (++milli == 10)
	{
		milli = 0;
		++seconds;

		if (seconds == 10)
		{
			seconds = 0;
			++min;

			if (min == 10)
			{
				min = 0;
				++hour;

				if (hour == 10)
					hour = 0;
			}
		}

	}
}

void reset_display(void)
{
	/* Resets the value of timer as 0.000 on the LCD display*/

	Configure7SegmentDisplay();

	Set7SegmentDisplayValue(SECONDS, Digit[0]);
	Set7SegmentDisplayValue(RIGHT_DISPLAY, Digit[0]);
	Set7SegmentDisplayValue(MIDDLE_DISPLAY, Digit[0]);
	Set7SegmentDisplayValue(LEFT_DISPLAY, Digit_[0]);

	PIT_Init(PIT_PERIOD, BOARD_MCK / 1000000);
	AIC_DisableIT(AT91C_ID_SYS);
	AIC_ConfigureIT(AT91C_ID_SYS, 0, ISR_System);
	AIC_EnableIT(AT91C_ID_SYS);
	PIT_EnableIT();
	PIT_Enable();
}

void begin_countdown(void)
{
	/* starts led count-down and checks for button interrupts, if any interrupt pauses the led count-down*/

	SetAllLEDs(LEDsAllRed);
	LEDpos = 8;
	while (LEDpos >= 0)
	{
		if (IsButtonPressed(BUTTON1) || IsButtonPressed(BUTTON2) || IsButtonPressed(BUTTON3) || IsButtonPressed(BUTTON4) || IsButtonPressed(BUTTON5) || IsButtonPressed(BUTTON6) || IsButtonPressed(BUTTON7) || IsButtonPressed(BUTTON8))
			delay_ms(100);
		else
		{
			SetLEDcolor(LEDpos, OFF);
			LEDpos--;
			delay_ms(1000);
		}

	}
}

int setRandom(void)
{
	/* sets a random LED on, also returns the random no for the button expected to be pressed*/

	int r = rand() % 7 + 1;
	SetLEDcolor(r, GREEN);
	return r;

}

int main()
{
	clear_all_LED();
	reset_display();

	while (1)
	{

		while (IsButtonReleased(BUTTON1))
		{
			/* Exits when Button 1 is pressed */

			Blink_First_LED();
		}

		reset_display();
		begin_countdown();

		int random = setRandom();
		milli = 0;seconds = 0;min = 0;hour = 0;

		while (IsButtonReleased(random))

		{
			/* exits when the random button assigned is pressed */

			if (milli == 9 && seconds == 9 && min == 9 && hour == 9)
			{
				/* stops the code from incorrectly displaying 9997 instead of 9999
				 * due to interrupt not reached before the loop exits
				 * and therefore doesn't update the display on time.
				 *
				 * exits when the timer reaches 9.999 */

				Set7SegmentDisplayValue(SECONDS, Digit[9]);
				Set7SegmentDisplayValue(RIGHT_DISPLAY, Digit[9]);
				Set7SegmentDisplayValue(MIDDLE_DISPLAY, Digit[9]);
				Set7SegmentDisplayValue(LEFT_DISPLAY, Digit_[9]);
				break;
			}

			else
			{
				Set7SegmentDisplayValue(SECONDS, Digit[milli]);
				Set7SegmentDisplayValue(RIGHT_DISPLAY, Digit[seconds]);
				Set7SegmentDisplayValue(MIDDLE_DISPLAY, Digit[min]);
				Set7SegmentDisplayValue(LEFT_DISPLAY, Digit_[hour]);
			}
		}

		SetLEDcolor(random, OFF);
	}
}
