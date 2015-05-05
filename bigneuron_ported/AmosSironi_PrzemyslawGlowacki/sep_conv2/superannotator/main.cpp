//qt4
#include <QtGui/QApplication>
//qt5
//#include <QtWidgets/QApplication>
#include "annotatorwnd.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AnnotatorWnd w;
    w.show();

    return a.exec();
}
