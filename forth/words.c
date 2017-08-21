//
// Created by cout970 on 2017-08-13.
//
// https://www.forth.com/starting-forth/9-forth-execution/
// http://astro.pas.rochester.edu/Forth/forth-words.html#fetch
// https://www.forth.com/starting-forth/1-forth-stacks-dictionary/
//

#include "dependencies.h"
#include "words.h"
#include "dictionary.h"
#include "stack.h"

/*** VARIABLES ***/

const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

jmp_buf onError;

// TIB (token input buffer)
char tib[TIB_SIZE];

char wordBuffer[WORD_BUFFER_SIZE];

int currentBlock = 0;
char *block;

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

Word *currentWord = NULL;
Word *currentWordList = NULL;
int instructionOffset = 0;

void readVariableAddress() {
    pushData((int) &currentWord->data[0]);
}

void readVariableValue() {
    pushData(currentWord->data[0]);
}

void *allot(int size) {
    char *aux = dp;
    dp = aux + size;
    return aux;
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

    for (i = 0; i < lines; ++i) {
        base = addr + i * 16;

        putchar(' ');
        putchar('0');
        putchar('x');

        putchar(digits[((int) base & 0xF0000000) >> 28]);
        putchar(digits[((int) base & 0x0F000000) >> 24]);
        putchar(digits[((int) base & 0x00F00000) >> 20]);
        putchar(digits[((int) base & 0x000F0000) >> 16]);
        putchar(digits[((int) base & 0x0000F000) >> 12]);
        putchar(digits[((int) base & 0x00000F00) >> 8]);
        putchar(digits[((int) base & 0x000000F0) >> 4]);
        putchar(digits[((int) base & 0x0000000F) >> 0]);

        putchar(':');
        putchar(' ');
        for (j = 0; j < 16; ++j) {
            aux = base + j;
            if (aux < end) {
                putchar(digits[(*aux & 0xF0) >> 4]);
                putchar(digits[(*aux & 0x0F) >> 0]);
            } else {
                putchar('-');
                putchar('-');
            }
            putchar(' ');
        }
        putchar(' ');

        for (j = 0; j < 16; ++j) {
            if (iscntrl(base[j])) {
                putchar('.');
            } else {
                putchar(base[j]);
            }
        }
        putchar('\n');
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

/*************************/
/*** Word Construction ***/
/*************************/

// CREATE (--) name
void fun_create() {

    // Reads a word from input (name of the variable)
    pushData(*bl->data);
    fun_word();
    String *addr = (String *) popData();

    String *name = createString(addr->array);
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
    int k = getchar();
    while (iscntrl(k)) {
        k = getchar();
    }
    putchar(k);
    pushData(k);
}

// CR (--)
void fun_cr() {
    putchar('\n');
}

// PAGE (--)
void fun_page() {
    clear_screen();
}

// EMIT
void fun_emit() {
    putchar(popData() & 0xFF);
}

// TYPE (--)
void fun_type() {
    int size = popData(), i;
    char *addr = (char *) popData();

    for (i = 0; i < size; ++i) {
        putchar(addr[i]);
    }
}

// PRINT (--)
void fun_print() {
    String *str = (String *) popData();
    printf("%s ", str->array);
}

static int printNumber(int num, int base) {
    int sum = 0;
    if (num < 0) {
        putchar('-');
        sum++;
        num = -num;
    }
    if (num < base) {
        putchar(digits[num]);
        sum++;
    } else {
        sum += printNumber(num / base, base);
        putchar(digits[num % base]);
        sum++;
    }
    return sum;
}

// . ( a -- ) pop a value and prints it
void fun_dot() {
    if (isDataStackEmpty()) {
        printf("Empty stack\n");
        fun_quit();
    } else {
        printNumber(popData(), *base->data);
        putchar(' ');
    }
}

// .S (--)  prints the stack
void fun_dot_s() {
    int i;
    for (i = dataStackPtr - 1; i >= 0; i--) {
        printNumber(dataStack[i], *base->data);
        putchar(' ');
    }
}

// WORDS (--)
void fun_words() {
    Word *word;
    for (word = dictionary; word != NULL; word = word->next) {
        printf("%s ", word->name->array);
    }
}

int byteToIntCeil(int bytes){
    int ints = bytes/4;
    int rest = bytes - ints * 4;

    return ints + ((rest != 0) ? 1 : 0);
}

void printWord(Word *word) {
    printf("Word(ptr = %x (%d),\n", (unsigned int) word, (int) word);
    printf("name = '%s'(%d), \n", word->name->array, word->name->size);
    printf("flags = %d (%s), \n", word->flags, (word->flags & IMMEDIATE_BIT_MASK) ? "IMMEDIATE" : "NORMAL");
    printf("next = %d (%s), \n", (int) word->next, word->next ? word->next->name->array : "NULL");
    int type = word->code == readVariableAddress
               ? 0//"VARIABLE"
               : word->code == readVariableValue
                 ? 1//"CONSTANT"
                 : word->code == fun_run_list
                   ? 2 //"COMPILED WORD"
                   : 3; //"NATIVE"

    printf("code = %d (%s)", (int) word->code,
           type == 0 ? "VARIABLE" : type == 1 ? "CONSTANT" : type == 2 ? "COMPILED WORD" : "NATIVE");

    if (type == 0 || type == 1) {
        printf(", \nvalue = %d (0x%x)", word->data[0], word->data[0]);
    } else if (type == 2) {
        printf(", \nwords: [ ");
        Word *item;
        for (int i = 0;; ++i) {
            item = (Word *) word->data[i];
            if (item == lit) {
                printf("LIT(%d) ", word->data[++i]);
            } else if (item == int_dot_quote) {
                printf(".\"(");
                char* str = (char *)(word->data + i + 1);
                int count = printf("%s", str) + 1;
                i += byteToIntCeil(count);
                printf(")");
            } else {
                printf("%s(%x) ", item->name->array, (unsigned int) (word->data + i));
            }

            if (item == int_exit || item == NULL) break;
        }
        printf("]");
    }
    printf(")");
}

// DEBUG (--) name
void fun_debug() {
    fun_minus_find();
    if (popData()) {
        Word *word = (Word *) popData();
        printWord(word);
    }
}

// ABORT" (--)
void fun_abort() {
    pushData('\"');
    fun_word();
    printf("%s", ((String *) popData())->array);
    fun_quit();
}

// ." (--)
void fun_dot_quote() {
    pushData('\"');
    fun_word();
    String *text = (String *) popData();
    if(text->size == 0)
        return;

    // scape the " char
    moreIn->data[0]++;

    if (*state->data) {
        *(int*)dp = (int) int_dot_quote;
        dp += 4;

        for (int i = 0; i < text->size; ++i) {
            *dp = text->array[i];
            dp++;
        }

        *dp = '\0';
        dp++;
        fun_align_word();
    } else {
        printf("%s", text->array);
    }
}

// (.") (--)
void fun_int_dot_quote() {
    char* str = (char *)(currentWordList->data + instructionOffset + 1);
    int countTest = printf("%s", str) + 1;
    instructionOffset += byteToIntCeil(countTest);
}

/***************/
/*** Disk IO ***/
/***************/

// BLOCK (n -- addr)
void fun_block() {
    int block = popData();
    DiskDrive drive = motherboard_get_floppy_drive();

    if (disk_drive_has_disk(drive) && block < disk_drive_get_num_sectors(drive)) {

        disk_drive_set_current_sector(drive, block - 1);
        disk_drive_signal(drive, DISK_DRIVE_SIGNAL_READ);

        motherboard_sleep((i8) disk_drive_get_access_time(drive));

        currentBlock = block;
        pushData((int) disk_drive_get_buffer(drive));
    } else {
        pushData(0);
    }
}

// LIST (n --)
void fun_list() {
    int block = popData();
    if (currentBlock != block) {
        pushData(block);
        fun_block();
    }
    DiskDrive drive = motherboard_get_floppy_drive();
    char *buffer = (char *) disk_drive_get_buffer(drive);

    for (int i = 0; i < 16; ++i) {
        putchar(digits[i / *base->data]);
        putchar(digits[i % *base->data]);
        putchar(' ');
        for (int j = 0; j < 64; ++j) {
            putchar(buffer[i * 64 + j]);
        }
        putchar('\n');
    }
}

// LOAD (n --)
void fun_load() {
    int block = popData();
    if (currentBlock != block) {
        pushData(block);
        fun_block();
    }
    *blk->data = block;
    fun_interpret();
}

// FLUSH (--)
void fun_flush() {
    int block = currentBlock;
    DiskDrive drive = motherboard_get_floppy_drive();

    if (block != 0 && disk_drive_has_disk(drive) && block < disk_drive_get_num_sectors(drive)) {

        disk_drive_set_current_sector(drive, block - 1);
        disk_drive_signal(drive, DISK_DRIVE_SIGNAL_WRITE);

        motherboard_sleep((i8) disk_drive_get_access_time(drive));
    }
}

// WIPE (--)
void fun_wipe() {
    int block = currentBlock;
    if (currentBlock == 0) {
        pushData(block);
        fun_block();
    }
    DiskDrive drive = motherboard_get_floppy_drive();
    char *buffer = (char *) disk_drive_get_buffer(drive);
    memset(buffer, ' ', 1024);
}

// PP (--)
void fun_pp() {
    int line = popData();
    int block = currentBlock;
    if (currentBlock == 0) {
        pushData(block);
        fun_block();
    }
    DiskDrive drive = motherboard_get_floppy_drive();
    char *buffer = (char *) disk_drive_get_buffer(drive);

    int totalSize = *hashTib->data;
    int alreadyRead = *moreIn->data + 1;
    int size = totalSize - alreadyRead;
    if (size > 0) {
        memcpy(buffer + line * 64, tib + alreadyRead, (size_t) size);
        *moreIn->data += size + 1;
    }
}

/*********************/
/*** Stack helpers ***/
/*********************/

// DUP (n -- n n)
void fun_dup() {
    pushData(peekData());
}

// DROP (n --)
void fun_drop() {
    popData();
}

/****************************/
/*** Compilation internal ***/
/****************************/

// EXIT (--)
void fun_exit() {}

void fun_run_list() {
    Word *word = currentWord;
    Word *next, *savedList = currentWordList;
    int saved = instructionOffset;

    currentWordList = currentWord;
    for (instructionOffset = 0;; instructionOffset++) {
        next = (Word *) word->data[instructionOffset];
//        printf("Executing word: \n");
//        printWord(next);
//        putchar('\n');
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

/****************/
/*** Compiler ***/
/****************/

// : (--)
void fun_colon() {
    fun_create();
    Word *word = (Word *) popData();
    word->code = fun_run_list;
    //fun_bracket();
//    printf("Word name: '%s'\n", word->name->array);
    *state->data = 1;

    while (1) {
        fun_minus_find();
        int flag = popData();

        if (flag) {
            Word *toRun = (Word *) peekData();
//            printf("%s Word: '%s', toRun: %d, semi_colon: %d\n", flag == 1 ? "Executing" : "Compiling",
//                   toRun->name->array,
//                   (int) toRun, (int) semi_colon);
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

            if (name->size == 0) {
                fun_query();
                continue;
            }

            pushData((int) name);
            fun_q_number(); // (StringAddr -- number flag)
            int numFlag = popData();
            if (numFlag) {
//                printf("Compiling number: %d\n", peekData());
                fun_literal();
            } else {
                printf("Invalid symbol: '%s'\n", name->array);
                fun_quit();
            }
        }
        fun_q_stack();
    }
    *state->data = 0;
}

/*********************/
/***  Interpreter  ***/
/*********************/

// (FIND) (StringAddr DictionaryAddr -- (wordAddr flags true) | false)
void fun_int_find() {
    Word *dict = (Word *) popData();
    String *name = (String *) popData();
    Word *res = findIn(dict, name);

    if (res == NULL) {
        pushData(0);
    } else {
        pushData((int) res);
        pushData(res->flags | res->name->size);
        pushData(1);
    }
}

// WORD (charDelimiter -- StringAddr)
void fun_word() {
    char delimiter = (char) popData();
    char *buffer, current;
    int size, offset = moreIn->data[0], j;

    if (*blk->data == 0) {
        buffer = tib;
        size = TIB_SIZE;
    } else {
        buffer = block;
        size = BLOCK_SIZE;
    }

    // trim word
    for (; offset < size && buffer[offset] == (char) *bl->data; ++offset);

    for (j = 1; offset < size; ++offset) {
        current = buffer[offset];
        if (j >= WORD_BUFFER_SIZE - 1) break;
        if (current == '\n' || current == '\0' || current == delimiter) {
            break;
        }
        wordBuffer[j++] = current;
    }
    wordBuffer[0] = (char) (j - 1);
    wordBuffer[j] = '\0';
    moreIn->data[0] = offset;
    pushData((int) wordBuffer);
}

// FIND (StringAddr -- wordAddr, flag) flag = 1 | 0 | -1 (immediate | not found | not immediate)
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
void fun_minus_find() {
    //read token from input
    pushData(bl->data[0]);
    fun_word();

    // if token is empty, it doesn't need to search
    if (((String *) peekData())->size == 0) {
        pushData(0);
        return;
    }
    //search command with same name
    fun_find();
}

// QUIT
void fun_quit() {
    longjmp(onError, 1);
}

// NUMBER? (StringAddr -- number flag) flag == 0  no number, flag == 1 valid number
void fun_q_number() {
    String *numString = (String *) popData();
    char *end = NULL;

    if (numString->size == 0) {
        pushData(0);
        pushData(0);
        return;
    }

    int num = strtol(numString->array, &end, base->data[0]);

    if (end != (numString->array + numString->size)) {
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
    if (w == NULL || w->code == 0) {
        return;
    }
    currentWord = w;
    w->code();
}

// ?STACK (--)
void fun_q_stack() {
    if (dataStackPtr < 0 || dataStackPtr > STACK_SIZE) {
        printf("Error: invalid stack ptr %d\n", dataStackPtr);
        emptyDataStack();
        fun_quit();
    }
}

// INTERPRET (--)
void fun_interpret() {
    int flag;
    String *name;
    *state->data = 0;
    while (1) {
        fun_minus_find();
        flag = popData();
        if (flag) {
            fun_execute();
        } else {
            name = (String *) peekData();
            fun_q_number();
            flag = popData();
            if (!flag) {
                if (popData() == -1) {
                    printf("Invalid symbol: '%s'\n", name->array);
                    fun_quit();
                } else {
                    break;
                }
            }
        }
        fun_q_stack();
    }
}

// EXPECT (bufferAddr bufferSize -- )
void fun_expect() {
    int size = popData();
    char *addr = (char *) popData();
    if (*state->data) {
        printf("compile:");
    } else {
        printf(">");
    }
    fgets(addr, size, stdin);
    putchar('\n');
    *span->data = strlen(addr);
}

// QUERY (--)
void fun_query() {
    pushData((int) tib);
    pushData(TIB_SIZE);
    fun_expect();
    *moreIn->data = 0;
    *blk->data = 0;
    *hashTib->data = *span->data;
}

// FORTH (--)
void fun_forth() {
    setjmp(onError);
    fun_query();
    fun_interpret();

    if (*span->data != 0) {
        printf(" ok\n");
    }
}


//TODO
/*
 * POSTPONE
 */

void init() {
    fun_align_word();
    block = (char *) disk_drive_get_buffer(motherboard_get_floppy_drive());

    extendDictionary(createWord("FORTH", fun_forth));
    extendDictionary(createWord("QUERY", fun_query));
    extendDictionary(createWord("INTERPRET", fun_interpret));
    extendDictionary(createConstant("TIB", (int) tib));
    extendDictionary(createConstant("BLOCK", (int) block));
    extendDictionary(createConstant("CELL", 4));
    extendDictionary(createConstant("SPACE", ' '));
    moreIn = extendDictionary(createVariable(">IN", 0));
    blk = extendDictionary(createVariable("BLK", 0));
    span = extendDictionary(createVariable("SPAN", 0));
    hashTib = extendDictionary(createVariable("#TIB", 0));
    bl = extendDictionary(createVariable("BL", ' '));
    extendDictionary(createWord("WORD", fun_word));
    extendDictionary(createWord("NUMBER?", fun_q_number));
    extendDictionary(createWord("-FIND", fun_minus_find));
    extendDictionary(createWord("(FIND)", fun_int_find));
    base = extendDictionary(createVariable("BASE", 10));
    state = extendDictionary(createVariable("STATE", 0));
    extendDictionary(createWord("EXECUTE", fun_execute));
    extendDictionary(createWord(".", fun_dot));
    extendDictionary(createWord(".S", fun_dot_s));
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
    extendDictionary(createWord("WORDS", fun_words));
    extendDictionary(createWord("TYPE", fun_type));
    extendDictionary(createWord("DUMP", fun_dump));
    extendDictionary(createWord("CELLS", fun_cells));
    extendDictionary(createWord("+", fun_plus));
    extendDictionary(createWord("-", fun_minus));
    extendDictionary(createWord("*", fun_times));
    extendDictionary(createWord("/", fun_div));
    extendDictionary(createWord("%", fun_mod));
    extendDictionary(createWord("PAGE", fun_page));
    extendDictionary(createWord("PRINT", fun_print));
    extendDictionary(createWord("EMIT", fun_emit));
    extendDictionary(createWord("CR", fun_cr));
    extendDictionary(createWord("'", fun_quote));
    extendDictionary(createWord(">BODY", fun_more_body));
    extendDictionary(createWord(">DOES", fun_more_does));
    extendDictionary(createWord("BLOCK", fun_block));
    extendDictionary(createWord("KEY", fun_key));
    extendDictionary(createWord("LIST", fun_list));
    extendDictionary(createWord("LOAD", fun_load));
    extendDictionary(createWord("WIPE", fun_wipe));
    extendDictionary(createWord("PP", fun_pp));
    extendDictionary(createWord("FLUSH", fun_flush));
    extendDictionary(createWord("DUP", fun_dup));
    extendDictionary(createWord("DROP", fun_drop));
    extendDictionary(createWord(":", fun_colon));
    semi_colon = extendDictionary(createInmediateWord(";", fun_semi_colon));
    lit = extendDictionary(createWord("LIT", fun_lit));
    int_exit = extendDictionary(createWord("EXIT", fun_exit));
    extendDictionary(createWord("ABORT\"", fun_abort));
    extendDictionary(createInmediateWord(".\"", fun_dot_quote));
    int_dot_quote = extendDictionary(createWord("(.\")", fun_int_dot_quote));
    extendDictionary(createWord("DEBUG", fun_debug));
}
