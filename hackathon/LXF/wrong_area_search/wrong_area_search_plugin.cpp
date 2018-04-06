/* wrong_area_search_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-3-13 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include "wrong_area_search_plugin.h"
#include"find_wrong_area.h"
#include"data_training.h"
#include"get_sample_area.h"
#include "get_sub_terafly.h"
using namespace std;
Q_EXPORT_PLUGIN2(wrong_area_search, wrong_area_searchPlugin);
//bool find_wrong_area(const V3DPluginArgList & input, V3DPluginArgList & output);
QStringList wrong_area_searchPlugin::menulist() const
{
	return QStringList() 
        <<tr("find_wrong_area")
        <<tr("data_training")
        <<tr("get_sample_area")
        <<tr("get_sub_terafly")
		<<tr("about");
}

QStringList wrong_area_searchPlugin::funclist() const
{
	return QStringList()
        <<tr("find_wrong_area")
        <<tr("data_training")
        <<tr("get_sample_area")
		<<tr("help");
}

void wrong_area_searchPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("find_wrong_area"))
	{
        bool bmenu = true;
        Input_para PARA;
        find_wrong_area(PARA,callback,bmenu,parent);
        v3d_msg("find_wrong_area_done.");
	}
    else if (menu_name == tr("data_training"))
	{
        data_training(callback,parent);
        v3d_msg("data_training_done.");
	}
    else if (menu_name == tr("get_sample_area"))
    {
        get_sample_area(callback,parent);
        v3d_msg("get_sample_done.");
    }
    else if(menu_name == tr("get_sub_terafly"))
    {
        //v3d_msg("Still working on.");
        get_sub_terafly(callback,parent);
    }
	else
	{
        v3d_msg(tr("This is wrong_area_search plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-3-13"));
	}
}

bool wrong_area_searchPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("find_wrong_area"))
	{
        vector<char*> infiles, inparas, outfiles;
        if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
        if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
        if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
        Input_para PARA;
        vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
        vector<char*>* outlist = NULL;
        vector<char*>* paralist = NULL;
        if(input.size() != 1)
        {
            printf("Please make sure there is just one input.\n");
            return false;
        }
//        paralist = (vector<char*>*)(input.at(1).p);
//        if(paralist->size()!=2)
//        {
//            printf("Please specify 6 parameter -  the resampling step length and ...");
//            return false;
//        }

        QString fileOpenName = QString(inlist->at(0));
        QString fileOpenName2 = QString(inlist->at(1));
        QString raw_img = QString(inlist->at(2));

        PARA.filename1 = fileOpenName;
        PARA.filename2 = fileOpenName2;
        PARA.filename3 = raw_img;
        PARA.para1 = atof(paralist->at(0));
        PARA.para2 = atof(paralist->at(1));
        PARA.para3 = atof(paralist->at(2));
        PARA.para4 = atof(paralist->at(3));
        PARA.model1 = atof(paralist->at(4));
        PARA.model2 = atof(paralist->at(5));
        bool bmenu = false;
        find_wrong_area(PARA,callback,bmenu,parent);
    }
    else if (func_name == tr("data_training"))
    {
        data_training(callback,input,output);
    }
    else if(func_name == tr("get_sample_area"))
    {
        get_sample_area(callback,input,output,parent);
    }
//    else if(func_name == tr("get_sub_terafly"))
//    {
//        get_sub_terafly(callback,input,output,parent);
//    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

