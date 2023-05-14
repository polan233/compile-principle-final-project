#include "compiler_program.h"

using namespace std;

int ch=' ';
int cc,ll;
char line[MAXLINE];
bool flag;// flag == true if file ends
int* readbuffer;
int pread;//readbuffer指针

FILE* fin;
FILE* foutput;

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
int getNextChar(){
    getch();
    return ch;
}

// my lexer
int gettok(){

    //skip whitespaces and tab and huan hang
    while(isspace(LastChar)||LastChar==10||LastChar==9){
        LastChar= getNextChar();
    }

    if(isalpha(LastChar)){ // id and commands
        IdentifierStr = LastChar;
        while(isalnum((LastChar=getNextChar()))){
            IdentifierStr += LastChar;
        }

        if(IdentifierStr=="int")
            return tok_int;
        if(IdentifierStr=="bool")
            return tok_bool;
        if(IdentifierStr=="if")
            return tok_if;
        if(IdentifierStr=="else")
            return tok_else;
        if(IdentifierStr=="while")
            return tok_while;
        if(IdentifierStr=="write")
            return tok_write;
        if(IdentifierStr=="read")
            return tok_read;

        return tok_identifier;
    }

    if(isdigit(LastChar)){ // unsigned int NUM
        std::string NumStr;
        do{
            NumStr += LastChar;
            LastChar = getNextChar();
        }while(isdigit(LastChar));
        NumVal = stoi(NumStr);
        return tok_uintnum;
    }

    //  divide or comment
    if(LastChar == '/'){
        LastChar=getNextChar();
        if(LastChar=='*'){ // comment
            bool commentOver=false;
            while (!commentOver) {
                LastChar=getNextChar();
                if(LastChar=='*'){
                    LastChar=getNextChar();
                    if(LastChar=='/'){
                        commentOver=true;
                        break;
                    }
                }
            }
            //skip what's in comment block and then
            // return a token
            if(LastChar!=EOF)
                return gettok();
        }
        else{ // divide
            return tok_div;
        }
    }

    if(LastChar=='+'){
        return tok_add;
    }
    if(LastChar=='-'){
        return tok_sub;
    }
    if(LastChar=='*'){
        return tok_mul;
    }
    if(LastChar=='<'){ // < and <=
        LastChar=getNextChar();
        if(LastChar=='='){ // <=

        }


    }
    //TO-DO: continue to finish this function
    //maybe i should add LastChar=getNextChar(); before return


}

int getNextToken(){
    return CurTok=gettok();
}
