#include "PythonEditorWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PythonEditorWidget* pythonEditor = new PythonEditorWidget(NULL);
    pythonEditor->show();
    return app.exec();
}

