//
// Created by cout970 on 19/09/17.
//

#include <ctype.h>
#include <string.h>
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
    if(lexer.bufferIndex >= LEXER_BUFFER_SIZE){
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

void lx_nextToken(Token *tk) {
    lexer.bufferIndex = 0;
    memset(lexer.buffer, 0, 128);
    nextChar();
    trimSpaces();

    if (lexer.character == EOF) {

        fillToken(tk, TK_EOF, "EOF");
        return;
    } else if (lexer.character == '(') {

        fillToken(tk, TK_LEFT_PAREN, "(");
        return;
    } else if (lexer.character == ')') {

        fillToken(tk, TK_RIGHT_PAREN, ")");
        return;
    } else if (lexer.character == '\'') {

        fillToken(tk, TK_QUOTE, "'");
        return;
    } else if (isdigit(lexer.character)) {

        while (isdigit(lexer.character)) {
            append();
            nextChar();
        }
        rd_unreadChar();
        fillToken(tk, TK_NUMBER, lexer.buffer);
        return;
    } else if (lexer.character == '"') {

        nextChar();
        while (lexer.character != '"') {
            append();
            nextChar();
        }
        fillToken(tk, TK_STRING, lexer.buffer);
        return;
    } else {

        while (strchr("()'\"", lexer.character) == NULL && lexer.character != EOF && !isspace(lexer.character)) {
            append();
            nextChar();
        }
        rd_unreadChar();
        fillToken(tk, TK_IDENTIFIER, lexer.buffer);
        return;
    }
}