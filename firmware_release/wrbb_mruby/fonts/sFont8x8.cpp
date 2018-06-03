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
#include "../fonts/sFont8x8_data.h"
#include "../wrbb.h"

#include "sFont.h"

static const UNICODE_FONT_TBL _misaki_font8x8_Tbl = {
	(unsigned short *)misaki_font8x8_CUniFontIdx,
	(unsigned char *)misaki_font8x8_CUniFontMap,
	(unsigned char *)misaki_font8x8_data
};

static const FONT_TBL misaki_font8x8_tbl = {
	.font_type = FONT_UNICODE,
	.font_name = (char *)"MisakiFont8x8",
	.font_wx = 8,
	.font_wy = 8,
	.ascii_font_tbl = (ASCII_FONT_TBL *)NULL,
	.unicode_font_tbl = (UNICODE_FONT_TBL *)&_misaki_font8x8_Tbl
};

//Font MisakiFont8x8((FONT_TBL *)&misaki_font8x8_tbl);
