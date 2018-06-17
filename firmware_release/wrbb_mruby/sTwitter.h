/*
 * sTwitter.h
 *
 * Copyright (c) 2018 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */

#ifndef STWITTER_H_
#define STWITTER_H_

class Twitter {
public:
	Twitter();
	virtual ~Twitter();
	void set_keys(char *cons_key, char *cons_sec, char *accs_key, char *accs_sec);
	void statuses_update(char *str, char *media_id_string);
	//void upload(char *media_id_string, char *buf, int size);
	//void upload_and_statuses_update(char *str, char *media_id_string, char *buf, int size);
private:
	char *_cons_key;
	char *_cons_sec;
	char *_accs_key;
	char *_accs_sec;
};

//extern TWITTERClass TWITTER;

//**************************************************
// ライブラリを定義します
//**************************************************
void twitter_Init(mrb_state *mrb);

#endif /* STWITTER_H_ */
