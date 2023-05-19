#include "CodeHighLighter.h"
#include <QtDebug>
#include <QBrush>
#include <QColor>
//TO-DO : learn the code from the blog
CodeHighLighter::CodeHighLighter(QTextDocument * parent): QSyntaxHighlighter(parent){

    HighlightingRule rule;

//    // reg below mark as purple, class name
//    classFormat.setFontWeight(QFont::Bold);
//    classFormat.setForeground(Qt::darkMagenta);
//    rule.pattern = QRegExp("\\b[A-Za-z]+:\\b");
//    rule.format = classFormat;
//    highlightingRules.append(rule);
//    rule.pattern = QRegExp("\\b[A-Za-z]+\\.\\b");
//    rule.format = classFormat;
//    highlightingRules.append(rule);

    //identifier
//    identifierFormat.setFontWeight(QFont::Bold);
//    rule.pattern = QRegExp("[a-z,A-Z]+[a-z,A-Z,0-9]*");
//    rule.format = identifierFormat;
//    highlightingRules.append(rule);


    //string dark red
    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("'.*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    //bool purple bold
    QColor color;
    color.setRgb(125,38,205);
    QBrush brush(color,Qt::SolidPattern);
    boolFormat.setForeground(brush);
    boolFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("true");
    rule.format = boolFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("false");
    rule.format = boolFormat;
    highlightingRules.append(rule);

    //number green
    numberFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("[1-9]\\d*.\\d*|0\\.\\d*[1-9]\\d*");
    rule.format = numberFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\d");
    rule.format = numberFormat;
    highlightingRules.append(rule);


    //function italic darkYellow
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::darkYellow);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    //keywords
    color.setRgb(205,129,98);
    QBrush brush_kw(color,Qt::SolidPattern);
    QStringList keywords = {
    "if","else","while","write","read","def"
    };
    //keywords mark as salmon3
    keywordFormat.setForeground(brush_kw);
//    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    for(int i=0; i<keywords.length(); i++)
    {
    QString pattern = "\\b" + keywords[i] + "\\b";
    rule.pattern = QRegExp(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
    }

    //commands
    QStringList types = {
        "int","bool","double"
    };
    //types mark as darkMagenta
    typeFormat.setForeground(Qt::darkMagenta);

    QStringList typePatterns;
    for(int i=0; i<types.length(); i++)
    {
    QString pattern = "\\b" + types[i] + "\\b";
    rule.pattern = QRegExp(pattern);
    rule.format = typeFormat;
    highlightingRules.append(rule);
    }

    //single line comment  green
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegExp("//[^\n]*");
    //rule.pattern = QRegExp("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // multi line comment
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    multiLineCommentFormat.setFontItalic(true);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void CodeHighLighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, rule.format);
        index = expression.indexIn(text, index + length);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);


    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
        setCurrentBlockState(1);
        commentLength = text.length() - startIndex;
        } else {
        commentLength = endIndex - startIndex
                + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

