#ifndef COMPILER_PROGRAM_H
#define COMPILER_PROGRAM_H
#include <string>
#include <iostream>
#include <memory>
#include <cstring>
#include <map>
#include <vector>
using namespace std;


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
    tok_float =-9,
    tok_writef =-10,
    tok_readf=-11,
    tok_for=-12,
    tok_switch=-13,
    tok_case=-14,
    tok_continue=-15,
    tok_break=-16,

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
    tok_mod = -69, //%
    tok_xor = -70, // XOR
    tok_odd = -71, // ODD
    tok_selfadd = -72, // ++
    tok_selfmin = -73, // --
    tok_lbracket = -74, // [
    tok_rbracket = -75, // ]
    tok_colon = -76, // :


    //other
    tok_identifier = -100, // start with alpha then alpha or num
    tok_intnum= -101, //  int NUM
    tok_true = -102,
    tok_false = -103,
    tok_floatnum = -104,
};

enum type{
    type_int,
    type_bool,
    type_float,
    type_iarr,
    type_farr,
};
#define typeCount 5


struct tablestruct
{
    std::string name;
    int type;
    double val;
    int size; //需要分配的数据区空间,留给方程使用
    int index; //在本namespace分配的堆栈中下标
    int name_space; //所处的namespace
};


#define maxnamespace 200 //最多有200个block

#define fillbackCount 3 //break continue return
#define fb_break 0
#define fb_continue 1
#define fb_return 2

void enter(int name_space,int type,std::string name);
void new_namespace(int father_namespace,int my_namespace); // 当进入一个block的时候调用,给这个block一个新的namespace,并根据block的参数设置上层namespace
struct tablestruct& getTablestructById(int name_space,std::string name); // 按name和namespace查找,如果找不到,报错
void fillback(int L,int fun,int lev);


int gettok();
int getNextToken();

#define MAXLINE 81

void getch();

void error(int n);
int test(std::string Vn_name);
// parser declareation
void program();
void block(int upper_namespace);

void decls(int lev);
void decl(int lev);

void stmts(int lev);
void selfaddmin_stmt(int lev);
void assign_stmt(int lev);
void if_stmt(int lev);
void while_stmt(int lev);
void for_stmt(int lev);
void switchcase_stmt(int lev);
void write_stmt(int lev);
void read_stmt(int lev);
void writef_stmt(int lev);
void readf_stmt(int lev);
void continue_stmt(int lev);
void break_stmt(int lev);
void stmt(int lev);

void intexpr(int lev); // expression with type int
void intterm(int lev);
void intfactor(int lev);

void floatexpr(int lev); // expression with type float
void floatterm(int lev);
void floatfactor(int lev);

void boolexpr(int lev); //expression with type bool
void boolexpr_(int lev);
void boolterm(int lev);
void boolterm_(int lev);
void boolfactor(int lev);
void rel(int lev);
void isOdd(int lev);
void frel(int lev);
void nega_rel(int lev);


#define Vn_count 33


void error();

#define cxmax 200 //最多虚拟机代码数
//虚拟机代码指令
enum fct{
    lit, opr, lod,
    sto, cal, ini,
    jmp, jpc, ssp,
    lsp, old, ost,
};
#define fctnum 12
/*
 * lit 把一个常数置入栈顶
 * lod 把一个变量置入栈顶         lod namespace index
 * opr 算数和关系运算指令
 * sto 从栈顶把书置入一个变量单元内 sto namespace index
 * cal 调用一个过程
 * ini 预留数据存储位置
 * jmp 无条件跳转
 * jpc 有条件跳转
*/

struct instruction
{
    enum fct f; // 虚拟机代码指令
    int l; //namespace
    double a; //根据f的不同而不同
};

void gen(enum fct x,int y,double z);
void exeAll();
int exeOne();

int compileCX(std::string file_in_name);
void printTable();
void listall();
void printStack();

#define amax 2048 //地址上界
#define stacksize 500 // 运行时数据栈元素最多500

static int waitInput=0;
static std::string inputtext="";


#endif // COMPILER_PROGRAM_H
