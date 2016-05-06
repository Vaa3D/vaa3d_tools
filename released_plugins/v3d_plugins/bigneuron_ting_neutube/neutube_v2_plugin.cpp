/* neutube_v2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-2-9 : by Zhi Zhou 
 */
 
#include "v3d_message.h"
#include <vector>
#include <QElapsedTimer>
#include "basic_surf_objs.h"

#include "neutube_v2_plugin.h"

#include "zqtheader.h" //Start including neutube headers
#include "zneurontracer.h"
#include "zstack.hxx"
#include "zswctree.h"
#include "zqtheader_undef.h" //neutube header end
#include "tz_math.h"

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
    if (func_name == tr("neutube_trace"))
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
        printf("vaa3d -x neuTube -f neutube_trace -i <inimg_file> -p <channel> <other parameters>\n");
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
    QElapsedTimer timer;
    timer.start();
    std::cout << "Start tracing (7) ... " << std::endl;

    std::cout << "Preparing for data ..." << std::endl;

    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    int xintv = 0;
    int yintv = 0;
    int zintv = 0;

    std::cout << "Data size: " <<N << "x" << M << "x" << P <<std::endl;
    Stack *data1d_ch1 = NULL;
    if (pagesz > (V3DLONG) 1024 * 1024 * 2048) {
      std::cout << pagesz << std::endl;
      std::cout << "Too big." << N << "x" << M << "x" << P << "--" <<
       " Downsampling ..." << std::endl;
      int intv = iround(sqrt(double(pagesz) / 1024/1024/1024));
      xintv = intv;
      yintv = intv;
      
      Stack tmpStack;
      tmpStack.kind = GREY;
      tmpStack.width = N;
      tmpStack.height = M;
      tmpStack.depth = P;
      tmpStack.array = data1d + offsetc;
      data1d_ch1= Downsample_Stack_Max(&tmpStack, xintv, yintv, zintv, NULL);
      std::cout << "Dowsample ratio: " << xintv << " " << yintv << " " << zintv << std::endl;
    } else {
      data1d_ch1 = C_Stack::make(GREY,(int)N,(int)M,(int)P);
      size_t offset = 0;
      for(V3DLONG z = 0; z < P; z++)
      {
        V3DLONG offsetk = z*M*N;
        for(V3DLONG y = 0; y < M; y++)
        {
          V3DLONG offsetj = y*N;
          for(V3DLONG x = 0; x < N; x++)
          {
            data1d_ch1->array[offset++] = 
              data1d[offsetc + offsetk + offsetj + x];
            //Set_Stack_Pixel(data1d_ch1,x,y,z,0,dataval);
          }
        }
      }
    }

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    ZNeuronTracer tracer;
    ZStack stack;
    stack.load(data1d_ch1,true);
    tracer.initTraceWorkspace(&stack);
    Print_Trace_Workspace(tracer.getTraceWorkspace());
    tracer.initConnectionTestWorkspace();
    ZSwcTree *tree = tracer.trace(&stack, true);

    std::cout << "Total tracing time: " << timer.elapsed() << std::endl;

    if (xintv > 0 || yintv > 0 || zintv > 0) {
      tree->rescale(xintv + 1, yintv + 1, zintv + 1);
    }

    QString swc_name = PARA.inimg_file + "_neutube.swc";
    tree->save(swc_name.toStdString().c_str());
    delete tree;


    v3d_msg(QString("Now you can drag and drop the generated swc file [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
