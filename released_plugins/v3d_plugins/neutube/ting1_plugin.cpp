/* ting1_plugin.cpp
 * a collaboration with Hanchuan
 * 2012-12-9 : by Ting Zhao and HP
 */
 
#include "v3d_message.h"
#include <vector>
#include "ting1_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(ting1, neutube);
 
void call_neutube();

QString getAppPath()
{
	QDir testPluginsDir = QDir(qApp->applicationDirPath());
    
#if defined(Q_OS_WIN)
	if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
    testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	// In a Mac app bundle, plugins directory could be either
	//  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
	//  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
	if (testPluginsDir.dirName() == "MacOS") {
		QDir testUpperPluginsDir = testPluginsDir;
		testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
		if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
		testPluginsDir.cdUp();
	}
#endif
    
	return testPluginsDir.absolutePath();
}


QStringList neutube::menulist() const
{
	return QStringList() 
		<<tr("neutube")
		<<tr("about");
}

QStringList neutube::funclist() const
{
	return QStringList()
		<<tr("neutube")
		<<tr("help");
}

void neutube::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neutube"))
	{
           call_neutube();
	}
	else
	{
		v3d_msg(tr("A plugin to invoke NeuTube neuron reconstruction. "
			"Developed by Hanchuan Peng and Ting Zhao, 2013-2014"));
	}
}

bool neutube::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("neutube"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}


void call_neutube()
{
        QString tmpfile;
    
#if defined(Q_OS_WIN)
    tmpfile = getAppPath().append("neuTube.exe");

#elif defined (Q_OS_MAC)
    tmpfile = getAppPath().append("neuTube.app");

#else
    tmpfile = getAppPath().append("neuTube");

#endif
    
        //
        QFile tmpqfile(tmpfile);
        if (!tmpqfile.exists())
    {
        v3d_msg("Cannot locate the executable of NeuTube program. Now you can specify where it is.");
        tmpfile = QFileDialog::getOpenFileName(0, QObject::tr("select the executable of NeuTube program"),
                                                          QDir::currentPath(),
                                                          QObject::tr("Executable File (*)"));
        
        if(tmpfile.isEmpty())
        {
            return;
        }
    }
    
#if defined(Q_OS_WIN)
    system(qPrintable(tmpfile));
#elif defined (Q_OS_MAC)
    system(qPrintable(tmpfile.prepend("open ")));
#else
    system(qPrintable(tmpfile.prepend("")));
#endif
    
}
