/*
 * sUtil.cpp
 *
 * Copyright (c) 2018 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */
#include <Arduino.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include "../wrbb.h"
#include "sUtil.h"

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)		{ Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#  define DEBUG_PRINTH(m,v)		{ Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v), 16); }
#else
#  define DEBUG_PRINT(m,v)		// do nothing
#  define DEBUG_PRINTH(m,v)		// do nothing
#endif

#define MAX_HEAP_SIZE	0x100000

// https://stackoverflow.com/questions/14386856/c-check-available-ram

static void* AllocateLargestFreeBlock(size_t* Size) {
	size_t s0, s1;
	void* p;

	//s0 = ~(size_t)0 ^ (~(size_t)0 >> 1);
	s0 = MAX_HEAP_SIZE;

	while (s0 && (p = malloc(s0)) == NULL)
		s0 >>= 1;

	if (p)
		free(p);

	DEBUG_PRINTH("malloc OK", s0);
	DEBUG_PRINTH("  addr", (int)p);
	s1 = s0 >> 1;

	while (s1) {
		if ((p = malloc(s0 + s1)) != NULL) {
			s0 += s1;
			free(p);
			DEBUG_PRINTH("malloc OK", s0);
			DEBUG_PRINTH("  addr", (int)p);
		}
		s1 >>= 1;
	}

	while (s0 && (p = malloc(s0)) == NULL)
		s0 ^= s0 & -s0;

	DEBUG_PRINTH("malloc result", s0);
	DEBUG_PRINTH("  addr", (int)p);
	*Size = s0;
	return p;
}

static size_t GetFreeSize(void) {
	size_t total = 0;
	void* pFirst = NULL;
	void* pLast = NULL;

	for (;;) {
		size_t largest;
		void* p = AllocateLargestFreeBlock(&largest);

		if (largest < sizeof(void*)) {
			if (p != NULL)
				free(p);
			break;
		}

		*(void**)p = NULL;

		total += largest;

		if (pFirst == NULL)
			pFirst = p;

		if (pLast != NULL)
			*(void**)pLast = p;

		pLast = p;
	}

	while (pFirst != NULL) {
		void* p = *(void**)pFirst;
		free(pFirst);
		pFirst = p;
	}

	DEBUG_PRINT("malloc total", total);
	return total;
}

//**************************************************
// 使用可能なヒープメモリサイズを返します
//**************************************************
mrb_value mrb_util_free_size(mrb_state *mrb, mrb_value self)
{
	int free_size = (int)GetFreeSize();
	return mrb_fixnum_value(free_size);
}

//**************************************************
// ライブラリを定義します
//**************************************************
void util_Init(mrb_state *mrb)
{
	struct RClass *utilModule = mrb_define_module(mrb, "Util");

	mrb_define_module_function(mrb, utilModule, "free_size", mrb_util_free_size, MRB_ARGS_NONE());
}
