#ifndef COMPILER_H
#define COMPILER_H

#include <QMainWindow>
#include <QEventLoop>

QT_BEGIN_NAMESPACE
namespace Ui { class compiler; }
QT_END_NAMESPACE

class compiler : public QMainWindow
{
    Q_OBJECT

public:
    compiler(QWidget *parent = nullptr);
    ~compiler();
    double getInput();
    void outputLog(std::string msg);


private slots:
    void newDocument();
    void open();
    void save();
    void saveAs();
    void exit();

    void redo();
    void undo();

    //    void setFontUnderline(bool underline);
    //    void setFontBold(bool bold);
    //    void setFontItalic(bool italic);
    void selectFont();
    void compile();
    void run();
    void singleStep();

    //void updateInput();

private:
    Ui::compiler *ui;
    QString currentFile;
};
#endif // COMPILER_H
