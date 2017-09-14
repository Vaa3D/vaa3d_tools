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
        files.push_back(QString(inlist->at(i)));
    }

    pointcloud.getPointCloud(files);

    // output .marker file (point cloud)
    QString outfileName;
    outfileName = QString(outlist->at(0));

    pointcloud.savePointCloud(outfileName);

    //
    return true;
}

bool samplingtree_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    float sampleratio = 2;
    if (input.size()==2)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            sampleratio = atof(paras->at(0));
            cout<<"sampleratio: "<<sampleratio<<endl;
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

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

    // load
    QStringList files;
    for (int i=0;i<inlist->size();i++)
    {
        files.push_back(QString(inlist->at(i)));
    }

    QString filename = files.at(0);

    if(filename.toUpper().endsWith(".SWC"))
    {
        NeuronTree nt = readSWC_file(filename);

        for(V3DLONG i=0; i<nt.listNeuron.size(); i++)
        {
            nt.listNeuron[i].x *= sampleratio;
            nt.listNeuron[i].y *= sampleratio;
            nt.listNeuron[i].z *= sampleratio;
            nt.listNeuron[i].r *= sampleratio;
        }

        // output
        QString outfileName;
        outfileName = QString(outlist->at(0));

        writeSWC_file(outfileName, nt);
    }

    //
    return true;
}

bool finetunepoints_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()==0 || output.size() != 1) return false;

    //parsing input
    char * paras = NULL;
    float radius = 5;
    if (input.size()==2)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            radius = atof(paras->at(0));
            cout<<"radius: "<<radius<<endl;
        }
        else
        {
            cerr<<"Too many parameters"<<endl;
            return false;
        }
    }

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

    // load
    QList <ImageMarker> pc;
    V3DLONG *szimg = 0;
    int datatype_img = 0;
    unsigned char* p1dImg = NULL;
    V3DLONG dimx, dimy, dimz;
    for (int i=0;i<inlist->size();i++)
    {
        QString filename = QString(inlist->at(i));

        if(filename.toUpper().endsWith(".MARKER"))
        {
            pc = readMarker_file(filename);
        }
        else if(filename.toUpper().endsWith(".V3DRAW"))
        {
            Image4DSimple * p4dImage = callback.loadImage( const_cast<char *>(filename.toStdString().c_str()) );
            if (!p4dImage || !p4dImage->valid())
            {
                cout<<"fail to load image!\n";
                return false;
            }

            if(p4dImage->getDatatype()!=V3D_UINT8)
            {
                cout<<"Not supported!\n";
                return false;
            }

            p1dImg = p4dImage->getRawData();
            dimx = p4dImage->getXDim();
            dimy = p4dImage->getYDim();
            dimz = p4dImage->getZDim();
        }
    }

    // fine tuning with mean-shift
    float distance = 2; // converge

    for(V3DLONG i=0; i<pc.size(); i++)
    {
        cout<<"before fine tuning ... "<<pc[i].x <<" "<<pc[i].y <<" "<<pc[i].z <<" "<<pc[i].radius<<endl;

    }



    // output
    QString outfileName;
    outfileName = QString(outlist->at(0));

    //
    return true;
}

void printHelp()
{
    cout<<"\n Optimal Constructing Neuron Trees: \n"<<endl;
}

