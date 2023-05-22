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
int maxerr=30;

FILE* fin;
FILE* foutput;

int tx=0;


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

//my parser
void log_error(std::string msg){
    cout << msg << endl;
}

void error(int n){
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
            fprintf(foutput,"%s^err%d: Unknown data type!\n",space,n);
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
        case 50:
            fprintf(foutput,"%s^err%d: Failed to pass the test function\n",space,n);
            break;
    }


    err = err + 1;
    if(err > maxerr){
        exit(286);
    }
}

void declare(int type,std::string id){
    table[tx].name=id;
    table[tx].type=type;
    tx++;
    return;
}

void program(){
    block();
    //fprintf(foutput,"\n===parsed a program!===\n");
}

void block(){
    if(CurTok==tok_lbrace){
        getNextToken(); // eat {
        decls();
        stmts();
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
        decls();
        stmts();
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
}

void decls(){
    while(CurTok==tok_int||CurTok==tok_bool){
        decl();
    }
    //fprintf(foutput,"\n===parsed a decls!===\n");
    return;
}

void decl(){
    if(CurTok==tok_int){
        getNextToken();
        if(CurTok==tok_identifier){
            std::string id= IdentifierStr;
            declare(type_uint,id);
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
            declare(type_bool,id);
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

void stmts(){
    while(CurTok==tok_identifier||CurTok==tok_if||CurTok==tok_while||
           CurTok==tok_write||CurTok==tok_read||CurTok==tok_lbrace){
        stmt();
    }
    //fprintf(foutput,"\n===parsed a stmts!===\n");
    return;
}

int getTypeById(std::string id){
    //to-do implement this function
    // return the type of the identifier
    // return -1 if its not in the table
    for(int i=0;i<tx;i++){
        if(table[i].name==id){
            return table[i].type;
        }
    }
    return -1;
}

void assign_stmt(){
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
                intexpr();
                break;
            }
            case type_bool:{
                boolexpr();
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

void if_stmt(){
    getNextToken();//eat if
    if(CurTok==tok_lparen){
        getNextToken();//eat (
        boolexpr();
        if(CurTok==tok_rparen){
            getNextToken();//eat )
            stmt();
            if(CurTok==tok_else){
                getNextToken(); // eat else
                stmt();
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

void while_stmt(){
    getNextToken(); //eat while
    if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr();
        if(CurTok==tok_rparen){
            getNextToken(); //eat )
            stmt();
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

void write_stmt(){
    getNextToken(); //eat write
    intexpr();
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

void read_stmt(){
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


void stmt(){
    switch(CurTok){
    case tok_identifier:{
        assign_stmt();
        break;
    }
    case tok_if:{
        if_stmt();
        break;
    }
    case tok_while:{
        while_stmt();
        break;
    }
    case tok_write:{
        write_stmt();
        break;
    }
    case tok_read:{
        read_stmt();
        break;
    }
    case tok_lbrace:{
        block();
        break;
    }
    default:
        log_error("unknown token when expecting a statement!");
        error(8);
        break;
    }
    //fprintf(foutput,"\n===parsed a stmt!===\n");
}

void intexpr(){
    intterm();
    switch(CurTok){
        case tok_add:{
            getNextToken(); // eat +
            intterm();
            break;
        }
        case tok_sub:{
            getNextToken(); // eat -
            intterm();
            break;
        }
        default:{
            //fprintf(foutput,"\n===parsed a intexpr!===\n");
            return;
        }
    }
}

void intterm(){
    intfactor();
    switch(CurTok){
    case tok_mul:{
        getNextToken(); // eat *
        intfactor();
        break;
    }
    case tok_div:{
        getNextToken(); // eat /
        intfactor();
        break;
    }
    default:{
        //fprintf(foutput,"\n===parsed a intterm!===\n");
        return;
    }
    }
}

void intfactor(){
    switch(CurTok){
        case tok_identifier:
        {
            std::string id=IdentifierStr;
            int type=getTypeById(id);

            getNextToken(); // eat id
            //fprintf(foutput,"\n===parsed a intfactor!===\n");

            return;
        }



        case tok_uintnum:
        {
            int num = (int)NumVal;

            getNextToken(); //eat NUM
            //fprintf(foutput,"\n===parsed a intfactor!===\n");

            return;
        }
        case tok_lbrace:
        {
            getNextToken(); //eat (
            intexpr();
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

void boolexpr(){
    boolterm();
    boolexpr_();
    //fprintf(foutput,"\n===parsed a boolexpr!===\n");
}

void boolexpr_(){
    while(CurTok==tok_or){
        getNextToken(); //eat ||
        boolterm();
    }
    //fprintf(foutput,"\n===parsed a boolexpr_!===\n");
    return;
}

void boolterm(){
    boolfactor();
    boolterm_();
    //fprintf(foutput,"\n===parsed a boolterm!===\n");

}

void boolterm_(){
    while(CurTok==tok_and){
        getNextToken(); //eat &&
        boolfactor();
    }
    //fprintf(foutput,"\n===parsed a boolterm_!===\n");
    return;
}

void boolfactor(){
    if(CurTok==tok_true){
        getNextToken(); //eat true;
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");
        return;
    }
    else if(CurTok==tok_false){
        getNextToken(); //eat false;
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");

        return;
    }
    else if(CurTok==tok_not){
        getNextToken(); // eat !
        boolfactor();
        //fprintf(foutput,"\n===parsed a boolfactor!===\n");

        return;
    }
    else if(CurTok==tok_lparen){
        getNextToken(); // eat (
        boolexpr();
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
        int type=getTypeById(id);
        if(type==type_uint){
            rel();
            //fprintf(foutput,"\n===parsed a boolfactor!===\n");

            return;
        }
        else if(type==type_bool){
            getNextToken(); //eat id
            //fprintf(foutput,"\n===parsed a boolfactor!===\n");

            return;
        }
        else{
            log_error("undefined identifier!");
            error(5);
        }
    }
    else if(CurTok==tok_uintnum){
        rel();
        // //fprintf(foutput,"\n===parsed a boolfactor!===\n");

        return;
    }
    else{
        log_error("unexcepted identifier");
        error(10);
    }
}

void rel(){
    if(CurTok==tok_identifier){
        std::string id=IdentifierStr;
        int type=getTypeById(id);

        getNextToken(); // eat id
    }
    else if(CurTok==tok_uintnum){
        int num=(int)NumVal;
        getNextToken(); // eat NUM
    }
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
        error(9);
        }
    }
    intexpr();
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
