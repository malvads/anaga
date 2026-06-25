#!/bin/bash

make

if [ -f build/anaga.bin ]; then
    qemu-system-i386 -kernel build/anaga.bin
else
    echo "Error: build/anaga.bin not found. Build failed?"
fi
