/* neuronrecon_func.cpp
 * a plugin to reconstruct neuron from multiple traced neurons
 * 09/11/2017 : by Yang Yu
 */

//
#include "neuronrecon_func.h"
#include "neuronrecon.h"

//
const QString title = QObject::tr("Neuron Reconstruction");

int neuronrecon_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    // select a folder

    // load all .swc from user selected folder

    // reconstruct neuron

    //
    return 0;
}

bool neuronrecon_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()==0)
    {
        cerr<<"You must specify input linker or swc files"<<endl;
        return false;
    }

    //parsing output
    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    // load multiple traced neurons (trees saved as .swc)
    PointCloud pointcloud;

    QStringList files;
    for (int i=0;i<inlist->size();i++)
    {
        files.push_back(QString(inlist[i]));
    }

    pointcloud.getPointCloud(files);

    // output .marker file (point cloud)
    QString outfileName;
    outfileName = QString(outlist->at(0));

    pointcloud.savePointCloud(outfileName);

    //
    return true;
}

void printHelp()
{
    cout<<"\nNeuron Reconstruction:"<<endl;
}

