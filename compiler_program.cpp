#include "compiler_program.h"

using namespace std;

std::string IdentifierStr;
double NumVal;
int CurTok;
int ch=' ';
int line_count=0;
int cc,ll;
char line[MAXLINE];
bool flag;// flag == true if file ends
int* readbuffer;
int pread;//readbuffer pointer

int err=0;
int maxerr=1;

FILE* fin;
FILE* foutput;

int tx=0;
int cx=0;
struct instruction code[cxmax];
char mnemonic[fctnum][5]; //虚拟机代码指令名称

int namespacecount=-1;

// here is lexer
/*
 * get a char and skip all the spaces
 * read a line from the input file, store into line, read another line
 * when it is all used
 * called by getNextChar
 */
void getch(){
    if(cc==ll){
        if(EOF==fscanf(fin,"%c",&ch)){
            flag = true;
            return;
        }
        line[0] = ch;
        fprintf(foutput,"%c",ch);
        ll=1;
        cc=0;
        ch = ' ';
        while(ch!=10){
            if(EOF==fscanf(fin,"%c",&ch)){
                line[ll]=0;
                break;
            }
            //printf("%c",LastChar);
            fprintf(foutput,"%c",ch);
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
void log_error(std::string msg){
    cout << msg << endl;
}

/*
 * 生成虚拟机代码
 * x: f
 * y: l;
 * z: a;
 */
void gen(enum fct x,int y,int z){
    if(cx>=cxmax){
        log_error("Program is too long!");
        exit(1);
    }
    if(z>=amax){
        log_error("Displacement address is too big!");
        exit(1);
    }
    code[cx].f=x;
    code[cx].l=y;
    code[cx].a=z;
    cx++;
}

void error(int n){
    if(err > maxerr){
        return;
    }
    char space[81];
    memset(space,32,81);
    space[cc-1] = 0;
    switch(n){
        case 0:
            fprintf(foutput,"%s^err%d: Lack left brace {\n",space,n);
            break;
        case 1:
            fprintf(foutput,"%s^err%d: Lack right brace }\n",space,n);
            break;
        case 2:
            fprintf(foutput,"%s^err%d: Expect a semicolon ;\n",space,n);
            break;
        case 3:
            fprintf(foutput,"%s^err%d: Expect an identifier! \n",space,n);
            break;
        case 4:
            fprintf(foutput,"%s^err%d: Unexpected data type!\n",space,n);
            break;
        case 5:
            fprintf(foutput,"%s^err%d: Undeclared identifier!\n",space,n);
            break;
        case 6:
            fprintf(foutput,"%s^err%d: Missing right paren ) !\n",space,n);
            break;
        case 7:
            fprintf(foutput,"%s^err%d: Missing left paren ( !\n",space,n);
            break;
        case 8:
            fprintf(foutput,"%s^err%d: Unknown token !\n",space,n);
            break;
        case 9:
            fprintf(foutput,"%s^err%d: Unexpected token !\n",space,n);
            break;
        case 10:
            fprintf(foutput,"%s^err%d: Unexpected identifier !\n",space,n);
            break;
        case 11:
            fprintf(foutput,"%s^err%d: Expect a relation operator here !\n",space,n);
            break;
        case 12:
            fprintf(foutput,"%s^err%d: Expect a number in a relation expression !\n",space,n);
            break;
        case 13:
            fprintf(foutput,"%s^err%d: Too many blocks !\n",space,n);
            break;
        case 50:
            fprintf(foutput,"%s^err%d: Failed to pass the test function\n",space,n);
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
    tx=0;
    block(0);
    //fprintf(foutput,"\n===parsed a program!===\n");
}

void block(int upper_namespace){ //lev 是上级namespace
    if(namespacecount<maxnamespace-1){
        namespacecount++;
    }
    else{
        error(13);
        return;
    }
    int my_lev=namespacecount;
    new_namespace(upper_namespace,my_lev);
    if(CurTok==tok_lbrace){
        getNextToken(); // eat {
        decls(my_lev);
        stmts(my_lev);
        if(CurTok==tok_rbrace){
            getNextToken();
            //fprintf(foutput,"\n===parsed a block!===\n");
            return;
        }
        else{
            log_error("missing right at the end of a block!");
            error(1);
            return;
        }
    }
    else{
        log_error("missing left brace at the start of a block!");
        error(0);
    }
}

void decls(int my_lev){ // 参数为自己的namespace
    while(CurTok==tok_int||CurTok==tok_bool){
        decl(my_lev);
    }
    //fprintf(foutput,"\n===parsed a decls!===\n");
    return;
}

void decl(int my_lev){
    if(CurTok==tok_int){
        getNextToken();
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            enter(my_lev,type_uint,id,0,0);
            getNextToken();
            if(CurTok==tok_semicolon){
                getNextToken();
                //fprintf(foutput,"\n===parsed a decl!===\n");
                return;
            }
            else{
                log_error("missing ; in the end of decalaration!");
                error(2);
            }
        }
        else{
            log_error("missing identifier in decalaration!");
            error(3);
        }
    }
    else if (CurTok==tok_bool){
        getNextToken();
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            enter(my_lev,type_bool,id,0,0);
            getNextToken();
            if(CurTok==tok_semicolon){
                getNextToken();
                //fprintf(foutput,"\n===parsed a decl!===\n");
                return;
            }
            else{
                log_error("missing ; in the end of decalaration!");
                error(2);
            }
        }
        else{
            log_error("missing identifier in decalaration!");
            error(3);
        }
    }
    else{
        log_error("unknown data type in declaration!");
        error(4);
    }
}

void stmts(int my_lev){
    while(CurTok==tok_identifier||CurTok==tok_if||CurTok==tok_while||
           CurTok==tok_write||CurTok==tok_read||CurTok==tok_lbrace){
        stmt(my_lev);
    }
    //fprintf(foutput,"\n===parsed a stmts!===\n");
    return;
}


// 按name和namespace查找,如果找不到,报错
// 这里有点乱,需要仔细检查
struct tablestruct& getTablestructById(int name_space,std::string name){
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
    error(5);
}

//to-do gencode
void assign_stmt(int my_lev){
    //to-do check the table to find out if the id is decalred and the data type of the id
    std::string id=IdentifierStr;
    int type=getTypeById(id);
    //        if(type==-1){
    //            log_error("undeclared identifier!");
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
                log_error("undeclared identifier!");
                error(5);
            }
        }
        if(CurTok==tok_semicolon){
            getNextToken();//eat ;
            //fprintf(foutput,"\n===parsed a assigm_stmt!===\n");
            return;
        }
        else{
            log_error("missing ; at the end of expression");
            error(2);
        }
    }
    return;
}

void if_stmt(int my_lev){
    getNextToken();//eat if
    if(CurTok==tok_lparen){
        getNextToken();//eat (
        boolexpr(my_lev);
        if(CurTok==tok_rparen){
            getNextToken();//eat )
            stmt(my_lev);
            if(CurTok==tok_else){
                getNextToken(); // eat else
                stmt(my_lev);
                //fprintf(foutput,"\n===parsed a if_stmt!===\n");
                return;
            }else{
                //fprintf(foutput,"\n===parsed a if_stmt!===\n");
                return;
            }
        }
        else{
            log_error("missing right paren!");
            error(6);
        }
    }
    else{
        log_error("missing left paren after if!");
        error(7);
    }
}

void while_stmt(int my_lev){
    getNextToken(); //eat while
    if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr(my_lev);
        if(CurTok==tok_rparen){
            getNextToken(); //eat )
            stmt(my_lev);
            //fprintf(foutput,"\n===parsed a while_stmt!===\n");
            return;
        }else{
            log_error("missing right paren!");
            error(6);
        }
    }
    else{
        log_error("missing left paren after while!");
        error(7);
    }
}

void write_stmt(int my_lev){
    getNextToken(); //eat write
    intexpr(my_lev);
    if(CurTok!=tok_semicolon){
        log_error("missing ; at the end of statement");
        error(2);
    }
    else{
        getNextToken(); //eat ;
    }
    //fprintf(foutput,"\n===parsed a write_stmt!===\n");
    return;
}

void read_stmt(int my_lev){
    getNextToken(); //eat read
    getNextToken(); // get the identifier
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        int type = getTypeById(id);
        // to-do check the type and do something
        getNextToken(); // eat id
        if(CurTok==tok_semicolon){
            getNextToken(); // eat ;
            //fprintf(foutput,"\n===parsed a read_stmt!===\n");
            return;
        }
        else{
            log_error("missing ;");
            error(2);
        }
    }
    else{
        log_error("expecting an identifire but receive other token!");
        error(3);
    }
}


void stmt(int my_lev){
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
        log_error("unknown token when expecting a statement!");
        error(8);
        break;
    }
    //fprintf(foutput,"\n===parsed a stmt!===\n");
}

void intexpr(int my_lev){
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
            //fprintf(foutput,"\n===parsed a intexpr!===\n");
            return;
        }
    }
}

void intterm(int my_lev){
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
        //fprintf(foutput,"\n===parsed a intterm!===\n");
        return;
    }
    }
}

void intfactor(int my_lev){
    switch(CurTok){
        case tok_identifier:
        {
            std::string id=IdentifierStr;
            int position=getIndexById(id);
            if(position==-1){
                error(5);
                return;
            }
            int type=table[position].type;
            if(type!=type_uint){
                error(4);
                return;
            }
            gen(lod,0,table[position].adr); //找到变量地址值并入栈
            getNextToken(); // eat id
            //fprintf(foutput,"\n===parsed a intfactor!===\n");

            return;
        }



        case tok_uintnum: //因子是一个立即数
        {
            int num = (int)NumVal;
            gen(lit,0,num);
            getNextToken(); //eat NUM
            //fprintf(foutput,"\n===parsed a intfactor!===\n");

            return;
        }
        case tok_lbrace: //因子是一个表达式
        {
            getNextToken(); //eat (
            intexpr(my_lev);
            if(CurTok==tok_rbrace){
                getNextToken();//eat )
                //fprintf(foutput,"\n===parsed a intfactor!===\n");

                return;
            }
            else{
                log_error("missing right brace!");
                error(1);
            }
            break;
        }
        default:
        {
            log_error("unexpected token!");
            error(9);
        }
    }
}

void boolexpr(int my_lev){
    boolterm(my_lev);
    boolexpr_(my_lev);
    //fprintf(foutput,"\n===parsed a boolexpr!===\n");
}

void boolexpr_(int my_lev){
    if(CurTok==tok_or){
        getNextToken(); //eat ||
        boolterm(my_lev);
        boolexpr_(my_lev);
        gen(opr,0,18);
    }
    else{
        return;
    }
    //fprintf(foutput,"\n===parsed a boolexpr_!===\n");
    return;
}

void boolterm(int my_lev){
    boolfactor(my_lev);
    boolterm_(my_lev);
    //fprintf(foutput,"\n===parsed a boolterm!===\n");

}

void boolterm_(int my_lev){
    if(CurTok==tok_and){
        getNextToken(); //eat &&
        boolfactor(my_lev);
        boolterm_(my_lev);
        gen(opr,0,17);
    }
    else{
        return;
    }
    //fprintf(foutput,"\n===parsed a boolterm_!===\n");
    return;
}

void boolfactor(int my_lev){
    if(CurTok==tok_true){
        getNextToken(); //eat true;
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");
        gen(lit,0,1);
        return;
    }
    else if(CurTok==tok_false){
        getNextToken(); //eat false;
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");
        gen(lit,0,0);
        return;
    }
    else if(CurTok==tok_not){
        getNextToken(); // eat !
        boolfactor(my_lev);
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");
        gen(opr,0,19);
        return;
    }
    else if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr(my_lev);
        if(CurTok!=tok_rparen){
            log_error("missing right paren!");
            error(6);
        }
        else{
            getNextToken(); // eat )
        }
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");

        return;
    }
    else if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        int position=getIndexById(id);
        int type=table[position].type;
        if(type==type_uint){
            rel(my_lev);
            //fprintf(foutput,"\n===parsed a boolfactor!===\n");

            return;
        }
        else if(type==type_bool){
            getNextToken(); //eat id
            gen(lod,lev-table[position].level,table[position].adr); // 根据变量地址并将其值入栈
            //fprintf(foutput,"\n===parsed a boolfactor!===\n");

            return;
        }
        else{
            log_error("undefined identifier!");
            error(5);
        }
    }
    else if(CurTok==tok_uintnum){
        rel(my_lev);
        // //fprintf(foutput,"\n===parsed a boolfactor!===\n");

        return;
    }
    else{
        log_error("unexcepted identifier");
        error(10);
    }
}

void rel(int my_lev){
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        int i=getIndexById(id);
        int type=table[i].type;
        if(type==type_uint){ //id is a number value
            gen(lod,lev-table[i].level,table[i].adr); //找到变量地址并将值入栈
        }
        else{
            error(12);
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
        log_error("unexcepted token!");
        error(11);
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
    // //fprintf(foutput,"\n===parsed a rel!===\n");

}

//test my lexer and parser
//int main(){
//   fin= fopen("testLexerInput.txt","r");
//   foutput = fopen("testLexerOutput.txt","w");
//   IdentifierStr="";
//   NumVal=0;
//   getNextToken();
//   program();
//   cout<< err << endl;
//   fclose(fin);
//   fclose(foutput);
//   return 0;
//}
