#include "Preprocess.h"

//QString Preprocess::transformproot(QString root)
//{
//    int n=root.length();
//    for(int i=0;i<n;i++){
//        if(root[i]=='/')
//            root[i]='\\';
//    }
//    return root;
//}

void Preprocess::gaussfilter(DataFlow *Predatamem,vector<char *> inputarg,int i,char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int j=0;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Predatamem->outputimg[i]);

    vector<Image4DSimple *> outputimg;              //output imgs
    Image4DSimple *otimg=new Image4DSimple();
    outputimg.push_back(otimg);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(&outputimg);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"gf start";
    qDebug()<<mcallback->callPluginFunc("gaussianfilter1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"gf end";

    Predatamem->outputimg[i]=otimg;
//    for(int k=0;k<data_paths.size();k++){
//        char *inputimg;
//        QByteArray ba1=(Proot+'\\'+data_paths[k]).toLatin1();
//        inputimg=ba1.data();
//        //v3d_msg(ch);
//        char inimage[200];
//        for(int i=0;i<200;i++){
//            inimage[i]=inputimg[i];
//            if(inputimg[i]=='\0')
//                break;
//        }

//        char *out0;
//        QByteArray ba2=("D:\\A_result\\"+data_paths[k]+"_gf.tiff").toLatin1();
//        out0=ba2.data();
//        char out[200];
//        for(int i=0;i<200;i++){
//            out[i]=out0[i];
//            if(out0[i]=='\0')
//                break;
//        }

//        //char out[200]="D:\\A_result\\1.tiff";
//        //char out1[200]="D:\\A_result\\2.tiff";
//        int isdatafile=0;
//        int isout=0;
//        int Wx = 7;
//        int Wy = 7;
//        int Wz = 3;
//        int c = 1;
//        double sigma = 1.0;
//        char cisdatafile[2],cisout[2],cWx[3],cWy[3],cWz[3],cc[2],csigma[5];
//        itoa(isdatafile,cisdatafile,10);
//        itoa(isout,cisout,10);
//        itoa(Wx,cWx,10);
//        itoa(Wy,cWy,10);
//        itoa(Wz,cWz,10);
//        itoa(c,cc,10);
//        sprintf(csigma,"%f",sigma);

//        V3DPluginArgList pluginInputList, pluginOutputList;
//        V3DPluginArgItem FileInput, inputParam, Output,Outputimg;
//        vector<char*> pluginInputFile;
//        vector<char*> pluginInputArgList;
//        vector<char*> pluginOutputArgList;

//        vector<Image4DSimple *> pluginInputimg;
//        vector<Image4DSimple *> pluginOutputimg;

//        Image4DSimple *input=new Image4DSimple();
//        input=mcallback->loadImage(inimage);

//        Image4DSimple *result=new Image4DSimple();
//        //pluginInputFile.push_back(inimage);         //inputimg path
//        pluginInputimg.push_back(input);

//        pluginInputArgList.push_back(cisdatafile);
//        pluginInputArgList.push_back(cisout);
//        pluginInputArgList.push_back(cWx);          //input para
//        pluginInputArgList.push_back(cWy);
//        pluginInputArgList.push_back(cWz);
//        pluginInputArgList.push_back(cc);
//        pluginInputArgList.push_back(csigma);

//        pluginOutputArgList.push_back(out);         //outputimg path

//        pluginOutputimg.push_back(result);          //outputimg mem

//        //qDebug()<<"result: "<<
//        FileInput.type = "";
//        FileInput.p = (void*)(&pluginInputimg);
//        inputParam.type = "";
//        inputParam.p = (void*)(&pluginInputArgList);

//        pluginInputList.push_back(FileInput);       //input arglist
//        pluginInputList.push_back(inputParam);


////        Output.type = "";
////        Output.p = (void*)(&pluginOutputArgList);
//        Outputimg.type="";
//        Outputimg.p = (void*)(&pluginOutputimg);

////        pluginOutputList.push_back(Output);         //output arglist
//        pluginOutputList.push_back(Outputimg);

//        qDebug()<<mcallback->callPluginFunc("gaussianfilter1.dll","gf",pluginInputList, pluginOutputList);
//        Predatamem->push_img(pluginOutputimg.at(0));
//    //qDebug()<<pluginOutputimg.at(0)->getXDim();
//    //mcallback->saveImage(pluginOutputimg.at(0),out1);
//    }
    //    qDebug()<<Predatamem->getimg_cnt();
}

void Preprocess::imPreprocess(DataFlow *Predatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int j=0;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Predatamem->outputimg[i]);

    vector<Image4DSimple *> outputimg;              //output imgs
    Image4DSimple *otimg=new Image4DSimple();
    outputimg.push_back(otimg);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(&outputimg);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"imPreprocess start";
    qDebug()<<mcallback->callPluginFunc("imPreProcess1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"imPreprocess end";

    Predatamem->outputimg[i]=otimg;
}

void Preprocess::gsdt(DataFlow *Predatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int j=0;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Predatamem->outputimg[i]);

    vector<Image4DSimple *> outputimg;              //output imgs
    Image4DSimple *otimg=new Image4DSimple;
    qDebug()<<otimg;
    outputimg.push_back(otimg);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(&outputimg);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"gsdt start";
    qDebug()<<mcallback->callPluginFunc("gsdt1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"gsdt end";
    //mcallback->saveImage(otimg,"D:\\A_DLcsz\\DLtrain\\fixed_data\\result\\1.tiff");
    //qDebug()<<otimg->getTotalBytes();
    //Predatamem->outputimg[i]->cleanExistData();
    Predatamem->outputimg[i]=otimg;
}

void Preprocess::cropTerafly(DataFlow *Predatamem, char * inputfile,std::vector<char *> inputarg)
{
    qDebug()<<inputarg.size();
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    //pluginInputArgList.push_back("0");
    for(int j=1;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<char *> Inputfile;               //input imgs
    Inputfile.push_back(inputfile);
    Inputfile.push_back(inputarg[0]);

//    vector<Image4DSimple *> outputimg;              //output imgs
//    Image4DSimple *otimg=new Image4DSimple();
//    outputimg.push_back(otimg);

    Inputimg.type="Inputfile";
    Inputimg.p = (void*)(&Inputfile);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(Predatamem);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"cropTerafly start";
    qDebug()<<mcallback->callPluginFunc("cropped3DImageSeries1.dll","cropTerafly",pluginInputList, pluginOutputList);
    qDebug()<<"cropTerafly end";

    //Predatamem->outputimg[i]=otimg;
}

void Preprocess::histogramEqualization(DataFlow *Predatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int j=0;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Predatamem->outputimg[i]);

    vector<Image4DSimple *> outputimg;              //output imgs
    Image4DSimple *otimg=new Image4DSimple();
    outputimg.push_back(otimg);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(&outputimg);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"histogramEqualization start";
    qDebug()<<mcallback->callPluginFunc("HistogramEqualization1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"histogramEqualization end";

    Predatamem->outputimg[i]=otimg;
}

void Preprocess::standardize(DataFlow *Predatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int j=0;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Predatamem->outputimg[i]);

    vector<Image4DSimple *> outputimg;              //output imgs
    Image4DSimple *otimg=new Image4DSimple();
    outputimg.push_back(otimg);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(&outputimg);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"standardize start";
    qDebug()<<mcallback->callPluginFunc("standardize_image1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"standardize end";

    Predatamem->outputimg[i]=otimg;
}

void Preprocess::datatypeconvert(DataFlow *Predatamem, std::vector<char *> inputarg, int i, char *funcname)
{
    V3DPluginArgList pluginInputList, pluginOutputList;
    V3DPluginArgItem Inputimg, InputParam, Outputimg;

    vector<char*> pluginInputArgList;               //input args
    pluginInputArgList.push_back("0");
    pluginInputArgList.push_back("0");
    for(int j=0;j<inputarg.size();j++){
        pluginInputArgList.push_back(inputarg[j]);
    }

    vector<Image4DSimple *> inputimg;               //input imgs
    inputimg.push_back(Predatamem->outputimg[i]);
    qDebug()<<Predatamem->outputimg[i]->getDatatype();
    vector<Image4DSimple *> outputimg;              //output imgs
    Image4DSimple *otimg=new Image4DSimple();
    outputimg.push_back(otimg);

    Inputimg.type="Inputimg";
    Inputimg.p = (void*)(&inputimg);

    InputParam.type="InputParam";
    InputParam.p = (void*)(&pluginInputArgList);

    Outputimg.type="Outputimg";
    Outputimg.p = (void*)(&outputimg);

    pluginInputList.push_back(Inputimg);
    pluginInputList.push_back(InputParam);

    pluginOutputList.push_back(Outputimg);

    qDebug()<<"datatypeconvert start";
    qDebug()<<mcallback->callPluginFunc("datatypeconvert1.dll",funcname,pluginInputList, pluginOutputList);
    qDebug()<<"datatypeconvert end";

    Predatamem->outputimg[i]=otimg;
}


