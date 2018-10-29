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
        QList<double> min_XYZ;
        QList<double> max_XYZ;
        for(int i=0;i<nt.listNeuron.size(); i++){
            NeuronSWC node = nt.listNeuron.at(i);
            if(i==0){
                min_XYZ.append(node.x);
                min_XYZ.append(node.y);
                min_XYZ.append(node.z);
                max_XYZ.append(node.x);
                max_XYZ.append(node.y);
                max_XYZ.append(node.z);
            }
            else{
                min_XYZ[0] = (min_XYZ[0]<node.x) ? min_XYZ[0] : node.x;
                min_XYZ[1] = (min_XYZ[1]<node.y) ? min_XYZ[1] : node.y;
                min_XYZ[2] = (min_XYZ[2]<node.z) ? min_XYZ[2] : node.z;
                max_XYZ[0] = (max_XYZ[0]>node.x) ? max_XYZ[0] : node.x;
                max_XYZ[1] = (max_XYZ[1]>node.y) ? max_XYZ[1] : node.y;
                max_XYZ[2] = (max_XYZ[2]>node.z) ? max_XYZ[2] : node.z;
            }
            nt.listNeuron[i].x *= xscale;
            nt.listNeuron[i].y *= yscale;
            nt.listNeuron[i].z *= zscale;
        }
        writeSWC_file(outputfile, nt);
        printf("Raw\tMin\tMax\n");
        printf("X:\t%f\t%f\n", min_XYZ[0], max_XYZ[0]);
        printf("Y:\t%f\t%f\n", min_XYZ[1], max_XYZ[1]);
        printf("Z:\t%f\t%f\n", min_XYZ[2], max_XYZ[2]);
        printf("Scaled\tMin\tMax\n");
        printf("X:\t%f\t%f\n", min_XYZ[0]*xscale, max_XYZ[0]*xscale);
        printf("Y:\t%f\t%f\n", min_XYZ[1]*yscale, max_XYZ[1]*yscale);
        printf("Z:\t%f\t%f\n", min_XYZ[2]*zscale, max_XYZ[2]*zscale);
    }
    else{
        fprintf(stderr, "input file format is wrong");
    }
    return;
}
