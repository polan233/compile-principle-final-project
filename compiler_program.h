#ifndef COMPILER_PROGRAM_H
#define COMPILER_PROGRAM_H
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>
#include <memory>
#include <cstring>



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
    tok_true = -102,
    tok_false = -103,
};

enum type{
    type_uint,
    type_bool
};

#define txmax 100

struct tablestruct
{
    std::string name;
    int type;
    int intVal;
    bool boolVal;
    int level;
    int adr;
    int size;
};
struct tablestruct table[txmax]; // 符号表

void declare(int type,std::string id);

int getIdType(std::string id);



int gettok();
int getNextToken();

#define MAXLINE 81

void getch();
int getNextChar();

void log_error();
// parser declareation
void program();
void block();

void decls();
void decl();

void intid(); // id for int
void boolid(); //id for bool

void stmts();
void assign_stmt();
int getTypeById(std::string id);
void if_stmt();
void while_stmt();
void write_stmt();
void read_stmt();
void stmt();

void intexpr(); // expression with type int
void intterm();
void intfactor();

void boolexpr(); //expression with type bool
void boolexpr_();
void boolterm();
void boolterm_();
void boolfactor();
void rel();

void error();

#endif // COMPILER_PROGRAM_H
