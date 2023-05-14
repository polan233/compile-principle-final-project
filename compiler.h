#ifndef COMPILER_H
#define COMPILER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class compiler; }
QT_END_NAMESPACE

class compiler : public QMainWindow
{
    Q_OBJECT

public:
    compiler(QWidget *parent = nullptr);
    ~compiler();

private:
    Ui::compiler *ui;
};
#endif // COMPILER_H
