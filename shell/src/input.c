//
// Created by cout970 on 11/02/18.
//

#include "../include/input.h"
#include <ctype.h>

Boolean parsetInt(String *str, Int *result) {
    unsigned long int acum = 0;
    int sign = 1;
    int pos = 0;
    if (str[0] == '-') {
        pos = 1;
        sign = -1;
    } else if (str[0] == '+') {
        pos = 1;
        sign = 1;
    }

    while (str[pos] != '\0') {
        int val = str[pos] - '0';
        if (val < 0 || val > 9) {
            return FALSE;
        }
        acum = acum * 10 + val;
        pos++;
    }

    *result = ((int) acum) * sign;
    return TRUE;
}

Int split(String *in, String *cmd, String *arg1, String *arg2, String *arg3) {
    int args = 0, index = 0, aux;

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        cmd[aux++] = in[index];
    }
    cmd[aux] = '\0';

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        arg1[aux++] = in[index];
        args = 1;
    }
    arg1[aux] = '\0';

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        arg2[aux++] = in[index];
        args = 2;
    }
    arg2[aux] = '\0';

    while (isspace(in[index])) index++;
    for (aux = 0; index < 80 && in[index] && !isspace(in[index]); ++index) {
        arg3[aux++] = in[index];
        args = 3;
    }
    arg3[aux] = '\0';
    return args;
}