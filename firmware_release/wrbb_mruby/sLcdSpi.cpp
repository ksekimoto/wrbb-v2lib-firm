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

#include "../wrbb.h"
#include "sLcdSpi.h"

#include "sFont.h"
#include "PCF8833.h"
#include "S1D15G10.h"

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

static uint8_t _resetPin = 5;
static uint8_t _csPin = 10;
static uint8_t _rsPin = 6;
static uint8_t _doutPin = 11;
static uint8_t _dinPin = 12;
static uint8_t _clkPin = 13;

static void delay_ms(volatile uint32_t n)
{
	delay(n);
}

void SPISW_Initialize()
{
	pinMode(_csPin, OUTPUT);
	pinMode(_doutPin, OUTPUT);
	pinMode(_clkPin, OUTPUT);
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

void SPISW_LCD_cmd0(uint8_t dat)
{
    // Enter command mode: SDATA=LOW at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	digitalWrite(_doutPin, LOW);
	digitalWrite(_clkPin, LOW);
	digitalWrite(_clkPin, HIGH);
    SPISW_Write(dat);
	digitalWrite(_csPin, HIGH);
}

void SPISW_LCD_dat0(uint8_t dat)
{
    // Enter data mode: SDATA=HIGH at rising edge of 1st SCLK
	digitalWrite(_csPin, LOW);
	digitalWrite(_doutPin, HIGH);
	digitalWrite(_clkPin, LOW);
	digitalWrite(_clkPin, HIGH);
	SPISW_Write(dat);
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
};

void LcdSpi::Clear()
{
	uint32_t x;
	SPISW_LCD_cmd0(_PASET);
	SPISW_LCD_dat0(0);
	SPISW_LCD_dat0(_PWX-1);
	SPISW_LCD_cmd0(_CASET);
	SPISW_LCD_dat0(0);
	SPISW_LCD_dat0(_PWY-1);
	SPISW_LCD_cmd0(_RAMWR);
	for (x = 0; x < ((_PWX * _PWY)/2); x++){
		SPISW_LCD_dat0(0);
		SPISW_LCD_dat0(0);
		SPISW_LCD_dat0(0);
	}
	_cx = 0;
	_cy = 0;
	return;
}

LcdSpi::LcdSpi(int num)
{
	Initialize(num);
#if 0
	_cx = 0;
	_cy = 0;
	_fcol = 0xFFFFFF;
	_bcol = 0x000000;
	_font_wx = 4;
	_font_wy = 8;
	pFont = (Font *)NULL;
	_PASET = lcdspi_param[num]._PASET;
	_CASET = lcdspi_param[num]._CASET;
	_RAMWR = lcdspi_param[num]._RAMWR;
	_disp_wx = lcdspi_param[num]._disp_wx;
	_disp_wy = lcdspi_param[num]._disp_wy;
	_PWX = lcdspi_param[num]._PWX;
	_PWY = lcdspi_param[num]._PWY;
	_text_sx = lcdspi_param[num]._text_sx;
	_text_sy = lcdspi_param[num]._text_sy;
	lcdspi_param[num].lcdspi_initialize();
#endif
};

void LcdSpi::Initialize(int num)
{
	_cx = 0;
	_cy = 0;
	_fcol = 0xFFFFFF;
	_bcol = 0x000000;
	_font_wx = 4;
	_font_wy = 8;
	pFont = (Font *)NULL;
	_PASET = lcdspi_param[num]._PASET;
	_CASET = lcdspi_param[num]._CASET;
	_RAMWR = lcdspi_param[num]._RAMWR;
	_disp_wx = lcdspi_param[num]._disp_wx;
	_disp_wy = lcdspi_param[num]._disp_wy;
	_PWX = lcdspi_param[num]._PWX;
	_PWY = lcdspi_param[num]._PWY;
	_text_sx = lcdspi_param[num]._text_sx;
	_text_sy = lcdspi_param[num]._text_sy;
	lcdspi_param[num].lcdspi_initialize();
	Clear();
	return;
}

void LcdSpi::BitBltEx565(int x, int y, int width, int height, uint32_t data[])
{
	int i, j;
	uint16_t v1, v2;
	uint16_t *pdata = (uint16_t *)data;
	//SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
	for (j = 0; j < height; j ++) {
		SPISW_LCD_cmd0(_PASET);
		SPISW_LCD_dat0(y + j);
		SPISW_LCD_dat0(y + j + 1);
		for (i = 0; i < width; i += 2) {
			SPISW_LCD_cmd0(_CASET);
			SPISW_LCD_dat0(x + i);
			SPISW_LCD_dat0(x + i + 1);
			v1 = *pdata++;
			v2 = *pdata++;
			SPISW_LCD_cmd0(_RAMWR);
			SPISW_LCD_dat0(R4G4(v1));
			SPISW_LCD_dat0(B4R4(v1, v2));
			SPISW_LCD_dat0(G4B4(v2));
		}
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
	uint16_t col0, col1;

	if (pFont == (Font *)NULL) {
		return;
	}
	unsigned char *font;
	if (c >= 0x80)
		c = 0;
	font = (unsigned char *)pFont->fontData((int)(c & 0x00ff));
	for (y = 0; y < _font_wy; y++) {
		SPISW_LCD_cmd0(_PASET);		//y set
		SPISW_LCD_dat0(cy * _font_wy + y + _text_sy);
		SPISW_LCD_dat0(_disp_wy - 1);
		SPISW_LCD_cmd0(_CASET);
		SPISW_LCD_dat0(cx * _font_wx + _text_sx);
		SPISW_LCD_dat0(_disp_wx - 1);
		SPISW_LCD_cmd0(_RAMWR);
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

void LcdSpi::WriteChar(unsigned char c, int row, int col)
{
	WriteChar_Color(c, row, col, _fcol, _bcol);
}

void LcdSpi::WriteFormattedChar(unsigned char ch)
{
	if (ch == 0xc) {
		Clear();
		_cx = 0;
		_cy = 0;
	} else if (ch == '\n') {
		_cy++;
		if (_cy == _disp_wy / _font_wy) {
			_cy = 0;
		}
	} else if (ch == '\r') {
		_cx = 0;
	} else {
		WriteChar(ch, _cx, _cy);
		_cx++;
		if (_cx == _disp_wx / _font_wx) {
			_cx = 0;
			_cy++;
			if (_cy == _disp_wy / _font_wy) {
				_cy = 0;
			}
		}
	}
}

void LcdSpi::SetFont(Font *font)
{
	pFont = font;
	_font_wx = font->fontWidth();
	_font_wy = font->fontHeight();
}

Font *LcdSpi::GetFont()
{
	return pFont;
}

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
	int lcdspi_num;
	DATA_TYPE(self) = &lcdspi_type;
	DATA_PTR(self) = NULL;
	mrb_get_args(mrb, "i", &lcdspi_num);
	LcdSpi *lcdspi = new LcdSpi(lcdspi_num);
	lcdspi->Initialize(lcdspi_num);
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
			lcdspi->WriteFormattedChar((*str++));
		}
		return mrb_fixnum_value( 1 );
	} else {
		return mrb_fixnum_value( 0 );
	}
}

//**************************************************
// ライブラリを定義します
//**************************************************
void lcdSpi_Init(mrb_state *mrb)
{
	struct RClass *sLcdSpiModule = mrb_define_class(mrb, "LcdSpi", mrb->object_class);
	MRB_SET_INSTANCE_TT(sLcdSpiModule, MRB_TT_DATA);

	mrb_define_module_function(mrb, sLcdSpiModule, "initialize", mrb_sLcdSpi_initialize, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "clear", mrb_sLcdSpi_clear, MRB_ARGS_REQ(0));
	mrb_define_module_function(mrb, sLcdSpiModule, "set_font", mrb_sLcdSpi_set_font, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "putxy", mrb_sLcdSpi_putxy, MRB_ARGS_REQ(3));
	mrb_define_module_function(mrb, sLcdSpiModule, "putc", mrb_sLcdSpi_putc, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sLcdSpiModule, "puts", mrb_sLcdSpi_puts, MRB_ARGS_REQ(1));
}
