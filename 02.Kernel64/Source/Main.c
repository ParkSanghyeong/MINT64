#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "AssemblyUtility.h"
#include "Utility.h"
#include "Task.h"
#include "PIT.h"

void Main(void) {
    int iCursorX, iCursorY;
    kInitializeConsole(0, 10);

    kPrintf("Switch To IA-32e Mode Success.\n");
    kPrintf("IA-32e C Language Kernel Start..............[PASS]\n");
    kPrintf("Initialize Console..........................[PASS]\n");

    // Booting Sequence
    kGetCursor(&iCursorX, &iCursorY);
    kPrintf("GDT Initialize And Switch For IA-32e Mode...[    ]");
    kInitializeGDTTableAndTSS();
    kLoadGDTR(GDTR_STARTADDRESS);
    kSetCursor(45, iCursorY++);
    kPrintf("PASS\n");

    kPrintf("TSS Segment Load............................[    ]");
    kLoadTR(GDT_TSSSEGMENT);
    kSetCursor(45, iCursorY++);
    kPrintf("PASS\n");

    kPrintf("IDT Initialize..............................[    ]");
    kInitializeIDTTables();
    kLoadIDTR(IDTR_STARTADDRESS);
    kSetCursor(45, iCursorY++);
    kPrintf("PASS\n");

    kPrintf("Total RAM Size Check........................[    ]");
    kCheckTotalRAMSize();
    kSetCursor(45, iCursorY++);
    kPrintf("PASS], Size = %d MB\n", kGetTotalRAMSize());
    
    kPrintf("TCB Pool And Scheduler Initialize...........[PASS]");
    iCursorY++;
    kInitializeScheduler();
    kInitializePIT(MSTOCOUNT(1), 1);
    
    kPrintf("KeyBoard Activate And Queue Initialize......[    ]");

    if(kInitializeKeyboard() == TRUE) {
        kSetCursor(45, iCursorY++);
        kPrintf("PASS\n");
        kChangeKeyboardLED(FALSE, FALSE, FALSE);
    }
    else {
        kSetCursor(45, iCursorY++);
        kPrintf("FAIL\n");
        while(1);
    }

    kPrintf("PIC Controller And Interrupt Initialize.....[    ]");
    kInitializePIC();
    kMaskPICInterrupt(0);
    kEnableInterrupt();
    kSetCursor(45, iCursorY++);
    kPrintf("PASS\n");

    kCreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_IDLE, (QWORD)kIdleTask);
    kStartConsoleShell();
}
