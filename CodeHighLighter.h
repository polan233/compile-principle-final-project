#ifndef CODEHIGHLIGHTER_H
#define CODEHIGHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>


class CodeHighLighter: public QSyntaxHighlighter{
    Q_OBJECT

public:
    CodeHighLighter(QTextDocument* parent =0);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule{
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression; // the start of cross-line comment /*
    QRegExp commentEndExpression; // the end of cross-line comment */


    QTextCharFormat keywordFormat; // keywords
    QTextCharFormat typeFormat; // types
    QTextCharFormat identifierFormat; // identifier
    QTextCharFormat singleLineKey;
    QTextCharFormat singleLineValue;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat; // string
    QTextCharFormat numberFormat; // numbers
    QTextCharFormat boolFormat; // true and false
    QTextCharFormat functionFormat; // function
    QTextCharFormat symbolFormat; //symbols like { + =
};

#endif // CODEHIGHLIGHTER_H
