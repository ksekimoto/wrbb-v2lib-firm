/*
 * SPI
 * sSpi.h
 *
 * Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */

#ifndef SSPI_H_
#define SSPI_H_

#include <Arduino.h>
#include <SPI.h>

#define SPI_MAX	4
#define SPI_CS_SD	1
extern bool SdBeginFlag;

class Spi2: public SPIClass {
public:
	Spi2(void) : settings_() {
		begin();
	}
	void setSettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode);
	SPISettings settings_;
};

extern SPIClass SPI;

//**************************************************
// ライブラリを定義します
//**************************************************
void spi_Init(mrb_state *mrb);

#endif /* _SSPI_H_ */
