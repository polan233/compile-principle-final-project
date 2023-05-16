#include "CodeHighLighter.h"
#include <QtDebug>
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
    identifierFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("[a-z,A-Z]+[a-z,A-Z,0-9]*");
    rule.format = identifierFormat;
    highlightingRules.append(rule);


    //string dark red
    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("'.*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    //function italic darkYellow
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::darkYellow);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    //keywords
    QStringList keywords = {
    "if","else","while","write","read","def"
    };
    //keywords mark as darkblue
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
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

