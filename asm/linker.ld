
OUTPUT_FORMAT("elf32-littlemips");
OUTPUT_ARCH(mips);

SECTIONS
{
    . = 0x0000;
    .text : {
        *(.text)
        PROVIDE (_etext_end = .);
    }
    .rodata : {
        *(.rodata*)
        PROVIDE (_rodata_end = .);
    }
    .data : {
        *(.data)
         PROVIDE (_data_end = .);
    }
    .bss : {
        *(.bss)
         PROVIDE (_bss_end = .);
    }
    PROVIDE (__end = .);
}