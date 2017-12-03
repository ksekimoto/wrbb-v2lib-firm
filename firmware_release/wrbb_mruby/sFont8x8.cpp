/*
 * 8x8 Unicode Font - Misaki Font
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

#include "sFont8x8.h"
#include "sFont8x8_data.h"

#define FONT_BITS   8  // 8x8
#define FONT_BYTES  (((FONT_BITS + 7) / 8) * FONT_BITS)

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

const unsigned char* Font_GetGlyph(unsigned char c)
{
	c &= 0xFF;
	return &font_4x8[c * 8];
}

const unsigned char* Font_GetGlyph16(unsigned short u)
{
	unsigned short i;
	unsigned short fidx;
	unsigned char *p;
	unsigned short tblH = u / CUNIFONT_TBL_SIZE;
	unsigned short tblL = u % CUNIFONT_TBL_SIZE;
	unsigned char mask = (unsigned char) (1 << (u & 7));

	if (CUniFontMap[tblH][tblL / 8] & mask) {
		fidx = CUniFontIdx[tblH];
		for (i = 0; i < tblL; i++) {
			mask = (1 << (i & 7));
			if (CUniFontMap[tblH][i / 8] & mask) {
				fidx++;
			}
		}
		DEBUG_PRINT("Font_GetGlyph16 fidx", fidx);
		p = (unsigned char*) &font_8x8[0] + (fidx * FONT_BYTES);
	} else
		p = (unsigned char*) NULL;
	return p;
}

int32_t Font_Width()
{
	return 4;
}

int32_t Font_Height()
{
	return 8;
}

int32_t Font_TabWidth()
{
	return 4;
}

#define U16_BUF_MAX	256
static unsigned char u16[U16_BUF_MAX];

static void cnv_u8_to_u16(unsigned char *src, int slen, unsigned char *dst, int dsize, int *dlen)
{
	int len;
	int idst = 0;
	unsigned char c;
	unsigned int u = 0;
	unsigned short *udst = (unsigned short *) dst;

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
			udst[idst] = (unsigned short)(0xD800 | (((u & 0x1FFC00) >> 10) - 0x40));
			udst[idst+1] = (unsigned short)(0xDC00 | (u & 0x3FF));
			idst += 2;
		} else {
			udst[idst] = u;
			idst ++;
		}
	}
	DEBUG_PRINT("len", idst)
	*dlen = idst;
}

mrb_value mrb_font8x8_getAsciiFont(mrb_state *mrb, mrb_value self)
{
	char *buf;
	int index;
	mrb_get_args(mrb, "i", &index);
	if (index >= 256)
		index = 0x20;
	buf = (char *)Font_GetGlyph((unsigned char)index);
	return mrb_str_new(mrb, (const char*)buf, 8);
}

mrb_value mrb_font8x8_getUnicodeFont(mrb_state *mrb, mrb_value self)
{
	char *buf;
	int index;
	mrb_get_args(mrb, "i", &index);
	buf = (char *)Font_GetGlyph16((unsigned short)index);
	if (buf == NULL) {
		DEBUG_PRINT("mrb_font8x8_getUnicodeFont", "buf=NULL");
		buf = (char *)Font_GetGlyph16((unsigned short)0x0020);
	}
	return mrb_str_new(mrb, (const char*)buf, 8);
}

mrb_value mrb_font8x8_cnvUtf8ToUnicode(mrb_state *mrb, mrb_value self)
{
	mrb_value vsrc;
	char *src;
	int slen;
	int size = 0;
	mrb_get_args(mrb, "Si", &vsrc, &slen);
	src = RSTRING_PTR(vsrc);
	//len = strlen(src);
	DEBUG_PRINT("mrb_font8x8_cnvUtf8ToUnicode src len", slen);
	cnv_u8_to_u16((unsigned char *)src, slen, (unsigned char *)u16, U16_BUF_MAX, &size);
	return mrb_str_new(mrb, (const char*)u16, size*2);
}

mrb_value mrb_font8x8_getUnicodeAtIndex(mrb_state *mrb, mrb_value self)
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

void font8x8_Init(mrb_state *mrb)
{
	mrb_define_method(mrb, mrb->kernel_module, "getAsciiFont", mrb_font8x8_getAsciiFont, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->kernel_module, "getUnicodeFont", mrb_font8x8_getUnicodeFont, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->kernel_module, "cnvUtf8ToUnicode", mrb_font8x8_cnvUtf8ToUnicode, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, mrb->kernel_module, "getUnicodeAtIndex", mrb_font8x8_getUnicodeAtIndex, MRB_ARGS_REQ(2));
}
