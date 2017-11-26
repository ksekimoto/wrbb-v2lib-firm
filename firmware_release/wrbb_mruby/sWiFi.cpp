/*
 * ESP-WROOM-02関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <string.h>
#include <SD.h>

#include <mruby.h>
#include <mruby/string.h>
#include <mruby/array.h>

#include "../wrbb.h"
#include "sKernel.h"
#include "sSerial.h"

#include "sWiFi.h"

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	#include "sSdCard.h"
#endif
#include <time.h>

extern HardwareSerial *RbSerial[];		//0:Serial(USB), 1:Serial1, 2:Serial3, 3:Serial2, 4:Serial6 5:Serial7

#define  WIFI_SERIAL	3
#define  WIFI_BAUDRATE	115200
//#define  WIFI_CTS		15
#define  WIFI_WAIT_MSEC	10000

unsigned char WiFiData[256];
int WiFiRecvOutlNum = -1;	//ESP8266からの受信を出力するシリアル番号: -1の場合は出力しない。

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#  define DEBUG_PRINT1(s)      { Serial.print((s)); }
#  define DEBUG_PRINTLN1(s)    { Serial.println((s)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#  define DEBUG_PRINT1(s)      // do nothing
#  define DEBUG_PRINTLN1(s)    // do nothing
#endif


//**************************************************
// OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、
// 指定されたシリアルポートに出力します
//
// 1:受信した, 0:受信できなかった 2:受信がオーバーフローした
//**************************************************
int getData(unsigned int wait_msec)
{
unsigned long times;
int c;
int okt = 0;
int ert = 0;
int len = 0;
int n = 0;

	WiFiData[0] = 0;
	times = millis();
	while(n < 256){
		//digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可

		//wait_msec 待つ
		if(millis() - times > wait_msec){
			DEBUG_PRINT("WiFi get Data","Time OUT");
			WiFiData[n] = 0;
			return 0;
		}

		while((len = RbSerial[WIFI_SERIAL]->available()) != 0)
		{
			//DEBUG_PRINT("len=",len);
			//DEBUG_PRINT("n=",n);

			for(int i=0; i<len; i++){
				c = RbSerial[WIFI_SERIAL]->read();

				//指定のシリアルポートに出す設定であれば、受信値を出力します
				if(WiFiRecvOutlNum >= 0){
					RbSerial[WiFiRecvOutlNum]->write((unsigned char)c);
				}
				//DEBUG_PRINT("c=",c);

				WiFiData[n] = c;
				n++;

				if(c == 'O'){
					okt++;
					ert++;
				}
				else if(c == 'K'){
					okt++;
				}
				else if(c == 0x0d){
					ert++;
					okt++;
				}
				else if(c == 0x0a){
					ert++;
					okt++;
					if(okt == 4 || ert == 7){

						// OK 0d0a || ERROR 0d0a
						WiFiData[n] = 0;
						DEBUG_PRINTLN1((const char*)WiFiData);
						return 1;
						//n = 256;
					}
					else{
						ert = 0;
						okt = 0;
					}
				}
				else if(c == 'E' || c == 'R'){
					ert++;
				}
				else{
					okt = 0;
					ert = 0;
				}
			}
			times = millis();
		}
	}
	//digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可

	//whileを抜けてくるということは、オーバーフローしている
	return 2;
}

//**************************************************
// ステーションモードの設定: WiFi.cwmode
//  WiFi.cwmode(mode)
//  WiFi.setMode(mode)
//  mode: 1:Station, 2:SoftAP, 3:Station + SoftAP
//**************************************************
mrb_value mrb_wifi_Cwmode(mrb_state *mrb, mrb_value self)
{
int	mode;

	mrb_get_args(mrb, "i", &mode);

	RbSerial[WIFI_SERIAL]->print("AT+CWMODE=");
	RbSerial[WIFI_SERIAL]->println(mode);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// コマンド応答のシリアル出力設定: WiFi.serialOut
//  WiFi.serialOut( mode[,serialNumber] )
//	mode: 0:出力しない, 1:出力する
//  serialNumber: 出力先のシリアル番号
//**************************************************
mrb_value mrb_wifi_Sout(mrb_state *mrb, mrb_value self)
{
int mode;
int num = -1;

	int n = mrb_get_args(mrb, "i|i", &mode, &num);

	if(mode == 0){
		WiFiRecvOutlNum = -1;
	}
	else{
		if(n >= 2){
			if(num >= 0){
				WiFiRecvOutlNum = num;
			}
		}
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// ATコマンドの送信: WiFi.at
//  WiFi.at( command[, mode] )
//	commnad: ATコマンド文字列
//  mode: 0:'AT+'を自動追加する、1:'AT+'を自動追加しない
//**************************************************
mrb_value mrb_wifi_at(mrb_state *mrb, mrb_value self)
{
mrb_value text;
int mode = 0;

	int n = mrb_get_args(mrb, "S|i", &text, &mode);

	char *s = RSTRING_PTR(text);
	int len = RSTRING_LEN(text);

	if(n <= 1 || mode == 0){
		RbSerial[WIFI_SERIAL]->print("AT+");
	}

	for(int i=0; i<254; i++){
		if( i >= len){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[len] = 0;

	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);
	//DEBUG_PRINT("WiFi.at",(const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// WiFi接続します: WiFi.connect
//  WiFi.connect(SSID,Passwd)
//  WiFi.cwjap(SSID,Passwd)
//  SSID: WiFiのSSID
//  Passwd: パスワード
//**************************************************
mrb_value mrb_wifi_Cwjap(mrb_state *mrb, mrb_value self)
{
mrb_value ssid;
mrb_value passwd;

	mrb_get_args(mrb, "SS", &ssid, &passwd);

	char *s = RSTRING_PTR(ssid);
	int slen = RSTRING_LEN(ssid);

	char *p = RSTRING_PTR(passwd);
	int plen = RSTRING_LEN(passwd);

	RbSerial[WIFI_SERIAL]->print("AT+CWJAP=");
	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for(int i=0; i<254; i++){
		if( i >= slen){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[slen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0x22;		//-> "
	WiFiData[3] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for(int i=0; i<254; i++){
		if( i >= plen){ break; }
		WiFiData[i] = p[i];
	}
	WiFiData[plen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}


//**************************************************
// WiFiアクセスポイントになります: WiFi.softAP
//  WiFi.softAP(SSID,Passwd,Channel,Encrypt)
//  SSID: WiFiのSSID
//  Passwd: パスワード
//  Channel: チャネル
//  Encrypt: 暗号タイプ 0:Open, 1:WEP, 2:WPA_PSK, 3:WPA2_PSK, 4:WPA_WPA2_PSK
//**************************************************
mrb_value mrb_wifi_softAP(mrb_state *mrb, mrb_value self)
{
mrb_value ssid;
mrb_value passwd;
int ch = 1;
int enc = 0;

	mrb_get_args(mrb, "SSii", &ssid, &passwd, &ch, &enc);

	char *s = RSTRING_PTR(ssid);
	int slen = RSTRING_LEN(ssid);

	char *p = RSTRING_PTR(passwd);
	int plen = RSTRING_LEN(passwd);

	if (enc < 0 || enc>4){
		enc = 0;
	}

	RbSerial[WIFI_SERIAL]->print("AT+CWSAP=");
	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for (int i = 0; i<254; i++){
		if (i >= slen){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[slen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0x22;		//-> "
	WiFiData[3] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for (int i = 0; i<254; i++){
		if (i >= plen){ break; }
		WiFiData[i] = p[i];
	}
	WiFiData[plen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	RbSerial[WIFI_SERIAL]->print(ch);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(enc);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// アクセスポイントに接続されているIP取得: WiFi.connetedIP
//  WiFi.connectedIP()
//**************************************************
mrb_value mrb_wifi_connectedIP(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CWLIF");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// DHCP有無の切り替え: WiFi.dhcp
//  WiFi.dhcp(mode, bool)
//  mode: 0:SoftAP, 1:Station, 2:Both softAP + Station
//  bool: 0:disable , 1:enable
//**************************************************
mrb_value mrb_wifi_dhcp(mrb_state *mrb, mrb_value self)
{
int	mode;
int bl = 0;

	mrb_get_args(mrb, "ii", &mode, &bl);

	RbSerial[WIFI_SERIAL]->print("AT+CWDHCP=");
	RbSerial[WIFI_SERIAL]->print(mode);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(bl);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// IPアドレスとMACアドレスの表示: WiFi.ipconfig
//  WiFi.ipconfig()
//  WiFi.cifsr()
//**************************************************
mrb_value mrb_wifi_Cifsr(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CIFSR");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// USBポートとESP8266をシリアルで直結します: WiFi.bypass
//  WiFi.bypass()
// リセットするまで、処理は戻りません。
//**************************************************
mrb_value mrb_wifi_bypass(mrb_state *mrb, mrb_value self)
{
	int len0, len1,len;
	//int retCnt = 0;

	while(true){
		len0 = RbSerial[0]->available();
		len1 = RbSerial[WIFI_SERIAL]->available();

		if(len0 > 0){
			len = len0<256 ? len0 : 256;

			for(int i=0; i<len; i++){
				WiFiData[i] = (unsigned char)RbSerial[0]->read();
			}
			RbSerial[WIFI_SERIAL]->write( WiFiData, len );
		}

		if(len1 > 0){
			len = len1<256 ? len1 : 256;
			
			for(int i=0; i<len; i++){
				WiFiData[i] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
			}
	        RbSerial[0]->write( WiFiData, len );
		}
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// バージョンを取得します: WiFi.version
//  WiFi.version()
//**************************************************
mrb_value mrb_wifi_Version(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+GMR");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// WiFiを切断します: WiFi.disconnect
//  WiFi.disconnect()
//**************************************************
mrb_value mrb_wifi_Disconnect(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CWQAP");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// 複数接続可能モードの設定: WiFi.multiConnect
//  WiFi.multiConnect(mode)
//  mode: 0:1接続のみ, 1:4接続まで可能
//**************************************************
mrb_value mrb_wifi_multiConnect(mrb_state *mrb, mrb_value self)
{
int	mode;

	mrb_get_args(mrb, "i", &mode);

	RbSerial[WIFI_SERIAL]->print("AT+CIPMUX=");
	RbSerial[WIFI_SERIAL]->println(mode);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// ファイルに含まれる+IPDデータを削除します
// ipd: ipdデータ列
// strFname1: 元ファイル
// strFname2: 削除したファイル
//**************************************************
int CutGarbageData(const char *ipd, const char *strFname1, const char *strFname2)
{
File fp, fd;

#if BOARD == BOARD_GR
	int led = digitalRead(PIN_LED0) | ( digitalRead(PIN_LED1)<<1) | (digitalRead(PIN_LED2)<<2)| (digitalRead(PIN_LED3)<<3);
#else
	int led = digitalRead(RB_LED);
#endif

	if( SD.exists(strFname2)){
		SD.remove(strFname2);
	}

	if( !(fp = SD.open(strFname1, FILE_READ)) ){
		return 2;
	}

	if( !(fd = SD.open(strFname2, FILE_WRITE)) ){
		return 3;
	}

	int tLED = 0;
	int ipdLen = strlen(ipd);
	int cnt;
	unsigned char c;
	int rc;
	bool findFlg = true;
	unsigned char str[16];
	int dLen;
	unsigned long seekCnt = 0;
	while(true){

		//LEDを点滅させる
#if BOARD == BOARD_GR
		digitalWrite(PIN_LED0, tLED);
		digitalWrite(PIN_LED1, tLED);
		digitalWrite(PIN_LED2, tLED);
		digitalWrite(PIN_LED3, tLED);
#else
		digitalWrite(RB_LED, tLED);
#endif
		tLED = 1 - tLED;

		//+IPD文字列を探します
		cnt = 0;
		while(true){
			rc = fp.read();

			if(rc < 0){
				findFlg = false;
				break;
			}

			c = (unsigned char)rc;
			if(ipd[cnt] == c){
				cnt++;
				if(cnt == ipdLen){
					seekCnt += cnt;
					break;
				}
			}
			else if(c == 0x0D){
				cnt = 1;
			}
			else{
				cnt = 0;
			}
		}

		//Serial.print("findFlg= ");
		//Serial.println(findFlg);

		if(findFlg == false){	break;	}

		//ここから後はバイト数が来ているはず
		cnt = 0;
		while(true){

			rc = fp.read();

			if(rc < 0){
				findFlg = false;
				break;
			}
			c = (unsigned char)rc;

			str[cnt] = c;

			if(c == ':'){
				str[cnt] = 0;
				seekCnt += cnt + 1;
				break;
			}
			else if(cnt >= 15){
				str[15] = 0;
				findFlg = false;
				break;
			}
			cnt++;
		}

		if(findFlg == false){	break;	}
	
		//読み込むバイト数を求めます
		dLen = atoi((const char*)str);

		seekCnt += dLen;

		//Serial.print("dLen= ");
		//Serial.println((const char*)str);

		while(dLen > 0){
			if(dLen >= 256){
				 fp.read(WiFiData, 256);

				 fd.write( WiFiData, 256);
				 dLen -= 256;
			}
			else{
				 fp.read(WiFiData, dLen);
				 fd.write( WiFiData, dLen);

				 dLen = 0;
			}
		}
	}

	if(findFlg == false){
		//処理していないところは、そのまま書きます
		fp.seek(seekCnt);

		while(true){
			dLen = fp.read(WiFiData, 256);
			fd.write(WiFiData, dLen);
			if(dLen < 256){
				break;
			}
		}
	}

	fd.flush();
	fd.close();

	fp.close();

	//LEDを元の状態に戻す
#if BOARD == BOARD_GR
	digitalWrite(PIN_LED0, led & 1);
	digitalWrite(PIN_LED1, (led >> 1) & 1);
	digitalWrite(PIN_LED2, (led >> 2) & 1);
	digitalWrite(PIN_LED3, (led >> 3) & 1);
#else
	digitalWrite(RB_LED, led);
#endif

	return 1;
}

//**************************************************
// http GETをSDカードに保存します: WiFi.httpGetSD
//  WiFi.httpGetSD( Filename, URL[,Headers] )
//	Filename: 保存するファイル名
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//		2: SDカードが使えない
//		... 各種エラー
//**************************************************
mrb_value mrb_wifi_getSD_sub(mrb_state *mrb, mrb_value self, int ssl)
{
mrb_value vFname, vURL, vHeaders;
const char *tmpFilename = "wifitmp.tmp";
const char *hedFilename = "hedrfile.tmp";
char	*strFname, *strURL;
int len = 0;
File fp, fd;
int sla, koron;

	//SDカードが利用可能か確かめます
	if(!sdcard_Init(mrb)){
		return mrb_fixnum_value( 2 );
	}

	int n = mrb_get_args(mrb, "SS|A", &vFname, &vURL, &vHeaders);

	strFname = RSTRING_PTR(vFname);
	strURL = RSTRING_PTR(vURL);

	//httpサーバに送信するデータを、strFname ファイルに生成します。

	//既にファイルがあれば消す
	if( SD.exists(hedFilename)){
		SD.remove(hedFilename);
	}
	//ファイルオープン
	if( !(fp = SD.open(hedFilename, FILE_WRITE)) ){
		return mrb_fixnum_value( 3 );
	}

	//1行目を生成
	{
		fp.write( (unsigned char*)"GET /", 5);

		//httpsかチェック
		if (ssl) {
			DEBUG_PRINTLN1("AT+CIPSSLSIZE=4096");
			RbSerial[WIFI_SERIAL]->println("AT+CIPSSLSIZE=4096");
			getData(WIFI_WAIT_MSEC);
		}
		//URLからドメインを分割する
		len = strlen(strURL);
		sla = len;
		koron = 0;
		for(int i=0; i<len; i++){
			if(strURL[i] == '/'){
				sla = i;
				break;
			}
			if(strURL[i] == ':'){
				koron = i;
			}
		}

		for(int i=sla + 1; i<len; i++){
			fp.write(strURL[i]);
		}

		fp.write( (unsigned char*)" HTTP/1.1", 9);
		fp.write(0x0D);	fp.write(0x0A);
	}

	//Hostヘッダを生成
	{
		fp.write( (unsigned char*)"Host: ", 6);
	
		if(koron == 0){
			koron = sla;
		}

		for(int i=0; i<koron; i++){
			fp.write(strURL[i]);
		}
		fp.write(0x0D);	fp.write(0x0A);
	}

	//ヘッダ情報が追加されているとき
	if(n >= 3){
		n = RARRAY_LEN( vHeaders );
		mrb_value hes;
		for (int i=0; i<n; i++) {
	
			hes = mrb_ary_ref(mrb, vHeaders, i);
			len = strlen(RSTRING_PTR(hes));
			
			//ヘッダの追記
			fp.write(RSTRING_PTR(hes), len);
			fp.write(0x0D);	fp.write(0x0A);
		}
	}

	//改行のみの行を追加する
	fp.write(0x0D);	fp.write(0x0A);

	fp.flush();
	fp.close();


	//****** AT+CIPSTARTコマンド ******

	//WiFiData[]に、ドメインとポート番号を取得
	for(int i=0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if(i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	if (ssl) {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"SSL\",\"");
	} else {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	}
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if( koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		if (ssl) {
			RbSerial[WIFI_SERIAL]->println("443");
		} else {
			RbSerial[WIFI_SERIAL]->println("80");
		}
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value( 0 );
	}
	//Serial.print("httpServer Connect: ");
	//Serial.print((const char*)WiFiData);

	//****** AT+CIPSEND コマンド ******

	//送信データサイズ取得
	if( !(fp = SD.open(hedFilename, FILE_READ)) ){
		return mrb_fixnum_value( 4 );
	}
	//ファイルサイズ取得
	int sByte = fp.size();
	fp.close();

	//Serial.print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");

	sprintf((char*)WiFiData, "%d", sByte);

	//Serial.println((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value( 0 );
	}

	//Serial.print("> Waiting: ");
	//Serial.print((const char*)WiFiData);

	//****** 送信データ受付モードになったので、http GETデータを送信する ******
	{
		if( !(fp = SD.open(hedFilename, FILE_READ)) ){
			return mrb_fixnum_value( 5 );
		}
		WiFiData[1] = 0;
		for(int i=0; i<sByte; i++){
			WiFiData[0] = (unsigned char)fp.read();
			//Serial.print((const char*)WiFiData);
			RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
		}
		fp.close();

		SD.remove(tmpFilename);		//受信するためファイルを事前に消している

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
			DEBUG_PRINTLN1("WIFI ERR");
			return mrb_fixnum_value( 0 );
		}
		//Serial.print("Send Finish: ");
		//Serial.print((const char*)WiFiData);
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	if( !(fp = SD.open(tmpFilename, FILE_WRITE)) ){
		return mrb_fixnum_value( 6 );
	}

	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	unsigned char recv[2];
	times = millis();

#if BOARD == BOARD_GR
	int led = digitalRead(PIN_LED0) | ( digitalRead(PIN_LED1)<<1) | (digitalRead(PIN_LED2)<<2)| (digitalRead(PIN_LED3)<<3);
#else
	int led = digitalRead(RB_LED);
#endif

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while((len = RbSerial[WIFI_SERIAL]->available()) != 0)
		{
			//LEDを点灯する
#if BOARD == BOARD_GR
			digitalWrite(PIN_LED0, HIGH);
			digitalWrite(PIN_LED1, HIGH);
			digitalWrite(PIN_LED2, HIGH);
			digitalWrite(PIN_LED3, HIGH);
#else
			digitalWrite(RB_LED, HIGH);
#endif
			for(int i=0; i<len; i++){
				recv[0] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
				fp.write( (unsigned char*)recv, 1);
			}
			times = millis();
			wait_msec = 1000;	//データが届き始めたら、1sec待ちに変更する

			//LEDを消灯する
#if BOARD == BOARD_GR
			digitalWrite(PIN_LED0, LOW);
			digitalWrite(PIN_LED1, LOW);
			digitalWrite(PIN_LED2, LOW);
			digitalWrite(PIN_LED3, LOW);
#else
			digitalWrite(RB_LED, LOW);
#endif
		}
	}
	fp.flush();
	fp.close();

	//****** 受信終了 ******
	//Serial.println("Recv Finish");

	//受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
	int ret = CutGarbageData("\r\n+IPD,4,", tmpFilename, strFname);
	if(ret != 1){
		return mrb_fixnum_value( 7 );
	}

	//****** AT+CIPCLOSE コマンド ******
	DEBUG_PRINTLN1("AT+CIPCLOSE=4");
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);
	
	//Serial.println((const char*)WiFiData);

#if BOARD == BOARD_GR
	digitalWrite(PIN_LED0, led & 1);
	digitalWrite(PIN_LED1, (led >> 1) & 1);
	digitalWrite(PIN_LED2, (led >> 2) & 1);
	digitalWrite(PIN_LED3, (led >> 3) & 1);
#else
	digitalWrite(RB_LED, led);
#endif

	return mrb_fixnum_value( 1 );
}

mrb_value mrb_wifi_getSD(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_getSD_sub(mrb, self, 0);
}

mrb_value mrb_wifi_getSD_ssl(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_getSD_sub(mrb, self, 1);
}

//**************************************************
// http GETプロトコルを送信する: WiFi.httpGet
//  WiFi.httpGet( URL[,Headers] )
//　送信のみで、結果を受信しない　
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//**************************************************
mrb_value mrb_wifi_get_sub(mrb_state *mrb, mrb_value self, int ssl)
{
mrb_value vURL, vHeaders;
char	*strURL;
int len = 0;
int sla, cnt;
int koron = 0;
char sData[1024];

	int n = mrb_get_args(mrb, "S|A", &vURL, &vHeaders);

	strURL = RSTRING_PTR(vURL);

	//httpsかチェック
	if (ssl) {
		DEBUG_PRINTLN1("AT+CIPSSLSIZE=4096");
		RbSerial[WIFI_SERIAL]->println("AT+CIPSSLSIZE=4096");
		getData(WIFI_WAIT_MSEC);
	}
	//URLからドメインを分割する
	len = strlen(strURL);
	sla = len;
	for(int i=0; i<len; i++){
		if(strURL[i] == '/'){
			sla = i;
			break;
		}
		if(strURL[i] == ':'){
			koron = i;
		}
	}

	if(koron == 0){
		koron = sla;
	}

	//****** AT+CIPSTARTコマンド ******
	//WiFiData[]に、ドメインとポート番号を取得
	for(int i=0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if(i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	if (ssl) {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"SSL\",\"");
	} else {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	}
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if( koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		if (ssl) {
			RbSerial[WIFI_SERIAL]->println("443");
		} else {
			RbSerial[WIFI_SERIAL]->println("80");
		}
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value( 0 );
	}

	//****** AT+CIPSEND コマンド ******

	//ヘッダの1行目を生成
	{
		strcpy(sData, "GET /");

		cnt = 5;
		for(int i=sla + 1; i<len; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}

		sData[cnt] = 0;
		strcat(sData, " HTTP/1.1\r\n");
	}

	//Hostヘッダを生成
	{
		strcat(sData, "Host: ");
	
		cnt = strlen(sData);
		for(int i=0; i<koron; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}
		sData[cnt] = 0;
		strcat(sData, "\r\n");
	}

	//ヘッダ情報が追加されているとき
	if(n >= 2){
		n = RARRAY_LEN( vHeaders );
		mrb_value hes;
		for (int i=0; i<n; i++) {
	
			hes = mrb_ary_ref(mrb, vHeaders, i);
			len = strlen(RSTRING_PTR(hes));
			
			//ヘッダの追記
			strcat(sData, RSTRING_PTR(hes));
			strcat(sData, "\r\n");
		}
	}

	//改行のみの行を追加する
	strcat(sData, "\r\n");

	//送信データサイズ取得
	len = strlen(sData);

	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->println(len);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value( 0 );
	}

	//****** 送信データ受付モードになったので、http GETデータを送信する ******
	{
		RbSerial[WIFI_SERIAL]->print((const char*)sData);

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
			DEBUG_PRINTLN1("WIFI ERR");
			return mrb_fixnum_value( 0 );
		}
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	times = millis();

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while((len = RbSerial[WIFI_SERIAL]->available()) != 0)
		{
			for(int i=0; i<len; i++){
				RbSerial[WIFI_SERIAL]->read();
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}
	//****** 受信終了 ******

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);

	return mrb_fixnum_value( 1 );
}

mrb_value mrb_wifi_get(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_get_sub(mrb, self, 0);
}

mrb_value mrb_wifi_get_ssl(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_get_sub(mrb, self, 1);
}

//**************************************************
// TCP/UDP接続を閉じる: WiFi.cClose
//  WiFi.cClose(number)
//  number: 接続番号(1～4)
//**************************************************
void wifi_cClose(int num)
{
	RbSerial[WIFI_SERIAL]->print("AT+CIPCLOSE=");
	RbSerial[WIFI_SERIAL]->println(num);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return;
}

mrb_value mrb_wifi_cClose(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);
	wifi_cClose(num);
	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// UDP接続を開始します: WiFi.udpOpen
//  WiFi.udpOpen( number, IP_Address, SendPort, ReceivePort )
//　number: 接続番号(1～4)
//	IP_Address: 通信相手アドレス
//	SendPort: 送信ポート番号
//	ReceivePort: 受信ポート番号
//**************************************************
static int chk_OK()
{
	char *p = (char *)WiFiData;
	int n = strlen((const char *)WiFiData);
	DEBUG_PRINTLN1((const char *)WiFiData);
	if (n >= 4) {
		if ((p[n-4] == 'O') && (p[n-3] == 'K'))
			return 1;
	}
	return 0;
}

static int wifi_udpOpen(int num, char *strIpAdd, int sport, int rport)
{
	//****** AT+CIPSTARTコマンド ******
	RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=");
	RbSerial[WIFI_SERIAL]->print(num);
	RbSerial[WIFI_SERIAL]->print(",\"UDP\",\"");
	RbSerial[WIFI_SERIAL]->print((const char*)strIpAdd);
	RbSerial[WIFI_SERIAL]->print("\",");
	RbSerial[WIFI_SERIAL]->print(sport);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(rport);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	return chk_OK();
}

mrb_value mrb_wifi_udpOpen(mrb_state *mrb, mrb_value self)
{
mrb_value vIpAdd;
char	*strIpAdd;
int	num, sport, rport;

	mrb_get_args(mrb, "iSii", &num, &vIpAdd, &sport, &rport);
	strIpAdd = RSTRING_PTR(vIpAdd);
	wifi_udpOpen(num, strIpAdd, sport, rport);
	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// 指定接続番号にデータを送信します: WiFi.send
//  WiFi.send( number, Data[, length] )
//　number: 接続番号(0～3)
//	Data: 送信するデータ
//　length: 送信データサイズ
//
//  戻り値は
//	  送信データサイズ
//**************************************************
static int wifi_send(int num, char *strdata, int len)
{
	//****** AT+CIPSTARTコマンド ******
	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=");
	RbSerial[WIFI_SERIAL]->print(num);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(len);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return 0;
	}

	//RbSerial[WIFI_SERIAL]->print((const char*)strdata);
	for (int i = 0; i < len; i++) {
		RbSerial[WIFI_SERIAL]->print((char)strdata[i]);
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){

		//タイムアウトと思われるので、強制的にデータサイズの不足分の0x0Dを送信する
		for(int i=0; i<len-(int)strlen(strdata); i++){
			RbSerial[WIFI_SERIAL]->print("\r");
		}
		return 0 ;
	}
	return len;
}

mrb_value mrb_wifi_send(mrb_state *mrb, mrb_value self)
{
mrb_value vdata;
char	*strdata;
int	num, len;

	int n = mrb_get_args(mrb, "iS|i", &num, &vdata, &len);
	strdata = RSTRING_PTR(vdata);

	//送信データサイズが指定されていないとき
	if(n < 3){
		len = RSTRING_LEN(vdata);
	}

	len = wifi_send(num, strdata, len);
	return mrb_fixnum_value( len );
}

//**************************************************
// 指定接続番号からデータを受信します: WiFi.recv
//  WiFi.recv( number )
//　number: 接続番号(0～3)
//
//  戻り値は
//	  受信したデータの配列　ただし、256以下
//**************************************************
static char *wifi_recv_buf[256];
int wifi_recv(int num, char *recv_buf, int *recv_cnt)
{
	unsigned char str[16];
	sprintf((char*)str, "\r\n+IPD,%d,", num);
	//if(RbSerial[WIFI_SERIAL]->available() == 0){
	//	return -1;
	//}

	//****** 受信開始 ******
	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	times = millis();
	int len = strlen((char*)str);
	int cnt = 0;
	unsigned char c;

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		if(RbSerial[WIFI_SERIAL]->available())
		{
			c = (unsigned char)RbSerial[WIFI_SERIAL]->read();

			if(str[cnt] == c){
				cnt++;
				if(cnt == len){
					break;
				}
			}
			else if(c == 0x0D){
				cnt = 1;
			}
			else{
				cnt = 0;
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}

	//ここから後はバイト数が来ているはず
	times = millis();
	cnt = 0;
	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			str[cnt] = 0;
			break;
		}

		if(RbSerial[WIFI_SERIAL]->available())
		{
			c = (unsigned char)RbSerial[WIFI_SERIAL]->read();

			str[cnt] = c;
			if(c == ':'){
				str[cnt] = 0;
				break;
			}
			else if(cnt >= 15){
				str[15] = 0;
				break;
			}
			cnt++;
			times = millis();
		}
	}

	len = atoi((const char*)str);

	//Serial.print("len= ");
	//Serial.println(len);

	//データを取りだします
	times = millis();
	*recv_cnt = 0;
	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		if(RbSerial[WIFI_SERIAL]->available())
		{
			recv_buf[*recv_cnt] = (char)RbSerial[WIFI_SERIAL]->read();
			(*recv_cnt)++;

			if(*recv_cnt >= len){
				break;
			}
			times = millis();
		}
	}
	//****** 受信終了 ******

	return 1;
}

mrb_value mrb_wifi_recv(mrb_state *mrb, mrb_value self)
{
	int num;
	mrb_value arv[256];
	int ret, i, cnt;
	mrb_get_args(mrb, "i", &num);

	ret = wifi_recv(num, (char *)wifi_recv_buf, &cnt);
	if (ret == -1) {
		arv[0] = mrb_fixnum_value(-1);
		return mrb_ary_new_from_values(mrb, 1, arv);
	} else {
		for (i = 0; i < cnt; i++) {
			arv[i] = mrb_fixnum_value((int)wifi_recv_buf[i] & 0xff);
		}
	}
	return mrb_ary_new_from_values(mrb, cnt, arv);
}

//**************************************************
// http POSTとしてSDカードのファイルをPOSTします: WiFi.httpPostSD
//  WiFi.httpPostSD( URL, Headers, Filename )
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//	Filename: POSTするファイル名
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//		2: SDカードが使えない
//		... 各種エラー
//**************************************************
mrb_value mrb_wifi_postSD_sub(mrb_state *mrb, mrb_value self, int ssl)
{
mrb_value vSFname, vURL, vHeaders, vDFname ;
const char *tmpFilename = "wifitmp.tmp";
const char *headFilename = "headfile.tmp";
char	*strSFname, *strURL;
char	*strDFname = (char *)NULL;
int len = 0;
File fp, fd;
int sla, koron;
int sBody, sHeader;

	//SDカードが利用可能か確かめます
	if (!sdcard_Init(mrb)){
		return mrb_fixnum_value(2);
	}

	int n = mrb_get_args(mrb, "SAS|S", &vURL, &vHeaders, &vSFname, &vDFname);

	strSFname = RSTRING_PTR(vSFname);
	strURL = RSTRING_PTR(vURL);
	if (n >= 4) {
		strDFname = RSTRING_PTR(vDFname);
	}

	//送信ファイルサイズ取得
	if (!(fp = SD.open(strSFname, FILE_READ))){
		return mrb_fixnum_value(3);
	}
	//ファイルサイズ取得
	sBody = fp.size();
	fp.close();

	//httpサーバに送信するデータを、strFname ファイルに生成します。

	//既にファイルがあれば消す
	if (SD.exists(headFilename)){
		SD.remove(headFilename);
	}
	//ファイルオープン
	if (!(fp = SD.open(headFilename, FILE_WRITE))){
		return mrb_fixnum_value(4);
	}

	RbSerial[WIFI_SERIAL]->println("AT+CIPMUX=1");
	getData(WIFI_WAIT_MSEC);
	//1行目を生成
	{
		fp.write((unsigned char*)"POST /", 6);

		//httpsかチェック
		if (ssl) {
			DEBUG_PRINTLN1("AT+CIPSSLSIZE=4096");
			RbSerial[WIFI_SERIAL]->println("AT+CIPSSLSIZE=4096");
			getData(WIFI_WAIT_MSEC);
		}
		//URLからドメインを分割する
		len = strlen(strURL);
		sla = len;
		koron = 0;
		for (int i = 0; i<len; i++){
			if (strURL[i] == '/'){
				sla = i;
				break;
			}
			if (strURL[i] == ':'){
				koron = i;
			}
		}

		for (int i = sla + 1; i<len; i++){
			fp.write(strURL[i]);
		}

		fp.write((unsigned char*)" HTTP/1.1", 9);
		fp.write(0x0D);	fp.write(0x0A);
	}

	//Hostヘッダを生成
	{
		fp.write((unsigned char*)"Host: ", 6);

		if (koron == 0){
			koron = sla;
		}

		for (int i = 0; i < koron; i++){
			fp.write(strURL[i]);
		}
		fp.write(0x0D);	fp.write(0x0A);
	}

	//Content-Lengthを付けます
	{
		fp.write((unsigned char*)"Content-Length: ", 16);
		sprintf((char*)WiFiData,"%u", sBody);
		fp.write(WiFiData,strlen((char*)WiFiData));
		fp.write(0x0D);	fp.write(0x0A);
	}

	//ヘッダ情報が追加されているとき
	{
		n = RARRAY_LEN(vHeaders);
		mrb_value hes;
		for (int i = 0; i<n; i++) {

			hes = mrb_ary_ref(mrb, vHeaders, i);
			len = strlen(RSTRING_PTR(hes));

			//ヘッダの追記
			fp.write(RSTRING_PTR(hes), len);
			fp.write(0x0D);	fp.write(0x0A);
		}
	}

	//改行のみの行を追加する
	fp.write(0x0D);	fp.write(0x0A);

	fp.flush();
	fp.close();


	//****** AT+CIPSTARTコマンド ******

	//WiFiData[]に、ドメインとポート番号を取得
	for (int i = 0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if (i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	if (ssl) {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"SSL\",\"");
	} else {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	}
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if (koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		if (ssl) {
			RbSerial[WIFI_SERIAL]->println("443");
		} else {
			RbSerial[WIFI_SERIAL]->println("80");
		}
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if (!(WiFiData[strlen((const char*)WiFiData) - 2] == 'K' || WiFiData[strlen((const char*)WiFiData) - 3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value(0);
	}
	//Serial.print("httpServer Connect: ");
	//Serial.print((const char*)WiFiData);

	//****** AT+CIPSEND コマンド ******
	
	//送信ヘッダのサイズ取得
	if (!(fp = SD.open(headFilename, FILE_READ))){
		return mrb_fixnum_value(5);
	}
	//ファイルサイズ取得
	sHeader = fp.size();
	fp.close();

	//Serial.print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");

	sprintf((char*)WiFiData, "%u", sHeader + sBody);

	//Serial.println((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if (!(WiFiData[strlen((const char*)WiFiData) - 2] == 'K' || WiFiData[strlen((const char*)WiFiData) - 3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value(0);
	}

	//Serial.print("> Waiting: ");
	//Serial.print((const char*)WiFiData);

	//****** 送信データ受付モードになったので、http POSTデータを送信する ******
	{
		//先ずヘッダデータを送信する
		if (!(fp = SD.open(headFilename, FILE_READ))){
			return mrb_fixnum_value(6);
		}
		WiFiData[1] = 0;
		for (int i = 0; i<sHeader; i++){
			WiFiData[0] = (unsigned char)fp.read();
			//Serial.print((const char*)WiFiData);
			RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
		}
		fp.close();

		//先ずボディデータを送信する
		if (!(fp = SD.open(strSFname, FILE_READ))){
			return mrb_fixnum_value(7);
		}
		WiFiData[1] = 0;
		for (int i = 0; i<sBody; i++){
			WiFiData[0] = (unsigned char)fp.read();
			//Serial.print((const char*)WiFiData);
			RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
		}
		fp.close();

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if (!(WiFiData[strlen((const char*)WiFiData) - 2] == 'K' || WiFiData[strlen((const char*)WiFiData) - 3] == 'K')){
			DEBUG_PRINTLN1("WIFI ERR");
			return mrb_fixnum_value(0);
		}
		//Serial.print("Send Finish: ");
		//Serial.print((const char*)WiFiData);
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	if (n >= 4) {
		if (SD.exists(tmpFilename)){
			SD.remove(tmpFilename);
		}
		if( !(fp = SD.open(tmpFilename, FILE_WRITE)) ){
			return mrb_fixnum_value( 6 );
		}
	}

	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	unsigned char recv[2];
	times = millis();

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while((len = RbSerial[WIFI_SERIAL]->available()) != 0)
		{
			for(int i=0; i<len; i++){
				// RbSerial[WIFI_SERIAL]->read();
				recv[0] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
				if (n >= 4) {
					fp.write( (unsigned char*)recv, 1);
				}
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}
	if (n >= 4) {
		fp.flush();
		fp.close();
	}

	//****** 受信終了 ******
	//Serial.println("Recv Finish");

	if (n >= 4) {
		//受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
		int ret = CutGarbageData("\r\n+IPD,4,", tmpFilename, (const char *)strDFname);
		if(ret != 1){
			return mrb_fixnum_value( 7 );
		}
	}

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);

	return mrb_fixnum_value( 1 );
}

mrb_value mrb_wifi_postSD(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_postSD_sub(mrb, self, 0);
}

mrb_value mrb_wifi_postSD_ssl(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_postSD_sub(mrb, self, 1);
}

//**************************************************
// http POSTする: WiFi.httpPost
//  WiFi.httpPost( URL, Headers, data )
//　送信のみで、結果を受信しない
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//　Data: POSTデータ
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//**************************************************
mrb_value mrb_wifi_post_sub(mrb_state *mrb, mrb_value self, int ssl)
{
const char *tmpFilename = "wifitmp.tmp";
mrb_value vData, vURL, vHeaders, vDFname;
char	*strData;
char	*strURL;
char	*strDFname;
int		sBody, sHeader;
int sla, cnt;
int koron = 0;
char sData[1024];
int len;
File fp, fd;

	int n = mrb_get_args(mrb, "SAS|S", &vURL, &vHeaders, &vData, &vDFname);

	strData = RSTRING_PTR(vData);
	sBody = strlen(strData);
	strURL = RSTRING_PTR(vURL);
	if (n >= 4) {
		strDFname = RSTRING_PTR(vDFname);
	}

	RbSerial[WIFI_SERIAL]->println("AT+CIPMUX=1");
	getData(WIFI_WAIT_MSEC);
	//httpsかチェック
	if (ssl) {
		DEBUG_PRINTLN1("AT+CIPSSLSIZE=4096");
		RbSerial[WIFI_SERIAL]->println("AT+CIPSSLSIZE=4096");
		getData(WIFI_WAIT_MSEC);
	}
	//URLからドメインを分割する
	len = strlen(strURL);
	sla = len;
	for(int i=0; i<len; i++){
		if(strURL[i] == '/'){
			sla = i;
			break;
		}
		if(strURL[i] == ':'){
			koron = i;
		}
	}

	if(koron == 0){
		koron = sla;
	}

	//****** AT+CIPSTARTコマンド ******
	//WiFiData[]に、ドメインとポート番号を取得
	for(int i=0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if(i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	if (ssl) {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"SSL\",\"");
	} else {
		RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	}
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if( koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		if (ssl) {
			RbSerial[WIFI_SERIAL]->println("443");
		} else {
			RbSerial[WIFI_SERIAL]->println("80");
		}
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value( 0 );
	}

	//****** AT+CIPSEND コマンド ******

	//ヘッダの1行目を生成
	{
		strcpy(sData, "POST /");

		cnt = 6;
		for(int i=sla + 1; i<len; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}

		sData[cnt] = 0;
		strcat(sData, " HTTP/1.1\r\n");
	}

	//Hostヘッダを生成
	{
		strcat(sData, "Host: ");
	
		cnt = strlen(sData);
		for(int i=0; i<koron; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}
		sData[cnt] = 0;
		strcat(sData, "\r\n");
	}

	//Content-Lengthを付けます
	{
		strcat(sData, "Content-Length: ");

		sprintf((char*)WiFiData,"%d", sBody);
		strcat(sData, (char*)WiFiData);
		strcat(sData, "\r\n");
	}

	//ヘッダ情報を付けます
	{
		int n = RARRAY_LEN( vHeaders );
		mrb_value hes;
		for (int i=0; i<n; i++) {
			hes = mrb_ary_ref(mrb, vHeaders, i);
			
			//ヘッダの追記
			strcat(sData, RSTRING_PTR(hes));
			strcat(sData, "\r\n");
		}
	}

	//改行のみの行を追加する
	strcat(sData, "\r\n");

	//送信データサイズ取得
	sHeader = strlen(sData);
	len = sHeader + sBody;


	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->println(len);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		DEBUG_PRINTLN1("WIFI ERR");
		return mrb_fixnum_value( 0 );
	}

	//****** 送信データ受付モードになったので、http POSTデータを送信する ******
	{
		//ヘッダを送信する
		RbSerial[WIFI_SERIAL]->print((const char*)sData);
		//Serial.print(sData);

		//ボディを送信する
		RbSerial[WIFI_SERIAL]->print((const char*)strData);
		//Serial.print(strData);

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
			DEBUG_PRINTLN1("WIFI ERR");
			return mrb_fixnum_value( 0 );
		}
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	if (n >= 4) {
		if (SD.exists(tmpFilename)){
			SD.remove(tmpFilename);
		}
		if( !(fp = SD.open(tmpFilename, FILE_WRITE)) ){
			return mrb_fixnum_value( 6 );
		}
	}

	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	unsigned char recv[2];
	times = millis();

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while((len = RbSerial[WIFI_SERIAL]->available()) != 0)
		{
			for(int i=0; i<len; i++){
				//RbSerial[WIFI_SERIAL]->read();
				recv[0] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
				if (n >= 4) {
					fp.write( (unsigned char*)recv, 1);
				}
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}
	if (n >= 4) {
		fp.flush();
		fp.close();
	}
	//****** 受信終了 ******
	if (n >= 4) {
		//受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
		int ret = CutGarbageData("\r\n+IPD,4,", tmpFilename, (const char *)strDFname);
		if(ret != 1){
			return mrb_fixnum_value( 7 );
		}
	}

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);

	return mrb_fixnum_value( 1 );
}

mrb_value mrb_wifi_post(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_post_sub(mrb, self, 0);
}

mrb_value mrb_wifi_post_ssl(mrb_state *mrb, mrb_value self)
{
	return mrb_wifi_post_sub(mrb, self, 1);
}

//**************************************************
// httpサーバを開始します: WiFi.httpServer
//  WiFi.httpServer( [Port] )
//　Port: 待ちうけポート番号
//　　　　-1: サーバ停止
//
//　SDカードが必須となります。
//　ポート番号を省略したときはアクセス確認します
//　戻り値
//　0: アクセスはありません
//　1: アクセスあり
//　2: SDカードが使えません
//　3: ファイルのアクセスに失敗しました
//
//　クライアントからアクセスがあるとき、通信内容と接続番号の2つが返ります
//  GET: パスが返ります
//  GET以外、ヘッダの1行目が返ります
//  ,
//  接続番号が返ります
//
//  revData, conNum = WiFi.httpServer()
//**************************************************
mrb_value mrb_wifi_server(mrb_state *mrb, mrb_value self)
{
const char *tmpFilename = "header.tmp";
const char *headFilename = "header.txt";
char ipd[] = { 0x0d, 0x0a, '+', 'I', 'P', 'D', ',', '0', ',', 0x00 };
int len = 0;
File fp;
int	port = 80;
int sesnum = 0;
int headerSize = 0;
int rc, posi;
mrb_value arv[2];

	int n = mrb_get_args(mrb, "|i", &port);

	if(n < 1 ){
		if(RbSerial[WIFI_SERIAL]->available() > 0){

			SD.remove(tmpFilename);		//受信するためファイルを事前に消している

			if( !(fp = SD.open(tmpFilename, FILE_WRITE)) ){
				return mrb_fixnum_value( 3 );
			}

			unsigned long times;
			unsigned int wait_msec = WIFI_WAIT_MSEC;
			unsigned char recv[2];
			times = millis();

			while(true){
				//wait_msec 待つ
				if(millis() - times > wait_msec){
					break;
				}

				while((len = RbSerial[WIFI_SERIAL]->available()) != 0)
				{
					for(int i=0; i<len; i++){
						recv[0] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
						fp.write( (unsigned char*)recv, 1);
					}
					times = millis();
					wait_msec = 1000;	//データが届き始めたら、1sec待ちに変更する
				}
			}
			fp.flush();
			fp.close();
		
			//セッション番号を取得します
			{
				if (!(fp = SD.open(tmpFilename, FILE_READ))){
					return mrb_fixnum_value(3);
				}
				headerSize = fp.size();

				unsigned char rcvc;
				posi = 0;
				//WiFiData[]に先頭行を取得します
				for (int i = 0; i < headerSize; i++){
					rcvc = (unsigned char)fp.read();

					if (posi == 0 && rcvc == '+'){
						posi++;
					}
					else if (posi== 1 && rcvc == 'I'){
						posi++;
					}
					else if (posi == 2 && rcvc == 'P'){
						posi++;
					}
					else if (posi == 3 && rcvc == 'D'){
						posi++;
					}
					else if (posi == 4 && rcvc == ','){
						// "+IPD,"を見つけた
						break;
					}
					else{
						posi = 0;
					}
				}
				if (posi == 4){
					sesnum = fp.read() - 0x30;
				}
				fp.close();
			}

			//受信データに '\r\n+\r\n+IPD,0,****:'というデータがあるので削除します
			ipd[7] = sesnum + 0x30;
			int ret = CutGarbageData((const char*)ipd, tmpFilename, headFilename);
			if(ret != 1){
				return mrb_fixnum_value( 3 );
			}

			//***** GETならパスを返します ******
			unsigned char *uc;
			{
				if( !(fp = SD.open(headFilename, FILE_READ)) ){
					return mrb_fixnum_value( 3 );
				}
				headerSize = fp.size();

				//WiFiData[]に先頭行を取得します
				for(int i=0; i<headerSize; i++){
					rc = fp.read();
					WiFiData[i] = (unsigned char)rc;
					if(i >= 255){
						posi = i;
						break;
					}
					if(WiFiData[i] == 0x0A){
						posi = i + 1;
						break;
					}
				}
				WiFiData[posi] = 0;

				fp.close();
			}

			if( (posi < 6) || !(WiFiData[0] == 'G' && WiFiData[1] == 'E' && WiFiData[2] == 'T')){
				arv[0] = mrb_str_new_cstr(mrb, (const char*)WiFiData);
				arv[1] = mrb_fixnum_value(sesnum);
				return mrb_ary_new_from_values(mrb, 2, arv);
			}

			// 先頭の'/'を探します
			posi = -1;
			for(int i=0; i<headerSize; i++){
				if(WiFiData[i] == '/'){
					posi = i;
					break;
				}
			}

			if(posi == -1){
				arv[0] = mrb_str_new_cstr(mrb, (const char*)WiFiData);
				arv[1] = mrb_fixnum_value(sesnum);
				return mrb_ary_new_from_values(mrb, 2, arv);
			}

			//posi以降のスペースを探してnullを入れます
			for(int i=posi + 1; i<headerSize; i++){
				if(WiFiData[i] == ' '){
					WiFiData[i] = 0;
					break;
				}
			}
		
			uc = &WiFiData[posi];
			arv[0] = mrb_str_new_cstr(mrb, (const char*)uc);
			arv[1] = mrb_fixnum_value(sesnum);
			return mrb_ary_new_from_values(mrb, 2, arv);
		}
		else{
			//データ無し
			return mrb_fixnum_value( 0 );
		}
	}

	//SDカードが利用可能か確かめます
	if (!sdcard_Init(mrb)){
		return mrb_fixnum_value( 2 );
	}

	//****** AT+CIPSERVERコマンド ******
	if(port < 0){
		RbSerial[WIFI_SERIAL]->println("AT+CIPSERVER=0");
	}
	else{
		RbSerial[WIFI_SERIAL]->print("AT+CIPSERVER=1,");
		RbSerial[WIFI_SERIAL]->println(port);
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

static char hex_to_num(char ch)
{
	if (('0' <= ch) && (ch <= '9')) {
		return (ch - '0');
	} else if (('A' <= ch) && (ch <= 'F')) {
		return (ch - 'A' + 10);
	} else if (('a' <= ch) && (ch <= 'f')) {
		return (ch - 'a' + 10);
	}
	return ch;
}

#define URL_BUF_SIZE 1024
static char url_buf[URL_BUF_SIZE];
static char hex[] = "0123456789ABCDEF";
static char num_to_hex(char num) {
	return hex[num & 0xf];
}

static int _isalnum(char ch)
{
	if ((('0' <= ch) && (ch <= '9')) || (('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z')))
		return 1;
	else
		return 0;
}

static char *_url_encode(char *str, char *dst, int size)
{
	char *pstr = str;
	char *pbuf = dst;
	if (size == 0)
		return dst;
	size--;
	while (size && (*pstr)) {
		if (_isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') {
			*pbuf++ = *pstr;
			size--;
		} else if (*pstr == ' ') {
			*pbuf++ = '+';
			size --;
		} else {
			*pbuf++ = '%';
			size--; if (size == 0) break;
			*pbuf++ = num_to_hex(*pstr >> 4);
			size--; if (size == 0) break;
			*pbuf++ = num_to_hex(*pstr & 0xf);
		}
		pstr++;
	}
	*pbuf = '\0';
	return dst;
}

static char *_url_decode(char *str, char *dst, int size)
{
	char *pstr = str;
	char *pbuf = dst;
	if (size == 0)
		return dst;
	size --;
	while (size && (*pstr)) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = hex_to_num(pstr[1]) << 4 | hex_to_num(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') {
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
		size--;
	}
	*pbuf = '\0';
	return dst;
}

mrb_value mrb_url_encode(mrb_state *mrb, mrb_value self)
{
	mrb_value vstr;
	char *str;
	char *dst = (char *)url_buf;

	mrb_get_args(mrb, "S", &vstr);
	str = RSTRING_PTR(vstr);
	dst = _url_encode(str, dst, URL_BUF_SIZE);
	return mrb_str_new_cstr(mrb, (const char*)dst);
}

mrb_value mrb_url_decode(mrb_state *mrb, mrb_value self)
{
	mrb_value vstr;
	char *str;
	char *dst = (char *)url_buf;

	mrb_get_args(mrb, "S", &vstr);
	str = RSTRING_PTR(vstr);
	dst = _url_decode(str, dst, URL_BUF_SIZE);
	return mrb_str_new_cstr(mrb, (const char*)dst);
}

mrb_value mrb_mktime(mrb_state *mrb, mrb_value self)
{
	mrb_value vtm;
	int n;
	int itm[6];
	time_t	t;
	struct tm work_tm;

	for (int i = 0; i < 6; i++) {
		itm[i] = 0;
	}
	mrb_get_args(mrb, "A", &vtm);
	n = RARRAY_LEN(vtm);
	if (n > 6)
		n = 6;
	for (int i = 0; i < n; i++) {
		itm[i] = mrb_fixnum(mrb_ary_ref(mrb, vtm, i));
	}
    work_tm.tm_year = itm[0] - 1900;
    work_tm.tm_mon = itm[1];
    work_tm.tm_mday = itm[2];
    work_tm.tm_hour = itm[3];
    work_tm.tm_min = itm[4];
    work_tm.tm_sec = itm[5];
    work_tm.tm_isdst = -1;
    t= mktime(&work_tm);
    return mrb_fixnum_value(t);
}

//**************************************************
// hmac-sha1
// ref: http://www.deadhat.com/wlancrypto/hmac_sha1.c
//**************************************************

/****************************************************************/
/* 802.11i HMAC-SHA-1 Test Code                                 */
/* Copyright (c) 2002, David Johnston                           */
/* Author: David Johnston                                       */
/* Email (home): dj@deadhat.com                                 */
/* Email (general): david.johnston@ieee.org                     */
/* Version 0.1                                                  */
/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*                                                              */
/* This code implements the NIST HMAC-SHA-1 algorithm as used   */
/* the IEEE 802.11i security spec.                              */
/*                                                              */
/* Supported message length is limited to 4096 characters       */
/* ToDo:                                                        */
/*   Sort out endian tolerance. Currently little endian.        */
/****************************************************************/

#define MAX_MESSAGE_LENGTH 2048

/****************************************/
/* sha1()                               */
/* Performs the NIST SHA-1 algorithm    */
/****************************************/
static unsigned long int ft(int t, unsigned long int x, unsigned long int y, unsigned long int z)
{
	unsigned long int a,b,c;
	if (t < 20) {
		a = x & y;
		b = (~x) & z;
		c = a ^ b;
	} else if (t < 40) {
		c = x ^ y ^ z;
	} else if (t < 60) {
		a = x & y;
		b = a ^ (x & z);
		c = b ^ (y & z);
	} else if (t < 80) {
		c = (x ^ y) ^ z;
	}
	return c;
}

static unsigned long int k(int t)
{
	unsigned long int c;
	if (t < 20) {
		c = 0x5a827999;
	} else if (t < 40) {
		c = 0x6ed9eba1;
	} else if (t < 60) {
		c = 0x8f1bbcdc;
	} else if (t < 80) {
		c = 0xca62c1d6;
	}
	return c;
}

//static unsigned long int rotr(int bits, unsigned long int a)
//{
//	unsigned long int c,d,e,f,g;
//	c = (0x0001 << bits)-1;
//	d = ~c;
//	e = (a & d) >> bits;
//	f = (a & c) << (32 - bits);
//	g = e | f;
//	return (g & 0xffffffff );
//}

static unsigned long int rotl(int bits, unsigned long int a)
{
	unsigned long int c,d,e,f,g;
	c = (0x0001 << (32-bits))-1;
	d = ~c;
	e = (a & c) << bits;
	f = (a & d) >> (32 - bits);
	g = e | f;
	return (g & 0xffffffff );
}

static void sha1(unsigned char *message, int message_length, unsigned char *digest)
{
	int i;
	int num_blocks;
	int block_remainder;
	int padded_length;

	unsigned long int l;
	unsigned long int t;
	unsigned long int h[5];
	unsigned long int a, b, c, d, e;
	unsigned long int w[80];
	unsigned long int temp;

	/* Calculate the number of 512 bit blocks */
	padded_length = message_length + 8; /* Add length for l */
	padded_length = padded_length + 1; /* Add the 0x01 bit postfix */

	l = message_length * 8;

	num_blocks = padded_length / 64;
	block_remainder = padded_length % 64;

	if (block_remainder > 0) {
		num_blocks++;
	}

	padded_length = padded_length + (64 - block_remainder);

	/* clear the padding field */
	for (i = message_length; i < (num_blocks * 64); i++) {
		message[i] = 0x00;
	}

	/* insert b1 padding bit */
	message[message_length] = 0x80;

	/* Insert l */
	message[(num_blocks * 64) - 1] = (unsigned char) (l & 0xff);
	message[(num_blocks * 64) - 2] = (unsigned char) ((l >> 8) & 0xff);
	message[(num_blocks * 64) - 3] = (unsigned char) ((l >> 16) & 0xff);
	message[(num_blocks * 64) - 4] = (unsigned char) ((l >> 24) & 0xff);

	/* Set initial hash state */
	h[0] = 0x67452301;
	h[1] = 0xefcdab89;
	h[2] = 0x98badcfe;
	h[3] = 0x10325476;
	h[4] = 0xc3d2e1f0;

	for (i = 0; i < num_blocks; i++) {
		/* Prepare the message schedule */
		for (t = 0; t < 80; t++) {
			if (t < 16) {
				w[t] = (256 * 256 * 256) * message[(i * 64) + (t * 4)];
				w[t] += (256 * 256) * message[(i * 64) + (t * 4) + 1];
				w[t] += (256) * message[(i * 64) + (t * 4) + 2];
				w[t] += message[(i * 64) + (t * 4) + 3];
			} else if (t < 80) {
				w[t] = rotl(1, (w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]));
			}
		}
		/* Initialize the five working variables */
		a = h[0];
		b = h[1];
		c = h[2];
		d = h[3];
		e = h[4];

		/* iterate a-e 80 times */
		for (t = 0; t < 80; t++) {
			temp = (rotl(5, a) + ft(t, b, c, d)) & 0xffffffff;
			temp = (temp + e) & 0xffffffff;
			temp = (temp + k(t)) & 0xffffffff;
			temp = (temp + w[t]) & 0xffffffff;
			e = d;
			d = c;
			c = rotl(30, b);
			b = a;
			a = temp;
		}

		/* compute the ith intermediate hash value */
		h[0] = (a + h[0]) & 0xffffffff;
		h[1] = (b + h[1]) & 0xffffffff;
		h[2] = (c + h[2]) & 0xffffffff;
		h[3] = (d + h[3]) & 0xffffffff;
		h[4] = (e + h[4]) & 0xffffffff;
	}

	digest[3] = (unsigned char) (h[0] & 0xff);
	digest[2] = (unsigned char) ((h[0] >> 8) & 0xff);
	digest[1] = (unsigned char) ((h[0] >> 16) & 0xff);
	digest[0] = (unsigned char) ((h[0] >> 24) & 0xff);
	digest[7] = (unsigned char) (h[1] & 0xff);
	digest[6] = (unsigned char) ((h[1] >> 8) & 0xff);
	digest[5] = (unsigned char) ((h[1] >> 16) & 0xff);
	digest[4] = (unsigned char) ((h[1] >> 24) & 0xff);
	digest[11] = (unsigned char) (h[2] & 0xff);
	digest[10] = (unsigned char) ((h[2] >> 8) & 0xff);
	digest[9] = (unsigned char) ((h[2] >> 16) & 0xff);
	digest[8] = (unsigned char) ((h[2] >> 24) & 0xff);
	digest[15] = (unsigned char) (h[3] & 0xff);
	digest[14] = (unsigned char) ((h[3] >> 8) & 0xff);
	digest[13] = (unsigned char) ((h[3] >> 16) & 0xff);
	digest[12] = (unsigned char) ((h[3] >> 24) & 0xff);
	digest[19] = (unsigned char) (h[4] & 0xff);
	digest[18] = (unsigned char) ((h[4] >> 8) & 0xff);
	digest[17] = (unsigned char) ((h[4] >> 16) & 0xff);
	digest[16] = (unsigned char) ((h[4] >> 24) & 0xff);
}

/******************************************************/
/* hmac-sha1()                                        */
/* Performs the hmac-sha1 keyed secure hash algorithm */
/******************************************************/
/* Moving local variables to static variables for not using stack  */
static unsigned char k0[64];
static unsigned char k0xorIpad[64];
static unsigned char step7data[64];
static unsigned char step8data[64 + 20];
static unsigned char step5data[MAX_MESSAGE_LENGTH + 128];

static void hmac_sha1(unsigned char *key, int key_length, unsigned char *data, int data_length, unsigned char *digest)
{
	int b = 64; /* blocksize */
	unsigned char ipad = 0x36;
	unsigned char opad = 0x5c;
	int i;

	for (i = 0; i < 64; i++) {
		k0[i] = 0x00;
	}
	if (key_length != b) /* Step 1 */
	{
		/* Step 2 */
		if (key_length > b) {
			sha1(key, key_length, digest);
			for (i = 0; i < 20; i++) {
				k0[i] = digest[i];
			}
		} else if (key_length < b) /* Step 3 */
		{
			for (i = 0; i < key_length; i++) {
				k0[i] = key[i];
			}
		}
	} else {
		for (i = 0; i < b; i++) {
			k0[i] = key[i];
		}
	}
	/* Step 4 */
	for (i = 0; i < 64; i++) {
		k0xorIpad[i] = k0[i] ^ ipad;
	}
	/* Step 5 */
	for (i = 0; i < 64; i++) {
		step5data[i] = k0xorIpad[i];
	}
	for (i = 0; i < data_length; i++) {
		step5data[i + 64] = data[i];
	}
	/* Step 6 */
	sha1(step5data, data_length + b, digest);
	/* Step 7 */
	for (i = 0; i < 64; i++) {
		step7data[i] = k0[i] ^ opad;
	}
	/* Step 8 */
	for (i = 0; i < 64; i++) {
		step8data[i] = step7data[i];
	}
	for (i = 0; i < 20; i++) {
		step8data[i + 64] = digest[i];
	}
	/* Step 9 */
	sha1(step8data, b + 20, digest);
}

#define DIGEST_SIZE	20
static unsigned char digest[DIGEST_SIZE];

//**************************************************
// sha1 hash計算
//　戻り値
// string
// ToDo: size error handling
//**************************************************
mrb_value mrb_hmac_sha1(mrb_state *mrb, mrb_value self)
{
	mrb_value vkey, vdata;
	char *key;
	char *data;
	int key_len;
	int data_len;

	mrb_get_args(mrb, "SiSi", &vdata, &data_len, &vkey, &key_len);
	key = RSTRING_PTR(vkey);
	data = RSTRING_PTR(vdata);
	hmac_sha1((unsigned char *)key, key_len, (unsigned char *)data, data_len, (unsigned char *)digest);
	return mrb_str_new(mrb, (const char*)digest, DIGEST_SIZE);
}

//**************************************************
// base64 encode and decode
// http://www.mycplus.com/source-code/c-source-code/base64-encode-decode/
//**************************************************
static char encoding_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
		'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
		'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
static char decoding_init = 0;
static char decoding_table[256] = {0};
static int mod_table[] = { 0, 2, 1 };
#define BASE64_MAX	1024
static unsigned char base64_buf[BASE64_MAX];

static char *base64_encode(const unsigned char *data, int input_length, char *encoded_data, int *output_length)
{
	*output_length = 4 * ((input_length + 2) / 3);

	for (int i = 0, j = 0; i < input_length;) {

		uint32_t octet_a = i < input_length ? (unsigned char) data[i++] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char) data[i++] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char) data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[*output_length - 1 - i] = '=';
	return encoded_data;
}

static void build_decoding_table()
{
	for (int i = 0; i < 64; i++)
		decoding_table[(unsigned char) encoding_table[i]] = i;
}

static char *base64_decode(const unsigned char *data, int input_length, char *decoded_data, int *output_length)
{
	if (decoding_init == 0) {
		build_decoding_table();
		decoding_init = 1;
	}
	if (input_length % 4 != 0)
		return NULL;
	*output_length = input_length / 4 * 3;
	if (data[input_length - 1] == '=')
		(*output_length)--;
	if (data[input_length - 2] == '=')
		(*output_length)--;

	for (int i = 0, j = 0; i < input_length;) {
		uint32_t sextet_a =
				data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t sextet_b =
				data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t sextet_c =
				data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t sextet_d =
				data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

		uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6)
				+ (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

		if (j < *output_length)
			decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *output_length)
			decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *output_length)
			decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
	}
	return decoded_data;
}

//**************************************************
// base64 エンコード
//　戻り値
// string
// ToDo: size error handling
//**************************************************
mrb_value mrb_base64_encode(mrb_state *mrb, mrb_value self)
{
	mrb_value vdata;
	char *data;
	int slen, dlen;

	mrb_get_args(mrb, "Si", &vdata, &slen);
	data = RSTRING_PTR(vdata);
	base64_encode((const unsigned char *)data, slen, (char *)base64_buf, &dlen);
	return mrb_str_new(mrb, (const char*)base64_buf, dlen);
}

//**************************************************
// base64 デコード
//　戻り値
// string
// ToDo: size error handling
//**************************************************
mrb_value mrb_base64_decode(mrb_state *mrb, mrb_value self)
{
	mrb_value vdata;
	char *data;
	int slen, dlen;

	mrb_get_args(mrb, "Si", &vdata, &slen);
	data = RSTRING_PTR(vdata);
	base64_decode((const unsigned char *)data, slen, (char *)base64_buf, &dlen);
	return mrb_str_new(mrb, (const char*)base64_buf, dlen);
}

//**************************************************
// ntp serverからtransfer timeを取得します
//  WiFi.ntp( ntp_server_ip, [time_flag] )
//
//　戻り値
//　ntp transfer time (time_flag == 0)
//　ntp transfer time in unix time format (time_flag == 1)
//**************************************************
#define NTP_PACKT_SIZE	48
static unsigned char ntp_send[NTP_PACKT_SIZE] =
{
	0xe3, 0x00, 0x06, 0xec, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static unsigned char ntp_recv[60];
#define NTP_SEND_PORT	123
#define NTP_LOCAL_PORT	8788

mrb_value mrb_ntp(mrb_state *mrb, mrb_value self)
{
	mrb_value vts;
	char *ts;
	int n;
	int tf;
	int cnt;
	int ret;
	int num = 1;
	uint32_t time = 0;

	n = mrb_get_args(mrb, "S|i", &vts, &tf);
	ts = RSTRING_PTR(vts);
	ret = wifi_udpOpen(num, ts, NTP_SEND_PORT, NTP_LOCAL_PORT);
	if (!ret) {
		DEBUG_PRINTLN1("wifi_udpOpen ERR")
		return mrb_fixnum_value(-1);
	}
	ret = wifi_send(num, (char *)ntp_send, 48);
	if (ret == 0) {
		DEBUG_PRINTLN1("wifi_send ERR")
		return mrb_fixnum_value(-1);
	}
	ret = wifi_recv(num, (char *)ntp_recv, &cnt);
	if (ret != 1) {
		DEBUG_PRINTLN1("wifi_recv ERR")
		return mrb_fixnum_value(-1);
	}
	wifi_cClose(num);
	time = ((uint32_t)ntp_recv[40] << 24) +
			((uint32_t)ntp_recv[41] << 16) +
			((uint32_t)ntp_recv[42] << 8) +
			((uint32_t)ntp_recv[43] << 0);
	if ((n == 2) && (tf == 1)) {
		time -= 2208988800;	// conversion to Unixtime
	}
	return mrb_fixnum_value(time);
}

//**************************************************
// ライブラリを定義します
//**************************************************
int esp8266_Init(mrb_state *mrb)
{	
	//ESP8266からの受信を出力しないに設定
	WiFiRecvOutlNum = -1;

	//CTS用にPIN15をOUTPUTに設定します
	//pinMode(wrb2sakura(WIFI_CTS), 1);
	//digitalWrite(wrb2sakura(WIFI_CTS), 1);

	//WiFiのシリアル3を設定
	//シリアル通信の初期化をします
	RbSerial[WIFI_SERIAL]->begin(WIFI_BAUDRATE);
	int len;
	int ret;
	int cnt = 0;

	while(true){
		//受信バッファを空にします
		while((len = RbSerial[WIFI_SERIAL]->available()) > 0){
			//RbSerial[0]->print(len);
			for(int i=0; i<len; i++){
				RbSerial[WIFI_SERIAL]->read();
			}
		}

		//ECHOオフコマンドを送信する
		RbSerial[WIFI_SERIAL]->println("ATE0");

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読む
		ret = getData(500);
		if(ret == 1){
			//1の時は、WiFiが使用可能
			break;
		}
		else if(ret == 0){
			//タイムアウトした場合は WiFiが使えないとする
			return 0;
		}

		//0,1で無いときは256バイト以上が返ってきている
		cnt++;
		if( cnt >= 3){
			//3回ATE0を試みてダメだったら、あきらめる。
			return 0;
		}
	}


	struct RClass *wifiModule = mrb_define_module(mrb, WIFI_CLASS);

	mrb_define_module_function(mrb, wifiModule, "at", mrb_wifi_at, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "serialOut", mrb_wifi_Sout, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "cwmode", mrb_wifi_Cwmode, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, wifiModule, "setMode", mrb_wifi_Cwmode, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "cwjap", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, wifiModule, "connect", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, wifiModule, "cifsr", mrb_wifi_Cifsr, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "ipconfig", mrb_wifi_Cifsr, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, wifiModule, "multiConnect", mrb_wifi_multiConnect, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "softAP", mrb_wifi_softAP, MRB_ARGS_REQ(4));
	mrb_define_module_function(mrb, wifiModule, "connectedIP", mrb_wifi_connectedIP, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "dhcp", mrb_wifi_dhcp, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, wifiModule, "httpGetSD", mrb_wifi_getSD, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "httpGet", mrb_wifi_get, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "udpOpen", mrb_wifi_udpOpen, MRB_ARGS_REQ(4));

	mrb_define_module_function(mrb, wifiModule, "send", mrb_wifi_send, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "recv", mrb_wifi_recv, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "httpPostSD", mrb_wifi_postSD, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "httpPost", mrb_wifi_post, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "cClose", mrb_wifi_cClose, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "httpServer", mrb_wifi_server, MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "version", mrb_wifi_Version, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "disconnect", mrb_wifi_Disconnect, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, wifiModule, "bypass", mrb_wifi_bypass, MRB_ARGS_NONE());

/* The followings are added by KS */

	mrb_define_module_function(mrb, wifiModule, "httpsGetSD", mrb_wifi_getSD_ssl, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "httpsGet", mrb_wifi_get_ssl, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "httpsPostSD", mrb_wifi_postSD_ssl, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "httpsPost", mrb_wifi_post_ssl, MRB_ARGS_REQ(3)|MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "url_encode", mrb_url_encode, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, wifiModule, "url_decode", mrb_url_decode, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "mktime", mrb_mktime, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "hmac_sha1", mrb_hmac_sha1, MRB_ARGS_REQ(4));

	mrb_define_module_function(mrb, wifiModule, "base64_encode", mrb_base64_encode, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, wifiModule, "base64_decode", mrb_base64_decode, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, wifiModule, "ntp", mrb_ntp, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	return 1;
}
