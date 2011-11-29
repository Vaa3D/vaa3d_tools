#include "PythonEditorWidget.h"
#include "PythonSyntaxHighlighter.h"
#include <iostream>

using namespace std;

PythonEditorWidget::PythonEditorWidget(QWidget* parent)
        : QMainWindow(parent)
{
    setupUi(this);
    setWindowIcon(QIcon(":/icons/python.png")); // TODO - does not work
    new PythonSyntaxHighlighter(textEdit->document());
}
