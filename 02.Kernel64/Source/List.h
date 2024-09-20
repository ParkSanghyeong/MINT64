#ifndef __LIST_H__
#define __LIST_H__

#include "Types.h"

#pragma pack(push, 1)

// 데이터를 연결하는 자료구조
// 반드시 데이터의 가장 앞부분에 위치해야 함
typedef struct kListLinkStruct {
    void *pvNext;
    QWORD qwID;
} LISTLINK;

typedef struct kListManagerStruct {
    int iItemCount;
    void *pvHeader;
    void *pvTail;
} LIST;

#pragma pack(pop)

void kInitializeList(LIST *pstList);
int kGetListCount(const LIST *pstList);
void kAddListToTail(LIST *pstList, void *pvItem);
void kAddListToHeader(LIST *pstList, void *pvItem);
void *kRemoveList(LIST *pstList, QWORD qwID);
void *kRemoveListFromHeader(LIST *pstList);
void *kRemoveListFromTail(LIST *pstList);
void *kFindList(const LIST *pstList, QWORD qwID);
void *kGetHeaderFromList(const LIST *pstList);
void *kGetTailFromList(const LIST *pstList);
void *kGetNextFromList(const LIST *pstList, void *pstCurrent);

#endif