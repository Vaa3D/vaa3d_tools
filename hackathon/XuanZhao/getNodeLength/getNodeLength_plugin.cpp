/* getNodeLength_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-6-13 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "getnodelengthdialog.h"
#include "getNodeLength_plugin.h"
#include "function.h"
#include <fstream>
#include <sstream>

using namespace std;
Q_EXPORT_PLUGIN2(getNodeLength, getNodeLengthPlugin);
 
QStringList getNodeLengthPlugin::menulist() const
{
	return QStringList() 
        <<tr("UtilityTree")
		<<tr("menu2")
		<<tr("about");
}

QStringList getNodeLengthPlugin::funclist() const
{
	return QStringList()
        <<tr("UtilityRendering")
        <<tr("AxonUtilityRendering")
        <<tr("getUtility")
        <<tr("UtilityContour")
       <<tr("LongAxonRemove")
        <<tr("getUtreeInfolder")
        <<tr("getCurve")
        <<tr("getCurve2")
		<<tr("help");
}

void getNodeLengthPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("UtilityTree"))
	{
//        getNodeLengthDialog a = getNodeLengthDialog(parent,callback);
//        a.exec();
//        a.setModal(false);
//        a.show();
//        a.setAttribute(Qt::WA_DeleteOnClose);
//        dlg->setAttribute(Qt::WA_DelecteOnClose);
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

    if (func_name == tr("UtilityRendering"))
	{
        //
        qDebug()<<"Utility rendering of Neuron";
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double axonRatio = (inparas.size()>=2) ? atof(inparas[1]) : 1;
        double otherR = (inparas.size()>=3) ? atof(inparas[2]) : 1;
        double thre = (inparas.size()>=4) ? atof(inparas[3]) : 1;
        getNodeLength(nt,maxR,axonRatio,otherR,thre);
        writeSWC_file(swcfile.split(".").at(0)+"_max_"+QString::number(maxR)+"_UtilityResult.swc",nt);
	}
    else if (func_name == tr("AxonUtilityRendering"))
    {
        qDebug()<<"Utility rendering of Axonal part";
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        double thre = (inparas.size() >= 1) ? atoi(inparas[0]) : 0;
        int maxR = (inparas.size()>=2) ? atof(inparas[1]) : 100;
        getAxonUtilityTree(nt,thre,maxR);
        writeSWC_file(swcfile.split(".").at(0)+"_"+QString::number(thre,10,2)+"_AxonUtilityTree.swc",nt);
    }
    else if (func_name == tr("LongAxonRemove"))
    {
        qDebug()<<"Remove long axon of neurons";
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        double thre = (inparas.size() >= 1) ? atoi(inparas[0]) : 10;
        int maxR = (inparas.size()>=2) ? atof(inparas[1]) : 100;
        removeLongAxon(nt,thre,maxR);
        writeESWC_file(swcfile+"_longAxonRemoved.eswc",nt);
    }
    else if (func_name == tr("getUtility")){
        qDebug()<<"Considering depth, under developing";
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double axon_ratio = (inparas.size() >= 2) ? atof(inparas[1]) : 1;
        double axon_terminal = (inparas.size() >= 3) ? atof(inparas[2]) : 1;
        double other_ratio = (inparas.size() >= 4) ? atof(inparas[3]) : 1;
        double other_terminal = (inparas.size() >= 5) ? atof(inparas[4]) : 1;
        getUtilityValue(nt,maxR,axon_ratio,axon_terminal,other_ratio,other_terminal);

        writeSWC_file(swcfile.split(".").at(0)+"_"+QString::number(maxR,10,2)+"_"+QString::number(axon_ratio,10,2)+"_Utility.swc",nt);
    }
    else if (func_name == tr("UtilityContour")){
        qDebug()<<"Rendering utility in multi-stage: like python/matlab contour";
        qDebug()<<"UtilityInterval:0-1,1-5,5-10,10-20,20-30,30-50,50-80,80-100";
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        getUtilityContour(nt);
        writeSWC_file(swcfile.split(".").at(0)+"_UtilityCountor.swc",nt);
    }
    else if (func_name == tr("getUtreeInfolder")){
        qDebug()<<"Get Utility rendering results of a folder:axon part";
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double thre = (inparas.size()>=2) ? atof(inparas[1]) : 1;
        QString dirPath = infiles[0];
        QStringList nameFilters;
        nameFilters<<"*.swc";

        QDir dir(dirPath);
        QStringList swcFiles = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);

        for(int i=0; i<swcFiles.size(); i++){
            QString swcFile = dirPath + '\\' + swcFiles[i];
            NeuronTree nt = readSWC_file(swcFile);
            getAxonUtilityTree(nt,maxR,thre);
            writeSWC_file(swcFile.split(".").at(0)+"_"+QString::number(thre,10,2)+"_AxonUtilityTree.swc",nt);
        }

    }
    else if (func_name == tr("getUtilityL1Infolder"))
    {
        qDebug()<<"Get Utility rendering results of a folder: include dendrite";
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double thre = (inparas.size()>=2) ? atof(inparas[1]) : 1;
        QString dirPath = infiles[0];
        QString save_path = outfiles[0];
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QStringList nameFilters;
        nameFilters<<"*.swc";
        QDir dir(dirPath);
        QStringList swcFiles = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);

        for(int i=0; i<swcFiles.size(); i++){
            QString swcFile = dirPath + '/' + swcFiles[i];
            NeuronTree nt = readSWC_file(swcFile);
            double axonRatio =1;
            double otherR =1;
            getUtilityL1Infolder(nt,maxR,axonRatio,otherR,thre);
            writeSWC_file(save_path+"/"+swcFiles[i].split(".").at(0)+"_max_"+QString::number(maxR)+"_UtilityL1Result.swc",nt);
//            getAxonUtilityTree(nt,maxR,thre);
//            writeSWC_file(swcFile.split(".").at(0)+"_"+QString::number(thre,10,2)+"_AxonUtilityTree.swc",nt);
        }
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
    else if (func_name == tr("getCurve2")){
        double thresholds[9] = {0.1,0.5,1.0,5.0,10.0,15.0,20.0,50.0,80.0};
        QString dirPath = infiles[0];
        const char* outFile = outfiles[0];
        QStringList nameFilters;
        nameFilters<<"*.swc";

        QDir dir(dirPath);
        QStringList swcFiles = dir.entryList(nameFilters,QDir::Files|QDir::Readable, QDir::Name);

        ofstream csvFile;
        csvFile.open(outFile,ios::out);

        csvFile<<" ";
        for(int j=0; j<9; j++){
            csvFile<<','<<thresholds[j];
        }
        csvFile<<endl;
        double thres,ratio;
        cout<<"file size: "<<swcFiles.size()<<endl;

        for(int i=0; i<swcFiles.size(); i++){
            QString swcFile = dirPath + '\\' + swcFiles[i];

            NeuronTree nt = readSWC_file(swcFile);
            csvFile<<QFileInfo(swcFile).baseName().toStdString().c_str();
            for(int j=0; j<9; j++){
                thres = thresholds[j];
                ratio = getAxonNodeLength(nt,thres);
                csvFile<<','<<ratio;
                if(j==0){
                    writeSWC_file(swcFile.split(".").at(0)+"_result.swc",nt);
                }
            }
            csvFile<<endl;
        }

    }
	else if (func_name == tr("help"))
	{
        qDebug()<<"[For Windows: Utility Rendeing]vaa3d /x <libname:getNodeLength> /f UtilityRendering /i <input_swc> -p <MaxRadius> <AxonRatio> <OtherTypeRatio>";
        qDebug()<<"[For Windows: Utility Rendeing]vaa3d /x <libname:getNodeLength> /f getUtreeInfolder /i <input_swc_folder_path> -p <MaxRadius> <Threshold>";
        qDebug()<<"[For Windows: Axon Utility Rendering]vaa3d /x <libname:getNodeLength> /f AxonUtilityRendering /i <input_swc> -p <Threshold> <MaxRadius>";
        qDebug()<<"[For Windows: Utility Contour]vaa3d /x <libname:getNodeLength> /f UtilityContour /i <input_swc>";

        //        qmake & nmake -f Makefile.Release
//		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

