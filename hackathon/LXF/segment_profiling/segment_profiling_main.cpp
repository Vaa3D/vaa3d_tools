#include "segment_profiling_main.h"
#include "get_sub_block.h"
#include "segment_mean_shift.h"


bool segment_profiling_main(V3DPluginCallback2 &callback,NeuronTree &nt,QString &filename)
{
    v3d_msg("check!");
    vector<MyMarker*> nt_marker;
    vector<NeuronSegment*> segment;
    nt_marker = nt2mm(nt.listNeuron,filename);
    swc_to_segments(nt_marker,segment);
    cout<<"segment.size = "<<segment.size()<<endl;
    for(V3DLONG i=0;i<segment.size();i++)
    {
        unsigned char* data1d = 0;
        V3DLONG im_cropped_sz[4];
        double original_o[3];
        get_sub_block(callback,1,segment[i]->markers,data1d,im_cropped_sz,i,original_o);

        cout<<"original_o = "<<original_o[0]<<endl;
//        vector<MyMarker*> nt_marker2;
//        QString temp_name = "tmp_"+QString::number(i)+".swc";
//        NeuronTree nt2 = mm2nt(segment[i]->markers,temp_name);
        LandmarkList marker_v;
        cout<<"segment[i]->markers.size() = "<<segment[i]->markers.size()<<endl;
        for(V3DLONG p=0;p<segment[i]->markers.size();p++)
        {
            //
            LocationSimple mm;
            mm.x = segment[i]->markers[p]->x-original_o[0];
            mm.y = segment[i]->markers[p]->y-original_o[1];
            mm.z = segment[i]->markers[p]->z-original_o[2];
            cout<<"mm = "<<mm.x<<"  "<<mm.y<<"  "<<mm.z<<endl;
            mm.color.a = 0;
            mm.color.b = 0;
            mm.color.g = 0;
            mm.color.r = 0;
            marker_v.push_back(mm);
        }
        v3d_msg("mean_shift");
        segment_mean_shift(data1d,marker_v,im_cropped_sz,i,segment[i]->markers);
        //if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}
