#include "compiler.h"
#include "ui_compiler.h"

compiler::compiler(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::compiler)
{
    ui->setupUi(this);
}

compiler::~compiler()
{
    delete ui;
}

