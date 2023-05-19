#include "compiler_program.h"

using namespace std;

std::string IdentifierStr;
int NumVal;
int CurTok;
int ch=' ';
int cc,ll;
char line[MAXLINE];
bool flag;// flag == true if file ends
int* readbuffer;
int pread;//readbuffer指针

FILE* fin;
FILE* foutput;

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

    }
    ch=line[cc];
    cc++;
}


// my lexer
int gettok(){
    if(flag){ // the file ends
        printf("program incomplete!");
        return tok_illegel;
    }

    //skip whitespaces and tab and huan hang
    while(isspace(ch)||ch==10||ch==9){
        getch();
        if(flag){ // the file ends
            printf("program incomplete!");
            return tok_illegel;
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

////my parser
//void log_error(std::string msg){
//    cout << msg << endl;
//}
//void program(){
//    block();
//}

//void block(){
//    if(CurTok==tok_lbrace){
//        getNextToken();
//        decls();
//        stmts();
//        if(CurTok==tok_rbrace){
//            getNextToken();
//            return;
//        }
//        else{
//            log_error("missing right brace in a block!");
//        }
//    }
//    else{
//        log_error("missing left brace in a block!");
//    }
//}

//void decls(){
//    while(CurTok==tok_int||CurTok==tok_bool){
//        decl();
//    }
//    return;
//}

//void decl(){
//    if(CurTok==tok_int){
//        getNextToken();
//        if(CurTok==tok_identifier){
//            std::string id= IdentifierStr;
//            //to-do: do something to store the int var
//            getNextToken();
//            if(CurTok==tok_semicolon){
//                getNextToken();
//                return;
//            }
//            else{
//                log_error("missing ; in the end of decalaration!");
//            }
//        }
//        else{
//            log_error("missing identifier in decalaration!");
//        }
//    }
//    else if (CurTok==tok_bool){
//        getNextToken();
//        if(CurTok==tok_identifier){
//            std::string id= IdentifierStr;
//            //to-do: do something to store the bool var
//            getNextToken();
//            if(CurTok==tok_semicolon){
//                getNextToken();
//                return;
//            }
//            else{
//                log_error("missing ; in the end of decalaration!");
//            }
//        }
//        else{
//            log_error("missing identifier in decalaration!");
//        }
//    }
//    else{
//        log_error("unknown data type in declaration!");
//    }
//}

//void stmts(){
//    while(CurTok==tok_identifier||CurTok==tok_if||CurTok==tok_while||
//           CurTok==tok_write||CurTok==tok_read||CurTok==tok_lbrace){
//        stmt();
//    }
//    return;
//}

//int getIdType(std::string id){
//    //to-do implement this function
//    // return the type of the identifier
//    // return -1 if its not in the table
//    return type_uint;
//}

//void stmt(){
//    switch(CurTok){
//    case tok_identifier:{
//        //to-do check the table to find out if the id is decalred and the data type of the id
//        auto id=IdentifierStr;
//        type t=getIdType();

//        getNextToken();//eat id;
//        if(CurTok==tok_assign){
//            getNextToken();//eat =
//            aexpr();
//        }
//            break;
//    }
//    case tok_if:{
//        break;
//    }
//    case tok_while:{
//        break;
//    }
//    case tok_write:{
//        break;
//    }
//    case tok_read:{
//        break;
//    }
//    case tok_lbrace:{
//        break;
//    }
//    default:
//        log_error("unknown token when expecting a statement!");
//        break;
//    }
//}

//test my lexer
// int main(){
//     fin= fopen("testLexerInput.txt","r");
//     foutput = fopen("testLexerOutput.txt","w");
//     IdentifierStr="";
//     NumVal=0;
//     while(ch!=EOF&&!flag){
//         getNextToken();
//         cout << CurTok << " ";
//         cout << IdentifierStr << " " << NumVal << endl;
//     }
//     fclose(fin);
//     fclose(foutput);
//     return 0;
// }
