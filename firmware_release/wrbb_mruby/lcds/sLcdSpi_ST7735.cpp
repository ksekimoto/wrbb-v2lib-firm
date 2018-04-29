/*
 * sLcdSpi_ST7735.cpp
 *
 *  Created on: 2018/01/06
 *      Author: ksekimoto
 */

#include <Arduino.h>
#include <mruby.h>
#include <mruby/string.h>
#include "../wrbb.h"
#include "sLcdSpi.h"
#include "ST7735.h"

/* ********************************************************************* */
/* ST7735 */
/* ********************************************************************* */

#if 0
static void SPISW_LCD_cmd1(uint8_t dat)
{
    // Enter command mode: SDATA=LOW at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	//digitalWrite(_rsPin, LOW);
    SPISW_Write(dat);
	digitalWrite(_csPin, HIGH);
}

void SPISW_LCD_dat1(uint8_t dat)
{
    // Enter data mode: SDATA=HIGH at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	//digitalWrite(_rsPin, HIGH);
    SPISW_Write(dat);
	digitalWrite(_csPin, HIGH);
}

static void ST7735B_Reset()
{
	delay_ms(100);
	digitalWrite(_resetPin, LOW);
	delay_ms(100);
	digitalWrite(_resetPin, HIGH);
	delay_ms(100);
}

static void ST7735B_Initialize()
{
	pinMode(_resetPin, OUTPUT);
	SPISW_Initialize();
	//pinMode(_rsPin, OUTPUT);
	ST7735B_Reset();

	SPISW_LCD_cmd(0x01);    // SWRESET
	delay_ms(50);
	SPISW_LCD_cmd(0x11);    // SLPOUT
	delay_ms(500);
	SPISW_LCD_cmd(0x3a);    // COLMOD
	SPISW_LCD_dat(0x05);    // 16 bit color
	SPISW_LCD_cmd(0xB1);    // FRMCTR1
	SPISW_LCD_dat(0x00);    // fastest refresh
	SPISW_LCD_dat(0x06);    // 6 lines front porch
	SPISW_LCD_dat(0x03);    // 3 lines back porch
	delay_ms(10);
	SPISW_LCD_cmd(0x36);    // memory access control
	SPISW_LCD_dat(0x08);
	SPISW_LCD_cmd(0xB6);    // DISSET5
	SPISW_LCD_dat(0x15); // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
	SPISW_LCD_dat(0x02);    // fix on VTL
	SPISW_LCD_cmd(0xB4);    // display inversion control
	SPISW_LCD_dat(0x00);
	SPISW_LCD_cmd(0xc0);    // power control 1
	SPISW_LCD_dat(0x02);
	SPISW_LCD_dat(0x70);
	delay_ms(10);
	SPISW_LCD_cmd(0xc1);    // power control 2
	SPISW_LCD_dat(0x05);
	SPISW_LCD_dat(0xc2);    // power control 3
	SPISW_LCD_dat(0x01);
	SPISW_LCD_dat(0x02);
	SPISW_LCD_cmd(0xc5);    // voltage control
	SPISW_LCD_dat(0x3c);    // VCOMH = 4V
	SPISW_LCD_dat(0x38);    // VCOML = -1.1.V

	SPISW_LCD_cmd(0xe0);    // GMCTRP1
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x1a);
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x18);
	SPISW_LCD_dat(0x2f);
	SPISW_LCD_dat(0x28);
	SPISW_LCD_dat(0x20);
	SPISW_LCD_dat(0x22);
	SPISW_LCD_dat(0x1f);
	SPISW_LCD_dat(0x1b);
	SPISW_LCD_dat(0x23);
	SPISW_LCD_dat(0x37);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x07);
	SPISW_LCD_dat(0x02);
	SPISW_LCD_dat(0x10);

	SPISW_LCD_cmd(0xe1);    // GMCTRN1
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x1b);
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x17);
	SPISW_LCD_dat(0x33);
	SPISW_LCD_dat(0x2c);
	SPISW_LCD_dat(0x29);
	SPISW_LCD_dat(0x2e);
	SPISW_LCD_dat(0x30);
	SPISW_LCD_dat(0x30);
	SPISW_LCD_dat(0x39);
	SPISW_LCD_dat(0x3f);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x07);
	SPISW_LCD_dat(0x03);
	SPISW_LCD_dat(0x10);

	SPISW_LCD_cmd(0x2a);    // CASET
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x02);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x81);

	SPISW_LCD_cmd(0x2B);    // RASET
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x02);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0xa0);

	SPISW_LCD_cmd(0x13);   // NORON
	delay_ms(10);
	SPISW_LCD_cmd(0x2c);   // RAMWR
	delay_ms(500);
	SPISW_LCD_cmd(0x29);   //Display on
	delay_ms(500);
}

static void ST7735R_Initialize()
{
	SPISW_Initialize();
	delay_ms(100);
	SPISW_Reset(0);
	delay_ms(100);
	SPISW_Reset(1);
	delay_ms(100);

	SPISW_LCD_cmd(0x01);    // SWRESET
	delay_ms(150);
	SPISW_LCD_cmd(0x11);    // SLPOUT
	delay_ms(500);
	//SPISW_LCD_cmd(0x3a);    // COLMOD
	//SPISW_LCD_dat(0x05);    // 16 bit color
	SPISW_LCD_cmd(0xB1);    // FRMCTR1
	SPISW_LCD_dat(0x01);    //
	SPISW_LCD_dat(0x2c);    //
	SPISW_LCD_dat(0x2d);    //
	SPISW_LCD_cmd(0xB2);    // FRMCTR2
	SPISW_LCD_dat(0x01);    //
	SPISW_LCD_dat(0x2c);    //
	SPISW_LCD_dat(0x2d);    //
	SPISW_LCD_cmd(0xB2);    // FRMCTR3
	SPISW_LCD_dat(0x01);    //
	SPISW_LCD_dat(0x2c);    //
	SPISW_LCD_dat(0x2d);    //
	SPISW_LCD_dat(0x01);    //
	SPISW_LCD_dat(0x2c);    //
	SPISW_LCD_dat(0x2d);    //
	//delay_ms(10);
	//SPISW_LCD_cmd(0x36);    // memory access control
	//SPISW_LCD_dat(0x08);
	//SPISW_LCD_cmd(0xB6);    // DISSET5
	//SPISW_LCD_dat(0x15);    // 1 clock cycle nonoverlap, 2 cycle gate rise, 3 cycle oscil. equalize
	//SPISW_LCD_dat(0x02);    // fix on VTL
	SPISW_LCD_cmd(0xB4);    // display inversion control
	SPISW_LCD_dat(0x07);
	SPISW_LCD_cmd(0xc0);    // power control 1
	SPISW_LCD_dat(0xa2);
	SPISW_LCD_dat(0x02);
	SPISW_LCD_dat(0x84);
	SPISW_LCD_cmd(0xc1);    // power control 2
	SPISW_LCD_dat(0xc5);
	SPISW_LCD_dat(0xc2);    // power control 3
	SPISW_LCD_dat(0x0a);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0xc3);    // power control 4
	SPISW_LCD_dat(0x8a);
	SPISW_LCD_dat(0x2a);
	SPISW_LCD_dat(0xc4);    // power control 5
	SPISW_LCD_dat(0x8a);
	SPISW_LCD_dat(0xee);
	SPISW_LCD_cmd(0xc5);    // voltage control
	SPISW_LCD_dat(0x0e);    //
	SPISW_LCD_cmd(0x20);    // INVOFF
	SPISW_LCD_cmd(0x36);    // memory access control
	SPISW_LCD_dat(0xc8);
	SPISW_LCD_cmd(0x3a);    // COLMOD
	SPISW_LCD_dat(0x05);    // 16 bit color

	SPISW_LCD_cmd(0x2a);    // CASET
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x7f);

	SPISW_LCD_cmd(0x00);    // RASET
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x9f);

	SPISW_LCD_cmd(0xe0);    // GMCTRP1
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x1a);
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x18);
	SPISW_LCD_dat(0x2f);
	SPISW_LCD_dat(0x28);
	SPISW_LCD_dat(0x20);
	SPISW_LCD_dat(0x22);
	SPISW_LCD_dat(0x1f);
	SPISW_LCD_dat(0x1b);
	SPISW_LCD_dat(0x23);
	SPISW_LCD_dat(0x37);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x07);
	SPISW_LCD_dat(0x02);
	SPISW_LCD_dat(0x10);

	SPISW_LCD_cmd(0xe1);    // GMCTRN1
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x1b);
	SPISW_LCD_dat(0x0f);
	SPISW_LCD_dat(0x17);
	SPISW_LCD_dat(0x33);
	SPISW_LCD_dat(0x2c);
	SPISW_LCD_dat(0x29);
	SPISW_LCD_dat(0x2e);
	SPISW_LCD_dat(0x30);
	SPISW_LCD_dat(0x30);
	SPISW_LCD_dat(0x39);
	SPISW_LCD_dat(0x3f);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(0x07);
	SPISW_LCD_dat(0x03);
	SPISW_LCD_dat(0x10);

	SPISW_LCD_cmd(0x29);   //Display on
	delay_ms(100);
	SPISW_LCD_cmd(0x13);   // NORON
	delay_ms(10);
	//SPISW_LCD_cmd(0x2c);   // RAMWR
	//delay_ms(500);
	//SPISW_LCD_cmd(0x29);   //Display on
	//delay_ms(500);
}

static void ST7735_Initialize()
{
	ST7735R_Initialize();
}

// color = 12-bit color value rrrrggggbbbb
static void ST7735_Clear()
{
	uint32_t x;
	SPISW_LCD_cmd (CASET);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(ST7735_OFSX);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(ST7735_WX + ST7735_OFSX);
	SPISW_LCD_cmd (PASET);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(ST7735_OFSY);
	SPISW_LCD_dat(0x00);
	SPISW_LCD_dat(ST7735_WY + ST7735_OFSY);
	SPISW_LCD_cmd (RAMWR);
#if (COLOR_MODE == 12)
	for (x = 0; x < ((ST7735_PWX * ST7735_PWY)/2); x++) {
		SPISW_LCD_dat(0);
		SPISW_LCD_dat(0);
		SPISW_LCD_dat(0);
	}
#elif (COLOR_MODE == 16)
	for (x = 0; x < (ST7735_PWX * ST7735_PWY); x++) {
		SPISW_LCD_dat(0);
		SPISW_LCD_dat(0);
	}
#else
	for (x = 0; x < (ST7735_PWX * ST7735_PWY); x++) {
		SPISW_LCD_dat(0);
		SPISW_LCD_dat(0);
		SPISW_LCD_dat(0);
	}
#endif
}

static void ST7735_BitBltEx565(int x, int y, int width, int height, uint32_t data[])
{
	int i, j;
	uint16_t_t v1, v2;
	uint16_t_t *pdata = (uint16_t_t *) data;
	//SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
	for (j = 0; j < height; j++) {
		SPISW_LCD_cmd (PASET);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(y + j + ST7735_OFSY);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(y + j + 1 + ST7735_OFSY);
#if (COLOR_MODE == 12)
		for (i = 0; i < width; i += 2) {
			SPISW_LCD_cmd(CASET);
			SPISW_LCD_dat(0x00);
			SPISW_LCD_dat(x + i);
			SPISW_LCD_dat(0x00);
			SPISW_LCD_dat(x + i + 1);
			v1 = *pdata++;
			v2 = *pdata++;
			SPISW_LCD_cmd(RAMWR);
			SPISW_LCD_dat(R4G4(v1));
			SPISW_LCD_dat(B4R4(v1, v2));
			SPISW_LCD_dat(G4B4(v2));
		}
#elif (COLOR_MODE == 16)
		SPISW_LCD_cmd (CASET);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(x + ST7735_OFSX);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(x + width + ST7735_OFSX);
		SPISW_LCD_cmd (RAMWR);
		for (i = 0; i < width; i += 1) {
			v1 = *(pdata + i);
			SPISW_LCD_dat((uint8_t) (v1 >> 8));
			SPISW_LCD_dat((uint8_t) (v1));
		}
		pdata += ST7735_PWX;
#else
#endif
	}
}

static void ST7735_BitBltEx(int x, int y, int width, int height, uint32_t data[])
{
	uint16_t_t *pdata = (uint16_t_t *) data;
	pdata += (y * ST7735_PWX + x);
	ST7735_BitBltEx565(x, y, width, height, (uint32_t *) pdata);
}

static void ST7735_WriteChar_Color(unsigned char c, uint8_t cx, uint8_t cy,
		uint16_t_t fgcol, uint16_t_t bgcol)
{
	uint8_t x, y;
	uint16_t_t col0, col1;

	char *font;
	if (c >= 0x80)
		c = 0;
	//font = &font8x8[(c & 0x00ff) << 3];
	font = (char *)Font_GetGlyph(c);
	for (y = 0; y < _font_wy; y++) {
		SPISW_LCD_cmd (PASET);    //y set
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(cy * _font_wy + y + TEXT_SY);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(ST7735_WY - 1);
		SPISW_LCD_cmd (CASET);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(cx * _font_wx + TEXT_SX);
		SPISW_LCD_dat(0x00);
		SPISW_LCD_dat(ST7735_WX - 1);
		SPISW_LCD_cmd (RAMWR);
#if (COLOR_MODE == 12)
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
			SPISW_LCD_dat((0xff & (uint8_t)(col0 >> 4)));
			SPISW_LCD_dat((0xf0 & (uint8_t)(col0 << 4)) | (0x0f & ((uint8_t)(col1 >> 8))));
			SPISW_LCD_dat((uint8_t)(0xff & col1));
		}
#elif (COLOR_MODE == 16)
		for (x = 0; x < _font_wx; x++) {
			if (font[y] & (0x80 >> x)) {
				col0 = fgcol;
			} else {
				col0 = bgcol;
			}
			SPISW_LCD_dat((uint8_t) (col0 >> 8));
			SPISW_LCD_dat((uint8_t) (col0));
		}
#else
#endif
	}
}

static void ST7735_WriteChar(unsigned char c, int cx, int cy)
{
	ST7735_WriteChar_Color(c, (uint8_t) cx, (uint8_t) cy, ST7735_FCOL,
			ST7735_BCOL);
}

static void ST7735_WriteFormattedChar(unsigned char ch)
{
	int x;
	if (ch == 0xc) {
		ST7735_Clear();
		cx = 0;
		cy = 0;
	} else if (ch == '\n') {
		cy++;
		if (cy == ST7735_WY / _font_wy) {
			cy = 0;
		}
	} else if (ch == '\r') {
		cx = 0;
	} else {
		if (cx == 0)
			for (x = 0; x < ST7735_WX / _font_wx; x++)
				ST7735_WriteChar(0x20, x, cy);
		ST7735_WriteChar(ch, cx, cy);
		cx++;
		if (cx == ST7735_WX / _font_wx) {
			cx = 0;
			cy++;
			if (cy == ST7735_WY / _font_wy) {
				cy = 0;
			}
		}
	}
}

static void LCD_Clear()
{
	cx = 0;
	cy = 0;
	ST7735_Clear();
}

static int LCD_Initialize()
{
	ST7735_Initialize();
	LCD_Clear();
	return 1;
}

static int LCD_Uninitialize()
{
	return 1;
}

static void LCD_BitBltEx(int x, int y, int width, int height, uint32_t data[])
{
	ST7735_BitBltEx(x, y, width, height, data);
}

static void LCD_BitBltEx565(int x, int y, int width, int height, uint32_t data[])
{
	ST7735_BitBltEx565(x, y, width, height, data);
}

static void LCD_BitBlt(int width, int height, int widthInWords, uint32_t data[],
		int fUseDelta)
{
	ST7735_BitBltEx(0, 0, width, height, data);
}

static void LCD_WriteChar(unsigned char c, int row, int col)
{
	ST7735_WriteChar(c, row, col);
}

static void LCD_WriteFormattedChar(unsigned char c)
{
	ST7735_WriteFormattedChar(c);
}

static void LCD_WriteFormattedChar16(unsigned short s)
{
	ST7735_WriteFormattedChar((char) s);
}

static int32_t LCD_GetWidth()
{
	return ST7735_WX;
}

static int32_t LCD_GetHeight()
{
	return ST7735_WY;
}

static int32_t LCD_GetBitsPerPixel()
{
	return ST7735_BITSPERPIXEL;
}

static uint32_t LCD_GetPixelClockDivider()
{
	return 0;
}
static int32_t LCD_GetOrientation()
{
	return 0;
}

static void LCD_PowerSave(int On)
{
}

static uint8_t *LCD_GetFrameBuffer()
{
	return (uint8_t *)0;
}

static uint32_t LCD_ConvertColor(uint32_t color)
{
	return color;
}
#endif
