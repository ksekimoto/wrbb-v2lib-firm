/*
 * 6x12 Ascii Font - Misaki Font
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
#include "../fonts/sFont6x12_data.h"
#include "../wrbb.h"

#include "sFont.h"

static ASCII_FONT_TBL _misaki_font6x12_Tbl = {
	(unsigned char *)misaki_font6x12_data
};

FONT_TBL misaki_font6x12_tbl = {
	FONT_ASCII,
	(char *)"MisakiFont6x12",
	6,
	12,
	(ASCII_FONT_TBL *)&_misaki_font6x12_Tbl,
	(UNICODE_FONT_TBL *)NULL
};

//Font MisakiFont6x12((FONT_TBL *)&misaki_font6x12_tbl);
