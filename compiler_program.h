#ifndef COMPILER_PROGRAM_H
#define COMPILER_PROGRAM_H
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>



enum Token {
    tok_eof =-1,
    tok_illegel=-1000,

    //commands
    tok_int = -2,
    tok_bool = -3,
    tok_if = -4,
    tok_else =-5,
    tok_while = -6,
    tok_write = -7,
    tok_read = -8,

    //symbols
    tok_add = -50, // +
    tok_sub= -51, // -
    tok_mul=-52, // *
    tok_div=-53, // /
    tok_lss=-54, // <
    tok_leq=-55, // <=
    tok_gtr=-56, // >
    tok_geq=-57, // >=
    tok_eql=-58, // ==
    tok_neq=-59, // !=
    tok_assign= -60, // =
    tok_or = -61, // ||
    tok_and = -62, // &&
    tok_not = -63, // !
    tok_semicolon = -64, // ;
    tok_lparen = -65, // (
    tok_rparen = -66, // )
    tok_lbrace = -67, // {
    tok_rbrace = -68, // }


    //other
    tok_identifier = -100, // start with alpha then alpha or num
    tok_uintnum= -101, // unsigned int NUM
};



int gettok();
int getNextToken();

#define MAXLINE 200
void getch();
int getNextChar();


#endif // COMPILER_PROGRAM_H
