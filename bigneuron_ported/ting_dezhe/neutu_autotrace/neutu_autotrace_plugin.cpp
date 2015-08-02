/* neutube_v2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-2-9 : by Zhi Zhou 
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "basic_surf_objs.h"

#include "neutu_autotrace_plugin.h"
#include "libNeuTu.h"

Q_EXPORT_PLUGIN2(neutube_v2, neutube_v2);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList neutube_v2::menulist() const
{
	return QStringList() 
		<<tr("tracing")
		<<tr("about");
}

QStringList neutube_v2::funclist() const
{
	return QStringList()
		<<tr("tracing")
		<<tr("help");
}

void neutube_v2::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
        v3d_msg(tr("This is a neuron reconstruction plugin.."
            "Developed by Ting Zhao, and imported by Zhi Zhou , 2015-2-9"));
	}
}

bool neutube_v2::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing"))
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


		printf("**** Usage of neutube_v2 tracing **** \n");
		printf("vaa3d -x neutube_v2 -f tracing -i <inimg_file> -p <channel> <other parameters>\n");
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

    //Stack *data1d_ch1 = C_Stack::make(GREY,(int)N,(int)M,(int)P);
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    size_t voxelNumber = ((size_t) N) * M * P;

    float *im3d = new float[voxelNumber];
    
    size_t voxelOffset = 0;
    float maxI = -FLT_MAX;
    float minI = FLT_MAX;

    for(V3DLONG z = 0; z < P; z++)
    {
         V3DLONG offsetk = z*M*N;
        for(V3DLONG y = 0; y < M; y++)
        {
            V3DLONG offsetj = y*N;
            for(V3DLONG x = 0; x < N; x++)
            {
              voxelOffset = offsetk + offsetj + x;
              float dataval = data1d[offsetc + voxelOffset];
              if (maxI < dataval) {
                maxI = dataval;
              }
              if (minI > dataval) {
                minI = dataval;
              }
              
              im3d[voxelOffset] = dataval;
              
                //double dataval = data1d[offsetc + offsetk + offsetj + x];
                //Set_Stack_Pixel(data1d_ch1,x,y,z,0,dataval);
            }
        }
    }

    //invert to bright field
    for (size_t i = 0; i < voxelNumber; ++i) {
      im3d[i] = (maxI - im3d[i])/(maxI - minI + 1e-10); 
    }
    QString swc_name = PARA.inimg_file + "_neutu_autotrace.swc";
#if defined(_MAC_APPLICATION_)
    std::string pluginPath = QApplication::applicationDirPath().toStdString() +       "/../../../" + PLUGIN_DIR;
#else
    std::string pluginPath = QApplication::applicationDirPath().toStdString() +       "/" + PLUGIN_DIR;
#endif
    std::string paramFile = pluginPath + "/NeuTuAuto.Params.dat";
    NeuTuAutoTraceOneStack(im3d, M, N, P, paramFile.c_str(), 
       swc_name.toStdString().c_str());

    std::cout << pluginPath << std::endl;

    delete []im3d;


    /*
    JNeuronTracer tracer;
    ZStack stack;
    stack.load(data1d_ch1,true);

    Stack *stackData = stack.c_stack();
    if (C_Stack::mode(stackData) > C_Stack::min(stackData)) {
      std::cout << "Bright field detected." << std::endl;
      Stack_Invert_Value(stackData);
    } else {
      std::cout << "Dark field detected." << std::endl;
    }

    //tracer.initTraceWorkspace(&stack);
    //tracer.initConnectionTestWorkspace();
    ZSwcTree *tree = tracer.trace(stackData);
    tree->setType(2);

    QString swc_name = PARA.inimg_file + "_neutu_autotrace.swc";
    tree->save(swc_name.toStdString().c_str());
    delete tree;
    */

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }


    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
