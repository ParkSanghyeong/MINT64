#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "InterruptHandler.h"
#include "Task.h"

SHELLCOMMANDENTRY gs_vstCommandTable[] = {
    {"help", "Show Help", kHelp},
    {"cls", "Clear Screen", kCls},
    {"totalram", "Show Total RAM Size", kShowTotalRAMSize},
    {"strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest},
    {"shutdown", "Shutdown And Reboot OS", kShutdown},
    {"settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
    {"wait", "Wait ms Using PIT, ex)wait 100(ms)", kWaitUsingPIT},
    {"rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter},
    {"cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed},
    {"date", "Show Date And Time", kShowDateAndTime},
    {"createtask", "Create Task, ex) createtask 1(type) 10(count)", kCreateTestTask},
    { "changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)", kChangeTaskPriority },
    { "tasklist", "Show Task List", kShowTaskList },
    { "killtask", "End Task, ex)killtask 1(ID)", kKillTask }, { "cpuload", "Show Processor Load", kCPULoad },
};

// Main Console Shell Loop
void kStartConsoleShell(void) {
    char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;

    kPrintf(CONSOLESHELL_PROMPTMESSAGE);

    while(1) {
        bKey = kGetCh();

        // Back Space
        if(bKey == KEY_BACKSPACE) {
            if(iCommandBufferIndex > 0) {
                kGetCursor(&iCursorX, &iCursorY);
                kPrintStringXY(iCursorX - 1, iCursorY, " ");
                kSetCursor(iCursorX - 1, iCursorY);
                iCommandBufferIndex--;
            }
        }

        // Enter 
        else if(bKey == KEY_ENTER) {
            kPrintf("\n");

            // execute Command
            if(iCommandBufferIndex > 0) {
                vcCommandBuffer[iCommandBufferIndex] = '\0';
                kExecuteCommand(vcCommandBuffer);
            }

            // reset Command Buffer
            kPrintf("%s", CONSOLESHELL_PROMPTMESSAGE);
            kMemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
            iCommandBufferIndex = 0;
        }

        // Dismiss
        else if((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) ||
                (bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) ||
                (bKey == KEY_SCROLLLOCK));

        else {
            if(bKey == KEY_TAB) {
                bKey = ' ';
            }

            if(iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT) {
                vcCommandBuffer[iCommandBufferIndex++] = bKey;
                kPrintf("%c", bKey);
            }
        }
    }
}

void kExecuteCommand(const char* pcCommandBuffer) {
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;

    iCommandBufferLength = kStrLen(pcCommandBuffer);
    for(iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++) {
        if(pcCommandBuffer[iSpaceIndex] == ' ') {
            break;
        }
    }

    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
    for(i = 0; i < iCount; i++) {
        iCommandLength = kStrLen(gs_vstCommandTable[i].pcCommand);

        if(iCommandLength == iSpaceIndex && kMemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer, iSpaceIndex) == 0) {
            gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
            break;
        } 
    }

    if(i >= iCount) {
        kPrintf("'%s' is not found.\n", pcCommandBuffer);
    }
}

void kInitializeParameter(PARAMETERLIST* pstList, const char* pcParameter) {
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen(pcParameter);
    pstList->iCurrentPosition = 0;
}

int kGetNextParameter(PARAMETERLIST* pstList, char* pcParameter) {
    int i;
    int iLength;

    if(pstList->iLength <= pstList->iCurrentPosition) {
        return 0;
    }

    // search
    for(i = pstList->iCurrentPosition; i < pstList->iLength; i++) {
        if(pstList->pcBuffer[i] == ' ') {
            break;
        }
    }

    // copy params
    kMemCopy(pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i);
    iLength = i - pstList->iCurrentPosition;
    pcParameter[iLength] = '\0';

    // update param pos
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}

//==============================================================================
//  Commands
//==============================================================================

static void kHelp(const char* pcCommandBuffer) {
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
    kPrintf( "=========================================================\n" );
    kPrintf( "                    MINT64 Shell Help                    \n" );
    kPrintf( "=========================================================\n" );
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}

static void kCls( const char* pcParameterBuffer ) {
    kClearScreen();
    kSetCursor( 0, 1 );
}

static void kShowTotalRAMSize(const char* pcParameterBuffer) {
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}


static void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 ) {
        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

        if( kMemCmp( vcParameter, "0x", 2 ) == 0 )
        {
            lValue = kAToI( vcParameter + 2, 16 );
            kPrintf( "HEX Value = %d\n", lValue );
        }
        else
        {
            lValue = kAToI( vcParameter, 10 );
            kPrintf( "Decimal Value = %d\n", lValue );
        }
        
        iCount++;
    }
}

static void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // 키보드 컨트롤러를 통해 PC를 재시작
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

static void kSetTimer(const char* pcParameterBuffer) {
    char vcParameter[100];
    PARAMETERLIST stList;
    long lValue;
    BOOL bPeriodic;

    kInitializeParameter(&stList, pcParameterBuffer);
    
    // millisecond
    if(kGetNextParameter(&stList, vcParameter) == 0) {
        kPrintf("ex) settimer 10(mx) 1(periodic)\n");
        return;
    }
    lValue = kAToI(vcParameter, 10);

    if(kGetNextParameter(&stList, vcParameter) == 0) {
        kPrintf("ex) settimer 10(mx) 1(periodic)\n");
        return;
    }
    bPeriodic = kAToI(vcParameter, 10);

    kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
    kPrintf("Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic);
}

static void kWaitUsingPIT(const char* pcParameterBuffer) {
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    long lMillisecond;
    int i;

    kInitializeParameter(&stList, pcParameterBuffer);
    if(kGetNextParameter(&stList, vcParameter) == 0) {
        kPrintf("ex) wait 100(ms)\n");
        return;
    }
    lMillisecond = kAToI(pcParameterBuffer, 10);
    kPrintf("%d ms Sleep Start...\n", lMillisecond);

    kDisableInterrupt();

    // wait
    for(i = 0; i < lMillisecond/30; i++) {
        kWaitUsingDirectPIT(MSTOCOUNT(30));
    }
    kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond%30));
    
    kEnableInterrupt();
    kPrintf("%d ms Sleep Complete\n", lMillisecond);

    // restore timer
    kInitializePIT(MSTOCOUNT(1), TRUE);
}

static void kReadTimeStampCounter(const char* pcParameterBuffer) {
    QWORD qwTSC;

    qwTSC = kReadTSC();
    kPrintf("Time Stamp Counter = %q\n", qwTSC);
}

static void kMeasureProcessorSpeed(const char* pcParameterBuffer) {
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;

    kPrintf("Now Measuring.");
    
    kDisableInterrupt();

    for(i = 0; i < 200; i++) {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT(MSTOCOUNT(50));
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf(".");
    }
    kInitializePIT(MSTOCOUNT(1), TRUE);
    kEnableInterrupt();

    kPrintf("\nCPU Speed = %d MHz\n", qwTotalTSC/10/1000/1000);
}

static void kShowDateAndTime(const char* pcParameterBuffer) {
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    kReadRTCTime(&bHour, &bMinute, &bSecond);
    kReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

    kPrintf("Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth, kConvertDayOfWeekToString(bDayOfWeek));
    kPrintf("Time: %d:%d:%d\n", bHour, bMinute, bSecond);
}

static void kCreateTestTask(const char* pcParameterBuffer) {
    PARAMETERLIST stList;
    char vcType[30];
    char vcCount[30];
    int i;
    
    kInitializeParameter(&stList, pcParameterBuffer);
    kGetNextParameter(&stList, vcType);
    kGetNextParameter(&stList, vcCount);

    switch (kAToI(vcType, 10)) 
    {
    case 1:
        for(i = 0; i < kAToI(vcCount, 10); i++) {
            if(kCreateTask(TASK_FLAGS_LOW, (QWORD)kTestTask1) == NULL) {
                break;
            }
        }
        kPrintf("Task1 %d Created\n", i);
        break;

    case 2:
    default:
        for(i = 0; i < kAToI(vcCount, 10); i++) {
            if(kCreateTask(TASK_FLAGS_LOW, (QWORD)kTestTask2) == NULL) {
                break;
            }
        }
        kPrintf("Task2 %d Created\n", i);
        break;
    }
}

static void kTestTask1(void) {
    BYTE bData;
    int i = 0, iX = 0, iY = 0, iMargin, j;
    CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS; 
    TCB* pstRunningTask;

    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = kGetRunningTask();
    iMargin = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) % 10;

    // 화면 네 귀퉁이를 돌면서 문자 출력 
    for(j = 0; j < 20000; j++) {
        switch(i) 
        {
        case 0:
            iX++;
            if(iX >= (CONSOLE_WIDTH - iMargin)) {
                i = 1; 
            }
            break;
        case 1: 
            iY++;
            if(iY >= (CONSOLE_HEIGHT - iMargin)) {
                i = 2; 
            }
            break;
        case 2: 
            iX--;
            if(iX < iMargin) {
                i = 3; 
            }
            break;
        case 3: 
            iY--;
            if(iY < iMargin) {
                i = 0;
            }
            break; 
        }
    
        // 문자 및 색깔 지정
        pstScreen[iY * CONSOLE_WIDTH + iX].bCharacter = bData;
        pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F; 
        bData++;
        //kSchedule(); 
    }
    kExitTask();
}

static void kTestTask2( void ) {
    int i = 0, iOffset;
    CHARACTER* pstScreen = (CHARACTER*) CONSOLE_VIDEOMEMORYADDRESS; 
    TCB* pstRunningTask;
    char vcData[ 4 ] = { '-', '\\', '|', '/' };

    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = kGetRunningTask();
    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2; 
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));
    
    while( 1 ) {
        // 회전하는 바람개비를 표시
        pstScreen[iOffset].bCharacter = vcData[i % 4];
        // 색깔 지정
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1; 
        i++;
        // 다른 태스크로 전환
        //kSchedule(); 
    }
}

static void kChangeTaskPriority(const char* pcParameterBuffer) {
    PARAMETERLIST stList;
    char vcID[30];
    char vccPriority[30];
    QWORD qwID;
    BYTE bPriority;

    kInitializeParameter(&stList, pcParameterBuffer);
    kGetNextParameter(&stList, vcID);
    kGetNextParameter(&stList, vccPriority);

    if(kMemCmp(vcID, "0x", 2) == 0) {
        qwID = kAToI(vcID + 2, 16);
    }
    else {
        qwID = kAToI(vcID, 10);
    }

    bPriority = kAToI(vccPriority, 10);

    kPrintf("Change Task Priority ID [0x%q] Priority[%d] ", qwID, bPriority);
    if(kChangePriority(qwID, bPriority) == TRUE) {
        kPrintf("Success\n");
    }
    else {
        kPrintf("Fail\n");
    }
}

static void kShowTaskList(const char* pcParameterBuffer) {
    int i;
    TCB* pstTCB;
    int iCount = 0;

    kPrintf("=========== Task Total Count [%d] ===========\n", kGetTaskCount());
    for(i = 0; i < TASK_MAXCOUNT; i++) {
        pstTCB = kGetTCBInTCBPool(i);
        if((pstTCB->stLink.qwID >> 32) != 0) {
            if((iCount != 0) && ((iCount % 10) == 0)) {
                kPrintf("Press any key to continue... ('q' is exit) : ");
                if(kGetCh() == 'q') {
                    kPrintf("\n");
                    break;
                }
                kPrintf("\n");
            }
            kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q]\n", 1 + iCount++, pstTCB->stLink.qwID, GETPRIORITY(pstTCB->qwFlags), pstTCB->qwFlags);
        }
    }
}

static void kKillTask(const char* pcParameterBuffer) {
    PARAMETERLIST stList;
    char vcID[30];
    QWORD qwID;

    kInitializeParameter(&stList, pcParameterBuffer);
    kGetNextParameter(&stList, vcID);

    if(kMemCmp(vcID, "0x", 2) == 0) {
        qwID = kAToI(vcID + 2, 16);
    }
    else {
        qwID = kAToI(vcID, 10);
    }

    kPrintf("Kill Task ID [0x%q] ", qwID);
    if(kEndTask(qwID) == TRUE) {
        kPrintf("Success\n");
    }
    else {
        kPrintf("Fail\n");
    }
}

static void kCPULoad(const char* pcParameterBuffer) {
    kPrintf("Processor Load: %d%%\n", kGetProcessorLoad());
}
