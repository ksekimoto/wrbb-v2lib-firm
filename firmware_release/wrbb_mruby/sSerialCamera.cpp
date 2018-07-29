/*
 * sSerialCamera.cpp
 *
 * Portion Copyright (c) 2018 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */

#include <Arduino.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>

#include <SD.h>
#include "../wrbb.h"
#include "sSdCard.h"
#include "sSerialCamera.h"

#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

#define SERIAL_MAX	6
static HardwareSerial *hardwareSerial[SERIAL_MAX] = {
	NULL,		//0:Serial(USB)
	&Serial1,	//1:Serial1
	&Serial2,	//2:Serial3
	&Serial3,	//3:Serial2
	&Serial4,	//4:Serial6
	&Serial5	//5:Serial7
};
static int serial_id = 1;
static int serial_baud = 115200;
static HardwareSerial *hSerial = hardwareSerial[serial_id];

#define PIC_PKT_LEN		128		// data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA		7		// 640x480
#define PIC_FMT_CIF		5		// 320x240
#define PIC_FMT_OCIF	3		// 160x128
#define CAM_ADDR		0

#define PIC_FMT        PIC_FMT_VGA

File myFile;

const byte cameraAddr = (CAM_ADDR << 5);  // addr
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;

void sc_clearRxBuf() {
	while (hSerial->available()) {
		hSerial->read();
	}
}

void sc_sendCmd(char cmd[], int cmd_len) {
	for (char i = 0; i < cmd_len; i++)
		hSerial->print(cmd[i]);
}

void sc_initialize() {
	// Sync Command
	char cmd[] = { 0xaa, 0x0d | cameraAddr, 0x00, 0x00, 0x00, 0x00 };
	unsigned char resp[6];

	hSerial->begin((unsigned long int)serial_baud);
	hSerial->setTimeout(500);
	while (1) {
		//clearRxBuf();
		sc_sendCmd(cmd, 6);
		if (hSerial->readBytes((char *)resp, 6) != 6) {
			continue;
		}
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d
		        && resp[4] == 0 && resp[5] == 0) {
			if (hSerial->readBytes((char *)resp, 6) != 6)
				continue;
			if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr)
			        && resp[2] == 0 && resp[3] == 0 && resp[4] == 0
			        && resp[5] == 0)
				break;
		}
	}
	cmd[1] = 0x0e | cameraAddr;
	cmd[2] = 0x0d;
	sc_sendCmd(cmd, 6);
	DEBUG_PRINT("initialize", "Camera initialization done.");
}

void sc_precapture(int jpegfmt) {
	// Initial Command image format
	char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
	unsigned char resp[6];

	cmd[5] = (char)jpegfmt;
	hSerial->setTimeout(100);
	while (1) {
		sc_clearRxBuf();
		sc_sendCmd(cmd, 6);
		if (hSerial->readBytes((char *)resp, 6) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01
		        && resp[4] == 0 && resp[5] == 0)
			break;
	}
}

void sc_capture() {
	char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff,
	        (PIC_PKT_LEN >> 8) & 0xff, 0 };
	unsigned char resp[6];

	hSerial->setTimeout(100);
	while (1) {
		sc_clearRxBuf();
		sc_sendCmd(cmd, 6);
		if (hSerial->readBytes((char *)resp, 6) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06
		        && resp[4] == 0 && resp[5] == 0)
			break;
	}
	cmd[1] = 0x05 | cameraAddr;
	cmd[2] = 0;
	cmd[3] = 0;
	cmd[4] = 0;
	cmd[5] = 0;
	while (1) {
		sc_clearRxBuf();
		sc_sendCmd(cmd, 6);
		if (hSerial->readBytes((char *)resp, 6) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05
		        && resp[4] == 0 && resp[5] == 0)
			break;
	}
	cmd[1] = 0x04 | cameraAddr;
	cmd[2] = 0x1;
	while (1) {
		sc_clearRxBuf();
		sc_sendCmd(cmd, 6);
		if (hSerial->readBytes((char *)resp, 6) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04
		        && resp[4] == 0 && resp[5] == 0) {
			hSerial->setTimeout(1000);
			if (hSerial->readBytes((char *)resp, 6) != 6) {
				continue;
			}
			if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr)
			        && resp[2] == 0x01) {
				picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
				DEBUG_PRINT("picTotalLen:", picTotalLen);
				break;
			}
		}
	}

}

void sc_getdata(char *picName) {
	unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);
	if ((picTotalLen % (PIC_PKT_LEN - 6)) != 0)
		pktCnt += 1;

	char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };
	unsigned char pkt[PIC_PKT_LEN];

	if (SD.exists(picName)) {
		SD.remove(picName);
	}

	myFile = SD.open(picName, FILE_WRITE);
	if (!myFile) {
		DEBUG_PRINT("sc_getdata", "myFile open fail...");
	} else {
		hSerial->setTimeout(1000);
		for (unsigned int i = 0; i < pktCnt; i++) {
			cmd[4] = i & 0xff;
			cmd[5] = (i >> 8) & 0xff;

			int retry_cnt = 0;
			retry: delay(10);
			sc_clearRxBuf();
			sc_sendCmd(cmd, 6);
			uint16_t cnt = hSerial->readBytes((char *)pkt, PIC_PKT_LEN);

			unsigned char sum = 0;
			for (int y = 0; y < cnt - 2; y++) {
				sum += pkt[y];
			}
			if (sum != pkt[cnt - 2]) {
				if (++retry_cnt < 100)
					goto retry;
				else
					break;
			}

			myFile.write((const uint8_t *)&pkt[4], cnt - 6);
			//if (cnt != PIC_PKT_LEN) break;
		}
		cmd[4] = 0xf0;
		cmd[5] = 0xf0;
		sc_sendCmd(cmd, 6);
	}
	myFile.close();
}

SerialCamera::SerialCamera(int id, int baud)
{
	if (id == 0)
		id = 1;
	serial_id = id;
	serial_baud = baud;
	initialize();
}

SerialCamera::~SerialCamera()
{

}

void SerialCamera::initialize()
{
	DEBUG_PRINT("initialize", "start");
	sc_initialize();
}

void SerialCamera::precapture(int jpegfmt)
{
	DEBUG_PRINT("precapture", "start");
	sc_precapture(jpegfmt);
}

void SerialCamera::capture()
{
	DEBUG_PRINT("capture", "start");
	sc_capture();
}

void SerialCamera::save(char *fn)
{
	DEBUG_PRINT("save", "start");
	sc_getdata(fn);
}

//**************************************************
// メモリの開放時に走る
//**************************************************
static void serialcamera_free(mrb_state *mrb, void *ptr) {
	SerialCamera* serialcamera = static_cast<SerialCamera*>(ptr);
	delete serialcamera;
}

//**************************************************
// この構造体の意味はよくわかっていない
//**************************************************
static struct mrb_data_type serialcamera_type = { "SerialCamera", serialcamera_free };

mrb_value mrb_sSerialCamera_initialize(mrb_state *mrb, mrb_value self) {
	int id;
	int baud;
	int n;
	DATA_TYPE(self) = &serialcamera_type;
	DATA_PTR(self) = NULL;
	n = mrb_get_args(mrb, "|ii", &id, &baud);
	if (n == 0) {
		id = 1;
		baud = 115200;
	} else if (n == 1) {
		baud = 115200;
	}
	SerialCamera *serialcamera = new SerialCamera(id, baud);
	DATA_PTR(self) = serialcamera;
	return self;
}

// jpeg format
// 7: 640x480
// 5: 320x240
// 3: 160x128
mrb_value mrb_sSerialCamera_precapture(mrb_state *mrb, mrb_value self)
{
	int n;
	int fmt, jpegfmt;
	SerialCamera* serialcamera = static_cast<SerialCamera*>(mrb_get_datatype(mrb, self, &serialcamera_type));
	n = mrb_get_args(mrb, "|i", &fmt);
	if (n == 0) {
		fmt = 0;
	}
	if (fmt == 2)
		jpegfmt = 3;
	else if (fmt == 1)
		jpegfmt = 5;
	else
		jpegfmt = 7;
	serialcamera->precapture(jpegfmt);
	return mrb_fixnum_value(1);
}

mrb_value mrb_sSerialCamera_capture(mrb_state *mrb, mrb_value self)
{
	SerialCamera* serialcamera = static_cast<SerialCamera*>(mrb_get_datatype(mrb, self, &serialcamera_type));
	serialcamera->capture();
	return mrb_fixnum_value(1);
}

char picName[] = "pic00.jpg";

mrb_value mrb_sSerialCamera_save(mrb_state *mrb, mrb_value self)
{
	mrb_value vfn;
	char *fn;
	int n;
	SerialCamera* serialcamera = static_cast<SerialCamera*>(mrb_get_datatype(mrb, self, &serialcamera_type));
	n = mrb_get_args(mrb, "|S", &vfn);
	if (n == 1) {
		fn = RSTRING_PTR(vfn);
	} else {
		picName[3] = picNameNum / 10 + '0';
		picName[4] = picNameNum % 10 + '0';
		fn = picName;
	}
	if (!sdcard_Init(mrb)) {
		DEBUG_PRINT("SD Failed: ", 2);
		return mrb_fixnum_value(2);
	}
	serialcamera->save(fn);
	picNameNum++;
	return mrb_fixnum_value(1);
}

void serialcamera_Init(mrb_state *mrb) {
	struct RClass *sSerialCameraModule = mrb_define_class(mrb, "SerialCamera",
	        mrb->object_class);
	MRB_SET_INSTANCE_TT(sSerialCameraModule, MRB_TT_DATA);

	mrb_define_module_function(mrb, sSerialCameraModule, "initialize", mrb_sSerialCamera_initialize, MRB_ARGS_OPT(2));
	mrb_define_module_function(mrb, sSerialCameraModule, "precapture", mrb_sSerialCamera_precapture, MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, sSerialCameraModule, "capture", mrb_sSerialCamera_capture, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, sSerialCameraModule, "save", mrb_sSerialCamera_save, MRB_ARGS_OPT(1));
}

