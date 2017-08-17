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


void readVariableAddress(Word *word) {
    pushData((int) &word->data[0]);
}

void readVariableValue(Word *word) {
    pushData(word->data[0]);
}

void *allot(int size) {
    char *aux = dp;
    dp = aux + size;
    return aux;
}

/***  WORDS  ***/

// (FIND) (StringAddr DictionaryAddr -- (wordAddr flags true) | false)
void fun_int_find(Word *word IGNORED) {
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
void fun_word(Word *word IGNORED) {
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
        if (current == '\n' || current == '\0' || current == delimiter) break;
        wordBuffer[j++] = current;
    }
    wordBuffer[0] = (char) (j - 1);
    wordBuffer[j] = '\0';
    moreIn->data[0] = offset;
    pushData((int) wordBuffer);
}

// FIND (StringAddr -- StringAddr, flag) flag = 1 | 0 | -1 (immediate | not found | not immediate)
void fun_find(Word *word IGNORED) {
    String *name = (String *) popData();
    pushData((int) name);
    pushData((int) dictionary);
    fun_int_find(NULL);
    int result = popData();
    if (result) {
        int flags = popData();
        pushData((flags & IMMEDIATE_BIT_MASK) > 0 ? 1 : -1);
    } else {
        pushData((int) name);
        pushData(0);
    }
}

// -FIND (-- StringAddr flag)
void fun_minus_find(Word *word IGNORED) {
    //read token from input
    pushData(bl->data[0]);
    fun_word(NULL);

    // if token is empty, it doesn't need to search
    if (((String *) peekData())->size == 0) {
        pushData(0);
        return;
    }
    //search command with same name
    fun_find(NULL);
}

// QUIT
void fun_quit(Word *word IGNORED) {
    longjmp(onError, 1);
}

// NUMBER? (StringAddr -- number flag)
void fun_q_number(Word *word IGNORED) {
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
void fun_execute(Word *word IGNORED) {
    if (isDataStackEmpty()) {
        return;
    }
    Word *w = (Word *) popData();
    if (w == NULL || w->code == 0) {
        return;
    }
    w->code(w);
}

// ?STACK (--)
void fun_q_stack(Word *word IGNORED) {
    if (dataStackPtr < 0 || dataStackPtr > STACK_SIZE) {
        printf("Error: invalid stack ptr %d\n", dataStackPtr);
        emptyDataStack();
        fun_quit(NULL);
    }
}

// INTERPRET (--)
void fun_interpret(Word *word IGNORED) {
    int flag;
    String *name;
    while (1) {
        fun_minus_find(NULL);
        flag = popData();
        if (flag) {
            fun_execute(NULL);
        } else {
            name = (String *) peekData();
            fun_q_number(NULL);
            flag = popData();
            if (!flag) {
                if (popData() == -1) {
                    printf("Invalid symbol: '%s'\n", name->array);
                    fun_quit(NULL);
                } else {
                    break;
                }
            }
        }
        fun_q_stack(NULL);
    }
}

// EXPECT (bufferAddr bufferSize -- )
void fun_expect(Word *word IGNORED) {
    int size = popData();
    char *addr = (char *) popData();
    printf(">");
    fgets(addr, size, stdin);
    putchar('\n');
    *span->data = strlen(addr);
}

// QUERY (--)
void fun_query(Word *word IGNORED) {
    pushData((int) tib);
    pushData(TIB_SIZE);
    fun_expect(NULL);
    *moreIn->data = 0;
    *blk->data = 0;
    *hashTib->data = *span->data;
}

// FORTH (--)
void fun_forth(Word *word IGNORED) {
    setjmp(onError);
    fun_query(NULL);
    fun_interpret(NULL);
    printf(" ok\n");
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
void fun_dot(Word *a IGNORED) {
    if (isDataStackEmpty()) {
        printf("Empty stack\n");
        fun_quit(NULL);
    } else {
        printNumber(popData(), *base->data);
        putchar(' ');
    }
}

// .S (--)  prints the stack
void fun_dot_s(Word *a IGNORED) {
    int i;
    for (i = dataStackPtr - 1; i >= 0; i--) {
        printNumber(dataStack[i], *base->data);
        putchar(' ');
    }
}

// @ ( addr -- value )
void fun_at(Word *a IGNORED) {
    int *addr = (int *) popData();
    pushData(*addr);
}

// ! ( n addr -- )
void fun_set(Word *a IGNORED) {
    int *addr = (int *) popData();
    int value = popData();
    *addr = value;
}

// C@ ( addr -- value )
void fun_c_at(Word *a IGNORED) {
    char *addr = (char *) popData();
    pushData(*addr);
}

// C! ( n addr -- )
void fun_c_set(Word *a IGNORED) {
    char *addr = (char *) popData();
    char value = (char) popData();
    *addr = value;
}

// HERE
void fun_here(Word *a IGNORED) {
    pushData((int) dp);
}

// DP
void fun_dp(Word *a IGNORED) {
    pushData((int) &dp);
}

// ALIGNW (--)
void fun_align_word(Word *a IGNORED) {
    dp = (char *) (((int) dp + 3) & (~3));
}

// CREATE (--) name
void fun_create(Word *a IGNORED) {

    // Reads a word from input (name of the variable)
    pushData(*bl->data);
    fun_word(NULL);
    String *addr = (String *) popData();

    String *name = createString(addr->array);
    Word *word = newWord(name, NULL, 0);

    extendDictionary(word);
}

// , (n -- )
void fun_comma(Word *a IGNORED) {
    int value = popData();
    fun_align_word(NULL);
    *(int *) dp = value;
    dp += 4;
}

// VARIABLE (--)
void fun_variable(Word *a IGNORED) {
    fun_create(NULL);
    *((int *) dp - 1) = (int) readVariableAddress;
    pushData(0);
    fun_comma(NULL);
}

// CONSTANT (n -- )
void fun_constant(Word *a IGNORED) {
    fun_create(NULL);
    *((int *) dp - 1) = (int) readVariableValue;
    fun_comma(NULL);
}

// WORDS (--)
void fun_words(Word *a IGNORED) {
    Word *word;
    for (word = dictionary; word != NULL; word = word->next) {
        printf("%s ", word->name->array);
    }
}


// DUMP (addr size --)
void fun_dump(Word *a IGNORED) {
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

void fun_cells(Word *a IGNORED) {
    pushData(popData() * 4);
}

// + (a b -- c) c = a + b
void fun_plus(Word *a IGNORED) {
    pushData(popData() + popData());
}

// - (a b -- c) c = a - b
void fun_minus(Word *ignore IGNORED) {
    int b = popData();
    int a = popData();
    pushData(a - b);
}

// * (a b -- c) c = a + b
void fun_times(Word *a IGNORED) {
    pushData(popData() * popData());
}

// / (a b -- c) c = a / b
void fun_div(Word *ignore IGNORED) {
    int b = popData();
    int a = popData();
    pushData(a / b);
}

// % (a b -- c) c = a % b
void fun_mod(Word *ignore IGNORED) {
    int b = popData();
    int a = popData();
    pushData(a % b);
}

// PAGE (--)
void fun_page(Word *ignore IGNORED) {
    clear_screen();
}

// EMIT
void fun_emit(Word *ignore IGNORED) {
    putchar(popData() & 0xFF);
}

// TYPE (--)
void fun_type(Word *a IGNORED) {
    int size = popData(), i;
    char *addr = (char *) popData();

    for (i = 0; i < size; ++i) {
        putchar(addr[i]);
    }
}

// PRINT (--)
void fun_print(Word *a IGNORED) {
    String *str = (String *) popData();
    printf("%s ", str->array);
}

// ' (--)
void fun_quote(Word *a IGNORED) {
    pushData(*bl->data);
    fun_word(NULL);
    String *addr = (String *) popData();

    Word *word = findIn(dictionary, addr);
    pushData((int) word);
}

// >BODY (addr1 -- addr2)
void fun_more_body(Word *a IGNORED) {
    Word *word = (Word *) popData();
    pushData((int) word->data);
}

// >DOES (addr1 -- addr2)
void fun_more_does(Word *a IGNORED) {
    Word *word = (Word *) popData();
    pushData((int) word->code);
}

// CR (--)
void fun_cr(Word *ignored IGNORED) {
    putchar('\n');
}

// BLOCK (n -- addr)
void fun_block(Word *ignored IGNORED) {
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

// KEY (-- char)
void fun_key(Word *ignored IGNORED) {
    int k = getchar();
    while (iscntrl(k)) {
        k = getchar();
    }
    putchar(k);
    pushData(k);
}


// LIST (n --)
void fun_list(Word *ignored IGNORED) {
    int block = popData();
    if (currentBlock != block) {
        pushData(block);
        fun_block(NULL);
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
void fun_load(Word *ignored IGNORED) {
    int block = popData();
    if (currentBlock != block) {
        pushData(block);
        fun_block(NULL);
    }
    *blk->data = block;
    fun_interpret(NULL);
}

// FLUSH (--)
void fun_flush(Word *ignored IGNORED) {
    int block = currentBlock;
    DiskDrive drive = motherboard_get_floppy_drive();

    if (block != 0 && disk_drive_has_disk(drive) && block < disk_drive_get_num_sectors(drive)) {

        disk_drive_set_current_sector(drive, block - 1);
        disk_drive_signal(drive, DISK_DRIVE_SIGNAL_WRITE);

        motherboard_sleep((i8) disk_drive_get_access_time(drive));
    }
}
// WIPE (--)
void fun_wipe(Word *ignored IGNORED) {
    int block = currentBlock;
    if (currentBlock == 0) {
        pushData(block);
        fun_block(NULL);
    }
    DiskDrive drive = motherboard_get_floppy_drive();
    char *buffer = (char *) disk_drive_get_buffer(drive);
    memset(buffer, ' ', 1024);
}

// PP (--)
void fun_pp(Word *ignored IGNORED) {
    int line = popData();
    int block = currentBlock;
    if (currentBlock == 0) {
        pushData(block);
        fun_block(NULL);
    }
    DiskDrive drive = motherboard_get_floppy_drive();
    char *buffer = (char *) disk_drive_get_buffer(drive);

    int totalSize = *hashTib->data;
    int alreadyRead = *moreIn->data + 1;
    int size = totalSize - alreadyRead;
    if(size > 0) {
        memcpy(buffer + line * 64, tib + alreadyRead, (size_t) size);
        *moreIn->data += size + 1;
    }
}

void init() {
    fun_align_word(NULL);
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
}
