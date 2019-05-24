#include "branch_angle.h"

#define VOID 1000000000
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

bool branch_angle1(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));
    NeuronTree nt=readSWC_file(fileOpenName);
    cout<<fileOpenName.toStdString()<<endl;
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;


    vector<vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );

//    nt.hashNeuron.clear();
//    for(V3DLONG i=1; i<nt.listNeuron.size();i++)
//    {
//       nt.hashNeuron.insert(nt.listNeuron[i].n, i);
//    }

    //getChildNum
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    //get branchpoints and wrongpoints
    int childNum;
    QList<NeuronSWC> branch_swclist,wrong_swclist;
    NeuronTree branch_ntlist,wrong_ntlist;
    QList<ImageMarker> branch_markerlist,wrong_markerlist;
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        //childNum = childs[nt.listNeuron[i].pn].size();
        childNum = childs[i].size();
        if(childNum == 2)
            branch_swclist.push_back(nt.listNeuron[i]);
        else if(childNum >= 3)
            wrong_swclist.push_back(nt.listNeuron[i]);
    }
    cout<<"branch_swclist = "<<branch_swclist.size()<<endl;
    cout<<"wrong_swclist = "<<wrong_swclist.size()<<endl;

    branch_markerlist = swc_to_marker(branch_swclist);
    branch_ntlist.listNeuron = branch_swclist;
    wrong_markerlist = swc_to_marker(wrong_swclist);
    wrong_ntlist.listNeuron = wrong_swclist;

    writeSWC_file(fileOpenName+QString("_branchpoints.swc"),branch_ntlist);
    writeMarker_file(fileOpenName+QString("_branchpoints.marker"),branch_markerlist);
    writeSWC_file(fileOpenName+QString("_wrongpoints.swc"),wrong_ntlist);
    writeMarker_file(fileOpenName+QString("_wrongpoints.marker"),wrong_markerlist);

    //method:remove by distance and angle
    //Calculation branchpoints distance and build branchponts relation
    QHash<int,vector<int> > relate_branch;
    double dist_branch,dist_thres = 10;
    QList<NeuronSWC> remove_swclist;
    NeuronTree remove_ntlist;
    for(int i = 0;i < branch_swclist.size();i++)
    {
        for(int j = i+1;j < branch_swclist.size();j++)
        {
            dist_branch = NTDIS(branch_swclist[i],branch_swclist[j]);
            if(dist_branch <= dist_thres)
                relate_branch[branch_swclist[i].n].push_back(branch_swclist[j].n);
        }
        cout<<"relate_branch "<<branch_swclist[i].n<<" :";
        for(vector<int>::iterator it = relate_branch[branch_swclist[i].n].begin();it != relate_branch[branch_swclist[i].n].end();++it)
            cout<< *it<<" ";
        cout<<endl;

        //choose branchchilds
        QList<NeuronSWC> choose_swclist1,choose_swclist2,branchChildtoolittle_swclist;
        NeuronTree choose_ntlist1,choose_ntlist2,branchChildtoolittle_ntlist;
        QList<ImageMarker> choose_markerlist1,choose_markerlist2;
        NeuronSWC child1,child2;
        for(vector<int>::iterator it = relate_branch[branch_swclist[i].n].begin();it != relate_branch[branch_swclist[i].n].end();++it)
        {
            if(*it == 0)  continue;
            //cout<<branch_swclist[i].n<<endl;
            //cout<<relate_branch[branch_swclist[i].n][0]<<endl;
            //cout<<relate_branch[branch_swclist[i].n].size()<<endl;
            for(int k = 0;k < relate_branch[branch_swclist[i].n].size();k++)
            {
                choose_swclist1.clear();
                choose_markerlist1.clear();
                choose_swclist2.clear();
                choose_markerlist2.clear();
                cout<<relate_branch[branch_swclist[i].n][k]-1<<endl;
                //cout<<childs[relate_branch[branch_swclist[i].n][k]-1].size()<<endl;
                V3DLONG par1 = childs[relate_branch[branch_swclist[i].n][k]-1][0];
                V3DLONG par2 = childs[relate_branch[branch_swclist[i].n][k]-1][1];
                cout<<"par1 = "<<par1<<"      par2 = "<<par2<<endl;
                //v3d_msg("check");
                child1 = nt.listNeuron[par1];
                child2 = nt.listNeuron[par2];
                //choose_swclist1.push_back(child1);
                //choose_swclist2.push_back(child2);

                for(int n =0;n < 5;n++)
                {
                    if(childs[child1.n-1].size()==0 || childs[child1.n-1].size()>=2)
                    {
                        branchChildtoolittle_swclist.push_back(nt.listNeuron[branch_swclist[i].n]);
                        break;
                    }
                    if(childs[child2.n-1].size()==0 || childs[child2.n-1].size()>=2)
                        break;
                    V3DLONG par1 = childs[child1.n-1][0];
                    //cout<<childs[child1.n-1].size()<<endl;
                    V3DLONG par2 = childs[child2.n-1][0];
                    //cout<<"par1 = "<<par1<<"    par2 = "<<par2<<endl;
                    child1 = nt.listNeuron[par1];
                    child2 = nt.listNeuron[par2];
                    choose_swclist1.push_back(child1);
                    choose_swclist2.push_back(child2);
               }

                cout<<"branchpoint "<<relate_branch[branch_swclist[i].n][k]<<": choose_swclist1 = "<<choose_swclist1.size()<<endl;
                cout<<"branchpoint "<<relate_branch[branch_swclist[i].n][k]<<": choose_swclist2 = "<<choose_swclist2.size()<<endl;

                choose_ntlist1.listNeuron = choose_swclist1;
                choose_ntlist2.listNeuron = choose_swclist2;
                choose_markerlist1 = swc_to_marker(choose_swclist1);
                choose_markerlist2 = swc_to_marker(choose_swclist2);
                writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(relate_branch[branch_swclist[i].n][k])+QString("_choosepoints1.swc"),choose_ntlist1);
                writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(relate_branch[branch_swclist[i].n][k])+QString("_choosepoints2.swc"),choose_ntlist2);
                writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(relate_branch[branch_swclist[i].n][k])+QString("_choosepoints1.marker"),choose_markerlist1);
                writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(relate_branch[branch_swclist[i].n][k])+QString("_choosepoints2.marker"),choose_markerlist2);

                branchChildtoolittle_ntlist.listNeuron = branchChildtoolittle_swclist;
                if(branchChildtoolittle_swclist.size()>0)
                {
                    writeSWC_file(fileOpenName + "_branchChildtoolittle.swc",branchChildtoolittle_ntlist);
                }

                //fitline
                double a,b,c,angle;
                cv::Vec6f line_para1,line_para2;
                if(choose_swclist1.size() < 3 || choose_swclist2.size() < 3 )  continue;
                getFitLine(choose_swclist1,choose_swclist2,line_para1,line_para2);

                //method1:calculate angle
//                a = ((line_para1[0])*(line_para2[0])+(line_para1[1])*(line_para2[1])+(line_para1[2])*(line_para2[2]));
//                b = sqrt((line_para1[0])*(line_para1[0])+(line_para1[1])*(line_para1[1])+(line_para1[2])*(line_para1[2]));
//                c = sqrt((line_para2[0])*(line_para2[0])+(line_para2[1])*(line_para2[1])+(line_para2[2])*(line_para2[2]));
//                cout<<"a = "<<a<<"  b = "<<b<<"  c = "<<c<<endl;
//                angle = a/(b*c);

                //method2:calculate angle
                PointCoordinate ProjPoint1,ProjPoint2,ProjPoint3,ProjPoint4;
                GetProjPoint(choose_swclist1,line_para1,ProjPoint1,ProjPoint2);
                GetProjPoint(choose_swclist2,line_para2,ProjPoint3,ProjPoint4);

                a = ((ProjPoint2.x - ProjPoint1.x)*(ProjPoint4.x - ProjPoint3.x)+(ProjPoint2.y - ProjPoint1.y)*(ProjPoint4.y - ProjPoint3.y)+(ProjPoint2.z - ProjPoint1.z)*(ProjPoint4.z - ProjPoint3.z));
                b= sqrt((ProjPoint2.x - ProjPoint1.x)*(ProjPoint2.x - ProjPoint1.x) + (ProjPoint2.y - ProjPoint1.y)*(ProjPoint2.y - ProjPoint1.y) + (ProjPoint2.z - ProjPoint1.z)*(ProjPoint2.z - ProjPoint1.z));
                c= sqrt((ProjPoint4.x - ProjPoint3.x)*(ProjPoint4.x - ProjPoint3.x) + (ProjPoint4.y - ProjPoint3.y)*(ProjPoint4.y - ProjPoint3.y) + (ProjPoint4.z - ProjPoint3.z)*(ProjPoint4.z - ProjPoint3.z));
                cout<<"a = "<<a<<"   b = "<<b<<"    c = "<<c<<endl;
                angle = a/(b*c);
                cout<<"angle = "<<angle<<endl;

                QList<NeuronSWC> chooseChild_swclist1,chooseChild_swclist2,chooseParent_swclist;
                NeuronTree chooseParent_ntlist;
                if(angle < -0.8)
                { 
                    V3DLONG par = relate_branch[branch_swclist[i].n][k]-1;

                    //remove parents
                    chooseParent_swclist.clear();
                    getAllParent(nt,childs,par,chooseParent_swclist,VOID);
                    for(int l = chooseParent_swclist.size()-1;l>=0;l--)
                    {
                        remove_swclist.push_back(chooseParent_swclist[l]);
                    }
                    //remove_swclist.append(chooseParent_swclist);
                    cout<<"chooseParent_swclist = "<<chooseParent_swclist.size()<<endl;
                    chooseParent_ntlist.listNeuron = chooseParent_swclist;
                    writeSWC_file(fileOpenName + QString("_chooseParent.swc"),chooseParent_ntlist);

                    //remove all childs
                    chooseChild_swclist1.clear();
                    chooseChild_swclist2.clear();
                    getAllChild(nt,childs,childs[par][0],chooseChild_swclist1,VOID);
                    getAllChild(nt,childs,childs[par][1],chooseChild_swclist2,VOID);
                    remove_swclist.append(chooseChild_swclist1);
                    remove_swclist.append(chooseChild_swclist2);
                }
                else
                    continue;
            }
        }
    }

    cout<<"remove_swclist = "<<remove_swclist.size()<<endl;
    remove_ntlist.listNeuron = remove_swclist;
    writeSWC_file(fileOpenName + QString("_remove.swc"),remove_ntlist);
    for(int p = 0;p < nt.listNeuron.size();p++)
    {
        for(int q = 0;q < remove_swclist.size();q++)
        {
            if((nt.listNeuron[p].x == remove_swclist[q].x)&&(nt.listNeuron[p].y == remove_swclist[q].y)&&(nt.listNeuron[p].z == remove_swclist[q].z))
                nt.listNeuron.removeAt(p);
        }
    }
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;
    writeSWC_file(fileOpenName + QString("_result.swc"),nt);

    return true;

}


bool calculate_subtree(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));
    NeuronTree nt=readSWC_file(fileOpenName);
    cout<<fileOpenName.toStdString()<<endl;
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;


    vector<vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );

    //getChildNum
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    //get branchpoints and wrongpoints
    int childNum;
    QList<NeuronSWC> branch_swclist,wrong_swclist;
    NeuronTree branch_ntlist,wrong_ntlist;
    QList<ImageMarker> branch_markerlist,wrong_markerlist;
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        //childNum = childs[nt.listNeuron[i].pn].size();
        childNum = childs[i].size();
        if(childNum == 2)
            branch_swclist.push_back(nt.listNeuron[i]);
        else if(childNum >= 3)
            wrong_swclist.push_back(nt.listNeuron[i]);
    }
    cout<<"branch_swclist = "<<branch_swclist.size()<<endl;
    cout<<"wrong_swclist = "<<wrong_swclist.size()<<endl;

    branch_markerlist = swc_to_marker(branch_swclist);
    branch_ntlist.listNeuron = branch_swclist;
    wrong_markerlist = swc_to_marker(wrong_swclist);
    wrong_ntlist.listNeuron = wrong_swclist;

    writeSWC_file(fileOpenName+QString("_branchpoints.swc"),branch_ntlist);
    writeMarker_file(fileOpenName+QString("_branchpoints.marker"),branch_markerlist);
    writeSWC_file(fileOpenName+QString("_wrongpoints.swc"),wrong_ntlist);
    writeMarker_file(fileOpenName+QString("_wrongpoints.marker"),wrong_markerlist);

    //calculate subtree angle
    QList<NeuronSWC> choose_swclist1,choose_swclist2,branchChildtoolittle_swclist,remove_swclist,angle_branch_swclist;
    NeuronTree choose_ntlist1,choose_ntlist2,branchChildtoolittle_ntlist,remove_ntlist,angle_branch_ntlist;
    QList<ImageMarker> choose_markerlist1,choose_markerlist2,angle_branch_markerlist;
    NeuronSWC child1,child2;
    for(int i = 0;i < branch_swclist.size();i++)
    {
        choose_swclist1.clear();
        choose_markerlist1.clear();
        choose_swclist2.clear();
        choose_markerlist2.clear();
        V3DLONG par11 = childs[branch_swclist[i].n-1][0];
        V3DLONG par21 = childs[branch_swclist[i].n-1][1];
        //cout<<"par11 = "<<par11<<"      par21 = "<<par21<<endl;
        child1 = nt.listNeuron[par11];
        child2 = nt.listNeuron[par21];
        //choose_swclist1.push_back(child1);
        //choose_swclist2.push_back(child2);

        for(int n =0;n < 5;n++)
        {
            if(childs[child1.n-1].size()==0 || childs[child1.n-1].size()>=2)
            {
                branchChildtoolittle_swclist.push_back(nt.listNeuron[branch_swclist[i].n]);
                break;
            }
            if(childs[child2.n-1].size()==0 || childs[child2.n-1].size()>=2)
                break;
            V3DLONG par1 = childs[child1.n-1][0];
            //cout<<childs[child1.n-1].size()<<endl;
            V3DLONG par2 = childs[child2.n-1][0];
            //cout<<"par1 = "<<par1<<"    par2 = "<<par2<<endl;
            child1 = nt.listNeuron[par1];
            child2 = nt.listNeuron[par2];
            choose_swclist1.push_back(child1);
            choose_swclist2.push_back(child2);
       }
        cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist1 = "<<choose_swclist1.size()<<endl;
        cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist2 = "<<choose_swclist2.size()<<endl;

        choose_ntlist1.listNeuron = choose_swclist1;
        choose_ntlist2.listNeuron = choose_swclist2;
        choose_markerlist1 = swc_to_marker(choose_swclist1);
        choose_markerlist2 = swc_to_marker(choose_swclist2);
        writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.swc"),choose_ntlist1);
        writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.swc"),choose_ntlist2);
        writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.marker"),choose_markerlist1);
        writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.marker"),choose_markerlist2);

        branchChildtoolittle_ntlist.listNeuron = branchChildtoolittle_swclist;
        if(branchChildtoolittle_swclist.size()>0)
        {
            writeSWC_file(fileOpenName + "_branchChildtoolittle.swc",branchChildtoolittle_ntlist);
        }

        //fitline
        double a,b,c,angle;
        cv::Vec6f line_para1,line_para2;
        if(choose_swclist1.size() < 3 || choose_swclist2.size() < 3 )  break;
        getFitLine(choose_swclist1,choose_swclist2,line_para1,line_para2);

        //calculate angle
        PointCoordinate ProjPoint1,ProjPoint2,ProjPoint3,ProjPoint4;
        GetProjPoint(choose_swclist1,line_para1,ProjPoint1,ProjPoint2);
        GetProjPoint(choose_swclist2,line_para2,ProjPoint3,ProjPoint4);

        a = ((ProjPoint2.x - ProjPoint1.x)*(ProjPoint4.x - ProjPoint3.x)+(ProjPoint2.y - ProjPoint1.y)*(ProjPoint4.y - ProjPoint3.y)+(ProjPoint2.z - ProjPoint1.z)*(ProjPoint4.z - ProjPoint3.z));
        b= sqrt((ProjPoint2.x - ProjPoint1.x)*(ProjPoint2.x - ProjPoint1.x) + (ProjPoint2.y - ProjPoint1.y)*(ProjPoint2.y - ProjPoint1.y) + (ProjPoint2.z - ProjPoint1.z)*(ProjPoint2.z - ProjPoint1.z));
        c= sqrt((ProjPoint4.x - ProjPoint3.x)*(ProjPoint4.x - ProjPoint3.x) + (ProjPoint4.y - ProjPoint3.y)*(ProjPoint4.y - ProjPoint3.y) + (ProjPoint4.z - ProjPoint3.z)*(ProjPoint4.z - ProjPoint3.z));
        cout<<"a = "<<a<<"   b = "<<b<<"    c = "<<c<<endl;
        angle = a/(b*c);
        cout<<"angle = "<<angle<<endl;

    }

    return true;
}

bool branch_angle2(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));
    NeuronTree nt=readSWC_file(fileOpenName);
    cout<<fileOpenName.toStdString()<<endl;
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;

    vector<vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );

    //getChildNum
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    //get branchpoints and wrongpoints
    int childNum;
    QList<NeuronSWC> branch_swclist,wrong_swclist;
    NeuronTree branch_ntlist,wrong_ntlist;
    QList<ImageMarker> branch_markerlist,wrong_markerlist;
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        //childNum = childs[nt.listNeuron[i].pn].size();
        childNum = childs[i].size();
        if(childNum == 2)
            branch_swclist.push_back(nt.listNeuron[i]);
        else if(childNum >= 3)
            wrong_swclist.push_back(nt.listNeuron[i]);
    }
    cout<<"branch_swclist = "<<branch_swclist.size()<<endl;
    cout<<"wrong_swclist = "<<wrong_swclist.size()<<endl;
//    for(int i = 0;i < childs.size();i++)
//    {
//        cout<<i<<" = ";
//        for(int j = 0;j < childs[i].size();j++)
//            cout<<childs[i][j]<<"  ";
//        cout<<endl;
//    }
    //v3d_msg("check!");

    branch_markerlist = swc_to_marker(branch_swclist);
    branch_ntlist.listNeuron = branch_swclist;
    wrong_markerlist = swc_to_marker(wrong_swclist);
    wrong_ntlist.listNeuron = wrong_swclist;

//    writeSWC_file(fileOpenName+QString("_branchpoints.swc"),branch_ntlist);
//    writeMarker_file(fileOpenName+QString("_branchpoints.marker"),branch_markerlist);
//    writeSWC_file(fileOpenName+QString("_wrongpoints.swc"),wrong_ntlist);
//    writeMarker_file(fileOpenName+QString("_wrongpoints.marker"),wrong_markerlist);

    QString filegold = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));
    NeuronTree nt_gold=readSWC_file(filegold);
    cout<<filegold.toStdString()<<endl;
    cout<<"nt_gold.listNeuron = "<<nt_gold.listNeuron.size()<<endl;

    //method:remove by parentpoints and angle
    QList<NeuronSWC> choose_swclist1,choose_swclist2,choose_swclist3,choose_swclist4,remove_swclist,angle_branch_swclist;
    NeuronTree choose_ntlist1,choose_ntlist2,choose_ntlist3,choose_ntlist4,remove_ntlist,angle_branch_ntlist;
    QList<ImageMarker> choose_markerlist1,choose_markerlist2,choose_markerlist3,choose_markerlist4,angle_branch_markerlist;
    double dist_branch,dist_thres = 8;
    int count =0;
    for(int i = 0;i < branch_swclist.size();i++)
    {
        V3DLONG par = branch_swclist[i].n-1;
        for(int n = 0;n < 10;n++)
        {
            V3DLONG par1 = getParent(par,nt);
            //cout<<"par1 = "<<par1<<endl;
             par = par1;
            if(nt.listNeuron[par1].pn == -1)
                break;    
            //cout<<"childs[par1].size() = "<<childs[par1].size()<<endl;
            if(childs[par1].size() == 2)
            {
                dist_branch = NTDIS(nt.listNeuron[par1],nt.listNeuron[branch_swclist[i].n-1]);
                if(dist_branch <= dist_thres)
                {
                    cout<<"nearby branch :"<<nt.listNeuron[par1].n<<"   "<<nt.listNeuron[branch_swclist[i].n-1].n<<"   dist_branch = "<<dist_branch<<endl;
                    //child and child
                    choose_swclist1.clear();
                    choose_markerlist1.clear();
                    choose_swclist2.clear();
                    choose_markerlist2.clear();
                    V3DLONG par11 = childs[branch_swclist[i].n-1][0];
                    V3DLONG par21 = childs[branch_swclist[i].n-1][1];
                    //cout<<"par11 = "<<par11<<"      par21 = "<<par21<<endl;

                    choosePoint(nt,childs,par11,par21,choose_swclist1,choose_swclist2,5);
                    cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist1 = "<<choose_swclist1.size()<<endl;
                    cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist2 = "<<choose_swclist2.size()<<endl;
                    choose_ntlist1.listNeuron = choose_swclist1;
                    choose_ntlist2.listNeuron = choose_swclist2;
                    choose_markerlist1 = swc_to_marker(choose_swclist1);
                    choose_markerlist2 = swc_to_marker(choose_swclist2);
                    //writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.swc"),choose_ntlist1);
                    //writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.swc"),choose_ntlist2);
                    //writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.marker"),choose_markerlist1);
                    //writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.marker"),choose_markerlist2);

                    //fitline
                    double a1,b1,c1,angle1;
                    cv::Vec6f line_para1,line_para2;
                    if(choose_swclist1.size() < 3 || choose_swclist2.size() < 3 )
                        break;
                    getFitLine(choose_swclist1,choose_swclist2,line_para1,line_para2);

                    //calculate angle
                    PointCoordinate ProjPoint1,ProjPoint2,ProjPoint3,ProjPoint4;
                    GetProjPoint(choose_swclist1,line_para1,ProjPoint1,ProjPoint2);
                    GetProjPoint(choose_swclist2,line_para2,ProjPoint3,ProjPoint4);

                    a1 = ((ProjPoint2.x - ProjPoint1.x)*(ProjPoint4.x - ProjPoint3.x)+(ProjPoint2.y - ProjPoint1.y)*(ProjPoint4.y - ProjPoint3.y)+(ProjPoint2.z - ProjPoint1.z)*(ProjPoint4.z - ProjPoint3.z));
                    b1= sqrt((ProjPoint2.x - ProjPoint1.x)*(ProjPoint2.x - ProjPoint1.x) + (ProjPoint2.y - ProjPoint1.y)*(ProjPoint2.y - ProjPoint1.y) + (ProjPoint2.z - ProjPoint1.z)*(ProjPoint2.z - ProjPoint1.z));
                    c1= sqrt((ProjPoint4.x - ProjPoint3.x)*(ProjPoint4.x - ProjPoint3.x) + (ProjPoint4.y - ProjPoint3.y)*(ProjPoint4.y - ProjPoint3.y) + (ProjPoint4.z - ProjPoint3.z)*(ProjPoint4.z - ProjPoint3.z));
                    cout<<"a1 = "<<a1<<"   b1 = "<<b1<<"    c1 = "<<c1<<endl;
                    angle1 = a1/(b1*c1);
                    cout<<"angle1 = "<<angle1<<endl;

                    //child and parent
                    cout<<"childs[par1].size() = "<<childs[par1].size()<<endl;
                    choose_swclist3.clear();
                    choose_swclist4.clear();
                    cout<<"par1 = "<<par1<<endl;
                    if(par1 <= 4)  break;
                    getAllParent(nt,childs,par1-1,choose_swclist3,5);

                    V3DLONG par_child1 = childs[par1][0];
                    V3DLONG par_child2 = childs[par1][1];
                    V3DLONG par_child;
                    cout<<"par_child1 = "<<par_child1<<"   par_child2 = "<<par_child2<<endl;
                    par_child = chooseonechildline(par_child1,par_child2,branch_swclist[i].n,nt,childs);
                    cout<<"par_child = "<<par_child<<endl;
                    choosePoint2(nt,childs,par_child,choose_swclist4,5);

                    cout<<"branchpoint "<<nt.listNeuron[par1].n<<": choose_swclist3 = "<<choose_swclist3.size()<<endl;
                    cout<<"branchpoint "<<nt.listNeuron[par1].n<<": choose_swclist4 = "<<choose_swclist4.size()<<endl;

                    choose_ntlist3.listNeuron = choose_swclist3;
                    choose_ntlist4.listNeuron = choose_swclist4;
                    choose_markerlist3 = swc_to_marker(choose_swclist3);
                    choose_markerlist4 = swc_to_marker(choose_swclist4);
                    //writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(nt.listNeuron[par1].n)+QString("_choosepoints3.swc"),choose_ntlist3);
                    //writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(nt.listNeuron[par1].n)+QString("_choosepoints4.swc"),choose_ntlist4);
                    //writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(nt.listNeuron[par1].n)+QString("_choosepoints3.marker"),choose_markerlist3);
                    //writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(nt.listNeuron[par1].n)+QString("_choosepoints4.marker"),choose_markerlist4);

                    //fitline
                    double a2,b2,c2,angle2;
                    cv::Vec6f line_para3,line_para4;
                    if(choose_swclist3.size() < 3 || choose_swclist4.size() < 3 )
                        break;
                    getFitLine(choose_swclist3,choose_swclist4,line_para3,line_para4);

                    //calculate angle
                    PointCoordinate ProjPoint5,ProjPoint6,ProjPoint7,ProjPoint8;
                    GetProjPoint(choose_swclist3,line_para3,ProjPoint5,ProjPoint6);
                    GetProjPoint(choose_swclist4,line_para4,ProjPoint7,ProjPoint8);

                    a2 = ((ProjPoint6.x - ProjPoint5.x)*(ProjPoint8.x - ProjPoint7.x)+(ProjPoint6.y - ProjPoint5.y)*(ProjPoint8.y - ProjPoint7.y)+(ProjPoint6.z - ProjPoint5.z)*(ProjPoint8.z - ProjPoint7.z));
                    b2= sqrt((ProjPoint6.x - ProjPoint5.x)*(ProjPoint6.x - ProjPoint5.x) + (ProjPoint6.y - ProjPoint5.y)*(ProjPoint6.y - ProjPoint5.y) + (ProjPoint6.z - ProjPoint5.z)*(ProjPoint6.z - ProjPoint5.z));
                    c2= sqrt((ProjPoint8.x - ProjPoint7.x)*(ProjPoint8.x - ProjPoint7.x) + (ProjPoint8.y - ProjPoint7.y)*(ProjPoint8.y - ProjPoint7.y) + (ProjPoint8.z - ProjPoint7.z)*(ProjPoint8.z - ProjPoint7.z));
                    cout<<"a2 = "<<a2<<"   b2 = "<<b2<<"    c2 = "<<c2<<endl;
                    angle2 = a2/(b2*c2);
                    cout<<"angle2 = "<<angle2<<endl;

                    //pattern search
                    QList<NeuronSWC> pattern_swclist;
                    NeuronTree pattern_ntlist;
                    if(angle1 < -0.7 && angle2 < -0.7);
                    {
                        count++;
                        angle_branch_swclist.push_back(nt.listNeuron[branch_swclist[i].n-1]);
                        for(int m = 0;m < nt_gold.listNeuron.size();m++)
                        {
                            if((nt_gold.listNeuron[m].x < nt.listNeuron[branch_swclist[i].n-1].x +10 && nt_gold.listNeuron[m].x > nt.listNeuron[branch_swclist[i].n-1].x - 10)&&(nt_gold.listNeuron[m].y < nt.listNeuron[branch_swclist[i].n-1].y +10 && nt_gold.listNeuron[m].y > nt.listNeuron[branch_swclist[i].n-1].y - 10)
                                    &&(nt_gold.listNeuron[m].z < nt.listNeuron[branch_swclist[i].n-1].z +10 && nt_gold.listNeuron[m].z > nt.listNeuron[branch_swclist[i].n-1].z - 10))
                                pattern_swclist.push_back(nt_gold.listNeuron[m]);
                        }
                    }
                    pattern_ntlist.listNeuron = pattern_swclist;
                    writeSWC_file(filegold+QString::number(i)+QString("_pattern.swc"),pattern_ntlist);


                    QList<NeuronSWC> chooseChild_swclist1,chooseChild_swclist2,chooseParent_swclist;
                    NeuronTree chooseParent_ntlist;
                    if(angle1 < -0.8 && angle2 < -0.8)
                    {
                        V3DLONG par = branch_swclist[i].n-1;

                        //remove parents
                        chooseParent_swclist.clear();
                        getAllParent(nt,childs,par,chooseParent_swclist,VOID);
                        for(int l = chooseParent_swclist.size()-1;l>=0;l--)
                        {
                            remove_swclist.push_back(chooseParent_swclist[l]);
                        }
                        cout<<"chooseParent_swclist = "<<chooseParent_swclist.size()<<endl;
                        chooseParent_ntlist.listNeuron = chooseParent_swclist;
                        //writeSWC_file(fileOpenName + QString("_chooseParent.swc"),chooseParent_ntlist);

                        //remove all childs
                        chooseChild_swclist1.clear();
                        chooseChild_swclist2.clear();
                        getAllChild(nt,childs,childs[par][0],chooseChild_swclist1,VOID);
                        getAllChild(nt,childs,childs[par][1],chooseChild_swclist2,VOID);
                        remove_swclist.append(chooseChild_swclist1);
                        remove_swclist.append(chooseChild_swclist2);
                    }
                    cout<<"remove_swclist_temp = "<<remove_swclist.size()<<endl;

                    break;
                }
            }

            if(childs[par1].size() >= 3)
            {
                for(int u = 0;u < childs[par1].size();u++)
                    cout<<"wrongpoint "<<u<<" = "<<childs[par1][u]<<"  ";
                cout<<endl;
                break;
            }
        }
    }

    cout<<"count = "<<count<<endl;
    if(count > 0)
    {
        angle_branch_ntlist.listNeuron = angle_branch_swclist;
        angle_branch_markerlist = swc_to_marker(angle_branch_swclist);
        writeSWC_file(fileOpenName + QString("_anglebranch.swc"),angle_branch_ntlist);
        writeMarker_file(fileOpenName + QString("anglebranch.marker"),angle_branch_markerlist);
    }

    cout<<"remove_swclist = "<<remove_swclist.size()<<endl;
    remove_ntlist.listNeuron = remove_swclist;
    writeSWC_file(fileOpenName + QString("_remove.swc"),remove_ntlist);
    for(int p = 0;p < nt.listNeuron.size();p++)
    {
        for(int q = 0;q < remove_swclist.size();q++)
        {
            if((nt.listNeuron[p].x == remove_swclist[q].x)&&(nt.listNeuron[p].y == remove_swclist[q].y)&&(nt.listNeuron[p].z == remove_swclist[q].z))
                nt.listNeuron.removeAt(p);
        }
    }
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;
    writeSWC_file(fileOpenName + QString("_result.swc"),nt);

    return true;
}


bool branch_angle(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    QString fileOpenName = QString(inlist->at(0));
    NeuronTree nt = readSWC_file(fileOpenName);
    cout<<fileOpenName.toStdString()<<endl;
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;
    QString fileSaveName;

    if (output.size()==0)
    {
        printf("No outputfile specified.\n");
        fileSaveName = fileOpenName + "_result.swc";
    }
    else if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        fileSaveName = QString(outlist->at(0));
    }
    else
    {
        printf("You have specified more than 1 output file.\n");
        return false;
    }

    vector<vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );

    //getChildNum
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    //get branchpoints and wrongpoints
    int childNum;
    QList<NeuronSWC> branch_swclist,wrong_swclist;
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        //childNum = childs[nt.listNeuron[i].pn].size();
        childNum = childs[i].size();
        if(childNum == 2)
            branch_swclist.push_back(nt.listNeuron[i]);
        else if(childNum >= 3)
            wrong_swclist.push_back(nt.listNeuron[i]);
    }
    cout<<"branch_swclist = "<<branch_swclist.size()<<endl;
    cout<<"wrong_swclist = "<<wrong_swclist.size()<<endl;

    //method:remove by parentpoints and angle
    QList<NeuronSWC> choose_swclist1,choose_swclist2,choose_swclist3,choose_swclist4,remove_swclist;
    NeuronTree remove_ntlist;
    QList<ImageMarker> choose_markerlist1,choose_markerlist2;
    double dist_branch,dist_thres = 10;
    for(int i = 0;i < branch_swclist.size();i++)
    {
        V3DLONG par = branch_swclist[i].n-1;
        //cout<<"par = "<<par<<endl;
        for(int n = 0;n < 10;n++)
        {
            V3DLONG par1 = getParent(par,nt);
            //cout<<"par1 = "<<par1<<endl;
             par = par1;
            if(nt.listNeuron[par1].pn == -1)
                break;
            //cout<<"childs[par1].size() = "<<childs[par1].size()<<endl;
            if(childs[par1].size() == 2)
            {
                dist_branch = NTDIS(nt.listNeuron[par1],nt.listNeuron[branch_swclist[i].n-1]);
                if(dist_branch <= dist_thres)
                {
                    cout<<"nearby branch :"<<nt.listNeuron[par1].n<<"   "<<nt.listNeuron[branch_swclist[i].n-1].n<<"     dist_branch = "<<dist_branch<<endl;
                    choose_swclist1.clear();
                    choose_markerlist1.clear();
                    choose_swclist2.clear();
                    choose_markerlist2.clear();
                    V3DLONG par11 = childs[branch_swclist[i].n-1][0];
                    V3DLONG par21 = childs[branch_swclist[i].n-1][1];
                    //cout<<"par11 = "<<par11<<"      par21 = "<<par21<<endl;

                    choosePoint(nt,childs,par11,par21,choose_swclist1,choose_swclist2,5);
                    cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist1 = "<<choose_swclist1.size()<<endl;
                    cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist2 = "<<choose_swclist2.size()<<endl;

                    //fitline
                    double a,b,c,angle;
                    cv::Vec6f line_para1,line_para2;
                    if(choose_swclist1.size() < 3 || choose_swclist2.size() < 3 )
                        break;
                    getFitLine(choose_swclist1,choose_swclist2,line_para1,line_para2);

                    //calculate angle
                    PointCoordinate ProjPoint1,ProjPoint2,ProjPoint3,ProjPoint4;
                    GetProjPoint(choose_swclist1,line_para1,ProjPoint1,ProjPoint2);
                    GetProjPoint(choose_swclist2,line_para2,ProjPoint3,ProjPoint4);

                    a = ((ProjPoint2.x - ProjPoint1.x)*(ProjPoint4.x - ProjPoint3.x)+(ProjPoint2.y - ProjPoint1.y)*(ProjPoint4.y - ProjPoint3.y)+(ProjPoint2.z - ProjPoint1.z)*(ProjPoint4.z - ProjPoint3.z));
                    b= sqrt((ProjPoint2.x - ProjPoint1.x)*(ProjPoint2.x - ProjPoint1.x) + (ProjPoint2.y - ProjPoint1.y)*(ProjPoint2.y - ProjPoint1.y) + (ProjPoint2.z - ProjPoint1.z)*(ProjPoint2.z - ProjPoint1.z));
                    c= sqrt((ProjPoint4.x - ProjPoint3.x)*(ProjPoint4.x - ProjPoint3.x) + (ProjPoint4.y - ProjPoint3.y)*(ProjPoint4.y - ProjPoint3.y) + (ProjPoint4.z - ProjPoint3.z)*(ProjPoint4.z - ProjPoint3.z));
                    cout<<"a = "<<a<<"   b = "<<b<<"    c = "<<c<<endl;
                    angle = a/(b*c);
                    cout<<"angle = "<<angle<<endl;

                    //child and parent
                    cout<<"childs[par1].size() = "<<childs[par1].size()<<endl;
                    choose_swclist3.clear();
                    choose_swclist4.clear();
                    cout<<"par1 = "<<par1<<endl;
                    if(par1 <= 4)  break;
                    getAllParent(nt,childs,par1-1,choose_swclist3,4);

                    V3DLONG par_child1 = childs[par1][0];
                    V3DLONG par_child2 = childs[par1][1];
                    V3DLONG par_child;
                    cout<<"par_child1 = "<<par_child1<<"   par_child2 = "<<par_child2<<endl;
                    par_child = chooseonechildline(par_child1,par_child2,branch_swclist[i].n,nt,childs);
                    cout<<"par_child = "<<par_child<<endl;
                    choosePoint2(nt,childs,par_child,choose_swclist4,5);

                    cout<<"branchpoint "<<nt.listNeuron[par1].n<<": choose_swclist3 = "<<choose_swclist3.size()<<endl;
                    cout<<"branchpoint "<<nt.listNeuron[par1].n<<": choose_swclist4 = "<<choose_swclist4.size()<<endl;

                    //fitline
                    double a2,b2,c2,angle2;
                    cv::Vec6f line_para3,line_para4;
                    if(choose_swclist3.size() < 3 || choose_swclist4.size() < 3 )
                        break;
                    getFitLine(choose_swclist3,choose_swclist4,line_para3,line_para4);

                    //calculate angle
                    PointCoordinate ProjPoint5,ProjPoint6,ProjPoint7,ProjPoint8;
                    GetProjPoint(choose_swclist3,line_para3,ProjPoint5,ProjPoint6);
                    GetProjPoint(choose_swclist4,line_para4,ProjPoint7,ProjPoint8);

                    a2 = ((ProjPoint6.x - ProjPoint5.x)*(ProjPoint8.x - ProjPoint7.x)+(ProjPoint6.y - ProjPoint5.y)*(ProjPoint8.y - ProjPoint7.y)+(ProjPoint6.z - ProjPoint5.z)*(ProjPoint8.z - ProjPoint7.z));
                    b2= sqrt((ProjPoint6.x - ProjPoint5.x)*(ProjPoint6.x - ProjPoint5.x) + (ProjPoint6.y - ProjPoint5.y)*(ProjPoint6.y - ProjPoint5.y) + (ProjPoint6.z - ProjPoint5.z)*(ProjPoint6.z - ProjPoint5.z));
                    c2= sqrt((ProjPoint8.x - ProjPoint7.x)*(ProjPoint8.x - ProjPoint7.x) + (ProjPoint8.y - ProjPoint7.y)*(ProjPoint8.y - ProjPoint7.y) + (ProjPoint8.z - ProjPoint7.z)*(ProjPoint8.z - ProjPoint7.z));
                    cout<<"a2 = "<<a2<<"   b2 = "<<b2<<"    c2 = "<<c2<<endl;
                    angle2 = a2/(b2*c2);
                    cout<<"angle2 = "<<angle2<<endl;

                    QList<NeuronSWC> chooseChild_swclist1,chooseChild_swclist2,chooseParent_swclist;
                    if(angle < -0.7 && angle2 < -0.7)
                    {
                        V3DLONG par = branch_swclist[i].n-1;

                        //remove parents
                        chooseParent_swclist.clear();
                        getAllParent(nt,childs,par,chooseParent_swclist,VOID);
                        for(int l = chooseParent_swclist.size()-1;l>=0;l--)
                        {
                            remove_swclist.push_back(chooseParent_swclist[l]);
                        }
                        cout<<"chooseParent_swclist = "<<chooseParent_swclist.size()<<endl;

                        //remove all childs
                        chooseChild_swclist1.clear();
                        chooseChild_swclist2.clear();
                        getAllChild(nt,childs,childs[par][0],chooseChild_swclist1,VOID);
                        getAllChild(nt,childs,childs[par][1],chooseChild_swclist2,VOID);
                        remove_swclist.append(chooseChild_swclist1);
                        remove_swclist.append(chooseChild_swclist2);
                    }
                    cout<<"remove_swclist_temp = "<<remove_swclist.size()<<endl;

                    break;
                }
            }

            if(childs[par1].size() >= 3)
            {
                for(int u = 0;u < childs[par1].size();u++)
                    cout<<"wrongpoint "<<u<<" = "<<childs[par1][u]<<"  ";
                cout<<endl;
                break;
            }
        }
    }

    cout<<"remove_swclist = "<<remove_swclist.size()<<endl;
    remove_ntlist.listNeuron = remove_swclist;
    writeSWC_file(fileOpenName + QString("_remove.swc"),remove_ntlist);
    for(int p = 0;p < nt.listNeuron.size();p++)
    {
        for(int q = 0;q < remove_swclist.size();q++)
        {
            if((nt.listNeuron[p].x == remove_swclist[q].x)&&(nt.listNeuron[p].y == remove_swclist[q].y)&&(nt.listNeuron[p].z == remove_swclist[q].z))
                nt.listNeuron.removeAt(p);
        }
    }
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;
    writeSWC_file(fileOpenName + QString("_result.swc"),nt);

//    QList<NeuronSWC> finalswc;
//    finalswc = nt.listNeuron;
//    if (!export_list2file(finalswc, fileSaveName, fileOpenName))
//    {
//        printf("fail to write the output swc file.\n");
//        return false;
//    }

    return true;
}


QList<ImageMarker> swc_to_marker(QList<NeuronSWC> swc_list)
{
    QList<ImageMarker> marker_list;
    for(V3DLONG i=0;i<swc_list.size();i++)
    {
        ImageMarker m;
        m.x = swc_list[i].x + 1;
        m.y = swc_list[i].y + 1;
        m.z = swc_list[i].z + 1;
        //m.radius = branchpoints[i].radius;
        m.radius=0.001;
        m.color.a = 0;
        m.color.b = 0;
        m.color.g = 0;
        m.type = swc_list[i].type;
        marker_list.push_back(m);
    }
    return marker_list;
}

void choosePoint(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG &par11,V3DLONG &par21,QList<NeuronSWC> &choose_swclist1,QList<NeuronSWC> &choose_swclist2,int m)
{
    //QList<NeuronSWC> branchChildtoolittle_swclist;
    //NeuronTree branchChildtoolittle_ntlist;
    NeuronSWC child1,child2;
    child1 = nt.listNeuron[par11];
    child2 = nt.listNeuron[par21];
    //choose_swclist1.push_back(child1);
    //choose_swclist2.push_back(child2);

    for(int n =0;n < m;n++)
    {
        if(childs[child1.n-1].size()==0 || childs[child1.n-1].size()>=2)
        {
            //branchChildtoolittle_swclist.push_back(nt.listNeuron[branch_swclist[i].n]);
            break;
        }
        if(childs[child2.n-1].size()==0 || childs[child2.n-1].size()>=2)
            break;
        V3DLONG par1 = childs[child1.n-1][0];
        //cout<<childs[child1.n-1].size()<<endl;
        V3DLONG par2 = childs[child2.n-1][0];
        //cout<<"par1 = "<<par1<<"    par2 = "<<par2<<endl;
        child1 = nt.listNeuron[par1];
        child2 = nt.listNeuron[par2];
        choose_swclist1.push_back(child1);
        choose_swclist2.push_back(child2);
    }

//    branchChildtoolittle_ntlist.listNeuron = branchChildtoolittle_swclist;
//    if(branchChildtoolittle_swclist.size()>0)
//    {
//        writeSWC_file("branchChildtoolittle.swc",branchChildtoolittle_ntlist);
//    }

    return;
}

void choosePoint2(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG &par11,QList<NeuronSWC> &choose_swclist1,int m)
{
    NeuronSWC child1;
    child1 = nt.listNeuron[par11];
    //choose_swclist1.push_back(child1);
    for(int n =0;n < m;n++)
    {
        if(childs[child1.n-1].size()==0 || childs[child1.n-1].size()>=2)
        {
            //branchChildtoolittle_swclist.push_back(nt.listNeuron[branch_swclist[i].n]);
            break;
        }

        V3DLONG par1 = childs[child1.n-1][0];
        //cout<<childs[child1.n-1].size()<<endl;
        child1 = nt.listNeuron[par1];
        choose_swclist1.push_back(child1);
    }

    return;
}


V3DLONG chooseonechildline(V3DLONG &par_child1,V3DLONG &par_child2,V3DLONG &branchpoint,NeuronTree &nt,vector<vector<V3DLONG> > &childs)
{
    cout<<"branchpoint = "<<branchpoint<<endl;
    V3DLONG par_child;
    NeuronSWC child1,child2;
    child1 = nt.listNeuron[par_child1];
    child2 = nt.listNeuron[par_child2];
    cout<<"child1.n = "<<child1.n<<"     child2.n = "<<child2.n<<endl;
    cout<<childs[child1.n-1].size()<<endl;
    for(int s = 0;s < 10;s++)
    {
        if(childs[child1.n-1].size() == 0)
            break;
        V3DLONG par = childs[child1.n-1][0];
        cout<<"par = "<<par<<endl;

        if(par == branchpoint)
        {
            par_child = par_child2;
            break;
        }
        child1 = nt.listNeuron[par];

    }
    if(par_child == par_child2)
        return par_child;
    else
    {
        for(int s = 0;s < 10;s++)
        {
            if(childs[child2.n-1].size() == 0)
                break;
            V3DLONG par = childs[child2.n-1][0];
            cout<<"par = "<<par<<endl;

            if(par == branchpoint)
            {
                par_child = par_child1;
                break;
            }

            child2 = nt.listNeuron[par];
        }
    }

    return par_child;
}

void getFitLine(QList<NeuronSWC> &swclist1,QList<NeuronSWC> &swclist2,Vec6f &line_para1,Vec6f &line_para2)
{
    //输入拟合点
    std::vector<cv::Point3d> points1,points2;
    for(int j = 0;j < swclist1.size();j++)
    {
        //cout<<choose_swclist1[j].x<<choose_swclist1[j].y<<choose_swclist1[j].z<<endl;
        points1.push_back(cv::Point3d(swclist1[j].x,swclist1[j].y,swclist1[j].z));
    }
    for(int j = 0;j < swclist2.size();j++)
    {
        points2.push_back(cv::Point3d(swclist2[j].x,swclist2[j].y,swclist2[j].z));
    }
    //cout<<"points1 = "<<points1.size()<<"     points2 = "<<points2.size()<<endl;

    //调用拟合函数
    //cv::Vec6f line_para1,line_para2;
    cv::fitLine(points1, line_para1, cv::DIST_L2, 0, 1e-2, 1e-2);
    cv::fitLine(points2, line_para2, cv::DIST_L2, 0, 1e-2, 1e-2);
    //cout<<"line_para1 = "<<line_para1<<endl;
    //cout<<"line_para2 = "<<line_para2<<endl;

    return;
}




void GetProjPoint(QList<NeuronSWC> &swclist,Vec6f &line_para,PointCoordinate &ProjPoint1,PointCoordinate &ProjPoint2)
{
    PointCoordinate Point1,Point2,LinePoint1;

    //linepoint1
    LinePoint1.x = line_para[3];
    LinePoint1.y = line_para[4];
    LinePoint1.z = line_para[5];

    int n = swclist.size()-1;
    //point
    Point1.x = swclist[0].x; Point1.y = swclist[0].y; Point1.z = swclist[0].z;
    Point2.x = swclist[n].x; Point2.y = swclist[n].y; Point2.z = swclist[n].z;

    double t1 = -((LinePoint1.x - Point1.x)*line_para[0]+(LinePoint1.y - Point1.y)*line_para[1]+(LinePoint1.z - Point1.z)*line_para[2])
            /(line_para[0]*line_para[0] + line_para[1]*line_para[1] + line_para[2]*line_para[2]);
    double t2 = -((LinePoint1.x - Point2.x)*line_para[0]+(LinePoint1.y - Point2.y)*line_para[1]+(LinePoint1.z - Point2.z)*line_para[2])
            /(line_para[0]*line_para[0] + line_para[1]*line_para[1] + line_para[2]*line_para[2]);
    //cout<<"t1 = "<<t1<<"     t2 = "<<t2<<endl;

    ProjPoint1.x = t1*line_para[0] + LinePoint1.x;
    ProjPoint1.y = t1*line_para[1] + LinePoint1.y;
    ProjPoint1.z = t1*line_para[2] + LinePoint1.z;

    ProjPoint2.x = t2*line_para[0] + LinePoint1.x;
    ProjPoint2.y = t2*line_para[1] + LinePoint1.y;
    ProjPoint2.z = t2*line_para[2] + LinePoint1.z;

    //cout<<"ProjPoint1.x = "<<ProjPoint1.x<<"  ProjPoint1.y = "<<ProjPoint1.y<<"  ProjPoint1.z = "<<ProjPoint1.z<<endl;
    //cout<<"ProjPoint2.x = "<<ProjPoint2.x<<"  ProjPoint2.y = "<<ProjPoint2.y<<"  ProjPoint2.z = "<<ProjPoint2.z<<endl;

//    double D,D1,D2;
//    D = -(line_para[0]*line_para[3] + line_para[1]*line_para[4] + line_para[2]*line_para[5]);
//    D1 = -(line_para[0]*ProjPoint1.x + line_para[1]*ProjPoint1.y + line_para[2]*ProjPoint1.z);
//    D2 = -(line_para[0]*ProjPoint2.x + line_para[1]*ProjPoint2.y + line_para[2]*ProjPoint2.z);
//    cout<<"D = "<<D<<"     D1 = "<<D1<<"     D2 = "<<D2<<endl;

    return;
}


PointCoordinate GetFootOfPerpendicular(PointCoordinate &pt,PointCoordinate &begin,PointCoordinate &end)
{
    PointCoordinate retVal;
    double dx = begin.x - end.x;
    double dy = begin.y - end.y;
    double dz = begin.z - end.z;
    if(abs(dx) < 0.00000001 && abs(dy) < 0.00000001 && abs(dz) < 0.00000001 )
    {
        retVal = begin;
        return retVal;
     }

    double u = (pt.x - begin.x)*(begin.x - end.x) +(pt.y - begin.y)*(begin.y - end.y) + (pt.z - begin.z)*(begin.z - end.z);
    u = u/((dx*dx)+(dy*dy)+(dz*dz));

    retVal.x = begin.x + u*dx;
    retVal.y = begin.y + u*dy;
    retVal.y = begin.z + u*dz;

    return retVal;
}

void getAllChild(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG par, QList<NeuronSWC> &swclist,int m)
{
    //cout<<"par = "<<par<<endl;
    //QList<NeuronSWC>swclist1,swclist2;
    for(int n =0;n < m;n++)
    {
        swclist.push_back(nt.listNeuron[par]);
        V3DLONG par1 = childs[par][0];
        //cout<<"par1 = "<<par1<<endl;

        if(childs[par1].size() == 0)
            break;
        if(childs[par1].size() >= 2)
        {
            swclist.push_back(nt.listNeuron[par1]);
            for(int i = 0;i < childs[par1].size();i++)
                 getAllChild(nt,childs,childs[par1][i],swclist,VOID);
            break;
        }

         par =par1;

    }
   // cout<<"swclist = "<<swclist.size()<<endl;
    return;
 }


void getAllParent(NeuronTree &nt,vector<vector<V3DLONG> > &childs,V3DLONG par, QList<NeuronSWC> &swclist,int n)
{
    for(int i = 0;i < n;i++)
    {
        V3DLONG par1 = getParent(par,nt);
        //cout<<"par1 = "<<par1<<endl;
        swclist.push_back(nt.listNeuron[par]);
        par = par1;
        if(childs[par1].size() == 2 || childs[par1].size() == 0)
            break;
    }

    return;
}

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
//	myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<endl;
//	myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
}
