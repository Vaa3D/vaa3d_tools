/* NeuroGPSTree_plugin.cpp
 * Trace dense network with multi-Neuron
 * Permit to provide soma swc file for the position of somas
 * 2016-1-1 : by ZhouHang
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
#include "Function/Trace/WeakSWCFilter.h"
#include "Function/IO/treewriter.h"
#ifdef _WIN32
#include <omp.h>
#endif
#ifdef __linux
#include <omp.h>
#endif

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
                                                tr("Please Input x resolution, y resolution, z resolution, binaryThreshold, trace Value, linear enhance value"),
                                                QLineEdit::Normal,tr("1 1 1 6 0 255"),&ok);
        if(!ok) return;
        QStringList listStr = paraStr.split(' ');
        PARA.xRes_ = listStr[0].toDouble();
        PARA.yRes_ = listStr[1].toDouble();
        PARA.zRes_ = listStr[2].toDouble();
        PARA.binaryThreshold = listStr[3].toDouble();
        PARA.traceValue = listStr[4].toDouble();
        PARA.enhanceValue = listStr[5].toDouble();
        paraStr = QInputDialog::getText(parent, tr("Input soma position file"),
                                                tr("Please Input soma position file path"),
                                                QLineEdit::Normal,tr("NULL"), &ok);

        PARA.threadNum = omp_get_max_threads();
        if(!ok) return;
        PARA.swcfile = paraStr.toStdString();

        reconstruction_func(callback,parent,PARA,bmenu);
    }
    else
    {
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by ZhouHang, 2016-1-1"));
    }
}

bool NeuroGPSTreePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
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
            PARA.xRes_ = (paras.size() >= k+1) ? atof(paras[k]) : 1;  ++k;
            PARA.yRes_ = (paras.size() >= k+1) ? atof(paras[k]) : 1;  ++k;
            PARA.zRes_ = (paras.size() >= k+1) ? atof(paras[k]) : 2;  ++k;
            PARA.binaryThreshold = (paras.size() >= k+1) ? atof(paras[k]) : 6;  ++k;
            PARA.traceValue = (paras.size() >= k+1) ? atof(paras[k]) : 0;  ++k;
            PARA.enhanceValue = (paras.size() >= k+1) ? atof(paras[k]) : 0;  ++k;
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
        printf("vaa3d -x NeuroGPSTree -f tracing_func -i <inimg_file> -p <XRESOLUTION> <YRESOLUTION> <ZRESOLUTION> <BINARYTHRESHOLD> <TRACEVALUE> <ENHANCE> \
               <SWCFILE> <ACCELERATE>\n");
        printf("inimg_file       The input image\n");
        printf("RESOLUTION       X Y Z Resolution of dataset for tracing(default 1 1 2).\n");
        printf("BINARYTHRESHOLD  the strength of binarization\n");
        printf("TRACEVALUE		 the strength of trace\n");
        printf("ENHANCE		     linear enhance the image.\n");
        printf("SWCFILE          swc file contain the position of somas\n");
        printf("ACCELERATE       accelerate binarization and tracing \n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

    }
    else return false;

    return true;
}

#ifdef _WIN32
bool NeuroGPSTreePlugin::ConvertVaa3dImg2NGImg(UChar *data1d,
                                               int X, int Y, int Z,
                                               double xR, double yR, double zR,
                                               std::tr1::shared_ptr<INeuronDataObject> img)
#else
bool NeuroGPSTreePlugin::ConvertVaa3dImg2NGImg(UChar *data1d,
                                               int X, int Y, int Z,
                                               double xR, double yR, double zR,
                                               std::shared_ptr<INeuronDataObject> img)
#endif
{
    //initialize
    if(!data1d){
        printf("error ocured in ConvertVaa3dImg2NGImg.\n");
        return false;
    }
    //
    xres_ = xR;
    yres_ = yR;
    zres_ = zR;
    //printf("res:%lf %lf %lf",xres_, yres_, zres_);
    int rx, ry, rz;
    SetRealResolution(xres_, X, xScale_, rx);
    SetRealResolution(yres_, Y, yScale_, ry);
    SetRealResolution(zres_, Z, zScale_, rz);
    double ratio = 4.0 / double(xScale_ * yScale_ * zScale_);
    xres_ *= xScale_;
    yres_ *= yScale_;
    zres_ *= zScale_;
    //printf("res:%lf %lf %lf",xres_, yres_, zres_);
    //system("pause");

#ifdef _WIN32
    std::tr1::shared_ptr<SVolume> tmpImg = std::tr1::dynamic_pointer_cast<SVolume>(img);
#else
    std::shared_ptr<SVolume> tmpImg = std::dynamic_pointer_cast<SVolume>(img);
#endif
    //printf("1\n");
    tmpImg->SetSize(rx,ry,rz);
    //printf("1\n");
    tmpImg->SetResolution(xres_, yres_, zres_);

    double enhanceRatio = std::abs(PARA.enhanceValue - 255.0) < 0.1 ? 1.0 : (1020.0 - 5.0) / (4.0 * PARA.enhanceValue);
    int XY = X * Y;
    int temp(0);
    //2015-8-13
    for(int nCur = 0; nCur < rz; ++nCur){
        for (int k = 0; k < zScale_; ++k) {
            if(xScale_ * yScale_ != 1){
                for (int i = 0; i < rx; ++ i)
                    for (int j = 0; j < ry; ++ j){
                        temp = 0;
                        for(int ii = 0; ii < xScale_; ++ii){
                            for(int jj = 0; jj < yScale_; ++jj){
                                //temp += TIFFGetG(raster[i*xScale_ + ii + (j * yScale_ + jj) * x]);
                                temp += data1d[i*xScale_ + ii + (j * yScale_ + jj) * X + nCur * XY];
                            }
                        }
                        tmpImg->operator ()(i,ry - j - 1,nCur) += temp;
                    }
            }
            else{
                for (int i = 0; i < rx; ++ i)
                    for (int j = 0; j < ry; ++ j){
                        temp = data1d[i + j * X + nCur * XY];
                        tmpImg->operator ()(i,ry - j - 1,nCur) += temp;//2015-8-13
                    }
            }
        }
        if (std::abs(PARA.enhanceValue - 255.0) < 0.1) {//no linear enhance
            for (int i = 0; i < rx; ++ i)
                for (int j = 0; j < ry; ++ j){
                    tmpImg->operator ()(i,j,nCur) *= ratio;//2015-8-13
                }
        } else{//linear enhance
            for (int i = 0; i < rx; ++ i)
                for (int j = 0; j < ry; ++ j){
                    tmpImg->operator ()(i,j,nCur) *= ratio;//2015-8-13
                    if (tmpImg->operator ()(i,j,nCur) > PARA.enhanceValue) tmpImg->operator ()(i,j,nCur) = 1020;
                    else tmpImg->operator ()(i,j,nCur) =  (unsigned short)(double(tmpImg->operator ()(i,j,nCur)) * enhanceRatio);
                    //image->operator ()(i,j,nCur) = (std::max)(unsigned short(5), image->operator ()(i,j,nCur));//2016-1-4
                }
        }
    }

    if (rx * ry * rz <  1000000000 ) {
        for(int k = 0; k < rz; ++k){
            GetHist(k);
            FillBlackArea(k);
        }
    }

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

#ifdef _WIN32
    std::tr1::shared_ptr<SeperateTree> tmpSeperateTree =
        std::tr1::dynamic_pointer_cast<SeperateTree>(seperateTree);
#else
    std::shared_ptr<SeperateTree> tmpSeperateTree =
        std::dynamic_pointer_cast<SeperateTree>(seperateTree);
#endif

    std::vector<std::vector<VectorVec5d> >& writeTree = tmpSeperateTree->GetTree();
    const std::vector<int> &typeList = tmpSeperateTree->GetTypeList();

#ifdef _WIN32
    std::tr1::shared_ptr<SVolume> tmpImg = std::tr1::dynamic_pointer_cast<SVolume>(OrigImage);
#else
    std::shared_ptr<SVolume> tmpImg = std::dynamic_pointer_cast<SVolume>(OrigImage);
#endif

    if(PARA.swcfile == "NULL"){
        //write
        /*FILE * fp = fopen((curFile ).toLatin1(), "wt");
        if (!fp)
        {
#ifndef DISABLE_V3D_MSG
            v3d_msg("Could not open the file to save the neuron.");
#endif
            return false;
        }*/

        /*if (!infostring.isEmpty())
        {
            for (int j=0;j<infostring.size();j++)
                fprintf(fp, "#%s\n", qPrintable(infostring.at(j).trimmed()));
        }*/

        NGTreeWriter writer = TreeWriter::New();
        writer->SetInput(seperateTree);
        //writer->SetInputXYZExtend(tmpImg->XResolution(), tmpImg->YResolution(), tmpImg->ZResolution());
        writer->SetInputXYZExtend(xScale_, yScale_, zScale_);
        writer->SetOutputFileName(curFile.toStdString());
        writer->Update();
        //double Y = tmpImg->y();

        //fprintf(fp, "##n,type,x,y,z,radius,parent\n");
        //int globalIndex = 1;
        //for(size_t i = 0; i < writeTree.size();++i){
        //    std::vector<VectorVec5d> &localTree = writeTree[i];
        //    for(size_t j = 0; j < localTree.size();++j){
        //        //One curve
        //        VectorVec5d &localCurve = localTree[j];
        //        //fprintf(fp,"%d %d %lf %lf %lf %lf -1\n", globalIndex, typeList[i],localCurve[0](0),
        //        //localCurve[0](1), localCurve[0](2),1.0);
        //        fprintf(fp,"%d 1 %lf %lf %lf %lf -1\n", globalIndex, localCurve[0](0) * tmpImg->XResolution(),
        //                (localCurve[0](1)) * tmpImg->YResolution(), localCurve[0](2) * tmpImg->ZResolution(),1.0);
        //        ++globalIndex;
        //        for(size_t k = 1; k < localCurve.size();++k){
        //            fprintf(fp,"%d 3 %lf %lf %lf %lf %d\n", globalIndex, localCurve[k](0) * tmpImg->XResolution(),
        //                    (localCurve[k](1)) * tmpImg->YResolution(), localCurve[k](2) * tmpImg->ZResolution(),1.0, globalIndex - 1);
        //                    ++globalIndex;
        //        }
        //    }
        //}
        //fclose(fp);
    }else{
        /*char line[256];
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
            }*/

            NGTreeWriter writer = TreeWriter::New();
            writer->SetInput(seperateTree);
            writer->SetInputXYZExtend(xScale_, yScale_, zScale_);
            writer->SetOutputFileName(curFile.toStdString());
            writer->Update();

            /*if (!infostring.isEmpty())
            {
                for (int j=0;j<infostring.size();j++)
                    fprintf(fp, "#%s\n", qPrintable(infostring.at(j).trimmed()));
            }*/

    //        fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    //        std::vector<VectorVec5d> &localTree = writeTree[i];
    //        int globalIndex = 1;
    //        for(size_t j = 0; j < localTree.size();++j){
    //            //One curve

    //            VectorVec5d &localCurve = localTree[j];
    //            //fprintf(fp,"%d %d %lf %lf %lf %lf -1\n", globalIndex, typeList[i],localCurve[0](0),
    //            //localCurve[0](1), localCurve[0](2),1.0);
                //fprintf(fp,"%d 1 %lf %lf %lf %lf -1\n", globalIndex, localCurve[0](0) * tmpImg->XResolution(),
                //	(localCurve[0](1)) * tmpImg->YResolution(), localCurve[0](2) * tmpImg->ZResolution(),1.0);
    //            ++globalIndex;
    //            for(size_t k = 1; k < localCurve.size();++k){
                //	fprintf(fp,"%d 3 %lf %lf %lf %lf %d\n", globalIndex, localCurve[k](0) * tmpImg->XResolution(),
                //		(localCurve[k](1)) * tmpImg->YResolution(), localCurve[k](2) * tmpImg->ZResolution(),1.0, globalIndex - 1);
    //                        ++globalIndex;
    //            }
    //        }
    //        fclose(fp);
        //}
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
#ifdef _WIN32
    if(!OrigImage) OrigImage = std::tr1::shared_ptr<SVolume>(new SVolume());
#else
    if(!OrigImage) OrigImage = std::shared_ptr<SVolume>(new SVolume());
#endif

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
#ifdef _WIN32
    std::tr1::shared_ptr<SVolume> tmpImg = std::tr1::dynamic_pointer_cast<SVolume>(OrigImage);
#else
    std::shared_ptr<SVolume> tmpImg = std::dynamic_pointer_cast<SVolume>(OrigImage);
#endif
    if(PARA.swcfile == std::string("NULL")){
#ifdef _WIN32
        soma = std::tr1::shared_ptr<Soma>(new Soma());
#else
        soma = std::shared_ptr<Soma>(new Soma());
#endif
    } else{
#ifdef _WIN32
        soma = std::tr1::shared_ptr<Soma>(new Soma());
#else
        soma = std::shared_ptr<Soma>(new Soma());
#endif
        NeuronTree vswc = readSWC_file(PARA.swcfile.c_str());

#ifdef _WIN32
        soma = std::tr1::shared_ptr<Soma>(new Soma());
        std::tr1::shared_ptr<Soma> tmpSoma = std::tr1::dynamic_pointer_cast<Soma>(soma);
#else
        soma = std::shared_ptr<Soma>(new Soma());
        std::shared_ptr<Soma> tmpSoma = std::dynamic_pointer_cast<Soma>(soma);
#endif

        QList <NeuronSWC> &listNeuron = vswc.listNeuron;
        for(int i = 0; i < listNeuron.size(); ++i){
            NeuronSWC &item = listNeuron[i];
            /*tmpSoma->push_back(Cell(i, item.x / tmpImg->XResolution() - 1, item.y / tmpImg->YResolution() - 1,
                item.z / tmpImg->ZResolution() - 1, 1));*/
            tmpSoma->push_back(Cell(i, item.x / xScale_ - 1, item.y / yScale_ - 1,
                item.z / zScale_ - 1, 1));

            printf("soma:%lf %lf %lf",item.x, (tmpImg->y() - item.y), item.z );
        }
    }
    printf("soma file has been read.\n");
    //binary
    AutoBinaryImage();
    //trace
    AutoTrace();

    //Output

    QString swc_name = PARA.inimg_file.section('.', 0,-2) + "_NeuroGPSTree.swc";
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
    filter->SetThreValue(255 * 4);//2015-8-13
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
    traceFilter->SetThreValue(PARA.traceValue );
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
#ifdef _WIN32
    std::tr1::shared_ptr<INeuronDataObject> breakCon = breaker->ReleaseData();
    std::tr1::shared_ptr<TreeConnect> tmpConnect = std::tr1::dynamic_pointer_cast<TreeConnect>(breakCon);
#else
    std::shared_ptr<INeuronDataObject> breakCon = breaker->ReleaseData();
    std::shared_ptr<TreeConnect> tmpConnect = std::dynamic_pointer_cast<TreeConnect>(breakCon);
#endif

    printf("begin clustre.\n");
    NGNeuronTreeCluster treeCluster = NeuroTreeCluster::New();
    treeCluster->SetInput(breakCon);
    treeCluster->SetInputDeleteID(breaker->getResultIndex());
    treeCluster->SetInputCurve(tree);
    treeCluster->SetInputSoma(soma);
    treeCluster->SetInputOrigImage(OrigImage);
    treeCluster->SetInputBackImage(BackImage);
    treeCluster->SetInputBinImage(BinImage);
    treeCluster->Update();

    seperateTree = treeCluster->ReleaseData();
    printf("Tracing complete.\n");
}

void NeuroGPSTreePlugin::SetRealResolution( double res, int origSz, int& scale, int& sz )
{
    //2015-8-13
    if(res > 0.25 && res <= 0.5 ) {
        scale = 2;
        sz = origSz / scale;
    }else if(res <= 0.25 ) {
        scale = 3;
        sz = origSz / scale;
    } else{
        scale = 1;
        sz = origSz;
    }
}

void NeuroGPSTreePlugin::GetHist(int z)
{
    //initial
#ifdef _WIN32
    std::tr1::shared_ptr<SVolume> image = std::tr1::dynamic_pointer_cast<SVolume>( OrigImage );
#else
    std::shared_ptr<SVolume> image = std::dynamic_pointer_cast<SVolume>( OrigImage );
#endif
    int x = image->x();
    int y = image->y();
    //int z = image->z();
    memset(histgram, 0, sizeof(int) * 1021);
    //make histgram
    for(int i = 0; i < x; ++i){
        for (int j = 0; j < y; ++j){
            //for (int ij = 0; ij < z; ++ij){
            int temp = image->GetPixel(i,j,z);
            ++histgram[image->GetPixel(i,j,z)];
            //}
        }
    }
}

void NeuroGPSTreePlugin::FillBlackArea(int z)
{
    //initial
#ifdef _WIN32
    std::tr1::shared_ptr<SVolume> image = std::tr1::dynamic_pointer_cast<SVolume>( OrigImage );
#else
    std::shared_ptr<SVolume> image = std::dynamic_pointer_cast<SVolume>( OrigImage );
#endif

    int x = image->x();
    int y = image->y();
    //int z = image->z();
    //get fill color
    int sumIndex = 0;
    int maxPixel = 0;
    int maxPixelSum = 0;
    int limitMaxPixel = 255;
    for(int i = 1; i < limitMaxPixel ; ++i){//donot care about 0
        if(sumIndex > 10) break;
        if(histgram[i] > maxPixel){
            ++sumIndex;
            maxPixelSum = histgram[i];
            maxPixel = i;
        }
    }
    //fill black area
    for(int i = 0; i < x; ++i){
        for (int j = 0; j < y; ++j){
            //for (int ij = 0; ij < z; ++ij){
            if(image->GetPixel(i,j,z) == 0)
                image->GetPixel(i,j,z) = maxPixel;
            //}
        }
    }
    //printf("black area of z: %d is painted with color: %d\n", z, maxPixel);
}
