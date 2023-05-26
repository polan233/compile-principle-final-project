#include "compiler.h"
#include "ui_compiler.h"
#include <QFontDialog>
#include <QFile>
#include <QWidget>
#include <QGridLayout>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include "CodeEditor.h"
#include "CodeHighLighter.h"
#include "compiler_program.h"
compiler::compiler(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::compiler)
{
    ui->setupUi(this);

//    CodeEditor* codeEditor = new CodeEditor();
//    codeEditor->setMode(EditorMode::EDIT);
//    codeEditor->setPlainText("test");

    this->setCentralWidget(ui->codeEditor);
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
}

compiler::~compiler()
{
    delete ui;
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
}

