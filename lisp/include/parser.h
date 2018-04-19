//
// Created by cout970 on 19/09/17.
//

#ifndef UNTITLED_PARSER_H
#define UNTITLED_PARSER_H

#include "lexer.h"
#include "object.h"

struct ParserState;

extern int indentation;

void pr_init();

struct ParserState *pr_save();

void pr_recover(struct ParserState *oldState);

Object *pr_parse();

#endif //UNTITLED_PARSER_H
