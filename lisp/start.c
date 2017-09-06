/* Original implementation: https://github.com/kristianlm/small-lisp */

#include "lisp.h"


/*** Initialization ***/
void init() {
    nil = createSymbol("nil");

    all_symbols = createCons(nil, nil);
    top_env = createCons(createCons(nil, nil), nil);
    tee = getOrCreateSymbol("t");

    extend_top(tee, tee);

    //@formatter:off
    quote       = getOrCreateSymbol("quote");
    s_if        = getOrCreateSymbol("if");
    s_lambda    = getOrCreateSymbol("lambda");
    s_define    = getOrCreateSymbol("define");
    s_defun     = getOrCreateSymbol("defun");
    s_setb      = getOrCreateSymbol("set!");
    s_begin     = getOrCreateSymbol("begin");
    //@formatter:on

    extend_top(getOrCreateSymbol("+"), createPrimop(prim_sum));
    extend_top(getOrCreateSymbol("-"), createPrimop(prim_sub));
    extend_top(getOrCreateSymbol("*"), createPrimop(prim_prod));
    extend_top(getOrCreateSymbol("/"), createPrimop(prim_divide));
    extend_top(getOrCreateSymbol("%"), createPrimop(prim_mod));
    extend_top(getOrCreateSymbol("="), createPrimop(prim_num_eq));

    extend_top(getOrCreateSymbol(">"), createPrimop(prim_gt));
    extend_top(getOrCreateSymbol(">="), createPrimop(prim_ge));

    extend_top(getOrCreateSymbol("<"), createPrimop(prim_lt));
    extend_top(getOrCreateSymbol("<="), createPrimop(prim_le));

    extend_top(getOrCreateSymbol("cons"), createPrimop(prim_cons));
    extend_top(getOrCreateSymbol("car"), createPrimop(prim_car));
    extend_top(getOrCreateSymbol("cdr"), createPrimop(prim_cdr));

    extend_top(getOrCreateSymbol("print"), createPrimop(prim_print));
    extend_top(getOrCreateSymbol("env"), createPrimop(prim_env));
    extend_top(getOrCreateSymbol("symbols"), createPrimop(prim_symbols));
    extend_top(getOrCreateSymbol("toInt"), createPrimop(prim_to_int));
    extend_top(getOrCreateSymbol("not"), createPrimop(prim_not));

    //native interactions
    extend_top(getOrCreateSymbol("read32"), createPrimop(prim_read32));
    extend_top(getOrCreateSymbol("read16"), createPrimop(prim_read16));
    extend_top(getOrCreateSymbol("read8"), createPrimop(prim_read8));

    extend_top(getOrCreateSymbol("write32"), createPrimop(prim_write32));
    extend_top(getOrCreateSymbol("write16"), createPrimop(prim_write16));
    extend_top(getOrCreateSymbol("write8"), createPrimop(prim_write8));

    extend_top(getOrCreateSymbol("and"), createPrimop(prim_and));
    extend_top(getOrCreateSymbol("or"), createPrimop(prim_or));
    extend_top(getOrCreateSymbol("xor"), createPrimop(prim_xor));

    //alias
    extend_top(getOrCreateSymbol("cond"), s_if);
    extend_top(getOrCreateSymbol("true"), tee);
    extend_top(getOrCreateSymbol("false"), nil);
    extend_top(getOrCreateSymbol("rest"), getOrCreateSymbol("cdr"));
    extend_top(getOrCreateSymbol("first"), getOrCreateSymbol("car"));

    //memory
    extend_top(getOrCreateSymbol("free"), createPrimop(prim_free));
    extend_top(getOrCreateSymbol("gc"), createPrimop(prim_gc));

    // fs
    extend_top(getOrCreateSymbol("ls"), createPrimop(prim_ls));
    extend_top(getOrCreateSymbol("cd"), createPrimop(prim_cd));
    extend_top(getOrCreateSymbol("mkdir"), createPrimop(prim_mkdir));
    extend_top(getOrCreateSymbol("mkfile"), createPrimop(prim_mkfile));
    extend_top(getOrCreateSymbol("delete"), createPrimop(prim_delete));
    extend_top(getOrCreateSymbol("cat"), createPrimop(prim_cat));
    extend_top(getOrCreateSymbol("load"), createPrimop(prim_load));

//    BLOCK
//            FLUSH
//    LIST
//            LOAD
//    WIPE
//            PP
//    OPEN

    //extra
    extend_top(getOrCreateSymbol("debug"), createPrimop(prim_debug));
    extend_top(getOrCreateSymbol("clear"), createPrimop(prim_clear));
    extend_top(getOrCreateSymbol("net"), createPrimop(prim_network));
}

// needed to start execution at main
#include "../driver/api/boot.h"

/*** Main Driver ***/
void main(){
//    clear_screen();
    init();
    printf("Lips Interpreter 1.0\n");

    initTokenizer();
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
        }
        write_output_flag = 0;
        if (output == NULL) break;
    }
    printf("Exit");
}


/*
(defun map (fun list)
    (if list
        (cons
            (fun (car list))
            (map fun (cdr list))
         )
        nil
     )
 )

 */