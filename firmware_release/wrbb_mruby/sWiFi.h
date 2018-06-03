/*
 * ESP-WROOM-02関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#ifndef _SWIFI_H_
#define _SWIFI_H_  1

#define WIFI_CLASS	"WiFi"

//**************************************************
// ライブラリを定義します
//**************************************************
int esp8266_Init(mrb_state *mrb);

uint32_t ntp(char *ipaddr, int tf);
int wifi_getSD(char *strURL, char *strFname, int n, char **head, int ssl);
int wifi_postSD(char *strURL, char *strSFname, char *strDFname, int n, char **head, int ssl);
int wifi_post(char *strURL, char *strData, char *strDFname, int n, char **head, int ssl);

#endif // _SWIFI_H_
