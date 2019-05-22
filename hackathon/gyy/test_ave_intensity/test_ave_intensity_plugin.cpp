/* test_ave_intensity_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-19 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test_ave_intensity_plugin.h"
#include <iostream>
#include "../../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(test_ave_intensity, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        //v3d_msg("To be implemented.");
        //proc_app2(callback, p2, versionStr);
        //QString markername = (tif --> marker)
        QStringList filepaths = QFileDialog::getOpenFileNames(parent, QString("open file tif:"),"/home/gyy/Desktop");
        V3DLONG size0 = filepaths.size();
        for(V3DLONG i = 0; i < size0; i ++)
        {
            NeuronTree nt_eswc = readSWC_file(filepaths[i]);
            QFileInfo fileInfo(filepaths[i]);
            QString tiffile = fileInfo.dir().absolutePath() + "/" + fileInfo.baseName() + ".tif";
            unsigned char *img1d = NULL;
            int datatype = 0;
            V3DLONG sz[4] = {0,0,0,0};
            simple_loadimage_wrapper(callback, tiffile.toStdString().c_str(), img1d, sz, datatype);
            V3DLONG num1 = sz[0]*sz[1]*sz[2];
            V3DLONG num = 0;
            double total_intensity, ave_intensity;
            unsigned char * pmask = new unsigned char[num1];
            unsigned char * pa = new unsigned char[num1];
            memset(pmask, 0, sizeof(unsigned char)*num1);
            memset(pa, 0, sizeof(unsigned char)*num1);
            ComputemaskImage(nt_eswc, pmask, sz[0], sz[1], sz[2]);
            for(V3DLONG i=0; i<num1; i++)
            {
                if(pmask[i]>0)
                {
                    pa[i]=img1d[i];
                    num++;
                    total_intensity+=(double)img1d[i];
                }
                else
                {
                    pa[i]=0;
                }
            }
            if(num!=0)
                ave_intensity=total_intensity/num;
            cout<<"ave_intensity="<<ave_intensity<<endl;
        }
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

