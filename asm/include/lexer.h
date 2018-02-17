//
// Created by cout970 on 17/02/18.
//

#ifndef COMPUTER_LEXER_H
#define COMPUTER_LEXER_H

#define TOKEN_BUFFER_SIZE 128

extern int tokenLook;
extern char tokenBuffer[TOKEN_BUFFER_SIZE];
extern int tokenNumberBase;

int scanToken();

void readToken();

void need(int tk);

void expect(int tk);

#endif //COMPUTER_LEXER_H
