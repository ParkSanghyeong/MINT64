#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "Types.h"
#include <stdarg.h>

void kMemSet(void* pvDestination, BYTE bData, int iSize);
int kMemCopy(void* pvDestination, const void* pvSource, int iSize);
int kMemCmp(void* pvDestination, const void* pvSource, int iSize);
BOOL kSetInterruptFlag(BOOL bEnableInterrupt);

void kCheckTotalRAMSize(void);
QWORD kGetTotalRAMSize(void);
int kSPrintf(char* pcBuffer, const char* pcFormatString, ...);
int kVSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap);
int kIToA(long lValue, char* pcBuffer, int iRadix);
long kAToI(const char* pcBuffer, int iRadix);
int kStrLen(const char* pcBuffer);

QWORD kGetTickCount(void);

extern volatile QWORD g_qwTickCount;

#endif