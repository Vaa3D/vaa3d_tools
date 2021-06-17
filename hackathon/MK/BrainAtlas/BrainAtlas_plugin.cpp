/* BrainAtlas_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2021-6-14 : by Mars Kuo
 */
 
#include "v3d_message.h"
#include <vector>
#include "BrainAtlas_plugin.h"

using namespace std;

Q_EXPORT_PLUGIN2(BrainAtlas, BrainAtlas_plugin);
 
QStringList BrainAtlas_plugin::menulist() const
{
	return QStringList() 
		<<tr("start Brain Atlas app")
		<<tr("about");
}

QStringList BrainAtlas_plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void BrainAtlas_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("start Brain Atlas app"))
	{
		string imgPath;
		if (!DEBUG) imgPath = ".\\BrainAtlas\\annotation_25_recolor.tif";
		else imgPath = "D:\\AllenVaa3D_2013_Qt486\\v3d_external\\bin\\BrainAtlas\\annotation_25_recolor.tif";
		
		const char* imgPathC = imgPath.c_str();
		Image4DSimple* CCFimgPtr = callback.loadImage(&imgPath.at(0));
		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, CCFimgPtr);
		callback.open3DWindow(newwin);

		this->ctrlPanelPtr = new BrainAtlasControlPanel(parent, &callback);
		this->ctrlPanelPtr->exec();
		delete this->ctrlPanelPtr;
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Mars Kuo, 2021-6-14"));
	}
}

bool BrainAtlas_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("hideFromLeftClick"))
	{
		if (this->ctrlPanelPtr != nullptr)
		{
			string inputParam = input.at(1).type.toStdString();
			cout << " -- Hiding region " << inputParam << endl;
			this->ctrlPanelPtr->hideRegionFromMouseClick(inputParam);
		}
	}
	else if (func_name == tr("scanInvolvedRegions"))
	{
		if (this->ctrlPanelPtr != nullptr)
		{
			QStringList coordQ = input.at(1).type.split("_");
			vector<float> coord(3);
			coord[0] = coordQ.at(0).toFloat();
			coord[1] = coordQ.at(1).toFloat();
			coord[2] = coordQ.at(2).toFloat();
			this->ctrlPanelPtr->scanInvolvedRegions(coord);
		}
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

