#include "v3d_message.h"
#include "branch_tip_detection.h"
#include <algorithm>
#include <vector>

//#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
    double xx = s1.x-s2.x;
    double yy = s1.y-s2.y;
    double zz = s1.z-s2.z;
    return (xx*xx+yy*yy+zz*zz);
}

bool branch_tip_detection(V3DPluginCallback2 &callback,QList<NeuronSWC> & result,QString fileOpenName,QWidget *parent)
{
    NeuronTree nt;
    nt.listNeuron=result;
    vector<vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );

    nt.hashNeuron.clear();
    for(V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
       nt.hashNeuron.insert(nt.listNeuron[i].n, i);
       //cout<<"i="<<i<<" "<<"nt.hashNeuron="<<nt.hashNeuron.size()<<" "<<"nt.listNeuron[i].n="<<nt.listNeuron[i].n<<" "<<"nt.listneuron[i].pn="<<nt.listNeuron[i].pn<<endl;
    }

    //getChildNum
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(nt.listNeuron[i].pn)].push_back(i);
        //cout<<"i="<<i<<"nt.hashNeuron.value(nt.listNeuron[i].pn)="<<nt.hashNeuron.value(nt.listNeuron[i].pn)<<endl;
    }

    QList<NeuronSWC> branchpoints,tips;
    int child_num;

    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC cur = nt.listNeuron[i];
        child_num = childs[i].size();   //need check
        //child_num = childs[nt.listNeuron[i].n].size();
        if(child_num==2)
            branchpoints.push_back(cur);
        else if(child_num==0)
            tips.push_back(cur);
    }

    //method1:Remove nearer branch and tip points------------node numbers
//    for(V3DLONG i=0;i<tips.size();i++)
//    {
//        NeuronSWC temp_tip =tips[i];
//        bool flag = false;
//        for(int n=0;n<5;n++)
//        {
//            for(V3DLONG k=0;k<branchpoints.size();k++)
//            {
//                if(temp_tip.pn==branchpoints[k].n)
//                   {
//                        tips.removeAt(i);
//                        branchpoints.removeAt(k);
//                        flag = true;
//                        break;
//                   }
//            }
//            if(flag)
//                break;
//            temp_tip=nt.listNeuron[temp_tip.pn];//  find his father
//        }

//    }

    //method2:Remove nearer branch and tip points----------distance
    bool ok;
    V3DLONG thres;
    thres = QInputDialog::getDouble(0, "Would you like to set a threshold for remove nearer branch and tip points?","threshold:(If you select 'cancel', the thres is 5)",0,0,2147483647,1,&ok);
    if (!ok)
        thres = 5;

    for(V3DLONG i=0;i<branchpoints.size();i++)
    {
        for(V3DLONG j=0;j<tips.size();j++)
        {
            if(NTDIS(branchpoints[i],tips[j])<thres)
            {
                tips.removeAt(j);
                branchpoints.removeAt(i);
            }
        }
    }


    cout<<"branchpoints="<<branchpoints.size()<<endl;
    cout<<"tips="<<tips.size()<<endl;


//    //method2
//    QList<NeuronSWC> branchpoints,tips;
//    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
//    {
//        int n =0;
//        for(V3DLONG j=0;j<nt.listNeuron.size();j++)
//        {
//            if(nt.listNeuron[i].n==nt.listNeuron[j].pn)
//                n++;
//        }
//        if(n==0)
//            tips.push_back(nt.listNeuron[i]);
//        else if(n>=2)
//            branchpoints.push_back(nt.listNeuron[i]);
//    }


      //create .marker file
//    QList<ImageMarker> branch_markerlist,tip_markerlist;
//    QString filename1 =fileOpenName + "_branch_markerlist.marker";
//    QString filename2=fileOpenName + "_tip_markerlist.marker";
//    for(V3DLONG i=0;i<branchpoints.size();i++)
//    {
//        ImageMarker m;
//        m.x = branchpoints[i].x;
//        m.y = branchpoints[i].y;
//        m.z = branchpoints[i].z;
//        //m.radius = branchpoints[i].radius;
//        m.radius=0.001;
//        m.color.a = 0;
//        m.color.b = 0;
//        m.color.g = 0;
//        m.type = branchpoints[i].type;
//        branch_markerlist.push_back(m);
//    }
//    writeMarker_file(filename1,branch_markerlist);

//    for(V3DLONG i=0;i<tips.size();i++)
//    {
//        ImageMarker m;
//        m.x = tips[i].x;
//        m.y = tips[i].y;
//        m.z = tips[i].z;
//        //m.radius = tips[i].radius;
//        m.radius=0.001;
//        m.color.a = 0;
//        m.color.b = 255;
//        m.color.g = 0;
//        m.type = tips[i].type;
//        tip_markerlist.push_back(m);
//    }
//    writeMarker_file(filename2,tip_markerlist);

    //create .apo file and .ano file
    unsigned int Vsize=50;
//    Vsize = QInputDialog::getInteger(parent, "Volume size ",
//                                  "Enter volume size:",
//                                  50, 1, 1000, 1);


    //create .apo and .ano file
    QList<CellAPO> branchpoint_inmarkers,tip_inmarkers;
    for(V3DLONG i = 0; i <branchpoints.size();i++)
    {
        CellAPO t;
        t.x = branchpoints.at(i).x+1;
        t.y = branchpoints.at(i).y+1;
        t.z = branchpoints.at(i).z+1;
        t.color.r=255;
        t.color.g=0;
        t.color.b=0;
        t.volsize = Vsize;
        branchpoint_inmarkers.push_back(t);
    }

    QString apo_name1 = fileOpenName + "_branchpiont.apo";
    writeAPO_file(apo_name1,branchpoint_inmarkers);

    for(V3DLONG i = 0; i <tips.size();i++)
    {
        CellAPO t;
        t.x = tips.at(i).x+1;
        t.y = tips.at(i).y+1;
        t.z = tips.at(i).z+1;
        t.color.r=0;
        t.color.g=0;
        t.color.b=255;

        t.volsize = Vsize;
        tip_inmarkers.push_back(t);
    }

    QString apo_name2 = fileOpenName + "_tip.apo";
    writeAPO_file(apo_name2,tip_inmarkers);
    QString linker_name = fileOpenName + ".ano";
    QFile qf_anofile(linker_name);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {   bool flag = false;
        v3d_msg("Cannot open file for writing!");
        return flag;
    }

    QTextStream out(&qf_anofile);
    out << "SWCFILE=" << QFileInfo(fileOpenName).fileName()<<endl;
    out << "APOFILE=" << QFileInfo(apo_name1).fileName()<<endl;
    out << "APOFILE=" << QFileInfo(apo_name2).fileName()<<endl;
    v3d_msg(QString("Save the linker file to: %1 Complete!").arg(linker_name));

    return true;
}

QList<NeuronSWC> SortSWC(QList<NeuronSWC> & neuron,V3DLONG newrootid, double thres, double root_dist_thres, QList<CellAPO> markers,QString fileOpenName)
{

    //get all root ids and parents of each node
    QList<NeuronSWC> result;
    QList<V3DLONG> rootlist;
    vector<long> parents0;
    vector<long> parents;
    vector<long> ids0;
    vector<long> ids;
    QList<V3DLONG> tips;
    //QList<NeuronSWC> neuron = neurons;

    //Remove duplicated nodes
    //get ids and reorder tree with ids following list

    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids0.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn !=-1)
        {
            neuron[i].pn=find(ids0.begin(), ids0.end(),neuron.at(i).pn) - ids0.begin()+1;
            parents0.push_back(neuron.at(i).pn);
        }
    }
    QList<V3DLONG>  child_num0;
    for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
    {
        child_num0.push_back(count(parents0.begin(),parents0.end(),neuron.at(i).n));
    }

    vector<bool> dupnodes;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        dupnodes.push_back(false);
    }
    for(V3DLONG h=0;h<neuron.size();h++)
    {
        for(V3DLONG i=h+1;i<neuron.size();i++)
        {
            if(neuron.at(h).x == neuron.at(i).x && neuron.at(h).y == neuron.at(i).y && neuron.at(h).z == neuron.at(i).z)
            {
                if(neuron.at(i).pn != -1)
                {
                    if(child_num0.at(i)!=0)
                    {
                        //find nodes that pointed to i and make them point to its parent node
                        for(V3DLONG j=0;j<neuron.size();j++)
                        {
                            if(neuron.at(j).pn==neuron.at(i).n)
                            {
                                neuron[j].pn = neuron.at(i).pn;
                            }
                        }
                    }
                    dupnodes[i]=true;
                }
                else
                {
                    dupnodes[h]=true;
                    dupnodes[i]=false;
                    if(child_num0.at(h)!=0)
                    {
                        //find nodes that pointed to i-1 and make them point to its parent node
                        for(V3DLONG j=0;j<neuron.size();j++)
                        {
                            if(neuron.at(j).pn==neuron.at(h).n)
                            {
                                neuron[j].pn = neuron.at(h).pn;
                            }
                        }
                    }
                }
            }
            else
            {
                //dupnodes.push_back(false);
                dupnodes[i]=false;
            }
        }
    }
    QList<NeuronSWC> neuron2;
    neuron2.append(neuron.at(0));
    for(V3DLONG i=1;i<neuron.size();i++)
    {
        if(dupnodes.at(i) == false) neuron2.append(neuron.at(i));
        //else qDebug()<<i;
    }
    neuron = neuron2;
    qDebug()<<"Removed" << count(dupnodes.begin(),dupnodes.end(),true) << " duplicated nodes";


    //get ids and reorder tree with ids following list
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
        if(neuron.at(i).n==newrootid)
        {
            newrootid=i;
        }
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn ==-1)
        {
            rootlist.push_back(i);//neuron.at(i).n-1);
        }
        else
        {
            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
            parents.push_back(neuron.at(i).pn);
        }
    }

    //find branches
    QList<V3DLONG>  child_num;
    QList<V3DLONG>  tiplist;
    for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
    {
        child_num.push_back(count(parents.begin(),parents.end(),neuron.at(i).n));
        if(child_num.at(i)==0 && neuron.at(i).pn!=-1)
        {
            tiplist.push_back(i);
        }
    }

    qDebug()<<"Root, tip ids and parents found";

    //check if marker exists and use it as root
    if(markers.size()>0)
    {
        if(markers.size()==1)
        {
            NeuronSWC S;
            //markers.at(0).operator XYZ;
            S.x = markers.at(0).x;
            S.y = markers.at(0).y;
            S.z = markers.at(0).z;
            S.r = markers.at(0).volsize;
            S.n = neuron.size();
            S.pn = -1;
            neuron.append(S);

            qDebug()<< "Marker found. Using it as root."; //<< neuron.size()<< neuron.at(neuron.size()-1).pn;
        }
        else{
            int count=0;
            for(int i=0;i<markers.size();i++)
            {
                if(count==0 && markers.at(i).comment == "soma")//markers.at(i).color.r == 0 && markers.at(i).color.g == 0 && markers.at(i).color.b == 255)
                {
                    NeuronSWC S;
                    //markers.at(0).operator XYZ;
                    S.x = markers.at(i).x;
                    S.y = markers.at(i).y;
                    S.z = markers.at(i).z;
                    S.r = markers.at(i).volsize;
                    S.n = neuron.size();
                    S.pn = -1;
                    neuron.append(S);
                    count++;
                }
            }
            qDebug()<< "Warning: more than one marker in the file, taking one commented as 'soma'";// << neuron.size();
        }
        newrootid = neuron.size()-1;
        rootlist.push_back(newrootid);
        //tiplist.push_back(newrootid);
        parents.push_back(-1);

    }else if(newrootid!=VOID) //check if new root id is valid and take first root in the list otherwise
    {
        //newrootid = newrootid;
        if(neuron.at(newrootid).pn==-1)
        {
            qDebug()<<"New root id is:" << ids.at(newrootid);
        }
    }
    else
    {
        newrootid = rootlist.at(0);
        qDebug()<<"Selected node is not a root, taking first root node:" << newrootid+1;
    }

    //assign segids
    int segid = 0;
    int rootsegid = VOID;
    V3DLONG walknode = 0;
    vector<int> visited_walknodes;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        visited_walknodes.push_back(0);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(child_num.at(i) ==0)
        {
            vector<int> segment;
            walknode = i;
            //neuron[walknode].seg_id = segid;
            while(neuron.at(walknode).pn!=-1)
            {
                neuron[walknode].seg_id = segid;
                segment.push_back(walknode);
                visited_walknodes[walknode] = 1;
                walknode = neuron.at(walknode).pn-1;//find(ids.begin(), ids.end(),neuron.at(walknode).pn) - ids.begin();
            }
            if(visited_walknodes.at(walknode)==0)
            {
                visited_walknodes[walknode] = 1;
                neuron[walknode].seg_id = segid;
                //qDebug()<<walknode<<segid;
                if(walknode == newrootid)
                {
                    rootsegid=segid;
                }
                segid++;
            }
            else
            {
                for(int j=0;j<segment.size();j++)
                {
                    neuron[segment.at(j)].seg_id =  neuron.at(walknode).seg_id;
                }
                qDebug() << "Visited segid:" << neuron.at(walknode).seg_id;
            }
        }
    }

    //connect all local branches directly to soma
    int counter = 0;
    for(V3DLONG ii=0;ii<segid;ii++)
    {
        //double dist2 = MAX_DOUBLE;
        double distroot = MAX_DOUBLE;
        double disttip = MAX_DOUBLE;
        int localroot = VOID;
        int localtip = VOID;
        //vector<int> localtips;
        for(V3DLONG i=0;i<neuron.size();i++)
        {
            if(neuron.at(i).seg_id==ii && ii!=rootsegid && neuron.at(i).pn==-1)
            {
                localroot = i;
                distroot = computeDist2(neuron.at(i),neuron.at(newrootid));
            }
            if(neuron.at(i).seg_id==ii && ii!=rootsegid && child_num.at(i)==0 && neuron.at(i).pn!=-1)
            {
                //localtips.push_back(i);
                if(computeDist2(neuron.at(i),neuron.at(newrootid))<disttip)
                {
                    localtip = i;
                    disttip = computeDist2(neuron.at(i),neuron.at(newrootid));
                }
            }
        }
        if(distroot<disttip && distroot<=root_dist_thres)
        {
            neuron[localroot].pn = newrootid+1;
            counter++;
            qDebug() << "Segment" << neuron.at(localroot).seg_id << "root connected to soma.";
        }
        else if(disttip<distroot && disttip<=root_dist_thres)
        {
            //neuron[localtip].pn
            int walknode1 = newrootid;
            int maxit = 0;
            int walknode2 = neuron.at(localtip).n-1;
            int lastnode = walknode2;
            int walknode3 = neuron.at(walknode2).pn-1;
            if(neuron.at(walknode3).pn==-1)
            {
                neuron[walknode2].pn = walknode1+1;
                neuron[walknode3].pn = walknode2+1;
                counter++;
                qDebug() << "Segment" << neuron.at(lastnode).seg_id << "tip connected to soma.";
            }
            else while(neuron.at(walknode3).pn!=-1 && maxit<neuron.size())
            {
                walknode1 = walknode2;
                walknode2 = walknode3;
                walknode3 = neuron.at(walknode2).pn-1;
                neuron[walknode2].pn = walknode1+1;
                if(maxit == 0)
                {
                    neuron[lastnode].pn = newrootid+1;
                    counter++;
                    qDebug() << "Segment" << neuron.at(lastnode).seg_id << "tip connected to soma.";
                }
                //qDebug()<<walknode1+1<<walknode2+1<<walknode3+1<<neuron.at(walknode2).pn;
                maxit++;
            }
            if(maxit == neuron.size())
            {
                qDebug() << "Careful, while broke"; // << walknode1+1 << walknode2+1 << walknode3+1 <<  neuron.at(tiplist.at(i)).n <<  neuron.at(tiplist.at(i)).pn << newrootid + 1;
            }
            if(neuron.at(walknode3).pn==-1 && walknode3 != newrootid)
            {
                neuron[walknode3].pn = walknode2+1;
            }
        }
    }


    if(counter==0 && markers.size()>0)
    {
        tiplist.push_back(newrootid);
    }
    qDebug()<<counter<<"local branches connected";

    //re-obtain roots and tips
    QList<V3DLONG> rootlist2;
    vector<long> parents2;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn ==-1)
        {
            rootlist2.push_back(i);//neuron.at(i).n-1);
        }
        else
        {
            parents2.push_back(neuron.at(i).pn);
        }
    }
    rootlist = rootlist2;
    parents = parents2;

    //find branches
    QList<V3DLONG>  child_num2;
    QList<V3DLONG>  tiplist2;
    for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
    {
        child_num2.push_back(count(parents2.begin(),parents2.end(),neuron.at(i).n));
        if(child_num2.at(i)==0 )
        {
            tiplist2.push_back(i);
        }
    }
    child_num = child_num2;
    tiplist = tiplist2;

    //re-assign segids
    vector<int> rootsegids;
    vector<int> visited_walknodes2;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        visited_walknodes2.push_back(0);
    }
    visited_walknodes = visited_walknodes2;
    segid = 0;
    walknode = 0;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(child_num.at(i) ==0)
        {
            vector<int> segment;
            walknode = i;
            ///neuron[walknode].seg_id = segid;
            //segment.push_back(walknode);
            //visited_walknodes[walknode] = 1;
            while(neuron.at(walknode).pn!=-1)
            {
                neuron[walknode].seg_id = segid;
                segment.push_back(walknode);
                visited_walknodes[walknode] = 1;
                walknode = neuron.at(walknode).pn-1;//find(ids.begin(), ids.end(),neuron.at(walknode).pn) - ids.begin();
                //qDebug()<<walknode<<segid;
            }
            //qDebug()<<walknode<<newrootid<<visited_walknodes.at(walknode);
            if(visited_walknodes.at(walknode)==0)
            {
                visited_walknodes[walknode] = 1;
                neuron[walknode].seg_id = segid;
                /*if(neuron.at(walknode).n == newrootid+1)
                {
                    rootsegids.push_back(segid);
                    //qDebug()<<segid;
                }*/
                segid++;
            }
            else
            {
                for(int j=0;j<segment.size();j++)
                {
                    neuron[segment.at(j)].seg_id =  neuron.at(walknode).seg_id;
                }
                //qDebug() << "Visited root segid:" << neuron.at(walknode).seg_id;
            }
        }
    }
    //qDebug()<<neuron.at(0).seg_id<<neuron.at(newrootid).seg_id;
    qDebug()<<"segids assigned";
    int root_segid = neuron.at(newrootid).seg_id;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(neuron.at(i).seg_id==0)
        {
            neuron[i].seg_id = root_segid;
        }
        else if(neuron.at(i).seg_id==root_segid)
        {
            neuron[i].seg_id = 0;
        }
    }
    //qDebug()<<neuron.at(0).seg_id<<neuron.at(1).seg_id<<neuron.at(newrootid).seg_id<<newrootid<<root_segid;
    qDebug()<<"segids swapped";
    //change other root segments
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(count(rootsegids.begin(),rootsegids.end(),neuron.at(i).seg_id)!=0)
        {
            neuron[i].seg_id = 0;
        }
    }
    qDebug()<<"Root segid assigned to 0";

    //connect remaining segments to closest neuron nodes
    double min = VOID;
    int pid = VOID;
    int cid = VOID;
    QList<V3DLONG>  segidtips;
    if(thres==VOID)
    {
        thres=MAX_DOUBLE;
    }
    //qDebug()<<neuron.at(newrootid).seg_id;

    //get tips of each segment
    for(V3DLONG ii=0;ii<=segid;ii++)
    {
        for(V3DLONG i=0;i<neuron.size();i++)
        {
            //get tips
            if(child_num.at(i)==0)
            {
                segidtips.push_back(i);
            }
        }
    }
    qDebug()<<"tips found";

    for(V3DLONG ii=1;ii<segid;ii++)
    {
        //qDebug()<<ii<<count(rootsegids.begin(),rootsegids.end(),ii);
        if(count(rootsegids.begin(),rootsegids.end(),ii)==0)
        {
            double dist2 = MAX_DOUBLE;
            min=VOID;
            //int pidchild = VOID;
            cout << "\r" <<"Connecting segments: "<< ii << " /" << segid;
            for(V3DLONG i=0;i<neuron.size();i++)
            {
                if(neuron.at(i).seg_id==ii && neuron.at(i).seg_id!=neuron.at(newrootid).seg_id)//only one segment that must be different from newroot's
                {
                    for(V3DLONG j=0;j<neuron.size();j++)
                    {
                        if(neuron.at(i).seg_id!=neuron.at(j).seg_id)// && segidparents.at(neuron.at(j).seg_id)!=neuron.at(i).seg_id)//last condition to avoid loops in segments
                        {
                            dist2 = computeDist2(neuron.at(i),neuron.at(j)); // check 0 1
                            if(dist2 < min && dist2 <= thres*thres)// && connectednodes.at(j)==0)
                            {
                                min = dist2;
                                cid = i;
                                pid = j;
                            }
                            //qDebug()<<cid+1<<pid+1<<neuron.at(i).pn<<neuron.at(j).pn;
                        }
                    }
                }
                //qDebug()<<i<<neuron.at(i).seg_id;
            }
            //qDebug()<<"Distances obtained";
            //qDebug()<<cid+1<<pid+1<<neuron.at(cid).pn<<neuron.at(pid).pn;
            if(cid != newrootid)//child node cannot be newroot
            {
                if(neuron.at(cid).pn==-1)// && neuron[pid].pn=!-1)//child node is root
                {
                    neuron[cid].pn = neuron.at(pid).n;
                    //qDebug()<<"Child node is a root";
                }
                else //if(child_num.at(cid)==0) //the child is a tip or a lonely root, if a tip the branch has to be reordered
                {
                    int walknode1 = pid;
                    int maxit = 0;
                    int walknode2 = cid;
                    ptrdiff_t walknode3 = neuron.at(walknode2).pn-1;//find(ids.begin(), ids.end(),neuron.at(walknode2).pn) - ids.begin();
                    //neuron[walknode2].pn = walknode1+1;
                    //qDebug()<<walknode1+1<<walknode2+1<<walknode3+1;
                    while(neuron.at(walknode3).pn!=-1 && neuron.at(walknode3).seg_id==neuron.at(cid).seg_id && maxit<neuron.size())// && neuron.at(walknode3).seg_id==segid) //is last condition necessary?
                    {
                        walknode1 = walknode2;
                        walknode2 = walknode3;
                        walknode3 = neuron.at(walknode2).pn-1;//find(ids.begin(), ids.end(), neuron.at(walknode2).pn) - ids.begin();
                        //walknode3 = neuron.at(walknode2).pn-1;
                        neuron[walknode2].pn = walknode1+1;//neuron.at(walknode1).n;
                        //qDebug()<<walknode1+1<<walknode2+1<<walknode3+1;
                        maxit++;
                    }
                    if(maxit==neuron.size())
                    {
                        qDebug()<<"Careful, while broke" << ii << count(rootsegids.begin(),rootsegids.end(),ii) << cid << pid << walknode1 << walknode2 << walknode3;
                    }
                    if(walknode3!=newrootid)// && neuron.at(walknode3).seg_id==segid)//is this condition necessary?
                    {
                        neuron[walknode3].pn = walknode2+1;//neuron.at(walknode2).n;
                    }
                    neuron[cid].pn = pid+1;//neuron.at(pid).n;
                    if(ii==segid)
                    {
                        qDebug() << cid+1 << pid+1;
                    }

                }
                int parentsegid = neuron.at(pid).seg_id;
                for(V3DLONG i=0;i<neuron.size();i++) //update subtree segid to avoid loops
                {
                    if(neuron.at(i).seg_id==ii)
                    {
                        neuron[i].seg_id=parentsegid;
                    }
                }
                //qDebug()<<parentsegid;
            }
        }
    }
    qDebug()<< "\n" << "\n" << "\n" << "Connected all roots below threshold";

    //sort tree
    int rootnum = 0;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(neuron.at(i).pn==-1) rootnum++;
    }
    if(rootnum!=1)
    {
        qDebug()<<rootnum;
        if(rootnum>1)
        {v3d_msg("Error, more than 1 root!");}
        if(rootnum==0)
        {v3d_msg("Error, 0 roots!");}
        //return(false);
    }
    else
    {
        //QList<V3DLONG> nodestoremove;
        for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
        {
            child_num[i] = count(parents.begin(),parents.end(),neuron.at(i).n);
        }
        //get path lengths and topological distances
        //walk down from each node to root and get topological path distance
        QList<V3DLONG> pl;
        QList<V3DLONG> lo;
        for(V3DLONG i=0;i<neuron.size();i++) // 0 or 1? check!
        {
            ptrdiff_t walknode = i;
            pl.push_back(0);
            lo.push_back(0);
            int maxit=0;
            //qDebug()<<"it's the while";
            while(walknode!=newrootid && maxit<neuron.size())
            {
                walknode = neuron.at(walknode).pn-1;//find(ids.begin(), ids.end(), neuron.at(walknode).pn) - ids.begin();
                //walknode = neuron.at(walknode).pn-1;
                pl[i]++;
                //cout << "it's the child number " << walknode << " " << newrootid << " " << neuron.at(walknode).seg_id << "\n";
                if(child_num.at(walknode)>1)
                {
                    //lo[i]+=child_num.at(walknode);
                    lo[i]++;
                }
                maxit++;
            }
            if(maxit==neuron.size())
            {
                neuron[walknode].pn = -1;
                qDebug()<<"Careful, while broke"<<neuron.at(walknode).n;
            }
        }
        qDebug()<< "Sorting..." <<"Path and topological lengths obtained";

        pl[newrootid] = 0;
        lo[newrootid] = 0;
        //sort nodes by path length
        //QList<NeuronSWC> unsort_neuron = neuron;
        for (V3DLONG i = 0; i<neuron.size(); i++)
        {
           for (V3DLONG j = 1; j < neuron.size() - i; j++)
           {
              if (pl.at(j - 1) > pl.at(j))// && j!=newrootid)
              {
                 swap(pl[j - 1], pl[j]);
                 swap(lo[j - 1], lo[j]);
                 swap(neuron[j - 1].n, neuron[j].n);
                 swap(neuron[j - 1].pn, neuron[j].pn);
                 swap(neuron[j - 1].x, neuron[j].x);
                 swap(neuron[j - 1].y, neuron[j].y);
                 swap(neuron[j - 1].z, neuron[j].z);
                 swap(neuron[j - 1].r, neuron[j].r);
                 swap(neuron[j - 1].type, neuron[j].type);
              }
           }
        }
        qDebug()<<"Sorting..." << "Sorted by topological path length";
        // and level order
        for (V3DLONG i = 0; i<neuron.size(); i++)
        {
           for (V3DLONG j = 1; j < neuron.size() - i; j++)
           {
              if (lo.at(j - 1) > lo.at(j))// && j!=newrootid)
              {
                 swap(pl[j - 1], pl[j]);
                 swap(lo[j - 1], lo[j]);
                 swap(neuron[j - 1].n, neuron[j].n);
                 swap(neuron[j - 1].pn, neuron[j].pn);
                 swap(neuron[j - 1].x, neuron[j].x);
                 swap(neuron[j - 1].y, neuron[j].y);
                 swap(neuron[j - 1].z, neuron[j].z);
                 swap(neuron[j - 1].r, neuron[j].r);
                 swap(neuron[j - 1].type, neuron[j].type);
              }
           }
        }
        qDebug()<<"Sorting..." << "Sorted by level order";
        //adjust parent nodes accordingly
        //create ordered oldids vector
        vector<int> ordoldids;
        for (V3DLONG j = 0; j<neuron.size(); j++)
        {
            ordoldids.push_back(neuron.at(j).n);
        }
        neuron[0].n=1;
        neuron[0].pn=-1;
        int newpn;
        for (V3DLONG j = 1; j<=neuron.size(); j++)
        {
            ptrdiff_t newpn = find(ordoldids.begin(), ordoldids.end(), neuron.at(j-1).pn) - ordoldids.begin()+1;
            neuron[j-1].pn=newpn;
            neuron[j-1].n=j;
        }
        neuron[0].pn=-1;
        qDebug()<<"Sorted swc";

        //construct neuron
        for (V3DLONG i = 0; i<neuron.size(); i++)
        {
            result.append(neuron.at(i));
        }
    }

    return result;
}

