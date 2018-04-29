/*
 * SPI
 * sSpi.cpp
 *
 * Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */
#include <Arduino.h>
#include <SPI.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include "../wrbb.h"
#include "sSpi.h"
#include "sKernel.h"
#include "sSerial.h"

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

void Spi2::setSettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
	settings_ = SPISettings((uint32_t)clock, (uint8_t)bitOrder, (uint8_t)dataMode);
}

//**************************************************
// メモリの開放時に走る
//**************************************************
static void spi_free(mrb_state *mrb, void *ptr) {
	Spi2* spi = static_cast<Spi2*>(ptr);
	delete spi;
}

static struct mrb_data_type spi_type = { "Spi", spi_free };

//**************************************************
// SPIを初期化します: Spi.new
//  Spi.new(num)
//  num: 通信番号(0:xx, 1:xx, 2:xx)
//
// 戻り値
//  Spiのインスタンス
//**************************************************
static mrb_value mrb_spi_initialize(mrb_state *mrb, mrb_value self) {
	// Initialize data type first, otherwise segmentation fault occurs.
	DATA_TYPE(self) = &spi_type;
	DATA_PTR(self) = NULL;

	Spi2* spi = new Spi2();

	DATA_PTR(self) = spi;
	return self;
}

mrb_value mrb_spi_beginTransaction(mrb_state *mrb, mrb_value self)
{
	int clock;
	int bitOrder;
	int dataMode;
	mrb_get_args(mrb, "iii", &clock, &bitOrder, &dataMode);
	Spi2* spi = static_cast<Spi2*>(mrb_get_datatype(mrb, self, &spi_type));
	spi->setSettings((uint32_t)clock, (uint8_t)bitOrder, (uint8_t)dataMode);
	spi->beginTransaction(spi->settings_);
	return mrb_nil_value();
}

mrb_value mrb_spi_endTransaction(mrb_state *mrb, mrb_value self)
{
	Spi2* spi = static_cast<Spi2*>(mrb_get_datatype(mrb, self, &spi_type));
	spi->endTransaction();
	return mrb_nil_value();
}

mrb_value mrb_spi_transfer(mrb_state *mrb, mrb_value self)
{
#if defined(DEBUG)
	int v;
#endif
	int data;
	mrb_get_args(mrb, "i", &data);
	Spi2* spi = static_cast<Spi2*>(mrb_get_datatype(mrb, self, &spi_type));
#if defined(DEBUG)
	DEBUG_PRINT("mrb_spi_transfer d", data);
	v =  (int)(spi->transfer((uint8_t)data));
	DEBUG_PRINT("mrb_spi_transfer v", v);
	return mrb_fixnum_value(v);
#endif
	return mrb_fixnum_value(spi->transfer((uint8_t)data));
}

mrb_value mrb_spi_transfers(mrb_state *mrb, mrb_value self)
{
	mrb_value vbuf;
	char *buf;
	int count;
	Spi2* spi = static_cast<Spi2*>(mrb_get_datatype(mrb, self, &spi_type));
	mrb_get_args(mrb, "Si", &vbuf, &count);
	buf = RSTRING_PTR(vbuf);
	DEBUG_PRINT("mrb_spi_transfers buf[0]", (int)buf[0]);
	DEBUG_PRINT("mrb_spi_transfers buf[1]", (int)buf[1]);
	spi->transfer((void *)buf, (size_t)count);
	return mrb_fixnum_value(1);
}

void spi_Init(mrb_state *mrb)
{
	struct RClass *spiModule = mrb_define_class(mrb, "Spi", mrb->object_class);
	MRB_SET_INSTANCE_TT(spiModule, MRB_TT_DATA);

	mrb_define_method(mrb, spiModule, "initialize", mrb_spi_initialize, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, spiModule, "beginTransaction", mrb_spi_beginTransaction, MRB_ARGS_REQ(3));
	mrb_define_method(mrb, spiModule, "endTransaction", mrb_spi_endTransaction, MRB_ARGS_REQ(0));
	mrb_define_method(mrb, spiModule, "transfer", mrb_spi_transfer, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, spiModule, "transfers", mrb_spi_transfers, MRB_ARGS_REQ(2));
}
