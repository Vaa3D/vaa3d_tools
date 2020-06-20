#include <QCoreApplication>
#include "manage.h"
#include "customdebug.h"



#define IMAGEDIR "image"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMsgHandler(customMessageHandler);
    ManageServer manageserver;


//    V_NeuronSWC__2__NeuronTree(seg_temp);
//        SS.nameQString filename(":/new/prefix1/C:/Users/Brain/Desktop/resample_18465_00303_SYY_YLL_YLL_stamp_2019_09_27_10_40.ano.eswc");
//        NeuronTree L0=readSWC_file(filename);
//        NeuronTree L2=readSWC_file(":/new/prefix1/C:/Users/Brain/Desktop/18465_00303.ano.eswc");

//        QList<NeuronTree> MergedNT;

//        V_NeuronSWC_list V_L2= NeuronTree__2__V_NeuronSWC_list(L2);
//        V_NeuronSWC_list V_L0= NeuronTree__2__V_NeuronSWC_list(L0);
//        for(int i=0;i<V_L2.seg.size();i++)
//        {
//            NeuronTree SS;
//            V_NeuronSWC seg_temp =  V_L2.seg.at(i);
//            seg_temp.reverse();
//            for(int i=0;i<seg_temp.row.size();i++)
//            {
//                seg_temp.row.at(i).type=2;
//                seg_temp.row.at(i).r=1;
//            }
//            SS =  = "loaded_" + QString("%1").arg(i);
//        if (SS.listNeuron.size()>=2)
//            MergedNT.push_back(SS);
//    }

////    qDebug()<<"gdhksjgfkhjsagfkhj\n\n\n";
//    QList<NeuronTree> ColordNT;
//    for(int i=0;i<V_L0.seg.size();i++)
//    {
//        NeuronTree SS,SS_Color;
//        V_NeuronSWC seg_temp =  V_L0.seg.at(i);
//        V_NeuronSWC seg_temp_color=seg_temp;
//        seg_temp.reverse();
//        for(int i=0;i<seg_temp.row.size();i++)
//        {
//            seg_temp.row.at(i).type=3;
//            seg_temp.row.at(i).x+=2;
//            seg_temp.row.at(i).y+=2;
//            seg_temp.row.at(i).z+=2;
////            seg_temp_color.row.at(i).type=seg_temp_color.row.at(i).r/10;
//        }
//        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
////        SS_Color=V_NeuronSWC__2__NeuronTree(seg_temp_color);

//        if (SS.listNeuron.size()>=2)
//        {
//            MergedNT.push_back(SS);
////            ColordNT.push_back(SS_Color);
//        }
//    }

//    V_NeuronSWC_list tosave;

//    tosave.clear();
//    for(int i=0;i<MergedNT.size();i++)
//    {
//        NeuronTree ss=MergedNT.at(i);
//        V_NeuronSWC ss_temp=NeuronTree__2__V_NeuronSWC_list(ss).seg.at(0);
//        tosave.seg.push_back(ss_temp);
//    }

//    NeuronTree wholeNT=V_NeuronSWC_list__2__NeuronTree(tosave);
//    writeESWC_file("result.swc",wholeNT);

//    qDebug()<<"sadjkhj";

    return a.exec();
}




