#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
using namespace std;

#include "pre_processing_main.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neuron_connector_func.h"
#if !defined(Q_OS_WIN32)
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include <io.h>
#endif

CellAPO get_marker(NeuronSWC node, double vol, double color_r, double color_g, double color_b){
    CellAPO marker;
    marker.x = node.x;
    marker.y = node.y;
    marker.z = node.z;
    marker.volsize = vol;
    marker.color.r = color_r;
    marker.color.g = color_g;
    marker.color.b = color_b;
    return marker;
}

bool my_saveANO(QString fileNameHeader, bool swc=true, bool apo=true){
    FILE * fp=0;
    fp = fopen((char *)qPrintable(fileNameHeader+QString(".ano")), "wt");
    if (!fp)
    {
        v3d_msg("Fail to open file to write.");
        return false;
    }
    if(fileNameHeader.count("/")>0){
        fileNameHeader = fileNameHeader.right(fileNameHeader.size()-fileNameHeader.lastIndexOf("/")-1);
    }

    if(swc){fprintf(fp, "SWCFILE=%s\n", qPrintable(fileNameHeader+QString(".swc")));}
    if(apo){fprintf(fp, "APOFILE=%s\n", qPrintable(fileNameHeader+QString(".apo")));}
    if(fp){fclose(fp);}
    return true;
}

NeuronTree rm_nodes(NeuronTree nt, QList<int> list){
    qSort(list);
    cout << "rm_nodes started" <<endl;
    cout << list.last() << "\t" << list.size() << "\t" << nt.listNeuron.size()<<endl;
    if(list.last()>=nt.listNeuron.size()){  // Index out of range
        cout << list.at(list.last())<<endl<<"rm_nodes Error: index out of range\n";
        return nt;
    }
    for(int i=(list.size()-1); i>=0; i--){
        cout << list.at(i) << "\t" << nt.listNeuron.size()<<endl;
        nt.listNeuron.removeAt(list.at(i));
    }
    cout << "rm_nodes finished" <<endl;

    return nt;
}
