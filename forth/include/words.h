//
// Created by cout970 on 2017-08-13.
//

#ifndef MAGNETICRAFTCOMPUTER_WORDS_H
#define MAGNETICRAFTCOMPUTER_WORDS_H

#include "definitions.h"

#define TIB_SIZE 80
#define BLOCK_SIZE 1024
#define WORD_BUFFER_SIZE 128
#define BLOCK_BUFFER_SIZE 1024

#define IMMEDIATE_BIT_MASK 128

void init();

void readVariableAddress();
void readVariableValue();
void* allot(int size);

void fun_dump();
void fun_cells();
void fun_at();
void fun_set();
void fun_c_at();
void fun_c_set();
void fun_here();
void fun_dp();
void fun_align_word();
void fun_plus();
void fun_minus();
void fun_times();
void fun_div();
void fun_mod();
void fun_create();
void fun_comma();
void fun_variable();
void fun_constant();
void fun_quote();
void fun_more_body();
void fun_more_does();
void fun_literal();
void fun_key();
void fun_cr();
void fun_page();
void fun_emit();
void fun_type();
void fun_print();
void fun_dot();
void fun_dot_s();
void fun_words();
void fun_block();
void fun_list();
void fun_load();
void fun_flush();
void fun_wipe();
void fun_pp();
void fun_dup();
void fun_drop();
void fun_colon();
void fun_exit();
void fun_run_list();
void fun_semi_colon();
void fun_lit();
void fun_int_find();
void fun_word();
void fun_find();
void fun_minus_find();
void fun_quit();
void fun_q_number();
void fun_execute();
void fun_q_stack();
void fun_interpret();
void fun_expect();
void fun_query();
void fun_forth();

#endif //MAGNETICRAFTCOMPUTER_WORDS_H
