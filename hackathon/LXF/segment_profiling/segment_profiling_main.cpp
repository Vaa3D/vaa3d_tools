#include "segment_profiling_main.h"
#include "get_sub_block.h"
#include "segment_mean_shift.h"


bool segment_profiling_main(V3DPluginCallback2 &callback,NeuronTree &nt,QString &filename)
{
    vector<MyMarker*> nt_marker;
    vector<NeuronSegment*> segment;
    nt_marker = nt2mm(nt.listNeuron,filename);
    swc_to_segments(nt_marker,segment);
    for(V3DLONG i=0;i<segment.size();i++)
    {
        PARA PA;
        PA.data1d = 0;

        get_sub_block(callback,1,segment[i]->markers,PA,i);

        //  cout<<"original_o = "<<original_o[0]<<endl;
        LandmarkList marker_v;
        cout<<"segment[i]->markers.size() = "<<segment[i]->markers.size()<<endl;
        NeuronTree outtree;
        if(segment[i]->markers.size()<3)continue;
        for(V3DLONG k=0;k<segment[i]->markers.size();k++)
        {
            NeuronSWC S;
            S.n = k;
            S.x = segment[i]->markers[k]->x-PA.original_o[0];
            S.y = segment[i]->markers[k]->y-PA.original_o[1];
            S.z = segment[i]->markers[k]->z-PA.original_o[2];
            S.r = 2;
            S.parent = k-1;
            S.type = 3;
            outtree.listNeuron.push_back(S);
        }
        writeSWC_file(QString("outtree"+QString::number(i)+".swc"),outtree);
        for(V3DLONG p=0;p<segment[i]->markers.size();p++)
        {
            LocationSimple mm;
            mm.x = segment[i]->markers[p]->x-PA.original_o[0];
            mm.y = segment[i]->markers[p]->y-PA.original_o[1];
            mm.z = segment[i]->markers[p]->z-PA.original_o[2];
            cout<<"mm = "<<mm.x<<"  "<<mm.y<<"  "<<mm.z<<endl;
            mm.color.a = 0;
            mm.color.b = 0;
            mm.color.g = 0;
            mm.color.r = 0;
            marker_v.push_back(mm);
        }

        //segment_mean_shift(data1d,marker_v,im_cropped_sz,i,segment[i]->markers);
        segment_mean_shift_v2(callback,marker_v,PA,i,segment[i]->markers);
        if(PA.data1d) {delete []PA.data1d; PA.data1d = 0;}
    }

}
