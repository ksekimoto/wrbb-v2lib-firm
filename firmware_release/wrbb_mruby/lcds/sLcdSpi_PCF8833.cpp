/*
 * sLcdSpi_PCF8833.cpp
 *
 *  Created on: 2018/01/06
 *      Author: ksekimoto
 */

#include <Arduino.h>
#include <mruby.h>
#include <mruby/string.h>
#include "../wrbb.h"
#include "sLcdSpi.h"
#include "PCF8833.h"

/* ********************************************************************* */
/* LCD controller: PCF8833 */
/* LCD Nokia 6100
/* ********************************************************************* */

static void PCF8833_Reset()
{
	delay_ms(100);
	digitalWrite(_resetPin, LOW);
	delay_ms(1000);
	digitalWrite(_resetPin, HIGH);
	delay_ms(100);
}

static void PCF8833_Initialize()
{
	SPISW_Initialize();
	PCF8833_Reset();

	SPISW_LCD_cmd0(PCF8833_SLEEPOUT);
	SPISW_LCD_cmd0(PCF8833_BSTRON);
	delay_ms(100);
	SPISW_LCD_cmd0(PCF8833_COLMOD);
	SPISW_LCD_dat0(0x03);
	SPISW_LCD_cmd0(PCF8833_MADCTL);
	SPISW_LCD_dat0(0x00);      // 0xc0 MirrorX, MirrorY
	SPISW_LCD_cmd0(PCF8833_SETCON);
	SPISW_LCD_dat0(0x35);
	delay_ms(100);
	SPISW_LCD_cmd0(PCF8833_DISPON);
}

// color = 12-bit color value rrrrggggbbbb
void PCF8833_Clear()
{
	uint32_t x;
	SPISW_LCD_cmd0(PCF8833_PASET);
	SPISW_LCD_dat0(0);
	SPISW_LCD_dat0(PCF8833_PWX-1);
	SPISW_LCD_cmd0(PCF8833_CASET);
	SPISW_LCD_dat0(0);
	SPISW_LCD_dat0(PCF8833_PWY-1);
	SPISW_LCD_cmd0(PCF8833_RAMWR);
	for (x = 0; x < ((PCF8833_PWX * PCF8833_PWY)/2); x++){
		SPISW_LCD_dat0(0);
		SPISW_LCD_dat0(0);
		SPISW_LCD_dat0(0);
	}
}

void PCF8833_BitBltEx565(int x, int y, int width, int height, uint32_t data[])
{
	int i, j;
	uint16_t v1, v2;
	uint16_t *pdata = (uint16_t *)data;
	//SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
	for (j = 0; j < height; j ++) {
		SPISW_LCD_cmd0(PCF8833_PASET);
		SPISW_LCD_dat0(y + j);
		SPISW_LCD_dat0(y + j + 1);
		for (i = 0; i < width; i += 2) {
			SPISW_LCD_cmd0(PCF8833_CASET);
			SPISW_LCD_dat0(x + i);
			SPISW_LCD_dat0(x + i + 1);
			v1 = *pdata++;
			v2 = *pdata++;
			SPISW_LCD_cmd0(PCF8833_RAMWR);
			SPISW_LCD_dat0(R4G4(v1));
			SPISW_LCD_dat0(B4R4(v1, v2));
			SPISW_LCD_dat0(G4B4(v2));
		}
	}
}

void PCF8833_BitBltEx(int x, int y, int width, int height, uint32_t data[])
{
	uint16_t *pdata = (uint16_t *)data;
    pdata += (y * PCF8833_PWX + x);
    PCF8833_BitBltEx565(x, y, width, height, (uint32_t *)pdata);
}

void PCF8833_WriteChar_Color(unsigned char c, uint8_t cx, uint8_t cy, uint16_t fgcol, uint16_t bgcol)
{
	uint8_t x, y;
	uint16_t    col0, col1;

	unsigned char *font;
	if (c >= 0x80)
		c = 0;
	font = (unsigned char *)pFont->fontData((int)(c & 0x00ff));
	for (y = 0; y < _font_wy; y++) {
		SPISW_LCD_cmd0(PCF8833_PASET);    //y set
		SPISW_LCD_dat0(cy * _font_wy + y + TEXT_SY);
		SPISW_LCD_dat0(disp_wy - 1);
		SPISW_LCD_cmd0(PCF8833_CASET);
		SPISW_LCD_dat0(cx * _font_wx + TEXT_SX);
		SPISW_LCD_dat0(disp_wx - 1);
		SPISW_LCD_cmd0(PCF8833_RAMWR);
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
			SPISW_LCD_dat0((0xff & (uint8_t)(col0 >> 4)));
			SPISW_LCD_dat0((0xf0 & (uint8_t)(col0 << 4)) | (0x0f & ((uint8_t)(col1 >> 8))));
			SPISW_LCD_dat0((uint8_t)(0xff & col1));
		}
	}
}

static void PCF8833_WriteChar(unsigned char c, int cx, int cy)
{
	PCF8833_WriteChar_Color(c, (uint8_t)cx, (uint8_t)cy, PCF8833_FCOL, PCF8833_BCOL);
}

void PCF8833_WriteFormattedChar(unsigned char ch)
{
	if (ch == 0xc) {
		PCF8833_Clear();
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
		PCF8833_WriteChar(ch, cx, cy);
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
