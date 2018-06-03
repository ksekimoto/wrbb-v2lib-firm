/*
 * sJpeg.cpp
 *
 * Portions Copyright (c) 2018 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 * Modified from https://github.com/MakotoKurauchi/JPEGDecoder.git
 *
 */
#include <Arduino.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include "../wrbb.h"

#include <SD.h>
#include "picojpeg.h"
#include "sJpeg.h"

#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

static File g_pInFile;
static uint32_t g_nInFileSize;
static uint32_t g_nInFileOfs;

sJpeg::sJpeg() {
	g_nInFileSize = 0;
	g_nInFileOfs = 0;
	err = 0;
	is_available = 0;
	mcu_x = 0;
	mcu_y = 0;
	row_pitch = 0;
	decoded_width = 0;
	decoded_height = 0;
	row_blocks_per_mcu = 0;
	col_blocks_per_mcu = 0;
	reduce = 0;
	pImage = (uint8_t *)NULL;
	comps = 0;
	MCUSPerRow = 0;
	MCUSPerCol = 0;
	MCUx = 0;
	MCUy = 0;
	scanType = PJPG_GRAYSCALE;
}

sJpeg::~sJpeg(){
	g_pInFile.close();
	DEBUG_PRINT("File closed", 0);
	if (pImage != (uint8_t *)NULL) {
		free(pImage);
		pImage = (uint8_t *)NULL;
		DEBUG_PRINT("pImage deallocated", 0);
	}
}

unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
	int n;
	n = (int)min(g_nInFileSize - g_nInFileOfs, buf_size);
#ifdef DEBUG_FILE
	DEBUG_PRINT("FileSize", g_nInFileSize);
	DEBUG_PRINT("FileOfs", g_nInFileOfs);
#endif
	g_pInFile.readBytes((char *)pBuf, n);
	*pBytes_actually_read = (unsigned char)(n);
	g_nInFileOfs += n;
	return 0;
}

int sJpeg::decode(char *filename, unsigned char pReduce)
{
	reduce = pReduce;
	g_pInFile = SD.open(filename, FILE_READ);
	if (!g_pInFile) {
		DEBUG_PRINT("File can't be opened", filename);
		return -1;
	} else {
		DEBUG_PRINT("File opened", filename);
	}
	g_nInFileOfs = 0;
	g_nInFileSize = g_pInFile.size();
	DEBUG_PRINT("FileSize", g_nInFileSize);
	err = (int)pjpeg_decode_init(&image_info,
	        (pjpeg_need_bytes_callback_t)pjpeg_need_bytes_callback,
	        (void *)NULL, (unsigned char)reduce);
	if (err != 0) {
		DEBUG_PRINT("pjpeg_decode_init() NG", (int )err);
		if (err == PJPG_UNSUPPORTED_MODE) {
			DEBUG_PRINT("Progressive JPEG not supported.", (int )err);
		}
		g_pInFile.close();
		DEBUG_PRINT("File closed", filename);
		return -1;
	}
	decoded_width =
	        reduce ? (image_info.m_MCUSPerRow * MCUWidth()) / 8 : width();
	decoded_height =
	        reduce ? (image_info.m_MCUSPerCol * MCUHeight()) / 8 : height();
	comps = image_info.m_comps;
	row_pitch = MCUWidth() * image_info.m_comps;
	MCUSPerRow = image_info.m_MCUSPerRow;
	MCUSPerCol = image_info.m_MCUSPerCol;
	scanType = image_info.m_scanType;
	DEBUG_PRINT("decoded_width", decoded_width);
	DEBUG_PRINT("decoded_height", decoded_height);
	DEBUG_PRINT("m_MCUWidth", MCUWidth());
	DEBUG_PRINT("m_MCUHeight", MCUHeight());
	DEBUG_PRINT("m_comps", image_info.m_comps);
	DEBUG_PRINT("row_pitch", row_pitch);
	DEBUG_PRINT("MCUSPerRow", MCUSPerRow);
	DEBUG_PRINT("MCUSPerCol", MCUSPerCol);
	DEBUG_PRINT("scanType", image_info.m_scanType);
	int ImageSize = MCUWidth() * MCUHeight() * image_info.m_comps;
	pImage = (uint8_t *)malloc(ImageSize);
	if (!pImage) {
		DEBUG_PRINT("Memory allocation failed.", -1);
		g_pInFile.close();
		DEBUG_PRINT("File closed", filename);
		return -1;
	} else {
		DEBUG_PRINT("pImage allocated", ImageSize);
	}
	memset(pImage, 0, ImageSize);
	row_blocks_per_mcu = MCUWidth() >> 3;
	col_blocks_per_mcu = MCUHeight() >> 3;
	is_available = 1;
	return decode_mcu();
}

int sJpeg::decode_mcu(void)
{
	err = pjpeg_decode_mcu();
	if (err != 0) {
		is_available = 0;
		g_pInFile.close();
		DEBUG_PRINT("File closed", 0);
		if (err != PJPG_NO_MORE_BLOCKS) {
			DEBUG_PRINT("pjpeg_decode_mcu() failed with status", err);
			if (pImage != (uint8_t *)NULL) {
				free(pImage);
				pImage = (uint8_t *)NULL;
				DEBUG_PRINT("pImage deallocated", 0);
			}
			return -1;
		}
	}
	return 1;
}

int sJpeg::read(void)
{
	int y, x;
	uint8_t *pDst_row;

	if (is_available == 0)
		return 0;
	if (mcu_y >= image_info.m_MCUSPerCol) {
		g_pInFile.close();
		DEBUG_PRINT("File closed", 0);
		if (pImage != (uint8_t *)NULL) {
			free(pImage);
			pImage = (uint8_t *)NULL;
			DEBUG_PRINT("pImage deallocated", 0);
		}
		return 0;
	}
	if (reduce) {
		pDst_row = pImage;
		if (image_info.m_scanType == PJPG_GRAYSCALE) {
			*pDst_row = image_info.m_pMCUBufR[0];
		} else {
			uint32_t y, x;
			for (y = 0; y < col_blocks_per_mcu; y++) {
				uint32_t src_ofs = (y * 128U);
				for (x = 0; x < row_blocks_per_mcu; x++) {
					pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
					pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
					pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
					pDst_row += 3;
					src_ofs += 64;
				}
				pDst_row += row_pitch - 3 * row_blocks_per_mcu;
			}
		}
	} else {
		pDst_row = pImage;
		for (y = 0; y < MCUHeight(); y += 8) {
			const int by_limit = min(8, height() - (mcu_y * MCUHeight() + y));
			for (x = 0; x < MCUWidth(); x += 8) {
				uint8_t *pDst_block = pDst_row + x * image_info.m_comps;
				// Compute source byte offset of the block in the decoder's MCU buffer.
				uint32_t src_ofs = (x * 8U) + (y * 16U);
				const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
				const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
				const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;
				const int bx_limit = min(8, width() - (mcu_x * MCUWidth() + x));
				if (image_info.m_scanType == PJPG_GRAYSCALE) {
					int bx, by;
					for (by = 0; by < by_limit; by++) {
						uint8_t *pDst = pDst_block;
						for (bx = 0; bx < bx_limit; bx++)
							*pDst++ = *pSrcR++;
						pSrcR += (8 - bx_limit);
						pDst_block += row_pitch;
					}
				} else {
					int bx, by;
					for (by = 0; by < by_limit; by++) {
						uint8_t *pDst = pDst_block;
						for (bx = 0; bx < bx_limit; bx++) {
							pDst[0] = *pSrcR++;
							pDst[1] = *pSrcG++;
							pDst[2] = *pSrcB++;
							pDst += 3;
						}
						pSrcR += (8 - bx_limit);
						pSrcG += (8 - bx_limit);
						pSrcB += (8 - bx_limit);
						pDst_block += row_pitch;
					}
				}
			}
			pDst_row += (row_pitch * 8);
		}
	}
	MCUx = mcu_x;
	MCUy = mcu_y;
	mcu_x++;
	if (mcu_x == image_info.m_MCUSPerRow) {
		mcu_x = 0;
		mcu_y++;
	}
	if (decode_mcu() == -1)
		is_available = 0;
	return 1;
}
