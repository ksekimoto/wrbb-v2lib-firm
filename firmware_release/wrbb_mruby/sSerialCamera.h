/*
 * sSerialCamera.h
 *
 * Copyright (c) 2018 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */

#ifndef _SSERIALCAMERA_H_
#define _SSERIALCAMERA_H_

class SerialCamera {
public:
	SerialCamera(int id, int baud);
	virtual ~SerialCamera();
	void initialize();
	void precapture(int jpegfmt);
	void capture();
	void save(char *fn);
private:
};

void serialcamera_Init(mrb_state *mrb);

#endif /* _SSERIALCAMERA_H_ */
