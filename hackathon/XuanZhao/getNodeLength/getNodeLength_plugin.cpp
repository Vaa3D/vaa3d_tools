/* getNodeLength_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-6-13 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "getNodeLength_plugin.h"
#include "function.h"
#include <fstream>
#include <sstream>

using namespace std;
Q_EXPORT_PLUGIN2(getNodeLength, getNodeLengthPlugin);
 
QStringList getNodeLengthPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList getNodeLengthPlugin::funclist() const
{
	return QStringList()
        <<tr("getNodeLength")
        <<tr("getCurve")
		<<tr("help");
}

void getNodeLengthPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by YourName, 2020-6-13"));
	}
}

bool getNodeLengthPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("getNodeLength"))
	{
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double dendritR = (inparas.size()>=2) ? atof(inparas[1]) : 1;
        double otherR = (inparas.size()>=3) ? atof(inparas[2]) : 1;
        double thre = (inparas.size()>=4) ? atof(inparas[3]) : 1;
        getNodeLength(nt,maxR,dendritR,otherR,thre);
        writeSWC_file(swcfile.split(".").at(0)+"_result.swc",nt);
	}
    else if (func_name == tr("getCurve"))
	{
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        ofstream csvFile;
        QStringList f = swcfile.split(".");//f.absoluteDir().absolutePath() + f.baseName() + "_result.csv";
        f.pop_back();
        csvFile.open((f.join(".") + "_result.csv").toStdString().c_str(),ios::out);
        csvFile<<"thre"<<','<<"useratio"<<endl;

        double useratio;
        for(int i=1; i<=100; i++){
            double threI = i*0.01;

            if(threI>=0.98 && threI<1){
                for(int j=0;j<=9;j++){
                    double threJ = threI + j*0.001;
                    if(threJ>=0.99 && threJ<1){
                        for(int k=0; k<=9; k++){
                            double thre = threJ + 0.0001*k;
                            useratio = getNodeLength2(nt,100,1,1,thre);
                            csvFile<<thre<<','<<useratio<<endl;
                        }
                    }else {
                        double thre = threJ;
                        useratio = getNodeLength2(nt,100,1,1,thre);
                        csvFile<<thre<<','<<useratio<<endl;
                    }

                }
            }else {
                double thre = threI;
                useratio = getNodeLength2(nt,100,1,1,thre);
                csvFile<<thre<<','<<useratio<<endl;
            }

        }
        csvFile.close();

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

