#pragma once

#include "../main.h"

#define PROFILE_START() static uint _profIndex = 0; \
	if (_profIndex == 0) { \
		_profIndex = profGetIndex(__PRETTY_FUNCTION__); \
	 } \
	profStart(_profIndex)

#define PROFILE_STOP() profStop(_profIndex)

uint   profGetIndex(const char *funcName);
void   profStart   (uint i);
void   profStop    (uint i);
u64    profGetTotal(uint i);
double profGetMean (uint i);
double profGetShare(uint i);
char  *profGetName (uint i);
const char *profGetReport();