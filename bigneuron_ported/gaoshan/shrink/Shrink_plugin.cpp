/* Shrink_plugin.cpp
 * This plugin use dynamic threshold and erosion to trace neurons
 * 2015-3-17 : by Gao Shan
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "Shrink_plugin.h"
#include "v3d_interface.h"
#include "stackutil.h"
#include "q_morphology3D.h"
#include <iostream>
Q_EXPORT_PLUGIN2(Shrink, TestPlugin);

using namespace std;

template <class T>
double sum(T *Input,long n)
{
    double Output = 0;
    for (int i = 0;i < n;i++)
    {
        Output+=Input[i];
    }
//    cout<<Output<<"\n"<<endl;
    return Output;
}

/*  function BinaryProcess is adopted from ada_threshold.cpp
 *  which is created by Yang, Jinzhu and Hanchuan Peng, on 11/22/10.
 *  Add dofunc() interface by Jianlong Zhou, 2012-04-18.
 *
 *  modified by Gao Shan
 *  2015/3/19
*/
template <class T>
void BinaryProcess(T *apsInput, T *aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
    V3DLONG i,j,k,n,count;
    double t,temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n=1 ; n <= d ;n++)
                {
                    if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) {temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  apsInput[curpos]-temp/(count);
                aspOutput[curpos]= (t > 0)? 255 : 0;
            }
        }
    }
}

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
        v3d_msg(tr("This plugin use dynamic threshold and erosion to trace neurons. "
            "Developed by Gao Shan, 2015-3-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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


		printf("**** Usage of Shrink tracing **** \n");
		printf("vaa3d -x Shrink -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
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
    ImagePixelType data_type;
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
        data_type = p4DImage->getDatatype();

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
        switch (datatype)
        {
            case 1:
            {
                data_type = V3D_UINT8;
            }
            case 2:
            {
                data_type = V3D_UINT16;
            }
            case 4:
            {
                data_type = V3D_FLOAT32;
            }
        };

    }

    //main neuron reconstruction code
    unsigned char *BinData = new unsigned char[N*M*P];
    BinaryProcess(data1d,BinData, N, M, P, 3, 5);

    Image4DSimple outimg;
    outimg.setData(BinData, N, M, P, 1, data_type);

    cout<<"finish thresholding\n"<<endl;

    QString binary_name = PARA.inimg_file + "_Binary.v3draw";
    char *outimg_name = new char[binary_name.capacity()];
    memcpy(outimg_name,binary_name.toStdString().c_str(),binary_name.capacity());

    callback.saveImage(&outimg, outimg_name);
    v3d_msg("Finish saving output file.",0);

    double* ori = new double[N*M*P];
    double* eroded = new double[N*M*P];
    double* dilated = new double[N*M*P];

    NeuronTree before_pruning;
    NeuronSWC TempNode;
    CKernelMP erode_kernel,dilate_kernel;
    if(!q_create_kernel3D(5,5,5,2,2,2,1,erode_kernel))
    {
        return;
    }
    if(!q_create_kernel3D(5,5,5,2,2,2,1,dilate_kernel))
    {
        return;
    }

    for (int i=0;i<N*M*P;i++)
    {
        ori[i]=BinData[i];
    }
    double RadiusNum = 0;
    long NodeNum = 0;
    while (sum(ori,N*M*P))
    {
        RadiusNum++;
        q_erosion3D(ori,N,M,P,dilate_kernel,eroded);
        q_dilation3D(eroded,N,M,P,dilate_kernel,dilated);
        bool *NewNodes = new bool[N*M*P];
        for (int i=0;i<N*M*P;i++)
        {
            NewNodes[i]=(ori[i]-dilated[i])>0?1:0;
        }
        for (int i=0;i<N;i++)
        {
            for (int j=0;j<M;j++)
            {
                for (int k=0;k<P;k++)
                {
                    long index=N*M*k+N*j+i;
                    if (NewNodes[index])
                    {
                        TempNode.n = NodeNum;
                        NodeNum++;
                        TempNode.r = RadiusNum;
                        TempNode.x = i;
                        TempNode.y = j;
                        TempNode.z = k;
                        before_pruning.listNeuron.append(TempNode);
//                        cout<<index<<endl;
                    }
                }
            }
        }
        for (int i=0;i<N*M*P;i++)
        {
            ori[i]=eroded[i];
        }
    }


    //Output

    QString swc_name = PARA.inimg_file + "_Shrink.swc";
//    NeuronTree nt;
    before_pruning.name = "Shrink tracing";
    writeSWC_file(swc_name.toStdString().c_str(),before_pruning);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}



