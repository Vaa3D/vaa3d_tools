/* getSomaFeature_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-8-4 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "getSomaFeature_plugin.h"

#include <fstream>
#include <sstream>

#include"somefunction.h"

using namespace std;
Q_EXPORT_PLUGIN2(getSomaFeature, getSomaFeaturePlugin);
 
QStringList getSomaFeaturePlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList getSomaFeaturePlugin::funclist() const
{
	return QStringList()
        <<tr("getSomaFeature")
        <<tr("getSomaFeature2")
		<<tr("help");
}

void getSomaFeaturePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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

bool getSomaFeaturePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("getSomaFeature"))
	{
        QString brainDir = infiles[0];

        QFileInfoList brainPaths = QDir(brainDir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(int i=0; i<brainPaths.size(); i++){
            QString brainId = brainPaths[i].baseName();
            qDebug()<<brainId;
            ofstream csvFile;
            csvFile.open((brainDir+"\\"+ brainId + "_somaFeature.csv").toStdString().c_str(),ios::out);
            csvFile<<"brainId"<<','<<"name"<<','<<"anum"<<','<<"bnum"<<','<<"cnum"<<','<<"admean"<<','<<"bdmean"<<','<<"cdmean"
                  <<','<<"bIntensityMean"<<','<<"bIntensityStd"<<','<<"th"<<','<<"somaIntensityMean"<<endl;


            QString brianPath = brainDir + "\\" + brainId;
            qDebug()<<brianPath;
            vector<somaFeature> somaFeatures = getBrainSomasFeature(brianPath,callback);
            for(int j=0; j<somaFeatures.size(); j++){
                somaFeature& sf = somaFeatures[j];
                csvFile<<brainId.toStdString().c_str()<<','<<sf.name.toStdString().c_str()<<','<<sf.anum<<','<<sf.bnum<<','<<sf.cnum<<','
                      <<sf.admean<<','<<sf.bdmean<<','<<sf.cdmean<<','<<sf.bIntensityMean<<','
                     <<sf.bIntensityStd<<','<<sf.th<<','<<sf.somaIntensityMean<<endl;
            }
            csvFile.close();
        }

	}
    else if (func_name == tr("getSomaFeature2"))
	{
        QString brainDir = infiles[0];

        QFileInfoList brainPaths = QDir(brainDir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(int i=0; i<brainPaths.size(); i++){
            QString brainId = brainPaths[i].baseName();
            qDebug()<<brainId;
            ofstream csvFile;
            csvFile.open((brainDir+"\\"+ brainId + "_somaFeature.csv").toStdString().c_str(),ios::out);
            csvFile<<"brainId"<<','<<"name"<<','<<"anum"<<','<<"bnum"<<','<<"cnum"<<','<<"admean"<<','<<"bdmean"<<','<<"cdmean"
                  <<','<<"bIntensityMean"<<','<<"bIntensityStd"<<','<<"th"<<','<<"somaIntensityMean"
                 <<','<<"bIntensityPartialStd"<<','<<"thPercent"<<','<<"intensityPercent1"<<','<<"intensityPercent5"
                <<','<<"multiType"<<','<<"isLowContrast"<<','<<"isLayered"<<endl;


            QString brianPath = brainDir + "\\" + brainId;
            qDebug()<<brianPath;
            vector<somaFeature> somaFeatures = getBrainSomasFeature2(brianPath,callback);
            for(int j=0; j<somaFeatures.size(); j++){
                somaFeature& sf = somaFeatures[j];
                csvFile<<brainId.toStdString().c_str()<<','<<sf.name.toStdString().c_str()<<','<<sf.anum<<','<<sf.bnum<<','<<sf.cnum<<','
                      <<sf.admean<<','<<sf.bdmean<<','<<sf.cdmean<<','<<sf.bIntensityMean<<','
                     <<sf.bIntensityStd<<','<<sf.th<<','<<sf.somaIntensityMean<<','
                    <<sf.bIntensityPartialStd<<','<<sf.thPercent<<','<<sf.intensityPercent1<<','<<sf.intensityPercent5<<','
                   <<sf.multiType<<','<<sf.isLowContrast<<','<<sf.isLayered<<endl;
            }
            csvFile.close();
        }
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

