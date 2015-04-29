/* shrink_plugin.cpp
 * This is a plugin for neuron tracing. It uses dynamic threshold and morphological operation to find the skeleton of neurons.
 * 2015-4-28 : by GaoShan
 */

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "shrink_plugin.h"
#include "shrink.h"
#include "shrink_threshold.h"
#include <iostream>

Q_EXPORT_PLUGIN2(shrink, Neuron_Reconstruction)

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

QStringList Neuron_Reconstruction::menulist() const
{
    return QStringList()
           <<tr("tracing_menu")
           <<tr("about");
}

QStringList Neuron_Reconstruction::funclist() const
{
    return QStringList()
           <<tr("tracing_func")
           <<tr("help");
}

void Neuron_Reconstruction::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("tracing_menu"))
    {
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

    }
    else
    {
        v3d_msg(tr("This is a plugin for neuron tracing. It uses dynamic threshold and morphological operation to find the skeleton of neurons.. "
                   "Developed by GaoShan, 2015-4-28"));
    }
}

bool Neuron_Reconstruction::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("tracing_func"))
    {
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA,bmenu);
    }
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


        printf("**** Usage of shrink tracing **** \n");
        printf("vaa3d -x shrink -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

    }
    else return false;

    return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    unsigned char *bw = new unsigned char[N*M*P];
    if(!bw)
    {
        cout<<"Error Allocating Memory"<<endl;
        return;
    }
    shrink_im2bw(data1d, bw, N, M, P, 2, 8);
    cout<<"finish thresholding\n"<<endl;

    NeuronTree before_pruning;
    shrink_tracing(bw, N, M, P, 3, &before_pruning);

    //Output
    QString swc_name = PARA.inimg_file + "_shrink.swc";
    before_pruning.name = "shrink";
    writeSWC_file(swc_name.toStdString().c_str(),before_pruning);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
