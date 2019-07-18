#include "expand_swc.h"
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))

bool expand_swc(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));

    NeuronTree nt,nt_result;
    cout<<fileOpenName.toStdString()<<endl;
    QList<NeuronSWC> neuron,neuron_result;
    nt=readSWC_file(fileOpenName);
    neuron = nt.listNeuron ;
    cout<<"neuron = "<<neuron.size()<<endl;

//    //exapend swc coordinate
//    double n = 1;
//    for(V3DLONG i = 0;i < neuron.size();i++)
//    {
//        NeuronSWC curr;
//        curr.x = n*(neuron[i].x);
//        curr.y = n*(neuron[i].y);
//        curr.z = n*(neuron[i].z);
//        curr.n = neuron[i].n;
//        curr.radius = neuron[i].radius;
//        curr.type = neuron[i].type;
//        curr.parent = neuron[i].parent;

//        neuron_result.push_back(curr);
//    }
//    cout<<"neuron_result = "<<neuron_result.size()<<endl;

//    nt_result.listNeuron = neuron_result;
//    QString filename = fileOpenName + "_result.swc";
//    writeSWC_file(filename,nt_result);

    //create .apo and .ano file
    QList<CellAPO> inmarkers;
    unsigned int Vsize=50;
    for(V3DLONG i = 0; i <neuron.size();i++)
    {
        CellAPO t;
        t.x = neuron.at(i).x+1;
        t.y = neuron.at(i).y+1;
        t.z = neuron.at(i).z+1;
        t.color.r=255;
        t.color.g=0;
        t.color.b=0;
        t.volsize = Vsize;
        inmarkers.push_back(t);
    }

    QString apo_name = fileOpenName + ".apo";
    writeAPO_file(apo_name,inmarkers);


    QString linker_name = fileOpenName + ".ano";
    QFile qf_anofile(linker_name);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {   bool flag = false;
        v3d_msg("Cannot open file for writing!");
        return flag;
    }

    QTextStream out(&qf_anofile);
    out << "SWCFILE=" << QFileInfo(fileOpenName).fileName()<<endl;
    out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
    v3d_msg(QString("Save the linker file to: %1 Complete!").arg(linker_name));

    return true;

}





bool expand_swc(const V3DPluginArgList &input,V3DPluginArgList &output)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;

    QString fileOpenName = QString(inlist->at(0));   

    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        nt = readSWC_file(fileOpenName);

    QList<NeuronSWC> neuron,neuron_result;
    neuron = nt.listNeuron ;
    cout<<"neuron = "<<neuron.size()<<endl;
    for(V3DLONG i = 0;i < neuron.size();i++)
    {
        cout<<neuron[i].n<<endl;
        v3d_msg("stop");

    }

    //exapend swc coordinate
//    double n = 0.5;
//    for(V3DLONG i = 0;i < neuron.size();i++)
//    {
//        NeuronSWC curr;
//        curr.x = n*(neuron[i].x);
//        curr.y = n*(neuron[i].y);
//        curr.z = n*(neuron[i].z);
//        curr.n = neuron[i].n;
//        curr.radius = neuron[i].radius;
//        curr.type = neuron[i].type;
//        curr.parent = neuron[i].parent;

//        neuron_result.push_back(curr);
//    }
//    cout<<"neuron_result = "<<neuron_result.size()<<endl;

//    NeuronTree nt_result;
//    nt.listNeuron = neuron_result;
//    QString filename = fileOpenName + "_result.swc";
//    writeSWC_file(filename,nt_result);

    return true;
}


