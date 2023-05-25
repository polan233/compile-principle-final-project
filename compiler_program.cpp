#include "compiler_program.h"
#include <map>
#include <vector>
#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>
#include <memory>
#include <cstring>
using namespace std;

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
int* readbuffer;
int pread;//readbuffer pointer

int err=0;
int maxerr=1;

FILE* fin;
FILE* ferr;
FILE* fcode;
char filename[MAXLINE];

int cx=0;
struct instruction code[cxmax];
std::string codeName[fctnum]={"lit","opr","lod","sto","cal","int","jmp","jpc","ssp","lsp"};

int namespacecount=-1;

//program namespace为0
//别的block 依次递增
//upperNamespaces表示 block的上层namespace
struct tablestruct temp_tablestruct;
// 符号表
std::vector<struct tablestruct> tables[maxnamespace];
std::vector< std::vector<int> > upperNamespaces(maxnamespace,std::vector<int>(0)); //表示namespace的依赖 upperNamespaces[namespace] 中存namespace的上级命名空间
int cantFindName;

map<std::string,int> Vntab;
vector<vector<int>> firsts(Vn_count,std::vector<int>(0));

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
        if(IdentifierStr=="true"){
            return tok_true;
        }
        if(IdentifierStr=="false"){
            return tok_false;
        }

        return tok_identifier;
    }

    if(isdigit(ch)){ // unsigned int NUM
        std::string NumStr;
        do{
            NumStr += ch;
            getch();
        }while(isdigit(ch));
        NumVal = stoi(NumStr);
        return tok_uintnum;
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

    if(ch=='+'){
        getch();
        return tok_add;
    }
    if(ch=='-'){
        getch();
        return tok_sub;
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
void gen(enum fct x,int y,int z){
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
    fprintf(ferr,"==========Compile Failed With Error==========\n",preline);
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
            fprintf(ferr,"line%d:%d err%d: Expect a number in a relation expression !\n",preline,precc-1,n);
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
    }
    err = err + 1;
}


void enter(int name_space,int type,std::string name,int size,double val){
    temp_tablestruct.name=name;
    temp_tablestruct.size=size;
    temp_tablestruct.val=val;
    temp_tablestruct.type=type;
    temp_tablestruct.name_space=name_space;
    temp_tablestruct.index=tables[name_space].size();
    tables[name_space].push_back(temp_tablestruct);
}

void program(){
    if(test("program")){
        error(15);
        return;
    }
    block(0);
    //fprintf(ferr,"\n===parsed a program!===\n");
}

void block(int upper_namespace){ //lev 是上级namespace
    if(test("block")){
        error(16);
        return;
    }
    if(err >= maxerr){
        return;
    }
    if(namespacecount<maxnamespace-1){
        namespacecount++;
    }
    else{
        error(13);
        return;
    }
    int my_lev=namespacecount;
    new_namespace(upper_namespace,my_lev);
    gen(ssp,my_lev,0);
    if(CurTok==tok_lbrace){
        getNextToken(); // eat {
        decls(my_lev);
        stmts(my_lev);
        if(CurTok==tok_rbrace){
            getNextToken();
            gen(lsp,my_lev,0);
            return;
        }
        else{
            //log_error("missing right at the end of a block!");
            error(1);
            return;
        }
    }
    else{
        //log_error("missing left brace at the start of a block!");
        error(0);
        return;
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
    while(CurTok==tok_int||CurTok==tok_bool){
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
        getNextToken();
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            gen(ini,0,1);
            enter(my_lev,type_uint,id,0,0);
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
    else if (CurTok==tok_bool){
        getNextToken();
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            gen(ini,0,1);
            enter(my_lev,type_bool,id,0,0);
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
    else{
        //log_error("unknown data type in declaration!");
        error(4);
        return;
    }
}

void stmts(int my_lev){
    if(test("stmts")){
        error(18);
        return;
    }
    if(err >= maxerr){
        return;
    }
    while(CurTok==tok_identifier||CurTok==tok_if||CurTok==tok_while||
           CurTok==tok_write||CurTok==tok_read||CurTok==tok_lbrace){
        if(err >= maxerr){
            return;
        }
        stmt(my_lev);
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
        for(int j=0;j<tables[searchns].size();j++){
            if(tables[searchns][j].name==name){
                return tables[searchns][j]; //找到了
            }
        }
    }
    cantFindName=1;
    error(5);
    return temp_tablestruct; //随便return一个反正出错了
}


void assign_stmt(int my_lev){
    if(err >= maxerr){
        return;
    }
    std::string id=IdentifierStr;
    struct tablestruct t=getTablestructById(my_lev,id);
    if(cantFindName){
        return;
    }
    int type=t.type;

    //        if(type==-1){
    //            //log_error("undeclared identifier!");
    //        }
    getNextToken();//eat id;
    if(CurTok==tok_assign){
        getNextToken();//eat =
        switch(type){
            case type_uint:{
                intexpr(my_lev);
                break;
            }
            case type_bool:{
                boolexpr(my_lev);
                break;
            }
            default:{
                //log_error("undeclared identifier!");
                error(5);
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
    return;
}


void if_stmt(int my_lev){
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
            stmt(my_lev);
            //这里的是 true-stmt,先不急gencode,需要在看了有没有else才能做
            if(CurTok==tok_else){
                //有else
                getNextToken(); // eat else
                cx1=cx;
                gen(jmp,0,0); // jmp L1
                code[cx0].a=cx; //接下来是false-stmt的code,我们让上面的jpc跳转到这里,没毛病嗷~~
                stmt(my_lev);
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


void while_stmt(int my_lev){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat while
    int l1=cx;
    if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr(my_lev);
        int cx0=cx;
        gen(jpc,0,0); //跳出循环
        if(CurTok==tok_rparen){
            getNextToken(); //eat )
            stmt(my_lev);
            gen(jmp,0,l1); //回到while开始
            code[cx0].a=cx; //让jpc 跳出循环
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


void write_stmt(int my_lev){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat write
    intexpr(my_lev);
    gen(opr,0,14); //输出栈顶的值,也就是刚才的intexpr的值;
    gen(opr,0,15); //输出换行符;
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


void read_stmt(int my_lev){
    if(err >= maxerr){
        return;
    }
    getNextToken(); //eat read
    getNextToken(); // get the identifier
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            return;
        }
        int type=t.type;
        if(type!=type_uint){
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


void stmt(int my_lev){
    if(test("stmt")){
        error(18);
        return;
    }
    if(err >= maxerr){
        return;
    }
    switch(CurTok){
    case tok_identifier:{
        assign_stmt(my_lev);
        break;
    }
    case tok_if:{
        if_stmt(my_lev);
        break;
    }
    case tok_while:{
        while_stmt(my_lev);
        break;
    }
    case tok_write:{
        write_stmt(my_lev);
        break;
    }
    case tok_read:{
        read_stmt(my_lev);
        break;
    }
    case tok_lbrace:{
        block(my_lev);
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
            gen(opr,0,1); //生成减法指令
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
        case tok_identifier:
        {
            std::string id=IdentifierStr;
            struct tablestruct t=getTablestructById(my_lev,id);
            if(cantFindName){
                return;
            }
            int type=t.type;
            if(type!=type_uint){
                error(4);
                return;
            }
            gen(lod,t.name_space,t.index); //找到变量值并入栈
            getNextToken(); // eat id
            return;
        }



        case tok_uintnum: //因子是一个立即数
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
            return;
        }
        int type=t.type;
        if(type==type_uint){
            rel(my_lev);

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
    else if(CurTok==tok_uintnum){
        rel(my_lev);

        return;
    }
    else{
        //log_error("unexcepted identifier");
        error(10);
        return;
    }
}

void rel(int my_lev){
    if(err >= maxerr){
        return;
    }
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        struct tablestruct t=getTablestructById(my_lev,id);
        if(cantFindName){
            return;
        }
        int type=t.type;
        if(type==type_uint){ //id is a number value
            gen(lod,t.name_space,t.index); //找到变量地址并将值入栈
        }
        else{
            error(12);
            return;
        }
        getNextToken(); // eat id
    }
    else if(CurTok==tok_uintnum){
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

/*
 * 解释程序
 */
void interpret(){
    int len=cx;
    int PC=0;
    int LR=0;
    int SP=0; //空栈 SP指向下一个入栈口 栈顶元素为 SP-1
    int base[maxnamespace]={0};
    struct instruction curCode; //当前指令
    double s[stacksize]={0}; //栈
    printf("start execute cx!\n");
    do{
        curCode=code[PC];
        PC++;
        switch(curCode.f){
            case lit: //入栈常量a
                s[SP]=curCode.a;
                SP++;
                break;
            case opr: //各种数学逻辑运算
                switch(curCode.a)
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
                    //to-do 改成从qt的ui读入
                        {
                        printf("%d",s[SP-1]);
                        SP--;
                        break;
                        }
                    case 15: // 输出换行符
                        {
                        printf("\n");
                        break;
                        }
                    case 16: //读入一个输入置于栈顶 read int
                        {
                        //to-do 现在是从标准输入流输入 之后改成从qt前端ui输入
                        int res;
                        printf("please input a int:");
                        scanf("%d",&res);
                        s[SP]=res;
                        SP++;
                        break;
                        }
                        //to-do 加入我补充的运算符

                }
                break; //end of case opr
            case lod: //  lod namespace dx (load int/bool)
            //根据base 找到namespcae的基址 然后将 栈中对应位置值入栈
                {
                int val=(int)s[base[curCode.l]+curCode.a];
                s[SP]=val;
                SP++;
                break;
                }
            case sto: //sto namespace dx (store)
                {
                s[base[curCode.l]+curCode.a] = s[SP-1];
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
                SP=SP+curCode.a;
                break;
                }
            case jmp: //直接跳转
                {
                PC=curCode.a;
                break;
                }
            case jpc: //条件跳转 如果栈顶是 0 跳转
                {
                int top=(int)s[SP-1];
                if(top==0){
                    PC=curCode.a;
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
        }
    }while(PC<len);
    printf("end cx!\n");
}

void init(){
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
    firsts[Vntab["program"]].push_back(tok_lbrace); //
    firsts[Vntab["block"]].push_back(tok_lbrace); //
    firsts[Vntab["decls"]].push_back(tok_bool); //
    firsts[Vntab["decls"]].push_back(tok_int);
    firsts[Vntab["decls"]].push_back(tok_identifier); // 加上stmt首符
    firsts[Vntab["decls"]].push_back(tok_if);
    firsts[Vntab["decls"]].push_back(tok_while);
    firsts[Vntab["decls"]].push_back(tok_write);
    firsts[Vntab["decls"]].push_back(tok_read);
    firsts[Vntab["decls"]].push_back(tok_lbrace);
    firsts[Vntab["decls"]].push_back(tok_rbrace); //加上program 结尾的}
    firsts[Vntab["decl"]].push_back(tok_bool); //
    firsts[Vntab["decl"]].push_back(tok_int);
    firsts[Vntab["stmts"]].push_back(tok_identifier); //
    firsts[Vntab["stmts"]].push_back(tok_if);
    firsts[Vntab["stmts"]].push_back(tok_while);
    firsts[Vntab["stmts"]].push_back(tok_write);
    firsts[Vntab["stmts"]].push_back(tok_read);
    firsts[Vntab["stmts"]].push_back(tok_lbrace);
    firsts[Vntab["stmts"]].push_back(tok_rbrace); //加上program 结尾的}
    firsts[Vntab["stmt"]].push_back(tok_identifier); //
    firsts[Vntab["stmt"]].push_back(tok_if);
    firsts[Vntab["stmt"]].push_back(tok_while);
    firsts[Vntab["stmt"]].push_back(tok_write);
    firsts[Vntab["stmt"]].push_back(tok_read);
    firsts[Vntab["stmt"]].push_back(tok_lbrace);
    firsts[Vntab["intexpr"]].push_back(tok_identifier); //
    firsts[Vntab["intexpr"]].push_back(tok_uintnum);
    firsts[Vntab["intexpr"]].push_back(tok_lparen);
    firsts[Vntab["boolexpr"]].push_back(tok_identifier); //
    firsts[Vntab["boolexpr"]].push_back(tok_true);
    firsts[Vntab["boolexpr"]].push_back(tok_false);
    firsts[Vntab["boolexpr"]].push_back(tok_not);
    firsts[Vntab["boolexpr"]].push_back(tok_lparen);
    firsts[Vntab["boolexpr"]].push_back(tok_uintnum);

    IdentifierStr="";
    NumVal=0;
    getNextToken();
}

int compileCX(std::string file_in_name){
    strcpy(filename,file_in_name.data());
    filename[file_in_name.size()]=0;
    fin=fopen(filename,"r");
    ferr=fopen("temp-log.txt","w");
    init();
    program();
    if(err==0){
        fprintf(ferr,"=============Compile Finish!============\n");
    }
    fclose(fin);
    fclose(ferr);
    return err;
}

//test my lexer and parser
int main(){
   std::string file="./testcodes/test-empty.cx";
   int e=compileCX(file);
   if(e==0){
        interpret();
   }
   return 0;
}
