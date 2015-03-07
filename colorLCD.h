/*
 * colorLCD.h
 *
 *  Created on: Mar 19, 2012
 *      Author: RobG
 *
 * Reworked for 16bit SSI(SPI) on Stellaris Launchpad and RobG's 2.2" LCD with touch
 * on 3/5/13 by Timotet
 *
 *
 */

#ifndef COLORLCD_H_
#define COLORLCD_H_

#include "integer.h"
#include "inc/lm4f120h5qr.h"


#define LCD_HEIGHT 220 //Vertical
#define LCD_WIDTH 176  //Vertical
#define LCD_OFFSET_HEIGHT 0
#define LCD_OFFSET_WIDTH 0

/*
#define LCD_HEIGHT 176 // horizontal
#define LCD_WIDTH 220 // horizontal
*/

//// pins for rgb led on stellaris launchpad /////
#define redLed   GPIO_PIN_1              // all port F
#define blueLed  GPIO_PIN_2
#define greenLed GPIO_PIN_3

#define redOn  GPIOPinWrite(GPIO_PORTF_BASE, redLed|blueLed|greenLed, redLed)
#define redOff  GPIOPinWrite(GPIO_PORTF_BASE, redLed|blueLed|greenLed, 0x00)
#define toggleRed GPIO_PORTF_DATA_R ^= 0x02

#define blueOn     GPIOPinWrite(GPIO_PORTF_BASE, redLed|blueLed|greenLed, blueLed)
#define blueOff     GPIOPinWrite(GPIO_PORTF_BASE, redLed|blueLed|greenLed, 0x00)
#define toggleBlue GPIO_PORTF_DATA_R ^= 0x04

#define greenOn  GPIOPinWrite(GPIO_PORTF_BASE, redLed|blueLed|greenLed, greenLed)
#define greenOff  GPIOPinWrite(GPIO_PORTF_BASE, redLed|blueLed|greenLed, 0x00)
#define toggleGreen GPIO_PORTF_DATA_R ^= 0x08

// pins /// see ssiInit for hardware SSI gpio setup
#define LCD_DC_PIN   GPIO_PIN_5             // data / command on LCD   portE
#define LCD_DC_HI    GPIOPinWrite(GPIO_PORTE_BASE, LCD_DC_PIN , LCD_DC_PIN)
#define LCD_DC_LO    GPIOPinWrite(GPIO_PORTE_BASE, LCD_DC_PIN , 0x00)

// PENIRQ for touch
#define TSC_PENIRQ_PIN	    GPIO_PIN_5
#define TSC_PENIRQ_PIN_LO   GPIOPinWrite(GPIO_PORTA_BASE, TSC_PENIRQ_PIN |TSC_CS_PIN , 0x00)
#define TSC_PENIRQ_PIN_HI   GPIOPinWrite(GPIO_PORTA_BASE, TSC_PENIRQ_PIN |TSC_CS_PIN , TSC_PENIRQ_PIN)

// TSC_CS_PIN for touch
#define TSC_CS_PIN		GPIO_PIN_7
#define TSC_CS_PIN_LO   GPIOPinWrite(GPIO_PORTA_BASE, TSC_PENIRQ_PIN |TSC_CS_PIN , 0x00)
#define TSC_CS_PIN_HI   GPIOPinWrite(GPIO_PORTA_BASE, TSC_PENIRQ_PIN |TSC_CS_PIN , TSC_CS_PIN)


void writeData(UINT data);
void writeCommand(UINT data);

void initILI9225B();
void setGRAMILI9225B();
void gammaAdjustmentILI9225B();

void setColor8(UINT color);
void setColor16(UINT color);

void setArea(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd);
void clearScreen(UCHAR blackWhite);
void clearSome(UCHAR blackWhite);
void drawPixel(UCHAR x, UCHAR y);
void drawChar(UCHAR x, UCHAR y, UCHAR c);
void drawString(UCHAR x, UCHAR y, UCHAR *string);
void drawChar11_16(UCHAR x, UCHAR y, UCHAR c);
void drawString11_16(UCHAR x, UCHAR y, UCHAR *string);
void drawChar8_12(UCHAR x, UCHAR y, UCHAR c);
void drawString8_12(UCHAR x, UCHAR y, UCHAR *string);
void drawLine(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd);
void drawRect(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd);
void drawCircle(UCHAR x, UCHAR y, UCHAR radius);

void fillRect(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd);
void fillCircle(UCHAR x, UCHAR y, UCHAR radius);

void drawLogicLine(UCHAR x, UCHAR y, UCHAR length, UCHAR height, UCHAR * data);
void drawImage(UCHAR x, UCHAR y, UCHAR w, UCHAR h, UINT * data);
void drawImageMono(UCHAR x, UCHAR y, UCHAR w, UCHAR h, UCHAR * data);

// this is for ChaNs xprintf
void LCD_output_func(unsigned char c);

// ILI9225B specific
#define ILIGRAMADDRX	0x0020
#define ILIGRAMADDRY 	0x0021
#define ILIWRDATA   	0x0022
#define ILIGRAMHEA		0x0036
#define ILIGRAMHSA		0x0037
#define ILIGRAMVEA		0x0038
#define ILIGRAMVSA		0x0039
#define ILIVSCROLLSSA   0x0031
#define ILIVSCROLLSEA   0x0032
#define ILIVSCROLLSTEP  0x0033


#endif /* COLORLCD_H_ */

