/*
 * sLcdSpi_S1D15G10.cpp
 *
 *  Created on: 2018/01/06
 *      Author: ksekimoto
 */

#include <Arduino.h>
#include <mruby.h>
#include <mruby/string.h>
#include "../wrbb.h"
#include "sLcdSpi.h"
#include "S1D15G10.h"

/* ********************************************************************* */
/* S1D15G10 */
/* ********************************************************************* */

static void S1D15G10_Reset()
{
	delay_ms(100);
	digitalWrite(_resetPin, LOW);
	delay_ms(1000);
	digitalWrite(_resetPin, HIGH);
	delay_ms(100);
}

static void S1D15G10_Initialize()
{
	pinMode(_resetPin, OUTPUT);
	SPISW_Initialize();
	S1D15G10_Reset();
	digitalWrite(_csPin, LOW);
	digitalWrite(_doutPin, LOW);
	digitalWrite(_clkPin, HIGH);

	delay_ms(200);
	SPISW_LCD_cmd0(S1D15G10_DISCTL);	// Display Control
	SPISW_LCD_dat0(0x0d);				// 0x00 = 2 divisions, switching period=8 (default)
	SPISW_LCD_dat0(0x20);				// 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
	SPISW_LCD_dat0(0x00);				// 0x00 = no inversely highlighted lines
	SPISW_LCD_cmd0(S1D15G10_COMSCN);	// common output scan direction
	SPISW_LCD_dat0(0x01);				// 0x01 = Scan 1->80, 160<-81
	SPISW_LCD_cmd0(S1D15G10_OSCON);		// oscillators on and get out of sleep mode.
	SPISW_LCD_cmd0(S1D15G10_SLPOUT);	// sleep out
	SPISW_LCD_cmd0(S1D15G10_PWRCTR);	// reference voltage regulator on, circuit voltage follower on, BOOST ON
	SPISW_LCD_dat0(0xf);				// everything on, no external reference resistors
	SPISW_LCD_cmd0(S1D15G10_DISNOR);	// display mode
	SPISW_LCD_cmd0(S1D15G10_DISINV);	// display mode
	SPISW_LCD_cmd0(S1D15G10_PTLOUT);	// Partial area off
	SPISW_LCD_cmd0(S1D15G10_DATCTL);	// The DATCTL command selects the display mode (8-bit or 12-bit).
	SPISW_LCD_dat0(0x00);				// 0x01 = page address inverted, col address normal, address scan in col direction
	SPISW_LCD_dat0(0x00);				// 0x00 = RGB sequence (default value)
	SPISW_LCD_dat0(0x02);				// 0x02 = Grayscale -> 16 (selects 12-bit color, type A)
	SPISW_LCD_cmd0(S1D15G10_VOLCTR);	// The contrast is set by the Electronic Volume Command (VOLCTR).
	SPISW_LCD_dat0(28);					// 32 volume value (adjust this setting for your display 0 .. 63)
	SPISW_LCD_dat0(0);					// 3 resistance ratio (determined by experiment)
	SPISW_LCD_cmd0(S1D15G10_TMPGRD);	// Temprature gradient
	SPISW_LCD_dat0(0);					// default value
	delay_ms(100);
	SPISW_LCD_dat0(0);
	SPISW_LCD_cmd0(S1D15G10_DISON);		// display on
}

// color = 12-bit color value rrrrggggbbbb
void S1D15G10_Clear()
{
	uint32_t x;
	SPISW_LCD_cmd0(S1D15G10_PASET);
	SPISW_LCD_dat0(0);
	SPISW_LCD_dat0(S1D15G10_PWX-1);
	SPISW_LCD_cmd0(S1D15G10_CASET);
	SPISW_LCD_dat0(0);
	SPISW_LCD_dat0(S1D15G10_PWY-1);
	SPISW_LCD_cmd0(S1D15G10_RAMWR);
	for (x = 0; x < ((S1D15G10_PWX * S1D15G10_PWY)/2); x++){
		SPISW_LCD_dat0(0x55);
		SPISW_LCD_dat0(0x55);
		SPISW_LCD_dat0(0);
	}
	SPISW_LCD_cmd0(S1D15G10_SCSTART);
	SPISW_LCD_dat0(0);
}

#define R4G4(v1)        ((uint8_t)(((v1 & 0xf000) >> 8) | ((v1 & 0x07e0) >> 7)))
#define B4R4(v1, v2)    ((uint8_t)(((v1 & 0x1f) << 3) | (v2 >> 12)))
#define G4B4(v2)        ((uint8_t)(((v2 & 0x07e0) >> 3) | ((v2 & 0x1f) >> 1)))

static void S1D15G10_BitBltEx565(int x, int y, int width, int height, uint32_t data[])
{
	int i, j;
	uint16_t v1, v2;
	uint16_t *pdata = (uint16_t *)data;
	SPISW_LCD_cmd0(S1D15G10_DATCTL); // The DATCTL command selects the display mode (8-bit or 12-bit).
	for (j = 0; j < height; j++) {
		SPISW_LCD_cmd0(S1D15G10_PASET);
		SPISW_LCD_dat0(y + j);
		SPISW_LCD_dat0(y + j + 1);
		for (i = 0; i < width; i += 2) {
			SPISW_LCD_cmd0(S1D15G10_CASET);
			SPISW_LCD_dat0(x + i);
			SPISW_LCD_dat0(x + i + 1);
			v1 = *pdata++;
			v2 = *pdata++;
			SPISW_LCD_cmd0(S1D15G10_RAMWR);
			SPISW_LCD_dat0(R4G4(v1));
			SPISW_LCD_dat0(B4R4(v1, v2));
			SPISW_LCD_dat0(G4B4(v2));
		}
	}
}

void S1D15G10_BitBltEx(int x, int y, int width, int height, uint32_t data[])
{
	uint16_t *pdata = (uint16_t *)data;
	pdata += (y * S1D15G10_PWX + x);
	S1D15G10_BitBltEx565(x, y, width, height, (uint32_t *)pdata);
}

void S1D15G10_WriteChar_Color(unsigned char c, uint16_t cx, uint16_t cy, uint16_t fgcol, uint16_t bgcol)
{
	uint8_t x, y;
	uint8_t col0, col1;

	unsigned char *font;
	if (c >= 0x80)
		c = 0;
	font = (unsigned char *)pFont->fontData((int)(c & 0x00ff));
	for (y = 0; y < _font_wy; y++) {
		SPISW_LCD_cmd0(S1D15G10_PASET);	//y set
		SPISW_LCD_dat0((uint8_t)(cy * _font_wy + y + TEXT_SY));
		SPISW_LCD_dat0(disp_wy - 1);
		SPISW_LCD_cmd0(S1D15G10_CASET);
		SPISW_LCD_dat0((uint8_t)(cx * _font_wx + TEXT_SX));
		SPISW_LCD_dat0(disp_wx - 1);
		SPISW_LCD_cmd0(S1D15G10_RAMWR);
		for (x = 0; x < (_font_wx / 2); x++) {
			if (font[y] & (0x80 >> (x * 2))) {
				col0 = fgcol;
			} else {
				col0 = bgcol;
			}
			if (font[y] & (0x40 >> (x * 2))) {
				col1 = fgcol;
			} else {
				col1 = bgcol;
			}
			SPISW_LCD_dat0((0xff & (uint8_t) (col0 >> 4)));
			SPISW_LCD_dat0((0xf0 & (uint8_t) (col0 << 4)) | (0x0f & ((uint8_t) (col1 >> 8))));
			SPISW_LCD_dat0((uint8_t) (0xff & col1));
		}
	}
}

static void S1D15G10_WriteChar(unsigned char c, int cx, int cy)
{
	S1D15G10_WriteChar_Color(c, (uint8_t) cx, (uint8_t) cy, S1D15G10_FCOL, S1D15G10_BCOL);
}

static void S1D15G10_WriteFormattedChar(unsigned char ch)
{
	if (ch == 0xc) {
		S1D15G10_Clear();
		cx = 0;
		cy = 0;
	} else if (ch == '\n') {
		cy++;
		if (cy == disp_wy / _font_wy) {
			cy = 0;
		}
	} else if (ch == '\r') {
		cx = 0;
	} else {
		S1D15G10_WriteChar(ch, cx, cy);
		cx++;
		if (cx == disp_wx / _font_wx) {
			cx = 0;
			cy++;
			if (cy == disp_wy / _font_wy) {
				cy = 0;
			}
		}
	}
}
