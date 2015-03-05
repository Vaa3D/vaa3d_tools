/* Mean_Shift_Center_plugin.cpp
 * Search for center using mean-shift
 * 2015-3-4 : by Yujie Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "mean_shift_center_plugin.h"
#include "mean_shift_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(mean_shift_center,mean_shift_plugin );
static mean_shift_dialog *dialog=0;


QStringList mean_shift_plugin::menulist() const
{
	return QStringList() 
        <<tr("mean_shift_center_finder")
		<<tr("about");
}

QStringList mean_shift_plugin::funclist() const
{
	return QStringList()
        <<tr("mean_shift_center_finder")
		<<tr("help");
}

void mean_shift_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("mean_shift_center_finder"))
	{
        dialog=new mean_shift_dialog(&callback);
        dialog->setWindowTitle("mean_shift_center_finder");
        dialog->show();
	}
//	else if (menu_name == tr("menu2"))
//	{
//		v3d_msg("To be implemented.");
//	}
	else
	{
        v3d_msg(tr("<p>The <b>Mean shift center finder</b> performs searches around each of the user-input markers and "
                   "returns the location of local maxima of intensity as the new marker in the output window.<p>"
			"Developed by Yujie Li, 2015-3-4"));
	}
}

bool mean_shift_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("mean_shift_center_finder"))
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

