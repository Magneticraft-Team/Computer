//
// Created by cout970 on 22/04/18.
//

#include <types.h>
#include <string.h>

static long int strtol_aux(const char *str, char **endptr, int base) {
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

Boolean strToInt(String *str, Int base, Int *result) {

    char *end = NULL;
    int res = (int) strtol_aux(str, &end, base);

    if (end == (str + strlen(str))) {
        *result = res;
        return TRUE;
    }
    return FALSE;
}