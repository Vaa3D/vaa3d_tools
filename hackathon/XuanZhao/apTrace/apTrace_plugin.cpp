/* apTrace_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-8-12 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "apTrace_plugin.h"

#include "mysurface.h"
#include "imagectrl.h"

//#include "pca.h"

Q_EXPORT_PLUGIN2(apTrace, TestPlugin);

using namespace std;

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
        <<tr("segment")
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
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-8-12"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("tracing_func"))
	{

        const char* file=(infiles.size()>=1)?infiles[0]:"";
        unsigned char* pdata=0;
        V3DLONG sz[4]={0,0,0,0};
        int datatype=0;
        simple_loadimage_wrapper(callback,file,pdata,sz,datatype);
        qDebug()<<"put image..."<<endl;
        ImageCtrl c(pdata,sz);

        vector<vector<vector<unsigned char>>> image;
        c.Data1d_to_3d(image);
        unsigned char*** data3d = c.get3ddata();

//        c.corrode(image);
//        c.corrode(image);
//        QString filename = (outfiles.size()>=2)?outfiles[1]:"";
//        c.SaveImage(filename,callback);

        QString markerfile=(outfiles.size()>=1)?outfiles[0]:"";

        apTracer ap;
        vector<assemblePoint> assemblepoints;

        double thres = c.getMode();

        ap.initialAssemblePoint(assemblepoints,image,data3d,sz,thres);
        ap.writeAssemblePoints(markerfile,assemblepoints);

        QList<ImageMarker> listMarker;
        for(int i=0; i<assemblepoints.size() ;++i)
        {
            qDebug()<<"cor: "<<assemblepoints[i].x<<" "<<assemblepoints[i].y<<" "<<assemblepoints[i].z<<endl;
            assemblepoints[i].getDirection(data3d,sz);
            assemblepoints[i].dire.norm_dir();
            assemblepoints[i].showDirection(listMarker);
        }

        QString directionfile = (outfiles.size()>=3) ? outfiles[2]:"";

        writeMarker_file(directionfile,listMarker);

        NeuronTree nt;
        vector<segment> segments;
//        ap.direc_trace(assemblepoints,image,segments,sz);
//        ap.connectPointandSegment(assemblepoints,segments);
        ap.trace(assemblepoints,image,data3d,nt,sz);

//        for(int i=0; i<segments.size(); ++i)
//        {
//            for(int j=0; j<segments[i].points.size(); ++j)
//            {
//                nt.listNeuron.push_back(segments[i].points[j]);
//            }
//        }



        QString eswcfile=(outfiles.size()>=2)?outfiles[1]:"";
        writeESWC_file(eswcfile,nt);

        c.getSegImage(assemblepoints);

        unsigned char* segdata = c.getdata();

        QString tiffile = (outfiles.size()>=4)?outfiles[3]:"";
        simple_saveimage_wrapper(callback,tiffile.toStdString().c_str(),segdata,sz,datatype);

        if(pdata) delete[] pdata;
        if(segdata) delete[] segdata;

        for(int i=0; i<sz[2]; ++i)
        {
            for(int j=0; j<sz[1]; ++j)
            {
                delete[] data3d[i][j];
            }
            delete[] data3d[i];
        }
        delete[] data3d;
	}
    else if (func_name == tr("segment"))
    {
        const char* file=(infiles.size()>=1)?infiles[0]:"";
        unsigned char* pdata=0;
        V3DLONG sz[4]={0,0,0,0};
        int datatype=0;
        simple_loadimage_wrapper(callback,file,pdata,sz,datatype);
        qDebug()<<"put image..."<<endl;
        ImageCtrl c(pdata,sz);

        vector<vector<vector<unsigned char>>> image;
//        c.histogram();
//        c.Data1d_to_3d(image);
//        c.corrode(image,2);
        c.segment0();
        unsigned char* segdata = c.getdata();
        QString tiffile = (outfiles.size()>=1)?outfiles[0]:"";
        simple_saveimage_wrapper(callback,tiffile.toStdString().c_str(),segdata,sz,datatype);

        if(pdata) delete[] pdata;
        if(segdata) delete segdata;
    }
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of apTrace tracing **** \n");
		printf("vaa3d -x apTrace -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
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

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

    //Output
    NeuronTree nt;
	QString swc_name = PARA.inimg_file + "_apTrace.swc";
	nt.name = "apTrace";
    writeSWC_file(swc_name.toStdString().c_str(),nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
