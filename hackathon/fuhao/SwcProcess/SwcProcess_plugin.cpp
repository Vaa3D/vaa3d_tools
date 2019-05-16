/* SwcProcess_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-1-3 : by fuhao
 */
 
#include "v3d_message.h"
#include <vector>
#include "SwcProcess_plugin.h"
#include <iostream>
#include "basic_surf_objs.h"
//#include"tipdetector.h"
#include<QFileDialog>
#include"tipdetector.h"
#include "my_surf_objs.h"
#include "neurontreepruneorgraft.h"
#include "scoreinput.h"

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))


using namespace std;
Q_EXPORT_PLUGIN2(SwcProcess, TestPlugin);
enum FunctionFromFileOr3dview{FunctionFromFile=0,FunctionFrom3dview=1};
 void processSwc(V3DPluginCallback2 &callback, QWidget *parent,QList<NeuronTree> * mTreeList,FunctionFromFileOr3dview func_type);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("SWC_Process_3d")
        <<tr("SWC_Process_file")
        <<tr("tip_detector")
//        <<tr("tip_checker_marker")
//        <<tr("try_prun")
//        <<tr("try_prun2")
//        <<tr("try_prun3")
//        <<tr("first_graft_then_prun")
//        <<tr("tip_based_grafting")
//        <<tr("sort_neuron")
//        <<tr("reset_radius")
        <<tr("NeuronTree_Prune_Graft")
        <<tr("TMI_tip_process")
        <<tr("TMI_tip_process_old")
        <<tr("TMI_tip_process_show")
//        <<tr("input_swc_output_img")
        <<tr("Hackthon_tip_postProcess")
        <<tr("about")
          ;
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void pruningSwcSmallLength(NeuronTree &nt,double length = 10)
{
    //input NeuronTree  ,small length
    //

    QVector<QVector<V3DLONG> > childs;
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    double *segment_length = new double[neuronNum];
    V3DLONG *parent_id = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        segment_length[i] = 100000.00;
        parent_id[i] = -1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0)
        {
            int parent_tip = getParent(i,nt);
            MyMarker curr_node, parent_node;
            curr_node.x = list.at(i).x;
            curr_node.y = list.at(i).y;
            curr_node.z = list.at(i).z;

            parent_node.x = list.at(parent_tip).x;
            parent_node.y = list.at(parent_tip).y;
            parent_node.z = list.at(parent_tip).z;
            double index_tip = dist(curr_node,parent_node);

            while(childs[parent_tip].size()<2)
            {
                MyMarker curr_node, parent_node;

                curr_node.x = list.at(parent_tip).x;
                curr_node.y = list.at(parent_tip).y;
                curr_node.z = list.at(parent_tip).z;

                parent_node.x = list.at(getParent(parent_tip,nt)).x;
                parent_node.y = list.at(getParent(parent_tip,nt)).y;
                parent_node.z = list.at(getParent(parent_tip,nt)).z;

                index_tip += dist(curr_node,parent_node);

                parent_tip = getParent(parent_tip,nt);

                if(parent_tip == 1000000000)
                    break;

             }

            int parent_index = parent_tip;

            if(index_tip < length)
            {
                flag[i] = -1;
                segment_length[i] = index_tip;
                parent_id[i] = parent_index;
                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    flag[parent_tip] = -1;
                    segment_length[parent_tip] = index_tip;
                    parent_id[parent_tip] = parent_index;
                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
                }
                if(segment_length[parent_tip] > index_tip)
                    segment_length[parent_tip]  = index_tip;
            }
        }
    }


    for (int i=0;i<before_prunning_swc.size();i++)
    {
        if(flag[i] == 1 || (flag[i] != 1 && (segment_length[i] > segment_length[parent_id[i]])))
        {
            after_prunning_swc.push_back(before_prunning_swc[i]);
        }

   }

    if(flag) {delete[] flag; flag = 0;}
    if(segment_length) {delete[] segment_length; segment_length = 0;}
    if(parent_id) {delete[] parent_id; parent_id = 0;}

    QString fileDefaultName = QString("_prunned.swc");
    //write new SWC to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.swc)"
                ";;Neuron structure	(*.swc)"
                ));

    saveSWC_file(fileSaveName.toStdString(), after_prunning_swc);
    // output as  after_prunning_swc
}


void processSwcOut(LandmarkList s)
{
    if(s.isEmpty())
        return;
    QString path=QFileDialog::getSaveFileName(0,"Save as marker",".","Marker Files(*.marker)");
   if(path.isEmpty())
       return;
    FILE * fp = fopen(path.toLatin1(), "wt");
    if (!fp)
    {
        v3d_msg("Could not open the file to save the marker.");
        return ;
    }
    fprintf(fp, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");
    for(int i=0;i<s.size();i++)
    {
        LocationSimple p=s.at(i);

        fprintf(fp, "%5.3f,%5.3f,%5.3f,%5.3f,%d,%s,%s,%d,%d,%d \n",
                p.x,p.y,p.z,p.radius,p.shape, p.name, p.comments,p.color.r,p.color.g,p.color.b);
    }
     fclose(fp);
    return;


}

void processTipPointFromLandmarkList(LandmarkList curlist, V3DPluginCallback2 &callback)
{
    if(curlist.isEmpty())
    {
        cout<<"no marker input"<<endl;
        return ;
    }
    else
    {
        cout<<"there input "<<curlist.size() <<" markers in the original markers"<<endl;
    }
    TipDetector tp;
    tp.getImgData(callback);
    tp.GUI_input_argu();
    LandmarkList realtip_list;
    for(int i=0;i<curlist.size();i++)
    {
        bool isTip;
        LocationSimple candi;
        isTip=tp.TipDetect_onePoint_neighbor(curlist.at(i),candi,1);
        if(!isTip)
        {
            cout<<"not 27neighbor tip "<<i<<"th x"<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
//                tip_point.at(i).color = blue;
        }
        else
        {
            cout<<"27neighbor tip "<<i<<"th x"<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
            cout<<"27neighbor tip and near "<<i<<"th x"<<candi.x<<" y "<<candi.y<<" z "<<candi.z<<endl;
            realtip_list<<candi;
        }
    }

    if(realtip_list.isEmpty())
    {
        cout<<"there is no true tip marker in the original markers"<<endl;
    }
    else
    {
        cout<<"there is "<<realtip_list.size() <<" true tip marker "<<"and "<<curlist.size()<<" original markers"<<endl;
        callback.setLandmark(callback.currentImageWindow(), realtip_list);
        tp.cluster(realtip_list);
//        cout<<"after cluster there is "<<realtip_list.size() <<" true tip marker "<<"and "<<curlist.size()<<" original markers"<<endl;
    }
    v3dhandle curwin = callback.currentImageWindow();
    callback.setLandmark(curwin, realtip_list);
//    callback.callPluginFunc();
    return;
}
void processTipPoint(LandmarkList tip_point,LandmarkList &real_tipPoints,V3DPluginCallback2 &callback)
{
    TipDetector tp;
    tp.getImgData(callback);
    tp.GUI_input_argu();
    RGBA8 blue;
    blue.r=0;
    blue.g=0;
    blue.b=255;
    for(int i=0;i<tip_point.size();i++)
    {
        bool isTip;
        LocationSimple candi;
        isTip=tp.TipDetect_onePoint_27neighbor(tip_point.at(i),candi);
        if(!isTip)
        {
//                cout<<"not 27neighbor tip "<<i<<"th x"<<tip_point.at(i).x<<" y "<<tip_point.at(i).y<<" z "<<tip_point.at(i).z<<endl;
//   //             tip_point.at(i).color = blue;
        }
        else
        {
//            cout<<"27neighbor tip "<<i<<"th x"<<tip_point.at(i).x<<" y "<<tip_point.at(i).y<<" z "<<tip_point.at(i).z<<endl;
            real_tipPoints<<candi;
        }
    }
       cout<<"tip_point size is "<<tip_point.size()<<endl;
    cout<<"real_tipPoints size is "<<real_tipPoints.size()<<endl;
//    tp.cluster(real_tipPoints);
    cout<<"after cluster real_tipPoints size is "<<real_tipPoints.size()<<endl;

    return;
}

void getNeuronTreeFromFile(V3DPluginCallback2 &callback, QWidget *parent)
{
    QList<NeuronTree> * mTreeList;
    mTreeList=new  QList<NeuronTree>;
    QString path=QFileDialog::getOpenFileName(0,"swc select",".","swc File(*.swc)");
    if(path.isEmpty())
        return;
    NeuronTree filetree=readSWC_file(path);
    cout<<"read SWC from file "<<path.toStdString()<<endl;
    mTreeList->push_back(filetree);
    processSwc(callback,parent,mTreeList,FunctionFromFileOr3dview::FunctionFromFile);

}

void getNeuronTreeFrom3dView(V3DPluginCallback2 &callback, QWidget *parent)
{
    QList<NeuronTree> * mTreeList;
    QList <V3dR_MainWindow *> list_3dviewer = callback.getListAll3DViewers();
    if (list_3dviewer.size() < 1)
    {
        v3d_msg("Please open  a SWC file from the main menu first!");
        return;
    }
    V3dR_MainWindow *surface_win = list_3dviewer[0];
    if (!surface_win){
        v3d_msg("Please open up a SWC file from the main menu first!");
        return;
    }
    cout<<"read SWC from 3dview "<<endl;
    mTreeList = callback.getHandleNeuronTrees_Any3DViewer(surface_win);
    processSwc(callback,parent,mTreeList,FunctionFromFileOr3dview::FunctionFrom3dview);
}
void furtherHandleOf3dView(V3DPluginCallback2 &callback, QWidget *parent,LandmarkList branch_point,LandmarkList tip_point)
{
    //process
    LandmarkList real_tipPoints;
    processTipPoint(tip_point,real_tipPoints,callback);


    //out
    if(!real_tipPoints.isEmpty())
    {
        callback.setLandmark(callback.currentImageWindow(), real_tipPoints);
    }

}

void processSwc(V3DPluginCallback2 &callback, QWidget *parent,QList<NeuronTree> * mTreeList,FunctionFromFileOr3dview func_type)
{
    if(mTreeList->size()<=0)
        return;
    LandmarkList tip_point;
    LandmarkList branch_point;

    LandmarkList curlist;
    LocationSimple s;
    RGBA8 red,green;
    red.r=255;
    red.g=0;
    red.b=0;
    green.r=0;
    green.g=255;
    green.b=0;
    V3DLONG branchCount=0;
    V3DLONG TipCount=0;
    for (int i = 0 ; i < mTreeList->size(); i++)
    {
        NeuronTree  resultTree=mTreeList->at(i);

        V3DLONG size = resultTree.listNeuron.size();
         cout<<"resultTree.listNeuron.size() "<<size<<endl;
        QList<NeuronSWC> neuron = resultTree.listNeuron;
//        int *countOfNeuron=new int[size];
//        if(countOfNeuron==NULL) return ;
        int *countOfNeuron;
        try{countOfNeuron=new int [size];}
        catch(...) {v3d_msg("cannot allocate memory for countOfNeuron."); return ;}
        for (V3DLONG i=0;i<size;i++)
        {
           countOfNeuron[i]=0;
        }
        for (V3DLONG i=0;i<size;i++)
        {
            int ind=neuron.at(i).parent;
            if(ind-1>=0&&ind-1<size)
            {countOfNeuron[ind-1]++;}
            else if(ind==-1)
            {
                countOfNeuron[i]--;
            }
        }


        for(V3DLONG i=0;i<size;i++)
        {
            if(countOfNeuron[i]==0)
            {
                s.x=neuron.at(i).x+1;
                s.y=neuron.at(i).y+1;
                s.z=neuron.at(i).z+1;
                s.radius=neuron.at(i).r;
                s.color = red;
//   //             curlist << s;
                TipCount++;
//    //            cout<<"tip s.x "<<s.x<<"s.y "<<s.y<<"s.z "<<s.z<<endl;
                tip_point<<s;
                curlist << s;
                //tip
            }
            else if(countOfNeuron[i]>=2)
            {
                s.x=neuron.at(i).x+1;
                s.y=neuron.at(i).y+1;
                s.z=neuron.at(i).z+1;
                s.radius=neuron.at(i).r;
                s.color = green;
                curlist << s;
                branch_point<<s;
                branchCount++;
 //     //           cout<<"branch s.x "<<s.x<<"s.y "<<s.y<<"s.z "<<s.z<<endl;
                //branch
            }
        }
    }
//    cout<<"it is ok before it 1";

    for(int i=0;i<tip_point.size();i++)
    {
        curlist<<tip_point.at(i);
    }

    cout<<"TipCount : "<<tip_point.size()<<endl;
    cout<<"branchCount : "<<branch_point.size()<<endl;
    if(func_type==FunctionFromFileOr3dview::FunctionFrom3dview)
    {
        furtherHandleOf3dView(callback,parent,branch_point, tip_point);


    }

    processSwcOut(curlist);

    return;
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("SWC_Process_file"))
	{
        QList <V3dR_MainWindow *> list_3dviewer = callback.getListAll3DViewers();
        if (list_3dviewer.size() < 1)
        {
            v3d_msg("Please open  a SWC file from the main menu first! list_3dviewer");
            return ;
        }
        V3dR_MainWindow *surface_win = list_3dviewer[0];
        if (!surface_win){
            v3d_msg("Please open up a SWC file from the main menu first!");
            return ;
        }
    //    cout<<"read SWC from 3d Viewer  "<<endl;
        QList<NeuronTree> * mTreeList = callback.getHandleNeuronTrees_Any3DViewer(surface_win);
//        if(mTreeList->size()<=0)
//        {
//            return false;
//        }
//        mTreeList;
        QString path=QFileDialog::getOpenFileName(0,"swc select",".","swc File(*.swc)");
        if(path.isEmpty())
            return;
        NeuronTree filetree=readSWC_file(path);
        cout<<"read SWC from file "<<path.toStdString()<<endl;
        if(mTreeList->isEmpty())
        {mTreeList->push_back(filetree);}
        else
        {
//            mTreeList->push_back(filetree);
            mTreeList->replace(0,filetree);
//            mTreeList->pop_back();
        }


//        getNeuronTreeFromFile(callback,parent);
	}
    else if (menu_name == tr("SWC_Process_3d"))
    {
//        TipDetector tp;
//        tp.GetNeuronTreeFrom3dview(callback);
//        tp.FromNeuronTreeGetTipBranch(callback);
//        getNeuronTreeFrom3dView(callback,parent);
        neurontreepruneorgraft ntpg;
        ntpg.getNeuronTree(callback,parent);
        v3dhandle curwin = callback.currentImageWindow();
        LandmarkList ls;
        NeuronTree temptree=ntpg.return_neurontree();
//        temptree=ntpg.sort_neuron(temptree);
        ls<<ntpg.FromNeuronTreeGetLeaf_branch(temptree);
        cout<<"LandmarkList ls.size "<<ls.size();
        callback.setLandmark(curwin, ls);

//		v3d_msg("To be implemented.");
    }// tip_detector
    else if (menu_name == tr("tip_detector"))
    {
        TipDetector tp;
        tp.getImgData(callback);

        if(!tp.GUI_input_argu())
        {
            return;
        }
        cout<<"whole_img1"<<endl;
        tp.whole_img(callback);
        cout<<"whole_img2"<<endl;
        tp.save_tp_as_temp(callback);
//        processSwcOut(tp.OutputTipPoint());
        v3dhandle curwin = callback.currentImageWindow();
        callback.setLandmark(curwin, tp.OutputTipPoint());
        cout<<"there is "<<tp.OutputTipPoint().size()<<"tip point in img"<<endl;
        tp.show_tipdetetor_img(callback);
//		v3d_msg(" tip_detector To be implemented.");
    }// tip_detector
    else if(menu_name == tr("tip_checker_marker"))
    {
        LandmarkList  curlist=callback.getLandmark(callback.currentImageWindow());
        if(curlist.isEmpty())
        {
            v3d_msg("there is no marker input ");
        }
        processTipPointFromLandmarkList(curlist,callback);
        // not ok

    }
    else if(menu_name == tr("try_prun2"))
    {
        // pruingSwc_one_leafnode
            neurontreepruneorgraft ntpg;
            ntpg.getImgData(callback, parent);
            NeuronTree temptree=ntpg.tipBasedPruning(ntpg.return_neurontree());
            temptree=ntpg.mergeSwc_two_leafnode(temptree);
            v3dhandle curwin=callback.currentImageWindow();
            cout<<"begin to setSWC"<<endl;
            callback.setSWC(curwin,temptree);
            cout<<"end to setSWC"<<endl;
    }
    else if(menu_name == tr("try_prun3"))
    {
        neurontreepruneorgraft ntpg;
        NeuronTree temp_tree;
        ntpg.getImgData(callback, parent);
        temp_tree=ntpg.tipBasedPruning_stop_before_tip(ntpg.return_neurontree());
        v3dhandle curwin=callback.currentImageWindow();
        cout<<"begin to setSWC"<<endl;
        callback.setSWC(curwin,temp_tree);
        cout<<"end to setSWC"<<endl;
    }
    else if(menu_name == tr("first_graft_then_prun"))
    {
        neurontreepruneorgraft ntpg;
        NeuronTree temp_tree;
        ntpg.getImgData(callback, parent);
        temp_tree=ntpg.graftToBorder(ntpg.return_neurontree());
        temp_tree=ntpg.tipBasedPruning_stop_before_tip(temp_tree);
        temp_tree=ntpg.mergeSwc_two_leafnode(temp_tree);
        temp_tree=ntpg.reestimate_radius(temp_tree);
        v3dhandle curwin=callback.currentImageWindow();
        cout<<"begin to setSWC"<<endl;
        callback.setSWC(curwin,temp_tree);
        cout<<"end to setSWC"<<endl;
    }
    else if(menu_name == tr("try_prun"))
    {
        // did not change and use
        QString path=QFileDialog::getOpenFileName(0,"swc select",".","swc File(*.swc)");
        if(path.isEmpty())
            return;
        NeuronTree nt=readSWC_file(path);
        double length=10;
        pruningSwcSmallLength(nt,length );
    }
    else if(menu_name == tr("tip_based_grafting"))
    {
        neurontreepruneorgraft ntpg;
        ntpg.getImgData(callback, parent);
        NeuronTree temp_tree;
        temp_tree=ntpg.graftToBorder(ntpg.return_neurontree());
        cout<<" graftToBorder program is over";
        v3dhandle curwin = callback.currentImageWindow();
        QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\1.swc";
        writeSWC_file(filename,temp_tree);
        cout<<"begin to set swc";
        callback.setSWC(curwin,temp_tree);
        cout<<"program is over";

    }
    else if(menu_name == tr("sort_neuron"))
    {

        neurontreepruneorgraft ntpg;
        ntpg.getImgData_not_process_tip(callback,parent);
        NeuronTree nt=ntpg.return_neurontree();
        NeuronTree final_tree;

        final_tree=ntpg.sort_neuron(nt);
        QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\1.swc";
        writeSWC_file(filename,final_tree);
        QString filename2="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\2.swc";
        writeSWC_file(filename2,nt);

        v3dhandle curwin = callback.currentImageWindow();
        callback.setSWC(curwin,final_tree);
        cout<<"sort_neuron program is over";

    }
    else if(menu_name == tr("reset_radius"))
    {
        neurontreepruneorgraft ntpg;
        ntpg.getImgData_not_process_tip(callback, parent);
        NeuronTree temp_tree;
        temp_tree=ntpg.reestimate_radius(ntpg.return_neurontree());

        v3dhandle curwin = callback.currentImageWindow();
        callback.setSWC(curwin,temp_tree);
        cout<<"sort_neuron program is over";
        QString img_name=callback.getImageName(curwin);
        cout<<"callback.getImageName(curwin) "<<img_name.toUtf8().data()<<endl;
    }// NeuronTree_Prune_Graft
    else if(menu_name == tr("NeuronTree_Prune_Graft"))
    {

        neurontreepruneorgraft ntpg;
        ntpg.do_function_dialog(callback,parent);
        LandmarkList need_to_show=ntpg.return_marker_need_to_show();
        v3dhandle curwin = callback.currentImageWindow();
        callback.setLandmark(curwin,need_to_show);

        cout<<"NeuronTree_Prune_Graft program is over";
//        v3d_msg("it is ok and not close");

    }
    else if(menu_name == tr("TMI_tip_process"))//
    {
        neurontreepruneorgraft ntpg;
        if(!ntpg.getImgData_not_process_tip(callback, parent))
        {return ;}
        // read
        v3dhandle curwin = callback.currentImageWindow();
        QString img_name=callback.getImageName(curwin);
        LandmarkList true_tip_TMI=Readdata_from_TMI_result(img_name);
        ntpg.tp.setTipList(true_tip_TMI);


        ntpg.setcallback(callback);
        ntpg.getImgData_not_process_tip(callback,parent);
//        NeuronTree temp_tree=ntpg.deal_not_match(ntpg.return_neurontree(),need_to_show);
        NeuronTree result_tree;


        ntpg.InputTreeCheck(ntpg.return_neurontree());
        result_tree=ntpg.TreeSegment(ntpg.return_neurontree());
        result_tree=ntpg.sort_neuron(result_tree);
        //TMI_simple_argu_input()
        if(!ntpg.TMI_simple_argu_input())
        {return ;}
        cout<<"begin to Binarization background_threshold:"<<ntpg.binary_threshold<<endl;
        ntpg.Binarization(ntpg.binary_threshold);
        result_tree=ntpg.pruningSwc_back_ground(result_tree);

        LandmarkList need_to_show=ntpg.find_leaf_need_prun(result_tree,true_tip_TMI);
//        callback.setLandmark(callback.currentImageWindow(),need_to_show);
//        callback.setSWC(callback.currentImageWindow(),result_tree);
//        return ;
        result_tree=ntpg.deal_not_match(result_tree,need_to_show);

//            writeSWC_file("result_tree_final.swc",result_tree);
//merge
        if(0)
        {
            result_tree=ntpg.mergeSwc_two_leafnode(result_tree);
        }

        writeSWC_file("C:\\Users\\204\\Desktop\\demo\\result_tree_final.swc",result_tree);

        if(1)
        {
            //for convinient  show
            result_tree=ntpg.TreeSegment(result_tree);
            cout<<"this neuronTree pruning "<<ntpg.pruning_branch_count<<" branch"<<endl;
        }
        if(1)
        {
            //pruningSwcSmallLength();
            ntpg.set_small_length(10);
            result_tree=ntpg.pruningSwc_small_length(result_tree);
        }
        writeSWC_file("C:\\Users\\204\\Desktop\\demo\\result_tree_final_convinient.swc",result_tree);
            cout<<"begin to setSWC"<<endl;
            callback.setSWC(callback.currentImageWindow(),result_tree);
//        LandmarkList need_to_show1=ntpg.find_leaf_need_prun(result_tree,true_tip_TMI);
//        need_to_show.append(true_tip_TMI);
//        callback.setLandmark(curwin,need_to_show1);

//        callback.setSWC(callback.currentImageWindow(),result_tree);
        //pruingSwc_one_leafnode_stop_before_tip
        return ;
    } else if(menu_name == tr("TMI_tip_process_old"))//
    {
        neurontreepruneorgraft ntpg;
        ntpg.getImgData_not_process_tip(callback, parent);
        // read
        v3dhandle curwin = callback.currentImageWindow();
        QString img_name=callback.getImageName(curwin);
        LandmarkList true_tip_TMI=Readdata_from_TMI_result(img_name);
        ntpg.tp.setTipList(true_tip_TMI);
        LandmarkList need_to_show=ntpg.find_leaf_need_prun(ntpg.return_neurontree(),true_tip_TMI);
//        need_to_show.append(ntpg.FromNeuronTreeGetLeaf(ntpg.return_neurontree()));
        ntpg.setcallback(callback);
        ntpg.getImgData_not_process_tip(callback,parent);
//        NeuronTree temp_tree=ntpg.deal_not_match(ntpg.return_neurontree(),need_to_show);
        NeuronTree result_tree;


        if(1)
        {
            result_tree=ntpg.tipBasedPruning_stop_before_tip_no_arg_input(need_to_show,ntpg.return_neurontree(),true_tip_TMI);
            callback.setSWC(callback.currentImageWindow(),result_tree);
        }
        else
        {
            ntpg.InputTreeCheck(ntpg.return_neurontree());
            result_tree=ntpg.TreeSegment(ntpg.return_neurontree());
            result_tree=ntpg.sort_neuron(result_tree);

            result_tree=ntpg.deal_not_match(result_tree,need_to_show);

//            writeSWC_file("result_tree_final.swc",result_tree);
//merge
            if(0)
            {
                result_tree=ntpg.mergeSwc_two_leafnode(result_tree);
            }

            writeSWC_file("C:\\Users\\204\\Desktop\\demo\\result_tree_final.swc",result_tree);
            cout<<"begin to setSWC"<<endl;
            cout<<"1244"<<endl;

            callback.setSWC(callback.currentImageWindow(),result_tree);
        }

//        LandmarkList need_to_show1=ntpg.find_leaf_need_prun(result_tree,true_tip_TMI);
//        need_to_show.append(true_tip_TMI);
        callback.setLandmark(curwin,need_to_show);
        return ;
    }
    //TMI_tip_process_show
    else if(menu_name == tr("TMI_tip_process_show"))//
    {
        neurontreepruneorgraft ntpg;
        ntpg.getImgData_not_process_tip(callback, parent);
        // read
        v3dhandle curwin = callback.currentImageWindow();
        QString img_name=callback.getImageName(curwin);
        LandmarkList true_tip_TMI=Readdata_from_TMI_result(img_name);
        ntpg.setShowMatchResult(true);
        LandmarkList need_to_show=ntpg.find_leaf_need_prun(ntpg.return_neurontree(),true_tip_TMI);
        need_to_show.append(true_tip_TMI);

        callback.setLandmark(curwin,need_to_show);
    }
    else if(menu_name == tr("input_swc_output_img"))//
    {
        cout<<11111<<endl;
        QList <V3dR_MainWindow *> list_3dviewer = callback.getListAll3DViewers();
        if (list_3dviewer.size() < 1)
        {
            v3d_msg("Please open  a SWC file from the main menu first! list_3dviewer");
            return ;
        }
        V3dR_MainWindow *surface_win = list_3dviewer[0];
        if (!surface_win){
            v3d_msg("Please open up a SWC file from the main menu first!");
            return ;
        }
        QList<NeuronTree> * mTreeList = callback.getHandleNeuronTrees_Any3DViewer(surface_win);
        if(mTreeList->size()<=0)
        {
            return ;
        }
        NeuronTree filetree=mTreeList->at(mTreeList->size()-1);
//        NeuronTree filetree=callback.getSWC(callback.currentImageWindow());
        cout<<"the filetree size is "<<filetree.listNeuron.size()<<endl;

        Image4DSimple* img1=callback.getImage(callback.currentImageWindow());
        V3DLONG sz[4];
        sz[0]=img1->getXDim();
        sz[1]=img1->getYDim();
        sz[2]=img1->getZDim();
        sz[3]=1;

        unsigned char * img_swc=NeuronTree2img(filetree,sz);
        if(img_swc==NULL)
        {
            cout<<"img_swc==NULL"<<endl;
            return;
        }

        simple_saveimage_wrapper(callback,"1234.tif",(unsigned char *)img_swc,sz,1);
        cout<<"save SWC img to file"<<endl;

    }//
    else if(menu_name == tr("Hackthon_tip_postProcess"))
    {
        neurontreepruneorgraft *ntpg;
        ntpg=new neurontreepruneorgraft;
        v3dhandle curwin = callback.currentImageWindow();
        QString img_name=callback.getImageName(curwin);

        if(!ntpg->getImgData_not_process_tip(callback, parent))
        {return ;}

        NeuronTree firstTree=ntpg->return_neurontree();
        ntpg->setcallback(callback);

        ntpg->InputTreeCheck(ntpg->return_neurontree());
//        NeuronTree
        NeuronTree result_tree=ntpg->sort_neuron(firstTree);
        result_tree=ntpg->TreeSegment(ntpg->return_neurontree());

        if(!ntpg->TMI_simple_argu_input())
        {return ;}
        ntpg->tp.getImgData(callback);
        if(!ntpg->tp.GUI_input_argu())
        {return ;}

        if(1)
        {
            cout<<"begin to Binarization background_threshold:"<<ntpg->binary_threshold<<endl;
            ntpg->Binarization(ntpg->binary_threshold);
        }
        else
        {
//            ntpg.setSegmentImg();
            //set segment img to binaryimg
        }

        if(0)
        {
            result_tree=ntpg->pruningSwc_back_ground(result_tree);

        }
        cout<<"begin whole_img"<<endl;
        ntpg->tp.whole_img(callback);
//        ntpg.tp.save_tp_as_temp(callback);
//        ntpg.tp.show_tipdetetor_img(callback);
        cout<<"show_tipdetetor_img"<<endl;
        ntpg->tp.save_tp_as_temp(callback);

        cout<<"begin Hackthon_process_tip"<<endl;
        ntpg->Hackathon_process_tip(result_tree);
//        ntpg.tp.show_tipdetetor_img(callback);
        callback.setLandmark(callback.currentImageWindow(),ntpg->need_to_show);


    }
    else if(menu_name == tr("about"))
	{
//        QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\331.swc";
//        NeuronTree temp_tree;
//        temp_tree=readSWC_file(filename);

//        v3dhandle curwin = callback.currentImageWindow();
//        callback.setSWC(curwin,temp_tree);
//		v3d_msg(tr("This is a test plugin, you can use it as a demo. Developed by fuhao, 2019-1-3"));
        ScoreInput* s;
//        cout<<"122"<<endl;
        s=new ScoreInput;;
        cout<<"1"<<endl;
        s->setV3DPluginCallback2(&callback);
        cout<<1<<endl;
        s->setWight(1,1,1);
        cout<<2<<endl;
        QStringList problem;
        problem.append("edfdfr");
        problem.append("edf454r");
        float sss[1],sss2[1],sss3[1];
        sss[0]=0.6;
        sss2[0]=0.5;
        sss3[0]=0.4;
        sss[1]=0.2;
        sss2[1]=0.7;
        sss3[1]=0.5;

        s->setData(2,problem,sss,sss2,sss3);
//        s->
        cout<<"42"<<endl;
        s->show();

	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}



