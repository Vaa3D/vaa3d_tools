#include "typelengthfunction.h"
#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"

#include <math.h>
#include <map>

using namespace std;

double dis(const NeuronSWC& p1, const NeuronSWC& p2){
    return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) + (p1.z - p2.z)*(p1.z - p2.z));
}


void typeLength(V3DPluginCallback2& callback){

    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    NeuronTree nt = openDlg->nt;

//    map<int,int> typeMap = map<int,int>();
//    int c = 0;

//    for(int i=0; i<nt.listNeuron.size(); i++){
//        if(typeMap.find(nt.listNeuron[i].type) == typeMap.end()){
//            typeMap[nt.listNeuron[i].type] = c;
//            c++;
//        }
//    }

    int typeC = 9;
    double* lengthType = new double[typeC];
    for(int i=0; i<typeC; i++){
        lengthType[i] = 0;
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        int type = nt.listNeuron[i].type;
//        int typeIndex = typeMap[type];
        if(type >= typeC)
            continue;

        NeuronSWC c = nt.listNeuron[i];
        if(c.parent>0){
            int pIndex = nt.hashNeuron.value(c.parent);
            NeuronSWC p = nt.listNeuron[pIndex];
            lengthType[type] += dis(c,p);
        }
    }

    double* ratio = new double[3];
    double length702 = lengthType[0] + lengthType[2] +lengthType[7];
    ratio[0] = length702 == 0 ? -1 : (lengthType[7]/length702)*100;
    ratio[1] = length702 == 0 ? -1 : (lengthType[0]/length702)*100;
    double length52 = lengthType[2] + lengthType[5];
    ratio[2] = length52 == 0 ? -1 : (lengthType[5]/length52)*100;

    QMessageBox infoBox;
    infoBox.setText("Type length of the neuron:");
    infoBox.setInformativeText(QString("<pre><font size='4'>"
                "Type 0                           : %1<br>"
                "Type 1                           : %2<br>"
                "Type 2                           : %3<br>"
                "Type 3                           : %4<br>"
                "Type 4                           : %5<br>"
                "Type 5                           : %6<br>"
                "Type 6                           : %7<br>"
                "Type 7                           : %8<br>"
                "Type 8                           : %9<br>"
                "Ratio 7/7+0+2                     : %10%<br>"
                "Ratio 0/7+0+2                     : %11%<br>"
                "Ratio 5/5+2                       : %12%</font></pre>")
                .arg(lengthType[0])
                .arg(lengthType[1])
                .arg(lengthType[2])
                .arg(lengthType[3])
                .arg(lengthType[4])
                .arg(lengthType[5])
                .arg(lengthType[6])
                .arg(lengthType[7])
                .arg(lengthType[8])
                .arg(ratio[0])
                .arg(ratio[1])
                .arg(ratio[2]));

    infoBox.exec();

    if(lengthType)
        delete[] lengthType;

    delete openDlg;

}
