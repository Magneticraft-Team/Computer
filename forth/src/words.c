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
#include "../include/dependencies.h"
#include "../include/words.h"
#include "../include/dictionary.h"
#include "../include/stack.h"
#include "../include/input.h"

/*** VARIABLES ***/

const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define VALUE(word) (*word->data)

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

void readVariableAddress() {
    pushData((int) &currentWord->data[0]);
}

void readVariableValue() {
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
            item = (Word *) word->data[i];
            if (item == lit) {
                kdebug("LIT(%d) ", word->data[++i]);
            } else if (item == int_dot_quote) {
                kdebug(".\"(");
                char *str = (char *) (word->data + i + 1);
                kdebug("%s", str);
                int count = strlen(str) + 1;
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
void fun_debug() {
    fun_minus_find();
    if (popData()) {
        Word *word = (Word *) popData();
        kdebug("\n");
        printWord(word);
    }
}

/**************/
/*** Memory ***/
/**************/

// DUMP (addr size --)
void fun_dump() {
    int size = popData();
    char *addr = (char *) popData();
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
void fun_cells() {
    pushData(popData() * 4);
}

// @ ( addr -- value )
void fun_at() {
    int *addr = (int *) popData();
    pushData(*addr);
}

// ! ( n addr -- )
void fun_set() {
    int *addr = (int *) popData();
    int value = popData();
    *addr = value;
}

// C@ ( addr -- value )
void fun_c_at() {
    char *addr = (char *) popData();
    pushData(*addr);
}

// C! ( n addr -- )
void fun_c_set() {
    char *addr = (char *) popData();
    char value = (char) popData();
    *addr = value;
}

// HERE
void fun_here() {
    pushData((int) dp);
}

// DP
void fun_dp() {
    pushData((int) &dp);
}

// ALIGNW (--)
void fun_align_word() {
    dp = (char *) (((int) dp + 3) & (~3));
}

/************/
/*** Math ***/
/************/

// + (a b -- c) c = a + b
void fun_plus() {
    pushData(popData() + popData());
}

// - (a b -- c) c = a - b
void fun_minus() {
    int b = popData();
    int a = popData();
    pushData(a - b);
}

// * (a b -- c) c = a + b
void fun_times() {
    pushData(popData() * popData());
}

// / (a b -- c) c = a / b
void fun_div() {
    int b = popData();
    int a = popData();
    pushData(a / b);
}

// % (a b -- c) c = a % b
void fun_mod() {
    int b = popData();
    int a = popData();
    pushData(a % b);
}

// = (a b -- c)
void fun_equals() {
    int b = popData();
    int a = popData();
    pushData(a == b);
}

/*************************/
/*** Word Construction ***/
/*************************/

// CREATE (--) name
void fun_create() {

    // Reads a word from input (name of the variable)
    pushData(*bl->data);
    fun_word();
    String *addr = (String *) popData();

    String *name = createString(addr);
    Word *word = newWord(name, NULL, 0, 0);

    extendDictionary(word);
    pushData((int) word);
}

// , (n -- )
void fun_comma() {
    int value = popData();
    fun_align_word();
    *(int *) dp = value;
    dp += 4;
}

// VARIABLE (--)
void fun_variable() {
    fun_create();
    Word *word = (Word *) popData();
    word->code = readVariableAddress;
    pushData(0);
    fun_comma();
}

// CONSTANT (n -- )
void fun_constant() {
    fun_create();
    Word *word = (Word *) popData();
    word->code = readVariableValue;
    fun_comma();
}

// ' (--)
void fun_quote() {
    pushData(*bl->data);
    fun_word();
    String *addr = (String *) popData();

    Word *word = findIn(dictionary, addr);
    pushData((int) word);
}

// >BODY (addr1 -- addr2)
void fun_more_body() {
    Word *word = (Word *) popData();
    pushData((int) word->data);
}

// >DOES (addr1 -- addr2)
void fun_more_does() {
    Word *word = (Word *) popData();
    pushData((int) word->code);
}

// LITERAL (n --)
void fun_literal() {
    int value = popData();
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
void fun_key() {
    char buffer[2];
    readInput(buffer, 2);
    pushData(*buffer);
}

// READ
void fun_read() {
    char buffer[80];
    readInput(buffer, 80);
    createString(buffer);
    pushData((int) createString(buffer));
}

// CR (--)
void fun_cr() {
    kdebug("\n");
}

// PAGE (--)
void fun_page() {
    monitor_clear(motherboard_get_monitor());
}

// EMIT
void fun_emit() {
    kdebug("%c", popData() & 0xFF);
}

// TYPE (--)
void fun_type() {
    String *str = (String *) popData();
    kdebug("%s", str);
}

// PRINT (--)
void fun_print() {
    char *str = (char *) popData();
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
void fun_dot() {
    if (isDataStackEmpty()) {
        kdebug("Empty stack\n");
        fun_quit();
    } else {
        printNumber(popData(), *base->data);
        kdebug(" ");
    }
}

// .S (--)  prints the stack
void fun_dot_s() {
    int i;
    for (i = 0; i < dataStackPtr; i++) {
        printNumber(dataStack[i], *base->data);
        kdebug(" ");
    }
}

// WORDS (--)
void fun_words() {
    Word *word;
    for (word = dictionary; word != NULL; word = word->next) {
        kdebug("%s ", word->name);
    }
}

// FREE (-- n)
void fun_free() {
    int totalRam = motherboard_get_memory_size();
    int ramWithoutStart = totalRam - ((int) dp);
    int stackSize = 0xffff - ((int) &totalRam);

    int freeRam = ramWithoutStart - stackSize;
    pushData(freeRam);
}

// ABORT" (--)
void fun_abort() {
    pushData('\"');
    fun_word();
    kdebug("%s ", ((String *) popData()));
    fun_quit();
}

// ." (--)
void fun_dot_quote() {
    pushData('\"');
    fun_word();
    String *text = (String *) popData();
    int size = strlen(text);
    if (size == 0) {
        return;
    }

    // scape the " char
    VALUE(moreIn)++;

    if (*state->data) {
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
void fun_int_dot_quote() {
    char *str = (char *) (currentWordList->data + instructionOffset + 1);
    kdebug("%s", str);
    int countTest = strlen(str) + 1;
    instructionOffset += byteToIntCeil(countTest);
}

/*********************************************************************************************************************/
/****************************************************** Disk IO ******************************************************/
/*********************************************************************************************************************/

#define CHESK_FS()  if(!hasDisk()){ kdebug("No disk\n"); return; }                      \
                    if(fs_getDevice() == -1){ kdebug("Disk not formatted\n"); return; } \
                    else { fs_init(motherboard_get_floppy_drive()); }

int hasDisk() {
    return disk_drive_has_disk(motherboard_get_floppy_drive());
}

// BLOCK (n -- addr) n >= 1
void fun_block() {
    int block = popData();

    CHESK_FS();

    if (*fileWord->data == FS_NULL_INODE_REF) {
        INodeRef folder = *folderWord->data;
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
        *fileWord->data = file;
    }

    if (*currentBlock->data != 0 && *currentBlock->data != block) {
        kdebug("Saving old block: %d\n", *currentBlock->data);
        // save old block
        fs_write(*fileWord->data, fileIOBuffer, (*currentBlock->data - 1) * BLOCK_BUFFER_SIZE, BLOCK_BUFFER_SIZE);
    }

    *currentBlock->data = block;
    int read = fs_read(*fileWord->data, fileIOBuffer, (*currentBlock->data - 1) * BLOCK_BUFFER_SIZE, BLOCK_BUFFER_SIZE);

    // fill remaining bytes with 0
    for (int i = read; i < BLOCK_BUFFER_SIZE; ++i) {
        fileIOBuffer[i] = 0;
    }
    pushData((int) fileIOBuffer);
}

// FLUSH (--)
void fun_flush() {
    CHESK_FS();

    if (*fileWord->data == 0) {
        kdebug("No file loaded\n");
        return;
    }

    int writen = fs_write(*fileWord->data,
                          fileIOBuffer, (*currentBlock->data - 1) * BLOCK_BUFFER_SIZE, BLOCK_BUFFER_SIZE);

    kdebug("Writen %d bytes\n", writen);
}

// LIST (n --)
void fun_list() {
    int block = popData();
    if (*currentBlock->data != block) {
        CHESK_FS();
        pushData(block);
        fun_block();
        if (popData() == 0) {
            return;
        }
    }

    for (int i = 0; i < 16; ++i) {
        kdebug("%c", digits[i / *base->data]);
        kdebug("%c", digits[i % *base->data]);
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
void fun_load() {
    int block = popData();
    if (VALUE(currentBlock) != block) {
        CHESK_FS();
        pushData(block);
        fun_block();
        popData();
    }
    *blk->data = block;
    *moreIn->data = 0;
    *span->data = strlen(fileIOBuffer);
    fun_interpret();
}

// WIPE (--)
void fun_wipe() {
    int block = *currentBlock->data;
    if (*currentBlock->data == 0) {
        CHESK_FS();
        pushData(block);
        fun_block();
    }
    memset(fileIOBuffer, ' ', 1024);
}

// PP (--)
void fun_pp() {
    int line = popData();
    int block = *currentBlock->data;
    if (*currentBlock->data == 0) {
        CHESK_FS();
        pushData(block);
        fun_block();
    }

    if (line >= 16) {
        kdebug("Invalid line: %d\n", line);
        return;
    }

    int totalSize = *hashTib->data;
    int alreadyRead = *moreIn->data + 1;
    int size = totalSize - alreadyRead;
    if (size > 0) {
        memset(fileIOBuffer + line * 64, ' ', 64);
        memcpy(fileIOBuffer + line * 64, tib + alreadyRead, (size_t) size);
        *moreIn->data += size + 1;
    }
}

// OPEN (--) name
void fun_open() {
    CHESK_FS();

    pushData(' ');
    fun_word();
    String *name = (String *) popData();
    if (strlen(name) == 0) {
        kdebug("Invalid file name\n");
        return;
    }

    INodeRef file = fs_findFile(*folderWord->data, name);
    if (file != FS_NULL_INODE_REF) {
        *fileWord->data = file;
    } else {
        kdebug("File not found: '%s'\n", name);
    }
    *currentBlock->data = 0;
}

// CD (--) name
void fun_cd() {
    CHESK_FS();

    pushData(' ');
    fun_word();
    String *name = (String *) popData();
    if (strlen(name) == 0) return;

    INodeRef child = fs_findFile(*folderWord->data, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s doesn't exist\n", name);
        return;
    }
    struct INode node;
    fs_getINode(child, &node);
    if (node.flags == FS_FLAG_DIRECTORY) {
        *folderWord->data = child;
        return;
    } else {
        kdebug("Error %s is not a folder\n", name);
        return;
    }
}

// MKFILE (--) name
void fun_mkfile() {
    CHESK_FS();

    pushData(' ');
    fun_word();
    String *name = (String *) popData();
    if (strlen(name) == 0) return;

    INodeRef res = fs_create(*folderWord->data, name, FS_FLAG_FILE);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
}

// MKDIR (--) name
void fun_mkdir() {
    if (!hasDisk()) {
        kdebug("No disk\n");
        return;
    }

    pushData(' ');
    fun_word();
    String *name = (String *) popData();
    if (strlen(name) == 0) return;

    INodeRef res = fs_create(*folderWord->data, name, FS_FLAG_DIRECTORY);
    if (res == FS_NULL_INODE_REF) {
        kdebug("Error unable to create '%s'\n", name);
    }
}

// LS (--)
void fun_ls() {
    CHESK_FS();

    struct DirectoryIterator iter;
    struct INode node;

    fs_iterInit(*folderWord->data, &iter);

    while (fs_iterNext(&iter)) {
        kdebug("%s", iter.entry.name);

        // Check if is directory
        fs_getINode(iter.entry.inode, &node);
        if (node.flags == FS_FLAG_DIRECTORY) {
            kdebug("/");
        }
        kdebug("\n");
    }
}

// DELETE (--) name
void fun_delete() {
    CHESK_FS();

    pushData(' ');
    fun_word();
    String *name = (String *) popData();
    if (strlen(name) == 0) return;

    INodeRef child = fs_findFile(*folderWord->data, name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s not found\n", name);
        return;
    }

    if (fs_delete(*folderWord->data, child)) {
        kdebug("Error unable to remove file '%s'\n", name);
        return;
    }
}

void fun_format() {
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
void fun_swap() {
    int b = popData();
    int a = popData();
    pushData(b);
    pushData(a);
}

// DUP (n -- n n)
void fun_dup() {
    pushData(peekData());
}

// DROP (n --)
void fun_drop() {
    popData();
}

/*********************************************************************************************************************/
/************************************************ Compilation internal ***********************************************/
/*********************************************************************************************************************/


// EXIT (--)
void fun_exit() {}

void fun_run_list() {
    Word *word = currentWord;
    Word *next, *savedList = currentWordList;
    int saved = instructionOffset;

    currentWordList = currentWord;
    for (instructionOffset = 0;; instructionOffset++) {
        next = (Word *) word->data[instructionOffset];
        pushData((int) next);
        fun_execute();
        if (next == int_exit) break;
    }
    instructionOffset = saved;
    currentWordList = savedList;
}

// ; (--)
void fun_semi_colon() {
    if (!(*state->data)) return;
    fun_align_word();
    *(int *) dp = (int) int_exit;
    dp += 4;
}

// LIT (-- n)
void fun_lit() {
    pushData(currentWordList->data[++instructionOffset]);
}

// BRANCH? ( n --)
void fun_q_branch() {
    int cond = popData();
    int jump = currentWordList->data[++instructionOffset];
    if (!cond) {
        instructionOffset += jump;
    }
}

// BRANCH (--)
void fun_branch() {
    int jump = currentWordList->data[++instructionOffset];
    instructionOffset += jump;
}

// IF (n --)
void fun_if() {
    *(int *) dp = (int) branchQWord;
    dp += 4;// word addr
    pushR((int) dp);
    dp += 4; //jump addr
}

// ELSE (--)
void fun_else() {
    int *jump = (int *) popR();

    *(int *) dp = (int) branchWord;
    dp += 4;// word addr
    pushR((int) dp);
    dp += 4; //jump addr
    int diff = ((int) dp) - ((int) jump);
    *jump = (diff / 4) - 1;
}

// THEN (--)
void fun_then() {
    int *jump = (int *) popR();
    int diff = ((int) dp) - ((int) jump);
    *jump = (diff / 4) - 1;
}

// DO (limit, index --)
void fun_do() {
    *(int *) dp = (int) doIntWord;
    dp += 4;// word addr
    pushR((int) dp);
}

// (DO) (limit, index --)
void fun_int_do() {
    int index = popData();
    int limit = popData();

    pushR(limit);
    pushR(index);
}

// LOOP (--)
void fun_loop() {
    int *jumpAddr = (int *) popR();

    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) loopIntWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4;// jump addr
}

// (LOOP) (--)
void fun_int_loop() {
    int index = popR();
    int limit = popR();
    int jump = currentWordList->data[++instructionOffset];

    if (index < limit - 1) {
        pushR(limit);
        pushR(index + 1);
        instructionOffset += jump;
    }
}

// +LOOP (--)
void fun_plus_loop() {
    int *jumpAddr = (int *) popR();

    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) plusLoopIntWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4;// jump addr
}

// (+LOOP) (n --)
void fun_int_plus_loop() {
    int inc = popData();
    int index = popR();
    int limit = popR();
    int jump = currentWordList->data[++instructionOffset];

    if (index < limit - 1) {
        pushR(limit);
        pushR(index + inc);
        instructionOffset += jump;
    }
}

// I (-- n)
void fun_i() {
    int index = peekR();
    pushData(index);
}

// J (-- n)
void fun_j() {
    int index = popR();
    int limit = popR();
    int superIndex = peekR();
    pushR(limit);
    pushR(index);
    pushData(superIndex);
}

// BEGIN (--)
void fun_begin() {
    pushR((int) dp);
}

// UNTIL (n --)
void fun_until() {
    int *jumpAddr = (int *) popR();
    int diff = ((int) dp) - ((int) jumpAddr);
    int jump = -(diff / 4) - 2;

    *(int *) dp = (int) branchQWord;
    dp += 4;// word addr
    *(int *) dp = jump;
    dp += 4; //jump addr
}

// AGAIN (--)
void fun_again() {
    int *jumpAddr = (int *) popR();
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
void fun_open_bracket() {
    *state->data = 0;
}

// ] (--)
void fun_close_bracket() {
    *state->data = 1;
}

// : (--)
void fun_colon() {
    fun_create();
    Word *word = (Word *) popData();
    word->code = fun_run_list;
    *state->data = 1;

    while (1) {
        fun_minus_find();
        int flag = popData();

        if (flag) {
            Word *toRun = (Word *) peekData();
            if (flag == 1) {
                fun_execute();
                if (toRun == semi_colon) {
                    break;
                }
            } else {
                fun_comma();
            }
        } else {
            String *name = (String *) popData();

            if (strlen(name) == 0) {
                fun_query();
                continue;
            }

            pushData((int) name);
            fun_q_number(); // (StringAddr -- number flag)
            int numFlag = popData();
            if (numFlag) {
                fun_literal();
            } else {
                if (popData() == -1) {
                    kdebug("Invalid symbol: '%s'\n", name);
                    emptyDataStack();
                    emptyRStack();
                    fun_quit();
                } else {
                    if (*span->data != 0) {
                        kdebug("ok\n");
                    }
                    break;
                }
            }
        }
        fun_q_stack();
    }
    *state->data = 0;
}

// POSTPONE (--)
void fun_postpone() {
    fun_minus_find();
    int flag = popData();
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
void fun_immediate() {
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
void fun_ticks() {
    int ticks = popData();
    while (ticks > 0) {
        int sleep = MIN(ticks, 127);
        ticks -= sleep;
        motherboard_sleep((Byte) sleep);
    }
}

// TIMES (n --) word
void fun_times_run() {
    int times = popData();
    fun_minus_find();
    int flag = popData();
    if (flag) {
        int addrs = popData();
        for (int i = 0; i < times; ++i) {
            pushData(addrs);
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

    pushData(mining_robot_signal(robot, signal));
}

// MINE (--)
void fun_mine() {
    robot_signal(ROBOT_SIGNAL_MINE_BLOCK);
}

// FRONT or FORWARD (--)
void fun_front() {
    robot_signal(ROBOT_SIGNAL_MOVE_FORWARD);
}

// BACK (--)
void fun_back() {
    robot_signal(ROBOT_SIGNAL_MOVE_BACK);
}

// LEFT (--)
void fun_left() {
    robot_signal(ROBOT_SIGNAL_ROTATE_LEFT);
}

// RIGHT (--)
void fun_right() {
    robot_signal(ROBOT_SIGNAL_ROTATE_RIGHT);
}

// UP (--)
void fun_up() {
    robot_signal(ROBOT_SIGNAL_ROTATE_UP);
}

// DOWN (--)
void fun_down() {
    robot_signal(ROBOT_SIGNAL_ROTATE_DOWN);
}

// SCAN (--)
void fun_scan() {
    if (robot == NULL) {
        kdebug("Unable to access mining robot API\n");
        return;
    }

    pushData(mining_robot_scan(robot));
}

/*********************************************************************************************************************/
/**************************************************** Interpreter ****************************************************/
/*********************************************************************************************************************/

// (FIND) (StringAddr DictionaryAddr -- (wordAddr flags true) | false)
// find word in dictionary
void fun_int_find() {
    Word *dict = (Word *) popData();
    String *name = (String *) popData();
    Word *res = findIn(dict, name);

    if (res == NULL) {
        pushData(0);
    } else {
        pushData((int) res);
        pushData(res->flags | strlen(res->name));
        pushData(1);
    }
}

// WORD (charDelimiter -- StringAddr)
// Read next token
void fun_word() {
    char delimiter = (char) popData();
    char *buffer, current;
    int size, offset = VALUE(moreIn), wordIndex;

    // Select input source
    if (*blk->data == 0) {
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

    VALUE(moreIn) = offset;
    pushData((int) wordBuffer);
}

// FIND (StringAddr -- wordAddr, flag) flag = 1 | 0 | -1 (immediate | not found | not immediate)
// Searchs a string in the dictionary and checks if is immediate
void fun_find() {
    String *name = (String *) popData();
    pushData((int) name);
    pushData((int) dictionary);
    fun_int_find();
    int result = popData();
    if (result) {
        int flags = popData();
        pushData((flags & IMMEDIATE_BIT_MASK) > 0 ? 1 : -1);
    } else {
        pushData((int) name);
        pushData(0);
    }
}

// -FIND (-- addr flag) wordAddr 1 | 0
// Read token from input and find if is in the dictionary
void fun_minus_find() {
    // Use space as delimiter
    pushData(VALUE(bl));
    // Read token from input
    fun_word();

    if (strlen((const char *) peekData()) == 0) {

        // if token is empty, it doesn't need to search
        pushData(0);
    } else {

        // Search command with same name
        fun_find();
    }
}

// QUIT (--)
void fun_quit() {
    longjmp(onError, 1);
}

#ifndef ENV_DEBUG

long int strtol(const char *str, char **endptr, int base) {
    long int acum = 0;
    int pos = 0;
    int sign = 1;

    if (str[pos] == '-') {
        sign = -1;
        pos++;
    } else if (str[pos] == '+') {
        pos++;
    }

    while (str[pos] != '\0') {
        int val;
        if (str[pos] >= '0' && str[pos] <= '9') {
            val = str[pos] - '0';
        } else if (str[pos] >= 'a' && str[pos] <= 'z') {
            val = str[pos] - 'a' + 10;
        } else if (str[pos] >= 'A' && str[pos] <= 'Z') {
            val = str[pos] - 'A' + 10;
        } else {
            val = -1;
        }
        if (val < 0 || val >= base) {
            *endptr = (char *) &str[pos];
            return acum;
        }
        acum = acum * base + val;
        pos++;
    }
    *endptr = (char *) &str[pos];

    return acum * sign;
}

#endif

// NUMBER? (StringAddr -- number flag) flag == 0  no number, flag == 1 valid number
void fun_q_number() {
    String *numString = (String *) popData();
    char *end = NULL;

    if (strlen(numString) == 0) {
        pushData(0);
        pushData(0);
        return;
    }

    int num = strtol(numString, &end, VALUE(base));

    if (end != (numString + strlen(numString))) {
        pushData(-1);
        pushData(0);
    } else {
        pushData(num);
        pushData(1);
    }
}

// EXECUTE (addr -- )
void fun_execute() {
    if (isDataStackEmpty()) {
        return;
    }
    Word *w = (Word *) popData();
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
void fun_q_stack() {
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
void fun_interpret() {
    int isWord, isNumber;
    *state->data = 0;

    while (1) {

        // read token and search in dictionary
        fun_minus_find();
        isWord = popData();

        if (isWord) {
            // the toke was a words, it gets executed
            fun_execute();
            fun_q_stack();
            continue;
        }

        String *token = (String *) peekData();
        // checks if it's a number and stack it if possible
        fun_q_number();
        isNumber = popData();

        if (isNumber) {
            fun_q_stack();
            continue;
        }

        if (popData() != -1) {
            // Print ok if the line was not empty
            if (*span->data > 1 && VALUE(blk) == 0) {
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
void fun_expect() {
    int size = popData();
    char *addr = (char *) popData();
    if (*state->data) {
        kdebug("compile: ");
    } else {
        kdebug("> ");
    }
    readInput(addr, size);

    int len = strlen(addr);
    *span->data = len;
}

// QUERY (--)
// Fill TIB buffer with user input
void fun_query() {
    pushData((int) tib);
    pushData(TIB_SIZE);
    fun_expect();
    *moreIn->data = 0;
    *blk->data = 0;
    *hashTib->data = *span->data;
}

// FORTH (--)
// Main loop
void fun_forth() {

    if (setjmp(onError)) {
        *state->data = 0;
    }
    //Read input
    fun_query();
    // Process Input
    fun_interpret();
}

static void searchPeripherals() {
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

void init() {
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
    extendDictionary(createConstant("TIB", (int) tib));
    extendDictionary(createConstant("CELL", 4));
    extendDictionary(createConstant("SPACE", ' '));

    // Variables
    moreIn = extendDictionary(createVariable(">IN", 0));
    blk = extendDictionary(createVariable("BLK", 0));
    span = extendDictionary(createVariable("SPAN", 0));
    hashTib = extendDictionary(createVariable("#TIB", 0));
    bl = extendDictionary(createVariable("BL", ' '));
    base = extendDictionary(createVariable("BASE", 10));
    state = extendDictionary(createVariable("STATE", 0));
    currentBlock = extendDictionary(createVariable("#BLOCK", 0));

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
    extendDictionary(createWord("ABORT\"", fun_abort));
    extendDictionary(createImmediateWord(".\"", fun_dot_quote));
    int_dot_quote = extendDictionary(createWord("(.\")", fun_int_dot_quote));

    // Flow
    branchQWord = extendDictionary(createWord("BRANCH?", fun_q_branch));
    branchWord = extendDictionary(createWord("BRANCH", fun_branch));
    extendDictionary(createImmediateWord("IF", fun_if));
    extendDictionary(createImmediateWord("ELSE", fun_else));
    extendDictionary(createImmediateWord("THEN", fun_then));
    doIntWord = extendDictionary(createWord("(DO)", fun_int_do));
    extendDictionary(createImmediateWord("DO", fun_do));
    loopIntWord = extendDictionary(createWord("(LOOP)", fun_int_loop));
    extendDictionary(createImmediateWord("LOOP", fun_loop));
    plusLoopIntWord = extendDictionary(createWord("(+LOOP)", fun_int_plus_loop));
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
    fileWord = extendDictionary(createVariable("FILE", FS_NULL_INODE_REF));
    folderWord = extendDictionary(createVariable("FOLDER", fs_getRoot()));
    extendDictionary(createWord("BLOCK", fun_block));
    extendDictionary(createWord("LIST", fun_list));
    extendDictionary(createWord("LOAD", fun_load));
    extendDictionary(createWord("WIPE", fun_wipe));
    extendDictionary(createWord("PP", fun_pp));
    extendDictionary(createWord("FLUSH", fun_flush));
    extendDictionary(createWord("LS", fun_ls));
    extendDictionary(createWord("OPEN", fun_open));
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
