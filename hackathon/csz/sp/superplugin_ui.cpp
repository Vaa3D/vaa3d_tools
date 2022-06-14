#include "superplugin_ui.h"


void SuperUI::processcmd(const V3DPluginArgList &input, V3DPluginArgList &output,QString funcname)
{
    initmap();
    if(funcname=="autoproduce")
        initautoproduce(input,output);
    else if(funcname=="batchrun")
        initbatchrun(input,output);
    else if(funcname=="usrdesigned"){
        QString otpath=QString((*(vector<char*> *)(output.at(0).p)).at(0));
        this->cmd->acceptcmd(input,output);
        char *dataline=(*(vector<char*> *)(input.at(1).p)).at(0);
        this->cmd->readCmdTxt(QString(dataline));
        //this->cmd->readCmdTxt("D:\\A_DLcsz\\DLtrain\\fixed_data\\dataline.txt");
        if(this->cmd->dtf.size()!=this->cmd->otresult.size()||this->cmd->dtf.size()!=this->cmd->DataFlowCmd.size()||this->cmd->otresult.size()!=this->cmd->DataFlowCmd.size()){
            qDebug()<<"something wrong in init cmd!";
            return;
        }
        for(int i=0;i<this->cmd->dtf.size();i++){
            this->assemblyline(this->cmd->dtf[i],this->cmd->DataFlowCmd[i],this->cmd->otresult[i],otpath);
        }

    }
}

void SuperUI::initautoproduce(const V3DPluginArgList &input, V3DPluginArgList &output)
{
    datamem=new DataFlow();

    paras=(*(vector<char*> *)(input.at(1).p));               //input.at(1)
    inputway=QString(paras[0]);
    if(inputway!="R"&&inputway!="T"){
        qDebug()<<"Wrong output parameter! Please input 'R' or 'T'.";
        return;
    }
    if(inputway=="R"){
        qDebug()<<"The dataflow will be loaded from your Raw local folder.";
    }else if(inputway=="T"){
        qDebug()<<"The dataflow will be loaded from your Terafly folder.";
    }

    outresult=QString(paras[1]);
    if(outresult!="img"&&outresult!="swc"&&outresult!="all"){
        qDebug()<<"Wrong output format! Please input 'img' or 'swc' or 'all'.";
        return;
    }
    qDebug()<<"Your output will be "+outresult+" format.";

    if(inputway=="R"){
        inputfile=((vector<char*> *)(input.at(0).p))->at(0);                    //input.at(0)
        qinputfile=QString(inputfile);
        inputimglist=getImgNames(qinputfile);
        inputswclist=getSwcNames(qinputfile);
        int count=inputimglist.size()>inputswclist.size()?inputimglist.size():inputswclist.size();
        //qDebug()<<"count";
        datamem->dataname=inputimglist.size()>inputswclist.size()?inputimglist:inputswclist;
        for(int i=0;i<count;i++){
            Image4DSimple *nimg=new Image4DSimple();
            NeuronTree *nswc=new NeuronTree();
            datamem->push_img(nimg);
            datamem->push_swc(nswc);
        }
        //qDebug()<<"count";
        for(int i=0;i<inputimglist.size();i++){
            Image4DSimple *nimg;

            QByteArray ba1=(qinputfile+"\\"+inputimglist[i]).toLatin1(); //+"\\"+inputimglist[i]
            char *inputimg=ba1.data();

            char inimage[200];
            for(int i=0;i<200;i++){
                inimage[i]=inputimg[i];
                if(inputimg[i]=='\0')
                    break;
            }

            nimg=mcallback->loadImage(inimage);

            datamem->outputimg[i]=nimg;

        }
        //qDebug()<<"count";
        for(int i=0;i<inputswclist.size();i++){
            NeuronTree *nswc=new NeuronTree();
            QString swcfile=qinputfile+"\\"+inputswclist[i];
            //qDebug()<<swcfile;
            *nswc=readSWC_file(swcfile);

            datamem->outputswc[i]=nswc;

        }
        qDebug()<<"count";
        outputfile=((vector<char*> *)(output.at(0).p))->at(0);                  //output.at(0)

        vector<char *> funcparas;

        for(int i=2;i<paras.size();i++){
            if(paras[i][0]>='a'&&paras[i][0]<='z'){         //DataFlowArg[0] is empty.
                DataFlowArg.push_back(funcparas);
                funcparas.clear();
                funcparas.push_back(paras[i]);
                if(i==paras.size()-1)
                    DataFlowArg.push_back(funcparas);
                continue;
            }
            funcparas.push_back(paras[i]);
            if(i==paras.size()-1)
                DataFlowArg.push_back(funcparas);
        }
        qDebug()<<DataFlowArg.size()-1;
    }else{
        qDebug()<<"Your first func must be 'cropTerafly'!";
        inputfile=((vector<char*> *)(input.at(0).p))->at(0);                    //input.at(0)
        //qinputfile=QString(inputfile);
        outputfile=((vector<char*> *)(output.at(0).p))->at(0);
        vector<char *> funcparas;
        for(int i=2;i<paras.size();i++){
            if(paras[i][0]>='a'&&paras[i][0]<='z'){         //DataFlowArg[0] is empty.
                DataFlowArg.push_back(funcparas);
                funcparas.clear();
                funcparas.push_back(paras[i]);
                if(i==paras.size()-1)
                    DataFlowArg.push_back(funcparas);
                continue;
            }
            funcparas.push_back(paras[i]);
            if(i==paras.size()-1)
                DataFlowArg.push_back(funcparas);
        }
        qDebug()<<DataFlowArg.size()-1;
    }
}

void SuperUI::initmap()
{
    fnametodll["gf"]="gspaussianfilter.dll";
    fnametodll["app2"]="vspn2.dll";
    fnametodll["im_sigma_correction"]="ispmPreProcess.dll";
    fnametodll["im_subtract_minimum"]="ispmPreProcess.dll";
    fnametodll["im_bilateral_filter"]="ispmPreProcess.dll";
    fnametodll["im_fft_filter"]="ispmPreProcess.dll";
    fnametodll["im_grey_morph"]="ispmPreProcess.dll";
    fnametodll["im_enhancement"]="ispmPreProcess.dll";
    fnametodll["gsdt"]="gspsdt.dll";
    fnametodll["cropTerafly"]="cspropped3DImageSeries.dll";
    fnametodll["he"]="HspistogramEqualization.dll";
    fnametodll["standardize"]="ssptandardize_image.dll";
    fnametodll["dtc"]="dspatatypeconvert.dll";
    fnametodll["resample_swc"]="rspesample_swc.dll";
    fnametodll["sort_swc"]="ssport_neuron_swc.dll";

    dlltomode["gspaussianfilter.dll"]="Preprocess";
    dlltomode["ispmPreProcess.dll"]="Preprocess";
    dlltomode["cspropped3DImageSeries.dll"]="Preprocess";
    dlltomode["gspsdt.dll"]="Preprocess";
    dlltomode["HspistogramEqualization.dll"]="Preprocess";
    dlltomode["ssptandardize_image.dll"]="Preprocess";
    dlltomode["dspatatypeconvert.dll"]="Preprocess";
    dlltomode["vspn2.dll"]="Computation";
    dlltomode["rspesample_swc.dll"]="Postprocess";
    dlltomode["ssport_neuron_swc.dll"]="Postprocess";

}

void SuperUI::assemblyline()
{
//    for(int i=1;i<DataFlowArg.size();i++){
////        V3DPluginArgList pluginInputList, pluginOutputList;
////        V3DPluginArgItem FileInput, inputParam, Output,Outputimg;
////        vector<char*> pluginInputFile;
////        vector<char*> pluginInputArgList;
////        vector<char*> pluginOutputArgList;

//        vector<char*> pluginInputArgList;

//        QString funcdll=fnametodll[QString(DataFlowArg[i][0])];
//        QString process=dlltomode[funcdll];
//        qDebug()<<funcdll;
//        qDebug()<<process;
//        if(process=="Preprocess"){
//            Preprocess * Preproc= new Preprocess(this->mcallback);
//            if(funcdll=="gspaussianfilter.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Preproc->gaussfilter(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }else if(funcdll=="ispmPreProcess.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Preproc->imPreprocess(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }else if(funcdll=="gspsdt.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Preproc->gsdt(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }else if(funcdll=="cspropped3DImageSeries.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                qDebug()<<"Executing NO. "<<i<<" func.";
//                Preproc->cropTerafly(datamem,inputfile,pluginInputArgList);
//            }else if(funcdll=="HspistogramEqualization.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Preproc->histogramEqualization(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }else if(funcdll=="ssptandardize_image.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Preproc->standardize(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }else if(funcdll=="dspatatypeconvert.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Preproc->datatypeconvert(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }
//            delete Preproc;
//            Preproc=nullptr;
//        }else if(process=="Computation"){
//            Computation *Comproc=new Computation(this->mcallback);
//            if(funcdll=="vspn2.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getimg_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Comproc->vn2(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }
//            delete Comproc;
//            Comproc=nullptr;
//        }else if(process=="Postprocess"){
//            Postprocess *Postproc=new Postprocess(this->mcallback);
//            if(funcdll=="rspesample_swc.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getswc_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Postproc->resample_swc(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }else if(funcdll=="ssport_neuron_swc.dll"){
//                pluginInputArgList.clear();
//                for(int j=1;j<DataFlowArg[i].size();j++){
//                    pluginInputArgList.push_back(DataFlowArg[i][j]);
//                }
//                for(int j=0;j<datamem->getswc_cnt();j++){
//                    qDebug()<<"Executing NO. "<<i<<" func.";
//                    Postproc->sort_neuron_swc(datamem,pluginInputArgList,j,DataFlowArg[i][0]);
//                }
//            }
//            delete Postproc;
//            Postproc=nullptr;
//        }

//    }
//    qDebug()<<"img count: "<<datamem->getimg_cnt();
//    qDebug()<<"swc count: "<<datamem->getswc_cnt();
//    for(int j=0;j<datamem->getimg_cnt();j++){
//        qDebug()<<"outresult is "<<outresult;
//        if(outresult=="img"){
//            qDebug()<<"saving img!";
//            saveimgresult(datamem,j,QString(outputfile));
//        }
//        else if(outresult=="swc"){
//            qDebug()<<"saving swc!";
//            saveswcresult(datamem,j,QString(outputfile));
//        }else if(outresult=="all"){
//            qDebug()<<"saving all!";
//            qDebug()<<"Please make sure you had used both img and swc!";
//            saveimgresult(datamem,j,QString(outputfile));
//            saveswcresult(datamem,j,QString(outputfile));
//        }
//    }
    this->assemblyline(datamem,DataFlowArg,outresult,QString(outputfile));
}

void SuperUI::assemblyline(DataFlow *databatch, std::vector<std::vector<char *> > parabatch, QString otresult,QString otpath)
{
    //qDebug()<<"parabatch size is "<<parabatch.size();
    for(int i=1;i<parabatch.size();i++){
        vector<char*> pluginInputArgList;
        //qDebug()<<QString(parabatch[i][0]).size();
        QString funcdll=fnametodll[QString(parabatch[i][0])];
        QString process=dlltomode[funcdll];
        qDebug()<<funcdll;
        qDebug()<<process;
        if(process=="Preprocess"){
            Preprocess * Preproc= new Preprocess(this->mcallback);
            if(funcdll=="gspaussianfilter.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Preproc->gaussfilter(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }else if(funcdll=="ispmPreProcess.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Preproc->imPreprocess(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }else if(funcdll=="gspsdt.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Preproc->gsdt(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }else if(funcdll=="cspropped3DImageSeries.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<DataFlowArg[i].size();j++){
                    pluginInputArgList.push_back(DataFlowArg[i][j]);
                }
                qDebug()<<"Executing NO. "<<i<<" func.";
                Preproc->cropTerafly(datamem,inputfile,pluginInputArgList);
            }else if(funcdll=="HspistogramEqualization.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Preproc->histogramEqualization(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }else if(funcdll=="ssptandardize_image.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Preproc->standardize(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }else if(funcdll=="dspatatypeconvert.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Preproc->datatypeconvert(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }
            delete Preproc;
            Preproc=nullptr;
        }else if(process=="Computation"){
            Computation *Comproc=new Computation(this->mcallback);
            if(funcdll=="vspn2.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getimg_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Comproc->vn2(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }
            delete Comproc;
            Comproc=nullptr;
        }else if(process=="Postprocess"){
            Postprocess *Postproc=new Postprocess(this->mcallback);
            if(funcdll=="rspesample_swc.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getswc_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Postproc->resample_swc(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }else if(funcdll=="ssport_neuron_swc.dll"){
                pluginInputArgList.clear();
                for(int j=1;j<parabatch[i].size();j++){
                    pluginInputArgList.push_back(parabatch[i][j]);
                }
                for(int j=0;j<databatch->getswc_cnt();j++){
                    qDebug()<<"Executing NO. "<<i<<" func.";
                    Postproc->sort_neuron_swc(databatch,pluginInputArgList,j,parabatch[i][0]);
                }
            }
            delete Postproc;
            Postproc=nullptr;
        }

    }
    qDebug()<<"img count: "<<databatch->getimg_cnt();
    qDebug()<<"swc count: "<<databatch->getswc_cnt();
    for(int j=0;j<databatch->getimg_cnt();j++){
        if(otresult=="img"){
            qDebug()<<"saving img!";
            saveimgresult(databatch,j,otpath);
        }
        else if(otresult=="swc"){
            qDebug()<<"saving swc!";
            saveswcresult(databatch,j,otpath);
        }else if(otresult=="all"){
            qDebug()<<"saving all!";
            qDebug()<<"Please make sure you had used both img and swc!";
            saveimgresult(databatch,j,otpath);
            saveswcresult(databatch,j,otpath);
        }
    }
}

void SuperUI::initbatchrun(const V3DPluginArgList &input, V3DPluginArgList &output)
{
    inputfile=((vector<char*> *)(input.at(0).p))->at(0);                    //input.at(0)
    qinputfile=QString(inputfile);
    inputlist=getNames(qinputfile);

    outputfile=((vector<char*> *)(output.at(0).p))->at(0);
    paras=(*(vector<char*> *)(input.at(1).p));
//    for(auto &name :inputlist)
//        qDebug()<<name;
    //    qDebug()<<outputfile;
}

void SuperUI::batchrun()
{
    char* plugin=paras[0];
    char* funcname=paras[1];
    char* suffix=paras[2];
    V3DPluginArgItem Input, InputParam, Output;
    V3DPluginArgList pluginInputList, pluginOutputList;
    vector<char *> inputparam;
    vector<char *> INput;
    vector<char *> OUTput;
    for(int i=3;i<paras.size();i++){
        inputparam.emplace_back(paras[i]);
    }

    for(int i=0;i<inputlist.size();i++){
        QString inputname,outputname;
        inputname=QString(inputfile)+"\\"+inputlist[i];
        outputname=QString(outputfile)+"\\"+inputlist[i]+"."+suffix;
        char *input=inputname.toLatin1().data();
        char *output=outputname.toLatin1().data();
        INput.clear();
        OUTput.clear();

        INput.push_back(input);
        OUTput.push_back(output);

        Input.type="input";
        Input.p= (void*)(&INput);

        Output.type="output";
        Output.p= (void*)(&OUTput);

        InputParam.type="InputParam";
        InputParam.p= (void*)(&inputparam);

        pluginInputList.push_back(Input);
        pluginInputList.push_back(InputParam);

        pluginOutputList.push_back(Output);

        this->mcallback->callPluginFunc(plugin,funcname,pluginInputList,pluginOutputList);
    }


}

QString SuperUI::finddll(char *funcname)
{
    return fnametodll[funcname];
}

void SuperUI::saveimgresult(DataFlow *data, int i,QString otpath)
{
//    QStringList inputlists;
//    inputlists=inputimglist.size()>inputswclist.size()?inputimglist:inputswclist;
    QString path;
    if(data->otdataname.size()==0)
        path=otpath+"\\"+data->dataname[i]+"_result.tiff";
    else
        path=otpath+"\\"+data->otdataname[i];
    QByteArray ba1=(path).toLatin1();
    char *otimg=ba1.data();
    //qDebug()<<inputimg;
    char outpath[200];
    for(int j=0;j<200;j++){
        outpath[j]=otimg[j];
        if(otimg[j]=='\0')
            break;
    }
    qDebug()<<"outpath is "<<outpath;
    V3DLONG  in_sz[4];
    in_sz[0] = data->outputimg[i]->getXDim();
    in_sz[1] = data->outputimg[i]->getYDim();
    in_sz[2] = data->outputimg[i]->getZDim();
    in_sz[3] = data->outputimg[i]->getCDim();
    simple_saveimage_wrapper(*mcallback,outpath,data->outputimg[i]->getRawData(),in_sz,1);
//    qDebug()<<data->outputimg[i];
//    mcallback->saveImage(data->outputimg[i],(char*)outpath);
    qDebug()<<"the "<<i<<" img saved.";

}

void SuperUI::saveswcresult(DataFlow *data, int i,QString otpath)
{
//    QStringList inputlists;
//    inputlists=inputimglist.size()>inputswclist.size()?inputimglist:inputswclist;
    QString path;
    if(data->otdataname.size()==0)
        path=otpath+"\\"+data->dataname[i]+"_result.swc";
    else
        path=otpath+"\\"+data->otdataname[i];
    QStringList infostring;
    infostring.clear();
    infostring.push_back("##Assembly line");
    infostring.push_back("##Output by superplugin");
    qDebug()<<writeSWC_file(path,*data->outputswc[i]);

}

void SuperUI::click_yes()
{
    QString str;
    str=preprocess->currentText();
    preproc=new Preprocess(this->mcallback);
    if(str=="gaussfilter"){
        //preproc->gaussfilter(datamem);
    }
    QString count;
    count=QString::fromStdString(std::to_string(datamem->getimg_cnt()));
    v3d_msg(count);
}
