/*
 * pythonConsoleTest.cpp
 *
 *  Created on: Dec 18, 2010
 *      Author: Christopher M. Bruns
 */

#include "PythonInterpreter.h"
#include "PythonConsoleWindow.h"
#include <boost/python.hpp>
#include "V3DPythonModule.h"

using namespace v3d;
namespace bp = boost::python;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PythonConsoleWindow* pythonConsole = new PythonConsoleWindow();
    initV3DPythonModule();
    pythonConsole->executeCommand("import v3d");

    pythonConsole->show();

    return app.exec();
}
