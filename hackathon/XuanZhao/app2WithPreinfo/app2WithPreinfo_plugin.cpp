/* app2WithPreinfo_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-8-4 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "app2WithPreinfo_plugin.h"

#include "somefunction.h"

#include <fstream>
#include <sstream>

using namespace std;
Q_EXPORT_PLUGIN2(app2WithPreinfo, app2WithPreinfoPlugin);
 
QStringList app2WithPreinfoPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList app2WithPreinfoPlugin::funclist() const
{
	return QStringList()
        <<tr("app2WithPreinfo")
        <<tr("app2WithPreinfoForBatch")
        <<tr("app2WithPreinfoForBatch2")
        <<tr("app2WithPreinfoForBatch3")
		<<tr("help");
}

void app2WithPreinfoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by ZX, 2020-8-4"));
	}
}

bool app2WithPreinfoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("app2WithPreinfo"))
	{
        QString dir = (infiles.size()>=1) ? infiles[0] : "";
        QString brainPath = (infiles.size()>=2) ? infiles[1] : "";
        QString outdir = (outfiles.size()>=1) ? outfiles[0] : "";

        ofstream csvFile;
        csvFile.open((dir + "\\" + "threshold.csv").toStdString().c_str(),ios::out);
        csvFile<<"ID neuron"<<','<<"frontmean"<<','<<"backmean"<<','<<"ratio"<<','<<"increment"<<','<<"threshold"<<endl;

        double ratio = (inparas.size()>=1) ? atof(inparas[0]) : 0;
        int th = (inparas.size()>=2) ? atoi(inparas[1]) : 0;
        qDebug()<<"ratio: "<<ratio<<" th: "<<th;
        app2WithPreinfo(dir,brainPath,outdir,ratio,th,csvFile,callback);
        csvFile.close();
	}
    else if (func_name == tr("app2WithPreinfoForBatch"))
	{
        QString dir = (infiles.size()>=1) ? infiles[0] : "";
        QString brainPath = (infiles.size()>=2) ? infiles[1] : "";
        double ratio = (inparas.size()>=1) ? atof(inparas[0]) : 0;
        int th = (inparas.size()>=2) ? atoi(inparas[1]) : 0;

        QStringList out = dir.split("\\");
        QString dirBaseName = out.back();
        out.pop_back();
        QString outDir = out.join("\\") + "\\" + dirBaseName + "_app2_" + QString::number(th);
        if(!QDir().exists(outDir)){
            QDir().mkdir(outDir);
        }

        ofstream csvFile;
        csvFile.open((outDir + "\\" +dirBaseName + "_app2_" + QString::number(th)+ "_threshold.csv").toStdString().c_str(),ios::out);
        csvFile<<"ID neuron"<<','<<"frontmean"<<','<<"backmean"<<','<<"ratio"<<','<<"increment"<<','<<"threshold"<<endl;
        app2WithPreinfoForBatch(dir,brainPath,ratio,th,csvFile,callback);
        csvFile.close();
	}
    else if (func_name == "app2WithPreinfoForBatch2") {
        QString dir = (infiles.size()>=1) ? infiles[0] : "";
        QString brainPath = (infiles.size()>=2) ? infiles[1] : "";
        int maxTh = (inparas.size()>=1) ? atoi(inparas[0]) : 5;
        float length = (inparas.size()>=2) ? atof(inparas[1]) : 1500;

        QStringList out = dir.split("\\");
        QString dirBaseName = out.back();
        out.pop_back();
        QString outDir = out.join("\\") + "\\" + dirBaseName + "_app2";
        if(!QDir().exists(outDir)){
            QDir().mkdir(outDir);
        }

        ofstream csvFile;
        csvFile.open((outDir + "\\" +dirBaseName + "_app2" + "_threshold.csv").toStdString().c_str(),ios::out);
        csvFile<<"ID neuron"<<','<<"frontmean"<<','<<"backmean"<<','<<"ratio"<<','<<"increment"<<','<<"threshold"<<endl;
        app2WithPreinfoForBatch2(dir,brainPath,csvFile,maxTh,length,callback);
        csvFile.close();
    }
    else if (func_name == "app2WithPreinfoForBatch3") {
        QString dir = (infiles.size()>=1) ? infiles[0] : "";
        QString brainPath = (infiles.size()>=2) ? infiles[1] : "";
        int maxTh = (inparas.size()>=1) ? atoi(inparas[0]) : 5;
        int minTh= (inparas.size()>=2) ? atoi(inparas[1]) : 0;

        QStringList out = dir.split("\\");
        QString dirBaseName = out.back();
        out.pop_back();
        QString outDir = out.join("\\") + "\\" + dirBaseName + "_app2";
        if(!QDir().exists(outDir)){
            QDir().mkdir(outDir);
        }

        ofstream csvFile;
        csvFile.open((outDir + "\\" +dirBaseName + "_app2" + "_threshold.csv").toStdString().c_str(),ios::out);
        csvFile<<"ID neuron"<<','<<"frontmean"<<','<<"backmean"<<','<<"ratio"<<','<<"increment"<<','<<"threshold"<<endl;
        app2WithPreinfoForBatch3(dir,brainPath,csvFile,maxTh,minTh,callback);
        csvFile.close();
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

