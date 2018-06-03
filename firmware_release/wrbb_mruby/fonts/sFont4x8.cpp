/*
 * 4x8 Ascii Font - Misaki Font
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
#include "../fonts/sFont4x8_data.h"
#include "../wrbb.h"

#include "sFont.h"

static const ASCII_FONT_TBL _misaki_font4x8_Tbl = {
	(unsigned char *)misaki_font4x8_data
};

static const FONT_TBL misaki_font4x8_tbl = {
	.font_type = FONT_ASCII,
	.font_name = (char *)"MisakiFont4x8",
	.font_wx = 4,
	.font_wy = 8,
	.ascii_font_tbl = (ASCII_FONT_TBL *)&_misaki_font4x8_Tbl,
	.unicode_font_tbl = (UNICODE_FONT_TBL *)NULL
};

//Font MisakiFont4x8((FONT_TBL *)&misaki_font4x8_tbl);
