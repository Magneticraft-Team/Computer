/* Original implementation: https://github.com/kristianlm/small-lisp */

#include "dependencies.h"
#include "object.h"
#include "globals.h"
#include "constructors.h"
#include "environment.h"
#include "print.h"
#include "tokenizer.h"
#include "eval.h"
#include "primitives.h"

Object *prim_network(Object *args IGNORED);

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

    //extra
    extend_top(getOrCreateSymbol("debug"), createPrimop(prim_debug));
    extend_top(getOrCreateSymbol("clear"), createPrimop(prim_clear));
    extend_top(getOrCreateSymbol("net"), createPrimop(prim_network));
}

/*** Main Driver ***/
int main() {
    clear_screen();
    init();
    printf("Lips Interpreter 1.0\n");

    initTokenizer();
    Object *input, *output = nil;
    for (;;) {
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
    return 0;
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