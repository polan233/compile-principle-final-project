#include "compiler_program.h"
#include <map>
#include <vector>
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>
#include <memory>
#include <cstring>
#include <compiler.h>
using namespace std;

extern compiler* w;
extern QEventLoop loop;

std::string IdentifierStr;
double NumVal;
int CurTok;
char ch=' ';
int line_count=0;
int cc=0,ll=0;
int precc=1;
int preline=1;
char line[MAXLINE];
bool flag;// flag == true if file ends


int err=0;
int maxerr=1;

FILE* fin;
FILE* ferr;
FILE* fcode;
FILE* ftable;
FILE* fstack;
char filename[MAXLINE];

int cx=0;
struct instruction code[cxmax];
std::string codeName[fctnum]={"lit","opr","lod","sto","cal","int","jmp","jpc","ssp","lsp","old","ost"};

int namespacecount=-1;
int dxs[maxnamespace];

std::vector<std::vector<int>> fillbackList_break(maxnamespace);
std::vector<std::vector<int>> fillbackList_continue(maxnamespace);
std::vector<std::vector<int>> fillbackList_return(maxnamespace);
std::vector<int> fillbackList_exit(maxnamespace);
int nestbreak=-1;
int nestcontinue=-1;
int nestreturn=-1;

int PC=0;
int LR=0;
int SP=0; //空栈 SP指向下一个入栈口 栈顶元素为 SP-1

int base[maxnamespace]={0};
struct instruction curCode; //当前指令
double s[stacksize]={0}; //栈


//program namespace为0
//别的block 依次递增
//upperNamespaces表示 block的上层namespace
struct tablestruct temp_tablestruct;
struct parameter temp_parameter;
// 符号表
std::vector<struct tablestruct> tables[maxnamespace];
std::vector< std::vector<int> > upperNamespaces(maxnamespace,std::vector<int>(0)); //表示namespace的依赖 upperNamespaces[namespace] 中存namespace的上级命名空间
int cantFindName=0;
std::string typeName[typeCount]={"int","bool","float","iarr","farr","vfun","ifun","ffun"};

map<std::string,int> Vntab;
vector<vector<int>> firsts(Vn_count,std::vector<int>(0));

void _exeAll();
void _exeOne();

// here is lexer
/*
 * get a char and skip all the spaces
 * read a line from the input file, store into line, read another line
 * when it is all used
 * called by getNextChar
 */
void getch(){
    precc=cc;
    preline=line_count;
    if(cc==ll){
        if(EOF==fscanf(fin,"%c",&ch)){
            flag = true;
            return;
        }
        line[0] = ch;
        //fprintf(ferr,"%c",ch);
        ll=1;
        cc=0;
        ch = ' ';
        while(ch!=10){
            if(EOF==fscanf(fin,"%c",&ch)){
                line[ll]=0;
                break;
            }
            //printf("%c",LastChar);
            //fprintf(ferr,"%c",ch);
            line[ll]=ch;
            ll++;
        }
        line_count++;
    }
    ch=line[cc];
    cc++;
}


// my lexer
int gettok(){
    if(flag){ // the file ends
        return tok_eof;
    }

    //skip whitespaces and tab and huan hang
    while(isspace(ch)||ch==10||ch==9){
        getch();
        if(flag){ // the file ends
            return tok_eof;
        }
    }

    if(isalpha(ch)){ // id and commands
        IdentifierStr = ch;
        getch();
        while(isalnum(ch)){
            IdentifierStr += ch;
            getch();
        }

        if(IdentifierStr=="int"){
            return tok_int;
        }
        if(IdentifierStr=="bool")
            {return tok_bool;}
        if(IdentifierStr=="float"){
            return tok_float;
        }
        if(IdentifierStr=="if")
            {return tok_if;}
        if(IdentifierStr=="else")
            {return tok_else;}
        if(IdentifierStr=="while")
            {return tok_while;}
        if(IdentifierStr=="write")
            {return tok_write;}
        if(IdentifierStr=="read")
            {return tok_read;}
        if(IdentifierStr=="for")
            {return tok_for;}
        if(IdentifierStr=="writef")
            {return tok_writef;}
        if(IdentifierStr=="readf")
            {return tok_readf;}
        if(IdentifierStr=="true"){
            return tok_true;
        }
        if(IdentifierStr=="false"){
            return tok_false;
        }
        if(IdentifierStr=="XOR"){
            return tok_xor;
        }
        if(IdentifierStr=="ODD"){
            return tok_odd;
        }
        if(IdentifierStr=="switch"){
            return tok_switch;
        }
        if(IdentifierStr=="case"){
            return tok_case;
        }
        if(IdentifierStr=="break"){
            return tok_break;
        }
        if(IdentifierStr=="continue"){
            return tok_continue;
        }
        if(IdentifierStr=="exit"){
            return tok_exit;
        }
        if(IdentifierStr=="void"){
            return tok_void;
        }
        if(IdentifierStr=="function"){
            return tok_function;
        }
        if(IdentifierStr=="return"){
            return tok_return;
        }

        return tok_identifier;
    }

    if(isdigit(ch)){ // number
        int isfloat=0;
        std::string NumStr;
        do{//吸收整数部分
            NumStr += ch;
            getch();
        }while(isdigit(ch));
        if(ch=='.'){
            isfloat=1;
            do{//吸收小数部分
                NumStr += ch;
                getch();
            }while(isdigit(ch));
        }
        if(isfloat){
            NumVal = stod(NumStr);
            return tok_floatnum;
        }
        else{
            NumVal = stoi(NumStr);
            return tok_intnum;
        }
    }

    //  divide or comment
    if(ch == '/'){
        getch();
        if(ch=='*'){ // comment
            bool commentOver=false;
            while (!commentOver) {
                getch();
                if(ch=='*'){
                    getch();
                    if(ch=='/'){
                        commentOver=true;
                        break;
                    }
                }
            }
            getch();
            //skip what's in comment block and then
            // return a token
            if(ch!=EOF)
                return gettok();
        }
        else{ // divide
            return tok_div;
        }
    }

    if(ch=='%'){
        getch();
        return tok_mod;
    }

    if(ch=='+'){
        getch();
        if(ch=='+'){
            getch();
            return tok_selfadd;
        }
        else {
            return tok_add;
        }
    }
    if(ch=='-'){
        getch();
        if(ch=='-'){
            getch();
            return tok_selfmin;
        }


        else {
            return tok_sub;
        }
    }
    if(ch=='*'){
        getch();
        return tok_mul;
    }
    if(ch=='<'){ // < and <=
        getch();
        if(ch=='='){ // <=
            getch();
            return tok_leq;
        }
        else{
            return tok_lss;
        }
    }

    if(ch=='>') // > and >=
    {
        getch();
        if(ch=='='){
            getch();
            return tok_geq;
        }
        else{
            return tok_gtr;
        }
    }

    if(ch=='=') // = and ==
    {
        getch();
        if(ch=='='){
            getch();
            return tok_eql;
        }
        else{
            return tok_assign;
        }
    }

    if(ch=='!') // != and !
    {
        getch();
        if(ch=='='){
            getch();
            return tok_neq;
        }
        else{
            return tok_not;
        }
    }

    if(ch=='|'){ // ||
        getch();
        if(ch=='|'){
            getch();
            return tok_or;
        }
        else{
            return tok_illegel;
        }
    }

    if(ch=='&'){ // &&
        getch();
        if(ch=='&'){
            getch();
            return tok_and;
        }
        else{
            return tok_illegel;
        }
    }

    if(ch==';'){
        getch();
        return tok_semicolon;
    }
    if(ch==','){
        getch();
        return tok_comma;
    }
    if(ch==':'){
        getch();
        return tok_colon;
    }
    if(ch=='('){
        getch();
        return tok_lparen;
    }
    if(ch==')'){
        getch();
        return tok_rparen;
    }
    if(ch=='{'){
        getch();
        return tok_lbrace;
    }
    if(ch=='}'){
        getch();
        return tok_rbrace;
    }
    if(ch=='['){
        getch();
        return tok_lbracket;
    }
    if(ch==']'){
        getch();
        return tok_rbracket;
    }

    return tok_illegel;
}

int getNextToken(){
    if(err>=maxerr){
        return 0;
    }
    return CurTok=gettok();
}

// 当进入一个block的时候调用,给这个block一个新的namespace,并根据block的参数设置上层namespace
void new_namespace(int father_namespace,int my_namespace)
{
    if(father_namespace==0){
        return;
    }
    upperNamespaces[my_namespace].push_back(father_namespace);
    for(int i=0;i<upperNamespaces[father_namespace].size();i++){
        upperNamespaces[my_namespace].push_back(upperNamespaces[father_namespace][i]);
    }
    return;
}

//my parser


//测试当前字符是否在首符集合中,不在就报错
//有错返回 1 否则返回0
int test(std::string Vn_name){
    int i=Vntab[Vn_name];
    for(int j=0;j<firsts[i].size();j++){
        if(CurTok==firsts[i][j])
            return 0; //找到返回0
    }
    return 1; //未预料的token 返回1
}

/*
 * 生成虚拟机代码
 * x: f
 * y: l;
 * z: a;
 */
void gen(enum fct x,int y,double z){
    if(cx>=cxmax){
        //log_error("Program is too long!");
        exit(1);
    }
    if(z>=amax){
        //log_error("Displacement address is too big!");
        exit(1);
    }
    code[cx].f=x;
    code[cx].l=y;
    code[cx].a=z;
    cx++;
}

void error(int n){
    precc= precc==1? 2:precc;
    //报错并把出错的行和报错信息输出
    if(err >= maxerr){
        return;
    }
    fprintf(ferr,"Compile Failed With Error\n",preline);
    switch(n){
        case 0:
            fprintf(ferr,"line%d:%d err%d: Lack left brace {\n",preline,precc-1,n);
            break;
        case 1:
            fprintf(ferr,"line%d:%d err%d: Lack right brace }\n",preline,precc-1,n);
            break;
        case 2:
            fprintf(ferr,"line%d:%d err%d: Expect a semicolon ;\n",preline,precc-1,n);
            break;
        case 3:
            fprintf(ferr,"line%d:%d err%d: Expect an identifier! \n",preline,precc-1,n);
            break;
        case 4:
            fprintf(ferr,"line%d:%d err%d: Unexpected data type!\n",preline,precc-1,n);
            break;
        case 5:
            fprintf(ferr,"line%d:%d err%d: Undeclared identifier!\n",preline,precc-1,n);
            break;
        case 6:
            fprintf(ferr,"line%d:%d err%d: Missing right paren ) !\n",preline,precc-1,n);
            break;
        case 7:
            fprintf(ferr,"line%d:%d err%d: Missing left paren ( !\n",preline,precc-1,n);
            break;
        case 8:
            fprintf(ferr,"line%d:%d err%d: Unknown token !\n",preline,precc-1,n);
            break;
        case 9:
            fprintf(ferr,"line%d:%d err%d: Unexpected token !\n",preline,precc-1,n);
            break;
        case 10:
            fprintf(ferr,"line%d:%d err%d: Unexpected identifier !\n",preline,precc-1,n);
            break;
        case 11:
            fprintf(ferr,"line%d:%d err%d: Expect a relation operator here !\n",preline,precc-1,n);
            break;
        case 12:
            fprintf(ferr,"line%d:%d err%d: Expect a number in a int relation expression !\n",preline,precc-1,n);
            break;
        case 13:
            fprintf(ferr,"line%d:%d err%d: Too many blocks !\n",preline,precc-1,n);
            break;
        case 14:
            fprintf(ferr,"line%d:%d err%d: Can only use read to a int type var !\n",preline,precc-1,n);
            break;
        case 15:
            fprintf(ferr,"line%d:%d err%d: Unexpected token at the beginning of a program!\n",preline,precc-1,n);
            break;
        case 16:
            fprintf(ferr,"line%d:%d err%d: Unexpected token at the beginning of a block!\n",preline,precc-1,n);
            break;
        case 17:
            fprintf(ferr,"line%d:%d err%d: Unexpected token at the beginning of a declaration!\n",preline,precc-1,n);
            break;
        case 18:
            fprintf(ferr,"line%d:%d err%d: Unexpected token at the beginning of a statement!\n",preline,precc-1,n);
            break;
        case 19:
            fprintf(ferr,"line%d:%d err%d: Unexpected token at the beginning of a number expression!\n",preline,precc-1,n);
            break;
        case 20:
            fprintf(ferr,"line%d:%d err%d: Unexpected token at the beginning of a bool expression!\n",preline,precc-1,n);
            break;
        case 21:
            fprintf(ferr,"line%d:%d err%d: Can only use readf to a double type var !\n",preline,precc-1,n);
            break;
        case 22:
            fprintf(ferr,"line%d:%d err%d: Expect a float number in a float relation expression !\n",preline,precc-1,n);
            break;
        case 23:
            fprintf(ferr,"line%d:%d err%d: Unexcepted operator !\n",preline,precc-1,n);
            break;
        case 24:
            fprintf(ferr,"line%d:%d err%d: Can only use ++ and -- on int !\n",preline,precc-1,n);
            break;
        case 25:
            fprintf(ferr,"line%d:%d err%d: Expect a int type!\n",preline,precc-1,n);
            break;
        case 26:
            fprintf(ferr,"line%d:%d err%d: Expect a float type!\n",preline,precc-1,n);
            break;
        case 27:
            fprintf(ferr,"line%d:%d err%d: Expect a ]!\n",preline,precc-1,n);
            break;
        case 28:
            fprintf(ferr,"line%d:%d err%d: Expect a integer to indicate the array size!\n",preline,precc-1,n);
            break;
        case 29:
            fprintf(ferr,"line%d:%d err%d: Array size must be greater than 0!\n",preline,precc-1,n);
            break;
        case 30:
            fprintf(ferr,"line%d:%d err%d: Expect a [!\n",preline,precc-1,n);
            break;
        case 31:
            fprintf(ferr,"line%d:%d err%d: Expect a identifier at the beginning at a assignment statement!\n",preline,precc-1,n);
            break;
        case 32:
            fprintf(ferr,"line%d:%d err%d: Expect a int number after case!\n",preline,precc-1,n);
            break;
        case 33:
            fprintf(ferr,"line%d:%d err%d: Expect a : after case!\n",preline,precc-1,n);
            break;
        case 34:
            fprintf(ferr,"line%d:%d err%d: Unexpected break!\n",preline,precc-1,n);
            break;
        case 35:
            fprintf(ferr,"line%d:%d err%d: Unexpected continue!\n",preline,precc-1,n);
            break;
        case 36:
            fprintf(ferr,"line%d:%d err%d: Unexpected function return type!\n",preline,precc-1,n);
            break;
        case 37:
            fprintf(ferr,"line%d:%d err%d: Expect a case here!\n",preline,precc-1,n);
            break;
    }
    err = err + 1;
}


void enter(int name_space,int type,std::string name,int size,double val,int dx){
    temp_tablestruct.name=name;
    temp_tablestruct.size=size;
    temp_tablestruct.val=val;
    temp_tablestruct.type=type;
    temp_tablestruct.name_space=name_space;
    temp_tablestruct.index=dx;
    tables[name_space].push_back(temp_tablestruct);
}
void enterFunction(int name_space,int type,std::string name,std::vector<struct parameter> param_list,int L){
    temp_tablestruct.name=name;
    temp_tablestruct.type=type;
    temp_tablestruct.name_space=name_space;
    temp_tablestruct.paramList=param_list;
    temp_tablestruct.index=L;
    tables[name_space].push_back(temp_tablestruct);
}

void fillback(int L,int fun,int lev){
    if(fun==fb_break){
        for(int i=0;i<fillbackList_break[lev].size();i++){
            int c=fillbackList_break[lev][i];
            code[c].a=(double)L;
        }
        fillbackList_break[lev].clear();
    }
    else if(fun==fb_continue){
        for(int i=0;i<fillbackList_continue[lev].size();i++){
            int c=fillbackList_continue[lev][i];
            code[c].a=(double)L;
        }
        fillbackList_continue[lev].clear();
    }
    else if(fun==fb_return){
        for(int i=0;i<fillbackList_return[lev].size();i++){
            int c=fillbackList_return[lev][i];
            code[c].a=(double)L;
        }
        fillbackList_return[lev].clear();
    }
    else if(fun==fb_exit){
        for(int i=0;i<fillbackList_exit.size();i++){
            int c=fillbackList_exit[i];
            code[c].a=(double)L;
        }
        fillbackList_exit.clear();
    }
    return;
}

void program(){
    if(test("program")){
        error(15);
        return;
    }
    block(0,returntype_notfunction);
    fillback(cx-1,fb_exit,0); //跳到程序最后一个ssp
    //fprintf(ferr,"\n===parsed a program!===\n");
}

int block(int upper_namespace,int returntype){ //lev 是上级namespace
    if(test("block")){
        error(16);
        return -1;
    }
    if(err >= maxerr){
        return -1;
    }
    if(namespacecount<maxnamespace-1){
        namespacecount++;
    }
    else{
        error(13);
        return -1;
    }
    int my_lev=namespacecount;
    new_namespace(upper_namespace,my_lev);
    gen(ssp,my_lev,0);
    if(CurTok==tok_lbrace){
        getNextToken(); // eat {
        decls(my_lev);
        stmts(my_lev,returntype);
        if(CurTok==tok_rbrace){
            getNextToken();
            gen(lsp,my_lev,0);
            return my_lev;
        }
        else{
            //log_error("missing right at the end of a block!");
            error(1);
            return -1;
        }
    }
    else{
        //log_error("missing left brace at the start of a block!");
        error(0);
        return -1;
    }
}

void decls(int my_lev){ // 参数为自己的namespace
    if(test("decls")){
        error(17);
        return;
    }
    if(err >= maxerr){
        return;
    }
    while(CurTok==tok_int||CurTok==tok_bool||CurTok==tok_float||CurTok==tok_function){
        decl(my_lev);
    }
    return;
}

void decl(int my_lev){
    if(test("decl")){
        error(17);
        return;
    }
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_int){
        getNextToken(); // eat int
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            gen(ini,0,1);
            enter(my_lev,type_int,id,1,0,dxs[my_lev]);
            dxs[my_lev]+=1;
            getNextToken();
            if(CurTok==tok_semicolon){
                getNextToken();
                return;
            }
            else{
                //log_error("missing ; in the end of decalaration!");
                error(2);
                return;
            }
        }
        else if(CurTok==tok_lbracket){ // int 数组
            getNextToken();//eat [
            if(CurTok!=tok_intnum){ //缺少数组大小
                error(28);
                return;
            }

            int size=(int)NumVal;
            if(size<=0){ //数组大小不对劲
                error(29);
                return;
            }
            getNextToken(); //eat num

            if(CurTok!=tok_rbracket){ //缺少右方括号
                error(27);
                return;
            }
            getNextToken(); //eat ]

            if(CurTok!=tok_identifier){
                error(3);
                return;
            }
            std::string id=IdentifierStr;
            gen(ini,0,size); //分配控件
            enter(my_lev,type_iarr,id,size,0,dxs[my_lev]);
            dxs[my_lev]+=size;
            getNextToken(); //eat id
            if(CurTok==tok_semicolon){
                getNextToken();
                return;
            }
            else{
                //log_error("missing ; in the end of decalaration!");
                error(2);
                return;
            }
        }
        else{
            //log_error("missing identifier in decalaration!");
            error(3);
            return;
        }
    }
    else if (CurTok==tok_bool){
        getNextToken();
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            gen(ini,0,1);
            enter(my_lev,type_bool,id,1,0,dxs[my_lev]);
            dxs[my_lev]+=1;
            getNextToken();
            if(CurTok==tok_semicolon){
                getNextToken();
                return;
            }
            else{
                //log_error("missing ; in the end of decalaration!");
                error(2);
                return;
            }
        }
        else{
            //log_error("missing identifier in decalaration!");
            error(3);
            return;
        }
    }
    else if (CurTok==tok_float){ //定义一个浮点变量
        getNextToken(); //eat float
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            gen(ini,0,1); //分配空间
            enter(my_lev,type_float,id,1,0,dxs[my_lev]);
            dxs[my_lev]+=1;
            getNextToken();
            if(CurTok==tok_semicolon){
                getNextToken();
                return;
            }
            else{
                //log_error("missing ; in the end of decalaration!");
                error(2);
                return;
            }
        }
        else if(CurTok==tok_lbracket){ //float 数组
            getNextToken();//eat [
            if(CurTok!=tok_intnum){ //缺少数组大小
                error(28);
                return;
            }

            int size=(int)NumVal;
            if(size<=0){ //数组大小不对劲
                error(29);
                return;
            }
            getNextToken(); //eat num

            if(CurTok!=tok_rbracket){ //缺少右方括号
                error(27);
                return;
            }
            getNextToken(); //eat ]

            if(CurTok!=tok_identifier){
                error(3);
                return;
            }
            std::string id=IdentifierStr;
            gen(ini,0,size); //分配空间
            enter(my_lev,type_farr,id,size,0,dxs[my_lev]);
            dxs[my_lev]+=size;
            getNextToken(); //eat id
            if(CurTok==tok_semicolon){
                getNextToken(); //eat ;
                return;
            }
            else{
                //log_error("missing ; in the end of decalaration!");
                error(2);
                return;
            }
        }
        else{
            //log_error("missing identifier in decalaration!");
            error(3);
            return;
        }
    }
    else if(CurTok==tok_function){ //定义一个方程
        getNextToken(); //eat function
        int returntype;
        if(CurTok==tok_void||CurTok==tok_int||CurTok==tok_float)
            returntype=CurTok;
        else {
            error(36);
            return;
        }
        int blockreturntype=-1;
        int functiontype=type_vfun;
        switch (returntype) {
            case tok_void:{
                blockreturntype=returntype_void;
                functiontype=type_vfun;
                break;
            }
            case tok_int:{
                blockreturntype=returntype_int;
                functiontype=type_ifun;
                break;
            }
            case tok_float:{
                blockreturntype=returntype_float;
                functiontype=type_ffun;
                break;
            }
            default:
                error(36);
                return;
        }
        getNextToken(); //eat void int float

        if(CurTok!=tok_identifier){
            error(3);
            return;
        }
        std::string functionname=IdentifierStr;

        getNextToken(); //eat id

        if(CurTok!=tok_lparen){
            error(7);
            return;
        }
        getNextToken(); //eat (

        if(CurTok==tok_rparen){ //无参数
            getNextToken(); //eat )
            int functionlev;
            std::vector<struct parameter> param_list(0);

            //jmp 0 0
            int cx1=cx;
            gen(jmp,0,0); //跳过方程中的语句
            enterFunction(my_lev,functiontype,functionname,param_list,cx);
            //对方程而言 val dx值是不用的
            functionlev=block(my_lev,blockreturntype);
            code[cx1].a=(double)cx; //跳过方程的定义
        }
        else if(CurTok==tok_int||CurTok==tok_float){ //有参数
            std::vector<struct parameter> param_list(0);
            int i=-1;
            while(CurTok==tok_int||CurTok==tok_float){
                int paramtype;
                switch(CurTok){
                    case tok_int:
                        paramtype=type_int;
                        break;
                    case tok_float:
                        paramtype=type_float;
                        break;
                }
                getNextToken(); //eat int/float
                if(CurTok!=tok_identifier){
                    error(3);
                    return;
                }
                std::string paramname=IdentifierStr;
                temp_parameter.type=paramtype;
                temp_parameter.name=paramname;
                temp_parameter.dx=i;
                enter(namespacecount+1,paramtype,paramname,1,0,i);
                i--;
                param_list.push_back(temp_parameter);
                getNextToken(); //eat id;
                if(CurTok==tok_comma){
                    getNextToken(); //eat ,
                    if(CurTok==tok_int||CurTok==tok_float){
                        continue;
                    }
                    else{
                        error(4);
                        return;
                    }
                }
                else if(CurTok==tok_rparen){
                    getNextToken(); //eat )
                    int cx1=cx;
                    gen(jmp,0,0);
                    enterFunction(my_lev,functiontype,functionname,param_list,cx);
                    int functionlev;
                    functionlev=block(my_lev,blockreturntype);
                    code[cx1].a=(double)cx; //回填跳过方程定义
                }
            }
        }
    }//end of function decl

    else{
        //log_error("unknown data type in declaration!");
        error(4);
        return;
    }
}

void stmts(int my_lev,int returntype){
    if(test("stmts")){
        error(18);
        return;
    }
    if(err >= maxerr){
        return;
    }
    while(CurTok==tok_identifier||CurTok==tok_if||CurTok==tok_while||
           CurTok==tok_write||CurTok==tok_read||CurTok==tok_lbrace
            ||CurTok==tok_writef||CurTok==tok_readf
            ||CurTok==tok_selfadd||CurTok==tok_selfmin
            ||CurTok==tok_for||CurTok==tok_switch
            ||CurTok==tok_break||CurTok==tok_continue||CurTok==tok_exit
          ){
        if(err >= maxerr){
            return;
        }
        stmt(my_lev,returntype);
    }
    return;
}


// 按name和namespace查找,如果找不到,报错
// 这里有点乱,需要仔细检查
struct tablestruct& getTablestructById(int name_space,std::string name){
    cantFindName=0;
    vector<int> searchRange(0);
    searchRange.push_back(0); // 0即program 是所有namespace的父
    //查询嵌套表找到所有要搜索的namespace
    //这里应该倒序搜索,优先匹配最近的那个
    for(int i=upperNamespaces[name_space].size()-1;i>=0;i--){
        searchRange.push_back(upperNamespaces[name_space][i]);
    }
    //最后把自己加入
    searchRange.push_back(name_space);
    //倒叙搜索searchRange 在符号表中找name
    for(int i=searchRange.size()-1;i>=0;i--){
        int searchns=searchRange[i];
        for(int j=tables[searchns].size()-1;j>=0;j--){ //倒序查找,重复的定义我们取最近的
            if(tables[searchns][j].name==name){
                return tables[searchns][j]; //找到了
            }
        }
    }
    cantFindName=1;
    error(5);
    return temp_tablestruct; //随便return一个反正出错了
}


void selfaddmin_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    switch(CurTok){
        case tok_selfadd:
            {
            getNextToken();// eat ++
            if(CurTok!=tok_identifier){
                error(3);
                return;
            }
            std::string id=IdentifierStr;
            tablestruct t=getTablestructById(my_lev,id);
            if(cantFindName){ //检查是否定义
                error(5);
                return;
            }
            if(t.type!=type_int){ // 检查类型
                error(25);
                return;
            }
            getNextToken(); //eat aid
            if(CurTok!=tok_semicolon){
                error(2);
                return;
            }
            getNextToken(); //eat ;
            gen(lod,t.name_space,t.index);
            gen(lit,0,1);
            gen(opr,0,2);
            gen(sto,t.name_space,t.index);
            return;
            }
        case tok_selfmin:
            {
            getNextToken();// eat --
            if(CurTok!=tok_identifier){
                error(3);
                return;
            }
            std::string id=IdentifierStr;
            tablestruct t=getTablestructById(my_lev,id);
            if(cantFindName){ //检查是否定义
                error(5);
                return;
            }
            if(t.type!=type_int){ // 检查类型
                error(25);
                return;
            }
            getNextToken(); //eat aid
            if(CurTok!=tok_semicolon){
                error(2);
                return;
            }
            getNextToken(); //eat ;
            gen(lod,t.name_space,t.index);
            gen(lit,0,1);
            gen(opr,0,3);
            gen(sto,t.name_space,t.index);
            return;
            }
        default:
            error(9);
            return;
    }
}

void assign_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    if(CurTok!=tok_identifier){
        error(31);
        return;
    }
    std::string id=IdentifierStr;
    struct tablestruct t=getTablestructById(my_lev,id);
    if(cantFindName){
        error(5);
        return;
    }
    int type=t.type;

    //        if(type==-1){
    //            //log_error("undeclared identifier!");
    //        }
    getNextToken();//eat id;
    if(type==type_bool||type==type_int||type==type_float)
    {
        if(CurTok==tok_assign){
            getNextToken();//eat =
            switch(type){
                case type_int:{
                    intexpr(my_lev);
                    break;
                }
                case type_bool:{
                    boolexpr(my_lev);
                    break;
                }
                case type_float:{
                    floatexpr(my_lev);
                    break;
                }
                default:{
                    //log_error("undeclared identifier!");
                    error(4);
                    return;
                }
            }
            gen(sto,t.name_space,t.index); // 这时候上面表达式的值会在栈顶,所以我们将值直接sto进符号表
            if(CurTok==tok_semicolon){
                getNextToken();//eat ;
                return;
            }
            else{
                //log_error("missing ; at the end of expression");
                error(2);
                return;
            }
        }
        else if(CurTok==tok_selfadd||CurTok==tok_selfmin){
            int op=CurTok;
            getNextToken(); //eat ++ --
            if(CurTok!=tok_semicolon){
                error(2);
                return;
            }
            getNextToken(); //eat ;
            if(type==type_int){
                switch(op){
                    case tok_selfadd:
                    {
                        gen(lod,t.name_space,t.index);
                        gen(lit,0,1);
                        gen(opr,0,2); // aid=aid+1;
                        gen(sto,t.name_space,t.index);
                        break;
                    }
                    case tok_selfmin:
                    {
                        gen(lod,t.name_space,t.index);
                        gen(lit,0,1);
                        gen(opr,0,3);
                        gen(sto,t.name_space,t.index);
                        break;
                    }
                }

            }
            else{
                error(24);
                return;
            }
        }
        else {
            error(9);
            return;
        }
    }
    else if(type==type_iarr||type==type_farr){
        if(CurTok!=tok_lbracket){
            error(30);
            return;
        }
        getNextToken(); //eat [

        intexpr(my_lev);
        //gen(opr,0,29); //栈顶值设为offset

        if(CurTok!=tok_rbracket){
            error(27);
            return;
        }
        getNextToken(); //ear ]

        if(CurTok==tok_assign){
            getNextToken();//eat =
            switch(type){
                case type_iarr:{
                    intexpr(my_lev);
                    break;
                }
                case type_farr:{
                    floatexpr(my_lev);
                    break;
                }
                default:{
                    //log_error("undeclared identifier!");
                    error(4);
                    return;
                }
            }
            gen(ost,t.name_space,t.index);
            // 这时候上面表达式的值会在栈顶,
            // offset 会在次栈顶
            //所以我们将值ost
            if(CurTok==tok_semicolon){
                getNextToken();//eat ;
                return;
            }
            else{
                //log_error("missing ; at the end of expression");
                error(2);
                return;
            }
        }
        else if(CurTok==tok_selfadd||CurTok==tok_selfmin){
            int op=CurTok;
            getNextToken(); //eat ++ --
            if(CurTok!=tok_semicolon){
                error(2);
                return;
            }
            getNextToken(); //eat ;
            if(type==type_iarr){
                gen(opr,0,29);
                switch(op){
                    case tok_selfadd:
                    {
                        gen(old,t.name_space,t.index);
                        gen(lit,0,1);
                        gen(opr,0,2); // aid=aid+1;
                        gen(ost,t.name_space,t.index);
                        break;
                    }
                    case tok_selfmin:
                    {
                        gen(old,t.name_space,t.index);
                        gen(lit,0,1);
                        gen(opr,0,3);
                        gen(ost,t.name_space,t.index);
                        break;
                    }
                }

            }
            else{
                error(24);
                return;
            }
        }
        else {
            error(9);
            return;
        }


    }
    else{
        error(4);
        return;
    }
    return;
}


void if_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken();//eat if

    int cx0; //记录 无else的 jpc L1 位置 或者 有else的 jpc Lfalse 位置
    int cx1; //记录 有else的 jpc L1 位置
    if(CurTok==tok_lparen){
        getNextToken();//eat (
        boolexpr(my_lev);
        cx0=cx;
        gen(jpc,0,0);
        //对于无else 这里是 jpc L1 否则是 jpc Lfalse
        //分别跳转到stmt后一句  跳转到 false-stmt的开头
        if(CurTok==tok_rparen){
            getNextToken();//eat )
            stmt(my_lev,returntype);
            //这里的是 true-stmt,先不急gencode,需要在看了有没有else才能做
            if(CurTok==tok_else){
                //有else
                getNextToken(); // eat else
                cx1=cx;
                gen(jmp,0,0); // jmp L1
                code[cx0].a=cx; //接下来是false-stmt的code,我们让上面的jpc跳转到这里,没毛病嗷~~
                stmt(my_lev,returntype);
                code[cx1].a=cx; //这里让jmp 跳出if else 块
                return;
            }else{
                //没有else
                code[cx0].a=cx; //这里给jpc L1打补丁,让if 在条件不满足时跳过stmt
                return;
            }
        }
        else{
            //log_error("missing right paren!");
            error(6);
            return;
        }
    }
    else{
        //log_error("missing left paren after if!");
        error(7);
        return;
    }
}

void exit_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat exit
    if(CurTok!=tok_semicolon){
        error(2);
        return;
    }
    getNextToken(); // eat ;
    fillbackList_exit.push_back(cx);
    gen(jmp,0,0);
    return;
}
//可以在switchcase while for中使用
void break_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    if(nestbreak<0){
        error(34);
        return;
    }
    getNextToken(); //eat break
    if(CurTok!=tok_semicolon){
        error(2);
        return;
    }
    getNextToken(); // eat ;
    fillbackList_break[nestbreak].push_back(cx);
    //在这里生成一个跳转语句
    //跳转地址会在对应的块处理完毕之后回填
    gen(jmp,0,0);
    return;
}

void continue_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    if(nestcontinue<0){
        error(35);
        return;
    }
    getNextToken(); //eat continue
    if(CurTok!=tok_semicolon){
        error(2);
        return;
    }
    getNextToken(); // eat ;
    fillbackList_continue[nestcontinue].push_back(cx);
    gen(jmp,0,0);
    return;
}

void switchcase_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken();//eat switch
    nestbreak++;

    if(CurTok!=tok_lparen){
        error(7);
        return;
    }
    getNextToken(); //eat (

    intexpr(my_lev);

    if(CurTok!=tok_rparen){
        error(6);
        return;
    }
    getNextToken(); //eat )

    if(CurTok!=tok_lbrace){
        error(0);
        return;
    }
    getNextToken(); //eat {

    if(namespacecount<maxnamespace-1){
        namespacecount++;
    }
    int lev=namespacecount;
    new_namespace(my_lev,lev);
    gen(ssp,lev,0);

    if(CurTok!=tok_case){
        error(37);
        return;
    }
    int c1=-1;
    while (CurTok==tok_case) {
        getNextToken(); //eat case
        if(CurTok!=tok_intnum){
            error(32);
            return;
        }
        int num=(int)NumVal;
        if(c1!=-1){ //如果不是第一个case,回填上面比较失败后的跳转地址
            code[c1].a=(double)cx;
        }
        gen(opr,0,29); //复制栈顶的那个intexpr
        gen(lit,0,num);
        gen(opr,0,8); //比较switch
        c1=cx;
        gen(jpc,0,0); //跳转到下一个case
        getNextToken(); //eat num

        if(CurTok!=tok_colon){
            error(33);
            return;
        }
        getNextToken(); //eat :

        block(my_lev,returntype);
        fillbackList_break[nestbreak].push_back(cx);
        gen(jmp,0,0);
    }

    if(CurTok!=tok_rbrace){
        error(1);
        return;
    }
    getNextToken(); // eat }
    code[c1].a=cx; //给上一个case回填跳转地址
    fillback(cx,fb_break,nestbreak); //给之前的block回填跳出switch case 块的地址
    nestbreak--;
    gen(lsp,lev,0);//回收空间
    gen(opr,0,30); //退出原本多的一个switch(里的值)
    return;
}

void while_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat while
    int hasBlock=0;
    int l1=cx; //l1为循环开始
    if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr(my_lev);
        int cx0=cx;
        int loopblocklev=0;
        gen(jpc,0,0); //跳出循环
        if(CurTok==tok_rparen){
            getNextToken(); //eat )

            if(CurTok==tok_lbrace)
                //循环体是block 我们允许使用break 否则不允许
            {
                nestbreak++;
                nestcontinue++;
                hasBlock=1;
                loopblocklev=block(my_lev,returntype);
            }

            else {
                hasBlock=0;
                stmt(my_lev,returntype);
            }

            gen(jmp,0,l1); //回到while开始


            if(hasBlock)
            {
                int cx1=0;
                int cx2=0;
                fillback(cx,fb_break,nestbreak); //将出循环的指令地址回填给break
                gen(lsp,loopblocklev,0); //break出来到这条指令,回收循环block堆栈
                cx1=cx;
                gen(jmp,0,0); //跳转到真的出循环
                fillback(cx,fb_continue,nestcontinue); //将回到循环开始的地址回填给continue
                gen(lsp,loopblocklev,0); //continue之后还是回收循环block堆栈
                cx2=cx;
                gen(jmp,0,l1); //跳转到循环开始
                nestbreak--;
                nestcontinue--;
                code[cx0].a=cx; //让jpc 跳出循环
                code[cx1].a=cx; //回填让break出来回收堆栈之后跳出循环
            }
            else{
                code[cx0].a=cx; //让jpc 跳出循环
            }
            return;
        }else{
            //log_error("missing right paren!");
            error(6);
            return;
        }
    }
    else{
        //log_error("missing left paren after while!");
        error(7);
        return;
    }
}

void for_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat for

    if(CurTok!=tok_lparen){
        error(7);
        return;
    }
    getNextToken(); //eat (

    assign_stmt(my_lev,returntype); // for循环初始化语句
    int lcondition=cx;

    boolexpr(my_lev); // for循环条件
    if(CurTok!=tok_semicolon){
        error(2);
        return;
    }
    int cx1=cx;
    gen(jpc,0,0);
    int cx2=cx;
    gen(jmp,0,0);

    getNextToken(); //eat ;

    int ldo=cx; //stmt中的continue应该跳转到 ldo
    assign_stmt(my_lev,returntype); //for循环每次循环后执行的
    gen(jmp,0,lcondition);

    if(CurTok!=tok_rparen){
        error(6);
        return;
    }
    getNextToken(); //eat )

    int lloop=cx;
    int loopblocklev=-1;
    int hasBlock=0;
    if(CurTok==tok_lbrace){
        hasBlock=1;
        nestbreak++;
        nestcontinue++;
        loopblocklev=block(my_lev,returntype);
    }
    else{
        stmt(my_lev,returntype);
    }
    gen(jmp,0,ldo);


    if(hasBlock){
        fillback(cx,fb_break,nestbreak);
        gen(lsp,loopblocklev,0);
        int cx3=cx;
        gen(jmp,0,0);
        fillback(cx,fb_continue,nestcontinue);
        gen(lsp,loopblocklev,0);
        gen(jmp,0,ldo);
        nestcontinue--;
        nestbreak--;

        int lout=cx; //stmt中的break应该跳转到 lout
        code[cx1].a=(double)lout;
        code[cx2].a=(double)lloop;
        code[cx3].a=(double)lout;
    }
    else{
        int lout=cx; //stmt中的break应该跳转到 lout
        code[cx1].a=(double)lout;
        code[cx2].a=(double)lloop;
    }



    return;
}


void write_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat write
    intexpr(my_lev);
    gen(opr,0,14); //输出栈顶的值,也就是刚才的intexpr的值;
    if(CurTok!=tok_semicolon){
        //log_error("missing ; at the end of statement");
        error(2);
        return;
    }
    else{
        getNextToken(); //eat ;
    }
    return;
}


void read_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat read
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            error(5);
            return;
        }
        int type=t.type;
        if(type!=type_int){
            error(14);
            return;
        }
        getNextToken(); // eat id
        if(CurTok==tok_semicolon){
            getNextToken(); // eat ;

            gen(opr,0,16); //从输入读取一个数然后放在栈顶
            gen(sto,t.name_space,t.index); //将刚才读取的栈顶的值输入给变量

            return;
        }
        else{
            //log_error("missing ;");
            error(2);
            return;
        }
    }
    else{
        //log_error("expecting an identifire but receive other token!");
        error(3);
        return;
    }
}

void writef_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat write
    floatexpr(my_lev);
    gen(opr,0,24); //输出栈顶的值,也就是刚才的intexpr的值;
    if(CurTok!=tok_semicolon){
        //log_error("missing ; at the end of statement");
        error(2);
        return;
    }
    else{
        getNextToken(); //eat ;
    }
    return;
}


void readf_stmt(int my_lev,int returntype){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat read
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            error(5);
            return;
        }
        int type=t.type;
        if(type!=type_float){
            error(21);
            return;
        }
        getNextToken(); // eat id
        if(CurTok==tok_semicolon){
            getNextToken(); // eat ;

            gen(opr,0,25); //从输入读取一个数然后放在栈顶
            gen(sto,t.name_space,t.index); //将刚才读取的栈顶的值输入给变量

            return;
        }
        else{
            //log_error("missing ;");
            error(2);
            return;
        }
    }
    else{
        //log_error("expecting an identifire but receive other token!");
        error(3);
        return;
    }
}


void stmt(int my_lev,int returntype){
    if(test("stmt")){
        error(18);
        return;
    }
    if(err >= maxerr){
        return;
    }
    switch(CurTok){
    case tok_selfadd:
    case tok_selfmin:
    {
        selfaddmin_stmt(my_lev,returntype);
        break;
    }
    case tok_identifier:{
        assign_stmt(my_lev,returntype);
        break;
    }
    case tok_if:{
        if_stmt(my_lev,returntype);
        break;
    }
    case tok_while:{
        while_stmt(my_lev,returntype);
        break;
    }
    case tok_for:{
        for_stmt(my_lev,returntype);
        break;
    }
    case tok_switch:{
        switchcase_stmt(my_lev,returntype);
        break;
    }
    case tok_write:{
        write_stmt(my_lev,returntype);
        break;
    }
    case tok_read:{
        read_stmt(my_lev,returntype);
        break;
    }
    case tok_writef:{
        writef_stmt(my_lev,returntype);
        break;
    }
    case tok_readf:{
        readf_stmt(my_lev,returntype);
        break;
    }
    case tok_lbrace:{
        block(my_lev,returntype);
        break;
    }
    case tok_break:{
        break_stmt(my_lev,returntype);
        break;
    }
    case tok_continue:{
        continue_stmt(my_lev,returntype);
        break;
    }
    case tok_exit:{
        exit_stmt(my_lev,returntype);
        break;
    }
    default:
        //log_error("unknown token when expecting a statement!");
        error(8);
        break;
    }
}

void intexpr(int my_lev){
    if(test("intexpr")){
        error(19);
        return;
    }
    if(err >= maxerr){
        return;
    }

    intterm(my_lev);
    switch(CurTok){
        case tok_add:{
            getNextToken(); // eat +
            intterm(my_lev);
            gen(opr,0,2); //生成加法指令
            break;
        }
        case tok_sub:{
            getNextToken(); // eat -
            intterm(my_lev);
            gen(opr,0,3); //生成减法指令
            break;
        }
        default:{

            return;
        }
    }
}

void intterm(int my_lev){
    if(err >= maxerr){
        return;
    }
    intfactor(my_lev);
    switch(CurTok){
    case tok_mul:{
        getNextToken(); // eat *
        intfactor(my_lev);
        gen(opr,0,4);//生成乘法指令
        break;
    }
    case tok_div:{
        getNextToken(); // eat /
        intfactor(my_lev);
        gen(opr,0,5);//生成除法指令
        break;
    }
    case tok_mod:{
        getNextToken(); //eat %
        intfactor(my_lev);
        gen(opr,0,26); //生成求余指令
        break;
    }
    default:{

        return;
    }
    }
}

void intfactor(int my_lev){
    if(err >= maxerr){
        return;
    }
    switch(CurTok){
        case tok_sub:
        {
            getNextToken(); //eat -

            if(CurTok==tok_identifier)
            {
                std::string id=IdentifierStr;
                struct tablestruct t=getTablestructById(my_lev,id);
                if(cantFindName){
                    error(5);
                    return;
                }
                int type=t.type;
                getNextToken();//eat id
                if(type==type_int){
                    //上面检查完是否定义和类型
                    gen(lod,t.name_space,t.index); //找到变量值并入栈
                    gen(opr,0,1); //栈顶取反
                    return;
                }
                else if(type==type_iarr){
                    if(CurTok!=tok_lbracket){
                        error(30);
                        return;
                    }
                    getNextToken(); //eat [

                    intexpr(my_lev);
                    //gen(opr,0,29);//设置OFFSET

                    if(CurTok!=tok_rbracket){
                        error(27);
                        return;
                    }
                    getNextToken(); //eat ]

                    gen(old,t.name_space,t.index); //找到数组元素然后入栈
                    gen(opr,0,1); //栈顶取反
                    return;
                }
                else{
                    error(25);
                    return;
                }
            }
            else if(CurTok==tok_intnum){
                int num=(int)NumVal;
                gen(lit,0,(double)num);
                gen(opr,0,1);
                getNextToken(); //eat num
                return;
            }
            else{
                error(9);
                return;
            }

        }
        case tok_identifier:
        {
            std::string id=IdentifierStr;
            struct tablestruct t=getTablestructById(my_lev,id);
            if(cantFindName){
                error(5);
                return;
            }
            int type=t.type;
            if(type==type_int){
                gen(lod,t.name_space,t.index); //找到变量值并入栈
                getNextToken(); // eat id
                return;
            }
            else if(type==type_iarr){
                getNextToken(); //eat id
                if(CurTok!=tok_lbracket){
                    error(30);
                    return;
                }
                getNextToken(); //eat [

                intexpr(my_lev);
                //gen(opr,0,29);//设置OFFSET

                if(CurTok!=tok_rbracket){
                    error(27);
                    return;
                }
                getNextToken(); //eat ]

                gen(old,t.name_space,t.index); //找到数组元素然后入栈

                return;
            }
            else{
                error(25);
                return;
            }
            break;
        }
        case tok_intnum: //因子是一个立即数
        {
            int num = (int)NumVal;
            gen(lit,0,num);
            getNextToken(); //eat NUM

            return;
        }
        case tok_lbrace: //因子是一个表达式
        {
            getNextToken(); //eat (
            intexpr(my_lev);
            if(CurTok==tok_rbrace){
                getNextToken();//eat )

                return;
            }
            else{
                //log_error("missing right brace!");
                error(1);
                return;
            }
            break;
        }
        default:
        {
            //log_error("unexpected token!");
            error(9);
            return;
        }
    }
}

void floatexpr(int my_lev){
    if(test("floatexpr")){
        error(19);
        return;
    }
    if(err >= maxerr){
        return;
    }
    floatterm(my_lev);
    switch(CurTok){
        case tok_add:{
            getNextToken(); // eat +
            floatterm(my_lev);
            gen(opr,0,20); //生成加法指令
            break;
        }
        case tok_sub:{
            getNextToken(); // eat -
            floatterm(my_lev);
            gen(opr,0,21); //生成减法指令
            break;
        }
        default:{

            return;
        }
    }
}

void floatterm(int my_lev){
    if(err >= maxerr){
        return;
    }
    floatfactor(my_lev);
    switch(CurTok){
    case tok_mul:{
        getNextToken(); // eat *
        floatfactor(my_lev);
        gen(opr,0,22);//生成乘法指令
        break;
    }
    case tok_div:{
        getNextToken(); // eat /
        floatfactor(my_lev);
        gen(opr,0,23);//生成除法指令
        break;
    }
    default:{

        return;
    }
    }
}

void floatfactor(int my_lev){
    if(err >= maxerr){
        return;
    }
    switch(CurTok){
        case tok_sub:
        {
            getNextToken(); //eat -
            if(CurTok==tok_identifier){
                std::string id=IdentifierStr;
                struct tablestruct t=getTablestructById(my_lev,id);
                if(cantFindName){
                    error(5);
                    return;
                }
                int type=t.type;
                if(type==type_float){
                    //上面检查完是否定义和类型
                    getNextToken();//eat cid
                    gen(lod,t.name_space,t.index); //找到变量值并入栈
                    gen(opr,0,1); //栈顶取反
                    return;
                }
                else if(type==type_farr){
                    getNextToken(); //eat id
                    if(CurTok!=tok_lbracket){
                        error(30);
                        return;
                    }
                    getNextToken(); //eat [

                    intexpr(my_lev);
                    //gen(opr,0,29);//设置OFFSET

                    if(CurTok!=tok_rbracket){
                        error(27);
                        return;
                    }
                    getNextToken(); //eat ]

                    gen(old,t.name_space,t.index); //找到数组元素然后入栈
                    gen(opr,0,1); //栈顶取反
                    return;
                }
                else{
                    error(26);
                    return;
                }
            }
            else if(CurTok==tok_floatnum){
                double num=NumVal;
                gen(lit,0,num);
                gen(opr,0,1);
                getNextToken(); //eat num
                return;
            }
            else{
                error(9);
                return;
            }


        }
        case tok_identifier:
        {
            std::string id=IdentifierStr;
            struct tablestruct t=getTablestructById(my_lev,id);
            if(cantFindName){
                error(5);
                return;
            }
            int type=t.type;
            if(type==type_float){
                gen(lod,t.name_space,t.index); //找到变量值并入栈
                getNextToken(); // eat id
                return;
            }
            else if(type==type_farr){
                getNextToken(); //eat id
                if(CurTok!=tok_lbracket){
                    error(30);
                    return;
                }
                getNextToken(); //eat [

                intexpr(my_lev);
                //gen(opr,0,29);//设置OFFSET

                if(CurTok!=tok_rbracket){
                    error(27);
                    return;
                }
                getNextToken(); //eat ]

                gen(old,t.name_space,t.index); //找到数组元素然后入栈
                return;

            }
            else{
                error(26);
                return;
            }
            break;

        }
        case tok_floatnum: //因子是一个立即数
        {
            double num = (double)NumVal;
            gen(lit,0,num);
            getNextToken(); //eat NUM
            return;
        }
        case tok_lbrace: //因子是一个表达式
        {
            getNextToken(); //eat (
            floatexpr(my_lev);
            if(CurTok==tok_rbrace){
                getNextToken();//eat )
                return;
            }
            else{
                //log_error("missing right brace!");
                error(1);
                return;
            }
            break;
        }
        default:
        {
            //log_error("unexpected token!");
            error(9);
            return;
        }
    }
}

void boolexpr(int my_lev){
    if(test("boolexpr")){
        error(20);
        return;
    }
    if(err >= maxerr){
        return;
    }
    boolterm(my_lev);
    boolexpr_(my_lev);
}

void boolexpr_(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_or){
        getNextToken(); //eat ||
        boolterm(my_lev);
        boolexpr_(my_lev);
        gen(opr,0,18);
    }
    else{
        return;
    }
    return;
}

void boolterm(int my_lev){
    if(err >= maxerr){
        return;
    }
    boolfactor(my_lev);
    boolterm_(my_lev);

}

void boolterm_(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_and){
        getNextToken(); //eat &&
        boolfactor(my_lev);
        boolterm_(my_lev);
        gen(opr,0,17);
    }
    else if(CurTok==tok_xor){
        getNextToken(); //eat XOR
        boolfactor(my_lev);
        boolterm_(my_lev);
        gen(opr,0,27);
    }
    else{
        return;
    }
    return;
}

void boolfactor(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_true){
        getNextToken(); //eat true;
        gen(lit,0,1);
        return;
    }
    else if(CurTok==tok_false){
        getNextToken(); //eat false;
        gen(lit,0,0);
        return;
    }
    else if(CurTok==tok_not){
        getNextToken(); // eat !
        boolfactor(my_lev);
        gen(opr,0,19);
        return;
    }
    else if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr(my_lev);
        if(CurTok!=tok_rparen){
            //log_error("missing right paren!");
            error(6);
            return;
        }
        else{
            getNextToken(); // eat )
        }

        return;
    }
    else if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            error(5);
            return;
        }
        int type=t.type;
        if(type==type_int){
            rel(my_lev);

            return;
        }
        else if(type==type_float){
            frel(my_lev);
            return;
        }
        else if(type==type_bool){
            getNextToken(); //eat id
            gen(lod,t.name_space,t.index); // 根据变量地址并将其值入栈
            return;
        }
        else{
            //log_error("undefined identifier!");
            error(5);
            return;
        }
    }
    else if(CurTok==tok_intnum){
        rel(my_lev);

        return;
    }
    else if(CurTok==tok_floatnum){
        frel(my_lev);

        return;
    }
    else if(CurTok==tok_odd){
        isOdd(my_lev);
        return;
    }
    else if (CurTok==tok_sub) {
        nega_rel(my_lev);
        return;
    }
    else{
        //log_error("unexcepted identifier");
        error(10);
        return;
    }
}

void isOdd(int my_lev){
    if(err>=maxerr){
        return;
    }
    if(CurTok!=tok_odd){
        error(9);
        return;
    }
    getNextToken(); //eat ODD
    intexpr(my_lev);
    gen(opr,0,28);
    return;
}

void rel(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            error(5);
            return;
        }
        int type=t.type;
        if(type==type_int){ //id is a number value
            getNextToken(); // eat id
            gen(lod,t.name_space,t.index); //找到变量地址并将值入栈

        }
        else if(type==type_iarr){
            getNextToken(); //eat id
            if(CurTok!=tok_lbracket){
                error(30);
                return;
            }
            getNextToken(); //eat [

            intexpr(my_lev);
            //gen(opr,0,29);//设置OFFSET

            if(CurTok!=tok_rbracket){
                error(27);
                return;
            }
            getNextToken(); //eat ]

            gen(old,t.name_space,t.index); //找到数组元素然后入栈


        }
        else{
            error(25);
            return;
        }

    }
    else if(CurTok==tok_intnum){
        int num=(int)NumVal;
        getNextToken(); // eat NUM
        gen(lit,0,num); //直接将当前立即数值入栈
    }

    int relop=CurTok;
    switch(CurTok){
        case tok_lss:{ // <
        getNextToken();//eat <
        break;
        }
        case tok_leq:{ // <=
        getNextToken(); // eat <=
        break;
        }
        case tok_gtr:{ // >
        getNextToken(); // eat >
        break;
        }
        case tok_geq:{ // >=
        getNextToken(); // eat >=
        break;
        }
        case tok_eql:{ // ==
        getNextToken(); // eat ==
        break;
        }
        case tok_neq:{ // !=
        getNextToken(); // eat !=
        break;
        }
        default:{
        //log_error("unexcepted token!");
        error(11);
        return;
        }
    }
    intexpr(my_lev);

    switch(relop){
        case tok_lss:{ // <
            gen(opr,0,10);
            break;
        }
        case tok_leq:{ // <=
            gen(opr,0,13);
            break;
        }
        case tok_gtr:{ // >
            gen(opr,0,12);
            break;
        }
        case tok_geq:{ // >=
            gen(opr,0,11);
            break;
        }
        case tok_eql:{ // ==
            gen(opr,0,8);
            break;
        }
        case tok_neq:{ // !=
            gen(opr,0,9);
            break;
        }
    }
    return;
}

void nega_rel(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok!=tok_sub){
        error(9);
        return;
    }
    getNextToken(); //eat -
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            error(5);
            return;
        }
        int type=t.type;
        if(type==type_int){ //id is a number value
            getNextToken(); // eat id
            gen(lod,t.name_space,t.index); //找到变量地址并将值入栈
            gen(opr,0,1); //取反
            int relop=CurTok;
            switch(CurTok){
                case tok_lss:{ // <
                getNextToken();//eat <
                break;
                }
                case tok_leq:{ // <=
                getNextToken(); // eat <=
                break;
                }
                case tok_gtr:{ // >
                getNextToken(); // eat >
                break;
                }
                case tok_geq:{ // >=
                getNextToken(); // eat >=
                break;
                }
                case tok_eql:{ // ==
                getNextToken(); // eat ==
                break;
                }
                case tok_neq:{ // !=
                getNextToken(); // eat !=
                break;
                }
                default:{
                //log_error("unexcepted token!");
                error(11);
                return;
                }
            }
            intexpr(my_lev);
            switch(relop){
                case tok_lss:{ // <
                    gen(opr,0,10);
                    break;
                }
                case tok_leq:{ // <=
                    gen(opr,0,13);
                    break;
                }
                case tok_gtr:{ // >
                    gen(opr,0,12);
                    break;
                }
                case tok_geq:{ // >=
                    gen(opr,0,11);
                    break;
                }
                case tok_eql:{ // ==
                    gen(opr,0,8);
                    break;
                }
                case tok_neq:{ // !=
                    gen(opr,0,9);
                    break;
                }
            }
            return;
        }
        else if(type==type_iarr){
            getNextToken(); // eat id
            if(CurTok!=tok_lbracket){
                error(30);
                return;
            }
            getNextToken(); //eat [

            intexpr(my_lev);
            //gen(opr,0,29);//设置OFFSET

            if(CurTok!=tok_rbracket){
                error(27);
                return;
            }
            getNextToken(); //eat ]

            gen(old,t.name_space,t.index); //找到数组元素然后入栈
            gen(opr,0,1); //取反
            int relop=CurTok;
            switch(CurTok){
                case tok_lss:{ // <
                getNextToken();//eat <
                break;
                }
                case tok_leq:{ // <=
                getNextToken(); // eat <=
                break;
                }
                case tok_gtr:{ // >
                getNextToken(); // eat >
                break;
                }
                case tok_geq:{ // >=
                getNextToken(); // eat >=
                break;
                }
                case tok_eql:{ // ==
                getNextToken(); // eat ==
                break;
                }
                case tok_neq:{ // !=
                getNextToken(); // eat !=
                break;
                }
                default:{
                //log_error("unexcepted token!");
                error(11);
                return;
                }
            }
            intexpr(my_lev);
            switch(relop){
                case tok_lss:{ // <
                    gen(opr,0,10);
                    break;
                }
                case tok_leq:{ // <=
                    gen(opr,0,13);
                    break;
                }
                case tok_gtr:{ // >
                    gen(opr,0,12);
                    break;
                }
                case tok_geq:{ // >=
                    gen(opr,0,11);
                    break;
                }
                case tok_eql:{ // ==
                    gen(opr,0,8);
                    break;
                }
                case tok_neq:{ // !=
                    gen(opr,0,9);
                    break;
                }
            }
            return;
        }
        else if(type==type_float){
            getNextToken(); // eat id
            gen(lod,t.name_space,t.index); //找到变量地址并将值入栈
            gen(opr,0,1); //取反
            int relop=CurTok;
            switch(CurTok){
                case tok_lss:{ // <
                getNextToken();//eat <
                break;
                }
                case tok_leq:{ // <=
                getNextToken(); // eat <=
                break;
                }
                case tok_gtr:{ // >
                getNextToken(); // eat >
                break;
                }
                case tok_geq:{ // >=
                getNextToken(); // eat >=
                break;
                }
                case tok_eql:{ // ==
                getNextToken(); // eat ==
                break;
                }
                case tok_neq:{ // !=
                getNextToken(); // eat !=
                break;
                }
                default:{
                //log_error("unexcepted token!");
                error(11);
                return;
                }
            }
            floatexpr(my_lev);
            switch(relop){
            case tok_lss:{ // <
                gen(opr,0,10);
                break;
            }
            case tok_leq:{ // <=
                gen(opr,0,13);
                break;
            }
            case tok_gtr:{ // >
                gen(opr,0,12);
                break;
            }
            case tok_geq:{ // >=
                gen(opr,0,11);
                break;
            }
            case tok_eql:{ // ==
                gen(opr,0,8);
                break;
            }
            case tok_neq:{ // !=
                gen(opr,0,9);
                break;
            }
            }
            return;
        }
        else if(type==type_farr){
            getNextToken(); // eat id
            if(CurTok!=tok_lbracket){
                error(30);
                return;
            }
            getNextToken(); //eat [

            intexpr(my_lev);
            //gen(opr,0,29);//设置OFFSET

            if(CurTok!=tok_rbracket){
                error(27);
                return;
            }
            getNextToken(); //eat ]

            gen(old,t.name_space,t.index); //找到数组元素然后入栈
            gen(opr,0,1); //取反
            int relop=CurTok;
            switch(CurTok){
                case tok_lss:{ // <
                getNextToken();//eat <
                break;
                }
                case tok_leq:{ // <=
                getNextToken(); // eat <=
                break;
                }
                case tok_gtr:{ // >
                getNextToken(); // eat >
                break;
                }
                case tok_geq:{ // >=
                getNextToken(); // eat >=
                break;
                }
                case tok_eql:{ // ==
                getNextToken(); // eat ==
                break;
                }
                case tok_neq:{ // !=
                getNextToken(); // eat !=
                break;
                }
                default:{
                //log_error("unexcepted token!");
                error(11);
                return;
                }
            }
            floatexpr(my_lev);
            switch(relop){
            case tok_lss:{ // <
                gen(opr,0,10);
                break;
            }
            case tok_leq:{ // <=
                gen(opr,0,13);
                break;
            }
            case tok_gtr:{ // >
                gen(opr,0,12);
                break;
            }
            case tok_geq:{ // >=
                gen(opr,0,11);
                break;
            }
            case tok_eql:{ // ==
                gen(opr,0,8);
                break;
            }
            case tok_neq:{ // !=
                gen(opr,0,9);
                break;
            }
            }
            return;
        }
        else{
            error(25);
            return;
        }
    }
    return;
}

void frel(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            error(5);
            return;
        }
        int type=t.type;
        if(type==type_float){ //id is a float value
            getNextToken(); // eat id
            gen(lod,t.name_space,t.index); //找到变量地址并将值入栈
        }
        else if(type==type_farr){
            getNextToken(); // eat id
            if(CurTok!=tok_lbracket){
                error(30);
                return;
            }
            getNextToken(); //eat [

            intexpr(my_lev);
            //gen(opr,0,29);//设置OFFSET

            if(CurTok!=tok_rbracket){
                error(27);
                return;
            }
            getNextToken(); //eat ]

            gen(old,t.name_space,t.index); //找到数组元素然后入栈
        }
        else{
            error(22);
            return;
        }

    }
    else if(CurTok==tok_floatnum){
        double num=(double)NumVal;
        getNextToken(); // eat NUM
        gen(lit,0,num); //直接将当前立即数值入栈
    }
    int relop=CurTok;
    switch(CurTok){
        case tok_lss:{ // <
        getNextToken();//eat <
        break;
        }
        case tok_leq:{ // <=
        getNextToken(); // eat <=
        break;
        }
        case tok_gtr:{ // >
        getNextToken(); // eat >
        break;
        }
        case tok_geq:{ // >=
        getNextToken(); // eat >=
        break;
        }
        case tok_eql:{ // ==
        getNextToken(); // eat ==
        break;
        }
        case tok_neq:{ // !=
        getNextToken(); // eat !=
        break;
        }
        default:{
        //log_error("unexcepted token!");
        error(11);
        return;
        }
    }
    floatexpr(my_lev);

    switch(relop){
        case tok_lss:{ // <
            gen(opr,0,10);
            break;
        }
        case tok_leq:{ // <=
            gen(opr,0,13);
            break;
        }
        case tok_gtr:{ // >
            gen(opr,0,12);
            break;
        }
        case tok_geq:{ // >=
            gen(opr,0,11);
            break;
        }
        case tok_eql:{ // ==
            gen(opr,0,8);
            break;
        }
        case tok_neq:{ // !=
            gen(opr,0,9);
            break;
        }
    }
    return;

}

/*
 * 解释程序
 */
void exeAll(){
    ferr=fopen("temp-log.txt","a");
    fprintf(ferr,"Start Execute\n");
    fclose(ferr);
    _exeAll();
    ferr=fopen("temp-log.txt","a");
    fprintf(ferr,"Finish Execute\n");
    fclose(ferr);
}
void _exeAll(){
    int len=cx;
    while(PC<len){
        _exeOne();
    }
}
int exeOne(){
    if(PC==0){
        ferr=fopen("temp-log.txt","a");
        fprintf(ferr,"Start Execute\n");
        fclose(ferr);
    }
    if(PC>=cx){
        ferr=fopen("temp-log.txt","a");
        fprintf(ferr,"Finish Execute\n");
        fclose(ferr);
        return 0;
    }
    else{
        _exeOne();
        printStack();
        return 1;
    }
}
void _exeOne(){
    int len=cx;
    if(PC<len){
        curCode=code[PC];
        PC++;
        switch(curCode.f){
            case lit: //入栈常量a
                s[SP]=curCode.a;
                SP++;
                break;
            case opr: //各种数学逻辑运算
                switch((int)curCode.a)
                {
                    case 1: //栈顶取反
                        s[SP-1]=-s[SP-1];
                        break;
                    case 2: //次栈顶加栈顶 退两个元素 结果入栈 (int add)
                        {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=a+b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 3: //次栈顶减栈顶 退两个元素 结果入栈 (int min)
                        {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=a-b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 4: //次栈顶乘栈顶 退两个元素 结果入栈 (int mul)
                        {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=a*b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 5: //次栈顶除以栈顶 退两个元素 结果入栈 (int div)
                        {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=a/b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 6: //栈顶元素奇偶判断, 栈顶出栈, 入栈判断结果
                        {
                        int res=(int)s[SP-1]%2;
                        s[SP-1]=res;
                        break;
                        }
                    case 8: //次栈顶与栈顶是否相等
                        {
                        double a=s[SP-2];
                        double b=s[SP-1];
                        int res=(a==b);
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 9: // !=
                        {
                        double a=s[SP-2];
                        double b=s[SP-1];
                        int res=(a!=b);
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 10: // <
                        {
                        double a=s[SP-2];
                        double b=s[SP-1];
                        int res=(a<b);
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 11: // >=
                        {
                        double a=s[SP-2];
                        double b=s[SP-1];
                        int res=(a>=b);
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 12: // >
                        {
                        double a=s[SP-2];
                        double b=s[SP-1];
                        int res=(a>b);
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 13: // <=
                        {
                        double a=s[SP-2];
                        double b=s[SP-1];
                        int res=(a<=b);
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 14: // 输出栈顶值 栈顶值出栈 write int
                        {
                        ferr=fopen("temp-log.txt","a");
                        fprintf(ferr,"%d\n",(int)s[SP-1]);
                        fclose(ferr);
                        SP--;
                        break;
                        }

                    case 16: //读入一个输入置于栈顶 read int
                        {

                        int res;
                        printf("please input a int:\n");
                        ferr=fopen("temp-log.txt","a");
                        fprintf(ferr,"input int ?\n");
                        fclose(ferr);
                        w->outputLog("请输入一个int");

                        loop.exec();
                        res=(int)w->getInput();
                        s[SP]=res;
                        SP++;
                        break;
                        }
                        //to-do 加入我补充的运算符
                    case 17: // && 对布尔类型做 与 结果置于栈顶
                    {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=0;
                        if(a==1&&b==1)
                            res=1;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                    }
                    case 18:// ||
                    {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=1;
                        if(a==0&&b==0)
                            res=0;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                    }
                    case 19: // 栈顶bool 取反
                    {
                        int res=(int)s[SP-1];
                        if(res==1){
                            res==0;
                        }
                        else{
                            res==1;
                        }
                        s[SP-1]=res;
                        break;
                    }
                    case 20: //次栈顶加栈顶 退两个元素 结果入栈 (float add)
                        {
                        double a=(double)s[SP-2];
                        double b=(double)s[SP-1];
                        double res=a+b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 21: //次栈顶减栈顶 退两个元素 结果入栈 (double min)
                        {
                        double a=(double)s[SP-2];
                        double b=(double)s[SP-1];
                        double res=a-b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 22: //次栈顶乘栈顶 退两个元素 结果入栈 (double mul)
                        {
                        double a=(double)s[SP-2];
                        double b=(double)s[SP-1];
                        double res=a*b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 23: //次栈顶除以栈顶 退两个元素 结果入栈 (double div)
                        {
                        double a=(double)s[SP-2];
                        double b=(double)s[SP-1];
                        double res=a/b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                        }
                    case 24: // 输出栈顶值 栈顶值出栈 write float
                        {
                        ferr=fopen("temp-log.txt","a");
                        fprintf(ferr,"%.2f\n",(double)s[SP-1]);
                        fclose(ferr);
                        SP--;
                        break;
                        }

                    case 25: //读入一个输入置于栈顶 read float
                        {
                        double res;
                        printf("please input a int:\n");
                        ferr=fopen("temp-log.txt","a");
                        fprintf(ferr,"input float ?\n");
                        fclose(ferr);
                        w->outputLog("请输入一个float");

                        loop.exec();
                        res=w->getInput();
                        s[SP]=res;
                        SP++;
                        break;
                        }
                    case 26: //次栈顶除以栈顶 余数入栈 int mod
                    {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int temp=a/b;
                        int res=a-temp*b;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                    }
                    case 27: // XOR 对布尔类型做 异或 结果置于栈顶
                    {
                        int a=(int)s[SP-2];
                        int b=(int)s[SP-1];
                        int res=0;
                        if(a!=b)
                            res=1;
                        SP=SP-1;
                        s[SP-1]=res;
                        break;
                    }
                    case 28: // 判断栈顶是不是奇数 是奇数1 不是奇数0 结果置于栈顶
                    {
                        int a=(int)s[SP-1];
                        s[SP-1]=(int)a%2;
                        break;
                    }
                    case 29: //复制栈顶
                    {
                        s[SP]=s[SP-1];
                        SP++;
                        break;
                    }
                    case 30: //退栈
                    {
                        SP--;
                        break;
                    }




                }
                break; //end of case opr
            case lod: //  lod namespace dx (load int/bool)
            //根据base 找到namespcae的基址 然后将 栈中对应位置值入栈
                {
                double val=s[base[curCode.l]+(int)curCode.a];
                s[SP]=val;
                SP++;
                break;
                }
            case sto: //sto namespace dx (store)
                {
                s[base[curCode.l]+(int)curCode.a] = s[SP-1];
                SP--;
                break;
                }
            case cal: //调用方程
                {
                break; //to-do 之后再来实现
                LR=PC;
                }
            case ini: //在数据栈中开辟a个单元
                {
                SP=SP+(int)curCode.a;
                break;
                }
            case jmp: //直接跳转
                {
                PC=(int)curCode.a;
                break;
                }
            case jpc: //条件跳转 如果栈顶是 0 跳转
                {
                int top=(int)s[SP-1];
                if(top==0){
                    PC=(int)curCode.a;
                }
                SP--;
                break;
                }
            case ssp: // ssp namespace  store curSP in to base[namespace]
                {
                base[curCode.l]=SP;
                break;
                }
            case lsp: // lsp namespace load base[namespace] into SP
                {
                SP=base[curCode.l];
                break;
                }
            case old:
            //根据栈顶的offset load
                {
                int offset=s[SP-1];
                SP--;
                double val=s[base[curCode.l]+(int)curCode.a+offset];
                s[SP]=val;
                SP++;
                break;
                }
            case ost:
            //根据次顶的offset store
                {
                int offset=s[SP-2];
                s[base[curCode.l]+(int)curCode.a+offset] = s[SP-1];
                SP-=2;
                break;
                }
        }
    }
}

void init(){

    ch=' ';
    line_count=0;
    cc=0,ll=0;
    precc=1;
    preline=1;
    memset(line,0,sizeof(char)*MAXLINE);
    flag=false;// flag == true if file ends
    err=0;
    maxerr=1;
    cx=0;
    PC=0;
    SP=0;
    LR=0;
    for(int i=0;i<maxnamespace;i++){
        tables[i].clear();
        upperNamespaces[i].clear();
    }
    memset(code,0,sizeof(struct instruction)*cxmax);
    memset(base,0,sizeof(int)*maxnamespace);
    memset(s,0,sizeof(double)*stacksize);
    cantFindName=0;
    namespacecount=-1;
    memset(dxs,0,sizeof(int)*maxnamespace);

    fillbackList_break.clear();
    fillbackList_break.resize(maxnamespace);
    fillbackList_continue.clear();
    fillbackList_continue.resize(maxnamespace);
    fillbackList_return.clear();
    fillbackList_return.resize(maxnamespace);
    fillbackList_exit.clear();
    nestbreak=-1;
    nestcontinue=-1;
    nestreturn=-1;

    //建立首符集
    Vntab.insert(pair<string,int>("program",0));
    Vntab.insert(pair<string,int>("block",1));
    Vntab.insert(pair<string,int>("decls",2));
    Vntab.insert(pair<string,int>("decl",3));
    Vntab.insert(pair<string,int>("stmts",4));
    Vntab.insert(pair<string,int>("stmt",5));
    Vntab.insert(pair<string,int>("intexpr",6));
    Vntab.insert(pair<string,int>("intterm",7));
    Vntab.insert(pair<string,int>("intfactor",8));
    Vntab.insert(pair<string,int>("boolexpr",9));
    Vntab.insert(pair<string,int>("boolexpr_",10));
    Vntab.insert(pair<string,int>("boolterm",11));
    Vntab.insert(pair<string,int>("boolterm_",12));
    Vntab.insert(pair<string,int>("boolfactor",13));
    Vntab.insert(pair<string,int>("rel",14));
    Vntab.insert(pair<string,int>("assign_stmt",15));
    Vntab.insert(pair<string,int>("if_stmt",16));
    Vntab.insert(pair<string,int>("while_stmt",17));
    Vntab.insert(pair<string,int>("write_stmt",18));
    Vntab.insert(pair<string,int>("read_stmt",19));
    Vntab.insert(pair<string,int>("floatexpr",20));
    Vntab.insert(pair<string,int>("floatterm",21));
    Vntab.insert(pair<string,int>("floatfactor",22));
    firsts[Vntab["program"]].push_back(tok_lbrace); //
    firsts[Vntab["block"]].push_back(tok_lbrace); //
    firsts[Vntab["decls"]].push_back(tok_bool); //
    firsts[Vntab["decls"]].push_back(tok_int);
    firsts[Vntab["decls"]].push_back(tok_float);
    firsts[Vntab["decls"]].push_back(tok_function);
    firsts[Vntab["decls"]].push_back(tok_identifier); // 加上stmt首符
    firsts[Vntab["decls"]].push_back(tok_selfadd);
    firsts[Vntab["decls"]].push_back(tok_selfmin);
    firsts[Vntab["decls"]].push_back(tok_if);
    firsts[Vntab["decls"]].push_back(tok_while);
    firsts[Vntab["decls"]].push_back(tok_for);
    firsts[Vntab["decls"]].push_back(tok_switch);
    firsts[Vntab["decls"]].push_back(tok_write);
    firsts[Vntab["decls"]].push_back(tok_read);
    firsts[Vntab["decls"]].push_back(tok_writef);
    firsts[Vntab["decls"]].push_back(tok_readf);
    firsts[Vntab["decls"]].push_back(tok_continue);
    firsts[Vntab["decls"]].push_back(tok_exit);
    firsts[Vntab["decls"]].push_back(tok_break);
    firsts[Vntab["decls"]].push_back(tok_lbrace);
    firsts[Vntab["decls"]].push_back(tok_rbrace); //加上program 结尾的}
    firsts[Vntab["decl"]].push_back(tok_bool); //
    firsts[Vntab["decl"]].push_back(tok_int);
    firsts[Vntab["decl"]].push_back(tok_float);
    firsts[Vntab["decl"]].push_back(tok_function);
    firsts[Vntab["stmts"]].push_back(tok_identifier); //
    firsts[Vntab["stmts"]].push_back(tok_selfadd);
    firsts[Vntab["stmts"]].push_back(tok_selfmin);
    firsts[Vntab["stmts"]].push_back(tok_if);
    firsts[Vntab["stmts"]].push_back(tok_while);
    firsts[Vntab["stmts"]].push_back(tok_for);
    firsts[Vntab["stmts"]].push_back(tok_switch);
    firsts[Vntab["stmts"]].push_back(tok_write);
    firsts[Vntab["stmts"]].push_back(tok_read);
    firsts[Vntab["stmts"]].push_back(tok_writef);
    firsts[Vntab["stmts"]].push_back(tok_readf);
    firsts[Vntab["stmts"]].push_back(tok_continue);
    firsts[Vntab["stmts"]].push_back(tok_exit);
    firsts[Vntab["stmts"]].push_back(tok_break);
    firsts[Vntab["stmts"]].push_back(tok_lbrace);
    firsts[Vntab["stmts"]].push_back(tok_rbrace); //加上program 结尾的}
    firsts[Vntab["stmt"]].push_back(tok_identifier); //
    firsts[Vntab["stmt"]].push_back(tok_selfadd);
    firsts[Vntab["stmt"]].push_back(tok_selfmin);
    firsts[Vntab["stmt"]].push_back(tok_identifier);
    firsts[Vntab["stmt"]].push_back(tok_if);
    firsts[Vntab["stmt"]].push_back(tok_while);
    firsts[Vntab["stmt"]].push_back(tok_for);
    firsts[Vntab["stmt"]].push_back(tok_switch);
    firsts[Vntab["stmt"]].push_back(tok_write);
    firsts[Vntab["stmt"]].push_back(tok_read);
    firsts[Vntab["stmt"]].push_back(tok_writef);
    firsts[Vntab["stmt"]].push_back(tok_readf);
    firsts[Vntab["stmt"]].push_back(tok_continue);
    firsts[Vntab["stmt"]].push_back(tok_exit);
    firsts[Vntab["stmt"]].push_back(tok_break);
    firsts[Vntab["stmt"]].push_back(tok_lbrace);
    firsts[Vntab["intexpr"]].push_back(tok_identifier); //
    firsts[Vntab["intexpr"]].push_back(tok_intnum);
    firsts[Vntab["intexpr"]].push_back(tok_sub);
    firsts[Vntab["intexpr"]].push_back(tok_lparen);
    firsts[Vntab["boolexpr"]].push_back(tok_identifier); //
    firsts[Vntab["boolexpr"]].push_back(tok_true);
    firsts[Vntab["boolexpr"]].push_back(tok_false);
    firsts[Vntab["boolexpr"]].push_back(tok_not);
    firsts[Vntab["boolexpr"]].push_back(tok_lparen);
    firsts[Vntab["boolexpr"]].push_back(tok_intnum);
    firsts[Vntab["boolexpr"]].push_back(tok_sub);
    firsts[Vntab["boolexpr"]].push_back(tok_floatnum);
    firsts[Vntab["boolexpr"]].push_back(tok_odd);

    firsts[Vntab["floatexpr"]].push_back(tok_identifier); //
    firsts[Vntab["floatexpr"]].push_back(tok_floatnum);
    firsts[Vntab["floatexpr"]].push_back(tok_sub);
    firsts[Vntab["floatexpr"]].push_back(tok_lparen);

    fstack=fopen("temp-stack.txt","w");
    fclose(fstack);

    IdentifierStr="";
    NumVal=0;
    getNextToken();
}

void listall(){
    for(int i=0;i<cx;i++){
        fprintf(fcode,"%d:\t%s %d %.2f\n",i, codeName[code[i].f].c_str() ,code[i].l,code[i].a);
    }
}

int compileCX(std::string file_in_name){
    strcpy(filename,file_in_name.data());
    filename[file_in_name.size()]=0;
    fin=fopen(filename,"r");
    ferr=fopen("temp-log.txt","w");
    fcode=fopen("temp-code.txt","w");
    init();
    program();
    if(err==0){
        fprintf(ferr,"Compile Complete With No Error!\n");
        printTable();
        listall();
    }
    fclose(fcode);
    fclose(fin);
    fclose(ferr);
    return err;
}

void printTable(){
    ftable=fopen("temp-table.txt","w");
    fprintf(ftable,"namespace\tname\ttype\tsize\tadr\t\n");
    for(int ns=0;ns<maxnamespace;ns++){
        for(int i=0;i<tables[ns].size();i++){
            fprintf(ftable,"%-9d\t%-8s%-8s%-8d%-8d\n",
            tables[ns][i].name_space,tables[ns][i].name.c_str(),typeName[tables[ns][i].type].c_str(),tables[ns][i].size,tables[ns][i].index);
        }
    }
    fclose(ftable);
}

void printStack(){
    fstack=fopen("temp-stack.txt","w");
    fprintf(fstack,"PC:%5d\nSP:%5d\nLR:%5d\n",PC,SP,LR);
    fprintf(fstack,"=====bottom=====\n");
    for(int i=0;i<SP;i++){
        fprintf(fstack,"%d:\t%.2lf\n",i,s[i]);
    }
    fclose(fstack);
}

//test my lexer and parser
//int main(){
// std::string file="./testcodes/testLexerInput.cx";
// int e=compileCX(file);
// if(e==0){
//     while(getchar()!='p'){
//        if(!exeOne()){
//            break;
//        }
//     }
// }
// return 0;
//}
