/* consensus_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-7-1 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "consensus_plugin.h"

#include "concensusfunction.h"
#include "multiApp2.h"

using namespace std;
Q_EXPORT_PLUGIN2(consensus, ConsensusPlugin);
 
QStringList ConsensusPlugin::menulist() const
{
	return QStringList() 
        <<tr("getApp2Results")
        <<tr("consensus")
        <<tr("rotateImage")
		<<tr("about");
}

QStringList ConsensusPlugin::funclist() const
{
	return QStringList()
        <<tr("consensus")
		<<tr("func2")
		<<tr("help");
}

void ConsensusPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("getApp2Results"))
	{
        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* image = callback.getImage(curwin);

        V3DLONG sz[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};
        V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
        unsigned char* normalData = new unsigned char[tolSZ];
        unsigned char* pdata = image->getRawData();

        int max = 80;
        int min = INT_MAX;
        for(int i=0; i<tolSZ; i++){
            if(pdata[i]<min)
                min = pdata[i];
            if(pdata[i]>80)
                pdata[i] = 80;
        }
        for(int i=0; i<tolSZ; i++){
            int tmp = (unsigned char)(((pdata[i] - min)/(double) (max-min)) * 255 + 0.5);
            if(tmp>255) tmp = 255;
            if(tmp<0) tmp = 0;
            normalData[i] = tmp;
        }
        simple_saveimage_wrapper(callback,(QString(image->getFileName())+QString("_normal.v3draw")).toStdString().c_str(),normalData,sz,1);



//        Image4DSimple* normalImage = new Image4DSimple();
//        normalImage->setData(normalData,image);

//        paraApp2 p = paraApp2();
//        p.initialize(callback);

//        vector<NeuronTree> app2NeuronTrees  = vector<NeuronTree>();

//        bool ok;
//        int k = QInputDialog::getInt(parent,tr("set k"),tr("k:"),2,2,100,1,&ok);
//        if(ok)


//        int* th = getThresholdByKmeans(normalImage,k);
//        p.p4dImage = normalImage;
//        for(int i=1; i<k; i++){
//            qDebug()<<i<<" th: "<<th[i];
//            p.bkg_thresh = th[i];
//            proc_app2(p);
//            p.result.color = XYZW(0,0,0,0);
//            NeuronTree tree = NeuronTree();
//            tree.deepCopy(p.result);
//            for(int j=0; j<tree.listNeuron.size(); j++){
//                tree.listNeuron[j].type = i+1;
//            }
//            app2NeuronTrees.push_back(tree);
//        }

//        NeuronTree merge = mergeNeuronTrees(app2NeuronTrees);
//        merge.color = XYZW(0,0,0,0);

//        callback.setSWC(curwin,merge);
//        callback.open3DWindow(curwin);
//        callback.getView3DControl(curwin)->updateWithTriView();

//        if(th)
//            delete[] th;
//        if(normalImage)
//            delete normalImage;


	}
    else if (menu_name == tr("consensus"))
	{
//        v3dhandle curwin = callback.currentImageWindow();
//        Image4DSimple* image = callback.getImage(curwin);


//        bool ok;
//        int k = QInputDialog::getInt(parent,tr("set k"),tr("k:"),2,2,100,1,&ok);
////        if(ok)


//        int* th = getThresholdByKmeans(image,k);
//        int app2Th = th[k-2];


//        if(th)
//            delete[] th;
//        LandmarkList landMarkers = callback.getLandmark(curwin);
//        vector<NeuronTree> app2NeuronTrees = getApp2NeuronTrees(app2Th,callback,parent);
//        NeuronTree consensusSwc =  consensus(app2NeuronTrees,image,landMarkers,callback);
//        writeSWC_file(QString(image->getFileName()) + "_consensus.swc",consensusSwc);
//        consensusSwc.color = XYZW(0,0,0,0);

//        callback.setSWC(curwin,consensusSwc);
//        callback.open3DWindow(curwin);
//        callback.getView3DControl(curwin)->updateWithTriView();

	}
    else if (menu_name == "rotateImage") {
        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* image = callback.getImage(curwin);
        LandmarkList landMarkers = callback.getLandmark(curwin);
        NeuronTree tree1 = getApp2RotateImage(image,landMarkers,0,30,-1,true);
        NeuronTree tree2 = getApp2RotateImage(image,landMarkers,1,30,-1,true);
        vector<NeuronTree> trees = vector<NeuronTree>();
        trees.push_back(tree1);
        trees.push_back(tree2);
        NeuronTree consensusSwc = consensus(trees,image,landMarkers,callback);

        consensusSwc.color = XYZW(0,0,0,0);

        callback.setSWC(curwin,consensusSwc);
        callback.open3DWindow(curwin);
        callback.getView3DControl(curwin)->updateWithTriView();
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2020-7-1"));
	}
}

bool ConsensusPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("consensus"))
    {
        QString imagePath = (infiles.size()>=1) ? infiles[0] : "";
        QString markerPath = (infiles.size()>=2) ? infiles[1] : "";
        bool kmeansTh = (inparas.size()>=1) ? atoi(inparas[0]) : false;

        qDebug()<<imagePath;
        qDebug()<<markerPath;
        QList<ImageMarker> markers =  readMarker_file(markerPath);
        LandmarkList m;
        for(int i=0; i<markers.size(); i++){
            m.push_back(LocationSimple(markers[i].x,markers[i].y,markers[i].z));
        }

        qDebug()<<"m size:"<<m.size();
//        if(m.size()>25)
//            return;

        consensus(imagePath,m,kmeansTh,callback);


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

