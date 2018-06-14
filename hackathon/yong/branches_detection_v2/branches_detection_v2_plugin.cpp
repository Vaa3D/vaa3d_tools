/* branches_detection_v2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-6-5 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "branches_detection_v2.h"
#include "branches_detection_v2_plugin.h"
Q_EXPORT_PLUGIN2(branches_detection_v2, branches_detection_v2Plugin);

using namespace std;

QStringList branches_detection_v2Plugin::menulist() const
{
	return QStringList() 
        <<tr("branches_detection_v2")
		<<tr("about");
}

QStringList branches_detection_v2Plugin::funclist() const
{
	return QStringList()
        <<tr("branches_detection_v2")
		<<tr("help");
}

void branches_detection_v2Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("branches_detection_v2"))
	{
        BRANCH_LS_PARA P;
        QString path_file = callback.getPathTeraFly();
        P.inimg_file =path_file;
        branches_detection_v2(callback,parent,P);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-6-5"));
	}
}

bool branches_detection_v2Plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("branches_detection_v2"))
	{
        BRANCH_LS_PARA P;
        QString path_file = callback.getPathTeraFly();
        P.inimg_file =path_file;
        branches_detection_v2(callback,parent,P);

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

	}
    else if (func_name == tr("help"))
    {

        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by YourName, 2018-6-5"));

	}
	else return false;

	return true;
}


