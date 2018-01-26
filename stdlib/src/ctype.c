//
// Created by cout970 on 2016-11-01.
//

#include "../include/ctype.h"

int isalnum(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9');
}

int isalpha(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int iscntrl(int c) {
    return c == 0x7F || (c >= 0 && c <= 0x1F);
}

int isdigit(int c) {
    return '0' <= c && c <= '9';
}

int isgraph(int c) {
    return c > 32;
}

int islower(int c) {
    return 'a' <= c && c <= 'z';
}

int isprint(int c) {
    return c >= 32;
}

int ispunct(int c) {
    return c == '.' || c == ',' || c == ';' || c == ':' || c == '(' || c == ')' || c == '[' || c == ']'
           || c == '{' || c == '}' || c == '?' || c == '!' || c == '-' || c == '\'' || c == '\"'
           || c == '#' || c == '$' || c == '%' || c == '*' || c == '+' || c == '/' || c == '\\'
           || c == '<' || c == '>' || c == '=' || c == '@' || c == '^' || c == '_' || c == '~';
}

int isspace(int c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f';
}

int isupper(int c) {
    return 'A' <= c && c <= 'Z';
}

int isxdigit(int c) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

int tolower(int c) {
    if (isupper(c)) {
        return 'a' + c - 'A';
    }
    return c;
}

int toupper(int c) {
    if (islower(c)) {
        return 'A' + c - 'a';
    }
    return c;
}

int isascii(int c) {
    return c >= 0 && c <= 127;
}