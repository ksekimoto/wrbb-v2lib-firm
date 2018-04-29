/*
 * 12x12 Unicode Font - Misaki Font
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
#include "../fonts/sFont12x12_data.h"
#include "../wrbb.h"

#include "sFont.h"

UNICODE_FONT_TBL _misaki_font12x12_Tbl = {
	(unsigned short *)misaki_font12x12_CUniFontIdx,
	(unsigned char *)misaki_font12x12_CUniFontMap,
	(unsigned char *)misaki_font12x12_data
};

FONT_TBL misaki_font12x12_tbl = {
	.font_type = FONT_UNICODE,
	.font_name = (char *)"MisakiFont12x12",
	.font_wx = 12,
	.font_wy = 12,
	.ascii_font_tbl = (ASCII_FONT_TBL *)NULL,
	.unicode_font_tbl = (UNICODE_FONT_TBL *)&_misaki_font12x12_Tbl
};

//Font MisakiFont12x12((FONT_TBL *)&misaki_font12x12_tbl);
