/* NeuroGPSTree_plugin.cpp
 * Trace dense network with multi-Neuron
 * Permit to provide soma swc file for the position of somas
 * 2015-3-16 : by ZhouHang
 * Main algorithm: Quan Tingwei, Zhou Hang, Zeng Shaoqun
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "NeuroGPSTree_plugin.h"
Q_EXPORT_PLUGIN2(NeuroGPSTree, NeuroGPSTreePlugin);

#include "Function/binaryfilter.h"
#include "Function/Trace/bridgebreaker.h"
#include "Function/Trace/neurotreecluster.h"
#include "Function/Trace/tracefilter.h"

using namespace std;


QStringList NeuroGPSTreePlugin::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
        << tr("help");
}

QStringList NeuroGPSTreePlugin::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void NeuroGPSTreePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        bool ok;
        QString paraStr = QInputDialog::getText(parent, tr("Input trace parameter"),
                                                tr("Please Input x resolution, y resolution, z resolution and binaryThreshold"),
                                                QLineEdit::Normal,tr("1 1 2 6"),&ok);
        if(!ok) return;
        QStringList listStr = paraStr.split(' ');
        PARA.xRes_ = listStr[0].toDouble();
        PARA.yRes_ = listStr[1].toDouble();
        PARA.zRes_ = listStr[2].toDouble();
        PARA.binaryThreshold = listStr[3].toDouble();
        paraStr = QInputDialog::getText(parent, tr("Input soma position file"),
                                                tr("Please Input soma position file path"),
                                                QLineEdit::Normal,tr("NULL"), &ok);
        if(!ok) return;
        PARA.swcfile = paraStr.toStdString();
        //input_PARA PARA;//set as global varient
        //PARA.xRes_ = 1;
        //PARA.yRes_ = 1;
        //PARA.zRes_ = 2;
        //PARA.binaryThreshold = 6;
        //PARA.swcfile = "NULL";
        reconstruction_func(callback,parent,PARA,bmenu);
	}
	else
	{
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by ZhouHang, 2015-3-16"));
	}
}

bool NeuroGPSTreePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //where is the construct function ?
    //if(!OrigImage) OrigImage = std::shared_ptr<SVolume>(new OrigImage());
    //v3d_msg(tr("dofunc"));
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        //sinput_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else{
            PARA.inimg_file = infiles[0];
            int k = 0;
         //try to use as much as the default value in the PARA_APP2 constructor as possible
            PARA.xRes_ = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  ++k;
            PARA.yRes_ = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  ++k;
            PARA.zRes_ = (paras.size() >= k+1) ? atoi(paras[k]) : 2;  ++k;
            PARA.binaryThreshold = (paras.size() >= k+1) ? atoi(paras[k]) : 6;  ++k;
            PARA.swcfile = (paras.size() >= k+1) ? std::string(paras[k]) : std::string("NULL");  ++k;
#ifdef _WIN32
            PARA.threadNum = (paras.size() >= k+1) ? atoi(paras[k]) : omp_get_max_threads();  ++k;
#endif
            reconstruction_func(callback,parent,PARA,bmenu);
        }
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of NeuroGPSTree tracing **** \n");
        printf("vaa3d -x NeuroGPSTree -f tracing_func -i <inimg_file> -p <XRESOLUTION> <YRESOLUTION> <ZRESOLUTION> <BINARYTHRESHOLD> <SWCFILE> <ACCELERATE>\n");
        printf("inimg_file       The input image\n");
        printf("RESOLUTION       X Y Z Resolution of dataset for tracing(default 1 1 2).\n");
        printf("BINARYTHRESHOLD  the strength of binarization\n");
        printf("SWCFILE          swc file contain the position of somas\n");
        printf("ACCELERATE       accelerate binarization and tracing \n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

    return true;
}

bool NeuroGPSTreePlugin::ConvertVaa3dImg2NGImg(UChar *data1d,
                                               int X, int Y, int Z,
                                               double xR, double yR, double zR,
                                               std::shared_ptr<INeuronDataObject> img)
{
    //initialize
    if(!data1d){
        printf("error ocured in ConvertVaa3dImg2NGImg.\n");
        return false;
    }

    std::shared_ptr<SVolume> tmpImg = std::dynamic_pointer_cast<SVolume>(img);
    //printf("1\n");
    tmpImg->SetSize(X,Y,Z);
    //printf("1\n");
    tmpImg->SetResolution(xR, yR, zR);
    //printf("1\n");
    int XY = X * Y;
    for(int i = 0; i < X; ++i){
        for(int j = 0; j < Y; ++j){
            for( int ij = 0; ij < Z; ++ij){
                tmpImg->operator ()(i,j,ij) = data1d[i + j * X + ij * XY] * 4;//TODO: I will ask prof. Quan for more details.
            }
        }
    }
    //printf("2\n");
    return true;
}

bool NeuroGPSTreePlugin::SaveSeperateTree(const QString &filename, const QStringList &infostring)
{
    //
    QString curFile = filename;
    if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
    {
        curFile = QFileDialog::getSaveFileName(0,
                                               "Select a SWC file to ssave the neuronal or relational data... ",
                                               ".swc",
                                               QObject::tr("Neuron structure file (*.swc);;(*.*)"
                                                           ));
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("save file: %1").arg(curFile), false);
#endif

        if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
            return false;
    }

    std::shared_ptr<SeperateTree> tmpSeperateTree =
            std::dynamic_pointer_cast<SeperateTree>(seperateTree);
    std::vector<std::vector<VectorVec5d> >& writeTree = tmpSeperateTree->GetTree();
    const std::vector<int> &typeList = tmpSeperateTree->GetTypeList();

    if(PARA.swcfile == "NULL"){
        //write
        FILE * fp = fopen((curFile ).toLatin1(), "wt");
        if (!fp)
        {
#ifndef DISABLE_V3D_MSG
            v3d_msg("Could not open the file to save the neuron.");
#endif
            return false;
        }

        if (!infostring.isEmpty())
        {
            for (int j=0;j<infostring.size();j++)
                fprintf(fp, "#%s\n", qPrintable(infostring.at(j).trimmed()));
        }

        fprintf(fp, "##n,type,x,y,z,radius,parent\n");
        int globalIndex = 1;
        for(size_t i = 0; i < writeTree.size();++i){
            std::vector<VectorVec5d> &localTree = writeTree[i];
            for(size_t j = 0; j < localTree.size();++j){
                //One curve
                VectorVec5d &localCurve = localTree[j];
                //fprintf(fp,"%d %d %lf %lf %lf %lf -1\n", globalIndex, typeList[i],localCurve[0](0),
                //localCurve[0](1), localCurve[0](2),1.0);
                fprintf(fp,"%d 2 %lf %lf %lf %lf -1\n", globalIndex, localCurve[0](0),
                        localCurve[0](1), localCurve[0](2),1.0);
                ++globalIndex;
                for(size_t k = 1; k < localCurve.size();++k){
                    fprintf(fp,"%d 2 %lf %lf %lf %lf %d\n", globalIndex, localCurve[k](0),
                            localCurve[k](1), localCurve[k](2),1.0, globalIndex - 1);
                            ++globalIndex;
                }
            }
        }
        fclose(fp);
    }else{
        char line[256];
        for(size_t i = 0; i < writeTree.size();++i){
            sprintf(line, "%05d.", i + 1);
            FILE * fp = fopen((curFile.section('.',0,-2) +
                               QString(line) + curFile.section('.', -1) ).toLatin1(), "wt");
            if (!fp)
            {
    #ifndef DISABLE_V3D_MSG
                v3d_msg("Could not open the file to save the neuron.");
    #endif
                return false;
            }

            if (!infostring.isEmpty())
            {
                for (int j=0;j<infostring.size();j++)
                    fprintf(fp, "#%s\n", qPrintable(infostring.at(j).trimmed()));
            }

            fprintf(fp, "##n,type,x,y,z,radius,parent\n");
            std::vector<VectorVec5d> &localTree = writeTree[i];
            int globalIndex = 1;
            for(size_t j = 0; j < localTree.size();++j){
                //One curve

                VectorVec5d &localCurve = localTree[j];
                //fprintf(fp,"%d %d %lf %lf %lf %lf -1\n", globalIndex, typeList[i],localCurve[0](0),
                //localCurve[0](1), localCurve[0](2),1.0);
                fprintf(fp,"%d %d %lf %lf %lf %lf -1\n", globalIndex,typeList[i], localCurve[0](0),
                        localCurve[0](1), localCurve[0](2),1.0);
                ++globalIndex;
                for(size_t k = 1; k < localCurve.size();++k){
                    fprintf(fp,"%d %d %lf %lf %lf %lf %d\n", globalIndex,typeList[i], localCurve[k](0),
                            localCurve[k](1), localCurve[k](2),1.0, globalIndex - 1);
                            ++globalIndex;
                }
            }
            fclose(fp);
        }
    }

#ifndef DISABLE_V3D_MSG
    v3d_msg(QString("done with saving file: ")+filename, false);
#endif
    return true;
}

void NeuroGPSTreePlugin::reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG im_sz[4];
    if(!OrigImage) OrigImage = std::shared_ptr<SVolume>(new SVolume());

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

        im_sz[0] = N;
        im_sz[1] = M;
        im_sz[2] = P;
        im_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, im_sz, datatype))
        {
            fprintf (stderr, "nima, Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }

        if (!data1d)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        N = im_sz[0];
        M = im_sz[1];
        P = im_sz[2];
        sc = im_sz[3];

    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
    X = N;
    Y = M;
    Z = P;
    //printf("%d %d %d\n", N, M, P);

    if(sc != 1){
        QMessageBox::information(0, "", "Only support single channel image.");
        return;
    }

    //convert vaa3d image to neurogps image
    printf("prepare to convert vaa3d img to neurogps image.\n");
    if(!ConvertVaa3dImg2NGImg(data1d, N, M, P, PARA.xRes_, PARA.yRes_,
                              PARA.zRes_, OrigImage)){
        printf("cannot convert vaa3d img to neurogps image.\n");
        if(!bmenu)
        {
            if(data1d) {delete []data1d; data1d = 0;}//avoid memory leak
        }
        return;
    }

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    printf("convert vaa3d img to neurogps image.\n");

    //read soma file
    if(PARA.swcfile == std::string("NULL")){
        soma = std::shared_ptr<Soma>(new Soma());

    } else{
        soma = std::shared_ptr<Soma>(new Soma());
        NeuronTree vswc = readSWC_file(PARA.swcfile.c_str());
        soma = std::shared_ptr<Soma>(new Soma());
        std::shared_ptr<Soma> tmpSoma = std::dynamic_pointer_cast<Soma>(soma);
        QList <NeuronSWC> &listNeuron = vswc.listNeuron;
        for(int i = 0; i < listNeuron.size(); ++i){
            NeuronSWC &item = listNeuron[i];
            tmpSoma->push_back(Cell(i, item.x, Y - item.y, item.z / PARA.zRes_, 1));
        }
    }
    printf("soma file has been read.\n");
    //binary
    AutoBinaryImage();
    //trace
    AutoTrace();

    //Output

    QString swc_name = PARA.inimg_file + "_NeuroGPSTree.swc";
    //NeuronTree nt;
    //nt.name = "tracing method";
    //writeSWC_file(swc_name.toStdString().c_str(),nt);
    //save swc
    if(!SaveSeperateTree(swc_name, QStringList()
                         << tr("compute parameter:")
                         << tr("xRes:%1 yRes:%2 zRes:%3").arg(N).arg(M).arg(P)
                         << tr("binaryTreshold:%1").arg(PARA.binaryThreshold))){
        printf("cannot save trees.\n");
        return;
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}

void NeuroGPSTreePlugin::AutoBinaryImage()
{
    NGBinaryFilter filter = BinaryFilter::New();
    filter->SetInput(OrigImage);
    filter->SetThreadNum(PARA.threadNum);
    filter->SetThreshold(PARA.binaryThreshold);
    filter->Update();
    BinImage = filter->ReleaseData();
    BackImage = filter->ReleaseBackNoiseImage();
    binPtSet = filter->GetBinPtSet();
    printf("Image Binarization complete.\n");
}

void NeuroGPSTreePlugin::AutoTrace()
{
    printf("begin tracing.\n");
    NGTraceFilter traceFilter = TraceFilter::New();
    traceFilter->SetInput(OrigImage);
    traceFilter->SetInputBack(BackImage);
    traceFilter->SetInputBin(BinImage);
    traceFilter->SetSoma(soma);
    traceFilter->Update();
    tree = traceFilter->ReleaseData();
    treeConInfo = traceFilter->GetConnect();

    //std::shared_ptr<TreeCurve> tmptree = std::dynamic_pointer_cast<TreeCurve>(tree);

    printf("begin break.\n");
    NGBridgeBreaker breaker = BridgeBreaker::New();
    breaker->SetInput(traceFilter->GetConnect());
    breaker->SetInputOrigImage(OrigImage);
    breaker->SetInputTree(tree);
    breaker->SetInputSoma(soma);
    breaker->Update();

    std::shared_ptr<INeuronDataObject> breakCon = breaker->ReleaseData();
    std::shared_ptr<TreeConnect> tmpConnect = std::dynamic_pointer_cast<TreeConnect>(breakCon);

    printf("begin clustre.\n");
    NGNeuronTreeCluster treeCluster = NeuroTreeCluster::New();
    treeCluster->SetInput(breakCon);
    treeCluster->SetInputDeleteID(breaker->getResultIndex());
    treeCluster->SetInputCurve(tree);
    treeCluster->SetInputSoma(soma);
    treeCluster->SetInputOrigImage(OrigImage);
    treeCluster->Update();

    seperateTree = treeCluster->ReleaseData();
    printf("Tracing complete.\n");
}
