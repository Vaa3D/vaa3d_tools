#include "decompose_swc_main.h"
using namespace std;

void decompose_to_multiple_swc(NeuronTree nt, QString output_folder)
{
    if(!output_folder.endsWith("/")){
        output_folder.append("/");
    }

    V_NeuronSWC_list nt_decomposed = NeuronTree__2__V_NeuronSWC_list(nt);
    vector <V_NeuronSWC> segment_list = nt_decomposed.seg;
    for(int i=0; i<segment_list.size(); i++){
        NeuronTree nt_segment = V_NeuronSWC__2__NeuronTree(segment_list.at(i));
        QString output_swc = output_folder+QString::number(i)+QString(".swc");
        writeSWC_file(output_swc, nt_segment);
    }
    return;
}
