#include "PythonSyntaxHighlighter.h"
#include <iostream>

using namespace std;

PythonSyntaxHighlighter::PythonSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    m_format[Other] = QTextCharFormat();
    m_format[Other].setForeground(QColor(0,0,0)); // black
    m_format[Other].setFontWeight(QFont::Normal);

    m_format[Keyword] = QTextCharFormat();
    m_format[Keyword].setForeground(QColor(10,0,100)); // dark blue
    m_format[Keyword].setFontWeight(QFont::Normal);

    m_format[String] = QTextCharFormat();
    m_format[String].setForeground(QColor(200,10,10)); // red
    m_format[String].setFontWeight(QFont::Normal);

    m_format[Comment] = QTextCharFormat();
    m_format[Comment].setForeground(QColor(10,200,10)); // green
    m_format[Comment].setFontWeight(QFont::Normal);
}

void PythonSyntaxHighlighter::highlightBlock(const QString& text_block)
{
    State state = (State) previousBlockState();
    int len = text_block.length();
    int pos = 0;
    int start = pos;
    for (; pos < len; ++pos)
    {
        switch(state) {
        case InComment:
            for (;pos < len; ++pos) {
                QChar ch = text_block.at(pos);
                if (ch == '\n') {
                    state = NormalState;
                    break;
                }
            }
            setFormat(start, pos - start, m_format[Comment]);
            break; // comment
        case NormalState:
        default:
            for (;pos < len; ++pos) {
                QChar ch = text_block.at(pos);
                if (ch == '#') {
                    state = InComment;
                    break;
                }
            }
            setFormat(start, pos - start, m_format[Other]);
            break; // Normal/default
        }
        // Note how far we highlighted so far
        start = pos;
    }
    setCurrentBlockState(state);
}
