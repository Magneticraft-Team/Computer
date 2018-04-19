//
// Created by cout970 on 19/09/17.
//

#include <ctype.h>
#include <string.h>
#include <debug.h>
#include "../include/token.h"
#include "../include/reader.h"
#include "../include/exception.h"

#define LEXER_BUFFER_SIZE 128
#define EOF (-1)

struct {
    int character;
    int bufferIndex;
    char buffer[LEXER_BUFFER_SIZE];
} lexer;

void lx_init() {
    // ignore
}

static void append() {
    if (lexer.bufferIndex >= LEXER_BUFFER_SIZE) {
        THROW(EXCEPTION_TOKEN_BUFFER_OVERFLOW);
    }
    lexer.buffer[lexer.bufferIndex++] = (char) lexer.character;
}

static void nextChar() {
    lexer.character = rd_readChar();
}

static void fillToken(Token *tk, TokenType type, char *str) {
    tk->name = str;
    tk->type = type;
}

static void trimSpaces() {
    while (isspace(lexer.character)) {
        nextChar();
    }
    if (lexer.character == ';') {
        while (lexer.character != '\n' && lexer.character != -1) {
            nextChar();
        }
        trimSpaces();
    }
}

void processString(Token *tk) {
    nextChar();
    while (lexer.character != '"') {
        append();
        nextChar();
    }
    fillToken(tk, TK_STRING, lexer.buffer);
}

void processNumber(Token *tk) {
    while (isdigit(lexer.character)) {
        append();
        nextChar();
    }
    rd_unreadChar();
    fillToken(tk, TK_NUMBER, lexer.buffer);
}

#define DIGIT_CHAR_CASES() case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':case '9': case '0':

#define SYMBOL_CHAR_CASES() case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': \
                            case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': \
                            case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': \
                            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': \
                            case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': \
                            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case '-': \
                            case '_': case '!': case '?': case '+': case '=': case '[': case ']': case '{': case '}': \
                            case '|': case '<': case '>': case '/': case '*': case '&': case '^': case '%': case '$': \
                            case '@': case '\\':

void processSymbol(Token *tk) {
    while (TRUE) {
        // @formatter:off
        switch (lexer.character) {
            DIGIT_CHAR_CASES()
            SYMBOL_CHAR_CASES() {
                append();
                nextChar();
                break;
            }
            default: goto out;
        }
         // @formatter:on
    }
    out:
    rd_unreadChar();
    fillToken(tk, TK_IDENTIFIER, lexer.buffer);
}

void processKeyword(Token *tk) {
    // save the ':' at the start
    append();
    nextChar();

    while (TRUE) {
        // @formatter:off
        switch (lexer.character) {
            DIGIT_CHAR_CASES()
            SYMBOL_CHAR_CASES() {
                append();
                nextChar();
                break;
            }
            default: goto out;
        }
         // @formatter:on
    }
    out:
    rd_unreadChar();
    fillToken(tk, TK_KEYWORD, lexer.buffer);
}

void lx_nextToken(Token *tk) {
    lexer.bufferIndex = 0;
    memset(lexer.buffer, 0, 128);
    nextChar();
    trimSpaces();

    // @formatter:off
    switch (lexer.character) {
        case '(': fillToken(tk, TK_LEFT_PAREN, "("); break;
        case ')': fillToken(tk, TK_RIGHT_PAREN, ")"); break;
        case '\'': fillToken(tk, TK_QUOTE, "'"); break;
        case '`': fillToken(tk, TK_QUASIQUOTE, "`"); break;
        case '.': fillToken(tk, TK_DOT, "."); break;
        case ',': fillToken(tk, TK_COMMA, ","); break;
        case '"': processString(tk); break;
        case ':': processKeyword(tk); break;
        DIGIT_CHAR_CASES() processNumber(tk); break;
        SYMBOL_CHAR_CASES() processSymbol(tk); break;
        case -1: fillToken(tk, TK_EOF, "EOF"); break;
        default: {
            kdebug("Error unknown char: '%c' (%d)\n", lexer.character, lexer.character);
            THROW(EXCEPTION_INVALID_CHAR);
        }
    }
    // @formatter:on
}