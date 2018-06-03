/*
 * sFont.cpp
 *
 * Portion Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */
#include <Arduino.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include "../wrbb.h"

#define MISAKIFONT4X8
#define MISAKIFONT6X12
#define MISAKIFONT8X8
//#define MISAKIFONT12X12

#include "sFont.h"
#ifdef MISAKIFONT4X8
#include "fonts/sFont4x8.h"
#include "fonts/sFont4x8.cpp"
#endif
#ifdef MISAKIFONT6X12
#include "fonts/sFont6x12.h"
#include "fonts/sFont6x12.cpp"
#endif
#ifdef MISAKIFONT8X8
#include "fonts/sFont8x8.h"
#include "fonts/sFont8x8.cpp"
#endif
#ifdef MISAKIFONT12X12
#include "fonts/sFont12x12.h"
#include "fonts/sFont12x12.cpp"
#endif

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

static const FONT_TBL *fontTblList[] = {
#ifdef MISAKIFONT4X8
	(FONT_TBL *)&misaki_font4x8_tbl,
#endif
#ifdef MISAKIFONT6X12
	(FONT_TBL *)&misaki_font6x12_tbl,
#endif
#ifdef MISAKIFONT8X8
	(FONT_TBL *)&misaki_font8x8_tbl,
#endif
#ifdef MISAKIFONT12X12
	(FONT_TBL *)&misaki_font12x12_tbl,
#endif
	(FONT_TBL *)NULL
};

Font MisakiFont4x8((FONT_TBL *)&misaki_font4x8_tbl);
Font MisakiFont6x12((FONT_TBL *)&misaki_font6x12_tbl);
Font MisakiFont8x8((FONT_TBL *)&misaki_font8x8_tbl);
#ifdef MISAKIFONT12X12
Font MisakiFont12x12((FONT_TBL *)&misaki_font12x12_tbl);
#endif

Font *fontList[] = {
	(Font *)&MisakiFont4x8,
	(Font *)&MisakiFont6x12,
	(Font *)&MisakiFont8x8,
#ifdef MISAKIFONT12X12
	(Font *)&MisakiFont12x12,
#endif
};

Font::Font(FONT_TBL *font_tbl)
{
	_font_tbl = font_tbl;
	_font_bytes = (((font_tbl->font_wx + 7) / 8) * font_tbl->font_wy);
}

Font::~Font()
{
}

char *Font::fontName(void)
{
	return _font_tbl->font_name;
}

int Font::fontWidth(void)
{
	return _font_tbl->font_wx;
}

int Font::fontHeight(void)
{
	return _font_tbl->font_wy;
}

int Font::fontBytes(void)
{
	return _font_bytes;
}

unsigned char *Font::fontData(int idx)
{
	unsigned char *p;
	if (idx < 0x100) {
		idx &= 0xff;
		DEBUG_PRINT("font8 idx: ", idx);
		p = _font_tbl->ascii_font_tbl->ascii_font_data;
		p += (idx * fontBytes());
		return p;
	} else {
		DEBUG_PRINT("font16 idx: ", idx);
		int i;
		int fidx;
		int tblH = idx / CUNIFONT_TBL_SIZE;
		int tblL = idx % CUNIFONT_TBL_SIZE;
		unsigned char mask = (unsigned char) (1 << (idx & 7));
		unsigned char *font_map = _font_tbl->unicode_font_tbl->CUniFontMap;
		unsigned short *font_idx = _font_tbl->unicode_font_tbl->CUniFontIdx;
		if (font_map[(tblH * CUNIFONT_TBL_SIZE)/8 + tblL/8] & mask) {
			fidx = font_idx[tblH];
			for (i = 0; i < tblL; i++) {
				mask = (1 << (i & 7));
				if (font_map[(tblH * CUNIFONT_TBL_SIZE)/8 + (i / 8)] & mask) {
					fidx++;
				}
			}
			DEBUG_PRINT("font16 fidx: ", fidx);
			p = _font_tbl->unicode_font_tbl->unicode_font_data;
			p += (fidx * fontBytes());
		} else {
			DEBUG_PRINT("font16 fidx: ", -1);
			p = (unsigned char *) NULL;
		}
		return p;
	}
}

static void cnv_u8_to_u16(unsigned char *src, int slen, unsigned char *dst, int dsize, int *dlen)
{
	int len;
	int idst = 0;
	unsigned char c;
	unsigned int u = 0;
	unsigned short *udst = (unsigned short *)dst;

	while ((slen > 0) && (idst < dsize)) {
		len = 0;
		c = *src++;
		slen--;
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
		while (len-- > 0 && ((c = *src) & 0xC0) == 0x80) {
			u = (u << 6) | (unsigned int)(c & 0x3F);
			src++;
			slen--;
		}
		DEBUG_PRINT("unicode",u)
		if ((0x10000 <= u) && (u <= 0x10FFFF)) {
			if (udst != NULL) {
				udst[idst] = (unsigned short)(0xD800 | (((u & 0x1FFC00) >> 10) - 0x40));
				udst[idst+1] = (unsigned short)(0xDC00 | (u & 0x3FF));
			}
			idst += 2;
		} else {
			if (udst != NULL) {
				udst[idst] = u;
			}
			idst ++;
		}
	}
	DEBUG_PRINT("len", idst)
	*dlen = idst;
}

int get_font_by_name(char *name)
{
	int idx = 0;
	FONT_TBL *p = (FONT_TBL *)fontTblList;
	while (p != NULL) {
		if (strcmp(p->font_name, name) == 0) {
			return idx;
		}
		idx++;
	}
	return -1;
}

//**************************************************
// メモリの開放時に走る
//**************************************************
static void font_free(mrb_state *mrb, void *ptr) {
	Font* font = static_cast<Font*>(ptr);
	delete font;
}

//**************************************************
// この構造体の意味はよくわかっていない
//**************************************************
static struct mrb_data_type font_type = { "Font", font_free };

mrb_value mrb_font_initialize(mrb_state *mrb, mrb_value self)
{
	DATA_TYPE(self) = &font_type;
	DATA_PTR(self) = NULL;
	int font_idx;
	mrb_get_args(mrb, "i", &font_idx);
	if (font_idx < 0)
		font_idx = 0;
	Font *font = fontList[font_idx];
	DEBUG_PRINT("mrb_font_initialize font_idx", font_idx);
	DATA_PTR(self) = font;
	return self;
}

mrb_value mrb_font_data(mrb_state *mrb, mrb_value self)
{
	Font* font = static_cast<Font*>(mrb_get_datatype(mrb, self, &font_type));
	unsigned char *buf;
	int idx;
	mrb_get_args(mrb, "i", &idx);
	buf = (unsigned char *)font->fontData(idx);
	DEBUG_PRINT("mrb_font_data buf", (int)buf);
	return mrb_str_new(mrb, (const char*)buf, font->fontBytes());
}

#if 0
mrb_value mrb_font_cnvUtf8ToUnicode(mrb_state *mrb, mrb_value self)
{
	mrb_value v;
	mrb_value vsrc;
	char *src;
	int slen;
	int size = 0;
	mrb_get_args(mrb, "Si", &vsrc, &slen);
	src = RSTRING_PTR(vsrc);
	DEBUG_PRINT("mrb_font_cnvUtf8ToUnicode src len", slen);
	cnv_u8_to_u16((unsigned char *)src, slen, (unsigned char *)NULL, 256, &size);
	unsigned char *u16 = (unsigned char *)malloc(sizeof(unsigned short)*(size+2));
	if (u16) {
		cnv_u8_to_u16((unsigned char *)src, slen, u16, sizeof(unsigned short)*(size+2), &size);
		v = mrb_str_new(mrb, (const char*)u16, size*2);
		free(u16);
	} else {
		v = mrb_str_new(mrb, (const char*)"*", 1);
	}
	return v;
}
#else
#define TMP_BUF_MAX	128
static unsigned char tmp[TMP_BUF_MAX];
mrb_value mrb_font_cnvUtf8ToUnicode(mrb_state *mrb, mrb_value self)
{
	mrb_value v;
	mrb_value vsrc;
	char *src;
	int slen;
	int size = 0;
	mrb_get_args(mrb, "Si", &vsrc, &slen);
	src = RSTRING_PTR(vsrc);
	DEBUG_PRINT("mrb_font_cnvUtf8ToUnicode src len", slen);
	cnv_u8_to_u16((unsigned char *)src, slen, (unsigned char *)tmp, TMP_BUF_MAX, &size);
	v = mrb_str_new(mrb, (const char*)tmp, size*2);
	return v;
}
#endif

mrb_value mrb_font_getUnicodeAtIndex(mrb_state *mrb, mrb_value self)
{
	mrb_value vsrc;
	unsigned char *src;
	int index;
	unsigned int u;
	mrb_get_args(mrb, "Si", &vsrc, &index);
	src = (unsigned char *)RSTRING_PTR(vsrc);
	index *= 2;
	u = ((unsigned int)src[index+1] << 8) + (unsigned int)src[index];
	return mrb_fixnum_value(u);
}

void font_Init(mrb_state *mrb)
{
	//クラスを作成する前には、強制gcを入れる
	mrb_full_gc(mrb);

	struct RClass *fontModule = mrb_define_class(mrb, "Font", mrb->object_class);
	MRB_SET_INSTANCE_TT(fontModule, MRB_TT_DATA);

	mrb_define_method(mrb, fontModule, "initialize", mrb_font_initialize, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, fontModule, "data", mrb_font_data, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, fontModule, "cnvUtf8ToUnicode", mrb_font_cnvUtf8ToUnicode, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, fontModule, "getUnicodeAtIndex", mrb_font_getUnicodeAtIndex, MRB_ARGS_REQ(2));
}

