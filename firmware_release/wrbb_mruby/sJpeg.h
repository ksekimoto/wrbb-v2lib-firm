#ifndef SJPEG_H
#define SJPEG_H

#include "picojpeg.h"

#ifndef max
#define max(a,b)     (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)     (((a) < (b)) ? (a) : (b))
#endif

class sJpeg
{
private:
	pjpeg_image_info_t image_info;
	int is_available;
	int mcu_x;
	int mcu_y;
	uint32_t row_pitch;
	uint32_t row_blocks_per_mcu;
	uint32_t col_blocks_per_mcu;
	uint8_t reduce;
	int decode_mcu(void);
public:
	uint8_t *pImage;

	int err;
	int decoded_width;
	int decoded_height;
	int comps;
	int MCUSPerRow;
	int MCUSPerCol;
	pjpeg_scan_type_t scanType;
	int MCUx;
	int MCUy;

	sJpeg();
	~sJpeg();
	int decode(char* pFilename, unsigned char pReduce);
	int available(void);
	int read(void);
	inline int width() {
		return (int)image_info.m_width;
	}
	inline int height() {
		return (int)image_info.m_height;
	}
	inline int MCUWidth() {
		return (int)image_info.m_MCUWidth;
	}
	inline int MCUHeight() {
		return (int)image_info.m_MCUHeight;
	}
};

extern sJpeg jpeg;

#endif
