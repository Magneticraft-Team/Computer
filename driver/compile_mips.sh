#!/bin/bash

if ! type "mipsel-none-elf-as" > /dev/null; then
    echo "Missing dependency MIPS assembler"
    exit -1
fi
if ! type "mipsel-none-elf-gcc" > /dev/null; then
    echo "Missing dependency MIPS gcc compiler"
    exit -1
fi
if ! type "mipsel-none-elf-objcopy" > /dev/null; then
    echo "Missing dependency MIPS objcopy utility"
    exit -1
fi

c_files="start.c api/motherboard.c api/monitor.c api/network.c api/disk_drive.c"
asm_files="boot.asm"
linker_script="linker.ld"
temp_o="output.o"
output="output.bin"
linker_flags="-Wl,-T,$linker_script"
compile_flags="-static -ffreestanding -G0 -g0 -Os -fno-zero-initialized-in-bss -fno-toplevel-reorder -fomit-frame-pointer -nostdlib -Wall -Wextra"

compiled_files=""

if [ ! -e 'out' ]; then
    mkdir out
fi
cd out
if [ ! -e 's' ]; then
    mkdir s
fi
cd ..

echo "Compiling asm"
for i in ${asm_files} ; do
    name=`echo ${i} | sed -r 's/([^/]+\/)?([^/]+)\.\w+$/\2/g'`
    mipsel-none-elf-as ${i} -o out/${name}.o || `echo "Error in asm" && exit 1`
    compiled_files="$compiled_files out/$name.o"
done

echo "Compiling c"
for i in ${c_files} ; do
     name=`echo ${i} | sed -r 's/([^/]+\/)?([^/]+)\.\w+$/\2/g'`
     mipsel-none-elf-gcc ${compile_flags} -o out/${name}.o -c ${i} || `exit 1`
     mipsel-none-elf-gcc ${compile_flags} -o out/s/${name}.asm -S ${i} || `exit 1`
     compiled_files="$compiled_files out/$name.o"
done

echo "Linking"
mipsel-none-elf-gcc ${linker_flags} -o out/${temp_o} ${compile_flags} ${compiled_files} -lgcc || `echo "Error linking" && exit 1`
mipsel-none-elf-ar rcs out/drivers.a ${compiled_files} || `echo "Error linking" && exit 1`

echo "Finishing"
mipsel-none-elf-objcopy -Obinary out/${temp_o} ${output} || `echo "Error finishing file" && exit 1`

echo "Successfully compiled"

