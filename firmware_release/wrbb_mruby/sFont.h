/*
 * sFont.h
 *
 * Portion Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */

#ifndef SFONT_H_
#define SFONT_H_

#define FONT_ASCII		1
#define FONT_UNICODE	2

typedef struct _ASCII_FONT_TBL {
	unsigned char *ascii_font_data;
} ASCII_FONT_TBL;

typedef struct _UNICODE_FONT_TBL {
	unsigned short *CUniFontIdx;
	unsigned char *CUniFontMap;
	unsigned char *unicode_font_data;
} UNICODE_FONT_TBL;

#define CUNIFONT_TBL_SIZE (0x100)
#define CUNIFONT_ARY_SIZE (0x10000/(CUNIFONT_TBL_SIZE))

typedef struct _FONT_TBL {
	int font_type;
	char *font_name;
	int font_wx;
	int	font_wy;
	ASCII_FONT_TBL *ascii_font_tbl;
	UNICODE_FONT_TBL *unicode_font_tbl;
	//union {
	//	ASCII_FONT_TBL *ascii_font_tbl;
	//	UNICODE_FONT_TBL *unicode_font_tbl;
	//};
} FONT_TBL;

class Font {
public:
	Font(FONT_TBL *font_tbl);
	virtual ~Font();
	char *fontName(void);
	unsigned char* fontData(int c);
	int fontWidth();
	int fontHeight();
	int fontBytes();
private:
	FONT_TBL *_font_tbl;
	int _font_bytes;
};

void font_Init(mrb_state *mrb);

#include "fonts/sFont4x8.h"
#include "fonts/sFont6x12.h"
#include "fonts/sFont8x8.h"
#include "fonts/sFont12x12.h"

extern Font *fontList[];

#endif /* SFONT_H_ */
