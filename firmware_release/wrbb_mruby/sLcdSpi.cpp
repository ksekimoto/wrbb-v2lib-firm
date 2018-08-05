/*
 * sLcdSpi.cpp
 *
 * Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */
#include <Arduino.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>

#include <SD.h>
#include "../wrbb.h"
#include "sFont.h"
#include "PCF8833.h"
#include "S1D15G10.h"
#include "ILI9340.h"
#include "sLcdSpi.h"
#include "sSpi.h"
#ifdef MR_JPEG
#include "sJpeg.h"
#endif

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	#include "sSdCard.h"
#endif

#define TEST_LCDSPI
#ifdef TEST_LCDSPI
#include "sLcdSpiTest.h"
#endif

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

static volatile int g_spihw = 0;
static Spi2* pspi;
static int _clock = 4000000;
static int _bitOrder = 1;	// 0:LSB First, 1:MSB First
static int _dataMode = 0;	// 0-3:Mode0-Mode3
static uint8_t _csPin = 10;
static uint8_t _clkPin = 13;
static uint8_t _doutPin = 11;
static uint8_t _resetPin = 6;
static uint8_t _rsPin = 5;
static uint8_t _dinPin = 12;

static void delay_ms(volatile uint32_t n)
{
	delay(n);
}

static void SPIHW_SetPinMode(void)
{
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	PORTC.PMR.BIT.B5 = 1;
	MPC.PC6PFS.BIT.PSEL = 0x0d;
	PORTC.PMR.BIT.B6 = 1;
	MPC.PC6PFS.BIT.PSEL = 0x0d;
	PORTC.PMR.BIT.B7 = 1;
	MPC.PC6PFS.BIT.PSEL = 0x0d;
}

static void SPISW_SetPinMode(void)
{
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	PORTC.PMR.BIT.B5 = 0;
	PORTC.PMR.BIT.B6 = 0;
	PORTC.PMR.BIT.B7 = 0;
}

void SPISW_Initialize()
{
	pinMode(_csPin, OUTPUT);
	pinMode(_resetPin, OUTPUT);
	pinMode(_rsPin, OUTPUT);
	pinMode(_clkPin, OUTPUT);
	pinMode(_doutPin, OUTPUT);
	pinMode(_dinPin, INPUT);
}

void SPISW_Write(uint8_t dat)
{
	uint8_t i = 8;
	uint8_t value;
	while (i-- > 0) {
		value = (dat & 0x80) ? 1 : 0;
		digitalWrite(_doutPin, value);
		digitalWrite(_clkPin, LOW);
		digitalWrite(_clkPin, HIGH);
		dat <<= 1;
	}
}

void SPI_Write(uint8_t dat)
{
	if (g_spihw) {
#if 0
		SPIHW_SetPinMode(void)
#endif
		pspi->transfer(dat);
#if 0
		SPISW_SetPinMode(void)
#endif
	} else {
		SPISW_Write(dat);
	}
}

void SPISW_LCD_cmd8_0(uint8_t dat)
{
    // Enter command mode: SDATA=LOW at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	digitalWrite(_doutPin, LOW);
	digitalWrite(_clkPin, LOW);
	digitalWrite(_clkPin, HIGH);
    SPI_Write(dat);
	digitalWrite(_csPin, HIGH);
}

void SPISW_LCD_dat8_0(uint8_t dat)
{
    // Enter data mode: SDATA=HIGH at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	digitalWrite(_doutPin, HIGH);
	digitalWrite(_clkPin, LOW);
	digitalWrite(_clkPin, HIGH);
	SPI_Write(dat);
	digitalWrite(_csPin, HIGH);
}

void SPISW_LCD_cmd8_1(uint8_t dat)
{
    // Enter command mode: RS=LOW at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	digitalWrite(_rsPin, LOW);
    SPI_Write(dat);
	digitalWrite(_csPin, HIGH);
}

void SPISW_LCD_dat8_1(uint8_t dat)
{
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	digitalWrite(_rsPin, HIGH);
	SPI_Write(dat);
	digitalWrite(_csPin, HIGH);
}

/* ********************************************************************* */
/* LCD Controller: PCF8833                                               */
/* LCD: Nokia 6100                                                       */
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
	//SPI_Initialize();
	PCF8833_Reset();

	SPISW_LCD_cmd8_0(PCF8833_SLEEPOUT);
	SPISW_LCD_cmd8_0(PCF8833_BSTRON);
	delay_ms(100);
	SPISW_LCD_cmd8_0(PCF8833_COLMOD);
	SPISW_LCD_dat8_0(0x03);
	SPISW_LCD_cmd8_0(PCF8833_MADCTL);
	SPISW_LCD_dat8_0(0x00);      // 0xc0 MirrorX, MirrorY
	SPISW_LCD_cmd8_0(PCF8833_SETCON);
	SPISW_LCD_dat8_0(0x35);
	delay_ms(100);
	SPISW_LCD_cmd8_0(PCF8833_DISPON);
}

/* ********************************************************************* */
/* LCD Controller: S1D15G10                                              */
/* LCD: Nokia 6100                                                       */
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
	//SPI_Initialize();
	S1D15G10_Reset();
	digitalWrite(_csPin, LOW);
	digitalWrite(_doutPin, LOW);
	digitalWrite(_clkPin, HIGH);

	delay_ms(200);
	SPISW_LCD_cmd8_0(S1D15G10_DISCTL);	// Display Control
	SPISW_LCD_dat8_0(0x0d);				// 0x00 = 2 divisions, switching period=8 (default)
	SPISW_LCD_dat8_0(0x20);				// 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
	SPISW_LCD_dat8_0(0x00);				// 0x00 = no inversely highlighted lines
	SPISW_LCD_cmd8_0(S1D15G10_COMSCN);	// common output scan direction
	SPISW_LCD_dat8_0(0x01);				// 0x01 = Scan 1->80, 160<-81
	SPISW_LCD_cmd8_0(S1D15G10_OSCON);	// oscillators on and get out of sleep mode.
	SPISW_LCD_cmd8_0(S1D15G10_SLPOUT);	// sleep out
	SPISW_LCD_cmd8_0(S1D15G10_PWRCTR);	// reference voltage regulator on, circuit voltage follower on, BOOST ON
	SPISW_LCD_dat8_0(0xf);				// everything on, no external reference resistors
	SPISW_LCD_cmd8_0(S1D15G10_DISNOR);	// display mode
	SPISW_LCD_cmd8_0(S1D15G10_DISINV);	// display mode
	SPISW_LCD_cmd8_0(S1D15G10_PTLOUT);	// Partial area off
	SPISW_LCD_cmd8_0(S1D15G10_DATCTL);	// The DATCTL command selects the display mode (8-bit or 12-bit).
	SPISW_LCD_dat8_0(0x00);				// 0x01 = page address inverted, col address normal, address scan in col direction
	SPISW_LCD_dat8_0(0x00);				// 0x00 = RGB sequence (default value)
	SPISW_LCD_dat8_0(0x02);				// 0x02 = Grayscale -> 16 (selects 12-bit color, type A)
	SPISW_LCD_cmd8_0(S1D15G10_VOLCTR);	// The contrast is set by the Electronic Volume Command (VOLCTR).
	SPISW_LCD_dat8_0(28);				// 32 volume value (adjust this setting for your display 0 .. 63)
	SPISW_LCD_dat8_0(0);				// 3 resistance ratio (determined by experiment)
	SPISW_LCD_cmd8_0(S1D15G10_TMPGRD);	// Temprature gradient
	SPISW_LCD_dat8_0(0);				// default value
	delay_ms(100);
	SPISW_LCD_dat8_0(0);
	SPISW_LCD_cmd8_0(S1D15G10_DISON);	// display on
}

/* ********************************************************************* */
/* LCD Controller: ILI9340                                               */
/* LCD: xxxxxxxxxx                                                       */
/* ********************************************************************* */

static void ILI9340_Reset()
{
	delay_ms(100);
	digitalWrite(_resetPin, LOW);
	delay_ms(1000);
	digitalWrite(_resetPin, HIGH);
	delay_ms(100);
}

static void ILI9340_Initialize()
{
	//SPI_Initialize();
	ILI9340_Reset();

	SPISW_LCD_cmd8_1(0xcb);
	SPISW_LCD_dat8_1(0x39);
	SPISW_LCD_dat8_1(0x2c);
	SPISW_LCD_dat8_1(0x00);
	SPISW_LCD_dat8_1(0x34);
	SPISW_LCD_dat8_1(0x02);

	SPISW_LCD_cmd8_1(0xcf);
	SPISW_LCD_dat8_1(0x00);
	SPISW_LCD_dat8_1(0xc1);
	SPISW_LCD_dat8_1(0x30);

	SPISW_LCD_cmd8_1(0xe8);
	SPISW_LCD_dat8_1(0x85);
	SPISW_LCD_dat8_1(0x00);
	SPISW_LCD_dat8_1(0x78);

	SPISW_LCD_cmd8_1(0xea);
	SPISW_LCD_dat8_1(0x00);
	SPISW_LCD_dat8_1(0x00);

	SPISW_LCD_cmd8_1(0xed);
	SPISW_LCD_dat8_1(0x64);
	SPISW_LCD_dat8_1(0x03);
	SPISW_LCD_dat8_1(0x12);
	SPISW_LCD_dat8_1(0x81);

	SPISW_LCD_cmd8_1(0xf7);
	SPISW_LCD_dat8_1(0x20);

	SPISW_LCD_cmd8_1(0xc0);
	SPISW_LCD_dat8_1(0x23);

	SPISW_LCD_cmd8_1(0xc1);
	SPISW_LCD_dat8_1(0x10);

	SPISW_LCD_cmd8_1(0xc5);
	SPISW_LCD_dat8_1(0x3e);
	SPISW_LCD_dat8_1(0x28);

	SPISW_LCD_cmd8_1(0xc7);
	SPISW_LCD_dat8_1(0x86);

	SPISW_LCD_cmd8_1(0x36);
	SPISW_LCD_dat8_1(0x48);

	SPISW_LCD_cmd8_1(0x3a);
	SPISW_LCD_dat8_1(0x55);

	SPISW_LCD_cmd8_1(0xb1);
	SPISW_LCD_dat8_1(0x00);
	SPISW_LCD_dat8_1(0x18);

	SPISW_LCD_cmd8_1(0xb6);
	SPISW_LCD_dat8_1(0x08);
	SPISW_LCD_dat8_1(0x82);
	SPISW_LCD_dat8_1(0x27);

	SPISW_LCD_cmd8_1(0xf2);
	SPISW_LCD_dat8_1(0x00);

	SPISW_LCD_cmd8_1(0x26);
	SPISW_LCD_dat8_1(0x01);

	SPISW_LCD_cmd8_1(0xe0);
	SPISW_LCD_dat8_1(0x0f);
	SPISW_LCD_dat8_1(0x31);
	SPISW_LCD_dat8_1(0x2b);
	SPISW_LCD_dat8_1(0x0c);
	SPISW_LCD_dat8_1(0x0e);
	SPISW_LCD_dat8_1(0x08);
	SPISW_LCD_dat8_1(0x4e);
	SPISW_LCD_dat8_1(0xf1);
	SPISW_LCD_dat8_1(0x37);
	SPISW_LCD_dat8_1(0x07);
	SPISW_LCD_dat8_1(0x10);
	SPISW_LCD_dat8_1(0x03);
	SPISW_LCD_dat8_1(0x0e);
	SPISW_LCD_dat8_1(0x09);
	SPISW_LCD_dat8_1(0x00);

	SPISW_LCD_cmd8_1(0xe1);
	SPISW_LCD_dat8_1(0x00);
	SPISW_LCD_dat8_1(0x0e);
	SPISW_LCD_dat8_1(0x14);
	SPISW_LCD_dat8_1(0x03);
	SPISW_LCD_dat8_1(0x11);
	SPISW_LCD_dat8_1(0x07);
	SPISW_LCD_dat8_1(0x31);
	SPISW_LCD_dat8_1(0xc1);
	SPISW_LCD_dat8_1(0x48);
	SPISW_LCD_dat8_1(0x08);
	SPISW_LCD_dat8_1(0x0f);
	SPISW_LCD_dat8_1(0x0c);
	SPISW_LCD_dat8_1(0x31);
	SPISW_LCD_dat8_1(0x36);
	SPISW_LCD_dat8_1(0x0f);

	SPISW_LCD_cmd8_1(0x11);
	delay_ms(120);

	SPISW_LCD_cmd8_1(0x29);
	SPISW_LCD_cmd8_1(0x2c);
}

static void ILI9340_addrset(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	SPISW_LCD_cmd8_1(ILI9340_CASET);
	SPISW_LCD_dat8_1((uint8_t)(x1>>8));
	SPISW_LCD_dat8_1((uint8_t)(x1&0xff));
	SPISW_LCD_dat8_1((uint8_t)(x2>>8));
	SPISW_LCD_dat8_1((uint8_t)(x2&0xff));
	SPISW_LCD_cmd8_1(ILI9340_PASET);
	SPISW_LCD_dat8_1((uint8_t)(y1>>8));
	SPISW_LCD_dat8_1((uint8_t)(y1&0xff));
	SPISW_LCD_dat8_1((uint8_t)(y2>>8));
	SPISW_LCD_dat8_1((uint8_t)(y2&0xff));
	SPISW_LCD_cmd8_1(ILI9340_RAMWR);
}

LCDSPI_PARAM lcdspi_param[] = {
	{
		PCF8833_Initialize,
		PCF8833_PASET,
		PCF8833_CASET,
		PCF8833_RAMWR,
		PCF8833_WX,
		PCF8833_WY,
		PCF8833_PWX,
		PCF8833_PWY,
		PCF8833_SX,
		PCF8833_SY,
	},
	{
		S1D15G10_Initialize,
		S1D15G10_PASET,
		S1D15G10_CASET,
		S1D15G10_RAMWR,
		S1D15G10_WX,
		S1D15G10_WY,
		S1D15G10_PWX,
		S1D15G10_PWY,
		S1D15G10_SX,
		S1D15G10_SY,
	},
	{
		ILI9340_Initialize,
		ILI9340_PASET,
		ILI9340_CASET,
		ILI9340_RAMWR,
		ILI9340_WX,
		ILI9340_WY,
		ILI9340_PWX,
		ILI9340_PWY,
		ILI9340_SX,
		ILI9340_SY,
	},
};

void LcdSpi::Clear()
{
	int x, y;
	if (_spihw) {
		pspi->beginTransaction(pspi->settings_);
	} else {
		SPISW_SetPinMode();
	}
	if (_lcd_id < 2) {
		SPISW_LCD_cmd8_0(_PASET);
		SPISW_LCD_dat8_0(0);
		SPISW_LCD_dat8_0(_PWX-1);
		SPISW_LCD_cmd8_0(_CASET);
		SPISW_LCD_dat8_0(0);
		SPISW_LCD_dat8_0(_PWY-1);
		SPISW_LCD_cmd8_0(_RAMWR);
		for (x = 0; x < ((_PWX * _PWY)/2); x++){
			SPISW_LCD_dat8_0(0);
			SPISW_LCD_dat8_0(0);
			SPISW_LCD_dat8_0(0);
		}
	} if (_lcd_id == 2) {
		ILI9340_addrset(
				(uint16_t)0,
				(uint16_t)0,
				(uint16_t)(_PWX-1),
				(uint16_t)(_PWY-1));
		for (y = 0; y < _PWY; y++) {
			for (x = 0; x < _PWX; x++) {
				SPISW_LCD_dat8_1(0);
				SPISW_LCD_dat8_1(0);
			}
		}
	}
	if (_spihw) {
		pspi->endTransaction();
	} else {
		SPIHW_SetPinMode();
	}
	_cx = 0;
	_cy = 0;
	return;
}

void LcdSpi::Initialize()
{
	_cx = 0;
	_cy = 0;
	_fcol = (uint16_t)0xFFFFFF;
	_bcol = (uint16_t)0x000000;
	_unit_wx = 4;
	_unit_wy = 8;
	pFont = (Font *)NULL;
	_PASET = lcdspi_param[_lcd_id]._PASET;
	_CASET = lcdspi_param[_lcd_id]._CASET;
	_RAMWR = lcdspi_param[_lcd_id]._RAMWR;
	_disp_wx = lcdspi_param[_lcd_id]._disp_wx;
	_disp_wy = lcdspi_param[_lcd_id]._disp_wy;
	_PWX = lcdspi_param[_lcd_id]._PWX;
	_PWY = lcdspi_param[_lcd_id]._PWY;
	_text_sx = lcdspi_param[_lcd_id]._text_sx;
	_text_sy = lcdspi_param[_lcd_id]._text_sy;
	if (_spihw) {
		pspi->beginTransaction(pspi->settings_);
	} else {
		SPISW_SetPinMode();
	}
	lcdspi_param[_lcd_id].lcdspi_initialize();
	if (_spihw) {
		pspi->endTransaction();
	} else {
		SPIHW_SetPinMode();
	}
	Clear();
	return;
}

LcdSpi::LcdSpi(int lcd_id, int spihw)
{
	_lcd_id = lcd_id;
	_spihw = spihw;
	g_spihw = spihw;
	if (_spihw) {
		SPIHW_SetPinMode();
		pinMode(_csPin, OUTPUT);
		pinMode(_resetPin, OUTPUT);
		pinMode(_rsPin, OUTPUT);
		pspi = new Spi2();
		pspi->setSettings((uint32_t)_clock, (uint8_t)_bitOrder, (uint8_t)_dataMode);
	} else {
		SPISW_SetPinMode();
		SPISW_Initialize();
	}
	Initialize();
};

LcdSpi::~LcdSpi()
{
	if (_spihw) {
		delete pspi;
	}
};

void LcdSpi::BitBltEx565(int x, int y, int width, int height, uint32_t data[])
{
	int i, j;
	uint16_t v1, v2;
	uint16_t *pdata = (uint16_t *)data;
	//SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
	if (_spihw) {
		SPIHW_SetPinMode();
		pspi->beginTransaction(pspi->settings_);
	} else {
		SPISW_SetPinMode();
	}
	if (_lcd_id < 2) {
		for (j = 0; j < height; j ++) {
			SPISW_LCD_cmd8_0(_PASET);
			SPISW_LCD_dat8_0(y + j);
			SPISW_LCD_dat8_0(y + j + 1);
			for (i = 0; i < width; i += 2) {
				SPISW_LCD_cmd8_0(_CASET);
				SPISW_LCD_dat8_0(x + i);
				SPISW_LCD_dat8_0(x + i + 1);
				v1 = *pdata++;
				v2 = *pdata++;
				SPISW_LCD_cmd8_0(_RAMWR);
				SPISW_LCD_dat8_0(R4G4(v1));
				SPISW_LCD_dat8_0(B4R4(v1, v2));
				SPISW_LCD_dat8_0(G4B4(v2));
			}
		}
	} else if (_lcd_id == 2){
		for (j = 0; j < height; j ++) {
			ILI9340_addrset(
					(uint16_t)x,
					(uint16_t)(y + j),
					(uint16_t)(x + width),
					(uint16_t)(y + j + 1));
			for (i = 0; i < width; i += 1) {
				v1 = *pdata++;
				SPISW_LCD_dat8_1((uint8_t)(v1 >> 8));
				SPISW_LCD_dat8_1((uint8_t)v1);
			}
		}

	}
	if (_spihw) {
		pspi->endTransaction();
	} else {
		SPIHW_SetPinMode();
	}
}

void LcdSpi::BitBltEx(int x, int y, int width, int height, uint32_t data[])
{
	uint16_t *pdata = (uint16_t *)data;
    pdata += (y * _PWX + x);
    BitBltEx565(x, y, width, height, (uint32_t *)pdata);
}

void LcdSpi::WriteChar_Color(unsigned char c, int cx, int cy, uint16_t fgcol, uint16_t bgcol)
{
	int x, y;
	int ux, uy;
	int wx, wy;
	uint16_t col0, col1;

	if (pFont == (Font *)NULL) {
		return;
	}
	unsigned char *font;
	if (c >= 0x80)
		c = 0;
	font = (unsigned char *)pFont->fontData((int)(c & 0x00ff));
	ux = pFont->fontUnitX();
	uy = pFont->fontUnitY();
	wx = pFont->fontWidth((int)(c & 0x00ff));
	wy = pFont->fontHeight((int)(c & 0x00ff));
	if (_spihw) {
		pspi->beginTransaction(pspi->settings_);
	} else {
		SPISW_SetPinMode();
	}
	if (_lcd_id < 2) {
		for (y = 0; y < wy; y++) {
			SPISW_LCD_cmd8_0(_CASET);
			SPISW_LCD_dat8_0(cx * ux + _text_sx);
			SPISW_LCD_dat8_0(_disp_wx - 1);
			SPISW_LCD_cmd8_0(_PASET);		//y set
			SPISW_LCD_dat8_0(cy * uy + y + _text_sy);
			SPISW_LCD_dat8_0(_disp_wy - 1);
			SPISW_LCD_cmd8_0(_RAMWR);
			for (x = 0; x < (wx / 2); x++) {
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
				SPISW_LCD_dat8_0((0xff & (uint8_t) (col0 >> 4)));
				SPISW_LCD_dat8_0(
						(0xf0 & (uint8_t) (col0 << 4))
								| (0x0f & ((uint8_t) (col1 >> 8))));
				SPISW_LCD_dat8_0((uint8_t) (0xff & col1));
			}
		}
	} if (_lcd_id == 2) {
		for (y = 0; y < wy; y++) {
			ILI9340_addrset(
					(uint16_t)(cx * ux + _text_sx),
					(uint16_t)(cy * uy + y + _text_sy),
					(uint16_t)(_disp_wx - 1),
					(uint16_t)(_disp_wy - 1));
			for (x = 0; x < wx; x++) {
				if (font[y] & (0x80 >> x)) {
					col0 = fgcol;
				} else {
					col0 = bgcol;
				}
				SPISW_LCD_dat8_1((uint8_t)(col0 >> 8));
				SPISW_LCD_dat8_1((uint8_t)col0);
			}
		}
	}
	if (_spihw) {
		pspi->endTransaction();
	} else {
		SPIHW_SetPinMode();
	}
}

void LcdSpi::WriteUnicode_Color(unsigned short u, int cx, int cy, uint16_t fgcol, uint16_t bgcol)
{
	int x, y;
	int ux, uy;
	int wx, wy;
	int off, sht;
	uint16_t col0, col1;

	if (pFont == (Font *)NULL) {
		return;
	}
	unsigned char *font;
	font = (unsigned char *)pFont->fontData((int)u);
	ux = pFont->fontUnitX();
	uy = pFont->fontUnitY();
	wx = pFont->fontWidth((int)u);
	wy = pFont->fontHeight((int)u);
	if (_spihw) {
		pspi->beginTransaction(pspi->settings_);
	} else {
		SPISW_SetPinMode();
	}
	off = 0;
	sht = 0;
	if (_lcd_id < 2) {
		for (y = 0; y < wy; y++) {
			SPISW_LCD_cmd8_0(_CASET);
			SPISW_LCD_dat8_0(cx * ux + _text_sx);
			SPISW_LCD_dat8_0(_disp_wx - 1);
			SPISW_LCD_cmd8_0(_PASET);		//y set
			SPISW_LCD_dat8_0(cy * uy + y + _text_sy);
			SPISW_LCD_dat8_0(_disp_wy - 1);
			SPISW_LCD_cmd8_0(_RAMWR);
			for (x = 0; x < wx; x+=2) {
				if (x == 8) {
					off++;
				}
				if (font[off] & (0x80 >> (x & 0x7))) {
					col0 = fgcol;
				} else {
					col0 = bgcol;
				}
				if (font[off] & (0x40 >> (x & 0x7))) {
					col1 = fgcol;
				} else {
					col1 = bgcol;
				}
				SPISW_LCD_dat8_0((0xff & (uint8_t) (col0 >> 4)));
				SPISW_LCD_dat8_0(
						(0xf0 & (uint8_t) (col0 << 4))
								| (0x0f & ((uint8_t) (col1 >> 8))));
				SPISW_LCD_dat8_0((uint8_t) (0xff & col1));
			}
			off++;
		}
	} if (_lcd_id == 2) {
		for (y = 0; y < wy; y++) {
			ILI9340_addrset(
					(uint16_t)(cx * ux + _text_sx),
					(uint16_t)(cy * uy + y + _text_sy),
					(uint16_t)(_disp_wx - 1),
					(uint16_t)(_disp_wy - 1));
			for (x = 0; x < wx; x++) {
				if (x == 8) {
					off++;
				}
				if (font[off] & (0x80 >> (x & 0x7))) {
					col0 = fgcol;
				} else {
					col0 = bgcol;
				}
				SPISW_LCD_dat8_1((uint8_t)(col0 >> 8));
				SPISW_LCD_dat8_1((uint8_t)col0);
			}
			off++;
		}
	}
	if (_spihw) {
		pspi->endTransaction();
	} else {
		SPIHW_SetPinMode();
	}
}

void LcdSpi::WriteChar(unsigned char c, int row, int col)
{
	WriteChar_Color(c, row, col, _fcol, _bcol);
}

void LcdSpi::WriteUnicode(unsigned short u, int row, int col)
{
	WriteUnicode_Color(u, row, col, _fcol, _bcol);
}


void LcdSpi::WriteFormattedChar(unsigned char ch)
{
	if (ch == 0xc) {
		Clear();
		_cx = 0;
		_cy = 0;
	} else if (ch == '\n') {
		_cy++;
		if (_cy == _disp_wy / pFont->fontUnitY()) {
			_cy = 0;
		}
	} else if (ch == '\r') {
		_cx = 0;
	} else {
		WriteChar(ch, _cx, _cy);
		_cx++;
		if (_cx == _disp_wx / pFont->fontUnitX()) {
			_cx = 0;
			_cy++;
			if (_cy == _disp_wy / pFont->fontUnitY()) {
				_cy = 0;
			}
		}
	}
}

void LcdSpi::WriteFormattedUnicode(unsigned short u)
{
	if ((char)u == 0xc) {
		Clear();
		_cx = 0;
		_cy = 0;
	} else if ((char)u == '\n') {
		_cy++;
		if (_cy == _disp_wy / pFont->fontUnitY()) {
			_cy = 0;
		}
	} else if ((char)u == '\r') {
		_cx = 0;
	} else {
		WriteUnicode(u, _cx, _cy);
		if (u < 0x100) {
			_cx++;
		} else {
			_cx+=2;
		}
		if (_cx >= _disp_wx / pFont->fontUnitX()) {
			_cx = 0;
			_cy++;
			if (_cy == _disp_wy / pFont->fontUnitY()) {
				_cy = 0;
			}
		}
	}
}

void LcdSpi::SetFont(Font *font)
{
	pFont = font;
}

Font *LcdSpi::GetFont()
{
	return pFont;
}

// https://ja.wikipedia.org/wiki/Windows_bitmap
#define BMP_HEADER_SIZE	0x8a
char BmpHeader[BMP_HEADER_SIZE];

int LcdSpi::DispBmpSd(int x, int y, const char *filename)
{
	File fp;
	int ofs, wx, wy, depth, lineBytes, bufSize;
	int bitmapDy = 1;

	if (SD.exists(filename) != true) {
		DEBUG_PRINT("File doesn't exist", filename);
		return -1;
	}
	fp = SD.open(filename, FILE_READ);
	if (!fp) {
		DEBUG_PRINT("File can't be opened", filename);
		return -1;
	}
	fp.readBytes((char *)BmpHeader, BMP_HEADER_SIZE);
	ofs = (int)*((uint32_t *)&BmpHeader[0x0a]);
	wx = (int)*((uint32_t *)&BmpHeader[0x12]);
	wy = (int)*((uint32_t *)&BmpHeader[0x16]);
    depth = (int)*((uint16_t *)&BmpHeader[0x1c]);
    lineBytes = wx * depth / 8;
    bufSize = lineBytes * bitmapDy;
	DEBUG_PRINT("wx=", wx);
	DEBUG_PRINT("wy=", wy);
	DEBUG_PRINT("depth=", depth);
	DEBUG_PRINT("bufSize=", bufSize);
    unsigned char *bitmapOneLine = (unsigned char *)malloc(bufSize);
    if (!bitmapOneLine) {
		DEBUG_PRINT("Memory allocation failed.", -1);
	    fp.close();
    	return -1;
    }
    fp.seek((uint32_t)ofs);
    if (depth == 16) {
    	for (int ty = wy - 1 - bitmapDy; ty >= 0; ty -= bitmapDy) {
    		fp.readBytes((char *)bitmapOneLine, bufSize);
    		BitBltEx565(x, y + ty, wx, bitmapDy, (uint32_t *)bitmapOneLine);
    	}
    } else if (depth == 24) {
    	for (int ty = wy - 1 - bitmapDy; ty >= 0; ty -= bitmapDy) {
    		fp.readBytes((char *)bitmapOneLine, bufSize);
    		for (int i = 0; i < wx; i++) {
    			uint16_t b = (uint16_t)bitmapOneLine[i*3];
    			uint16_t g = (uint16_t)bitmapOneLine[i*3+1];
    			uint16_t r = (uint16_t)bitmapOneLine[i*3+2];
    			uint16_t v = (b >> 3) + ((g >> 2) << 5) + ((r >> 3) << 11);
    			bitmapOneLine[i*2] = (uint8_t)(v);
    			bitmapOneLine[i*2+1] = (uint8_t)(v >> 8);
    		}
    		BitBltEx565(x, y + ty, wx, bitmapDy, (uint32_t *)bitmapOneLine);
    	}
    }
    if (bitmapOneLine) {
    	free(bitmapOneLine);
    }
	return 1;
}

#ifdef MR_JPEG
int LcdSpi::DispJpegSd(int x, int y, const char *filename)
{
	sJpeg jpeg;
	uint8_t *img;
	int cx, cy;
	int sx, sy;
	int decoded_width;
	int decoded_height;
	int dDiv = 2;
	int split_disp = 1;
	int split = 0;
	int alloc_size = 0;
	unsigned short *dispBuf = (unsigned short *)NULL;

	jpeg.decode((char *)filename, split);
	if (jpeg.err != 0) {
		DEBUG_PRINT("jpeg decode error", -1);
		return -1;
	}
	decoded_width = jpeg.decoded_width;
	decoded_height = jpeg.decoded_height;
	if (split_disp) {
		alloc_size = jpeg.MCUWidth() * jpeg.MCUHeight() * sizeof (unsigned short);
	} else {
		// alloc buf for jpeg size
		alloc_size = jpeg.width() * jpeg.height() * sizeof (unsigned short);
	}
	dispBuf = (unsigned short *)malloc(alloc_size);
	if (!dispBuf) {
		DEBUG_PRINT("dispBuf allocation failed.", -1);
		return -1;
	} else {
		DEBUG_PRINT("dispBuf allocated", alloc_size);
	}
	while (jpeg.read()) {
		img = jpeg.pImage;
		sx = jpeg.MCUx * jpeg.MCUWidth();
		//sy = jpeg.MCUy * jpeg.MCUHeight() % (_disp_wx / dDiv);
		sy = jpeg.MCUy * jpeg.MCUHeight();
		memset(dispBuf, 0, alloc_size);
		for (int ty = 0; ty < jpeg.MCUHeight(); ty++) {
			for (int tx = 0; tx < jpeg.MCUWidth(); tx++) {
				cx = sx + tx;
				cy = sy + ty;
				if ((cx < jpeg.width()) && (cy < jpeg.height())) {
					if (jpeg.comps == 1) {
						if ((cx < _disp_wx) && (cy < _disp_wy/dDiv)) {
							if (split_disp) {
								DEBUG_PRINT("ofs", ty * decoded_width + tx);
								dispBuf[ty * decoded_width + tx] = (((img[0]>>3)<<11))|(((img[0]>>2)<<5))|((img[0]>>3));
							} else {
								dispBuf[cy * decoded_width + cx] = (((img[0]>>3)<<11))|(((img[0]>>2)<<5))|((img[0]>>3));
							}
						}
					} else {
						//if ((cx < _disp_wx) && (cy < _disp_wy/dDiv)) {
							if (split_disp) {
								//DEBUG_PRINT("ofs", ty * decoded_width + tx);
								dispBuf[ty * jpeg.MCUWidth() + tx] = (((img[0]>>3)<<11))|(((img[1]>>2)<<5))|((img[2]>>3));
							} else {
								dispBuf[cy * decoded_width + cx] = (((img[0]>>3)<<11))|(((img[1]>>2)<<5))|((img[2]>>3));
							}
						//}
					}
				}
				img += jpeg.comps;
			}
		}
		if (split_disp) {
			//DEBUG_PRINT("sx", sx);
			//DEBUG_PRINT("sy", sy);
			int disp_height;
			if ((jpeg.MCUy + 1) * jpeg.MCUHeight() > jpeg.height()) {
				disp_height =  jpeg.height() - sy;
			} else {
				disp_height = jpeg.MCUHeight();
			}
			BitBltEx565(x + sx, y + sy, jpeg.MCUWidth(), disp_height, (uint32_t *)dispBuf);
		}
	}
	DEBUG_PRINT("err=", jpeg.err);
	if (jpeg.err == 0 || jpeg.err == PJPG_NO_MORE_BLOCKS) {
		if (!split_disp) {
			BitBltEx565(x, y, jpeg.width(), jpeg.height(), (uint32_t *)dispBuf);
		}
	}
	if (dispBuf) {
		DEBUG_PRINT("dispBuf deallocated", 0);
		free(dispBuf);
	}
	return 1;
}
#endif

//**************************************************
// メモリの開放時に走る
//**************************************************
static void lcdspi_free(mrb_state *mrb, void *ptr) {
	LcdSpi* lcdspi = static_cast<LcdSpi*>(ptr);
	delete lcdspi;
}

//**************************************************
// この構造体の意味はよくわかっていない
//**************************************************
static struct mrb_data_type lcdspi_type = { "LcdSpi", lcdspi_free };

mrb_value mrb_sLcdSpi_initialize(mrb_state *mrb, mrb_value self)
{
	int lcdspi_id;
	int spihw;
	int cs, clk, dout, reset, rs, din;
	int n;
	DATA_TYPE(self) = &lcdspi_type;
	DATA_PTR(self) = NULL;
	n = mrb_get_args(mrb, "ii|iiiiii", &lcdspi_id, &spihw, &cs, &clk, &dout, &reset, &rs, &din);
	if ((n >= 2) && (cs != -1))
		_csPin = (uint8_t)cs;
	if ((n >= 3) && (clk != -1))
		_clkPin = (uint8_t)clk;
	if ((n >= 4) && (dout != -1))
		_doutPin = (uint8_t)dout;
	if ((n >= 5) && (reset != -1))
		_resetPin = (uint8_t)reset;
	if ((n >= 6) && (rs != -1))
		_rsPin = (uint8_t)rs;
	if ((n >= 7) && (din != -1))
		_dinPin = (uint8_t)din;
	LcdSpi *lcdspi = new LcdSpi(lcdspi_id, spihw);
	DATA_PTR(self) = lcdspi;
	return self;
}

mrb_value mrb_sLcdSpi_clear(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	lcdspi->Clear();
	return mrb_fixnum_value( 1 );
}

mrb_value mrb_sLcdSpi_set_font(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	int font_idx;
	mrb_get_args(mrb, "i", &font_idx);
	lcdspi->SetFont((Font *)fontList[font_idx]);
	return mrb_fixnum_value( 1 );
}

mrb_value mrb_sLcdSpi_putxy(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vstr;
	unsigned char *str;
	int row, col;
	if (lcdspi->GetFont() != (Font *)NULL) {
		mrb_get_args(mrb, "iiS", &row, &col, &vstr);
		str = (unsigned char *)RSTRING_PTR(vstr);
		DEBUG_PRINT("c=", (*str));
		lcdspi->WriteChar((*str), (int)row, (int)col);
		return mrb_fixnum_value( 1 );
	} else {
		return mrb_fixnum_value( 0 );
	}
}

mrb_value mrb_sLcdSpi_putc(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vstr;
	unsigned char *str;
	if (lcdspi->GetFont() != (Font *)NULL) {
		mrb_get_args(mrb, "S", &vstr);
		str = (unsigned char *)RSTRING_PTR(vstr);
		DEBUG_PRINT("c=", (*str));
		lcdspi->WriteFormattedChar((*str));
		return mrb_fixnum_value( 1 );
	} else {
		return mrb_fixnum_value( 0 );
	}
}

mrb_value mrb_sLcdSpi_puts(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vstr;
	unsigned char *str;
	if (lcdspi->GetFont() != (Font *)NULL) {
		mrb_get_args(mrb, "S", &vstr);
		str = (unsigned char *)RSTRING_PTR(vstr);
		while (*str) {
			//lcdspi->WriteFormattedChar((*str++));
			lcdspi->WriteFormattedUnicode(((unsigned)(*str++)) & 0xff);
		}
		return mrb_fixnum_value( 1 );
	} else {
		return mrb_fixnum_value( 0 );
	}
}

unsigned short cnvUtf8ToUnicode(unsigned char *str, int *size)
{
	unsigned int u = 0;
	unsigned char c = *str++;
	int len;
	if ((c & 0x80) == 0) {
		u = c & 0x7F;
		len = 0;
	} else if ((c & 0xE0) == 0xC0) {
		u = c & 0x1F;
		len = 1;
	} else if ((c & 0xF0) == 0xE0) {
		u = c & 0x0F;
		len = 2;
	} else if ((c & 0xF8) == 0xF0) {
		u = c & 0x07;
		len = 3;
	} else if ((c & 0xFC) == 0xF8) {
		u = c & 0x03;
		len = 4;
	} else if ((c & 0xFE) == 0xFC) {
		u = c & 0x01;
		len = 5;
	}
	*size = len + 1;
	while (len-- > 0 && ((c = *str) & 0xC0) == 0x80) {
		u = (u << 6) | (unsigned int)(c & 0x3F);
		str++;
	}
	return (unsigned short)u;
}

mrb_value mrb_sLcdSpi_pututf8(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vstr;
	unsigned char *str;
	unsigned short u;
	int len;
	if (lcdspi->GetFont() != (Font *)NULL) {
		mrb_get_args(mrb, "S", &vstr);
		str = (unsigned char *)RSTRING_PTR(vstr);
		while (*str) {
			u = cnvUtf8ToUnicode(str, &len);
			lcdspi->WriteFormattedUnicode(u);
			str += len;
		}
		return mrb_fixnum_value( 1 );
	} else {
		return mrb_fixnum_value( 0 );
	}
}

mrb_value mrb_sLcdSpi_BitBlt(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vbuf;
	int x, y, width, height;
	unsigned char *buf;
	mrb_get_args(mrb, "iiiiS", &x, &y, &width, &height, &vbuf);
	buf = (unsigned char *)RSTRING_PTR(vbuf);
#ifdef TEST_LCDSPI
	buf = (unsigned char *)testBmpData;
#endif
	DEBUG_PRINT("x=", x);
	DEBUG_PRINT("y=", y);
	DEBUG_PRINT("w=", width);
	DEBUG_PRINT("h=", height);
	lcdspi->BitBltEx(x, y, width, height, (uint32_t *)buf);
	return mrb_fixnum_value(0);
}

mrb_value mrb_sLcdSpi_dispBmpSd(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vfn;
	int x, y;
	char *fn;
	int ret;
	if (!sdcard_Init(mrb)) {
		DEBUG_PRINT("SD Failed: ", 2);
		return mrb_fixnum_value(2);
	}
	mrb_get_args(mrb, "iiS", &x, &y, &vfn);
	fn = RSTRING_PTR(vfn);
	ret = lcdspi->DispBmpSd(x, y, fn);
	return mrb_fixnum_value(ret);
}

mrb_value mrb_sLcdSpi_dispJpegSd(mrb_state *mrb, mrb_value self)
{
	LcdSpi* lcdspi = static_cast<LcdSpi*>(mrb_get_datatype(mrb, self, &lcdspi_type));
	mrb_value vfn;
	int x, y;
	char *fn;
	int ret;
	if (!sdcard_Init(mrb)) {
		DEBUG_PRINT("SD Failed: ", 2);
		return mrb_fixnum_value(2);
	}
	mrb_get_args(mrb, "iiS", &x, &y, &vfn);
	fn = RSTRING_PTR(vfn);
	ret = lcdspi->DispJpegSd(x, y, fn);
	return mrb_fixnum_value(ret);
}

//**************************************************
// ライブラリを定義します
//**************************************************
void lcdSpi_Init(mrb_state *mrb)
{
	struct RClass *sLcdSpiModule = mrb_define_class(mrb, "LcdSpi", mrb->object_class);
	MRB_SET_INSTANCE_TT(sLcdSpiModule, MRB_TT_DATA);

	mrb_define_module_function(mrb, sLcdSpiModule, "initialize", mrb_sLcdSpi_initialize, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(6));
	mrb_define_module_function(mrb, sLcdSpiModule, "clear", mrb_sLcdSpi_clear, MRB_ARGS_REQ(0));
	mrb_define_module_function(mrb, sLcdSpiModule, "set_font", mrb_sLcdSpi_set_font, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "putxy", mrb_sLcdSpi_putxy, MRB_ARGS_REQ(3));
	mrb_define_module_function(mrb, sLcdSpiModule, "putc", mrb_sLcdSpi_putc, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "puts", mrb_sLcdSpi_puts, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "pututf8", mrb_sLcdSpi_pututf8, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "BitBlt", mrb_sLcdSpi_BitBlt, MRB_ARGS_REQ(5));
	mrb_define_module_function(mrb, sLcdSpiModule, "dispBmpSD", mrb_sLcdSpi_dispBmpSd, MRB_ARGS_REQ(3));
#ifdef MR_JPEG
	mrb_define_module_function(mrb, sLcdSpiModule, "dispJpegSD", mrb_sLcdSpi_dispJpegSd, MRB_ARGS_REQ(3));
#endif
}
