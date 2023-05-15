#include "compiler.h"
#include "ui_compiler.h"
#include <QFontDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
using namespace kgl;
compiler::compiler(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::compiler)
{
    ui->setupUi(this);
    //this->setCentralWidget(ui->textEdit);
    QCodeEditor *editor = new QCodeEditor;
    setCentralWidget(editor); // or: ui->someLayout->addWidget(editor);

    connect(ui->actionNew, &QAction::triggered, this, &compiler::newDocument);
    connect(ui->actionOpen, &QAction::triggered , this, &compiler::open);
    connect(ui->actionSave, &QAction::triggered, this, &compiler::save);
    connect(ui->actionExit, &QAction::triggered, this, &compiler::exit);
    connect(ui->actionUndo, &QAction::triggered, this, &compiler::undo);
    connect(ui->actionRedo, &QAction::triggered, this, &compiler::redo);
    connect(ui->actionFont, &QAction::triggered, this, &compiler::selectFont);
}

compiler::~compiler()
{
    delete ui;
}

void compiler::newDocument(){
    currentFile.clear();
    ui->textEdit->setText(QString());
}

void compiler::open(){ //open a file and copy all content into textEdit
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
    ui->textEdit->setText(text);
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
    QString text = ui->textEdit->toPlainText();
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
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.close();
}

void compiler:: exit(){
    QCoreApplication::quit();
}


void compiler::redo(){
    ui->textEdit->redo();
}
void compiler::undo(){
    ui->textEdit->undo();
}

void compiler:: selectFont(){
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected,this);
    if(fontSelected){
        ui->textEdit->setFont(font);
    }
}

