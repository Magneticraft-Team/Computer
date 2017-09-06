//
// Created by cout970 on 2017-08-12.
//

#include "lisp.h"

/*** Primitives ***/

Object *prim_sum(Object *args) {
    int sum;
    for (sum = 0; !isNil(args); sum += getInt(getFirst(args)), args = getRest(args));
    return createInt(sum);
}

Object *prim_sub(Object *args) {
    int sum;
    for (sum = getInt(getFirst(args)), args = getRest(args);
         !isNil(args);
         sum -= getInt(getFirst(args)), args = getRest(args));
    return createInt(sum);
}

Object *prim_prod(Object *args) {
    int prod;
    for (prod = 1; !isNil(args); prod *= getInt(getFirst(args)), args = getRest(args));
    return createInt(prod);
}

Object *prim_divide(Object *args) {
    int prod = getInt(getFirst(args));
    args = getRest(args);
    while (!isNil(args)) {
        prod /= getInt(getFirst(args));
        args = getRest(args);
    }
    return createInt(prod);
}

Object *prim_mod(Object *args) {
    int prod = getInt(getFirst(args));
    args = getRest(args);
    while (!isNil(args)) {
        prod %= getInt(getFirst(args));
        args = getRest(args);
    }
    return createInt(prod);
}

Object *prim_gt(Object *args) {
    return getInt(getFirst(args)) > getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_lt(Object *args) {
    return getInt(getFirst(args)) < getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_ge(Object *args) {
    return getInt(getFirst(args)) >= getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_le(Object *args) {
    return getInt(getFirst(args)) <= getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_num_eq(Object *args) {
    return getInt(getFirst(args)) == getInt(getFirst(getRest(args))) ? tee : nil;
}

Object *prim_cons(Object *args) { return createCons(getFirst(args), getFirst(getRest(args))); }

Object *prim_car(Object *args) { return getFirst(getFirst(args)); }

Object *prim_cdr(Object *args) { return getRest(getFirst(args)); }


/*** Helpers ***/

Object *prim_print(Object *args) {
    while (!isNil(args)) {
        printObjFormatted(getFirst(args));
        args = getRest(args);
        printf(" ");
    }
    printf("\n");
    return nil;
}

Object *prim_clear(Object *args IGNORED) {
    clear_screen();
    monitor_set_cursor_pos_y(motherboard_get_monitor(), 0);
    write_output_flag = 1;
    return nil;
}

Object *prim_free(Object *args) {
    if (!isNil(args)) { printf(""); }
    int used = (int) malloc(4);
    free((void *) used);
    register int *sp asm ("sp");

    int free = motherboard_get_memory_size() - used - (0xFFFF - (int) sp);
    printf("%d bytes used, %d bytes free, malloc ptr: %d, string count: %d, sp: %d\n",
           total_malloc, free, used, string_count, (int) sp);
    return nil;
}

Object *prim_env(Object *args IGNORED) {
    Object *symbolList;

    for (symbolList = getRest(top_env); !isNil(symbolList); symbolList = getRest(symbolList)) {
        printf("%s ", getSymbolName(getFirst(getFirst(symbolList))));
    }
    return nil;
}

Object *prim_symbols(Object *args IGNORED) {
    Object *symbolList;

    for (symbolList = all_symbols; !isNil(symbolList); symbolList = getRest(symbolList)) {
        printf("%s ", getSymbolName(getFirst(symbolList)));
    }
    return nil;
}

Object *prim_to_int(Object *args) {
    return createInt(getInt(args));
}

Object *prim_not(Object *args) {
    return isNil(args) ? tee : nil;
}

Object *prim_gc(Object *args IGNORED) {
    markAll();
    sweep();
    malloc_compact();
    return nil;
}

Object *prim_debug(Object *args IGNORED) {
    Object *obj;

    for (obj = all_objects; obj != NULL; obj = (Object *) obj->next) {
        printObj(obj);
        putchar(' ');
    }
    putchar('\n');
    return nil;
}

Object *prim_read32(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    int32_t *ptr = (int32_t *) getInt(addr);
    return createInt(*ptr);
}

Object *prim_read16(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    int16_t *ptr = (int16_t *) getInt(addr);
    return createInt(*ptr);
}

Object *prim_read8(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    int8_t *ptr = (int8_t *) getInt(addr);
    return createInt(*ptr);
}

Object *prim_write32(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    Object *value = getFirst(getRest(args));
    int32_t *ptr = (int32_t *) getInt(addr);
    *ptr = getInt(value);
    return nil;
}

Object *prim_write16(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    Object *value = getFirst(getRest(args));
    int16_t *ptr = (int16_t *) getInt(addr);
    *ptr = (int16_t) getInt(value);
    return nil;
}

Object *prim_write8(Object *args) {
    if (isNil(args)) return nil;
    Object *addr = getFirst(args);
    Object *value = getFirst(getRest(args));
    int8_t *ptr = (int8_t *) getInt(addr);
    *ptr = (int8_t) getInt(value);
    return nil;
}

Object *prim_and(Object *args) {
    if (isNil(args)) return nil;
    Object *a = getFirst(args);
    Object *b = getFirst(getRest(args));
    return createInt(getInt(a) & getInt(b));
}

Object *prim_or(Object *args) {
    if (isNil(args)) return nil;
    Object *a = getFirst(args);
    Object *b = getFirst(getRest(args));
    return createInt(getInt(a) | getInt(b));
}

Object *prim_xor(Object *args) {
    if (isNil(args)) return nil;
    Object *a = getFirst(args);
    Object *b = getFirst(getRest(args));
    return createInt(getInt(a) ^ getInt(b));
}

Network net = NULL;

Network getNet() {
    int i, count;
    if (!net) {
        for (i = 0, count = motherboard_get_max_devices(); i < count; i++) {
            if (motherboard_get_devices()[i]->type == DEVICE_TYPE_NETWORK_CARD) {
                net = (Network) motherboard_get_devices()[i];
                break;
            }
        }
        if (!net) {
            printf("Error: Unable to locate network card\n");
            exit(-1);
        }
    }
    return net;
}

Object *prim_network(Object *args IGNORED) {
    Network net = getNet();
    //https://raw.githubusercontent.com/Magneticraft-Team/Magneticraft/1.12/src/main/resources/assets/magneticraft/blockstates/battery.json
    network_set_target_ip(net, "raw.githubusercontent.com");
    network_set_target_port(net, 443);
    network_signal(net, 3);
    network_set_input_pointer(net, 0);
    network_set_output_pointer(net, 0);

    const char *get = "GET /Magneticraft-Team/Magneticraft/1.12/src/main/resources/assets/magneticraft/blockstates/battery.json HTTP/1.1\r\n"
            "Host: raw.githubusercontent.com\r\n"
            "Connection: close\r\n"
            "\r\n";

    i8 *ptr = network_get_output_buffer(net);
    strcpy(ptr, get);
    network_set_output_pointer(net, strlen(get));

    while (network_get_output_pointer(net) && network_is_connection_open(net)) {
        motherboard_sleep(1);
    }

    while (!network_get_input_pointer(net) && network_is_connection_open(net)) {
        motherboard_sleep(1);
    }
    printf("%s (%d)\n", network_get_input_buffer(net), network_get_input_pointer(net));

    network_signal(net, NETWORK_SIGNAL_CLOSE_TCP_CONNECTION);
    return nil;
}

// IO

static File *currentFolder = NULL;

inline DiskDrive getDisk() {
    DiskDrive drive = motherboard_get_floppy_drive();
    if (currentFolder == NULL) {
        currentFolder = (int) file_get_root(drive);
    }
    return drive;
}

inline File *getCurrentFolder() {
    return currentFolder;
}

inline int hasDisk() {
    return disk_drive_has_disk(motherboard_get_floppy_drive());
}

Object *prim_ls(Object *args IGNORED) {
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();
    File *folder = getCurrentFolder();
    int entryCount = folder->size / sizeof(DirectoryEntry);
    putchar('\n');
    DirectoryEntry entry;
    for (int i = 0; i < entryCount; ++i) {
        file_read(drive, folder, byteArrayOf(&entry, sizeof(DirectoryEntry)), sizeof(DirectoryEntry) * i);
        printf("%d - %s\n", i, entry.name);
    }
    return nil;
}

Object *prim_cd(Object *args) {
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();

    if (isNil(args)) return nil;
    Object *a = getElem(args, 0);
    if (a->type != SYM) {
        printf("");
        return nil;
    }

    const char *name = a->string;
    if (name == NULL) return nil;

    File *file = file_open(drive, getCurrentFolder(), name);
    if (file == NULL) {
        printf("Unable to find %s\n", name);
        return nil;
    }
    if (file->type != FILE_TYPE_DIRECTORY) {
        printf("Not a directory\n");
        return nil;
    }
    currentFolder = file;
    return nil;
}

Object *prim_mkdir(Object *args) {
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();

    if (isNil(args)) return nil;
    Object *a = getElem(args, 0);
    if (a->type != SYM) {
        printf("Invalid type: %s, expected symbol\n", objTypeNames[a->type]);
        return nil;
    }

    const char *name = a->string;
    if (name == NULL) return nil;

    file_close(drive, file_create(drive, getCurrentFolder(), name, FILE_TYPE_DIRECTORY));
}

Object *prim_mkfile(Object *args) {
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();

    if (isNil(args)) return nil;
    Object *a = getElem(args, 0);
    if (a->type != SYM) {
        printf("Invalid type: %s, expected symbol\n", objTypeNames[a->type]);
        return nil;
    }

    const char *name = a->string;
    if (name == NULL) return nil;

    file_close(drive, file_create(drive, getCurrentFolder(), name, FILE_TYPE_NORMAL));
}

Object *prim_delete(Object *args) {
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();

    if (isNil(args)) return nil;
    Object *a = getElem(args, 0);
    if (a->type != SYM) {
        printf("Invalid type: %s, expected symbol\n", objTypeNames[a->type]);
        return nil;
    }

    const char *name = a->string;
    if (name == NULL) return nil;

    File *file = file_open(drive, getCurrentFolder(), name);
    if (file == NULL) {
        printf("Unable to find file: %s\n", name);
        return nil;
    }
    file_delete(drive, getCurrentFolder(), file);
    return nil;
}

Object *prim_cat(Object *args){
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();

    if (isNil(args)) return nil;
    Object *a = getElem(args, 0);
    if (a->type != SYM) {
        printf("Invalid type: %s, expected symbol\n", objTypeNames[a->type]);
        return nil;
    }

    const char *name = a->string;
    if (name == NULL) return nil;

    File *file = file_open(drive, getCurrentFolder(), name);
    if (file == NULL) {
        printf("Unable to find file: %s\n", name);
        return nil;
    }

    Object *index = getElem(args, 1);
    int block = index->type != INT ? 0 : index->number;

    char buffer[1024];

    file_read(drive, file, byteArrayOf(&buffer, 1024), block * 1024);

    // print
    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int base = 10;

    putchar('\n');
    for (int i = 0; i < 16; ++i) {
        putchar(digits[i / base]);
        putchar(digits[i % base]);
        putchar(' ');
        for (int j = 0; j < 64; ++j) {
            char c = buffer[i * 64 + j];
            if (c == 0) {
                putchar('.');
            } else {
                putchar(c);
            }
        }
        putchar('\n');
    }
}

Object *prim_load(Object *args){
    if (!hasDisk()) {
        printf("No disk\n");
        return nil;
    }

    DiskDrive drive = getDisk();

    if (isNil(args)) return nil;
    Object *a = getElem(args, 0);
    if (a->type != SYM) {
        printf("Invalid type: %s, expected symbol\n", objTypeNames[a->type]);
        return nil;
    }

    const char *name = a->string;
    if (name == NULL) return nil;

    File *file = file_open(drive, getCurrentFolder(), name);
    if (file == NULL) {
        printf("Unable to find file: %s\n", name);
        return nil;
    }

    int line_num = getLineNumber();
    setInputFile(file);
    setLineNumber(1);
    Object *input, *output = nil;
    while (canReadMore()) {
        //read input
        input = readObj();

        if (setjmp(onError) == 0) {
            //eval
            output = eval(input, top_env);
            //print output
            if (!write_output_flag) {
                printObjFormatted(output);
                printf("\n");
            }
        }else{
            break;
        }
        write_output_flag = 0;
        if (output == NULL) break;
    }
    setInputFile(NULL);
    setLineNumber(line_num);
}