/* spine_detector_plugin.cpp
 * This tool detects spine
 * 2015-3-11 : by Yujie Li
 */
 
#include "spine_detector_plugin.h"
#include "app2.h"


using namespace std;
Q_EXPORT_PLUGIN2(spine_detector, spine_detector);
static spine_detector *dialog=0;

QString versionStr = "v2.620";
QStringList spine_detector::menulist() const
{
	return QStringList() 
        <<tr("spine_detector")
        <<tr("skeleton analysis")
        <<tr("about");
}

QStringList spine_detector::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void spine_detector::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("spine_detector"))
	{

        spine_detector_dialog *dialog=new spine_detector_dialog(&callback);
        dialog->exec();
	}
    else if (menu_name == tr("skeleton analysis"))
	{
        PARA_APP2 p;
        if(!p.initialize(callback)) //here will initialize the image pointer, bounding box, etc.
            return;

        // fetch parameters from dialog
        if (!p.app2_dialog())
            return;

        if (!proc_app2(callback, p, versionStr))
            return;
	}
	else
	{
		v3d_msg(tr("This tool detects spine. "
			"Developed by Yujie Li, 2015-3-11"));
	}
}

bool spine_detector::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
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

