#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>

volatile QWORD g_qwTickCount = 0;

void kMemSet(void* pvDestination, BYTE bData, int iSize) {
    int i;

    for(i = 0; i < iSize; i++) {
        ((char*)pvDestination)[i] = bData;
    }
}

int kMemCopy(void* pvDestination, const void* pvSource, int iSize) {
    int i;

    for(i = 0; i < iSize; i++) {
        ((char*)pvDestination)[i] = ((char*)pvSource)[i];
        
    }
    return iSize;
}

int kMemCmp(void* pvDestination, const void* pvSource, int iSize) {
    int i;
    char cTemp;

    for(i = 0; i < iSize; i++) {
        cTemp = ((char*)pvDestination)[i] - ((char*)pvSource)[i];
        if(cTemp != 0) {
            return (int)cTemp;
        }
    }
    return 0;
}

BOOL kSetInterruptFlag(BOOL bEnableInterrupt) {
    QWORD qwRFLAGS;

    qwRFLAGS = kReadRFLAGS();
    if(bEnableInterrupt == TRUE) {
        kEnableInterrupt();
    }
    else {
        kDisableInterrupt();
    }

    if(qwRFLAGS & 0x0200) {
        return TRUE;
    }
    return FALSE;
}

int kStrLen(const char* pcBuffer) {
    int i;
    for(i = 0; ; i++) {
        if(pcBuffer[i] == '\0') {
            break;
        }
    }
    return i;
}

// ========== RAM SIZE ==========
static QWORD gs_qwTotalRAMMBSize = 0;

void kCheckTotalRAMSize(void) {
    DWORD* pdwCurrentAddress;
    DWORD dwPreviousValue;

    pdwCurrentAddress = (DWORD*) 0x4000000;
    
    while(1) {
        dwPreviousValue = *pdwCurrentAddress;
        *pdwCurrentAddress = 0x12345678;
        if(*pdwCurrentAddress != 0x12345678) {
            break;
        }
        *pdwCurrentAddress = dwPreviousValue;
        pdwCurrentAddress += (0x400000 / 4);
    }
    gs_qwTotalRAMMBSize = (QWORD) pdwCurrentAddress / 0x100000;
}

QWORD kGetTotalRAMSize(void) {
    return gs_qwTotalRAMMBSize;
}

int kIToA(long lValue, char* pcBuffer, int iRadix) {
    BOOL bIsNegative = FALSE;
    long i = 0;
    int rem;

    if(lValue == 0) {
        pcBuffer[i++] = '0';
        pcBuffer[i] = '\0';
        return 1;
    }

    if(lValue < 0) {
        bIsNegative = TRUE;
        lValue = -lValue;
    }

    while(lValue != 0) {
        rem = lValue % iRadix;
        pcBuffer[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        lValue = lValue / iRadix;
    }

    if(bIsNegative) {
        pcBuffer[i++] = '-';
    }

    pcBuffer[i] = '\0';
    reverse(pcBuffer, i);

    return i;
}

void reverse(char buffer[], int iLength) {
    int start = 0;
    int end = iLength - 1;

    while(start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

long kAToI(const char* pcBuffer, int iRadix) {
    long lValue = 0;
    int i = 0;

    if(iRadix == 10 && pcBuffer[0] == '-') {
        i = 1;
    }

    for(; pcBuffer[i] != '\0'; i++) {
        lValue *= iRadix;

        if(iRadix == 16) {
            if('a' <= pcBuffer[i] && pcBuffer[i] <= 'z') {
                lValue += pcBuffer[i] - 'a' + 10;
                continue;
            }
            if('A' <= pcBuffer[i] && pcBuffer[i] <= 'Z') {
                lValue += pcBuffer[i] - 'A' + 10;
                continue;
            }
        }
        
        lValue += pcBuffer[i] - '0';
    }

    if(iRadix == 10 && pcBuffer[0] == '-') {
        lValue = -lValue;
    }

    return lValue;
}

int kSPrintf(char* pcBuffer, const char* pcFormatString, ...) {
    int iLength;
    va_list va;

    va_start(va, pcFormatString);
    iLength = kVSPrintf(pcBuffer, pcFormatString, va);
    va_end(va);
    return iLength;
}

int kVSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap) {
    QWORD i, j;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char* pcCopyString;
    QWORD qwValue;
    int iValue;

    iFormatLength = kStrLen(pcFormatString);
    for(i = 0; i < iFormatLength; i++) {
        if(pcFormatString[i] == '%') {
            i++;
            switch (pcFormatString[i])
            {
            case 's':
                pcCopyString = (char*)(va_arg(ap, char*));
                iCopyLength = kStrLen(pcCopyString);
                kMemCopy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
                iBufferIndex += iCopyLength;
                break;

            case 'c':
                pcBuffer[iBufferIndex] = (char)(va_arg(ap, int));
                iBufferIndex++;
                break;

            case 'd':
            case 'i':
                iValue = (int)(va_arg(ap, int));    
                iBufferIndex += kIToA(iValue, pcBuffer + iBufferIndex, 10);
                break;

            case 'x':
            case 'X':
                qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xffffffff;
                iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
                break;
            
            case 'q':
            case 'Q':
            case 'p':
                qwValue = (QWORD)(va_arg(ap, QWORD));
                iBufferIndex += kIToA(qwValue, pcBuffer + iBufferIndex, 16);
                break;

            default:
                pcBuffer[iBufferIndex] = pcFormatString[i];
                iBufferIndex++;
                break;
            }
        }
        else {
            pcBuffer[iBufferIndex] = pcFormatString[i];
            iBufferIndex++;
        }
    }
    pcBuffer[iBufferIndex] = '\0';
    return iBufferIndex;
}

QWORD kGetTickCount(void) {
    return g_qwTickCount;
}