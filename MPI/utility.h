#pragma once
#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "stdafx.h"	
using namespace std;
void debugMessage(const char *fmt, ...)
{
#ifdef _DEBUG_
	va_list args;
	va_start(args, fmt);
	printf(fmt, args);
#endif
}

#endif
