/* swcPruning_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-9-4 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "swcPruning_plugin.h"

#include "pruning.h"
#include "branchtree.h"

#include <fstream>
#include <sstream>

using namespace std;
Q_EXPORT_PLUGIN2(swcPruning, swcPruningPlugin);
 
QStringList swcPruningPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList swcPruningPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void swcPruningPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by ZX, 2020-9-4"));
	}
}

bool swcPruningPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("pruningCross"))
	{
        QString swcPath = infiles[0];
        QString swcFalsePath = swcPath + "_false.swc";
        QString swcPrunedPath = swcPath + "_crossPruned.swc";
        NeuronTree nt = readSWC_file(swcPath);

        double bifurcationD = inparas.size()>=1 ? atof(inparas[0]) : 10;

        pruningCross3(nt,bifurcationD);

        writeSWC_file(swcFalsePath,nt);

        NeuronTree outnt = pruningByType(nt,2);
        writeSWC_file(swcPrunedPath,outnt);

	}
    else if (func_name == tr("pruningSoma"))
	{
        QString swcPath = infiles[0];
        QString swcPrunedPath = swcPath + "_somaPruned.swc";
        double times = inparas.size()>=1 ? atof(inparas[0]) : 5;
        NeuronTree nt = readSWC_file(swcPath);
        for(int i=0; i<2; i++){
            NeuronTree outnt = pruningSoma(nt,times);
            nt.deepCopy(outnt);
        }
        writeSWC_file(swcPrunedPath,nt);
    }else if (func_name == tr("pruningByLength")) {
        QString swcPath = infiles[0];
        QString swcPrunedPath = swcPath + "_pruned.swc";
        double length = inparas.size()>=1 ? atof(inparas[0]) : 5;
        NeuronTree nt = readSWC_file(swcPath);
        NeuronTree outnt = pruningByLength(nt,length);
        writeSWC_file(swcPrunedPath,outnt);
    }else if (func_name == tr("pruning")) {
        QString swcPath = infiles[0];
        QString swcPrunedPath = swcPath + "_pruned.swc";

        double bifurcationD = inparas.size()>=1 ? atof(inparas[0]) : 10;
        double somaTimes = inparas.size()>=2 ? atof(inparas[1]) : 6;

        NeuronTree nt = readSWC_file(swcPath);
        if(somaTimes>0){
            NeuronTree tmpnt1 = pruningSoma(nt,somaTimes);
            nt.listNeuron.clear();
            nt.hashNeuron.clear();
            nt.deepCopy(tmpnt1);
        }

        pruningCross3(nt,bifurcationD);
        NeuronTree tmpnt = pruningByType(nt,2);
        NeuronTree outnt = pruningByLength(tmpnt,5);

        writeSWC_file(swcPrunedPath,outnt);

    }
    else if(func_name == tr("pruningInit"))
    {
        QString swcPath = infiles[0];
        QString imgPath = infiles[1];
        QString swcPrunedPath = swcPath + "_initPruned.swc";

        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        unsigned char* pdata = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),pdata,sz,dataType);

        double bifurcationD = inparas.size()>=1 ? atof(inparas[0]) : 10;
        double somaTimes = inparas.size()>=2 ? atof(inparas[1]) : 6;

        NeuronTree nt = readSWC_file(swcPath);

        NeuronTree outnt = pruningInit(nt,pdata,sz,bifurcationD,somaTimes);
        writeSWC_file(swcPrunedPath,outnt);

    }
    else if (func_name == tr("getCandidateFalsePoint")) {
        QString swcPath = infiles[0];
        QString imgPath = infiles[1];

        unsigned char* inimg1d = 0;
        V3DLONG szs[4] = {0,0,0,0};
        int dataType = 1;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),inimg1d,szs,dataType);

        bool isStandardSwc = (inparas.size()>=1) ? atoi(inparas[0]) : false;
        double d = (inparas.size()>=2) ? atof(inparas[1]) : 10;
        double cosAngleThres = (inparas.size()>=3) ? atof(inparas[2]): -0.5;

        qDebug()<<"standardSwc: "<<isStandardSwc;

        NeuronTree nt = readSWC_file(swcPath);
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].type = 3;
        }

        QString inflectionPointCsvPath = swcPath+"_inflectionPoint.csv";
        QString bifurcationPointCsvPath = swcPath+"_bifurcationPoint.csv";

        if(isStandardSwc){
            inflectionPointCsvPath = swcPath+"_inflectionPoint_standardSwc.csv";
            bifurcationPointCsvPath = swcPath+"_bifurcationPoint_standardSwc.csv";
            for(int i=0; i<nt.listNeuron.size(); i++){
                nt.listNeuron[i].z *= 5;
            }
        }

        ofstream csvFile;
        csvFile.open(inflectionPointCsvPath.toStdString().c_str(),ios::out);
        csvFile<<"level"<<','<<"rLevel"<<','<<"length"<<','<<"cosAngle"<<endl;

        BranchTree bt;
        bt.initialize(nt);

//        bt.refineBifurcationPoint();

        bt.findBranchInflectionPoint(csvFile,d,cosAngleThres);
        csvFile.close();

        long* sz = new long[4];
        for(int i=0; i<4; i++)
            sz[i] = (long)szs[i];

        bt.checkBranchInflectionPoint(inimg1d,sz);

        csvFile.open(bifurcationPointCsvPath.toStdString().c_str(),ios::out);
        csvFile<<"level"<<','<<"rLevel"<<','<<"length"<<','<<"distance"<<','
              <<"lengtToSoma"<<','<<"weight"<<','<<"sWeight"<<','
             <<"localAngle1"<<','<<"localAngle2"<<','<<"localAngle3"<<','
            <<"globalAngle1"<<','<<"globalAngle2"<<','<<"globalAngle3"<<','
           <<"isT"<<endl;
        bt.groupBifurcationPoint(csvFile,d);
//        bt.groupBifurcationPoint2(csvFile,d);

        QString markerPath = swcPath + "_out.marker";
        bt.saveMarkerFlag(markerPath);

        QString outSwcPath = swcPath + "_out.swc";
        writeESWC_file(outSwcPath,bt.nt);

        csvFile.close();

    }
    else if (func_name == tr("getNeighborPoint")){
        QString swcPath = infiles[0];
        double d = (inparas.size()>=1) ? atof(inparas[0]) : 10;
        QString csvPath1 = (inparas.size()>=2) ? inparas[1] : "";
        QString csvPath2 = (inparas.size()>=3) ? inparas[2] : "";

        ofstream csvFile;
        csvFile.open(csvPath1.toStdString().c_str(),ios::app);

        if(!QFile(csvPath1).exists()){
            csvFile<<"Level"<<','<<"rLevel"<<','<<"length"<<endl;
        }
        NeuronTree nt = readSWC_file(swcPath);
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].type = 3;
        }
        BranchTree bt;
        bt.initialize(nt);

        bt.refineBifurcationPoint();

        ofstream csvFileInflection;
        QString inflectionPointCsvPath = swcPath+"_inflectionPoint.csv";
        csvFileInflection.open(inflectionPointCsvPath.toStdString().c_str(),ios::out);
        bt.findBranchInflectionPoint(csvFileInflection,d,0);

        csvFileInflection.close();

        bt.calculateChildrenBranchAngle(csvFile,d);

//        bt.groupBifurcationPoint3(csvFile,d);
//        QString outSwcPath = swcPath + "_out.swc";
//        writeESWC_file(outSwcPath,bt.nt);

        csvFile.close();

        csvFile.open(csvPath2.toStdString().c_str(),ios::app);
        bt.calculateChildrenBranchGlobalAngle(csvFile,d);
        csvFile.close();
    }
    else if (func_name == tr("refineBifurcationPoint")){
        QString swcPath = infiles[0];
        NeuronTree nt = readSWC_file(swcPath);

        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].type = 3;
        }

        BranchTree bt;
        bt.initialize(nt);
        bt.refineBifurcationPoint();

        QString outSwcPath = swcPath + "_refineBifurcationPoint.swc";
        writeESWC_file(outSwcPath,bt.nt);
    }
    else if (func_name == tr("getHierarchySegmentLength")) {
        QString swcPath = infiles[0];
        QString imagePath = infiles[1];
        NeuronTree nt = readSWC_file(swcPath);

        unsigned char* pdata = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);

        QString csvPath = (inparas.size()>=1) ? inparas[0] : "";

        ofstream csvFile;
        csvFile.open(csvPath.toStdString().c_str(),ios::app);
        getHierarchySegmentLength(nt,csvFile,pdata,sz);

        csvFile.close();

    }
    else if (func_name == tr("pruningSWC")){
        QString swcPath = infiles[0];
        double d = (inparas.size()>=1) ? atof(inparas[0]) : 10;

        NeuronTree nt = readSWC_file(swcPath);
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].type = 3;
        }

        BranchTree bt;
        bt.initialize(nt);
        bt.refineBifurcationPoint();



        ofstream csvFileInflection;
        QString inflectionPointCsvPath = swcPath+"_inflectionPoint.csv";
        csvFileInflection.open(inflectionPointCsvPath.toStdString().c_str(),ios::out);
        bt.findBranchInflectionPoint(csvFileInflection,d,-0.5);

        csvFileInflection.close();

        bt.pruningCross(d);

        QString outSwcPath = swcPath + "_pruned.swc";
        writeESWC_file(outSwcPath,bt.nt);

    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

