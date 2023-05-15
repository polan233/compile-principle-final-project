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

private slots:
    void newDocument();
    void open();
    void save();
    void saveAs();
    void exit();
    void about();

    void redo();
    void undo();

    //    void setFontUnderline(bool underline);
    //    void setFontBold(bool bold);
    //    void setFontItalic(bool italic);
    void selectFont();

private:
    Ui::compiler *ui;
    QString currentFile;
};
#endif // COMPILER_H
