//
// Created by cout970 on 26/02/18.
//

#include <types.h>
#include <debug.h>
#include <monitor.h>
#include <motherboard.h>
#include <fs/filesystem.h>
#include <util/cmd.h>
#include <robot.h>
#include <util/input.h>
#include <base.h>
#include "../include/functions.h"
#include "../include/interpreter.h"
#include "../include/exception.h"
#include "../include/gc.h"

#define CHESK_FS()  if(!hasDisk()){ kdebug("No disk\n"); return obj_nil; }                      \
                    if(fs_getDevice() == -1){ kdebug("Disk not formatted\n"); return obj_nil; } \
                    else { fs_init(motherboard_get_floppy_drive()); }

Object *obj_nil = NULL;
Object *obj_env = NULL;
Object *obj_t = NULL;
Object *obj_quote = NULL;
Object *obj_progn = NULL;
Object *obj_quasiquote = NULL;
Object *obj_unquote = NULL;
Object *obj_folder = NULL;
Object *obj_robot = NULL;
Object *obj_help = NULL;

static Object *registerVariable(const char *name, Object *value) {
    Object *sym = createSymbol(name);
    extendEnv(sym, value);
    return sym;
}

static Object *registerSymbol(const char *name) {
    Object *sym = createSymbol(name);
    return extendEnv(sym, sym);
}

static Object *registerFunc(const char *name, NativeFunc func) {
    Object *sym = createSymbol(name);
    extendEnv(sym, createNativeFunc(func));
    return sym;
}

static INodeRef getFolder(Object *env) {
    Object *value = lookupSymbol(obj_folder, env);
    return getNumber(value);
}

static int hasDisk(void) {
    return disk_drive_has_disk(motherboard_get_floppy_drive());
}

static int find_robot() {
#ifdef DEBUG_ENV
    return 0;
#else
    const struct device_header **device_table = motherboard_get_devices();

    for (int i = 0; i < MOTHERBOARD_MAX_DEVICES; ++i) {
        if (device_table[i]->online && device_table[i]->type == DEVICE_TYPE_MINING_ROBOT) {
            return (int) device_table[i];
        }
    }
    return 0;
#endif
}

Object *f_if(Object *lst, Object *env) {
    //(if (exp) ifTrue ifFalse)

    Object *ifExp = getFirst(lst);

    if (eval(ifExp, env) != obj_nil) {
        return eval(getElem(lst, 1), env);
    } else {
        return eval(getElem(lst, 2), env);
    }
}

Object *f_lambda(Object *lst, Object *env) {
    // (lambda (args...) code)
    return createFun(getFirst(lst), getRest(lst), env);
}

Object *f_quote(Object *lst, Object *env IGNORED) {
    // '(listElements...)
    return getFirst(lst);
}

Object *f_quasiquote(Object *lst, Object *env IGNORED) {
    // `(0 1 2) -> '(0 1 2)
    // `(1 ,(+ 1 2) 4) -> '(1 3 4)
    // (quasiquote (0 (unquote (+ 1 2)) 4))
    Object *template = getFirst(lst);
    return createCons(obj_quote, expandMacro(template, env));
}

Object *f_unquoted(Object *lst, Object *env IGNORED) {
    // ,(+ 1 2)
    return getFirst(lst);
}

Object *f_eval(Object *lst, Object *env) {
    // (eval '(println "Hello world"))
    return eval(eval(getFirst(lst), env), env);
}

void main();

Object *f_debug(Object *lst, Object *env IGNORED) {
    // (debug function)
    Object *obj = eval(getFirst(lst), env);

    debugObj(obj);

    if (obj->type == FUNC) {
        return createCons(obj->args, obj->code);
    } else if (obj->type == MACRO) {
        return createCons(obj->macro_args, obj->macro_code);
    }

    return obj_nil;
}

Object *f_macro_expand(Object *lst, Object *env IGNORED) {
    Object *args = evalList(lst, env);
    Object *macro = getFirst(args);
    Object *newEnv = addArgsToEnv(macro->macro_env, macro->macro_args, evalList(getRest(args), env));

    return expandMacro(macro->code, newEnv);
}

Object *f_define(Object *lst, Object *env) {
    // (define symbol value)
    Object *sym = getSymbol(getElem(lst, 0));

    if (assoc(sym, env) != obj_nil) {
        kdebug("Symbol %s already defined\n", sym->name);
        THROW(EXCEPTION_SYMBOL_ALREADY_DEFINED);
        return obj_nil;
    }
    return extendEnv(sym, eval(getElem(lst, 1), env));
}

Object *f_set(Object *lst, Object *env) {
    // (set! symbol newValue)
    Object *sym = getSymbol(getElem(lst, 0));
    Object *pair = assoc(sym, env);
    Object *newVal = eval(getElem(lst, 1), env);

    if (pair == obj_nil) {
        if (newVal != obj_nil) {
            extendEnv(sym, newVal);
        }
        return newVal;

    } else {
        if (symbolEquals(sym, obj_t) || symbolEquals(sym, obj_nil)) {
            kdebug("Attempt to override the value of %s\n", sym->name);
            THROW(EXCEPTION_ILLEGAL_ACTION);
            return obj_nil;
        }

        pair->cdr = newVal;
        return newVal;

    }
}

Object *f_progn(Object *lst, Object *env) {
    // (progn
    //          line1
    //          line2
    //          ...)
    Object *aux = obj_nil;

    while (lst != obj_nil) {
        aux = eval(getFirst(lst), env);
        lst = getRest(lst);
    }

    return aux;
}

Object *f_defun(Object *lst, Object *env) {
    // (defun symbol (args...) code)
    Object *fn = createFun(getElem(lst, 1), getRest(getRest(lst)), env);
    Object *name = getSymbol(getElem(lst, 0));

    extendEnv(name, fn);
    return fn;
}

Object *f_defmacro(Object *lst, Object *env) {
    // (defmacro e () (+ 3 5))
    // (e) -> 8
    // (defmacro e2 () '(+ 3 5))
    // (e2) -> (+ 3 5)
    // (defmacro e3 (x) `(+ 3 ,x))
    // (e3 5) -> (+ 3 5)
    // (defmacro e4 (x) `(+ 3 ,(+ 1 x))
    // (e4 7) -> (+ 3 8)
    Object *name = getSymbol(getElem(lst, 0));
    Object *fn = createMacro(getElem(lst, 1), getElem(lst, 2), env);

    return extendEnv(name, fn);
}

Object *f_car(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getFirst(getElem(args, 0));
}

Object *f_cdr(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getRest(getElem(args, 0));
}

Object *f_cadr(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getFirst(getRest(getElem(args, 0)));
}

Object *f_first(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getElem(getElem(args, 0), 0);
}

Object *f_second(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getElem(getElem(args, 0), 1);
}

Object *f_third(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getElem(getElem(args, 0), 2);
}

Object *f_fourth(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getElem(getElem(args, 0), 3);
}

Object *f_fifth(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return getElem(getElem(args, 0), 4);
}

Object *f_atom(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0)->type != CONS) ? obj_t : obj_nil;
}

Object *f_nth(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *index = getElem(args, 0);
    Object *list = getElem(args, 1);
    return getElem(list, getNumber(index));
}

Object *f_cons(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *first = getElem(args, 0);
    Object *second = getElem(args, 1);
    return createCons(first, second);
}

Object *f_length(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *list = getFirst(args);
    int i;
    for (i = 0; list != obj_nil; ++i) {
        list = getRest(list);
    }
    return createNumber(i);
}

Object *f_reverse(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *list = getFirst(args);
    return reverse(list);
}

Object *f_append(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *arg = args, *list;
    Object *acum = obj_nil;

    while (arg != obj_nil) {
        list = getFirst(arg);

        while (list != obj_nil) {
            acum = createCons(getFirst(list), acum);
            list = getRest(list);
        }

        arg = getRest(arg);
    }
    return reverse(acum);
}

Object *f_null(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getFirst(args) == obj_nil) ? obj_t : obj_nil;
}

Object *f_list(Object *lst, Object *env) {
    // (list 1 2 3 4) => (1 2 3 4)
    return evalList(lst, env);
}

Object *f_list_star(Object *lst, Object *env) {
    // (list* 1 2 3 4) => (1 2 3 . 4)
    Object *args = evalList(lst, env);
    Object *lastCons, *prevValue, *iter = args;

    while (1) {
        prevValue = getFirst(iter);
        lastCons = getRest(iter);

        if (getRest(lastCons) == obj_nil) {
            if (lastCons->type == CONS) {
                iter->cdr = getFirst(lastCons);
                break;
            } else {
                // list only has 1 item
                return prevValue;
            }
        }
        iter = getRest(iter);
    }

    return args;
}

Object *f_eq(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (objEquals(getElem(args, 0), getElem(args, 1))) ? obj_t : obj_nil;
}

Object *f_plus(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return createNumber(a + b);
}

Object *f_minus(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return createNumber(a - b);
}

Object *f_times(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return createNumber(a * b);
}

Object *f_div(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    if (b == 0) {
        kdebug("Divided by zero\n");
        THROW(EXCEPTION_DIVIDED_BY_ZERO);
    }
    return createNumber(a / b);
}

Object *f_rem(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    if (b == 0) {
        kdebug("Divided by zero\n");
        THROW(EXCEPTION_DIVIDED_BY_ZERO);
    }
    return createNumber(a % b);
}

Object *f_equals(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a == b ? obj_t : obj_nil;
}

Object *f_greater(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a > b ? obj_t : obj_nil;
}

Object *f_less(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a < b ? obj_t : obj_nil;
}

Object *f_greater_equals(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a >= b ? obj_t : obj_nil;
}

Object *f_less_equals(Object *lst, Object *env) {
    Object *args = evalList(lst, env);

    int a = getNumber(getElem(args, 0));
    int b = getNumber(getElem(args, 1));
    return a <= b ? obj_t : obj_nil;
}

Object *f_and(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return ((getElem(args, 0) != obj_nil) && (getElem(args, 1) != obj_nil)) ? obj_t : obj_nil;
}

Object *f_or(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return ((getElem(args, 0) != obj_nil) || (getElem(args, 1) != obj_nil)) ? obj_t : obj_nil;
}

Object *f_not(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0) == obj_nil) ? obj_t : obj_nil;
}

Object *f_numberp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0)->type == NUMBER) ? obj_t : obj_nil;
}

Object *f_symbolp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0)->type == SYMBOL) ? obj_t : obj_nil;
}

Object *f_stringp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getElem(args, 0)->type == STRING) ? obj_t : obj_nil;
}

Object *f_evenp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getNumber(getElem(args, 0)) % 2 == 0) ? obj_t : obj_nil;
}

Object *f_oddp(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    return (getNumber(getElem(args, 0)) % 2 != 0) ? obj_t : obj_nil;
}

Object *f_dotimes(Object *lst, Object *env) {
    // (dotimes (i 10) (println i))
    Object *iterator = getElem(getFirst(lst), 0);
    Object *limit = getElem(getFirst(lst), 1);

    int times = getNumber(eval(limit, env));
    Object *code = getRest(lst), *tmpEnv;

    getSymbol(iterator);

    for (int i = 0; i < times; ++i) {
        tmpEnv = createCons(createCons(iterator, createNumber(i)), env);
        evalList(code, tmpEnv);
    }
    return obj_nil;
}

Object *f_dolist(Object *lst, Object *env) {
    // (dolist (x '(1 5 10 55)) (println x))
    Object *iterator = getElem(getFirst(lst), 0);
    Object *list = eval(getElem(getFirst(lst), 1), env);
    Object *code = getRest(lst), *tmpEnv;

    getSymbol(iterator);

    while (list != obj_nil) {
        tmpEnv = createCons(createCons(iterator, getFirst(list)), env);
        evalList(code, tmpEnv);
        list = getRest(list);
    }

    return obj_nil;
}

static Object *f_map_aux(Object *code, Object *list, Object *env) {
    if (list == obj_nil) return obj_nil;
    Object *newValue = callFunction(code, createCons(getFirst(list), obj_nil), env);

    return createCons(newValue, f_map_aux(code, getRest(list), env));
}

Object *f_map(Object *lst, Object *env) {
    // (map println '(1 2 5 8)) -> (10 20 50 80)
    Object *args = evalList(lst, env);
    Object *code = getElem(args, 0);
    Object *list = getElem(args, 1);

    return f_map_aux(code, list, env);
}

static Object *f_filter_aux(Object *code, Object *list, Object *env) {
    if (list == obj_nil) return obj_nil;
    Object *cond = callFunction(code, createCons(getFirst(list), obj_nil), env);

    if (cond != obj_nil) {
        return createCons(getFirst(list), f_filter_aux(code, getRest(list), env));
    } else {
        return f_filter_aux(code, getRest(list), env);
    }
}

Object *f_filter(Object *lst, Object *env) {
    // (filter evenp '(1 2 3 4)) -> (2 4)
    Object *args = evalList(lst, env);
    Object *code = getElem(args, 0);
    Object *list = getElem(args, 1);

    return f_filter_aux(code, list, env);
}

Object *f_apply(Object *lst, Object *env) {
    // (apply '+ '(1 2)) -> 3
    Object *func = eval(getFirst(lst), env);
    Object *funcArgs = eval(getElem(lst, 1), env);

    if (func->type == SYMBOL) {
        func = lookupSymbol(func, env);
    }

    return callFunction(func, funcArgs, env);
}

Object *f_funcall(Object *lst, Object *env) {
    // (funcall '+ 1 2) =>  3
    Object *func = eval(getFirst(lst), env);
    Object *funcArgs = getRest(lst);

    if (func->type == SYMBOL) {
        func = lookupSymbol(func, env);
    }

    return callFunction(func, funcArgs, env);
}

Object *f_pipe(Object *lst, Object *env) {
    // (pipe 1 (+ 1) (* 2)) =>  4
    Object *aux;

    aux = eval(getFirst(lst), env);
    lst = getRest(lst);

    while (lst != obj_nil) {
        Object *call = getFirst(getFirst(lst));
        Object *args = createCons(aux, getRest(getFirst(lst)));

        aux = callFunction(eval(call, env), evalList(args, env), env);
        lst = getRest(lst);
    }

    return aux;
}

Object *f_println(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *value = getFirst(args);

    if (value->type == STRING) {
        kdebug("%s\n", getString(value));
    } else {
        printObj(value);
        kdebug("\n");
    }
    return obj_nil;
}

Object *f_print(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *value = getFirst(args);

    if (value->type == STRING) {
        kdebug("%s", getString(value));
    } else {
        printObj(value);
    }
    return obj_nil;
}

Object *f_clear(Object *lst IGNORED, Object *env IGNORED) {
    monitor_clear(motherboard_get_monitor());
    return obj_nil;
}

Object *f_sleep(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int ticks = getNumber(getFirst(args)) & 0xFF;
    motherboard_sleep((Byte) ticks);
    return obj_nil;
}

Object *f_format(Object *lst IGNORED, Object *env IGNORED) {
    if (!hasDisk()) {
        kdebug("No disk!\n");
        return obj_nil;
    }
    fs_init(motherboard_get_floppy_drive());
    fs_format();
    return obj_nil;
}


Object *f_ls(Object *lst IGNORED, Object *env) {
    CHESK_FS();
    cmd_ls(getFolder(env));
    return obj_nil;
}

Object *f_cd(Object *lst, Object *env) {
    CHESK_FS();
    Object *args = evalList(lst, env);
    String *name = getString(getFirst(args));

    Object *pair = assoc(obj_folder, env);
    INodeRef folder = getNumber(getFirst(pair));

    pair->cdr = createNumber(cmd_cd(folder, name));
    return obj_nil;
}

Object *f_rm(Object *lst, Object *env) {
    CHESK_FS();
    Object *args = evalList(lst, env);
    String *name = getString(getFirst(args));

    cmd_rm(getFolder(env), name);
    return obj_nil;
}

Object *f_mkdir(Object *lst, Object *env) {
    CHESK_FS();
    Object *args = evalList(lst, env);
    String *name = getString(getFirst(args));

    cmd_mkdir(getFolder(env), name);
    return obj_nil;
}

Object *f_mkfile(Object *lst, Object *env) {
    CHESK_FS();
    Object *args = evalList(lst, env);
    String *name = getString(getFirst(args));

    cmd_mkfile(getFolder(env), name);
    return obj_nil;
}

Object *f_load(Object *lst, Object *env) {
    CHESK_FS();
    Object *args = evalList(lst, env);
    String *name = getString(getFirst(args));

    INodeRef child = fs_findFile(getFolder(env), name);
    if (child == FS_NULL_INODE_REF) {
        kdebug("Error %s not found\n", name);
        return FALSE;
    }

    FD fd = file_open_inode(child, FILE_OPEN_READ_ONLY);
    struct ReaderState *state = rd_setInput(fd);
    struct ParserState *pr_state = pr_save();

    in_run();

    rd_recover(state);
    pr_recover(pr_state);
    file_close(fd);

    return obj_nil;
}

Object *f_forget(Object *lst IGNORED, Object *thisEnv) {
    Object *pair, *env = obj_env;
    while (env != obj_nil) {
        pair = getFirst(env);
        if (symbolEquals(getFirst(pair), obj_help)) {
            break;
        }
        env = getRest(env);
    }

    obj_env->cdr = env;
    gc(obj_env);

    return obj_nil;
}

Object *f_member(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *value = getElem(args, 0);
    Object *list = getElem(args, 1);

    for (Object *pair = list; pair != obj_nil; pair = getRest(pair)) {
        if (objEquals(getFirst(pair), value)) {
            return obj_t;
        }
    }

    return obj_nil;
}

Object *f_read(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    Object *limitObj = getElem(args, 0);
    int limit = 80;

    if (limitObj->type == NUMBER) {
        limit = getNumber(limitObj);
    }

    char *buffer = malloc((UInt) limit);

    readString(buffer, limit);

    Object *str = createString(buffer);

    free(buffer);

    return str;
}

//Object *f_write_to_file(Object *lst, Object *env) {
//    char line[78];
//    int index = 0;
//
//    Object *args = evalList(lst, env);
//    String *name = getString(getFirst(args));
//    INodeRef folder = getFolder(env);
//
//    INodeRef file = fs_findFile(folder, name);
//    if (file == FS_NULL_INODE_REF) {
//        file = fs_create(folder, name, FS_FLAG_FILE);
//        if (file == FS_NULL_INODE_REF) {
//            kdebug("Error unable to create '%s'\n", name);
//            return obj_nil;
//        }
//    }
//
//    fs_truncate(file, 0);
//    kdebug("Type 'EOF' to exit\n");
//    while (1) {
//        kdebug(">>");
//        readString(line, 78);
//        if (strcmp(line, "EOF\n") == 0) return obj_nil;
//        fs_write(file, line, index, (Int) strlen(line));
//        index += strlen(line);
//    }
//}

Object *f_str_to_hex(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    String *str = getString(getElem(args, 0));

    long int value = strtol(str, &(char *) {0}, 16);

    return createNumber((int) value);
}

Object *f_str_to_dec(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    String *str = getString(getElem(args, 0));

    long int value = strtol(str, &(char *) {0}, 10);

    return createNumber((int) value);
}

Object *f_hex_to_str(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    unsigned int num = (unsigned int) getNumber(getElem(args, 0));
    char buff[9];
    int i;

    for (i = 0; num; ++i) {
        int aux = num % 16;
        num = num / 16;
        buff[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[aux];
    }
    int size = i - 1;
    for (int j = 0; j < i / 2; ++j) {
        char aux = buff[j];
        buff[j] = buff[size - j];
        buff[size - j] = aux;
    }
    buff[i] = '\0';

    return createString(buff);
}

Object *f_dec_to_str(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    unsigned int num = (unsigned int) getNumber(getElem(args, 0));
    char buff[11];
    int i;

    for (i = 0; num; ++i) {
        int aux = num % 10;
        num = num / 10;
        buff[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[aux];
    }

    int size = i - 1;
    for (int j = 0; j < i / 2; ++j) {
        char aux = buff[j];
        buff[j] = buff[size - j];
        buff[size - j] = aux;
    }
    buff[i] = '\0';

    return createString(buff);
}

Object *f_r32(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int addr = getNumber(getElem(args, 0));
    return createNumber(*((UInt *) addr));
}

Object *f_r16(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int addr = getNumber(getElem(args, 0));
    return createNumber(*((UShort *) addr));
}

Object *f_r8(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int addr = getNumber(getElem(args, 0));
    return createNumber(*((UByte *) addr));
}

Object *f_w32(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int addr = getNumber(getElem(args, 0));
    int value = getNumber(getElem(args, 1));
    *((UInt *) addr) = (UInt) value;
    return obj_nil;
}

Object *f_w16(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int addr = getNumber(getElem(args, 0));
    int value = getNumber(getElem(args, 1));
    *((UShort *) addr) = (UShort) value;
    return obj_nil;
}

Object *f_w8(Object *lst, Object *env) {
    Object *args = evalList(lst, env);
    int addr = getNumber(getElem(args, 0));
    int value = getNumber(getElem(args, 1));
    *((UByte *) addr) = (UByte) value;
    return obj_nil;
}

MiningRobot *getMiningRobot(Object *env) {
    MiningRobot *robot = (MiningRobot *) getNumber(lookupSymbol(obj_robot, env));
    if (robot == NULL) {
        kdebug("Unable to access mining robot API\n");
        THROW(EXCEPTION_ILLEGAL_ACTION);
    }
    return robot;
}

static Object *robot_signal(int signal, Object *env) {
    MiningRobot *robot = getMiningRobot(env);

    int result = mining_robot_signal(robot, signal);
    if (result == ROBOT_NO_FAIL) {
        return obj_nil;
    } else {
        return createNumber(result);
    }
}

Object *f_mine(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_MINE_BLOCK, env);
}

Object *f_front(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_MOVE_FORWARD, env);
}

Object *f_back(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_MOVE_BACK, env);
}

Object *f_left(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_ROTATE_LEFT, env);
}

Object *f_right(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_ROTATE_RIGHT, env);
}

Object *f_up(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_ROTATE_UP, env);
}

Object *f_down(Object *lst IGNORED, Object *env) {
    return robot_signal(ROBOT_SIGNAL_ROTATE_DOWN, env);
}

Object *f_scan(Object *lst IGNORED, Object *env) {
    return mining_robot_scan(getMiningRobot(env)) ? obj_t : obj_nil;
}

Object *f_help(Object *lst IGNORED, Object *env) {
    kdebug("Avaliable commands: \n");
    for (Object *cons = env; cons != obj_nil; cons = getRest(cons)) {
        Object *pair = getFirst(cons);

        if (getFirst(pair) != obj_nil) {
            printObj(getFirst(pair));
            kdebug(" ");
        }
    }
    kdebug("\n");
    return obj_nil;
}

void fn_init() {
    obj_nil = createSymbol("nil");
    obj_env = createCons(createCons(obj_nil, obj_nil), obj_nil);
    obj_t = registerSymbol("t");
    obj_folder = registerVariable("folder", createNumber(fs_getRoot()));
    obj_robot = registerVariable("robot", createNumber(find_robot()));
    obj_quote = registerFunc("quote", f_quote);
    obj_progn = registerFunc("progn", f_progn);
    obj_quasiquote = registerFunc("quasiquote", f_quasiquote);
    obj_unquote = registerFunc("unquote", f_unquoted);

    registerFunc("eval", f_eval);
    registerFunc("debug", f_debug);
    registerFunc("macro-expand", f_macro_expand);
    registerFunc("if", f_if);
    registerFunc("lambda", f_lambda);
    registerFunc("define", f_define);
    registerFunc("set!", f_set);
    registerFunc("defun", f_defun);
    registerFunc("defmacro", f_defmacro);
    registerFunc("car", f_car);
    registerFunc("cdr", f_cdr);
    registerFunc("cadr", f_cadr);
    registerFunc("first", f_first);
    registerFunc("second", f_second);
    registerFunc("third", f_third);
    registerFunc("fourth", f_fourth);
    registerFunc("fifth", f_fifth);
    registerFunc("atom", f_atom);
    registerFunc("first", f_car);
    registerFunc("rest", f_cdr);
    registerFunc("nth", f_nth);
    registerFunc("cons", f_cons);
    registerFunc("length", f_length);
    registerFunc("append", f_append);
    registerFunc("reverse", f_reverse);
    registerFunc("null", f_null);
    registerFunc("list", f_list);
    registerFunc("list*", f_list_star);
    registerFunc("eq", f_eq);
    registerFunc("+", f_plus);
    registerFunc("-", f_minus);
    registerFunc("*", f_times);
    registerFunc("/", f_div);
    registerFunc("%", f_rem);
    registerFunc("=", f_equals);
    registerFunc(">", f_greater);
    registerFunc("<", f_less);
    registerFunc(">=", f_greater_equals);
    registerFunc("<=", f_less_equals);
    registerFunc("and", f_and);
    registerFunc("or", f_or);
    registerFunc("not", f_not);
    registerFunc("numberp", f_numberp);
    registerFunc("symbolp", f_symbolp);
    registerFunc("stringp", f_stringp);
    registerFunc("evenp", f_evenp);
    registerFunc("oddp", f_oddp);
    registerFunc("dotimes", f_dotimes);
    registerFunc("dolist", f_dolist);
    registerFunc("map", f_map);
    registerFunc("filter", f_filter);
    registerFunc("apply", f_apply);
    registerFunc("funcall", f_funcall);
    registerFunc("pipe", f_pipe);
    registerFunc("println", f_println);
    registerFunc("print", f_print);
    registerFunc("clear", f_clear);
    registerFunc("env", f_help);
    registerFunc("sleep", f_sleep);
    registerFunc("forget", f_forget);
    registerFunc("member", f_member);
    registerFunc("read", f_read);
//    registerFunc("write-to-file", f_write_to_file);

    registerFunc("str-to-dec", f_str_to_dec); // "255" -> 255
    registerFunc("str-to-hex", f_str_to_hex); // "FF" -> 255
    registerFunc("hex-to-str", f_hex_to_str); // 255 -> "FF"
    registerFunc("dec-to-str", f_dec_to_str); // 255 -> "255"

    registerFunc("r32", f_r32);
    registerFunc("r16", f_r16);
    registerFunc("r8", f_r8);

    registerFunc("w32", f_w32);
    registerFunc("w16", f_w16);
    registerFunc("w8", f_w8);

    registerFunc("mine", f_mine);
    registerFunc("up", f_up);
    registerFunc("down", f_down);
    registerFunc("left", f_left);
    registerFunc("right", f_right);
    registerFunc("back", f_back);
    registerFunc("front", f_front);
    registerFunc("scan", f_scan);

    registerFunc("format", f_format);
    registerFunc("ls", f_ls);
    registerFunc("cd", f_cd);
    registerFunc("rm", f_rm);
    registerFunc("mkdir", f_mkdir);
    registerFunc("mkfile", f_mkfile);
    registerFunc("load", f_load);

    // this must be the last entry
    obj_help = registerFunc("help", f_help);
}