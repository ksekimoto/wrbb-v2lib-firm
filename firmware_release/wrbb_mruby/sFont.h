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
	int font_wx;
	int	font_wy;
	int	font_bytes;
	unsigned char *ascii_font_data;
} ASCII_FONT_TBL;

typedef struct _UNICODE_FONT_TBL {
	int font_wx;
	int	font_wy;
	int	font_bytes;
	unsigned short *CUniFontIdx;
	unsigned char *CUniFontMap;
	unsigned char *unicode_font_data;
} UNICODE_FONT_TBL;

#define CUNIFONT_TBL_SIZE (0x100)
#define CUNIFONT_ARY_SIZE (0x10000/(CUNIFONT_TBL_SIZE))

typedef struct _FONT_TBL {
	int font_type;
	int font_unitx;
	int font_unity;
	char *font_name;
	ASCII_FONT_TBL *ascii_font_tbl;
	UNICODE_FONT_TBL *unicode_font_tbl;
} FONT_TBL;

class Font {
public:
	Font(FONT_TBL *font_tbl);
	virtual ~Font();
	char *fontName(void);
	unsigned char* fontData(int c);
	int fontType();
	int fontUnitX();
	int fontUnitY();
	int fontWidth(int c);
	int fontHeight(int c);
	int fontBytes(int c);
private:
	FONT_TBL *_font_tbl;
};

void font_Init(mrb_state *mrb);

extern Font *fontList[];

#endif /* SFONT_H_ */
