#ifndef COMPILER_PROGRAM_H
#define COMPILER_PROGRAM_H
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>
#include <memory>
#include <cstring>
#include <map>
#include <vector>



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
    double val;
    int size; //需要分配的数据区空间,留给方程使用
    int index; //在本namespace符号表中下标
    int name_space; //所处的namespace
};


#define maxnamespace 100 //最多有100个block

//变量值直接存在符号表内
//program namespace为0
//别的block 依次递增
//upperNamespaces表示 block的上层namespace
using namespace std;
struct tablestruct temp_tablestruct;
// 符号表

std::vector<struct tablestruct> tables[maxnamespace];
std::vector< std::vector<int> > upperNamespaces(maxnamespace,std::vector<int>(0)); //表示namespace的依赖 upperNamespaces[namespace] 中存namespace的上级命名空间



void enter(int name_space,int type,std::string name);
void new_namespace(int father_namespace,int my_namespace); // 当进入一个block的时候调用,给这个block一个新的namespace,并根据block的参数设置上层namespace
struct tablestruct& getTablestructById(int name_space,std::string name); // 按name和namespace查找,如果找不到,报错
int base(int l,int* s,int b);


int gettok();
int getNextToken();

#define MAXLINE 81

void getch();
int getNextChar();

void log_error();
// parser declareation
void program();
void block(int upper_namespace);

void decls(int lev);
void decl(int lev);

void stmts(int lev);
void assign_stmt(int lev);
int getTypeById(std::string id);
int getIndexById(std::string id);
void assign_stmt(int lev);
void if_stmt(int lev);
void while_stmt(int lev);
void write_stmt(int lev);
void read_stmt(int lev);
void stmt(int lev);

void intexpr(int lev); // expression with type int
void intterm(int lev);
void intfactor(int lev);

void boolexpr(int lev); //expression with type bool
void boolexpr_(int lev);
void boolterm(int lev);
void boolterm_(int lev);
void boolfactor(int lev);
void rel(int lev);

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
