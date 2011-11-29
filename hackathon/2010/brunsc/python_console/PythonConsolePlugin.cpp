
#include "PythonConsolePlugin.h"
#include "PythonConsoleWindow.h"
#include "V3DPythonModule.h"
#include <iostream>

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(PythonConsole, PythonConsolePlugin);

PythonConsoleWindow* PythonConsolePlugin::pythonConsoleWindow = 0;

QString getMovieMakerScriptPath() 
{
    QString path = QCoreApplication::applicationDirPath() + 
        "/plugins/python_scripting/python_console/movie_gui.py";
    cerr << path.toStdString() << endl;
    if (QFile(path).exists())
        return path;
    else return "";
}

QStringList PythonConsolePlugin::menulist() const
{
    QStringList answer = QStringList();
    if (getMovieMakerScriptPath().length() > 0)
        answer << tr("Movie maker...");
    answer
        << tr("Run script...")
		<< tr("Open V3D python console...")
		<< tr("About V3D python console...");
    return answer;
}

void createPythonConsole(V3DPluginCallback2 & v3d, QWidget * parent) 
{
	if (PythonConsolePlugin::pythonConsoleWindow) return;
	PythonConsolePlugin::pythonConsoleWindow = new PythonConsoleWindow(parent);
	v3d::initV3DPythonModule(&v3d);
	PythonConsolePlugin::pythonConsoleWindow->executeCommand("import v3d");
}

void launchMovieMaker(V3DPluginCallback2 & v3d, QWidget * parent)
{
    QString movieScriptPath = getMovieMakerScriptPath();
    if (movieScriptPath.length() > 0) {
	    // PythonConsolePlugin::pythonConsoleWindow->show(); // TODO - possibly remove this line?
        PythonConsolePlugin::pythonConsoleWindow->pythonInterpreter->
            runScriptFile(movieScriptPath);
    }
}

void PythonConsolePlugin::domenu(const QString & menu_name,
		V3DPluginCallback2 & v3d, QWidget * parent)
{
	if (menu_name == tr("Open V3D python console...")) {
		createPythonConsole(v3d, parent);
		pythonConsoleWindow->show();
	}
	else if (menu_name == tr("Movie maker...")) {
    	createPythonConsole(v3d, parent);
		launchMovieMaker(v3d, parent);
	}
	else if (menu_name == tr("Run script...")) {
		createPythonConsole(v3d, parent);
		PythonConsolePlugin::pythonConsoleWindow->runScript();
	}
	else
	{ // User selected "About..."
        QString msg = QString("V3D python console plugin version %1\n"
        		"Developed by Christopher M. Bruns. \n"
        		"(Janelia Farm Research Campus, HHMI)")
                .arg(getPluginVersion(), 1, 'f', 1);
		QMessageBox::information(parent, "About the V3D python console", msg);
	}
}
