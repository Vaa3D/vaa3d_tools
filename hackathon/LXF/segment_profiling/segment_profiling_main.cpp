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
    QString path = QFileInfo(".").absolutePath();
    int pos = path.lastIndexOf(QChar('/'));
    //v3d_msg();
    QString path_curr = path.left(pos+1);
    QList<NeuronTree> sketchedNTList;
    QList<QList<IMAGE_METRICS> > CSVlist;
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
        PA.curr_path = path_curr;

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
        system("mkdir ori_swc");
        writeSWC_file(QString("ori_swc/outtree"+QString::number(i)+".swc"),outtree);
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

        profile_swc(callback,PA,CSVlist);
        if(PA.data1d) {delete []PA.data1d; PA.data1d = 0;}
    }
    QString output_csv_file ="result.csv";
    writeCSV(CSVlist,output_csv_file);
    return true;

}
bool writeCSV(QList<QList<IMAGE_METRICS> > &CSVlist, QString output_csv_file)
{
    QFile file(output_csv_file);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<output_csv_file.toStdString().c_str() << endl;
        return false;
    }
    else
    {
        QTextStream stream (&file);
        stream<< "segment_type,num_of_nodes,dynamic_range,cnr,snr,tubularity_mean,tubularity_std,fg_mean,fg_std,bg_mean,bg_std"<<"\n";
        for (int j  = 0; j < CSVlist.size() ; j++)
        {

            QList<IMAGE_METRICS> result_metrics = CSVlist[j];
            for (int i  = 0; i < result_metrics.size() ; i++)
            {
                stream << result_metrics[i].type       <<","
                       << result_metrics[i].num_of_nodes <<","
                       << result_metrics[i].dy         <<","
                       << result_metrics[i].cnr        <<","
                       << result_metrics[i].snr        <<","
                       << result_metrics[i].tubularity_mean <<","
                       << result_metrics[i].tubularity_std <<","
                       << result_metrics[i].fg_mean    <<","
                       << result_metrics[i].fg_std     <<","
                       << result_metrics[i].bg_mean    <<","
                       << result_metrics[i].bg_std     <<"\n";
            }
        }
    file.close();
    }
    return true;

}
