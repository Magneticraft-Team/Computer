
OUTPUT_FORMAT("elf32-littlemips");
OUTPUT_ARCH(mips);

SECTIONS
{
    . = 0x0000;
    .text : { *(.text) }
    .rodata : { *(.rodata*) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}

