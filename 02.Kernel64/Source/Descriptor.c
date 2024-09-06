#include "Descriptor.h"
#include "Utility.h"

void kInitializeGDTTableAndTSS(void) {
    GDTR* pstGDTR;
    GDTENTRY8* pstEntry;
    TSSSEGMENT* pstTSS;
    int i;

    pstGDTR = (GDTR*) GDTR_STARTADDRESS;
    pstEntry = (GDTENTRY8*)(GDTR_STARTADDRESS + sizeof(GDTR));
    pstGDTR->wLimit = GDT_TABLESIZE - 1;
    pstGDTR->qwBaseAddress = (QWORD)pstEntry;
    pstTSS = (TSSSEGMENT*)((QWORD)pstEntry + GDT_TABLESIZE);

    // null, code, data
    kSetGDTEntry8(&(pstEntry[0]), 0, 0, 0, 0, 0);
    kSetGDTEntry8(&(pstEntry[1]), 0, 0xfffff, GDT_FLAGS_UPPER_CODE, GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE);
    kSetGDTEntry8(&(pstEntry[2]), 0, 0xfffff, GDT_FLAGS_UPPER_DATA, GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA);
    // TSS
    kSetGDTEntry16(((GDTENTRY16*)&pstEntry[3]), (QWORD)pstTSS, sizeof(TSSSEGMENT) - 1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);

    kInitializeTSSSegment(pstTSS);
}

void kSetGDTEntry8(GDTENTRY8* pstEntry, DWORD dwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType) {
    pstEntry->wLowerLimit = dwLimit & 0xffff;
    pstEntry->wLowerBaseAddress = dwBaseAddress & 0xffff;
    pstEntry->bUpperBaseAddress1 = (dwBaseAddress >> 16) & 0xff;
    pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
    pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0x0f) | bUpperFlags;
    pstEntry->bUpperBaseAddress2 = (dwBaseAddress >> 24) & 0xff;
}

void kSetGDTEntry16(GDTENTRY16* pstEntry, QWORD qwBaseAddress, DWORD dwLimit, BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType) {
    pstEntry->wLowerLimit = dwLimit & 0xffff;
    pstEntry->wLowerBaseAddress = qwBaseAddress & 0xffff;
    pstEntry->bMiddleBaseAddress1 = (qwBaseAddress >> 16) & 0xff;
    pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
    pstEntry->bUpperLimitAndUpperFlag = ((dwLimit >> 16) & 0x0f) | bUpperFlags;
    pstEntry->bMiddleBaseAddress2 = (qwBaseAddress >> 24) & 0xff;
    pstEntry->dwUpperBaseAddress = qwBaseAddress >> 32;
    pstEntry->dwReserved = 0;
}

void kInitializeTSSSegment(TSSSEGMENT* pstTSS) {
    kMemSet(pstTSS, 0, sizeof(TSSSEGMENT));
    pstTSS->qwIST[0] = IST_STARTADDRESS + IST_SIZE;
    pstTSS->wIOMapBaseAddress = 0xffff;
}

void kInitializeIDTTables(void) {
    IDTR* pstIDTR;
    IDTENTRY* pstEntry;
    int i;

    pstIDTR = (IDTR*) IDTR_STARTADDRESS;
    pstEntry = (IDTENTRY*)(IDTR_STARTADDRESS + sizeof(IDTR));
    pstIDTR->qwBaseAddress = (QWORD)pstEntry;
    pstIDTR->wLimit = IDT_TABLESIZE - 1;

    for(i = 0; i < IDT_MAXENTRYCOUNT; i++) {
        kSetIDTEntry(&(pstEntry[i]), kDummyHandler, GDT_KERNELCODESEGMENT, IDT_FLAGS_IST1, IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    }
}

void kSetIDTEntry(IDTENTRY* pstEntry, void* pvHandler, WORD wSelector, BYTE bIST, BYTE bFlags, BYTE bType) {
    pstEntry->wLowerBaseAddress = (QWORD)pvHandler & 0xffff;
    pstEntry->wSegmentSelector = wSelector;
    pstEntry->bIST = bIST & 0x3;
    pstEntry->bTypeAndFlags = bType | bFlags;
    pstEntry->wMiddleBaseAddress = ((QWORD)pvHandler >> 16) & 0xffff;
    pstEntry->dwUpperBaseAddress = (QWORD)pvHandler >> 32;
    pstEntry->dwReserved = 0;
}

void kDummyHandler(void) {
    kPrintString(0, 0, "==================================================");
    kPrintString(0, 1, "                Dummy Handler excute              ");
    kPrintString(0, 2, "           Interrupt or Exception occured         ");
    kPrintString(0, 3, "==================================================");
    while(1);
}