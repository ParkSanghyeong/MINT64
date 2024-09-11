#!/bin/bash

if [ $# == 1 ] && [ $1 == 'debug' ]; then
    qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -M pc -gdb tcp::1234 -S
else
    qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -M pc
fi
