/* compare_swc2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-3-12 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "compare_swc2_plugin.h"
#include "n_class.h"
using namespace std;
Q_EXPORT_PLUGIN2(compare_swc2, compare_swc);
 
QStringList compare_swc::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList compare_swc::funclist() const
{
	return QStringList()
        <<tr("get_bifurcation_block")
        <<tr("get_un_bifurcation_block")
        <<tr("get_single_swc_block")
        <<tr("pipline")
		<<tr("help");
}

void compare_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2020-3-12"));
	}
}

bool compare_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("get_bifurcation_block"))
	{
        QString swc = infiles[0];
        QString braindir = infiles[1];
        int resolutionX = atoi(inparas[0]);
        int resolutionY = atoi(inparas[1]);
        int resolutionZ = atoi(inparas[2]);
        bool all = (bool)atoi(inparas[3]);
        QString outdir = outfiles[0];
        NeuronTree nt = readSWC_file(swc);
        SwcTree s;
        s.initialize(nt);
        s.get_bifurcation_image(outdir,resolutionX,resolutionY,resolutionZ,all,braindir,callback);
	}
    else if (func_name == tr("get_un_bifurcation_block"))
	{
        QString swc = infiles[0];
        QString braindir = infiles[1];
        int resolutionX = atoi(inparas[0]);
        int resolutionY = atoi(inparas[1]);
        int resolutionZ = atoi(inparas[2]);
        bool all = (bool)atoi(inparas[3]);
        QString outdir = outfiles[0];
        NeuronTree nt = readSWC_file(swc);
        SwcTree s;
        s.initialize(nt);
        s.get_un_bifurcation_image(outdir,resolutionX,resolutionY,resolutionZ,all,braindir,callback);
	}
    else if (func_name == tr("get_single_swc_block"))
    {
        QString swc = infiles[0];
        QString braindir = infiles[1];

        NeuronTree nt = readSWC_file(swc);
        SwcTree s;
        s.initialize(nt);

        vector<int> resolutions ={30,40,50};
        vector<QString> outdirs = {"D:\\ZX\\data\\bifurcation_block_30x30x30",
                               "D:\\ZX\\data\\un_bifurcation_block_30x30x30",
                               "D:\\ZX\\data\\bifurcation_block_40x40x40",
                               "D:\\ZX\\data\\un_bifurcation_block_40x40x40",
                               "D:\\ZX\\data\\bifurcation_block_50x50x50",
                               "D:\\ZX\\data\\un_bifurcation_block_50x50x50"};
        for(int i=0; i<resolutions.size(); ++i)
        {
//            qDebug()<<"resolution: "<<resolutions[i]<<" dir: "<<outdirs[i*2]<<" "<<outdirs[i*2+1];
            s.get_bifurcation_image(outdirs[i*2],resolutions[i],resolutions[i],resolutions[i],true,braindir,callback);
            s.get_un_bifurcation_image(outdirs[i*2+1],resolutions[i],resolutions[i],resolutions[i],false,braindir,callback);
        }
    }
    else if (func_name == tr("pipline"))
    {
        vector<QString> swclist =
        {
            "D:\\ZX\\release_20191231\\17545_00023.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00024.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00025.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00026.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00027.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00028.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00029.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00030.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00040.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00041.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00044.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00045.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00046.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00047.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00048.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00050.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00052.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00053.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00054.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00055.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00056.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00058.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00059.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00060.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00063.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00064.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00065.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00066.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00067.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00068.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00071.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00073.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00074.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00075.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00078.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00081.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00082.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00088.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00089.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00090.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00091.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00092.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00093.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00094.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00095.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00110.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00111.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00113.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00115.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00116.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00118.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00119.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00120.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00121.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00122.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00124.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00125.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00126.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00129.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00130.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00131.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00132.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00136.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00138.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00140.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00142.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00143.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00144.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00145.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00146.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00147.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00149.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00150.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00154.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00155.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00158.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00159.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00162.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00163.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00164.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00165.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00166.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00168.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00169.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00170.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00172.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00173.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00174.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00175.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00177.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00179.ano.swc'",
             "D:\\ZX\\release_20191231\\17545_00180.ano.swc'"
        };
        for(int i=0; i<swclist.size(); ++i){
            QString swc = swclist[i].left(swclist[i].size() - 1);
//            qDebug()<<swclist[i];
            QString braindir = "E:\\mouse17545_teraconvert\\RES(54600x35989x10750)";

            NeuronTree nt = readSWC_file(swc);
            SwcTree s;
            s.initialize(nt);

            vector<int> resolutions ={30,40,50};
            vector<QString> outdirs = {"D:\\ZX\\data\\bifurcation_block_30x30x30",
                                   "D:\\ZX\\data\\un_bifurcation_block_30x30x30",
                                   "D:\\ZX\\data\\bifurcation_block_40x40x40",
                                   "D:\\ZX\\data\\un_bifurcation_block_40x40x40",
                                   "D:\\ZX\\data\\bifurcation_block_50x50x50",
                                   "D:\\ZX\\data\\un_bifurcation_block_50x50x50"};
            for(int i=0; i<resolutions.size(); ++i)
            {
    //            qDebug()<<"resolution: "<<resolutions[i]<<" dir: "<<outdirs[i*2]<<" "<<outdirs[i*2+1];
                s.get_bifurcation_image(outdirs[i*2],resolutions[i],resolutions[i],resolutions[i],true,braindir,callback);
                s.get_un_bifurcation_image(outdirs[i*2+1],resolutions[i],resolutions[i],resolutions[i],false,braindir,callback);
            }
        }

    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

