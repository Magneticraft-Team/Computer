//
// Created by cout970 on 19/09/17.
//

#include <debug.h>
#include <kprint.h>
#include <string.h>
#include <malloc.h>
#include "../include/lexer.h"
#include "../include/object.h"
#include "../include/functions.h"
#include "../include/exception.h"
#include "../include/token.h"

char *tk_names[] = {
        "IDENTIFIER",
        "NUMBER",
        "STRING",
        "LEFT_PAREN",
        "RIGHT_PAREN",
        "QUOTE",
        "QUASIQUOTE",
        "KEYWORD",
        "DOT",
        "COMMA",
        "EOF"
};

struct ParserState {
    Token tk;
    Boolean consumed;
} parser;

int indentation = 0;

void pr_init() {
    parser.tk.type = TK_EOF;
    parser.tk.name = "EOF";
    parser.consumed = TRUE;
}

struct ParserState *pr_save() {
    struct ParserState *state = malloc(sizeof(struct ParserState));
    memcpy(state, &parser, sizeof(struct ParserState));
    pr_init();
    return state;
}

void pr_recover(struct ParserState *oldState) {
    memcpy(&parser, oldState, sizeof(struct ParserState));
    free(oldState);
}

static Object *readObject();

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

static void nextToken() {
    if (parser.consumed) {
        lx_nextToken(&parser.tk);
        parser.consumed = FALSE;
    }
}

void consumeToken() {
    parser.consumed = TRUE;
}

void expectToken(TokenType type) {
    consumeToken();
    if (parser.tk.type != type) {
        kprint("Expected %s (%d), but found %s (%d)\n", tk_names[type], type, tk_names[parser.tk.type], parser.tk.type);
        THROW(EXCEPTION_INVALID_TOKEN);
    }
}

static Boolean matchToken(TokenType type) {
    if (parser.tk.type == type) {
        consumeToken();
        return TRUE;
    }
    return FALSE;
}

static Boolean matchList() {
    return matchToken(TK_LEFT_PAREN);
}

static Object *readList() {
    indentation++;
    nextToken();

    if (matchToken(TK_RIGHT_PAREN)) {
        indentation--;
        return obj_nil;
    }

    Boolean dot = matchToken(TK_DOT);

    Object *obj = readObject();
    if (obj == NULL) {
        indentation--;
        return NULL;
    }

    if (dot) {
        nextToken();
        expectToken(TK_RIGHT_PAREN);
        indentation--;
        return obj;
    } else {
        indentation--;
        Object *list = createCons(obj, readList());
        return list;
    }
}

static Boolean matchSymbol() {
    return matchToken(TK_IDENTIFIER);
}

static Object *readSymbol() {
    return createSymbol(parser.tk.name);
}

static Boolean matchKeyword() {
    return matchToken(TK_KEYWORD);
}

static Object *readKeyword() {
    return createKeyword(parser.tk.name);
}

static Boolean matchNumber() {
    return matchToken(TK_NUMBER);
}

static Object *readNumber() {
    return createNumber((int) strtol(parser.tk.name, &(char *) {0}, 10));
}

static Boolean matchString() {
    return matchToken(TK_STRING);
}

static Object *readString() {
    return createString(parser.tk.name);
}

static Boolean matchQuote() {
    return matchToken(TK_QUOTE);
}

static Object *readQuotedObject() {
    nextToken();

    Object *obj = readObject();
    if (obj == NULL) return NULL;

    return createCons(obj_quote, createCons(obj, obj_nil));
}

static Boolean matchMacro() {
    return matchToken(TK_QUASIQUOTE);
}

static Object *readMacro() {
    nextToken();

    Object *obj = readObject();
    if (obj == NULL) return NULL;

    return createCons(obj_quasiquote, createCons(obj, obj_nil));
}

static Boolean matchUnquoted() {
    return matchToken(TK_COMMA);
}

static Object *readUnquoted() {
    nextToken();

    Object *obj = readObject();
    if (obj == NULL) return NULL;

    return createCons(obj_unquote, createCons(obj, obj_nil));
}

static Object *readObject() {
    nextToken();

    if (matchList()) {
        return readList();

    } else if (matchKeyword()) {
        return readKeyword();

    } else if (matchSymbol()) {
        return readSymbol();

    } else if (matchNumber()) {
        return readNumber();

    } else if (matchString()) {
        return readString();

    } else if (matchQuote()) {
        return readQuotedObject();

    } else if (matchMacro()) {
        return readMacro();

    } else if (matchUnquoted()) {
        return readUnquoted();

    } else if (parser.tk.type == TK_EOF) {
        return NULL;

    } else {
        consumeToken();
        kprint("Unexpected Token: %s (%d)\n", tk_names[parser.tk.type], parser.tk.type);
        THROW(EXCEPTION_INVALID_TOKEN);
        return NULL;
    }
}

Object *pr_parse() {
    indentation = 0;
    return readObject();
}
