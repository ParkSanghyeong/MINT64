#include "PIC.h"
#include "AssemblyUtility.h"

void kInitializePIC(void) {
    // ICW1 (Master)
    kOutPortByte(PIC_MASTER_PORT1, 0x11);
    // ICW2
    kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);
    // ICW3
    kOutPortByte(PIC_MASTER_PORT2, 0x04);
    // ICW4
    kOutPortByte(PIC_MASTER_PORT2, 0x01);

    // ICW1 (Slave)
    kOutPortByte(PIC_SLAVE_PORT1, 0x11);
    // ICW2
    kOutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR + 8);
    // ICW3
    kOutPortByte(PIC_SLAVE_PORT2, 0x02);
    // ICW4
    kOutPortByte(PIC_SLAVE_PORT2, 0x01);
}

void kMaskPICInterrupt(WORD wIRQBitmask) {
    // OCW1
    // IRQ 0-7
    kOutPortByte(PIC_MASTER_PORT2, (BYTE)wIRQBitmask);
    // IRQ 8-15
    kOutPortByte(PIC_SLAVE_PORT2, (BYTE)wIRQBitmask >> 8);
}

void kSendEOIToPIC(int iIRQNumber) {
    // OCW2
    kOutPortByte(PIC_MASTER_PORT1, 0x20);

    if(iIRQNumber >= 8) {
        kOutPortByte(PIC_SLAVE_PORT1, 0x20);
    }
}
