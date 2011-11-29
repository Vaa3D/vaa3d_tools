#ifndef V3D_PYTHON_SYNTAX_HIGHLIGHTER_H_
#define V3D_PYTHON_SYNTAX_HIGHLIGHTER_H_

#include <QSyntaxHighlighter>

class PythonSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    enum Format {
        Other,
        Keyword,
        String,
        Comment,
        FinalFormat = Comment
    };

    PythonSyntaxHighlighter(QTextDocument* parent);
    virtual void highlightBlock(const QString& text_block);

protected:
    enum State {
        NormalState = -1,
        InComment,
        InString
    };

private:
    QTextCharFormat m_format[FinalFormat + 1];
};

#endif /* V3D_PYTHON_SYNTAX_HIGHLIGHTER_H_ */
