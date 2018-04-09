//
// Created by cout970 on 19/09/17.
//

#ifndef UNTITLED_TOKENTYPE_H
#define UNTITLED_TOKENTYPE_H

typedef enum {
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,
    TK_LEFT_PAREN,
    TK_RIGHT_PAREN,
    TK_QUOTE,
    TK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char* name;
} Token;

#endif //UNTITLED_TOKENTYPE_H
