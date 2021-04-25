/* DendritePruning_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2021-4-6 : by XZ
 */
 
#include "v3d_message.h"
#include <vector>
#include "DendritePruning_plugin.h"

#include "branchtree.h"

#include "swccompare.h"

using namespace std;
Q_EXPORT_PLUGIN2(DendritePruning, DendritePruningPlugin);
 
QStringList DendritePruningPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList DendritePruningPlugin::funclist() const
{
	return QStringList()
        <<tr("pruning")
		<<tr("func2")
		<<tr("help");
}

void DendritePruningPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by XZ, 2021-4-6"));
	}
}

bool DendritePruningPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("pruning"))
	{
        QString swcPath = infiles[0];
        QString imgPath = infiles[1];


        double d = inparas.size()>=1 ? atof(inparas[0]) : 10;
        double angleTh = inparas.size()>=2 ? atof(inparas[1]) : 165;
        double somaRth = inparas.size()>=3 ? atof(inparas[2]) : 20;
        double times = inparas.size()>=4 ? atof(inparas[3]) : 5;

        unsigned char* pdata = 0;
        int dataType = 1;
        V3DLONG sz[4] = {0,0,0,0};
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),pdata,sz,dataType);

        NeuronTree t = readSWC_file(swcPath);
        for(int i=0; i<t.listNeuron.size(); ++i){
            t.listNeuron[i].type = 3;
        }
        BranchTree bt = BranchTree();
        bt.initialize(t);

        bt.pruningByLength(pdata,sz,d);
        bt.saveNeuronTree(swcPath + "_out1.swc");
        bt.savePrunedNeuronTree(swcPath + "_outp1.swc");
        bt.update();

        bt.pruningSoma(times);
        bt.saveNeuronTree(swcPath + "_out2.swc");
        bt.savePrunedNeuronTree(swcPath + "_outp2.swc");
        bt.update();

        bt.pruningAdjacentSoma(somaRth);
        bt.saveNeuronTree(swcPath + "_out3.swc");
        bt.savePrunedNeuronTree(swcPath + "_outp3.swc");
        bt.update();

        bt.pruningCross(angleTh,d);
        bt.saveNeuronTree(swcPath + "_out4.swc");
        bt.savePrunedNeuronTree(swcPath + "_outp4.swc");
        bt.update();

//        bt.pruningInflectionPoints(pdata,sz,d,0);
//        bt.saveNeuronTree(swcPath + "_out5.swc");
//        bt.savePrunedNeuronTree(swcPath + "_outp5.swc");

	}
    else if (func_name == tr("calManualFeature"))
	{
        QString swcPath = infiles[0];
        QString imgPath = infiles[1];
        NeuronTree t = readSWC_file(swcPath);

        unsigned char* pdata = 0;
        int dataType = 1;
        V3DLONG sz[4] = {0,0,0,0};
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),pdata,sz,dataType);

        NeuronTree dendrite;
        for(int i=0; i<t.listNeuron.size(); ++i){
            NeuronSWC s = t.listNeuron[i];
            if(s.type == 1 || s.type == 3 ){
                dendrite.listNeuron.push_back(s);
            }
        }
        dendrite.hashNeuron.clear();
        for(int i=0; i<dendrite.listNeuron.size(); ++i){
            dendrite.hashNeuron.insert(dendrite.listNeuron[i].n,i);
        }

        BranchTree bt = BranchTree();
        ofstream csvFile;
        QString csvPath = "F:\\manual\\rLevel0.csv";
        csvFile.open(csvPath.toStdString().c_str(),ios::app);
        bt.initialize(dendrite);
//        bt.show(pdata,sz);
        bt.calRlevel0Branches(pdata,sz,csvFile);


	}
    else if (func_name == tr("getSomaMarker")) {
        QString imgPath = infiles.size()>=1 ? infiles[0] : "";
        unsigned char* pdata = 0;
        int dataType = 1;
        V3DLONG sz[4] = {0,0,0,0};
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),pdata,sz,dataType);
        ImageMarker soma = getCenterSoma(pdata,sz);
        QList <ImageMarker> markers;
        markers.push_back(soma);
        QString markerPath = imgPath + ".marker";
        writeMarker_file(markerPath,markers);
    }
    else if (func_name == tr("compareSwc")) {
        QString swcAutoPath = infiles.size()>=1 ? infiles[0] : "";
        QString swcManualPath = infiles.size()>=2 ? infiles[1] : "";
        QString swcPrunedPath = infiles.size()>=3 ? infiles[2] : "";

        NeuronTree swcAuto = readSWC_file(swcAutoPath);
        NeuronTree swcManual = readSWC_file(swcManualPath);
        NeuronTree swcPruned = readSWC_file(swcPrunedPath);

        compareResult cr;
        compareSwc(swcAuto,swcManual,swcPruned,cr);
        ofstream csvFile;
        QString csvPath = "F:\\manual\\compareResult.csv";
        csvFile.open(csvPath.toStdString().c_str(),ios::app);
        csvFile<<swcAutoPath.toStdString()<<','<<cr.rBranchNumber<<','<<cr.fBranchNumber<<','
              <<cr.negativeBranchNumber<<','<<cr.aBranchNumber<<','
             <<cr.rLength<<','<<cr.fLength<<','<<cr.negativeLength<<','<<cr.alength<<endl;


    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

