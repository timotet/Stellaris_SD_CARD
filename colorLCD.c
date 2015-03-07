/*
 * colorLCD.c
 *
 * Created on: Mar 20, 2012
 *      Author: RobG
 *
 * Reworked for 16 bit SSI(SPI) on 3/5/13 for Stellaris Launchpad and RobG's 2.2" LCD with touch
 * touch is not enabled for this example
 *
 * reworked by Timotet
 *
 *
 */

#include "colorLCD.h"
#include "font_5x7.h"
#include "font_11x16.h"
#include "font_8x12.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "delay.h"

UINT color16 = 0;
UINT bgcolor16 = 0;

UINT lcdY = 0; // for keeping track of the line Y position while using ChanNs xprintf on RobG's lcd
UINT lcdX = 0;    // same as above for X

//////////////////////
// color
//////////////////////
void setColor8(UINT color) {
	color16 = color;
}

void setColor16(UINT color) {
	color16 = color;
}

void setBackgroungColor8(UINT color) {
	bgcolor16 = color;
}

void setBackgroundColor16(UINT color) {
	bgcolor16 = color;
}

/////////////////
// drawing
/////////////////

void setArea(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd) {
	writeCommand(ILIGRAMHEA);
	writeData(xEnd + LCD_OFFSET_WIDTH);
	writeCommand(ILIGRAMHSA);
	writeData(xStart + LCD_OFFSET_WIDTH);
	writeCommand(ILIGRAMVEA);
	writeData(yEnd + LCD_OFFSET_HEIGHT);
	writeCommand(ILIGRAMVSA);
	writeData(yStart + LCD_OFFSET_HEIGHT);
	writeCommand(ILIGRAMADDRX);
	writeData(xStart + LCD_OFFSET_WIDTH);
	writeCommand(ILIGRAMADDRY);
	writeData(yStart + LCD_OFFSET_HEIGHT);
	writeCommand(ILIWRDATA);
}

/*
 //for horizontal
 char trueXStart, trueYStart, trueXEnd, trueYEnd; // horizontal
 trueXStart = yStart; // horizontal
 trueXEnd = yEnd; // horizontal
 trueYStart = xStart; // horizontal
 trueYEnd = xEnd; // horizontal

 writeCommand(ILIGRAMHEA);
 writeData(0);
 writeData(trueXEnd + LCD_OFFSET_WIDTH); // horizontal
 writeCommand(ILIGRAMHSA);
 writeData(0);
 writeData(trueXStart + LCD_OFFSET_WIDTH); // horizontal

 writeCommand(ILIGRAMVEA);
 writeData(0);
 writeData(trueYEnd + LCD_OFFSET_HEIGHT); // horizontal
 writeCommand(ILIGRAMVSA);
 writeData(0);
 writeData(trueYStart + LCD_OFFSET_HEIGHT); // horizontal

 writeCommand(ILIGRAMADDRX);
 writeData(0);
 writeData(trueXStart + LCD_OFFSET_WIDTH); // horizontal
 writeCommand(ILIGRAMADDRY);
 writeData(0);
 writeData(trueYStart + LCD_OFFSET_HEIGHT); // horizontal

 */

void clearScreen(UCHAR blackWhite) {
	setArea(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	const UINT totalPixels = LCD_WIDTH * LCD_HEIGHT;
	setBackgroundColor16(blackWhite ? 0x0000 : 0xFFFF);
	UINT c = 0;
	while (c < totalPixels) {
		writeData(bgcolor16);
		c++;
	}
}

//clear for 5x7 text
void clearSome(UCHAR blackWhite) {
	setArea(lcdX, lcdY, LCD_WIDTH, lcdY + 7);
	const UINT totalPixels = LCD_WIDTH  * (lcdY + 7);
	setBackgroundColor16(blackWhite ? 0x0000 : 0xFFFF);
	UINT c = 0;
	while (c < totalPixels) {
		writeData(bgcolor16);
		c++;
	}
}

void drawPixel(UCHAR x, UCHAR y) {
	setArea(x, y, x, y);
	writeData(color16);
}

////////////////
// 5x7 font - this function draws background pixels
////////////////
void drawChar(UCHAR x, UCHAR y, UCHAR c) {
	UCHAR col = 0;
	UCHAR row = 0;
	UCHAR bit = 0x01;
	UCHAR oc = c - 0x20;
	setArea(x, y, x + 4, y + 7); // if you want to fill column between chars, change x + 4 to x + 5

	while (row < 8) {
		while (col < 5) {
			if (font[oc][col] & bit) {
				//foreground
				writeData(color16);
			}

			else {
				//background
				writeData(bgcolor16);
			}
			col++;
		}
		// if you want to fill column between chars, writeData(bgColor) here
		col = 0;
		bit <<= 1;
		row++;
	}
}

void drawString(UCHAR x, UCHAR y, UCHAR *string) {
	UCHAR xs = x;
	while (*string) {
		drawChar(xs, y, *string++);
		xs += 6;
	}
}

////////////////
// 11x16 font - this function draws background pixels
////////////////
void drawChar11_16(UCHAR x, UCHAR y, UCHAR c) {
	UCHAR col = 0;
	UCHAR row = 0;
	UINT bit = 0x0001;
	UCHAR oc = c - 0x20;
	setArea(x, y, x + 10, y + 15);
	while (row < 16) {
		while (col < 11) {
			if (font_11x16[oc][col] & bit) {
				//foreground
				writeData(color16);
			}

			else {
				//background
				writeData(bgcolor16);
			}
			col++;
		}
		col = 0;
		bit <<= 1;
		row++;
	}
}

void drawString11_16(UCHAR x, UCHAR y, UCHAR *string) {
	UCHAR xs = x;
	while (*string) {
		drawChar11_16(xs, y, *string++);
		xs += 12;
	}
}

////////////////
// 8x12 font - this function draws background pixels
////////////////
void drawChar8_12(UCHAR x, UCHAR y, UCHAR c) {
	UCHAR col = 0;
	UCHAR row = 0;
	UCHAR bit = 0x80;
	UCHAR oc = c - 0x20;
	setArea(x, y, x + 7, y + 11);
	while (row < 12) {
		while (col < 8) {
			if (font_8x12[oc][row] & bit) {
				//foreground
				writeData(color16);
			}

			else {
				//background
				writeData(bgcolor16);
			}
			bit >>= 1;
			col++;
		}
		bit = 0x80;
		col = 0;
		row++;
	}

}

void drawString8_12(UCHAR x, UCHAR y, UCHAR *string) {
	UCHAR xs = x;
	while (*string) {
		drawChar8_12(xs, y, *string++);
		xs += 8;
	}
}

////////////////////////
// images
////////////////////////
//data is 16 bit color
void drawImage(UCHAR x, UCHAR y, UCHAR w, UCHAR h, UINT * data) {
	setArea(x, y, x + w - 1, y + h - 1);

	int i = 0;
	for (i = 0; i < w * h; i++) {
		writeData(data[i]);
	}
}

// each bit represents color, fg and bg colors are used, ?how about 0 as a mask?
void drawImageMono(UCHAR x, UCHAR y, UCHAR w, UCHAR h, UCHAR * data) {

	//need todo

}

////////////////////////
// shapes
////////////////////////

void drawLogicLine(UCHAR x, UCHAR y, UCHAR length, UCHAR height,
		UCHAR * data) {
	UCHAR last = (*data & 0x80);
	UCHAR counter = 0;
	while (counter < length) {
		UCHAR bitCounter = 0;
		UCHAR byte = *data;
		while (bitCounter < 8 && counter < length) {
			if (last == (byte & 0x80)) {
				//draw pixel
				UCHAR h = (byte & 0x80) ? (height + y) : y;
				drawPixel(x + counter, h);
			} else {
				// draw line
				if (byte & 0x80) {
					drawLine(x + counter - 1, y, x + counter, y + height);
				} else {
					drawLine(x + counter - 1, y + height, x + counter, y);
				}
			}
			last = byte & 0x80;
			byte <<= 1;
			bitCounter++;
			counter++;
		}
		*data++;
	}
}

void drawLine(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd) {

	UCHAR x0, x1, y0, y1;
	UCHAR d = 0;

	// handle direction
	if (yStart > yEnd) {
		y0 = yEnd;
		y1 = yStart;
	} else {
		y1 = yEnd;
		y0 = yStart;
	}

	if (xStart > xEnd) {
		x0 = xEnd;
		x1 = xStart;
	} else {
		x1 = xEnd;
		x0 = xStart;
	}

	// check if horizontal
	if (y0 == y1) {
		d = x1 - x0 + 1;
		setArea(x0, y0, x1, y1);
		while (d-- > 0) {
			writeData(color16);
		}
	}

	else if (x0 == x1) { // check if vertical
		d = y1 - y0 + 1;
		setArea(x0, y0, x1, y1);
		while (d-- > 0) {
			writeData(color16);
		}

	} else { // angled
		char dx, dy;
		int sx, sy;

		if (xStart < xEnd) {
			sx = 1;
			dx = xEnd - xStart;
		} else {
			sx = -1;
			dx = xStart - xEnd;
		}

		if (yStart < yEnd) {
			sy = 1;
			dy = yEnd - yStart;
		} else {
			sy = -1;
			dy = yStart - yEnd;
		}

		int e1 = dx - dy;
		int e2;

		while (1) {
			drawPixel(xStart, yStart);
			if (xStart == xEnd && yStart == yEnd)
				break;
			e2 = 2 * e1;
			if (e2 > -dy) {
				e1 = e1 - dy;
				xStart = xStart + sx;
			}
			if (e2 < dx) {
				e1 = e1 + dx;
				yStart = yStart + sy;
			}
		}
	}
}

void drawRect(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd) {

	drawLine(xStart, yStart, xEnd, yStart);
	drawLine(xStart, yEnd, xEnd, yEnd);
	drawLine(xStart, yStart, xStart, yEnd);
	drawLine(xEnd, yStart, xEnd, yEnd);
}

void drawCircle(UCHAR x, UCHAR y, UCHAR radius) {

	int dx = radius;
	int dy = 0;
	int xChange = 1 - 2 * radius;
	int yChange = 1;
	int radiusError = 0;
	while (dx >= dy) {
		drawPixel(x + dx, y + dy);
		drawPixel(x - dx, y + dy);
		drawPixel(x - dx, y - dy);
		drawPixel(x + dx, y - dy);
		drawPixel(x + dy, y + dx);
		drawPixel(x - dy, y + dx);
		drawPixel(x - dy, y - dx);
		drawPixel(x + dy, y - dx);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 2;
		}
	}
}

/////////////////////////
// fill
/////////////////////////

void fillRect(UCHAR xStart, UCHAR yStart, UCHAR xEnd, UCHAR yEnd) {

	setArea(xStart, yStart, xEnd, yEnd);
	UINT total = (xEnd - xStart + 1) * (yEnd - yStart + 1);
	UINT c = 0;
	while (c < total) {
		writeData(color16);
		c++;
	}
}

void fillCircle(UCHAR x, UCHAR y, UCHAR radius) {

	int dx = radius;
	int dy = 0;
	int xChange = 1 - 2 * radius;
	int yChange = 1;
	int radiusError = 0;
	while (dx >= dy) {
		drawLine(x + dy, y + dx, x - dy, y + dx);
		drawLine(x - dy, y - dx, x + dy, y - dx);
		drawLine(x - dx, y + dy, x + dx, y + dy);
		drawLine(x - dx, y - dy, x + dx, y - dy);
		dy++;
		radiusError += yChange;
		yChange += 2;
		if (2 * radiusError + xChange > 0) {
			dx--;
			radiusError += xChange;
			xChange += 2;
		}
	}
}

// initialize ILI9225B
void initILI9225B() {

	writeCommand(0x0028);
	delay(20);
	writeCommand(0x0001);
	writeData(0x011C); // set SS and NL bit
	writeCommand(0x0002);
	writeData(0x0100); // set 1 line inversion
	writeCommand(0x0003);
	writeData(0x1030); // set GRAM write direction and BGR=1.//1030
	writeCommand(0x0008);
	writeData(0x0808); // set BP and FP
	writeCommand(0x000C);
	writeData(0x0000); // RGB interface setting 0x0110 for RGB 18Bit and 0111for RGB16Bit
	writeCommand(0x000F);
	writeData(0x0b01); // Set frame rate//0b01
	delay(5);
	writeCommand(0x0010);
	writeData(0x0a00); // Set SAP,DSTB,STB//0800
	writeCommand(0x11);
	writeData(0x1038); // Set APON,PON,AON,VCI1EN,VC
	delay(5);
	writeCommand(0x0012);
	writeData(0x1121); // Internal reference voltage= Vci;
	writeCommand(0x0013);
	writeData(0x0063); // Set GVDD
	writeCommand(0x14);
	writeData(0x4b44); // Set VCOMH/VCOML voltage
	writeCommand(ILIGRAMADDRX);
	writeData(0x0000);
	writeCommand(ILIGRAMADDRY);
	writeData(0x0000);
	setGRAMILI9225B();
	gammaAdjustmentILI9225B();

	/*
    // vertical scroll
    writeCommand(ILIVSCROLLSSA);   // set vertical scroll start address
    writeData(0x00D9);
    writeCommand(ILIVSCROLLSEA);   // set vertical scroll  end address
    writeData(0x0000);
    writeCommand(ILIVSCROLLSTEP); // set vertical scroll step size to 7 steps
    writeData(0x0111);
    */

	//TODO add horizontal/vertical settings
	delay(5);
	writeCommand(0x0007);
	writeData(0x1017);
}

////////////////////////////////////
// gamma, lut, and other inits
////////////////////////////////////

void gammaAdjustmentILI9225B() {

	const UINT data[] = { 0x0003, 0x0900, 0x0d05, 0x0900, 0x0407, 0x0502,
			0x0000, 0x0005, 0x1700, 0x001f };
	const UINT cmd = 0x0050;
	UINT c = 0;
	while (c < 10) {
		writeCommand(cmd + c);
		writeData(data[c]);
		c++;
	}
}

void setGRAMILI9225B() {
	const UINT data[] = { 0x0000, 0x00DB, 0x0000, 0x0000, 0x00DB, 0x0000,
			0x00AF, 0x0000, 0x00DB, 0x0000 };
	const UINT cmd = 0x0030;
	UINT c = 0;
	while (c < 10) {
		writeCommand(cmd + c);
		writeData(data[c]);
		c++;
	}
}

////////////////////////////////////
// write Data/Command
////////////////////////////////////
void writeData(UINT data) {

	LCD_DC_HI;
	SSIDataPut(SSI2_BASE, data);

	// Wait until SSI2 is done transferring all the data in the transmit FIFO.
	while (SSIBusy(SSI2_BASE)) {
	}
}

void writeCommand(UINT data) {

	LCD_DC_LO;
	SSIDataPut(SSI2_BASE, data);

	// Wait until SSI2 is done transferring all the data in the transmit FIFO.
	while (SSIBusy(SSI2_BASE)) {
	}
}


////////////////////////////////////
////// Write a 5x7 character to the output device
////// this works with ChaNs xprintf
//////////////////////////////////
void LCD_output_func(UCHAR c) { /* Character to be output */
	UCHAR col = 0;
	UCHAR row = 0;
	UCHAR bit = 0x01;
	UCHAR oc = c - 0x20;

	setArea(lcdX, lcdY, lcdX + 4, lcdY + 7);


	if(lcdY == 210 && c == '\n'){  // end of screen reached start over
	    lcdX = 0;
	    lcdY = 210;
	    //clearScreen(1);
	}else if (c == '\n') {
		lcdY += 7;
		lcdX = 0;
	} else if (c == '\r') {
		lcdX = 0;

	} else {
		while (row < 8) {
			while (col < 5) {
				if (font[oc][col] & bit) {
					//foreground
					writeData(color16);
				}

				else {
					//background
					writeData(bgcolor16);
				}
				col++;
			}
			// if you want to fill column between chars, writeData(bgColor) here
			col = 0;
			bit <<= 1;
			row++;
		}

		lcdX += 5;

		if (lcdX == 175) {
			lcdX = 0;
			lcdY += 7;
	   }
   }
}
