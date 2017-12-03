/*
 * 8x8 Unicode Font - Misaki Font
 *
 * Portion Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */

#ifndef SFONT8X8_H_
#define SFONT8X8_H_

#define CUNIFONT_TBL_SIZE (0x100)
#define CUNIFONT_ARY_SIZE (0x10000/(CUNIFONT_TBL_SIZE))

const unsigned char* Font_GetGlyph(unsigned char c);
const unsigned char* Font_GetGlyph16(unsigned short u);
int32_t Font_Width();
int32_t Font_Height();
int32_t Font_TabWidth();

void font8x8_Init(mrb_state *mrb);

#endif /* SFONT8X8_H_ */
