//
// Created by cout970 on 2017-08-13.
//
// https://www.forth.com/starting-forth/9-forth-execution/
// http://astro.pas.rochester.edu/Forth/forth-words.html#fetch
// https://www.forth.com/starting-forth/1-forth-stacks-dictionary/
//

#include <setjmp.h>
#include <debug.h>
#include <motherboard.h>
#include <robot.h>
#include <network.h>
#include <macros.h>
#include <util/cmd.h>
#include <util/number_utils.h>
#include <util/input.h>
#include "../include/dependencies.h"
#include "../include/words.h"
#include "../include/dictionary.h"
#include "../include/stack.h"

/*** VARIABLES ***/

const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define VALUE(word) (*word->data)
#define INT_VALUE(word) (*word->data).i32
#define valueOf(something) ((Value){(void*)(something)})

jmp_buf onError;

// >IN (offset of the input buffer to start reading new characters)
Word *moreIn;
// BLK (disk block to read, if zero, read from TIB)
Word *blk;
// SPAN
Word *span;
// #TIB
Word *hashTib;
// BL (space character)
Word *bl;
// BASE (number base encoding)
Word *base;
// (LIT)
Word *lit;
// (EXIT)
Word *int_exit;
// ;
Word *semi_colon;
// STATE (0 = interpreter, 1 = compiler)
Word *state;
// (.")
Word *int_dot_quote;
// FILE
Word *fileWord;
// FOLDER
Word *folderWord;
// BRANCH?
Word *branchQWord;
// BRANCH
Word *branchWord;
// (DO)
Word *doIntWord;
// (LOOP)
Word *loopIntWord;
// (+LOOP)
Word *plusLoopIntWord;
// #BLOCK
Word *currentBlock;

Word *currentWord = NULL;
Word *currentWordList = NULL;
int instructionOffset = 0;

// TIB (token input buffer)
char tib[TIB_SIZE];
// WORD token buffer
char wordBuffer[WORD_BUFFER_SIZE];
// File IO Buffer
char fileIOBuffer[BLOCK_BUFFER_SIZE];
const char fileIOBufferEnd = 0;

void readVariableAddress(void) {
    pushData(valueOf(&currentWord->data[0]));
}

void readVariableValue(void) {
    pushData(VALUE(currentWord));
}

void *allot(int size) {
    char *aux = dp;
    dp = aux + size;
    return aux;
}

int byteToIntCeil(int bytes) {
    int ints = bytes / 4;
    int rest = bytes - ints * 4;

    return ints + ((rest != 0) ? 1 : 0);
}

/**************/
/*** DEBUG ***/
/**************/

void printWord(Word *word) {
    kdebug("Word(ptr = %x (%d),\n", (unsigned int) word, (int) word);
    kdebug("name = '%s'(%d), \n", word->name, strlen(word->name));
    kdebug("flags = %d (%s), \n", word->flags, (word->flags & IMMEDIATE_BIT_MASK) ? "IMMEDIATE" : "NORMAL");
    kdebug("next = %d (%s), \n", (int) word->next, word->next ? word->next->name : "NULL");

    int type = word->code == readVariableAddress
               ? 0//"VARIABLE"
               : word->code == readVariableValue
                 ? 1//"CONSTANT"
                 : word->code == fun_run_list
                   ? 2 //"COMPILED WORD"
                   : 3; //"NATIVE"

    kdebug("code = %d (%s)", (int) word->code,
           type == 0 ? "VARIABLE" : type == 1 ? "CONSTANT" : type == 2 ? "COMPILED WORD" : "NATIVE");

    if (type == 0 || type == 1) {
        kdebug(", \nvalue = %d (0x%x)", VALUE(word), VALUE(word));
    } else if (type == 2) {
        kdebug(", \nwords: [ ");
        Word *item;
        for (int i = 0;; ++i) {
            item = word->data[i].word;
            if (item == lit) {
                kdebug("LIT(%d) ", word->data[++i]);
            } else if (item == int_dot_quote) {
                kdebug(".\"(");
                char *str = (char *) (word->data + i + 1);
                kdebug("%s", str);
                int count = (int) (strlen(str) + 1);
                i += byteToIntCeil(count);
                kdebug(")");
            } else if (item == branchQWord) {
                kdebug("BRANCH(%d) ", word->data[++i]);
            } else if (item == branchWord) {
                kdebug("BRANCH?(%d) ", word->data[++i]);
            } else if (item == loopIntWord) {
                kdebug("(LOOP)(%d) ", word->data[++i]);
            } else if (item == plusLoopIntWord) {
                kdebug("(+LOOP)(%d) ", word->data[++i]);
            } else {
                kdebug("%s(%x) ", item->name, (unsigned int) (word->data + i));
            }

            if (item == int_exit || item == NULL) break;
        }
        kdebug("]");
    }
    kdebug(")");
}

// DEBUG (--) name
void fun_debug(void) {
    fun_minus_find();
    if (popData().i32) {
        Word *word = popData().word;
        kdebug("\n");
        printWord(word);
    }
}

/**************/
/*** Memory ***/
/**************/

// DUMP (addr size --)
void fun_dump(void) {
    int size = popData().i32;
    char *addr = popData().str;
    char *base, *end = addr + size, *aux;
    int i, j, lines = (size + 15) / 16;

    kdebug("\n");

    for (i = 0; i < lines; ++i) {
        base = addr + i * 16;

        kdebug(" ");
        kdebug("0");
        kdebug("x");

        kdebug("%c", digits[((int) base & 0xF0000000) >> 28]);
        kdebug("%c", digits[((int) base & 0x0F000000) >> 24]);
        kdebug("%c", digits[((int) base & 0x00F00000) >> 20]);
        kdebug("%c", digits[((int) base & 0x000F0000) >> 16]);
        kdebug("%c", digits[((int) base & 0x0000F000) >> 12]);
        kdebug("%c", digits[((int) base & 0x00000F00) >> 8]);
        kdebug("%c", digits[((int) base & 0x000000F0) >> 4]);
        kdebug("%c", digits[((int) base & 0x0000000F) >> 0]);

        kdebug(":");
        kdebug(" ");
        for (j = 0; j < 16; ++j) {
            aux = base + j;
            if (aux < end) {
                kdebug("%c", digits[(*aux & 0xF0) >> 4]);
                kdebug("%c", digits[(*aux & 0x0F) >> 0]);
            } else {
                kdebug("-");
                kdebug("-");
            }
            kdebug(" ");
        }
        kdebug(" ");

        for (j = 0; j < 16; ++j) {
            if (iscntrl(base[j])) {
                kdebug(".");
            } else {
                kdebug("%c", base[j]);
            }
        }
        kdebug("\n");
    }
}

// CELLS (a -- b)  b = size of a cell * a
void fun_cells(void) {
    pushData(valueOf(popData().i32 * 4));
}

// @ ( addr -- value )
void fun_at(void) {
    int *addr = popData().ptr;
    pushData(valueOf(*addr));
}

// ! ( n addr -- )
void fun_set(void) {
    int *addr = popData().ptr;
    int value = popData().i32;
    *addr = value;
}

// C@ ( addr -- value )
void fun_c_at(void) {
    char *addr = popData().str;
    pushData(valueOf(*addr));
}

// C! ( n addr -- )
void fun_c_set(void) {
    char *addr = popData().str;
    char value = popData().i8;
    *addr = value;
}

// HERE
void fun_here(void) {
    pushData(valueOf(dp));
}

// DP
void fun_dp(void) {
    pushData(valueOf(&dp));
}

// ALIGNW (--)
void fun_align_word(void) {
    dp = (char *) (((long int) dp + 3) & (~3));
}

/************/
/*** Math ***/
/************/

// + (a b -- c) c = a + b
void fun_plus(void) {
    Value b = popData();
    Value a = popData();
    pushData(valueOf(a.i32 + b.i32));
}

// - (a b -- c) c = a - b
void fun_minus(void) {
    Value b = popData();
    Value a = popData();
    pushData(valueOf(a.i32 - b.i32));
}

// * (a b -- c) c = a + b
void fun_times(void) {
    Value b = popData();
    Value a = popData();
    pushData(valueOf(a.i32 * b.i32));
}

// / (a b -- c) c = a / b
void fun_div(void) {
    Value b = popData();
    Value a = popData();
    pushData(valueOf(a.i32 / b.i32));
}

// % (a b -- c) c = a % b
void fun_mod(void) {
    Value b = popData();
    Value a = popData();
    pushData(valueOf(a.i32 % b.i32));
}

// = (a b -- c)
void fun_equals(void) {
    Value b = popData();
    Value a = popData();
    pushData(valueOf(a.ptr == b.ptr));
}

/*************************/
/*** Word Construction ***/
/*************************/

// CREATE (--) name
void fun_create(void) {

    // Reads a word from input (name of the variable)
    pushData(*bl->data);
    fun_word();
    String *addr = popData().str;

    String *name = createString(addr);
    Word *word = newWord(name, NULL, 0, 0);

    extendDictionary(word);
    pushData(valueOf(word));
}

// , (n -- )
void fun_comma(void) {
    int value = popData().i32;
    fun_align_word();
    *(int *) dp = value;
    dp += 4;
}

// VARIABLE (--)
void fun_variable(void) {
    fun_create();
    Word *word = popData().word;
    word->code = readVariableAddress;
    pushData(valueOf(0));
    fun_comma();
}

// CONSTANT (n -- )
void fun_constant(void) {
    fun_create();
    Word *word = popData().word;
    word->code = readVariableValue;
    fun_comma();
}

// ' (--)
void fun_quote(void) {
    pushData(*bl->data);
    fun_word();
    String *addr = popData().str;

    Word *word = findIn(dictionary, addr);
    pushData(valueOf(word));
}

// >BODY (addr1 -- addr2)
void fun_more_body(void) {
    Word *word = popData().word;
    pushData(valueOf(word->data));
}

// >DOES (addr1 -- addr2)
void fun_more_does(void) {
    Word *word = popData().word;
    pushData(valueOf(word->code));
}

// LITERAL (n --)
void fun_literal(void) {
    int value = popData().i32;
    fun_align_word();
    *(int *) dp = (int) lit;
    dp += 4;
    *(int *) dp = value;
    dp += 4;
}

/*******************/
/*** Terminal IO ***/
/*******************/

// KEY (-- char)
void fun_key(void) {
    char buffer[2];
    readString(buffer, 2);
    pushData(valueOf(*buffer));
}

// READ
void fun_read(void) {
    char buffer[80];
    readString(buffer, 80);
    createString(buffer);
    pushData(valueOf(createString(buffer)));
}

// CR (--)
void fun_cr(void) {
    kdebug("\n");
}

// PAGE (--)
void fun_page(void) {
    monitor_clear(motherboard_get_monitor());
}

// EMIT
void fun_emit(void) {
    kdebug("%c", popData().i8);
}

// TYPE (--)
void fun_type(void) {
    String *str = popData().str;
    kdebug("%s", str);
}

// PRINT (--)
void fun_print(void) {
    char *str = popData().str;
    kdebug("%s\n", str);
}

static int printNumber(int num, int base) {
    int sum = 0;
    if (num < 0) {
        kdebug("-");
        sum++;
        num = -num;
    }
    if (num < base) {
        kdebug("%c", digits[num]);
        sum++;
    } else {
        sum += printNumber(num / base, base);
        kdebug("%c", digits[num % base]);
        sum++;
    }
    return sum;
}

// . ( a -- ) pop a value and prints it
void fun_dot(void) {
    if (isDataStackEmpty()) {
        kdebug("Empty stack\n");
        fun_quit();
    } else {
        printNumber(popData().i32, INT_VALUE(base));
        kdebug(" ");
    }
}

// .S (--)  prints the stack
void fun_dot_s(void) {
    int i;
    for (i = 0; i < dataStackPtr; i++) {
        printNumber(dataStack[i].i32, INT_VALUE(base));
        kdebug(" ");
    }
}

// WORDS (--)
void fun_words(void) {
    Word *word;
    for (word = dictionary; word != NULL; word = word->next) {
        kdebug("%s ", word->name);
    }
}

// FREE (-- n)
void fun_free(void) {
    int totalRam = motherboard_get_memory_size();
    int freeRam = totalRam - ((int) dp) - 1024 * 4;
    pushData(valueOf(freeRam));
}

// ABORT" (--)
void fun_abort(void) {
    pushData(valueOf('\"'));
    fun_word();
    kdebug("%s ", popData().str);
    fun_quit();
}

// ." (--)
void fun_dot_quote(void) {
    pushData(valueOf('\"'));
    fun_word();
    String *text = popData().str;
    int size = (int) strlen(text);
    if (size == 0) {
        return;
    }

    // scape the " char
    INT_VALUE(moreIn)++;

    if (INT_VALUE(state)) {
        *(int *) dp = (int) int_dot_quote;
        dp += 4;

        for (int i = 0; i < size; ++i) {
            *dp = text[i];
            dp++;
        }

        *dp = '\0';
        dp++;
        fun_align_word();
    } else {
        kdebug("%s", text);
    }
}

// (.") (--)
void fun_int_dot_quote(void) {
    char *str = (char *) (currentWordList->data + instructionOffset + 1);
    kdebug("%s", str);
    int countTest = (int) (strlen(str) + 1);
    instructionOffset += byteToIntCeil(countTest);
}

/*********************************************************************************************************************/
/****************************************************** Disk IO ******************************************************/
/*********************************************************************************************************************/

#define CHESK_FS()  if(!hasDisk()){ kdebug("No disk\n"); return; }                      \
                    if(fs_getDevice() == -1){ kdebug("Disk not formatted\n"); return; } \
                    else { fs_init(motherboard_get_floppy_drive()); }

int hasDisk(void) {
    return disk_drive_has_disk(motherboard_get_floppy_drive());
}

// BLOCK (n -- addr) n >= 1
void fun_block(void) {
    int block = popData().i32;

    CHESK_FS();

    if (INT_VALUE(fileWord) == FS_NULL_INODE_REF) {
        INodeRef folder = INT_VALUE(folderWord);
        INodeRef file = fs_findFile(folder, "forth.txt");

        if (file == FS_NULL_INODE_REF) {
            kdebug("Creating: forth.txt\n");
            file = fs_create(folder, "forth.txt", FS_FLAG_FILE);
            if (file == FS_NULL_INODE_REF) {
                kdebug("Error: unable to create forth.txt\n");
                return;
            }
        } else {
            kdebug("Opening file: forth.txt\n");
        }
        INT_VALUE(fileWord) = file;
    }

    if (INT_VALUE(currentBlock) != 0 && INT_VALUE(currentBlock) != block) {
        kdebug("Saving old block: %d\n", *currentBlock->data);
        // save old block
        fs_write(INT_VALUE(fileWord), fileIOBuffer, (INT_VALUE(currentBlock) - 1) * BLOCK_BUFFER_SIZE,
                 BLOCK_BUFFER_SIZE);
    }

    INT_VALUE(currentBlock) = block;
    int read = fs_read(INT_VALUE(fileWord), fileIOBuffer, (INT_VALUE(currentBlock) - 1) * BLOCK_BUFFER_SIZE,
                       BLOCK_BUFFER_SIZE);

    // fill remaining bytes with 0
    for (int i = read; i < BLOCK_BUFFER_SIZE; ++i) {
        fileIOBuffer[i] = 0;
    }
    pushData(valueOf(fileIOBuffer));
}

// FLUSH (--)
void fun_flush(void) {
    CHESK_FS();

    if (INT_VALUE(fileWord) == 0) {
        kdebug("No file loaded\n");
        return;
    }

    int writen = fs_write(INT_VALUE(fileWord), fileIOBuffer, (INT_VALUE(currentBlock) - 1) * BLOCK_BUFFER_SIZE,
                          BLOCK_BUFFER_SIZE);

    kdebug("Writen %d bytes\n", writen);
}

// LIST (n --)
void fun_list(void) {
    int block = popData().i32;
    if (INT_VALUE(currentBlock) != block) {
        CHESK_FS();
        pushData(valueOf(block));
        fun_block();
        if (popData().i32 == 0) {
            return;
        }
    }

    for (int i = 0; i < 16; ++i) {
        kdebug("%c", digits[i / INT_VALUE(base)]);
        kdebug("%c", digits[i % INT_VALUE(base)]);
        kdebug(" ");
        for (int j = 0; j < 64; ++j) {
            char c = fileIOBuffer[i * 64 + j];
            if (!isprint(c)) {
                kdebug(".");
            } else {
                kdebug("%c", c);
            }
        }
        kdebug("\n");
    }
}

// LOAD (n --)
void fun_load(void) {
    int block = popData().i32;
    if (INT_VALUE(currentBlock) != block) {
        CHESK_FS();
        pushData(valueOf(block));
        fun_block();
        popData();
    }
    INT_VALUE(blk) = block;
    INT_VALUE(moreIn) = 0;
    INT_VALUE(span) = (int) strlen(fileIOBuffer);
    fun_interpret();
}

// WIPE (--)
void fun_wipe(void) {
    int block = INT_VALUE(currentBlock);
    if (block == 0) {
        CHESK_FS();
        pushData(valueOf(block));
        fun_block();
    }
    memset(fileIOBuffer, ' ', 1024);
}

// PP (--)
void fun_pp(void) {
    int line = popData().i32;
    int block = INT_VALUE(currentBlock);
    if (block == 0) {
        CHESK_FS();
        pushData(valueOf(block));
        fun_block();
    }

    if (line >= 16) {
        kdebug("Invalid line: %d\n", line);
        return;
    }

    int totalSize = INT_VALUE(hashTib);
    int alreadyRead = INT_VALUE(moreIn) + 1;
    int size = totalSize - alreadyRead;
    if (size > 0) {
        memset(fileIOBuffer + line * 64, ' ', 64);
        memcpy(fileIOBuffer + line * 64, tib + alreadyRead, (size_t) size);
        INT_VALUE(moreIn) += size + 1;
    }
}

// OPEN (--) name
void fun_open(void) {
    CHESK_FS();

    pushData(valueOf(' '));
    fun_word();
    String *name = popData().str;
    if (strlen(name) == 0) {
        kdebug("Invalid file name\n");
        return;
    }

    INodeRef file = fs_findFile(INT_VALUE(fileWord), name);
    if (file != FS_NULL_INODE_REF) {
        VALUE(fileWord) = valueOf(file);
    } else {
        kdebug("File not found: '%s'\n", name);
    }
    INT_VALUE(currentBlock) = 0;
}

// CD (--) name
void fun_cd(void) {
    CHESK_FS();

    pushData(valueOf(' '));
    fun_word();
    String *name = popData().str;
    if (strlen(name) == 0) return;

    INT_VALUE(folderWord) = cmd_cd(INT_VALUE(folderWord), name);
}

// MKFILE (--) name
void fun_mkfile(void) {
    CHESK_FS();

    pushData(valueOf(' '));
    fun_word();
    String *name = popData().str;
    if (strlen(name) == 0) return;

    cmd_mkfile(INT_VALUE(folderWord), name);
}

// MKDIR (--) name
void fun_mkdir(void) {
    CHESK_FS();

    pushData(valueOf(' '));
    fun_word();
    String *name = popData().str;
    if (strlen(name) == 0) return;

    cmd_mkdir(INT_VALUE(folderWord), name);
}

// LS (--)
void fun_ls(void) {
    CHESK_FS();

    cmd_ls(INT_VALUE(folderWord));
}

// DELETE (--) name
void fun_delete(void) {
    CHESK_FS();

    pushData(valueOf(' '));
    fun_word();
    String *name = popData().str;
    if (strlen(name) == 0) return;

    cmd_rm(INT_VALUE(folderWord), name);
}

void fun_format(void) {
    if (!hasDisk()) {
        kdebug("No disk!\n");
        return;
    }
    fs_init(motherboard_get_floppy_drive());
    fs_format();
}

/*********************************************************************************************************************/
/*************************************************** Stack helpers ***************************************************/
/*********************************************************************************************************************/

// SWAP (A B -- B A)
void fun_swap(void) {
    Value b = popData();
    Value a = popData();
    pushData(b);
    pushData(a);
}

// DUP (n -- n n)
void fun_dup(void) {
    pushData(peekData());
}

// DROP (n --)
void fun_drop(void) {
    popData();
}

/*********************************************************************************************************************/
/************************************************ Compilation internal ***********************************************/
/*********************************************************************************************************************/


// EXIT (--)
void fun_exit(void) {}

void fun_run_list(void) {
    Word *word = currentWord;
    Word *next, *savedList = currentWordList;
    int saved = instructionOffset;

    currentWordList = currentWord;
    for (instructionOffset = 0;; instructionOffset++) {
        next = word->data[instructionOffset].word;
        pushData(valueOf(next));
        fun_execute();
        if (next == int_exit) break;
    }
    instructionOffset = saved;
    currentWordList = savedList;
}

// ; (--)
void fun_semi_colon(void) {
    if (!INT_VALUE(state)) return;
    fun_align_word();
    *(int *) dp = (int) int_exit;
    dp += 4;
}

// LIT (-- n)
void fun_lit(void) {
    pushData(currentWordList->data[++instructionOffset]);
}

// BRANCH? ( n --)
void fun_q_branch(void) {
    int cond = popData().i32;
    int jump = currentWordList->data[++instructionOffset].i32;
    if (!cond) {
        instructionOffset += jump;
    }
}

// BRANCH (--)
void fun_branch(void) {
    int jump = currentWordList->data[++instructionOffset].i32;
    instructionOffset += jump;
}

// IF (n --)
void fun_if(void) {
    *(int *) dp = (int) branchQWord;
    dp += 4;// word addr
    pushR(valueOf(dp));
    dp += 4; //jump addr
}

// ELSE (--)
void fun_else(void) {
    int *jump = popR().ptr;

    *(int *) dp = (int) branchWord;
    dp += 4;// word addr
    pushR(valueOf(dp));
    dp += 4; //jump addr
    int diff = ((int) dp) - ((int) jump);
    *jump = (diff / 4) - 1;
}

// THEN (--)
void fun_then(void) {
    int *jump = popR().ptr;
    int diff = ((int) dp) - ((int) jump);
    *jump = (diff / 4) - 1;
}

// DO (limit, index --)
void fun_do(void) {
    *(int *) dp = (int) doIntWord;
    dp += 4;// word addr
    pushR(valueOf(dp));
}

// (DO) (limit, index --)
void fun_int_do(void) {
    int index = popData().i32;
    int limit = popData().i32;

    pushR(valueOf(limit));
    pushR(valueOf(index));
}

// LOOP (--)
void fun_loop(void) {
    int *jumpAddr = popR().ptr;

    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) loopIntWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4;// jump addr
}

// (LOOP) (--)
void fun_int_loop(void) {
    int index = popR().i32;
    int limit = popR().i32;
    int jump = currentWordList->data[++instructionOffset].i32;

    if (index < limit - 1) {
        pushR(valueOf(limit));
        pushR(valueOf(index + 1));
        instructionOffset += jump;
    }
}

// +LOOP (--)
void fun_plus_loop(void) {
    int *jumpAddr = popR().ptr;

    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) plusLoopIntWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4;// jump addr
}

// (+LOOP) (n --)
void fun_int_plus_loop(void) {
    int inc = popData().i32;
    int index = popR().i32;
    int limit = popR().i32;
    int jump = currentWordList->data[++instructionOffset].i32;

    if (index < limit - 1) {
        pushR(valueOf(limit));
        pushR(valueOf(index + inc));
        instructionOffset += jump;
    }
}

// I (-- n)
void fun_i(void) {
    int index = peekR().i32;
    pushData(valueOf(index));
}

// J (-- n)
void fun_j(void) {
    int index = popR().i32;
    int limit = popR().i32;
    int superIndex = peekR().i32;
    pushR(valueOf(limit));
    pushR(valueOf(index));
    pushData(valueOf(superIndex));
}

// BEGIN (--)
void fun_begin(void) {
    pushR(valueOf(dp));
}

// UNTIL (n --)
void fun_until(void) {
    int *jumpAddr = popR().ptr;
    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) branchQWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4; //jump addr
}

// AGAIN (--)
void fun_again(void) {
    int *jumpAddr = popR().ptr;
    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) branchWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4; //jump addr
}

//// LEAVE (--)
//void fun_leave(){
////    int *jumpAddr = (int *) popR();
////    int diff = ((int) dp) - ((int) jumpAddr);
////    int jump = -(diff / 4) - 2;
////
////    *(int *) dp = (int) branchWord;
////    dp += 4;// word addr
////    *(int *) dp = (int) jump;
////    dp += 4; //jump addr
//}

/*********************************************************************************************************************/
/***************************************************** Compiler ******************************************************/
/*********************************************************************************************************************/

// [ (--)
void fun_open_bracket(void) {
    VALUE(state) = valueOf(0);
}

// ] (--)
void fun_close_bracket(void) {
    VALUE(state) = valueOf(1);
}

// : (--)
void fun_colon(void) {
    fun_create();
    Word *word = popData().word;
    word->code = fun_run_list;
    VALUE(state) = valueOf(1);

    while (1) {
        fun_minus_find();
        int flag = popData().i32;

        if (flag) {
            Word *toRun = peekData().word;
            if (flag == 1) {
                fun_execute();
                if (toRun == semi_colon) {
                    break;
                }
            } else {
                fun_comma();
            }
        } else {
            String *name = popData().str;

            if (strlen(name) == 0) {
                fun_query();
                continue;
            }

            pushData(valueOf(name));
            fun_q_number(); // (StringAddr -- number flag)
            int numFlag = popData().i32;
            if (numFlag) {
                fun_literal();
            } else {
                if (popData().i32 == -1) {
                    kdebug("Invalid symbol: '%s'\n", name);
                    emptyDataStack();
                    emptyRStack();
                    fun_quit();
                } else {
                    if (INT_VALUE(span) != 0) {
                        kdebug("ok\n");
                    }
                    break;
                }
            }
        }
        fun_q_stack();
    }
    VALUE(state) = valueOf(0);
}

// POSTPONE (--)
void fun_postpone(void) {
    fun_minus_find();
    int flag = popData().i32;
    if (flag) {
//        Word *toRun = (Word *) peekData();
        if (flag == 1) {
            fun_comma();
        } else {
            // TODO
            /*
             * https://www.forth.com/starting-forth/11-forth-compiler-defining-words/
             *
             * Be sure to note the "intelligence" built into POSTPONE. POSTPONE parses the next word in the input
             * stream, decides if it is immediate or not, and proceeds accordingly. If the word was not immediate,
             * POSTPONE compiles the address of the word into a compilee definition; think of it as deferred
             * compilation. If the word is immediate, POSTPONE compiles the address of this word into the definition
             * currently being defined; this is ordinary compilation, but of an immediate word which otherwise would
             * have been executed.
             *
             * WTF?!!
             * What I'm supposed to implement?
             */
        }
    }
}

// IMMEDIATE (--)
void fun_immediate(void) {
    Word *last = dictionary;
    while (last->next) {
        last = last->next;
    }
    last->flags |= IMMEDIATE_BIT_MASK;
}

/*********************************************************************************************************************/
/******************************************************* Time ********************************************************/
/*********************************************************************************************************************/

// TICKS (n --)
void fun_ticks(void) {
    int ticks = popData().i32;
    while (ticks > 0) {
        int sleep = MIN(ticks, 127);
        ticks -= sleep;
        motherboard_sleep((Byte) sleep);
    }
}

// TIMES (n --) word
void fun_times_run(void) {
    int times = popData().i32;
    fun_minus_find();
    int flag = popData().i32;
    if (flag) {
        Word *addrs = popData().word;
        for (int i = 0; i < times; ++i) {
            pushData(valueOf(addrs));
            fun_execute();
        }
    } else {
        kdebug("Not a word\n");
    }
}


/*********************************************************************************************************************/
/*************************************************** Mining Robot ****************************************************/
/*********************************************************************************************************************/

static MiningRobot *robot = NULL;
static NetworkCard *networkCard = NULL;

static void robot_signal(int signal) {
    if (robot == NULL) {
        kdebug("Unable to access mining robot API\n");
        return;
    }

    pushData(valueOf(mining_robot_signal(robot, signal)));
}

// MINE (--)
void fun_mine(void) {
    robot_signal(ROBOT_SIGNAL_MINE_BLOCK);
}

// FRONT or FORWARD (--)
void fun_front(void) {
    robot_signal(ROBOT_SIGNAL_MOVE_FORWARD);
}

// BACK (--)
void fun_back(void) {
    robot_signal(ROBOT_SIGNAL_MOVE_BACK);
}

// LEFT (--)
void fun_left(void) {
    robot_signal(ROBOT_SIGNAL_ROTATE_LEFT);
}

// RIGHT (--)
void fun_right(void) {
    robot_signal(ROBOT_SIGNAL_ROTATE_RIGHT);
}

// UP (--)
void fun_up(void) {
    robot_signal(ROBOT_SIGNAL_ROTATE_UP);
}

// DOWN (--)
void fun_down(void) {
    robot_signal(ROBOT_SIGNAL_ROTATE_DOWN);
}

// SCAN (--)
void fun_scan(void) {
    if (robot == NULL) {
        kdebug("Unable to access mining robot API\n");
        return;
    }

    pushData(valueOf(mining_robot_scan(robot)));
}

/*********************************************************************************************************************/
/**************************************************** Interpreter ****************************************************/
/*********************************************************************************************************************/

// (FIND) (StringAddr DictionaryAddr -- (wordAddr flags true) | false)
// find word in dictionary
void fun_int_find(void) {
    Word *dict = popData().word;
    String *name = popData().str;
    Word *res = findIn(dict, name);

    if (res == NULL) {
        pushData(valueOf(0));
    } else {
        pushData(valueOf(res));
        pushData(valueOf((res->flags | strlen(res->name))));
        pushData(valueOf(1));
    }
}

// WORD (charDelimiter -- StringAddr)
// Read next token
void fun_word(void) {
    char delimiter = popData().i8;
    char *buffer, current;
    int size, offset = INT_VALUE(moreIn), wordIndex;

    // Select input source
    if (INT_VALUE(blk) == 0) {
        // Terminal/Monitor
        buffer = tib;
        size = TIB_SIZE;
    } else {
        // File
        buffer = fileIOBuffer;
        size = BLOCK_SIZE;
    }

    // trim word
    for (; offset < size && isspace(buffer[offset]); ++offset);

    // copy token to wordBuffer
    for (wordIndex = 0; offset < size && wordIndex < WORD_BUFFER_SIZE - 1; ++offset) {
        current = buffer[offset];
        if (current == '\n' || current == '\0' || current == delimiter) {
            break;
        }
        wordBuffer[wordIndex++] = current;
    }
    wordBuffer[wordIndex] = '\0';

    VALUE(moreIn) = valueOf(offset);
    pushData(valueOf(wordBuffer));
}

// FIND (StringAddr -- wordAddr, flag) flag = 1 | 0 | -1 (immediate | not found | not immediate)
// Searchs a string in the dictionary and checks if is immediate
void fun_find(void) {
    String *name = popData().str;
    pushData(valueOf(name));
    pushData(valueOf(dictionary));
    fun_int_find();

    if (popData().i32) {
        int flags = popData().i32;
        pushData(valueOf((flags & IMMEDIATE_BIT_MASK) > 0 ? 1 : -1));
    } else {
        pushData(valueOf(name));
        pushData(valueOf(0));
    }
}

// -FIND (-- addr flag) wordAddr 1 | 0
// Read token from input and find if is in the dictionary
void fun_minus_find(void) {
    // Use space as delimiter
    pushData(VALUE(bl));
    // Read token from input
    fun_word();

    if (strlen(peekData().str) == 0) {

        // if token is empty, it doesn't need to search
        pushData(valueOf(0));
    } else {

        // Search command with same name
        fun_find();
    }
}

// QUIT (--)
void fun_quit(void) {
    longjmp(onError, 1);
}

// NUMBER? (StringAddr -- number flag) flag == 0  no number, flag == 1 valid number
void fun_q_number(void) {
    String *numString = popData().str;

    if (strlen(numString) == 0) {
        pushData(valueOf(0));
        pushData(valueOf(0));
        return;
    }

    int num = 0;
    if (strToInt(numString, INT_VALUE(base), &num)) {
        pushData(valueOf(num));
        pushData(valueOf(1));
    } else {
        pushData(valueOf(-1));
        pushData(valueOf(0));
    }
}

// EXECUTE (addr -- )
void fun_execute(void) {
    if (isDataStackEmpty()) {
        return;
    }
    Word *w = popData().word;
    if (w == NULL) {
        return;
    }
    if (((int) w) % 4 != 0) {
        kdebug("Word not aligned: %d\n", (int) w);
    }
    if (w->code == 0) {
        return;
    }
    currentWord = w;
    w->code();
}

// ?STACK (--)
void fun_q_stack(void) {
    if (dataStackPtr < 0 || dataStackPtr > STACK_SIZE) {
        if (dataStackPtr < 0) {
            kdebug("Error: stack underflow (%d)\n", dataStackPtr);
        } else {
            kdebug("Error: stack overflow (%d)\n", dataStackPtr);
        }
        emptyDataStack();
        emptyRStack();
        fun_quit();
    }
}

// INTERPRET (--)
void fun_interpret(void) {
    int isWord, isNumber;
    INT_VALUE(state) = 0;

    while (1) {

        // read token and search in dictionary
        fun_minus_find();
        isWord = popData().i32;

        if (isWord) {
            // the toke was a words, it gets executed
            fun_execute();
            fun_q_stack();
            continue;
        }

        String *token = peekData().str;
        // checks if it's a number and stack it if possible
        fun_q_number();
        isNumber = popData().i32;

        if (isNumber) {
            fun_q_stack();
            continue;
        }

        if (popData().i32 != -1) {
            // Print ok if the line was not empty
            if (INT_VALUE(span) > 1 && INT_VALUE(blk) == 0) {
                kdebug("ok\n");
            }
            // End of line
            break;
        }

        // Error: none of the above cases
        kdebug("Invalid symbol: '%s'\n", token);
        emptyDataStack();
        emptyRStack();
        fun_quit();
    }
}

// EXPECT (bufferAddr bufferSize -- )
// Read use input
void fun_expect(void) {
    int size = popData().i32;
    char *addr = popData().str;
    if (INT_VALUE(state)) {
        kdebug("compile: ");
    } else {
        kdebug("> ");
    }
    readString(addr, size);

    int len = (int) strlen(addr);
    *span->data = valueOf(len);
}

// QUERY (--)
// Fill TIB buffer with user input
void fun_query(void) {
    pushData(valueOf(tib));
    pushData(valueOf(TIB_SIZE));
    fun_expect();
    INT_VALUE(moreIn) = 0;
    INT_VALUE(blk) = 0;
    INT_VALUE(hashTib) = INT_VALUE(span);
}

// FORTH (--)
// Main loop
void fun_forth(void) {

    if (setjmp(onError)) {
        INT_VALUE(state) = 0;
    }
    //Read input
    fun_query();
    // Process Input
    fun_interpret();
}

static void searchPeripherals(void) {
    const struct device_header **a = motherboard_get_devices();
    networkCard = NULL;
    robot = NULL;

    for (int i = 0; i < MOTHERBOARD_MAX_DEVICES; ++i) {
        if (a[i] && a[i]->online) {
            if (a[i]->type == DEVICE_TYPE_NETWORK_CARD) {
                networkCard = (NetworkCard *) a[i];
            } else if (a[i]->type == DEVICE_TYPE_MINING_ROBOT) {
                robot = (MiningRobot *) a[i];
            }
        }
    }
}

void init(void) {
    searchPeripherals();

    // align dp to word boundaries
    fun_align_word();

    // Interpreter
    extendDictionary(createWord("FORTH", fun_forth));
    extendDictionary(createWord("QUERY", fun_query));
    extendDictionary(createWord("INTERPRET", fun_interpret));
    extendDictionary(createWord("EXPECT", fun_expect));
    extendDictionary(createWord("?STACK", fun_q_stack));
    extendDictionary(createWord("EXECUTE", fun_execute));
    extendDictionary(createWord("NUMBER?", fun_q_number));
    extendDictionary(createWord("QUIT", fun_quit));
    extendDictionary(createWord("-FIND", fun_minus_find));
    extendDictionary(createWord("FIND", fun_find));
    extendDictionary(createWord("WORD", fun_word));
    extendDictionary(createWord("(FIND)", fun_int_find));

    // Constants
    extendDictionary(createConstant("TIB", valueOf(tib)));
    extendDictionary(createConstant("CELL", valueOf(4)));
    extendDictionary(createConstant("SPACE", valueOf(' ')));

    // Variables
    moreIn = extendDictionary(createVariable(">IN", valueOf(0)));
    blk = extendDictionary(createVariable("BLK", valueOf(0)));
    span = extendDictionary(createVariable("SPAN", valueOf(0)));
    hashTib = extendDictionary(createVariable("#TIB", valueOf(0)));
    bl = extendDictionary(createVariable("BL", valueOf(' ')));
    base = extendDictionary(createVariable("BASE", valueOf(10)));
    state = extendDictionary(createVariable("STATE", valueOf(0)));
    currentBlock = extendDictionary(createVariable("#BLOCK", valueOf(0)));

    // Math
    extendDictionary(createWord("+", fun_plus));
    extendDictionary(createWord("-", fun_minus));
    extendDictionary(createWord("*", fun_times));
    extendDictionary(createWord("/", fun_div));
    extendDictionary(createWord("%", fun_mod));
    extendDictionary(createWord("=", fun_equals));

    // Memory
    extendDictionary(createWord("CELLS", fun_cells));
    extendDictionary(createWord("@", fun_at));
    extendDictionary(createWord("!", fun_set));
    extendDictionary(createWord("C@", fun_c_at));
    extendDictionary(createWord("C!", fun_c_set));
    extendDictionary(createWord("DP", fun_dp));
    extendDictionary(createWord("HERE", fun_here));
    extendDictionary(createWord(",", fun_comma));
    extendDictionary(createWord("CREATE", fun_create));
    extendDictionary(createWord("VARIABLE", fun_variable));
    extendDictionary(createWord("CONSTANT", fun_constant));
    extendDictionary(createWord("'", fun_quote));
    extendDictionary(createWord(">BODY", fun_more_body));
    extendDictionary(createWord(">DOES", fun_more_does));
    extendDictionary(createWord("FREE", fun_free));

    // Time
    extendDictionary(createWord("TIMES", fun_times_run));
    extendDictionary(createWord("TICKS", fun_ticks));

    // Compiler
    extendDictionary(createImmediateWord("]", fun_close_bracket));
    extendDictionary(createImmediateWord("[", fun_open_bracket));
    extendDictionary(createWord(":", fun_colon));
    extendDictionary(createImmediateWord("POSTPONE", fun_postpone));
    extendDictionary(createImmediateWord("IMMEDIATE", fun_immediate));
    semi_colon = extendDictionary(createImmediateWord(";", fun_semi_colon));

    // Compiler internal
    int_exit = extendDictionary(createWord("EXIT", fun_exit));
    lit = extendDictionary(createWord("LIT", fun_lit));
    int_dot_quote = extendDictionary(createWord("(.\")", fun_int_dot_quote));
    extendDictionary(createWord("ABORT\"", fun_abort));
    extendDictionary(createImmediateWord(".\"", fun_dot_quote));

    // Flow
    branchQWord = extendDictionary(createWord("BRANCH?", fun_q_branch));
    branchWord = extendDictionary(createWord("BRANCH", fun_branch));
    extendDictionary(createImmediateWord("IF", fun_if));
    extendDictionary(createImmediateWord("ELSE", fun_else));
    extendDictionary(createImmediateWord("THEN", fun_then));
    doIntWord = extendDictionary(createWord("(DO)", fun_int_do));
    loopIntWord = extendDictionary(createWord("(LOOP)", fun_int_loop));
    plusLoopIntWord = extendDictionary(createWord("(+LOOP)", fun_int_plus_loop));
    extendDictionary(createImmediateWord("DO", fun_do));
    extendDictionary(createImmediateWord("LOOP", fun_loop));
    extendDictionary(createImmediateWord("+LOOP", fun_plus_loop));
    extendDictionary(createWord("I", fun_i));
    extendDictionary(createWord("J", fun_j));
    extendDictionary(createImmediateWord("BEGIN", fun_begin));
    extendDictionary(createImmediateWord("UNTIL", fun_until));
    extendDictionary(createImmediateWord("AGAIN", fun_again));
//    extendDictionary(createImmediateWord("LEAVE", fun_leave));

    // Stack
    extendDictionary(createWord("SWAP", fun_swap));
    extendDictionary(createWord("DUP", fun_dup));
    extendDictionary(createWord("DROP", fun_drop));

    // Disk IO
    fileWord = extendDictionary(createVariable("FILE", valueOf(FS_NULL_INODE_REF)));
    folderWord = extendDictionary(createVariable("FOLDER", valueOf(fs_getRoot())));
    extendDictionary(createWord("FLUSH", fun_flush));
    extendDictionary(createWord("WIPE", fun_wipe));
    extendDictionary(createWord("LOAD", fun_load));
    extendDictionary(createWord("LIST", fun_list));
    extendDictionary(createWord("PP", fun_pp));
    extendDictionary(createWord("BLOCK", fun_block));
    extendDictionary(createWord("OPEN", fun_open));

    extendDictionary(createWord("LS", fun_ls));
    extendDictionary(createWord("MKDIR", fun_mkdir));
    extendDictionary(createWord("MKFILE", fun_mkfile));
    extendDictionary(createWord("CD", fun_cd));
    extendDictionary(createWord("DELETE", fun_delete));
    extendDictionary(createWord("FORMAT", fun_format));

    // Terminal IO
    extendDictionary(createWord(".", fun_dot));
    extendDictionary(createWord(".S", fun_dot_s));
    extendDictionary(createWord("WORDS", fun_words));
    extendDictionary(createWord("TYPE", fun_type));
    extendDictionary(createWord("DUMP", fun_dump));
    extendDictionary(createWord("KEY", fun_key));
    extendDictionary(createWord("READ", fun_read));
    extendDictionary(createWord("PAGE", fun_page));
    extendDictionary(createWord("PRINT", fun_print));
    extendDictionary(createWord("EMIT", fun_emit));
    extendDictionary(createWord("CR", fun_cr));

    // Robot
    extendDictionary(createWord("MINE", fun_mine));
    extendDictionary(createWord("FRONT", fun_front));
    extendDictionary(createWord("FORWARD", fun_front));
    extendDictionary(createWord("BACK", fun_back));
    extendDictionary(createWord("LEFT", fun_left));
    extendDictionary(createWord("RIGHT", fun_right));
    extendDictionary(createWord("UP", fun_up));
    extendDictionary(createWord("DOWN", fun_down));
    extendDictionary(createWord("SCAN", fun_scan));

    // Debug
    extendDictionary(createWord("DEBUG", fun_debug));

    fs_init(motherboard_get_floppy_drive());

    // bytes free
    fun_free();

    kdebug("%d bytes free\n", popData());
}
