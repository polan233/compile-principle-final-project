#include "compiler.h"
#include "ui_compiler.h"
#include <QFontDialog>
#include <QFile>
#include <QWidget>
#include <QGridLayout>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QDebug>
#include "CodeEditor.h"
#include "CodeHighLighter.h"
#include "compiler_program.h"

extern QEventLoop loop;

//to-do 加入修改后多一个星号的功能 加入退出前询问是否保存的功能
compiler::compiler(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::compiler)
{
    ui->setupUi(this);

//    CodeEditor* codeEditor = new CodeEditor();
//    codeEditor->setMode(EditorMode::EDIT);
//    codeEditor->setPlainText("test");

    ui->codeEditor->setMode(EditorMode::EDIT);
    QFont font;
    //设置文字字体
    font.setFamily("Consolas");
    font.setPointSize(14);
    ui->codeEditor->setFont(font);


//    ui->gridLayout->addWidget(codeEditor);

    // or: ui->someLayout->addWidget(editor);
    CodeHighLighter* highlighter = new CodeHighLighter();
    highlighter->setDocument(ui->codeEditor->document());

    connect(ui->actionNew, &QAction::triggered, this, &compiler::newDocument);
    connect(ui->actionOpen, &QAction::triggered , this, &compiler::open);
    connect(ui->actionSave, &QAction::triggered, this, &compiler::save);
    connect(ui->actionExit, &QAction::triggered, this, &compiler::exit);
    connect(ui->actionUndo, &QAction::triggered, this, &compiler::undo);
    connect(ui->actionRedo, &QAction::triggered, this, &compiler::redo);
    connect(ui->actionSaveAs, &QAction::triggered , this , &compiler::saveAs);
    connect(ui->actionSet_Font,&QAction::triggered , this , &compiler::selectFont);
    connect(ui->actionCompile,&QAction::triggered,this,&compiler::compile);
    connect(ui->actionRun,&QAction::triggered,this,&compiler::run);
    connect(ui->actionDebug,&QAction::triggered,this,&compiler::singleStep);
    connect(ui->input,&QLineEdit::returnPressed,&loop,&QEventLoop::quit);

}

compiler::~compiler()
{
    delete ui;
}


void compiler::outputLog(std::string msg){
    QMessageBox::information(this,"提示",QString::fromStdString(msg));
    //输出编译信息
    QFile flog("temp-log.txt");
    if(!flog.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
        QMessageBox::warning(this,"Warning","Cannot open file:"+flog.errorString());
        return;
    }
    QTextStream login(&flog);
    QString text=login.readAll();
    ui->log->setPlainText(text);
    flog.close();
    return;
}

double compiler::getInput(){
    QString text = ui->input->text();
    std::string str=text.toStdString();
    ui->input->setText("");
    return stod(str);
}

void compiler::newDocument(){
    currentFile.clear();
    ui->codeEditor->setPlainText(QString());
}

void compiler::open(){ //open a file and copy all content into codeEditor
    QString fileName=QFileDialog::getOpenFileName(this,"Open the file");
    QFile file(fileName);
    currentFile = fileName;
    if(!file.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
        QMessageBox::warning(this,"Warning","Cannot open file:"+file.errorString());
        return;
    }
    setWindowTitle(fileName);
    QTextStream in(&file);
    QString text=in.readAll();
    ui->codeEditor->setPlainText(text);
    file.close();
}

void compiler::save(){
    QString fileName;
    // if dont have a fileName, we create one
    if(currentFile.isEmpty()){
        fileName=QFileDialog::getSaveFileName(this,"Name the file.");
        currentFile=fileName;
    }
    else{
        fileName=currentFile;
    }
    QFile file(fileName);
    //handle error when opening the file
    if(!file.open(QIODevice::WriteOnly|QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot save file: "+file.errorString());
        return;
    }
    setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = ui->codeEditor->toPlainText();
    out << text;
    file.close();
}

void compiler:: saveAs(){
    QString fileName= QFileDialog::getSaveFileName(this,"Save as:");
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot save file: "+file.errorString());
        return;
    }
    currentFile = fileName;
    setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = ui->codeEditor->toPlainText();
    out << text;
    file.close();
}

void compiler:: exit(){
    QCoreApplication::quit();
}


void compiler::redo(){
    ui->codeEditor->redo();
}
void compiler::undo(){
    ui->codeEditor->undo();
}

void compiler:: selectFont(){
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected,this);
    if(fontSelected){
        ui->codeEditor->setFont(font);
    }
}

void compiler:: compile(){
    //先做保存
    QString fileName;
    // if dont have a fileName, we create one
    if(currentFile.isEmpty()){
        fileName=QFileDialog::getSaveFileName(this,"Name the file.");
        currentFile=fileName;
    }
    else{
        fileName=currentFile;
    }
    QFile file(fileName);
    //handle error when opening the file
    if(!file.open(QIODevice::WriteOnly|QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot save file: "+file.errorString());
        return;
    }
    setWindowTitle(fileName);
    QTextStream out(&file);
    QString text = ui->codeEditor->toPlainText();
    out << text;
    file.close();
    int err=compileCX(currentFile.toStdString());
    cout << err;

    //输出编译信息
    QFile flog("temp-log.txt");
    if(!flog.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
        QMessageBox::warning(this,"Warning","Cannot open file:"+flog.errorString());
        return;
    }
    QTextStream login(&flog);
    text=login.readAll();
    ui->log->setPlainText(text);
    flog.close();

    //清空数据栈
    ui->stack->setPlainText("");

    if(err==0){
        //输出符号表
        QFile ftable("temp-table.txt");
        if(!ftable.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
            QMessageBox::warning(this,"Warning","Cannot open file:"+ftable.errorString());
            return;
        }
        QTextStream tablein(&ftable);
        text=tablein.readAll();
        ui->table->setPlainText(text);
        ftable.close();
        //输出中间代码
        QFile fcode("temp-code.txt");
        if(!fcode.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
            QMessageBox::warning(this,"Warning","Cannot open file:"+fcode.errorString());
            return;
        }
        QTextStream codein(&fcode);
        text=codein.readAll();
        ui->pcode->setPlainText(text);
        fcode.close();
    }


}

void compiler:: run(){
    compile();
    exeAll();
    //输出log信息
    QFile flog("temp-log.txt");
    if(!flog.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
        QMessageBox::warning(this,"Warning","Cannot open file:"+flog.errorString());
        return;
    }
    QTextStream login(&flog);
    QString text=login.readAll();
    ui->log->setPlainText(text);
    flog.close();
}

void compiler:: singleStep(){
    if(exeOne()){
        //输出log信息
        QFile flog("temp-log.txt");
        if(!flog.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
            QMessageBox::warning(this,"Warning","Cannot open file:"+flog.errorString());
            return;
        }
        QTextStream login(&flog);
        QString text=login.readAll();
        ui->log->setPlainText(text);
        flog.close();

        //输出数据栈
        QFile fs("temp-stack.txt");
        if(!fs.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
            QMessageBox::warning(this,"Warning","Cannot open file:"+fs.errorString());
            return;
        }
        QTextStream sin(&fs);
        text=sin.readAll();
        ui->stack->setPlainText(text);
        fs.close();
    }
    else{
        QMessageBox::information(this,"提示","运行完毕!");
        //输出log信息
        QFile flog("temp-log.txt");
        if(!flog.open(QIODevice::ReadOnly|QFile::Text)){ //cant open
            QMessageBox::warning(this,"Warning","Cannot open file:"+flog.errorString());
            return;
        }
        QTextStream login(&flog);
        QString text=login.readAll();
        ui->log->setPlainText(text);
        flog.close();
    }
}
