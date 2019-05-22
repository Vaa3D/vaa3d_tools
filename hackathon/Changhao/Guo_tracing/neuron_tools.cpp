#include "neuron_tools.h"
#include <math.h>
#include "ClusterAnalysis_20190409.h"
#define PI 3.1415926
#define INF 1E9

// lroundf() is gcc-specific --CMB
#ifdef _MSC_VER
inline long lroundf(float num) { return static_cast<long>(num > 0 ? num + 0.5f : ceilf(num - 0.5f)); }
#endif

QList <ImageMarker> LandmarkListQList_ImageMarker(LandmarkList marker_l)
{
    QList <ImageMarker> markers;
    for(V3DLONG i=0;i<marker_l.size();i++)
    {
        ImageMarker temp;
        temp.x=marker_l.at(i).x;
        temp.y=marker_l.at(i).y;
        temp.z=marker_l.at(i).z;
        temp.z=marker_l.at(i).z;
        temp.radius=marker_l.at(i).radius;
        temp.shape=marker_l.at(i).shape;
        temp.name=QString::fromStdString(marker_l.at(i).name);
        temp.color.r=marker_l.at(i).color.r;
        temp.color.g=marker_l.at(i).color.g;
        temp.color.b=marker_l.at(i).color.b;
        markers.push_back(temp);
    }
    return markers;


}

LandmarkList QList_ImageMarker2LandmarkList(QList <ImageMarker> markers)
{
    LandmarkList marker_l;
    for(V3DLONG i=0;i<markers.size();i++)
    {
        LocationSimple temp;
        temp.x=markers.at(i).x;
        temp.y=markers.at(i).y;
        temp.z=markers.at(i).z;
        temp.z=markers.at(i).z;
        temp.radius=markers.at(i).radius;
        temp.shape=PxLocationMarkerShape(markers.at(i).shape);
        temp.name=markers.at(i).name.toStdString();
        temp.color.r=markers.at(i).color.r;
        temp.color.g=markers.at(i).color.g;
        temp.color.b=markers.at(i).color.b;
        marker_l.push_back(temp);
    }
    return marker_l;

}


LocationSimple MyMarker2LocationSimple(MyMarker* marker)
{
    RGBA8 red;
    red.r=255;
    red.g=0;
    red.b=0;
   LocationSimple s;
   s.x=marker->x;
   s.y=marker->y;
   s.z=marker->z;
   s.radius=marker->radius;
   s.color = red;
   return s;
}

LocationSimple MyMarker2LocationSimple(MyMarker marker)
{
    RGBA8 red;
    red.r=255;
    red.g=0;
    red.b=0;
   LocationSimple s;
   s.x=marker.x;
   s.y=marker.y;
   s.z=marker.z;
   s.radius=marker.radius;
   s.color = red;
   return s;
}
//LandmarkList FromNeurontreeGetLeaf(NeuronTree nt)
//{
//    LocationSimple s;
//    RGBA8 red;
//    red.r=255;
//    red.g=0;
//    red.b=0;
////    RGBA8 green;
////    green.r=0;
////    green.g=255;
////    green.b=0;
//    V3DLONG size = nt.listNeuron.size();
//    cout<<"nt.listNeuron.size() "<<size<<endl;
//    QList<NeuronSWC> neuron = nt.listNeuron;
//    int *countOfNeuron;
//    try{countOfNeuron=new int [size];}
//    catch(...) {v3d_msg("cannot allocate memory for countOfNeuron."); return ;}
//    for (V3DLONG i=0;i<size;i++)
//    {
//       countOfNeuron[i]=0;
//    }
//    for (V3DLONG i=0;i<size;i++)
//    {
//        int ind=neuron.at(i).parent;
//        if(ind-1>=0&&ind-1<size)
//        {countOfNeuron[ind-1]++;}
//        else if(ind==-1)
//        {
//            countOfNeuron[i]--;
//        }
//    }
//    LandmarkList swc_tip_list;
////    LandmarkList swc_branch_list;
//    for(V3DLONG i=0;i<size;i++)
//    {
//        if(countOfNeuron[i]==0)
//        {
//            s.x=neuron.at(i).x;
//            s.y=neuron.at(i).y;
//            s.z=neuron.at(i).z;
//            s.radius=neuron.at(i).r;
//            s.color = red;
//            swc_tip_list<<s;
//            //tip
//        }
////        else if(countOfNeuron[i]>=2)
////        {
////            s.x=neuron.at(i).x;
////            s.y=neuron.at(i).y;
////            s.z=neuron.at(i).z;
////            s.radius=neuron.at(i).r;
////            s.color = green;
////            swc_branch_list<<s;
////            //branch
////        }
//    }
//    return swc_tip_list;
//}

MyMarker* LocationSimple2MyMarkerP(LocationSimple s)
{
    MyMarker* marker=new MyMarker;
    marker->x=s.x;
    marker->y=s.y;
    marker->z=s.z;
    marker->radius=s.radius;
    return marker;
}
MyMarker LocationSimple2MyMarker(LocationSimple s)
{
    MyMarker marker;
    marker.x=s.x;
    marker.y=s.y;
    marker.z=s.z;
    marker.radius=s.radius;
    return marker;
}
vector<MyMarker*> readSWC_file1(string swc_file)
{
    vector<MyMarker*> swc;
    ifstream ifs(swc_file.c_str());
    if(ifs.fail())
    {
        cout<<"open swc file : "<< swc_file <<" error"<<endl;
        return swc;
    }
    map<int, MyMarker*> marker_map;
    map<MyMarker*, int> parid_map;
    while(ifs.good())
    {
        if(ifs.peek() == '#'){ifs.ignore(1000,'\n'); continue;}
        MyMarker *  marker = new MyMarker;
        int my_id = -1 ; ifs >> my_id;
        if(my_id == -1) break;
        if(marker_map.find(my_id) != marker_map.end())
        {
            cerr<<"Duplicate Node. This is a graph file. Please read is as a graph."<<endl; return vector<MyMarker*>();
        }
        marker_map[my_id] = marker;

        ifs>> marker->type;
        ifs>> marker->x;
        ifs>> marker->y;
        ifs>> marker->z;
        ifs>> marker->radius;
        int par_id = -1; ifs >> par_id;

        parid_map[marker] = par_id;
        swc.push_back(marker);
    }
    ifs.close();
    vector<MyMarker*>::iterator it = swc.begin();
    while(it != swc.end())
    {
        MyMarker * marker = *it;
        marker->parent = marker_map[parid_map[marker]];
        it++;
    }
    return swc;
}

bool writeSWC_file1(const QString& filename, const NeuronTree& nt)
{
    QString curFile = filename;
    if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
    {
        curFile = QFileDialog::getSaveFileName(0,
                                               "Select a SWC file to save the neuronal or relational data... ",
                                               ".swc",
                                               QObject::tr("Neuron structure file (*.swc);;(*.*)"
                                                           ));
        if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
            return false;
    }

    FILE * fp = fopen(curFile.toLatin1(), "wt");
    if (!fp)
    {
        return false;
    }
    fprintf(fp, "#name %s\n", qPrintable(nt.name.trimmed()));
    fprintf(fp, "#comment %s\n", qPrintable(nt.comment.trimmed()));
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    NeuronSWC * p_pt=0;
    for (int i=0;i<nt.listNeuron.size(); i++)
    {
        p_pt = (NeuronSWC *)(&(nt.listNeuron.at(i)));
        fprintf(fp, "%ld %d %f %f %f %f %ld\n",
                p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn);
    }

    fclose(fp);
    return true;
}
bool saveSWC_file1(string swc_file, vector<MyMarker*> & outmarkers)
{
    if(swc_file.find_last_of(".dot") == swc_file.size() - 1) return saveDot_file(swc_file, outmarkers);
    map<MyMarker*, int> ind;
    ofstream ofs(swc_file.c_str());
    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }
    ofs<<"#name "<<swc_file<<endl;
    ofs<<"##n,type,x,y,z,radius,parent"<<endl;
    for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

    for(int i = 0; i < outmarkers.size(); i++)
    {
        MyMarker * marker = outmarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[marker->parent];
        ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
    }
    ofs.close();
    return true;
}

vector<MyMarker*>  NeuronTree2vectorofMyMarker(NeuronTree nt)
{
//    //copy NeuronTree to vector<MyMarker*>
    vector<MyMarker*> swc;

//    map<int, MyMarker*> marker_map;
//    map<MyMarker*, int> parid_map;
//    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
//    {
//        MyMarker *  marker = new MyMarker;
//        marker_map[i] = marker;

//        marker->type=nt.listNeuron.at(i).type;
//        marker->x =nt.listNeuron.at(i).x;
//        marker->y=nt.listNeuron.at(i).y;
//        marker->z=nt.listNeuron.at(i).z;
//        marker->radius=nt.listNeuron.at(i).radius;
////        marker->parent=nt.listNeuron.at(i).parent;
//        parid_map[marker] = nt.listNeuron.at(i).parent;
//        swc.push_back(marker);
//    }

//    vector<MyMarker*>::iterator it = swc.begin();
//    while(it != swc.end())
//    {
//        MyMarker * marker = *it;
//        marker->parent = marker_map[parid_map[marker]];
//        it++;
//    }
    QString tempname="tempname_for_transfer_1.swc";
    writeSWC_file1(tempname,nt);
    swc=readSWC_file1(tempname.toStdString());

    return swc;

}

NeuronTree merge_two_neuron(NeuronTree swc1 ,NeuronTree nt)
{
    cout<<"begin to merge two neuron";
    vector<MyMarker*> tree_new=NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*>  swc=NeuronTree2vectorofMyMarker(swc1);

    for(V3DLONG j=0;j<tree_new.size();j++)
    {
        swc.push_back(tree_new.at(j));
    }

    NeuronTree temp_tree=vectorofMyMarker2NeuronTree(swc);

    if(0)
    {
//        cout<<" free memory ";
        //free memory
        for(V3DLONG j=0;j<tree_new.size();j++)
        {
            if(tree_new.at(j))
            {free(tree_new.at(j));}

        }
        for(V3DLONG i=0;i<tree_new.size();i++)
        {
            if(swc.at(i))
            {free(swc.at(i));}
//            free(swc.at(i));
        }
    }
    return temp_tree;
}

void merge_two_neuron(NeuronTree nt1 ,NeuronTree nt2, NeuronTree &nt_merged)
{
    cout<<"begin to merge two neuron";
    vector<MyMarker*> tree_new=NeuronTree2vectorofMyMarker(nt2);
    vector<MyMarker*>  swc=NeuronTree2vectorofMyMarker(nt1);

    for(V3DLONG j=0;j<tree_new.size();j++)
    {
        swc.push_back(tree_new.at(j));
    }

    nt_merged=vectorofMyMarker2NeuronTree(swc);

    if(0)
    {
//        cout<<" free memory ";
        //free memory
        for(V3DLONG j=0;j<tree_new.size();j++)
        {
            if(tree_new.at(j))
            {free(tree_new.at(j));}

        }
        for(V3DLONG i=0;i<tree_new.size();i++)
        {
            if(swc.at(i))
            {free(swc.at(i));}
//            free(swc.at(i));
        }
    }
}

void merge_two_neuron(vector<MyMarker*> & swc ,NeuronTree nt)
{
    //
    vector<MyMarker*> tree_new=NeuronTree2vectorofMyMarker(nt);
    for(V3DLONG i=0;i<swc.size();i++)
    {
        MyMarker* temp_point_swc=swc.at(i);
        for(V3DLONG j=0;j<tree_new.size();j++)
        {
            MyMarker* temp_point_nt=tree_new.at(j);
            if(dist(*temp_point_swc,*temp_point_nt)<0.01)
            {
                //need to merge
                for(V3DLONG k=0;k<tree_new.size();k++)
                {
                    MyMarker* temp_point_nt_p=tree_new.at(k);
                    if(temp_point_nt_p->parent==temp_point_nt)
                    {
                        temp_point_nt_p->parent=temp_point_swc;
                    }
                }
                tree_new[j]=tree_new.back();
                j=j-1;
                tree_new.pop_back();
                free(temp_point_nt);
            }
        }
    }
    for(V3DLONG j=0;j<tree_new.size();j++)
    {
        swc.push_back(tree_new.at(j));
    }
}

void merge_two_neuron(vector<MyMarker*> & swc_main ,vector<MyMarker*> & swc)
{
    for(V3DLONG i=0;i<swc_main.size();i++)
    {
        MyMarker* temp_point_swc=swc_main.at(i);
        for(V3DLONG j=0;j<swc.size();j++)
        {
            MyMarker* temp_point_nt=swc.at(j);
            if(dist(*temp_point_swc,*temp_point_nt)<0.01)
            {
                //need to merge
                for(V3DLONG k=0;k<swc.size();k++)
                {
                    MyMarker* temp_point_nt_p=swc.at(k);
                    if(temp_point_nt_p->parent==temp_point_nt)
                    {
                        temp_point_nt_p->parent=temp_point_swc;
                    }
                }
                swc[j]=swc.back();
                j=j-1;
                swc.pop_back();
                free(temp_point_nt);
            }
        }
    }
    for(V3DLONG j=0;j<swc.size();j++)
    {
        swc_main.push_back(swc.at(j));
    }
}

NeuronTree readSWC_file1(const QString& filename)
{
    NeuronTree nt;
    nt.file = QFileInfo(filename).absoluteFilePath();
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return nt;
    }
    int count = 0;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    QString name = "";
    QString comment = "";
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
        //  add #name, #comment
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
            if (buf[1]=='n'&&buf[2]=='a'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]==' ')
                name = buf+6;
            if (buf[1]=='c'&&buf[2]=='o'&&buf[3]=='m'&&buf[4]=='m'&&buf[5]=='e'&&buf[6]=='n'&&buf[7]=='t'&&buf[8]==' ')
                comment = buf+9;

            continue;
        }
        count++;
        NeuronSWC S;
        QStringList qsl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;
        for (int i=0; i<qsl.size(); i++)
        {
            qsl[i].truncate(99);
            if (i==0) S.n = qsl[i].toInt();
            else if (i==1) S.type = qsl[i].toInt();
            else if (i==2) S.x = qsl[i].toFloat();
            else if (i==3) S.y = qsl[i].toFloat();
            else if (i==4) S.z = qsl[i].toFloat();
            else if (i==5) S.r = qsl[i].toFloat();
            else if (i==6) S.pn = qsl[i].toInt();
            //the ESWC extension, by PHC, 20120217
            else if (i==7) S.seg_id = qsl[i].toInt();
            else if (i==8) S.level = qsl[i].toInt();
    //change ESWC format to adapt to flexible feature number, by WYN, 20150602
            else
        S.fea_val.append(qsl[i].toFloat());
       }
        //if (! listNeuron.contains(S)) // 081024
        {
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    if (listNeuron.size()<1)
        return nt;
    //now update other NeuronTree members
    nt.n = 1; //only one neuron if read from a file
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');
    return nt;
}

NeuronTree vectorofMyMarker2NeuronTree( vector<MyMarker*> swc )
{
    //copy  vector<MyMarker*> to NeuronTree
    // change two  listNeuron
    NeuronTree nt;
//    map<MyMarker*, int> ind;
//    QHash <int, int>  hashNeuron;
//    hashNeuron.clear();
//    for(int i = 0; i < swc.size(); i++)
//        ind[swc[i]] = i;

//    for(int i = 0; i < swc.size(); i++)
//    {
//        MyMarker * marker = swc[i];
//        int parent_id;
//        if(marker->parent == 0) parent_id = -1;
//        else parent_id = ind[marker->parent];
//        NeuronSWC S;
//        S.n = i;
//        S.type =marker->type;
//        S.x=marker->x;
//        S.y=marker->y;
//        S.z=marker->z;
//        S.radius=marker->radius;
//        S.parent=parent_id;
//        nt.listNeuron.append(S);
//        hashNeuron.insert(i, nt.listNeuron.size()-1);
//    }
//    nt.hashNeuron = hashNeuron;

    QString tempname="tempname_for_transfer_2.swc";
    saveSWC_file1(tempname.toStdString(),swc);
    nt=readSWC_file1(tempname);

    return nt;
}
#define TMI_Data
#ifdef TMI_Data
LandmarkList Readdata_from_TMI_result(QString img_name)
{
    LandmarkList curlist;
    //input image name return LandmarkList
    //v3dhandle curwin = callback.currentImageWindow();
    //img_name=callback.getImageName(curwin);
    QFileInfo fileinfo(img_name);
    QString path_tp=fileinfo.path();
//    cout<<"path_tp before "<<path_tp.toUtf8().data()<<endl;
    path_tp.replace("images","enhanced");//replace dir
    path_tp.append("//");
    path_tp.append(fileinfo.baseName());
    path_tp.append("result");

    QDir dir;
    if(!dir.exists(path_tp))
    {
        QString errormsg="dir don't exist ";
        errormsg.append(path_tp);

        v3d_msg(errormsg);
        return curlist;
    }
//    cout<<"path_tp after append "<<path_tp.toUtf8().data()<<endl;
    //D:\dateset\TMI_data\BN_noisy\images\BN5_1.tif
    //‪D:\dateset\TMI_data\BN_noisy\enhanced\BN5_1result\1030_BN5_1_yuantu.marker
    //

    QString marker_filename=path_tp;
    marker_filename.append("//");
    marker_filename.append("1030_");
    marker_filename.append(fileinfo.baseName());
    marker_filename.append("_yuantu.marker");
    cout<<"marker_filename"<<marker_filename.toUtf8().data()<<endl;
//        writeMarker_file(marker_filename,LandmarkListQList_ImageMarker(tip_true_list));
    curlist=QList_ImageMarker2LandmarkList(readMarker_file(marker_filename));
    RGBA8 white;
    white.r=255;white.g=255;white.b=255;
    for(int i=0;i<curlist.size();i++)
    {
        curlist[i].color.r=0;
        curlist[i].color.g=255;
        curlist[i].color.b=0;
    }
//
    return curlist;
}
//LandmarkList Match_leaf_tip(LandmarkList leaf_swc,LandmarkList real_tip,double threshold=5,bool ignore_z=false)
//{
//    if(leaf_swc.isEmpty())
//    {
//        v3d_msg("Match_leaf_tipthe input leaf_swc isempty");
//        return leaf_swc;
//    }
//    if(real_tip.isEmpty())
//    {
//        v3d_msg("Match_leaf_tipthe input real_tip isempty");
//        return leaf_swc;
//    }

//    for(V3DLONG num=0;num<leaf_swc.size();num++)
//    {
//        LocationSimple leafpoint;
//        leafpoint.x=leaf_swc.at(num).x;
//        leafpoint.y=leaf_swc.at(num).y;
//        leafpoint.z=leaf_swc.at(num).z;
//        leafpoint.name="leaf_need_prun";
//        for(V3DLONG num2=0;num2<leaf_swc.size();num2++)
//        {

//        }


//    }




//}

LandmarkList FromTreeGetBranch(NeuronTree nt)
{
    cout<<"Using 1st method to find branchPoint:";
    LandmarkList point_list;
    LocationSimple s;
    RGBA8 red,green;
    red.r=255;      red.g=0;        red.b=0;
    green.r=0;      green.g=255;    green.b=0;
    V3DLONG neuronNum = nt.listNeuron.size();
    V3DLONG *childs_num;
    childs_num=new V3DLONG[neuronNum];
    for(V3DLONG i=0;i<neuronNum;i++)
    {
        childs_num[i]=0;
    }

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) {
            childs_num[i]--;
            continue;
        }
        childs_num[nt.hashNeuron.value(par)]++;
    }
    //get every leaf node of every branch
    V3DLONG branchcount=0;
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs_num[i]>=2)
       {
           s.x=nt.listNeuron.at(i).x;
           s.y=nt.listNeuron.at(i).y;
           s.z=nt.listNeuron.at(i).z;
           s.radius=nt.listNeuron.at(i).r;
           s.color = red;
           point_list<<s;
           branchcount++;
       }
    }

    if(point_list.size()!=0)
    {   cout<<" Success!"<<endl;}
    else
    {
        cout<<" Failure!"<<endl;
        cout<<"Using 2nd method to find branchPoint:";
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            if(nt.listNeuron[i].pn==-1)
            {
                s.x=nt.listNeuron.at(i).x;
                s.y=nt.listNeuron.at(i).y;
                s.z=nt.listNeuron.at(i).z;
                s.radius=nt.listNeuron.at(i).r;
                s.color = red;
                point_list<<s;
            }
        }
    }

    if(point_list.size()!=0)
    {   cout<<" Success!"<<endl;}
    else
    {
        cout<<"Failure!"<<endl;
        //other method to find branchPoint
    }

    return point_list;
}

LandmarkList getCalcuMarker(LandmarkList markerFromMarkerFile)
{
    LandmarkList calcuMarker;
    LocationSimple temMarker;
    for(V3DLONG i = 0; i < markerFromMarkerFile.size(); i++)
    {
        temMarker.x = markerFromMarkerFile.at(i).x-1;
        temMarker.y = markerFromMarkerFile.at(i).y-1;
        temMarker.z = markerFromMarkerFile.at(i).z-1;
        temMarker.radius = markerFromMarkerFile.at(i).radius;
        temMarker.shape = markerFromMarkerFile.at(i).shape;
        temMarker.name = markerFromMarkerFile.at(i).name;
        temMarker.comments = markerFromMarkerFile.at(i).comments;
        temMarker.category = markerFromMarkerFile.at(i).category;
        temMarker.color = markerFromMarkerFile.at(i).color;
        temMarker.ave = markerFromMarkerFile.at(i).ave;
        temMarker.sdev = markerFromMarkerFile.at(i).sdev;
        temMarker.skew = markerFromMarkerFile.at(i).skew;
        temMarker.curt = markerFromMarkerFile.at(i).curt;
        temMarker.on = markerFromMarkerFile.at(i).on;
        calcuMarker.push_back(temMarker);
    }
    return calcuMarker;
}

void getCalcuMarker(LandmarkList markerFromMarkerFile, LandmarkList &calcuMarker)
{
    LocationSimple temMarker;
    for(V3DLONG i = 0; i < markerFromMarkerFile.size(); i++)
    {
        temMarker.x = markerFromMarkerFile.at(i).x-1;
        temMarker.y = markerFromMarkerFile.at(i).y-1;
        temMarker.z = markerFromMarkerFile.at(i).z-1;
        temMarker.radius = markerFromMarkerFile.at(i).radius;
        temMarker.shape = markerFromMarkerFile.at(i).shape;
        temMarker.name = markerFromMarkerFile.at(i).name;
        temMarker.comments = markerFromMarkerFile.at(i).comments;
        temMarker.category = markerFromMarkerFile.at(i).category;
        temMarker.color = markerFromMarkerFile.at(i).color;
        temMarker.ave = markerFromMarkerFile.at(i).ave;
        temMarker.sdev = markerFromMarkerFile.at(i).sdev;
        temMarker.skew = markerFromMarkerFile.at(i).skew;
        temMarker.curt = markerFromMarkerFile.at(i).curt;
        temMarker.on = markerFromMarkerFile.at(i).on;
        calcuMarker.push_back(temMarker);
    }
}

void getCalcuMarker(LandmarkList markerFromMarkerFile, vector<MyMarker>& calcuMarker)
{
    MyMarker temMarker;
    for(V3DLONG i = 0; i < markerFromMarkerFile.size(); i++)
    {
        temMarker.x = markerFromMarkerFile.at(i).x-1;
        temMarker.y = markerFromMarkerFile.at(i).y-1;
        temMarker.z = markerFromMarkerFile.at(i).z-1;
        temMarker.radius = markerFromMarkerFile.at(i).radius;
        calcuMarker.push_back(temMarker);
    }

}

void displayMarker(V3DPluginCallback2 &callback, v3dhandle windows, LandmarkList calcuMarker)
{
    LandmarkList MarkerforDisplaying;
    LocationSimple temMarker;
    for(V3DLONG i = 0; i < calcuMarker.size(); i++)
    {
        temMarker.x = calcuMarker.at(i).x+1;
        temMarker.y = calcuMarker.at(i).y+1;
        temMarker.z = calcuMarker.at(i).z+1;
        temMarker.radius = calcuMarker.at(i).radius;
        temMarker.shape = calcuMarker.at(i).shape;
        temMarker.name = calcuMarker.at(i).name;
        temMarker.comments = calcuMarker.at(i).comments;
        temMarker.category = calcuMarker.at(i).category;
        temMarker.color = calcuMarker.at(i).color;
        temMarker.ave = calcuMarker.at(i).ave;
        temMarker.sdev = calcuMarker.at(i).sdev;
        temMarker.skew = calcuMarker.at(i).skew;
        temMarker.curt = calcuMarker.at(i).curt;
        temMarker.on = calcuMarker.at(i).on;
        MarkerforDisplaying.push_back(temMarker);
    }

    callback.setLandmark(windows, MarkerforDisplaying);
}

LandmarkList mergeLandmark(LandmarkList a, LandmarkList b)
{
    LandmarkList c;
    for(V3DLONG i =0; i < a.size(); i++)
    {
        c.push_back(a.at(i));
    }
    for(V3DLONG i =0; i < b.size(); i++)
    {
        c.push_back(b.at(i));
    }
    return c;
}

vector<LocationSimple> LandmarkList2vectorLocationSimple(LandmarkList a)
{
    vector<LocationSimple> b;
    for(V3DLONG i = 0; i < a.size(); i++)
    {
        b.push_back(a.at(i));
    }
    return b;
}

LandmarkList vectorLocationSimple2LandmarkList(vector<LocationSimple> a)
{
    LandmarkList b;
    for(V3DLONG i = 0; i < a.size(); i++)
    {
        b.push_back(a.at(i));
    }
    return b;
}

LandmarkList calibrate_tipPoints(V3DPluginCallback2 &callback, LandmarkList original_tipPoint, int raidus3D, int bkg_thres)
{
    //define the color
    RGBA8 red, green, blue, yellow, pink, orange, darkOrange, purple;
    red.r=255;          red.g=0;            red.b=0;
    green.r=0;          green.g=255;        green.b=0;
    blue.r=0;           blue.g=0;           blue.b=255;
    yellow.r=255;       yellow.g=255;       yellow.b=0;
    pink.r=255;         pink.g=105;         pink.b=180;
    orange.r=255;       orange.g=165;       orange.b=0;
    darkOrange.r=255;   darkOrange.g=100;   darkOrange.b=0;
    purple.r=255;       purple.g=0;         purple.b=255;

    v3dhandle curwin = callback.currentImageWindow();
    Image4DSimple *p4DImage = callback.getImage(curwin);
    LandmarkList calibrated_tipPoint;
    vector<vector<float> > x_dis(16*32,vector<float>(100)), y_dis(16*32,vector<float>(100)),z_dis(16*32,vector<float>(100));

    for(int len = 0; len < raidus3D; len++)
    {
        int mm = 0;
        for(int n = 1; n <= 16; n++ )
        {
            for(int k = 0; k <=31; k++ )
            {
                x_dis[mm][len] = len * sin(PI * n/16) * cos(PI * k / 16);
                y_dis[mm][len] = len * sin(PI * n/16) * sin(PI * k / 16);
                z_dis[mm][len] = len * cos(PI * n/16);
                mm++;
            }
        }
    }
    cout<<"create 3D_ray success"<<endl;

    for(V3DLONG t = 0; t < original_tipPoint.size(); t++)
    {
        ClusterAnalysis mycluster; mycluster.Init();
        int x_location = floor(original_tipPoint.at(t).x+0.5);
        int y_location = floor(original_tipPoint.at(t).y+0.5);
        int z_location = floor(original_tipPoint.at(t).z+0.5);

        LandmarkList curlist; curlist.clear();
        LocationSimple s;


        vector<float> x_loc; x_loc.clear();
        vector<float> y_loc; y_loc.clear();
        vector<float> z_loc; z_loc.clear();
        vector<float> Pixe; Pixe.clear();
        for(int  i =0;i<16*32;i++)
        {
            for(int j=0; j < raidus3D; j+=1)
            {
                double pixe=p4DImage->getValueUINT8(x_location+x_dis[i][j],y_location+y_dis[i][j],z_location+z_dis[i][j],0);
                x_loc.push_back(x_location+x_dis[i][j]);
                y_loc.push_back(y_location+y_dis[i][j]);
                z_loc.push_back(z_location+z_dis[i][j]);
                // cout<<"the x location is "<<x_location-1+ray_x[i][j]<<" "<<"the y location is "<<y_location-1+ray_y[i][j]<<endl;
                Pixe.push_back(pixe);
            }
        }

         v3d_msg(QString("the x_loc size is %1").arg(x_loc.size()),0);
        for(V3DLONG i = 0; i < x_loc.size(); i++)
        {
            if(Pixe.at(i)<bkg_thres)
            {
            //                 s.color=blue;
            }
            else
            {
                s.x=  x_loc[i];
                s.y = y_loc[i];
                s.z = z_loc[i];
                s.radius = 1;
                s.color = red;
                curlist<<s;
            }
        }

        v3d_msg(QString("curlist.size:%1").arg(curlist.size()),0);

        mycluster.Read_from_curlist(curlist,4,0);
        int flag = mycluster.DoDBSCANRecursive();
//        cout<<"cluster flag:"<<flag<<endl;

        LocationSimple p, target_p;
//        p.x = x_location;
//        p.y = y_location;
//        p.z = z_location;
        target_p = mycluster.find_CenterPoint_of_MaxPointSet(p, curlist);
        calibrated_tipPoint.push_back(target_p);
    }

    return calibrated_tipPoint;
}

bool saveSoma(QString path,LocationSimple SOMA)
{
    path = QFileDialog::getSaveFileName(0,
                                      "Save as marker file",
                                      path,
                                      QObject::tr("marker file (*.marker);;(*.*)"
                                      ));

    if(path.isEmpty())
    {
        cout<<"Saving Path error!"<<endl;
        return false;
    }
    FILE * fp = fopen(path.toLatin1(), "wt");
    if (!fp)
    {
        v3d_msg("Could not open the file to save the marker.");
        return false;
    }
    fprintf(fp, "##x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");


    fprintf(fp, "%5.3f,%5.3f,%5.3f,%5.3f,%d,%s,%s,%d,%d,%d \n",
            SOMA.x,SOMA.y,SOMA.z,SOMA.radius,SOMA.shape, SOMA.name,SOMA.comments,SOMA.color.r,SOMA.color.g,SOMA.color.b);

    fclose(fp);

    return true;
}

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
    QHash<V3DLONG, V3DLONG> neuron_id_table;
    for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
        neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
    return neuron_id_table;
}

void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, double margin)
{
    NeuronSWC *p_cur = 0;
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = sx*sy;
    cout<<"neurons.listNeuron.size():"<<neurons.listNeuron.size()<<endl;

    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        V3DLONG i,j,k;
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
        xs = p_cur->x;
        ys = p_cur->y;
        zs = p_cur->z;
        if(xs<0 || ys<0 || zs<0)
            continue;
        rs = p_cur->r+margin;//margin added by PHC 20170531

        double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

        ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
        ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
        if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

        bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
        bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
        if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

        ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
        ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
        if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

        //mark all voxels close to the swc node(s)
        for (k = ballz0; k <= ballz1; k++){
            for (j = bally0; j <= bally1; j++){
                for (i = ballx0; i <= ballx1; i++){
                    V3DLONG ind = (k)*pagesz + (j)*sx + i;
                    if (pImMask[ind]>0)pImMask[ind] = 0;
                }
            }
        }


        //find previous node
        if (p_cur->pn < 0) continue;//then it is root node already
        //get the parent info
        const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
        xe = pp.x;
        ye = pp.y;
        ze = pp.z;
        re = pp.r;

        //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
        if (xe==xs && ye==ys && ze==zs)
        {
            v3d_msg(QString("Detect overlapping coordinates of node\n"), 0);
            continue;
        }

        double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
        double dx = (xe - xs);
        double dy = (ye - ys);
        double dz = (ze - zs);
        double x = xs;
        double y = ys;
        double z = zs;

        int steps = lroundf(l);
        steps = (steps < fabs(dx))? fabs(dx):steps;
        steps = (steps < fabs(dy))? fabs(dy):steps;
        steps = (steps < fabs(dz))? fabs(dz):steps;
        if (steps<1) steps =1;

        double xIncrement = double(dx) / (steps*2);
        double yIncrement = double(dy) / (steps*2);
        double zIncrement = double(dz) / (steps*2);

        V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
        if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
         pImMask[idex1] = 0;

        for (int i = 0; i <= steps; i++)
        {
            x += xIncrement;
            y += yIncrement;
            z += zIncrement;

            x = ( x > sx )? sx : x;
            y = ( y > sy )? sy : y;
            z = ( z > sz )? sz : z;

            V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
            if (pImMask[idex]>0) continue;
            if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
            pImMask[idex] = 0;
        }

        //finding the envelope of the current line segment

        double rbox = (rs>re) ? rs : re;
        double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
        double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
        double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
        double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
        double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
        double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

        //compute cylinder and flag mask

        for (k=z_down; k<=z_top; k++)
        {
            for (j=y_down; j<=y_top; j++)
            {
                for (i=x_down; i<=x_top; i++)
                {
                    double rr = 0;
                    double countxsi = (xs-i);
                    double countysj = (ys-j);
                    double countzsk = (zs-k);
                    double countxes = (xe-xs);
                    double countyes = (ye-ys);
                    double countzes = (ze-zs);
                    double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
                    double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
                    double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
                    double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
                    double t1 = -dots1021/norms21;
                    if(t1<0) dist = sqrt(norms10);
                    else if(t1>1)
                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
                    //compute rr
                    if (rs==re) rr =rs;
                    else
                    {
                        // compute point of intersection
                        double v1 = xe - xs;
                        double v2 = ye - ys;
                        double v3 = ze - zs;
                        double vpt = v1*v1 + v2*v2 +v3*v3;
                        double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
                        double xc = xs + v1*t;
                        double yc = ys + v2*t;
                        double zc = zs + v3*t;
                        double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
                        double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
                        rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
                    }
                    V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;
                    if (pImMask[ind1]>0) continue;
                    if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
                    if (dist <= rr || dist<=1)
                    {
                        pImMask[ind1] = 0;
                    }
                }
            }
        }

    }

}


/**     using missedBranchPoint's locations to avoid neuron broken lines     **/
void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, double margin, LandmarkList missedBranchPoint)
{
    NeuronSWC *p_cur = 0;
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = sx*sy;
    cout<<"neurons.listNeuron.size():"<<neurons.listNeuron.size()<<endl;

    vector<double>branchPointx, branchPointy, branchPointz;
    vector<int> ind_bP;
    for(V3DLONG aa = 0; aa < 1/*missedBranchPoint.size()*/; aa++)
    {
        cout<<"x:"<<missedBranchPoint.at(aa).x<<"  y:"<<missedBranchPoint.at(aa).y<<"  z:"<<missedBranchPoint.at(aa).z;
        branchPointx.clear(); branchPointy.clear(); branchPointz.clear();
        branchPointx.push_back(missedBranchPoint.at(aa).x-3);
        branchPointx.push_back(missedBranchPoint.at(aa).x-2);
        branchPointx.push_back(missedBranchPoint.at(aa).x-1);
        branchPointx.push_back(missedBranchPoint.at(aa).x);
        branchPointx.push_back(missedBranchPoint.at(aa).x+1);
        branchPointx.push_back(missedBranchPoint.at(aa).x+2);
        branchPointx.push_back(missedBranchPoint.at(aa).x+3);
        branchPointx.push_back(missedBranchPoint.at(aa).x+4);
        branchPointx.push_back(missedBranchPoint.at(aa).x-4);
        branchPointx.push_back(missedBranchPoint.at(aa).x+5);
        branchPointx.push_back(missedBranchPoint.at(aa).x-5);
        branchPointx.push_back(missedBranchPoint.at(aa).x+6);
        branchPointx.push_back(missedBranchPoint.at(aa).x-6);
        branchPointx.push_back(missedBranchPoint.at(aa).x+7);
        branchPointx.push_back(missedBranchPoint.at(aa).x-7);

        branchPointy.push_back(missedBranchPoint.at(aa).y-3);
        branchPointy.push_back(missedBranchPoint.at(aa).y-2);
        branchPointy.push_back(missedBranchPoint.at(aa).y-1);
        branchPointy.push_back(missedBranchPoint.at(aa).y);
        branchPointy.push_back(missedBranchPoint.at(aa).y+1);
        branchPointy.push_back(missedBranchPoint.at(aa).y+2);
        branchPointy.push_back(missedBranchPoint.at(aa).y+3);
        branchPointy.push_back(missedBranchPoint.at(aa).y+4);
        branchPointy.push_back(missedBranchPoint.at(aa).y-4);
        branchPointy.push_back(missedBranchPoint.at(aa).y+5);
        branchPointy.push_back(missedBranchPoint.at(aa).y-5);
        branchPointy.push_back(missedBranchPoint.at(aa).y+6);
        branchPointy.push_back(missedBranchPoint.at(aa).y-6);
        branchPointy.push_back(missedBranchPoint.at(aa).y+7);
        branchPointy.push_back(missedBranchPoint.at(aa).y-7);

        branchPointz.push_back(missedBranchPoint.at(aa).z-3);
        branchPointz.push_back(missedBranchPoint.at(aa).z-2);
        branchPointz.push_back(missedBranchPoint.at(aa).z-1);
        branchPointz.push_back(missedBranchPoint.at(aa).z);
        branchPointz.push_back(missedBranchPoint.at(aa).z+1);
        branchPointz.push_back(missedBranchPoint.at(aa).z+2);
        branchPointz.push_back(missedBranchPoint.at(aa).z+3);
        branchPointz.push_back(missedBranchPoint.at(aa).z+4);
        branchPointz.push_back(missedBranchPoint.at(aa).z-4);
        branchPointz.push_back(missedBranchPoint.at(aa).z+5);
        branchPointz.push_back(missedBranchPoint.at(aa).z-5);
        branchPointz.push_back(missedBranchPoint.at(aa).z+6);
        branchPointz.push_back(missedBranchPoint.at(aa).z-6);
        branchPointz.push_back(missedBranchPoint.at(aa).z+7);
        branchPointz.push_back(missedBranchPoint.at(aa).z-7);

        for(V3DLONG kk=0; kk < branchPointz.size(); kk++)
        {
            for(V3DLONG jj=0; jj < branchPointy.size(); jj++)
            {
                for(V3DLONG ii=0; ii < branchPointx.size(); ii++)
                {   ind_bP.push_back(branchPointz.at(kk)*pagesz + branchPointy.at(jj)*sx + branchPointx.at(ii));
                }
            }
        }
//        for(V3DLONG kk=0; kk < branchPointz.size(); kk++)
//        {ind_bP.push_back(branchPointz.at(kk)*pagesz + branchPointy.at(kk)*sx + branchPointx.at(kk));}
    }
    v3d_msg(QString("ind_bP.size:%1").arg(ind_bP.size()));
    vector<double > img_val;
    for(V3DLONG i = 0; i < ind_bP.size(); i++)
    {
        img_val.push_back(pImMask[ind_bP.at(i)]);
        if(img_val.at(i)>1)cout<<img_val.at(i)<<"  ";
    }
    v3d_msg(QString("img_val.size:%1").arg(img_val.size()));

    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        V3DLONG i,j,k;
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
        xs = p_cur->x;
        ys = p_cur->y;
        zs = p_cur->z;
        if(xs<0 || ys<0 || zs<0)
            continue;
        rs = p_cur->r+margin;//margin added by PHC 20170531

        double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

        ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
        ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
        if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

        bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
        bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
        if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

        ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
        ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
        if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

        //mark all voxels close to the swc node(s)
        for (k = ballz0; k <= ballz1; k++)
        {
            int isbreak=0;
            for (j = bally0; j <= bally1; j++)
            {
                for (i = ballx0; i <= ballx1; i++)
                {
//                    for(V3DLONG cc = 0; cc < ind_bP.size(); cc++)
//                    {
                        V3DLONG ind = (k)*pagesz + (j)*sx + i;
                        if (pImMask[ind]>0)
                        {
                            pImMask[ind] = 0;
                        }
//                    }
                }
            }
//                        if(ind!=ind_bP.at(cc))
//                        {
//                            if (pImMask[ind]>0)
//                            {
//                                pImMask[ind] = 0;
//                            }
//                        }
//                        else
//                        {
//                            isbreak=1; break;
//                        }
//                    }
//                    if(isbreak)continue;
//                }
//                if(isbreak)continue;
//            }
//            if(isbreak)continue;
        }


        //find previous node
        if (p_cur->pn < 0) continue;//then it is root node already
        //get the parent info
        const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
        xe = pp.x;
        ye = pp.y;
        ze = pp.z;
        re = pp.r;

        //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
        if (xe==xs && ye==ys && ze==zs)
        {
            v3d_msg(QString("Detect overlapping coordinates of node\n"), 0);
            continue;
        }

        double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
        double dx = (xe - xs);
        double dy = (ye - ys);
        double dz = (ze - zs);
        double x = xs;
        double y = ys;
        double z = zs;

        int steps = lroundf(l);
        steps = (steps < fabs(dx))? fabs(dx):steps;
        steps = (steps < fabs(dy))? fabs(dy):steps;
        steps = (steps < fabs(dz))? fabs(dz):steps;
        if (steps<1) steps =1;

        double xIncrement = double(dx) / (steps*2);
        double yIncrement = double(dy) / (steps*2);
        double zIncrement = double(dz) / (steps*2);

        V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
        if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
         pImMask[idex1] = 0;

        for (int i = 0; i <= steps; i++)
        {
            x += xIncrement;
            y += yIncrement;
            z += zIncrement;

            x = ( x > sx )? sx : x;
            y = ( y > sy )? sy : y;
            z = ( z > sz )? sz : z;

            V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
            if (pImMask[idex]>0) continue;
            if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
            pImMask[idex] = 0;
        }

        //finding the envelope of the current line segment

        double rbox = (rs>re) ? rs : re;
        double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
        double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
        double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
        double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
        double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
        double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

        //compute cylinder and flag mask

        for (k=z_down; k<=z_top; k++)
        {
            for (j=y_down; j<=y_top; j++)
            {
                for (i=x_down; i<=x_top; i++)
                {
                    double rr = 0;
                    double countxsi = (xs-i);
                    double countysj = (ys-j);
                    double countzsk = (zs-k);
                    double countxes = (xe-xs);
                    double countyes = (ye-ys);
                    double countzes = (ze-zs);
                    double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
                    double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
                    double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
                    double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
                    double t1 = -dots1021/norms21;
                    if(t1<0) dist = sqrt(norms10);
                    else if(t1>1)
                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
                    //compute rr
                    if (rs==re) rr =rs;
                    else
                    {
                        // compute point of intersection
                        double v1 = xe - xs;
                        double v2 = ye - ys;
                        double v3 = ze - zs;
                        double vpt = v1*v1 + v2*v2 +v3*v3;
                        double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
                        double xc = xs + v1*t;
                        double yc = ys + v2*t;
                        double zc = zs + v3*t;
                        double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
                        double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
                        rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
                    }
                    V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;
                    if (pImMask[ind1]>0) continue;
                    if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
                    if (dist <= rr || dist<=1)
                    {
                        pImMask[ind1] = 0;
                    }
                }
            }
        }

    }
    for(V3DLONG i = 0; i < img_val.size(); i++)
    {
//        pImMask[ind_bP.at(i)]=img_val.at(i);
        pImMask[ind_bP.at(i)]=255;

    }

}

void ComputemaskImage(V3DPluginCallback2 &callback, NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, double margin, LandmarkList missedBranchPoint)
{
    NeuronSWC *p_cur = 0;
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = sx*sy;
    cout<<"neurons.listNeuron.size():"<<neurons.listNeuron.size()<<endl;
    LandmarkList toshow;
    vector<double>branchPointx, branchPointy, branchPointz;
    vector<V3DLONG> ind_bP;
    missedBranchPoint[0].x = 3;
    missedBranchPoint[0].y = 3;
    missedBranchPoint[0].z = 3;

    for(V3DLONG aa = 0; aa < 1/*missedBranchPoint.size()*/; aa++)
    {
        cout<<"x:"<<missedBranchPoint.at(aa).x<<"  y:"<<missedBranchPoint.at(aa).y<<"  z:"<<missedBranchPoint.at(aa).z;
        branchPointx.clear(); branchPointy.clear(); branchPointz.clear();
        branchPointx.push_back(missedBranchPoint.at(aa).x-3);
        branchPointx.push_back(missedBranchPoint.at(aa).x-2);
        branchPointx.push_back(missedBranchPoint.at(aa).x-1);
        branchPointx.push_back(missedBranchPoint.at(aa).x);
        branchPointx.push_back(missedBranchPoint.at(aa).x+1);
        branchPointx.push_back(missedBranchPoint.at(aa).x+2);
        branchPointx.push_back(missedBranchPoint.at(aa).x+3);
//        branchPointx.push_back(missedBranchPoint.at(aa).x+4);
//        branchPointx.push_back(missedBranchPoint.at(aa).x-4);
//        branchPointx.push_back(missedBranchPoint.at(aa).x+5);
//        branchPointx.push_back(missedBranchPoint.at(aa).x-5);
//        branchPointx.push_back(missedBranchPoint.at(aa).x+6);
//        branchPointx.push_back(missedBranchPoint.at(aa).x-6);
//        branchPointx.push_back(missedBranchPoint.at(aa).x+7);
//        branchPointx.push_back(missedBranchPoint.at(aa).x-7);

        branchPointy.push_back(missedBranchPoint.at(aa).y-3);
        branchPointy.push_back(missedBranchPoint.at(aa).y-2);
        branchPointy.push_back(missedBranchPoint.at(aa).y-1);
        branchPointy.push_back(missedBranchPoint.at(aa).y);
        branchPointy.push_back(missedBranchPoint.at(aa).y+1);
        branchPointy.push_back(missedBranchPoint.at(aa).y+2);
        branchPointy.push_back(missedBranchPoint.at(aa).y+3);
//        branchPointy.push_back(missedBranchPoint.at(aa).y+4);
//        branchPointy.push_back(missedBranchPoint.at(aa).y-4);
//        branchPointy.push_back(missedBranchPoint.at(aa).y+5);
//        branchPointy.push_back(missedBranchPoint.at(aa).y-5);
//        branchPointy.push_back(missedBranchPoint.at(aa).y+6);
//        branchPointy.push_back(missedBranchPoint.at(aa).y-6);
//        branchPointy.push_back(missedBranchPoint.at(aa).y+7);
//        branchPointy.push_back(missedBranchPoint.at(aa).y-7);

        branchPointz.push_back(missedBranchPoint.at(aa).z-3);
        branchPointz.push_back(missedBranchPoint.at(aa).z-2);
        branchPointz.push_back(missedBranchPoint.at(aa).z-1);
        branchPointz.push_back(missedBranchPoint.at(aa).z);
        branchPointz.push_back(missedBranchPoint.at(aa).z+1);
        branchPointz.push_back(missedBranchPoint.at(aa).z+2);
        branchPointz.push_back(missedBranchPoint.at(aa).z+3);
//        branchPointz.push_back(missedBranchPoint.at(aa).z+4);
//        branchPointz.push_back(missedBranchPoint.at(aa).z-4);
//        branchPointz.push_back(missedBranchPoint.at(aa).z+5);
//        branchPointz.push_back(missedBranchPoint.at(aa).z-5);
//        branchPointz.push_back(missedBranchPoint.at(aa).z+6);
//        branchPointz.push_back(missedBranchPoint.at(aa).z-6);
//        branchPointz.push_back(missedBranchPoint.at(aa).z+7);
//        branchPointz.push_back(missedBranchPoint.at(aa).z-7);

        for(V3DLONG kk=0; kk < branchPointz.size(); kk++)
        {
            for(V3DLONG jj=0; jj < branchPointy.size(); jj++)
            {
                for(V3DLONG ii=0; ii < branchPointx.size(); ii++)
                {   ind_bP.push_back(branchPointz.at(kk)*pagesz + branchPointy.at(jj)*sx + branchPointx.at(ii));
                    LocationSimple p;
                    p.x=branchPointx.at(ii);
                    p.y=branchPointy.at(jj);
                    p.z=branchPointz.at(kk);
                    v3d_msg(QString("%1").arg(p.z*sx*sy+p.y*sx+p.x),0);
                    pImMask[V3DLONG(p.z*sx*sy+p.y*sx+p.x)]=255;
                    toshow.append(p);
                }
            }
        }
//        for(V3DLONG kk=0; kk < branchPointz.size(); kk++)
//        {ind_bP.push_back(branchPointz.at(kk)*pagesz + branchPointy.at(kk)*sx + branchPointx.at(kk));}
    }


    callback.setLandmark(callback.currentImageWindow(),toshow);
//    v3d_msg(QString("ind_bP.size:%1").arg(ind_bP.size()));
//    vector<double > img_val;
//    for(V3DLONG i = 0; i < ind_bP.size(); i++)
//    {
//        img_val.push_back(pImMask[ind_bP.at(i)]);
//        if(img_val.at(i)>1)cout<<img_val.at(i)<<"  ";
//    }
//    v3d_msg(QString("img_val.size:%1").arg(img_val.size()));

//    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
//    {
//        V3DLONG i,j,k;
//        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
//        xs = p_cur->x;
//        ys = p_cur->y;
//        zs = p_cur->z;
//        if(xs<0 || ys<0 || zs<0)
//            continue;
//        rs = p_cur->r+margin;//margin added by PHC 20170531

//        double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

//        ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
//        ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
//        if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

//        bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
//        bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
//        if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

//        ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
//        ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
//        if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

//        //mark all voxels close to the swc node(s)
//        for (k = ballz0; k <= ballz1; k++)
//        {
//            int isbreak=0;
//            for (j = bally0; j <= bally1; j++)
//            {
//                for (i = ballx0; i <= ballx1; i++)
//                {
////                    for(V3DLONG cc = 0; cc < ind_bP.size(); cc++)
////                    {
//                        V3DLONG ind = (k)*pagesz + (j)*sx + i;
//                        if (pImMask[ind]>0)
//                        {
//                            pImMask[ind] = 0;
//                        }
////                    }
//                }
//            }
////                        if(ind!=ind_bP.at(cc))
////                        {
////                            if (pImMask[ind]>0)
////                            {
////                                pImMask[ind] = 0;
////                            }
////                        }
////                        else
////                        {
////                            isbreak=1; break;
////                        }
////                    }
////                    if(isbreak)continue;
////                }
////                if(isbreak)continue;
////            }
////            if(isbreak)continue;
//        }


//        //find previous node
//        if (p_cur->pn < 0) continue;//then it is root node already
//        //get the parent info
//        const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
//        xe = pp.x;
//        ye = pp.y;
//        ze = pp.z;
//        re = pp.r;

//        //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
//        if (xe==xs && ye==ys && ze==zs)
//        {
//            v3d_msg(QString("Detect overlapping coordinates of node\n"), 0);
//            continue;
//        }

//        double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));
//        double dx = (xe - xs);
//        double dy = (ye - ys);
//        double dz = (ze - zs);
//        double x = xs;
//        double y = ys;
//        double z = zs;

//        int steps = lroundf(l);
//        steps = (steps < fabs(dx))? fabs(dx):steps;
//        steps = (steps < fabs(dy))? fabs(dy):steps;
//        steps = (steps < fabs(dz))? fabs(dz):steps;
//        if (steps<1) steps =1;

//        double xIncrement = double(dx) / (steps*2);
//        double yIncrement = double(dy) / (steps*2);
//        double zIncrement = double(dz) / (steps*2);

//        V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
//        if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
//         pImMask[idex1] = 0;

//        for (int i = 0; i <= steps; i++)
//        {
//            x += xIncrement;
//            y += yIncrement;
//            z += zIncrement;

//            x = ( x > sx )? sx : x;
//            y = ( y > sy )? sy : y;
//            z = ( z > sz )? sz : z;

//            V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
//            if (pImMask[idex]>0) continue;
//            if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
//            pImMask[idex] = 0;
//        }

//        //finding the envelope of the current line segment

//        double rbox = (rs>re) ? rs : re;
//        double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
//        double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
//        double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
//        double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
//        double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
//        double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

//        //compute cylinder and flag mask

//        for (k=z_down; k<=z_top; k++)
//        {
//            for (j=y_down; j<=y_top; j++)
//            {
//                for (i=x_down; i<=x_top; i++)
//                {
//                    double rr = 0;
//                    double countxsi = (xs-i);
//                    double countysj = (ys-j);
//                    double countzsk = (zs-k);
//                    double countxes = (xe-xs);
//                    double countyes = (ye-ys);
//                    double countzes = (ze-zs);
//                    double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
//                    double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
//                    double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
//                    double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
//                    double t1 = -dots1021/norms21;
//                    if(t1<0) dist = sqrt(norms10);
//                    else if(t1>1)
//                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
//                    //compute rr
//                    if (rs==re) rr =rs;
//                    else
//                    {
//                        // compute point of intersection
//                        double v1 = xe - xs;
//                        double v2 = ye - ys;
//                        double v3 = ze - zs;
//                        double vpt = v1*v1 + v2*v2 +v3*v3;
//                        double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
//                        double xc = xs + v1*t;
//                        double yc = ys + v2*t;
//                        double zc = zs + v3*t;
//                        double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
//                        double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
//                        rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
//                    }
//                    V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;
//                    if (pImMask[ind1]>0) continue;
//                    if (lroundf(z)>(sz-1)||lroundf(y)>(sy-1)||lroundf(x)>(sx-1)) continue;
//                    if (dist <= rr || dist<=1)
//                    {
//                        pImMask[ind1] = 0;
//                    }
//                }
//            }
//        }

//    }
//    for(V3DLONG i = 0; i < ind_bP.size(); i++)
//    {
////        pImMask[ind_bP.at(i)]=img_val.at(i);
//        pImMask[ind_bP.at(i)]=255;

//    }

}



void Dilation(unsigned char* image, unsigned char* res, int nHeight, int nWidth)
{
    int thresh=10;
    for (int i = 1; i < nHeight - 1; i++)
    {
        for (int j = 1; j < nWidth - 1; j++)
        {
            int pos = i*nWidth + j;
            int up = pos - nWidth;
            int upleft = pos - nWidth - 1;
            int upright = pos - nWidth + 1;
            int left = pos - 1;
            int right = pos + 1;
            int downleft = pos + nWidth - 1;
            int down = pos + nWidth;
            int downright = pos + nWidth + 1;

            (image[pos]>thresh || image[up]>thresh || image[upleft]>thresh || image[upright]>thresh || image[left]>thresh || image[right]>thresh || image[downleft]>thresh || image[down]>thresh || image[downright]>thresh) ? res[pos] = 255 : res[pos] = 0;
        }
    }
    for (int i = 0; i < 1; i++)
    {
        for (int j = 0; j < nWidth; j++)
        {
            int tmppos = i*nWidth + j;
            res[tmppos] = image[tmppos];
        }
    }

    for (int i = 1; i < nHeight - 1; i++)
    {

        int tmppos1 = i*nWidth;
        res[tmppos1] = image[tmppos1];

        int tmppos2 = (i + 1)*nWidth - 1;
        res[tmppos2] = image[tmppos2];
    }

    for (int i = nHeight - 1; i < nHeight; i++)
    {
        for (int j = 0; j < nWidth; j++)
        {
            int tmppos = i*nWidth + j;
            res[tmppos] = image[tmppos];
        }
    }
}

void Erosion(unsigned char* image, unsigned char* res, int nHeight, int nWidth)
{
    int thresh = 10;
    for (int i = 1; i < nHeight - 1; i++)
    {
        for (int j = 1; j < nWidth - 1; j++)
        {
            int pos = i*nWidth + j;
            int up = pos - nWidth;
            int upleft = pos - nWidth - 1;
            int upright = pos - nWidth + 1;
            int left = pos - 1;
            int right = pos + 1;
            int downleft = pos + nWidth - 1;
            int down = pos + nWidth;
            int downright = pos + nWidth + 1;

            (image[pos]>thresh && image[up]>thresh && image[upleft]>thresh && image[upright]>thresh && image[left]>thresh && image[right]>thresh && image[downleft]>thresh && image[down]>thresh && image[downright]>thresh) ? res[pos] = 255 : res[pos] = 0;
        }
    }
    for (int i = 0; i < 1; i++)
    {
        for (int j = 0; j < nWidth; j++)
        {
            int tmppos = i*nWidth + j;
            res[tmppos] = image[tmppos];
        }
    }

    for (int i = 1; i < nHeight - 1; i++)
    {

        int tmppos1 = i*nWidth;
        res[tmppos1] = image[tmppos1];

        int tmppos2 = (i + 1)*nWidth - 1;
        res[tmppos2] = image[tmppos2];
    }

    for (int i = nHeight - 1; i < nHeight; i++)
    {
        for (int j = 0; j < nWidth; j++)
        {
            int tmppos = i*nWidth + j;
            res[tmppos] = image[tmppos];
        }
    }
}

void Closeoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth)
{
    unsigned char* transition = new unsigned char[nHeight*nWidth];
    Dilation(image, transition, nHeight, nWidth);
    Erosion(transition, res, nHeight, nWidth);
    delete[]transition;
}

void Openoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth)
{
    unsigned char* transition = new unsigned char[nHeight*nWidth];
    Erosion(image, transition, nHeight, nWidth);
    Dilation(transition, res, nHeight, nWidth);
    delete[]transition;
}

void Z_mip(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip)
{
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < nz; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                //    v3d_msg(QString("max_mip is %1").arg(max_mip));
                }
            }
        }
    }

}

void gaussian_filter(unsigned char* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     double sigma,
                     float* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || outimg)
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }

     // for filter kernel
     double sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
     double pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)

     float min_val = INF, max_val = 0;

     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;

     //filtering
     V3DLONG offset_init = (c-1)*pagesz;

     //declare temporary pointer
     float *pImage = new float [pagesz];
     if (!pImage)
     {
          printf("Fail to allocate memory.\n");
          return;
     }
     else
     {
          for(V3DLONG i=0; i<pagesz; i++)
               pImage[i] = data1d[i + offset_init];  //first channel data (red in V3D, green in ImageJ)
     }
       //Filtering
     //
     //   Filtering along x
     if(N<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsX = 0;
          WeightsX = new float [Wx];
          if (!WeightsX)
               return;

          float Half = (float)(Wx-1)/2.0;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
       //   for (unsigned int Weight = 0; Weight < Half; ++Weight)
       //   {
       //        const float  x = Half* float (Weight) / float (Half);
      //         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
      //    }

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  x = float(Weight)-Half;
              WeightsX[Weight] = WeightsX[Wx-Weight-1] = pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               k += WeightsX[Weight];

          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               WeightsX[Weight] /= k;

         printf("\n x dierction");

         for (unsigned int Weight = 0; Weight < Wx; ++Weight)
             printf("/n%f",WeightsX[Weight]);

          //   Allocate 1-D extension array
          float  *extension_bufferX = 0;
          extension_bufferX = new float [N + (Wx<<1)];

          unsigned int offset = Wx>>1;

          //	along x
          const float  *extStop = extension_bufferX + N + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG iy = 0; iy < M; iy++)
               {
                    float  *extIter = extension_bufferX + Wx;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferX - 1;
                    float  *extLeft = extension_bufferX + Wx - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + N + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);

                    }

                    //	Filtering
                    extIter = extension_bufferX + offset;

                    float  *resIter = &(pImage[iz*M*N + iy*N]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsX;
                         const float  *const End = WeightsX + Wx;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *(resIter++) = sum;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;


                    }

               }
          }
          //de-alloc
           if (WeightsX) {delete []WeightsX; WeightsX=0;}
           if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}

     }

     //   Filtering along y
     if(M<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsY = 0;
          WeightsY = new float [Wy];
          if (!WeightsY)
               return;

          float Half = (float)(Wy-1)/2.0;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
         /* for (unsigned int Weight = 0; Weight < Half; ++Weight)
          {
               const float  y = Half* float (Weight) / float (Half);
               WeightsY[(int)Half - Weight] = WeightsY[(int)Half + Weight] = pi_sigma * exp(-y * y *sigma_s2); // Corresponding symmetric WeightsY
          }*/

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  y = float(Weight)-Half;
              WeightsY[Weight] = WeightsY[Wy-Weight-1] = pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               k += WeightsY[Weight];

          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               WeightsY[Weight] /= k;

          //	along y
          float  *extension_bufferY = 0;
          extension_bufferY = new float [M + (Wy<<1)];

          unsigned int offset = Wy>>1;
          const float *extStop = extension_bufferY + M + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {
                    float  *extIter = extension_bufferY + Wy;
                    for(V3DLONG iy = 0; iy < M; iy++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferY - 1;
                    float  *extLeft = extension_bufferY + Wy - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + M + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferY + offset;

                    float  *resIter = &(pImage[iz*M*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsY;
                         const float  *const End = WeightsY + Wy;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;


                    }

               }
          }

          //de-alloc
          if (WeightsY) {delete []WeightsY; WeightsY=0;}
          if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}


     }

     //  Filtering  along z
     if(P<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsZ = 0;
          WeightsZ = new float [Wz];
          if (!WeightsZ)
               return;

          float Half = (float)(Wz-1)/2.0;

         /* for (unsigned int Weight = 1; Weight < Half; ++Weight)
          {
               const float  z = Half * float (Weight) / Half;
               WeightsZ[(int)Half - Weight] = WeightsZ[(int)Half + Weight] = pi_sigma * exp(-z * z * sigma_s2) ; // Corresponding symmetric WeightsZ
          }*/

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  z = float(Weight)-Half;
              WeightsZ[Weight] = WeightsZ[Wz-Weight-1] = pi_sigma * exp(-(z * z *sigma_s2)); // Corresponding symmetric WeightsZ
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               k += WeightsZ[Weight];

          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               WeightsZ[Weight] /= k;

          //	along z
          float  *extension_bufferZ = 0;
          extension_bufferZ = new float [P + (Wz<<1)];

          unsigned int offset = Wz>>1;
          const float *extStop = extension_bufferZ + P + offset;

          for(V3DLONG iy = 0; iy < M; iy++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {

                    float  *extIter = extension_bufferZ + Wz;
                    for(V3DLONG iz = 0; iz < P; iz++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferZ - 1;
                    float  *extLeft = extension_bufferZ + Wz - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + P + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferZ + offset;

                    float  *resIter = &(pImage[iy*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsZ;
                         const float  *const End = WeightsZ + Wz;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += M*N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;

                    }

               }
          }

          //de-alloc
          if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
          if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}


     }

    outimg = pImage;


    return;
}

double dist(MyMarker a, LocationSimple b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

double dist(LocationSimple a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

double dist(LocationSimple a, LocationSimple b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

double dist(NeuronSWC a, LocationSimple b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

void nt_processing_for_mask(NeuronTree nt, LandmarkList pointSet, NeuronTree &nt_new, double margin)
{
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    NeuronSWC S;
    V3DLONG neuronNum = nt.listNeuron.size();
    V3DLONG *flag = new V3DLONG[neuronNum];
    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        flag[i]=0;
        for(V3DLONG j=0;j<pointSet.size();j++)
        {  
            if(dist(curr,pointSet[j])<margin)//(abs(curr.x-pointSet[j].x)+abs(curr.y-pointSet[j].y)+abs(curr.z-pointSet[j].z))<5)
            {
                flag[i]=1;
            }
        }
    }
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        if(flag[i]!=1)
        {
            S.n 	= curr.n;
            S.type 	= curr.type;
            S.x 	= curr.x;
            S.y 	= curr.y;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    nt_new.n = -1;
    nt_new.on = true;
    nt_new.listNeuron = listNeuron;
    nt_new.hashNeuron = hashNeuron;
}

bool saveSWC_file_app2(string swc_file, vector<MyMarker*> & outmarkers, list<string> & infostring)
{
    if(swc_file.find_last_of(".dot") == swc_file.size() - 1) return saveDot_file(swc_file, outmarkers);

    cout<<"marker num = "<<outmarkers.size()<<", save swc file to "<<swc_file<<endl;
    map<MyMarker*, int> ind;
    ofstream ofs(swc_file.c_str());

    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }
    ofs<<"#name "<<"APP2_Tracing"<<endl;
    ofs<<"#comment "<<endl;

    list<string>::iterator it;
    for (it=infostring.begin();it!=infostring.end(); it++)
        ofs<< *it <<endl;

    ofs<<"##n,type,x,y,z,radius,parent"<<endl;
    for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

    for(int i = 0; i < outmarkers.size(); i++)
    {
        MyMarker * marker = outmarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[marker->parent];
        ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
    }
    ofs.close();
    return true;
}

//int merge_neurons_real(NeuronTree nt1, NeuronTree nt2, NeuronTree nt_merged, float thresh)
//{
//    nt_merged.deepCopy(nt1);
//    NeuronSWC tmp;
//    V3DLONG neuronNum1 = nt1.listNeuron.size();
//    V3DLONG neuronNum2 = nt2.listNeuron.size();
//    V3DLONG *flag = new V3DLONG[neuronNum2];
//    V3DLONG *pn = new V3DLONG[neuronNum2];
//    V3DLONG *m_ind = new V3DLONG[neuronNum1];
//    QList<NeuronSWC> list1 = nt1.listNeuron;
//    QList<NeuronSWC> list2 = nt2.listNeuron;

//    for (V3DLONG i=0; i<neuronNum1; i++)
//    {
//        NeuronSWC curr1 = list1.at(i);
//        for(V3DLONG j=0; j<neuronNum2; j++)
//        {
//            NeuronSWC curr2 = list2.at(j);
//            flag[j] = 0;
//            if(dist(curr1, curr2) < thresh)
//            {
//                flag[j]=1; pn[j]=curr1.n;
//                m_ind[i]=i;
//            }
//        }
//    }

//    for (V3DLONG t=0; t<neuronNum2; t++)
//    {
//        NeuronSWC curr = list2.at(t);
//        if(flag[t]==0)
//        {
////            if(flag[curr.pn]==1)
////            {
////                tmp.pn 	= -1;
////            }
////            else
////            {
////                tmp.pn 	= nt_merged.n
////            }
//            tmp.n   = nt_merged.n+1;
//            tmp.type    = curr.type;
//            tmp.x 	= curr.x;
//            tmp.y 	= curr.y;
//            tmp.z 	= curr.z;
//            tmp.r 	= curr.r;
//            nt_merged.listNeuron.append(tmp);
//            nt_merged.hashNeuron.insert(tmp.n, nt_merged.listNeuron.size()-1);
//        }
//    }

//    return 1;
//}


QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1||method_code==3)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}

#endif

//
