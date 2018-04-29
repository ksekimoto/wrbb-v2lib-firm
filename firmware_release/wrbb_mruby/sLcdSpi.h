/*
 * sLcdSpi.h
 *
 * Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */
#ifndef SLCDSPI_H_
#define SLCDSPI_H_

#include "sFont.h"
//#include "S1D15G10.h"
//#include "PCF8833.h"
//#include "ST7735.h"

#define LCDSPI_PCF8833	0
#define LCDSPI_S1D15G10	1
#define LCDSPI_ST7735	2

// RGB 565 format x2 => RG BR GB 44 44 44 format
// v1: rrrrrggg gggbbbbb
// v2: rrrrrggg gggbbbbb
#define R4G4(v1)        ((uint8_t)(((v1 & 0xf000) >> 8) | ((v1 & 0x07e0) >> 7)))
#define B4R4(v1, v2)    ((uint8_t)(((v1 & 0x1f) << 3) | (v2 >> 12)))
#define G4B4(v2)        ((uint8_t)(((v2 & 0x07e0) >> 3) | ((v2 & 0x1f) >> 1)))

typedef struct _LCDSPI_PARAM {
	void (*lcdspi_initialize)();
	uint8_t _PASET;
	uint8_t _CASET;
	uint8_t _RAMWR;
	int _disp_wx;
	int _disp_wy;
	int _PWX;
	int _PWY;
	int _text_sx;
	int _text_sy;
} LCDSPI_PARAM;

class LcdSpi {
public:
	LcdSpi(int num);
	virtual ~LcdSpi() {};
	void Initialize(int num);
	void Clear();
	void BitBltEx565(int x, int y, int width, int height, uint32_t data[]);
	void BitBltEx(int x, int y, int width, int height, uint32_t data[]);
	void WriteChar_Color(unsigned char c, int cx, int cy, uint16_t fgcol, uint16_t bgcol);
	void WriteChar(unsigned char c, int cx, int cy);
	void WriteFormattedChar(unsigned char ch);
	void SetFont(Font *font);
	Font *GetFont();
private:
	int _lcdspi_type;
	uint16_t _cx;
	uint16_t _cy;
	uint16_t _disp_wx;
	uint16_t _disp_wy;
	uint16_t _fcol;
	uint16_t _bcol;
	uint8_t _PASET;
	uint8_t _CASET;
	uint8_t _RAMWR;
	int _PWX;
	int _PWY;
	int _text_sx;
	int _text_sy;
	Font *pFont;
	int _font_wx;
	int _font_wy;
};

void lcdSpi_Init(mrb_state *mrb);

void SPISW_Initialize(void);
void SPISW_Reset(void);
void SPISW_LCD_cmd0(uint8_t dat);
void SPISW_LCD_dat0(uint8_t dat);
void SPISW_LCD_cmd1(uint8_t dat);
void SPISW_LCD_dat1(uint8_t dat);

#endif /* SLCDSPI_H_ */
