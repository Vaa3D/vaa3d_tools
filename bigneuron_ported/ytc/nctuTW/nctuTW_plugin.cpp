/* nctuTW_plugin.cpp
 * This is the implementation of the tracing algorithm published in PLoS Computational Biology, "High-throughput Computer Method for 3D Neuronal Structure Reconstruction from the Image Stack of the Drosophila Brain and Its Applications", September 13, 2013. Implemented by P. C. Lee, K. W. He, C. C. Cheng, Y. T. Ching.
 * 2015-5-23 : by P. C. Lee, K. W. He, C. C. Cheng, Y. T. Ching.
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "nctuTW_plugin.h"
#include "neurontracing.h"
#include "QDlgPara.h"

Q_EXPORT_PLUGIN2(nctuTW, nctuTW);

using namespace std;

struct input_PARA
{
    QString inimg_file,inmarker_file;
    double threshold;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList nctuTW::menulist() const
{
	return QStringList() 
        <<tr("start tracing")
		<<tr("about");
}

QStringList nctuTW::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void nctuTW::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("start tracing"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
        v3d_msg(QString("This is the implementation of the tracing algorithm published in PLoS Computational Biology, \"High-throughput Computer Method for 3D Neuronal Structure Reconstruction from the Image Stack of the Drosophila Brain and Its Applications\", September 13, 2013. "
            "Implemented by P. C. Lee, K. W. He, C. C. Cheng, Y. T. Ching, 2015-5-23. Version %1").arg(getPluginVersion()));
	}
}

bool nctuTW::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
        PARA.inmarker_file = paras.empty() ? "" : paras[k]; if(PARA.inmarker_file == "NULL") PARA.inmarker_file = ""; k++;
        PARA.threshold = (paras.size() >= k+1) ? atof(paras[k]) : 0.9;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of nctuTW tracing **** \n");
        printf("vaa3d -x nctuTW -f tracing_func -i <inimg_file> -p <inmarker_file> <threshold>\n");
        printf("inimg_file       The input image\n");
        printf("inmarker_file    If no input marker file, please set this para to NULL and it will detect soma automatically.\n");
        printf("threshold        Default 0.9, otherwise the threshold (from 0 to 1) specified by a user will be used.\n");

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
    v3dhandle curwin;
    if(bmenu)
    {
        curwin = callback.currentImageWindow();
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
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];

    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
//  ofstream oImgInfofile;
    string sImgPath = PARA.inimg_file.toStdString();
    g_sAppDir = normalizePath(parentPath(sImgPath));

//    g_sImageInfoPath = g_sAppDir + g_sImageInfoName;
//    g_sInputRawFilePath = g_sAppDir + g_sInputRawFileName;
    QString swc_name = PARA.inimg_file + "_nctuTW.swc";
    g_sOutSwcFilePath = swc_name.toStdString();
    //g_sOutSwcFilePath = g_sAppDir+ g_sOutSwcFileName;

    size_t nDataSize = N * M * P * sc ;

    width = N;
    height = M;
    zSize = P;

//    oImgInfofile.open(g_sImageInfoPath.data(), ios::out|ios::binary);
//    if(oImgInfofile.is_open()){
//        oImgInfofile << N << " " << M << " " << P << endl;
//        oImgInfofile.close();
//    }

    imgBuf_raw = new unsigned char [nDataSize];
    memcpy(imgBuf_raw, data1d, nDataSize);

//    ofstream oRawFile;
//    oRawFile.open(g_sInputRawFilePath.data(), ios::out|ios::binary);
//    if(oRawFile.is_open()){
//        oRawFile.write((char *)data1d, nDataSize);
//        oRawFile.close();
//    }


/*
    // For debug
    if(nParaState == QDialog::Accepted){
        std::string sPara = "Threshold = " + toString(g_rThreshold) + "; " +
                            "Soma = (" + toString(g_nSomaX) + ", " + toString(g_nSomaY) + ", " + toString(g_nSomaZ) + ")";
        v3d_msg(QString(sPara.c_str()));
    }
*/
    QDlgPara* pqDlgPara;

    g_nSomaX = -1;
    g_nSomaY = -1;
    g_nSomaZ = -1;

    int nParaState = 1;

    if(bmenu)       //ui
    {
        LandmarkList Landmark;
        Landmark = callback.getLandmark(curwin);
        if(Landmark.size()>0)
        {
            g_nSomaX = Landmark.at(0).x-1;
            g_nSomaY = Landmark.at(0).y-1;
            g_nSomaZ = Landmark.at(0).z-1;
        }
        pqDlgPara = new QDlgPara(parent);
        nParaState = pqDlgPara->exec();

    }
    else        //command line
    {    
        QList<ImageMarker> file_inmarkers;
        if(!PARA.inmarker_file.isEmpty())
        {
            file_inmarkers = readMarker_file(PARA.inmarker_file);
            g_nSomaX = file_inmarkers.at(0).x;
            g_nSomaY = file_inmarkers.at(0).y;
            g_nSomaZ = file_inmarkers.at(0).z;
        }
        else
        {
           g_nSomaX = -1;
           g_nSomaY = -1;
           g_nSomaZ = -1;
        }
        g_rThreshold = PARA.threshold;
    }


    if(nParaState)
    {
        NeuronTracingMain();
        v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
    }

    if(bmenu) delete pqDlgPara;
     bmenu=true;
    //Output

//    NeuronTree nt;
    //QString swc_name = PARA.inimg_file + "_nctuTW.swc";
//	nt.name = "nctuTW";
//    writeSWC_file(swc_name.toStdString().c_str(),nt);

    //QString swc_name(g_sOutSwcFilePath.data());
    //Output

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    delete [] imgBuf_raw;

    return;
}
