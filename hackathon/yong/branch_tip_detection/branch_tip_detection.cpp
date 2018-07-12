
#include "branch_tip_detection.h"
#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "v3d_interface.h"
#include <algorithm>

#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"

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
    //v3d_msg("check1");

    //getChildNum
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(nt.listNeuron[i].pn)].push_back(i);
        //cout<<"i="<<i<<"nt.hashNeuron.value(nt.listNeuron[i].pn)="<<nt.hashNeuron.value(nt.listNeuron[i].pn)<<endl;
    }
    //v3d_msg("check2");

    QList<NeuronSWC> branchpoints,tips;
    int child_num;

    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC cur = nt.listNeuron[i];
        child_num = childs[i].size();
        //cout<<"i="<<i<<" "<<"child_num="<<child_num<<endl;
        if(child_num>=2)
            branchpoints.push_back(cur);
        else if(child_num==0)
            tips.push_back(cur);
    }
    //v3d_msg("check3");


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

    cout<<"branchpoints="<<branchpoints.size()<<endl;
    cout<<"tips="<<tips.size()<<endl;

    //Output swc_to_marker_file
    QList<ImageMarker> branch_markerlist,tip_markerlist;
    QString filename1 =fileOpenName + "_branch_markerlist.marker";
    QString filename2=fileOpenName + "_tip_markerlist.marker";
    for(V3DLONG i=0;i<branchpoints.size();i++)
    {
        ImageMarker m;
        m.x = branchpoints[i].x;
        m.y = branchpoints[i].y;
        m.z = branchpoints[i].z;
        //m.radius = branchpoints[i].radius;
        m.radius=0.001;
        m.color.a = 0;
        m.color.b = 0;
        m.color.g = 0;
        m.type = branchpoints[i].type;
        branch_markerlist.push_back(m);
    }
    writeMarker_file(filename1,branch_markerlist);

    for(V3DLONG i=0;i<tips.size();i++)
    {
        ImageMarker m;
        m.x = tips[i].x;
        m.y = tips[i].y;
        m.z = tips[i].z;
        //m.radius = tips[i].radius;
        m.radius=0.001;
        m.color.a = 0;
        m.color.b = 255;
        m.color.g = 0;
        m.type = tips[i].type;
        tip_markerlist.push_back(m);
    }
    writeMarker_file(filename2,tip_markerlist);

    //Output swc_to_apo_file
    unsigned int Vsize;
    Vsize = QInputDialog::getInteger(parent, "Volume size ",
                                  "Enter volume size:",
                                  50, 1, 1000, 1);


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

    QString fileDefaultName1 = fileOpenName + "_branchpiont.apo";
    writeAPO_file(fileDefaultName1,branchpoint_inmarkers);

    //    QString fileSaveName1 = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
    //                                                        fileDefaultName1,
    //                                                        QObject::tr("Supported file (*.apo)"));
    //    if (fileSaveName1.isEmpty())
    //        v3d_msg("return false");
    //    else
    //    {
    //        writeAPO_file(fileSaveName1,branchpoint_inmarkers);
    //        v3d_msg(QString("Point Cloud file is save as %1").arg(fileSaveName1.toStdString().c_str()));
    //    }

    for(V3DLONG i = 0; i <tips.size();i++)
    {
        CellAPO t;
        t.x = tips.at(i).x+1;
        t.y = tips.at(i).y+1;
        t.z = tips.at(i).z+1;
        t.color.r=255;
        t.color.g=0;
        t.color.b=0;

        t.volsize = Vsize;
        tip_inmarkers.push_back(t);
    }

    QString fileDefaultName2 = fileOpenName + "_tip.apo";
    writeAPO_file(fileDefaultName2,tip_inmarkers);

//    QString fileSaveName2 = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
//                                                        fileDefaultName2,
//                                                        QObject::tr("Supported file (*.apo)"));
//    if (fileSaveName2.isEmpty())
//        v3d_msg("return false");
//    else
//    {
//        writeAPO_file(fileSaveName2,tip_inmarkers);
//        v3d_msg(QString("Point Cloud file is save as %1").arg(fileSaveName2.toStdString().c_str()));
//    }

    return true;
}

QList<NeuronSWC>  removedupSWC(QList<NeuronSWC> & neuron,QString fileOpenName)
{
    //Remove duplicated nodes
    //get ids and reorder tree with ids following list
    vector<long> parents0;
    vector<long> ids0;
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

    QList<NeuronSWC> result;
    result.append(neuron.at(0));
    for(V3DLONG i=1;i<neuron.size();i++)
    {
        if(dupnodes.at(i) == false) result.append(neuron.at(i));
        //else qDebug()<<i;
    }
    //neuron = neuron2;
    qDebug()<<"Removed" << count(dupnodes.begin(),dupnodes.end(),true) << " duplicated nodes";

    NeuronTree nt;
    nt.listNeuron=result;
    QString filename=fileOpenName+"_Remove_duplicated_nodes.swc";
    writeSWC_file(filename,nt);

    return result;
}

