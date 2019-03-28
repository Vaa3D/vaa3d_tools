//#include <vector>
#include "branch_detect.h"

#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14159265359
#define MIN_DIST 10
#define VOID 1000000000
#ifndef SWC_RADIUS
#define SWC_RADIUS 5
#endif
#ifndef RX
#define RX 0.2
#endif
#ifndef RY
#define RY 0.2
#endif
#ifndef RZ
#define RZ 1
#endif

double marker_dist(MyMarker a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
using namespace std;


void get_branches(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString image_file=infiles.at(1);
//    QString swc_file = infiles.at(1);
    QString output_dir=outfiles.at(0);

    QString swc_file = infiles.at(0);

    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();
    QString output_fake = output_dir+"/"+flag1+"_fake.apo";
    QString output_branch = output_dir+"/"+flag1+"_branch.apo";

    printf("welcome to use get_branch\n");
    NeuronTree nt = readSWC_file(swc_file);
//    if(!output_dir.endsWith("/")){
//        output_dir = output_dir+"/";
//    }
    //QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
    //cell_name = cell_name.left(cell_name.indexOf("."));
    //QString output_file=output_dir;

    //cout<<"image loading"<<endl;
    // Find branch points
    V3DLONG *in_zz;
    cout<<"image file name is "<<image_file.toStdString()<<endl;
    if(!callback.getDimTeraFly(image_file.toStdString(), in_zz))
    {
//        v3d_msg("Cannot load terafly images.",0);
        cout<<"can not load terafly images"<<endl;
    }
    cout<<"check1"<<endl;
    XYZ block_size=XYZ(100,100,20);
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;

    QList<int> branch_list;
    QList<int> plist;
    QList<int> alln;
    QList<NeuronSWC> apo_list;
    QList<NeuronSWC> apo_branch;
    QList<int> parent;
    int N=nt.listNeuron.size();
    map<int, int> t;
    QList<int> branch;
    map<int,int > child1;
    map<int,int > child2;


    for(int i=0; i<N; i++){
        //qDebug() << nt.listNeuron.at(i).n << nt.listNeuron.at(i).pn;
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
        t.insert(pair<int,int>(plist.at(i),0));
    }

    for(int i=0; i<N; i++){
        t.at(plist.at(i))=t.at(plist.at(i))+1;
        if((plist.count(plist.at(i))>1)&(t.at(plist.at(i)) == 1))
        {
            //cout<<"pruning"<<endl;

//            int p_index;
//            p_index = plist.indexOf(i);
//            if(nt.listNeuron.at(p_index).pn!=-1){
//            branch.push_back(p_index);
            if(nt.listNeuron.at(i).pn!=-1){
            branch.push_back(nt.listNeuron.at(i).pn);
            //parent.push_back(p_index);
            child1.insert(pair<int,int>(plist.at(i),alln.at(i)));
            }
         }
        else if(t.at(plist.at(i)) ==2){
            child2.insert(pair<int,int>(plist.at(i),alln.at(i)));
        }
        }

//    for(map<int,int>::iterator it=child1.begin();it!=child1.end();it++){
//        cout<<it->first<<":"<<it->second<<endl;}
//    for(map<int,int>::iterator it=child2.begin();it!=child2.end();it++){
//        cout<<it->first<<":"<<it->second<<endl;}

        //cout<<"size="<<branch.size()<<endl;
    for(int i=0; i<branch.size(); i++){
            // p_index: line where branch point is in
            int p_index;
            p_index = alln.indexOf(branch[i]);
//            if(nt.listNeuron.at(p_index).pn!=-1){
//            cout<<"index"<<p_index<<endl;
            //double d = dist(nt.listNeuron.at(alln.at(i)),nt.listNeuron.at(p_index));
            //distance & intensity
        //int bid = alln.indexOf(plist.at(branch[i]));
        //cout<<plist.at(branch[i])<<" "<<branch[i]<<" "<<bid<<endl;
//            V3DLONG nodex = nt.listNeuron.at(branch[i]).x;
//            V3DLONG nodey = nt.listNeuron.at(branch[i]).y;
//            V3DLONG nodez = nt.listNeuron.at(branch[i]).z;
        V3DLONG nodex = nt.listNeuron.at(p_index).x;
        V3DLONG nodey = nt.listNeuron.at(p_index).y;
        V3DLONG nodez = nt.listNeuron.at(p_index).z;

            cout<<"check2"<<endl;

            //cout<<"cordinates:"<<nodex<<" ,"<<nodey<<" ,"<<nodez<<endl;
            //block
            if(p_index != 0){
            block crop_block = offset_block(zcenter_block, XYZ(nodex, nodey, nodez));

            XYZ small=XYZ(crop_block.small);
            XYZ large=XYZ(crop_block.large);
            small.x = floor(small.x);
            small.y = floor(small.y);
            small.z = floor(small.z);
            large.x = ceil(large.x)+1;
            large.y = ceil(large.y)+1;
            large.z = ceil(large.z)+1;
            unsigned char * cropped_image = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small.x, large.x,
                                                         small.y, large.y,
                                                         small.z, large.z);
//            cropped_image = callback.getSubVolumeTeraFly(image_file.toStdString(),
//                                                         10, 20,
//                                                         10, 20,
//                                                         10, 20);
            //cout<<"image"<<endl;
            V3DLONG mysz[4];
            mysz[0] = large.x-small.x;
            mysz[1] = large.y-small.y;
            mysz[2] = large.z-small.z;
            mysz[3] = in_zz[3];
            //cout<<"dim"<<mysz[0]<<" "<<mysz[1]<<" "<<mysz[2]<<" "<<mysz[3]<<endl;
            QString output_swc = output_dir+flag1+".eswc";
            //QString output_swc1=output_dir+"/";
            crop_swc_cuboid(nt, output_swc, crop_block);
            NeuronTree nt_crop_sorted;
            nt_crop_sorted=readSWC_file(output_swc);
            int branch_point=find_tip(nt_crop_sorted,mysz[0],mysz[1],mysz[2]);
            cout <<"#################"<<branch_point<<endl;
            V3DLONG bx = nt_crop_sorted.listNeuron.at(branch_point).x;
            V3DLONG by = nt_crop_sorted.listNeuron.at(branch_point).y;
            V3DLONG bz = nt_crop_sorted.listNeuron.at(branch_point).z;


            cout<<"check3"<<endl;

            unsigned char *data1d_crop=cropped_image;
            //distance
            int cchild1 = child1.at(branch[i]);
            int cchild2 = child2.at(branch[i]);

            //intensity
            //child1
            V3DLONG cx1 = nt.listNeuron.at(alln.indexOf(cchild1)).x;
            //V3DLONG cy1 = nt.listNeuron.at(child1.at(branch[i])).y;
            V3DLONG cy1 = nt.listNeuron.at(alln.indexOf(cchild1)).y;
            V3DLONG cz1 = nt.listNeuron.at(alln.indexOf(cchild1)).z;

            block crop_block_c1 = offset_block(zcenter_block, XYZ(cx1,cy1,cz1));
            XYZ small_1 = XYZ(crop_block_c1.small);
            XYZ large_1=XYZ(crop_block_c1.large);
            small_1.x = floor(small_1.x);
            small_1.y = floor(small_1.y);
            small_1.z = floor(small_1.z);
            large_1.x = ceil(large_1.x)+1;
            large_1.y = ceil(large_1.y)+1;
            large_1.z = ceil(large_1.z)+1;
            unsigned char * cropped_image_1 = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image_1 = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small_1.x, large_1.x,
                                                         small_1.y, large_1.y,
                                                         small_1.z, large_1.z);
            unsigned char *data1d_crop_1=cropped_image_1;



            QString out_swc1 = output_dir+flag1+"_child1.eswc";
            crop_swc_cuboid(nt, out_swc1, crop_block_c1);
            NeuronTree nt_sorted1;
            nt_sorted1=readSWC_file(out_swc1);
            int c1_point=find_tip(nt_sorted1,mysz[0],mysz[1],mysz[2]);
            V3DLONG childx1 = nt_sorted1.listNeuron.at(c1_point).x;
            V3DLONG childy1 = nt_sorted1.listNeuron.at(c1_point).y;
            V3DLONG childz1 = nt_sorted1.listNeuron.at(c1_point).z;



            //child2
            V3DLONG cx2 = nt.listNeuron.at(alln.indexOf(cchild2)).x;
            V3DLONG cy2 = nt.listNeuron.at(alln.indexOf(cchild2)).y;
            V3DLONG cz2 = nt.listNeuron.at(alln.indexOf(cchild2)).z;
            block crop_block_c2 = offset_block(zcenter_block, XYZ(cx2,cy2,cz2));
            XYZ small_2 = XYZ(crop_block_c2.small);
            XYZ large_2=XYZ(crop_block_c2.large);
            small_2.x = floor(small_2.x);
            small_2.y = floor(small_2.y);
            small_2.z = floor(small_2.z);
            large_2.x = ceil(large_2.x)+1;
            large_2.y = ceil(large_2.y)+1;
            large_2.z = ceil(large_2.z)+1;
            unsigned char * cropped_image_2 = 0;
            //cout<<"dim"<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z<<endl;
            cropped_image_2 = callback.getSubVolumeTeraFly(image_file.toStdString(),
                                                         small_2.x, large_2.x,
                                                         small_2.y, large_2.y,
                                                         small_2.z, large_2.z);
            unsigned char *data1d_crop_2=cropped_image_2;
            QString out_swc2 = output_dir+flag1+"_child2.eswc";
            crop_swc_cuboid(nt, out_swc2, crop_block_c2);
            NeuronTree nt_sorted2;
            nt_sorted2=readSWC_file(out_swc2);
            int c2_point=find_tip(nt_sorted2,mysz[0],mysz[1],mysz[2]);
            V3DLONG childx2 = nt_sorted2.listNeuron.at(c2_point).x;
            V3DLONG childy2 = nt_sorted2.listNeuron.at(c2_point).y;
            V3DLONG childz2 = nt_sorted2.listNeuron.at(c2_point).z;

            //angle: whether overlap or not
            int grandp;
            int bgp;
            bgp = plist.at(p_index);
            for (int k=0; k<5; k++){
                    grandp = alln.indexOf(bgp);
                    if(grandp>0){
                        bgp = plist.at(grandp);
                        continue;}
                    else{
                        break;
                    }
                }

            int branch_parent = grandp;
            V3DLONG bpx = nt.listNeuron.at(branch_parent).x;
            V3DLONG bpy = nt.listNeuron.at(branch_parent).y;
            V3DLONG bpz = nt.listNeuron.at(branch_parent).z;
            XYZ BP = XYZ(bpx-nodex,bpy-nodey,bpz-nodez);


            int child_11;
            int parent1 =cchild1;
            int p1;
            XYZ BC1;
            double ang1 = 0;
            double a1;
            p1 = cchild1;
            for (int k=0; k<5; k++){
                if(plist.count(p1) <2){
                    child_11 = plist.indexOf(p1);
                    //cout<<"child_11````````````````"<<child_11<<endl;
                    if(child_11>0){
                        p1 = alln.at(child_11);
                        BC1 = XYZ(nt.listNeuron.at(child_11).x-nodex,nt.listNeuron.at(child_11).y-nodey,nt.listNeuron.at(child_11).z-nodez);
                        a1 = Angle(BP,BC1);
                        if(a1>ang1){
                            ang1 = a1;
                            parent1 = p1;
                        }
                        continue;}
                    else if(child_11<0){
                        break;
                    }
                    }
                else{
                   int idx = alln.indexOf(parent1);
                   BC1 = XYZ(nt.listNeuron.at(idx).x-nodex,nt.listNeuron.at(idx).y-nodey,nt.listNeuron.at(idx).z-nodez);
                   ang1 = Angle(BP,BC1);
                   break;
                }
                }

            V3DLONG ct1x = nt.listNeuron.at(alln.indexOf(parent1)).x;
            V3DLONG ct1y = nt.listNeuron.at(alln.indexOf(parent1)).y;
            V3DLONG ct1z = nt.listNeuron.at(alln.indexOf(parent1)).z;


            int child_22;
            int parent2 =cchild2;
            int p2;
            double a2;
            double ang2=0;
            XYZ BC2;
            p2 = cchild2;
            for (int k=0; k<5; k++){
                if(plist.count(p2) <2){
                child_22 = plist.indexOf(p2);
                if(child_22>=0){
                    p2 = alln.at(child_22);
                    //cout<<"```````````"<<idx<<endl;
                    BC2 = XYZ(nt.listNeuron.at(child_22).x-nodex,nt.listNeuron.at(child_22).y-nodey,nt.listNeuron.at(child_22).z-nodez);
                    a2 = Angle(BP,BC2);
                    if(a2>ang2){
                        ang2 = a2;
                        parent2 = p2;
                    }
                    continue;}
                else if(child_22<0){
                    ang2 = a2;
                    break;
                }
                }
                else{
                    int idx = alln.indexOf(parent2);
                    BC2 = XYZ(nt.listNeuron.at(idx).x-nodex,nt.listNeuron.at(idx).y-nodey,nt.listNeuron.at(idx).z-nodez);
                    ang2 = Angle(BP,BC2);
                    break;
                }
            }

            V3DLONG ct2x = nt.listNeuron.at(alln.indexOf(parent2)).x;
            V3DLONG ct2y = nt.listNeuron.at(alln.indexOf(parent2)).y;
            V3DLONG ct2z = nt.listNeuron.at(alln.indexOf(parent2)).z;

            double ang3 = Angle(BC1,BC2);

            double d1;
            double d2;
            //find grandchildren
            int grandch1 = plist.indexOf(cchild1);
            int grandch2 = plist.indexOf(cchild2);
            if(grandch1 <0){
                d1 = dist(nt.listNeuron.at(cchild1), nt.listNeuron.at(p_index));
            //cout<<"cchild1 distance:"<<d1<<endl;
            }
            else{d1 = 50;}
            if(grandch2 <0){
                d2 = dist(nt.listNeuron.at(cchild2), nt.listNeuron.at(p_index));
            //cout<<"cchild2 distance:"<<d2<<endl;
            }
            else{d2 = 50;}
            //cout<<branch.at(i)<<"............."<<endl;
            if((data1d_crop_1[V3DLONG(childz1*mysz[0]*mysz[1]+childy1*mysz[0]+childx1)]>=10)&(data1d_crop_2[V3DLONG(childz2*mysz[0]*mysz[1]+childy2*mysz[0]+childx2)]>=10)){
            if((data1d_crop[V3DLONG(bz*mysz[0]*mysz[1]+by*mysz[0]+bx)]>=10)&(d1>10)&(d2>10)){
                //if((data1d_crop[V3DLONG(childz1*mysz[0]*mysz[1]+childy1*mysz[0]+childx1)]>signal1)&(data1d_crop[V3DLONG(childz2*mysz[0]*mysz[1]+childy2*mysz[0]+childx2)]>signal2)){
                //if(signal1>15 & signal2>15){
                if((ang1>5)&(ang2>5)&(ang3<175)){
                        branch_list.push_back(p_index);
                        NeuronSWC cur = nt.listNeuron.at(p_index);
                        apo_branch.push_back(cur);
                        //cout<<ang1<<"++++++++++++"<<ang2<<"+++++++++"<<ang2<<endl;
                    }
                    else{
                        NeuronSWC cur = nt.listNeuron.at(p_index);
                        apo_list.push_back(cur);
                        cout<<"Angles:!!!!!!!!!!!!!!"<<ang1<<" "<<ang2<<" "<<ang3<<endl;
                        cout<<branch_parent<<"@@@@@@@"<<alln.at(p_index)<<"@@@@@@@@@@@@"<<parent1<<"@@@@@@@@@@@@@"<<parent2<<endl;
                        cout<<nodex<<" "<<nodey<<" "<<nodez<<" "<<ct2x<<" "<<ct2y<<" "<<ct2z<<endl;
                    }
                //}
//                else{
//                    NeuronSWC cur = nt.listNeuron.at(branch[i]-1);
//                    apo_list.push_back(cur);
//                }
            }
            else{
                NeuronSWC cur = nt.listNeuron.at(p_index);
                apo_list.push_back(cur);
                cout<<d1<<" "<<d2<<"^^^^^^^^^^^^^"<<endl;
            }
            }
            else{
                NeuronSWC cur = nt.listNeuron.at(p_index);
                apo_list.push_back(cur);
            }
            cout<<"check55"<<endl;
    //}
            }
    }
    cout<<branch_list.size()<<"............."<<apo_list.size()<<endl;
    //create .apo
    unsigned int Vsize=50;
    QList<CellAPO> apo;

    for(int i = 0; i <apo_list.size();i++)
    {
        CellAPO m;
        m.x = apo_list.at(i).x;
        m.y = apo_list.at(i).y;
        m.z = apo_list.at(i).z;
        m.color.r=255;
        m.color.g=0;
        m.color.b=0;
        m.volsize = Vsize;
        apo.push_back(m);
    }
    writeAPO_file(output_fake,apo);

    QList<CellAPO> Bran;
    for(int i = 0; i <apo_branch.size();i++)
    {
        CellAPO b;
        b.x = apo_branch.at(i).x;
        b.y = apo_branch.at(i).y;
        b.z = apo_branch.at(i).z;
        b.color.r=0;
        b.color.g=255;
        b.color.b=0;
        b.volsize = Vsize;
        Bran.push_back(b);
    }

    //QString apo_name = filename + ".apo";

    writeAPO_file(output_branch,Bran);

    //cout<<"number of branch points "<<branch_list.size()<<endl;


    //QList<int> branch_list = get_branch_points(callback, nt, false, image_file,output_apo);
//    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;
    //    // Crop tip-centered regions one by one4
    //    block zcenter_block; // This is a block centered at (0,0,0)
    //    zcenter_block.small = 0-block_size/2;
    //    zcenter_block.large = block_size/2;
    //    QList<QString> output_suffix;
    //    output_suffix.append(QString("nrrd"));
    //    output_suffix.append(QString("swc"));

    //    for(int i=0; i<tip_list.size(); i++){
    //       if(i>0){break;}
    //        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
    //        qDebug()<<node.n;
    //        if(node.type > 5){continue;}
    //        // create a tip-centered block
    //        XYZ shift;
    //        shift.x = (int)node.x;
    //        shift.y = (int)node.y;
    //        shift.z = (int)node.z;

    //        block crop_block = offset_block(zcenter_block, shift);
    //        crop_block.name = cell_name + "_"+QString::number(i);
    //        // crop image
    //        qDebug()<<crop_block.name;
    //        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
    //        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

    //        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
    //        // crop swc
    //        QString output_swc = output_dir+crop_block.name+".swc";
    //        crop_swc(swc_file, output_swc, crop_block);
    //        my_saveANO(output_dir, crop_block.name, output_suffix);
    //    }
      return;
}

double Angle(XYZ p1,XYZ p2){
    double dot = p1.x*p2.x+p1.y*p2.y+p1.z*p2.z;
    double norm1 = sqrt(p1.x*p1.x+p1.y*p1.y+p1.z*p1.z);
    double norm2 = sqrt(p2.x*p2.x+p2.y*p2.y+p2.z*p2.z);
    double cross = norm1*norm2;
    if(cross == 0){
        return 0;
        cout<<"$$$$$$$$$$$$$$$"<<endl;
    }
    else{
        double theta = acos(dot/cross);
        theta = theta*180/PI;
        return theta;
    }

}



MyMarker adjust_size(MyMarker marker, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    marker.x *= ratio_x;
    marker.y *= ratio_y;
    marker.z *= ratio_z;
    return marker;
}
MyMarker readjust_size(MyMarker marker, double ratio_x=RX, double ratio_y=RY, double ratio_z=RZ){
    marker.x /= ratio_x;
    marker.y /= ratio_y;
    marker.z /= ratio_z;
    return marker;
}

double signal_at_markers(vector<MyMarker> allmarkers, unsigned char * data1d, long sz0, long sz1, long sz2)
{
//    printf("welcome to use signal_at_markers\n");
    double signal = 0;
    double in_block_ct = 0;
    long sz01 = sz0 * sz1;
    long total_sz = sz0 * sz1 * sz2;
    unsigned char * outimg1d=0;

    //cout<<"---------------markersize="<<allmarkers.size()<<endl;


    for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x;
        int y = allmarkers[i].y;
        int z = allmarkers[i].z;
        if((x<0) || (x>=sz0) || (y<0) || (y>=sz1) || (z<0) || (z>=sz2)){continue;} // out of image border
        int id = z*sz01+y*sz0+x;

        cout<<"id="<<id<<endl;
        cout<<"total_sz"<<total_sz<<endl;
//        cout<<id<<"\t";
        if(id<total_sz){
            signal += data1d[id];
            in_block_ct += 1;
            cout<<"%%%%%%%%%%%%%%%%%"<<signal<<endl;
        }
//        cout<<"pass"<<endl;
    }
//    qDebug()<<"Done collecting pixels";
    if(in_block_ct>0)
    {
        signal = signal / in_block_ct;
    }
    return signal;
}

bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2)
{
//    printf("welcome to use getMarkersBetween\n");
//    qDebug()<<m1.x<<m1.y<<m1.z;
//    qDebug()<<m2.x<<m2.y<<m2.z;

    double ratio_x = 1;
    double ratio_y = 1;
    double ratio_z = 5;
    m1 = adjust_size(m1, ratio_x, ratio_y, ratio_z);
    m2 = adjust_size(m2, ratio_x, ratio_y, ratio_z);

    double A = m2.x - m1.x;
    double B = m2.y - m1.y;
    double C = m2.z - m1.z;
    double R = m2.radius - m1.radius;
    double D = sqrt(A*A + B*B + C*C);
    A = A/D; B = B/D; C = C/D; R = R/D;

//    double ctz = A/sqrt(A*A + B*B);
//    double stz = B/sqrt(A*A + B*B);

//    double cty = C/sqrt(A*A + B*B + C*C);
//    double sty = sqrt(A*A + B*B)/sqrt(A*A + B*B + C*C);

    double x0 = m1.x;
    double y0 = m1.y;
    double z0 = m1.z;
    double r0 = m1.radius;

    set<MyMarker> marker_set;

    for(double t = 0.0; t <= marker_dist(m1, m2); t += 1.0)
    {
        MyMarker marker;
        int cx = x0 + A*t + 0.5;
        int cy = y0 + B*t + 0.5;
        int cz = z0 + C*t + 0.5;
        int radius = r0 + R*t + 0.5;
        int radius2 = radius * radius;

        for(int k = -radius; k <= radius; k++)
        {
            for(int j = -radius; j <= radius; j++)
            {
                for(int i = -radius; i <= radius; i++)
                {
                    if(i * i + j * j + k * k > radius2) continue;
                    double x = i, y = j, z = k;
//                    double x1, y1, z1;
//                    y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
//                    x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
//                    z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
                    x += cx; y += cy; z += cz;
                    marker = MyMarker(x, y, z);
                    marker = readjust_size(marker, ratio_x, ratio_y, ratio_z);
                    marker.x = (int)(marker.x+0.5);
                    marker.y = (int)(marker.y+0.5);
                    marker.z = (int)(marker.z+0.5);
                    marker_set.insert(marker);
                }
            }
        }
    }

    allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
//    qDebug()<<"Done getting markers";
    return true;
}

double signal_between_markers(unsigned char * data1d, NeuronSWC n1, NeuronSWC n2, long sz0, long sz1, long sz2, long sz3, V3DPluginCallback2 & callback)
{
//    qDebug()<<"welcome to use signal_between_markers\n";
    // To be implemented: read image as 1d image
    vector<MyMarker> allmarkers;  // allmarkers: pixels within the cylinder specified by n1&n2;
    //displacement

    MyMarker m1 = MyMarker(n1.x, n1.y,n1.z);
    MyMarker m2 = MyMarker(n2.x, n2.y,n2.z);
    m1.radius = SWC_RADIUS;
    m2.radius = SWC_RADIUS;
    getMarkersBetween(allmarkers, m1, m2);
    // Signal: average signal at allmarkers;
    double signal = signal_at_markers(allmarkers, data1d, sz0, sz1, sz2);
//    if(signal<MIN_DIST){
//        qDebug()<<"size of marker list:"<<allmarkers.size();
//    }
    qDebug()<<"size of marker list:"<<allmarkers.size();
    return signal;
}


bool  crop_swc_cuboid(NeuronTree nt, QString qs_output,block input_block)
{
    double xs=input_block.small.x; double ys=input_block.small.y; double zs=input_block.small.z;
    double xe=input_block.large.x; double ye=input_block.large.y; double ze=input_block.large.z;
    double xshift=input_block.small.x;double yshift=input_block.small.y; double zshift=input_block.small.z;
    printf("welcome to use crop_swc_cuboid\n");
    // 1. read input
    XYZ small(xs, ys, zs);
    XYZ large(xe, ye, ze);
    qDebug()<<small.x<<small.y<<small.z;
    qDebug()<<large.x<<large.y<<large.z;
    //NeuronTree nt = readSWC_file(qs_input);
    // 2. Decide which nodes are within the cuboid
    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(in_cuboid(node, small, large)){inside_nlist.append(node.n);}
    }
    qDebug()<<inside_nlist.size();
    //3. create a cropped tree
    NeuronTree new_tree;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        // If node is outside of the shell, skip to the next node.
        if(inside_nlist.lastIndexOf(node.n)<0){
            continue;
        }
        // If the parent node is to be deleted, put itself as a parent node.
        if(inside_nlist.lastIndexOf(node.pn)<0){
            node.pn = -1;
        }
        new_tree.listNeuron.append(node);
    }
    //NeuronSWC noderoot = nt.listNeuron.at(i);
    //new_tree.listNeuron.at(0).pn=-1;
    nt.deepCopy(my_SortSWC(new_tree, VOID, 0));

    //4. shift if needed
    if((xshift!=0) || (yshift!=0) || (zshift!=0)){
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].x -= xshift;
            nt.listNeuron[i].y -= yshift;
            nt.listNeuron[i].z -= zshift;
        }
    }
    // 5. save output
    export_list22file(nt.listNeuron, qs_output);
    return 1;
}
//NeuronTree my_SortSWC(NeuronTree nt, V3DLONG newrootid, double thres){
//    NeuronTree new_tree;
//    QList<NeuronSWC> neuronlist;
////    v3d_msg(QString("id is %1, ths is %2").arg(newrootid).arg(thres));
////    writeSWC_file("C:/Users/pengx/Desktop/test/tmp.swc",nt);
//    SortSWC(nt.listNeuron, neuronlist, newrootid, thres);
////    v3d_msg("done!");
//    new_tree.deepCopy(neuronlist_2_neurontree(neuronlist));
//    return new_tree;
//}
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}
bool in_cuboid(NeuronSWC node, XYZ small, XYZ large){
    if((node.x>=small.x) & (node.x<=large.x) &
            (node.y>=small.y) & (node.y<=large.y) &
            (node.z>=small.z) & (node.z<=large.z)
            )
    {
        return 1;
    }
    return 0;
}
bool export_list22file(const QList<NeuronSWC>& lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    bool eswc_flag=false;
    if(fileSaveName.section('.',-1).toUpper()=="ESWC")
        eswc_flag=true;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin neuron_connector"<<endl;
    if(eswc_flag)
        myfile<<"##n,type,x,y,z,radius,parent,segment_id,segment_layer,feature_value"<<endl;
    else
        myfile<<"##n,type,x,y,z,radius,parent"<<endl;
    for (V3DLONG i=0;i<lN.size();i++){
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn;
        if(eswc_flag){
            myfile<<" "<<lN.at(i).seg_id<<" "<<lN.at(i).level;
            for(int j=0; j<lN.at(i).fea_val.size(); j++)
                myfile <<" "<< lN.at(i).fea_val.at(j);
        }
        myfile << endl;
    }
    file.close();
    return true;
}
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2)
{
    // Return the node at center of the image as tip node
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node)<MIN_DIST ) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+1 ) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+2 ) {
            return i;
        }
    }
    printf("No tip found!\n");
}

block offset_block(block input_block, XYZ offset)
{
    input_block.small = offset_XYZ(input_block.small, offset);
    input_block.large = offset_XYZ(input_block.large, offset);
    return input_block;
}

// find other branch points in the same cropped block
vector< vector<int> > get_close_points(NeuronTree nt,vector<int> a){
    vector< vector<int> > neighbours;
    int n=a.size();
    for(int i=0; i<n; i++){
        vector<int> cp;
        NeuronSWC node1 = nt.listNeuron.at(a.at(i));
        int min_x = 50;
        int min_y = 50;
        int min_z = 10;
        for(int j=0; j<n; j++){
            NeuronSWC node2 = nt.listNeuron.at(a.at(j));
            if(i != j){
                if(abs(node1.x-node2.x) < min_x){
                    min_x = ceil(abs(node1.x-node2.x));
                }
                if(abs(node1.y-node2.y) < min_y){
                    min_y = ceil(abs(node1.y-node2.y));
                }
                if(abs(node1.z-node2.z) < min_z){
                    min_z = ceil(abs(node1.z-node2.z));
                }
           }
        }
        if(min_x<25){
            min_x = 25;
        }
        if(min_y<25){
            min_y = 25;
        }
        if(min_z<5){
            min_z = 5;
        }
        cp.push_back(min_x);
        cp.push_back(min_y);
        cp.push_back(min_z);
        neighbours.push_back(cp);
    }
    return neighbours;
}


XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}

//void crop_swc(QString input_swc, QString output_swc, block crop_block)
//{

////        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
////        // crop swc
////        QString output_swc = output_dir+crop_block.name+".swc";
////        crop_swc(swc_file, output_swc, crop_block);
//////        my_saveANO(output_dir, crop_block.name, output_suffix);
////    }
//    return;
//}

//void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
//{
//    vector<char*> infiles, inparas, outfiles;
//    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
//    QString input_swc=infiles.at(0);
//    QString input_image=inparas.at(0);
//    QString output_2d_dir=outfiles.at(0);
//    if(!output_2d_dir.endsWith("/")){
//        output_2d_dir = output_2d_dir+"/";
//    }
//    QStringList list=input_swc.split("/");
//    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
//    QString flag1=list1.first();//dont need to match list index
//    //printf("______________:%s\n",output_2d_dir.data());
//    qDebug()<<input_swc;
//    qDebug("number:%s",qPrintable(flag1));
//    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
//    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
//    int nChannel = p4dImage->getCDim();

//    V3DLONG mysz[4];
//    mysz[0] = p4dImage->getXDim();
//    mysz[1] = p4dImage->getYDim();
//    mysz[2] = p4dImage->getZDim();
//    mysz[3] = nChannel;
//    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
//    //printf("+++++++++++:%p\n",p4dImage);

//   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
//   unsigned char* data1d_mask = 0;
//   data1d_mask = new unsigned char [pagesz];
//   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
//   double margin=0;//by PHC 20170531
//   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], margin);
//   //QString labelSaveString = pathname + ".v3draw_label.tif";
//   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

//   V3DLONG stacksz =mysz[0]*mysz[1];
//   unsigned char *image_mip=0;
//   image_mip = new unsigned char [stacksz];//2D orignal image
//   unsigned char *label_mip=0;
//   label_mip = new unsigned char [stacksz];//2D annotation
//   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
//   {
//       V3DLONG offsetj = iy*mysz[0];
//       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
//       {
//           int max_mip = 0;
//           int max_label = 0;
//           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
//           {
//               V3DLONG offsetk = iz*mysz[1]*mysz[0];
//               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
//               {
//                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
//                   max_mip = data1d_crop[offsetk + offsetj + ix];
//               }
//               if(data1d_mask[offsetk + offsetj + ix] >= max_label)
//               {
//                   label_mip[iy*mysz[0] + ix] = data1d_mask[offsetk + offsetj + ix];
//                   max_label = data1d_mask[offsetk + offsetj + ix];
//               }
//           }
//       }
//   }
//   unsigned char* data1d_2D = 0;
//   data1d_2D = new unsigned char [3*stacksz];//3 channels image
//   for(V3DLONG i=0; i<stacksz; i++)
//       data1d_2D[i] = image_mip[i];

//   for(V3DLONG i=0; i<stacksz; i++)
//   {
//       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
//   }
//   for(V3DLONG i=0; i<stacksz; i++)
//       data1d_2D[i+2*stacksz] = image_mip[i];

//   mysz[2] = 1;
//   mysz[3] = 3;
//   QString mipoutpuut = output_2d_dir +flag1+"_"+"mip.tif";
//   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
//   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
//   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
//   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
//   if(image_mip) {delete [] image_mip; image_mip=0;}
//   if(label_mip) {delete [] label_mip; label_mip=0;}
//   //listNeuron.clear();
//}


LandmarkList get_missing_branches_menu(V3DPluginCallback2 &callback, QWidget *parent, Image4DSimple * p4DImage)
{
    LandmarkList candidates_m;

    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return candidates_m;

    // Get Neuron
    NeuronTree nt = openDlg->nt;
    QList<NeuronSWC> neuron = nt.listNeuron;

    // Get Image Data
    int nChannel = p4DImage->getCDim();

    V3DLONG mysz[4];
    mysz[0] = p4DImage->getXDim();
    mysz[1] = p4DImage->getYDim();
    mysz[2] = p4DImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);
    printf("+++++++++++:%p\n",p4DImage);

    vector<long> ids;
    vector<long> parents;
    // Reorder tree ids so that neuron.at(i).n=i+1
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn !=-1)
        {
            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
            parents.push_back(neuron.at(i).pn);
        }
    }

    // Get foreground points in a shell around SWC
    V3DLONG radius = 40;
    vector <struct XYZ> candidates;
    vector <struct XYZ> locswc;
    vector <struct XYZ> final_pts;
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        locswc.push_back(XYZ(neuron.at(id).x,neuron.at(id).y,neuron.at(id).z));
    }
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        V3DLONG nodex = neuron.at(id).x;
        V3DLONG nodey = neuron.at(id).y;
        V3DLONG nodez = neuron.at(id).z;
        //qDebug() << nodex << nodey << nodez;
        for(double theta=0; theta<PI; theta+=0.8) // Check the step in function of the radius
        {
            //qDebug() << theta;
            for(double phi=0; phi<2*PI; phi+=0.8)
            {
                //qDebug() << phi;
                struct XYZ shellp = XYZ(nodex+round(radius*sin(theta)*cos(phi)),nodey+round(radius*sin(theta)*sin(phi)),nodez+round(radius*cos(theta)/5));
                if((shellp.x<0) || (shellp.x>=mysz[0]) || (shellp.y<0) || (shellp.y>=mysz[1]) || (shellp.z<0) || (shellp.z>=mysz[2])){continue;} // out of image border
                for(int i=0; i<neuron.size(); i++)
//                for(int i=1; i<80; i++)
                {
                    //qDebug() << i;
                    // Checks whether a point is in a shell of radius 20<r<40 pixels and if intensity >40
                    if(dist_L2(shellp,locswc.at(i))>=20 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);
                }
            }
        }
    }
    cout << endl;
    for (V3DLONG i=0; i<candidates.size(); i++)
    {
        LocationSimple candidate;
        candidate.x = candidates.at(i).x;
        candidate.y = candidates.at(i).y;
        candidate.z = candidates.at(i).z;

        candidates_m.push_back(candidate);
    }
    qDebug() << "Candidates vector size is:" << candidates.size();
    return candidates_m;
}
