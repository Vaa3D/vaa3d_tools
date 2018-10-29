#include "v3d_interface.h"
#include "reconstruction_IO_main.h"

void reconstruction_IO_main(QString input, QString output,
                            V3DLONG tx, V3DLONG ty, V3DLONG tz,
                            V3DLONG rx, V3DLONG ry, V3DLONG rz,
                            V3DLONG sx, V3DLONG sy, V3DLONG sz){

}

void convert_format(QString inputfile, QString outputlabel, QString informat, QString outformat){
    QStringList allowed_formats = (QStringList() << "swc" << "apo" << "markers");
    if(not(allowed_formats.contains(informat) && allowed_formats.contains(outformat))){
        printf("Do nothing. Please specify input/output format as one of the following: swc, apo, markers.");
        return;
    }
    if (informat == outformat){
        printf("Input and output formats are the same, do nothing.");
        return;
    }
    if (outputlabel.size()==0){
        outputlabel = inputfile;
    }
    QString outputfile = outputlabel + QString(".") + outformat;

    // Case 1: apo to other formats
    if (informat == QString("apo")){
        QList<CellAPO> apolist = readAPO_file(inputfile);
        if (outformat == "swc"){
            QList<NeuronSWC> swclist;
            for(int i=0; i<apolist.size(); i++){
                NeuronSWC node;
                node.n = i;
                node.x = apolist.at(i).x;
                node.y = apolist.at(i).y;
                node.z = apolist.at(i).z;
                node.pn = -1;
                node.r = 1;
                swclist.append(node);
            }
            NeuronTree nt;
            nt.listNeuron = swclist;
            writeSWC_file(outputfile, nt);
            return;
        }
    }

    // Case 2: swc to other formats
    // Case 3: markers to other formatss
    return;
}

void scale(QString inputfile, QString outputlabel, double xscale, double yscale, double zscale){
    QString outformat;
    if(outputlabel.size()==0){
        outputlabel = inputfile;
    }
    if(inputfile.endsWith("swc") || inputfile.endsWith("SWC")){
        outformat = QString("swc");
        QString outputfile = outputlabel + QString(".")+ outformat;
        NeuronTree nt = readSWC_file(inputfile);
        for(int i=0;i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].x *= xscale;
            nt.listNeuron[i].y *= yscale;
            nt.listNeuron[i].z *= zscale;
        }
        writeSWC_file(outputfile, nt);
    }
    else{
        fprintf(stderr, "input file format is wrong");
    }
    return;
}
