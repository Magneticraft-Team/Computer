.section .text
.global _start
.type _start, @function
_start:
    # Sets the stack pointer
    la $sp, 0x9ffc
    # jumps to start() in start.c
    jal main
_shutdown:
    # Sets 0xFFFF0001 to 0, this sends a signal to the motherboard that halts the PC
    li $t0, 0
    sb $t0, (0xFFFF0001)
    j _shutdown
