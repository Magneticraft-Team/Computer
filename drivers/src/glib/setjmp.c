//
// Created by cout970 on 2016-11-01.
//

#include <types.h>
#include <glib/setjmp.h>

int setjmp(jmp_buf buf IGNORED) {
    asm volatile(""
            "sw $v1, 0($a0)\n"
            "sw $a0, 4($a0)\n"
            "sw $a1, 8($a0)\n"
            "sw $a2, 12($a0)\n"
            "sw $a3, 16($a0)\n"
            "sw $t0, 20($a0)\n"
            "sw $t1, 24($a0)\n"
            "sw $t2, 28($a0)\n"
            "sw $t3, 32($a0)\n"
            "sw $t4, 36($a0)\n"
            "sw $t5, 40($a0)\n"
            "sw $t6, 44($a0)\n"
            "sw $t7, 48($a0)\n"
            "sw $s0, 52($a0)\n"
            "sw $s1, 56($a0)\n"
            "sw $s2, 60($a0)\n"
            "sw $s3, 64($a0)\n"
            "sw $s4, 68($a0)\n"
            "sw $s5, 72($a0)\n"
            "sw $s6, 76($a0)\n"
            "sw $s7, 80($a0)\n"
            "sw $t8, 84($a0)\n"
            "sw $t9, 88($a0)\n"
            "sw $k0, 92($a0)\n"
            "sw $k1, 96($a0)\n"
            "sw $gp, 100($a0)\n"
            "sw $sp, 104($a0)\n"
            "sw $fp, 108($a0)\n"
            "sw $ra, 112($a0)\n"
            "la $v0, 0\n"
            "jr $ra\n"
    );
    //this is only to get ride of the compiler warning
    return 0;
}

void longjmp(jmp_buf buf IGNORED, int value IGNORED) {
    asm volatile(""
            "sw $a0, 116($a0)\n"
            "sw $a1, 120($a0)\n"
            "lw $v1, 0($a0)\n"
            "lw $a2, 12($a0)\n"
            "lw $a3, 16($a0)\n"
            "lw $t0, 20($a0)\n"
            "lw $t1, 24($a0)\n"
            "lw $t2, 28($a0)\n"
            "lw $t3, 32($a0)\n"
            "lw $t4, 36($a0)\n"
            "lw $t5, 40($a0)\n"
            "lw $t6, 44($a0)\n"
            "lw $t7, 48($a0)\n"
            "lw $s0, 52($a0)\n"
            "lw $s1, 56($a0)\n"
            "lw $s2, 60($a0)\n"
            "lw $s3, 64($a0)\n"
            "lw $s4, 68($a0)\n"
            "lw $s5, 72($a0)\n"
            "lw $s6, 76($a0)\n"
            "lw $s7, 80($a0)\n"
            "lw $t8, 84($a0)\n"
            "lw $t9, 88($a0)\n"
            "lw $k0, 92($a0)\n"
            "lw $k1, 96($a0)\n"
            "lw $gp, 100($a0)\n"
            "lw $sp, 104($a0)\n"
            "lw $fp, 108($a0)\n"
            "lw $ra, 112($a0)\n"
            "move $v0, $a1\n"
            "lw $a1, 120($a0)\n"
            "lw $a0, 116($a0)\n"
            "jr $ra\n"
    );
}