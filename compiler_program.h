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
    int level; //所处层
    int adr; //地址
    int size; //需要分配的数据区空间,留给方程使用
};
struct tablestruct table[txmax]; // 符号表

void enter(int type,int* ptx,int lev,int* pdx);
int base(int l,int* s,int b);

int getIdType(std::string id);



int gettok();
int getNextToken();

#define MAXLINE 81

void getch();
int getNextChar();

void log_error();
// parser declareation
void program();
void block(int lev,int* ptx);

void decls(int lev,int* ptx,int* pdx);
void decl(int lev,int* ptx,int* pdx);

void stmts(int lev,int* ptx);
void assign_stmt(int lev,int* ptx);
int getTypeById(std::string id);
int getIndexById(std::string id);
void assign_stmt(int lev,int* ptx);
void if_stmt(int lev,int* ptx);
void while_stmt(int lev,int* ptx);
void write_stmt(int lev,int* ptx);
void read_stmt(int lev,int* ptx);
void stmt(int lev,int* ptx);

void intexpr(int lev,int* ptx); // expression with type int
void intterm(int lev,int* ptx);
void intfactor(int lev,int* ptx);

void boolexpr(int lev,int* ptx); //expression with type bool
void boolexpr_(int lev,int* ptx);
void boolterm(int lev,int* ptx);
void boolterm_(int lev,int* ptx);
void boolfactor(int lev,int* ptx);
void rel(int lev,int* ptx);

void error();

#define cxmax 200 //最多虚拟机代码数
//虚拟机代码指令
enum fct{
    lit, opr, lod,
    sto, cal, ini,
    jmp, jpc,
};
#define fctnum 8
/*
 * lit 把一个常数置入栈顶
 * lod 把一个变量置入栈顶
 * opr 算数和关系运算指令
 * sto 从栈顶把书置入一个变量单元内
 * cal 调用一个过程
 * ini 预留数据存储位置
 * jmp 无条件跳转
 * jpc 有条件跳转
*/

struct instruction
{
    enum fct f; // 虚拟机代码指令
    int l; //引用层与声明层层次差
    int a; //根据f的不同而不同
};

void gen(enum fct x,int y,int z);

#define amax 2048 //地址上界
#define stacksize 500 // 运行时数据栈元素最多500

#endif // COMPILER_PROGRAM_H
