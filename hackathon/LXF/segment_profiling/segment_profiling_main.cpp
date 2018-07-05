#include "segment_profiling_main.h"
#include "get_sub_block.h"
#include "segment_mean_shift.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/neuron_format_converter.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.h"
#include "profile_snr.h"

bool segment_profiling_main(V3DPluginCallback2 &callback,NeuronTree &nt,QString &filename)
{
    //vector<MyMarker*> nt_marker;
    //vector<NeuronSegment*> segment;
    QList<NeuronTree> sketchedNTList;
    V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(nt);
    for(int i= 0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        //convert each segment to NeuronTree with one single path
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        //append to editable sketchedNTList
        SS.name = "loaded_" + QString("%1").arg(i);
        if (SS.listNeuron.size()>0)
            sketchedNTList.push_back(SS);
    }

    //nt_marker = nt2mm(nt.listNeuron,filename);
    //swc_to_segments(nt_marker,segment);
    for(V3DLONG i=0;i<sketchedNTList.size();i++)
    {
        PARA PA;
        PA.data1d = 0;

        get_sub_block(callback,1,sketchedNTList[i],PA,i);
        LandmarkList marker_v;
        NeuronTree outtree;
        if(sketchedNTList[i].listNeuron.size()<3)continue;
        for(V3DLONG k=0;k<sketchedNTList[i].listNeuron.size();k++)
        {
            NeuronSWC S;
            S.n = k;
            S.x = sketchedNTList[i].listNeuron[k].x-PA.original_o[0];
            S.y = sketchedNTList[i].listNeuron[k].y-PA.original_o[1];
            S.z = sketchedNTList[i].listNeuron[k].z-PA.original_o[2];
            S.r = sketchedNTList[i].listNeuron[k].r;
            S.parent = k-1;
            S.type = 3;
            outtree.listNeuron.push_back(S);
        }
        writeSWC_file(QString("outtree"+QString::number(i)+".swc"),outtree);
        for(V3DLONG p=0;p<sketchedNTList[i].listNeuron.size();p++)
        {
            LocationSimple mm;
            mm.x = sketchedNTList[i].listNeuron[p].x-PA.original_o[0];
            mm.y = sketchedNTList[i].listNeuron[p].y-PA.original_o[1];
            mm.z = sketchedNTList[i].listNeuron[p].z-PA.original_o[2];
            cout<<"mm = "<<mm.x<<"  "<<mm.y<<"  "<<mm.z<<endl;
            mm.color.a = 0;
            mm.color.b = 0;
            mm.color.g = 0;
            mm.color.r = 0;
            marker_v.push_back(mm);
        }

        //segment_mean_shift(data1d,marker_v,im_cropped_sz,i,segment[i]->markers);
        segment_mean_shift_v2(callback,marker_v,PA,i,sketchedNTList[i].listNeuron);

        profile_swc(callback,PA);
        if(PA.data1d) {delete []PA.data1d; PA.data1d = 0;}
    }
    return true;

}
