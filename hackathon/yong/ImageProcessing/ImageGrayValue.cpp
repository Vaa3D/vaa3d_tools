#include "ImageGrayValue.h"

bool ImageGrayValue(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return false;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return false;
    }

    unsigned char* data1d = p4DImage->getRawData();

    //V3DLONG c=1;
    V3DLONG in_sz[4];
    in_sz[0]=p4DImage->getXDim();
    in_sz[1]=p4DImage->getYDim();
    in_sz[2]=p4DImage->getZDim();
    in_sz[3]=p4DImage->getCDim();

    V3DLONG sz0 = in_sz[0];  //M
    V3DLONG sz1 = in_sz[1];  //N
    V3DLONG sz2 = in_sz[2];  //P
    V3DLONG sz01 = sz0 * sz1;
    V3DLONG tol_sz = sz01 * sz2;

    cout<<"**********creat gray(.txt) and coordinate(.marker) file**********"<<endl;
    QList<ImageMarker> signalCoordinate_list;
    vector<float> signalValue;
    vector<ImageSignal> vec_imagesignal;
    ImageMarker * p_pt=0;

    for(V3DLONG j = 0; j < tol_sz; j++)
    {
        if(data1d[j] > 18)
        {
            V3DLONG signal_loc = j;
            ImageMarker signal_coordinate(signal_loc % sz0, signal_loc % sz01 / sz0, signal_loc / sz01);
            signalValue.push_back(data1d[j]);
            signalCoordinate_list.push_back(signal_coordinate);
        }
    }
    for (int n=0;n<signalCoordinate_list.size(); n++)
    {
        ImageSignal imagesignal;
        p_pt = (ImageMarker *)(&(signalCoordinate_list.at(n)));
        imagesignal.x = p_pt->x;
        imagesignal.y = p_pt->y;
        imagesignal.z = p_pt->z;
        imagesignal.signal = signalValue[n];
        vec_imagesignal.push_back(imagesignal);
    }

    cout<<"vec_imagesignal = "<<vec_imagesignal.size()<<endl;

    export_TXT(vec_imagesignal,p4DImage->getFileName()+QString(".txt"));
    writeMarker_file(p4DImage->getFileName()+QString(".marker"),signalCoordinate_list);
    v3d_msg("checkout");

    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));

    NeuronTree nt;
    cout<<fileOpenName.toStdString()<<endl;
    nt=readSWC_file(fileOpenName);

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
    QList<NeuronSWC> branch_swclist;
    NeuronTree branch_ntlist;
    QList<ImageMarker> branch_markerlist;
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        //childNum = childs[nt.listNeuron[i].pn].size();
        childNum = childs[i].size();
        if(childNum == 2)
            branch_swclist.push_back(nt.listNeuron[i]);
    }
    cout<<"branch_swclist = "<<branch_swclist.size()<<endl;
    branch_markerlist = swc_to_marker(branch_swclist);
    branch_ntlist.listNeuron = branch_swclist;
    writeSWC_file(fileOpenName+QString("_branchpoints.swc"),branch_ntlist);
    writeMarker_file(fileOpenName+QString("_branchpoints.marker"),branch_markerlist);

    NeuronTree choose_ntlist1,choose_ntlist2;
    QList<NeuronSWC> choose_swclist1,choose_swclist2;
    QList<ImageMarker> choose_markerlist1,choose_markerlist2;
    for(int i = 0;i <branch_swclist.size();i++)
    {
        choose_swclist1.clear();
        choose_markerlist1.clear();
        choose_swclist2.clear();
        choose_markerlist2.clear();
        V3DLONG par1 = childs[branch_swclist[i].n-1][0];
        V3DLONG par2 = childs[branch_swclist[i].n-1][1];
        cout<<"par1 = "<<par1<<"      par2 = "<<par2<<endl;

        choosePoint(nt,childs,par1,par2,choose_swclist1,choose_swclist2,15);
        cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist1 = "<<choose_swclist1.size()<<"   choose_swclist2 = "<<choose_swclist2.size()<<endl;
        if(choose_swclist1.size()<5 || choose_swclist2.size()<5) continue;

        cout<<"**********creat gray(.txt) and coordinate(.marker) file**********"<<endl;
        QList<ImageMarker> signalCoordinate_list1,signalCoordinate_list2;
        vector<float> signalValue1,signalValue2;
        vector<ImageSignal> vec_imagesignal1,vec_imagesignal2;
        ImageMarker * p_pt1=0;
        ImageMarker * p_pt2=0;

        for(int k = 0;k < choose_swclist1.size();k++)
        {
            V3DLONG j= sz01*choose_swclist1[k].z +sz1*choose_swclist1[k].y + choose_swclist1[k].x;
            V3DLONG signal_loc1 = j;
            ImageMarker signal_coordinate1(signal_loc1 % sz0, signal_loc1 % sz01 / sz0, signal_loc1 / sz01);
            cout<<"j = "<<j<<"  signal_loc1 % sz0 = "<<signal_loc1 % sz0<<"   signal_loc1 % sz01 / sz0 = "<<signal_loc1 % sz01 / sz0<<"   signal_loc1 / sz01 = "<<signal_loc1 / sz01<<endl;
            signalValue1.push_back(int(data1d[j]));
            signalCoordinate_list1.push_back(signal_coordinate1);
        }
        for (int n=0;n<signalCoordinate_list1.size(); n++)
        {
            ImageSignal imagesignal1;
            p_pt1 = (ImageMarker *)(&(signalCoordinate_list1.at(n)));
            imagesignal1.x = p_pt1->x;
            imagesignal1.y = p_pt1->y;
            imagesignal1.z = p_pt1->z;
            imagesignal1.signal = signalValue1[n];
            vec_imagesignal1.push_back(imagesignal1);
        }

        for(int k = 0;k < choose_swclist2.size();k++)
        {
            V3DLONG l= sz01*choose_swclist2[k].z +sz1*choose_swclist2[k].y + choose_swclist2[k].x;
            V3DLONG signal_loc2 = l;
            ImageMarker signal_coordinate2(signal_loc2 % sz0, signal_loc2 % sz01 / sz0, signal_loc2 / sz01);
            signalValue2.push_back((data1d[l]));
            signalCoordinate_list2.push_back(signal_coordinate2);
        }
        for (int n=0;n<signalCoordinate_list2.size(); n++)
        {
            ImageSignal imagesignal2;
            p_pt2 = (ImageMarker *)(&(signalCoordinate_list2.at(n)));
            imagesignal2.x = p_pt2->x;
            imagesignal2.y = p_pt2->y;
            imagesignal2.z = p_pt2->z;
            imagesignal2.signal = signalValue2[n];
            vec_imagesignal2.push_back(imagesignal2);
        }

        cout<<"vec_imagesignal1 = "<<vec_imagesignal1.size()<<endl;
        cout<<"vec_imagesignal2 = "<<vec_imagesignal2.size()<<endl;

        export_TXT(vec_imagesignal1,fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.txt"));
        export_TXT(vec_imagesignal2,fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.txt"));

        writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_signalCoordinate1.marker"),signalCoordinate_list1);
        writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_signalCoordinate2.marker"),signalCoordinate_list2);

       choose_ntlist1.listNeuron = choose_swclist1;
       choose_ntlist2.listNeuron = choose_swclist2;
       choose_markerlist1 = swc_to_marker(choose_swclist1);
       choose_markerlist2 = swc_to_marker(choose_swclist2);
       writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.swc"),choose_ntlist1);
       writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.swc"),choose_ntlist2);
       writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.marker"),choose_markerlist1);
       writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.marker"),choose_markerlist2);

    }


    return true;
}


bool ImageGrayValue(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString inimage = infiles[0];
    cout<<"inimageName :"<<inimage.toStdString()<<endl;

    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int dataType;
    if(!simple_loadimage_wrapper(callback, inimage.toStdString().c_str(), data1d, in_sz, dataType))
    {
        cerr<<"load image "<<inimage.toStdString()<<" error!"<<endl;
        return false;
    }
    Image4DSimple* p4DImage = new Image4DSimple;
    p4DImage->setData((unsigned char*)data1d, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);

    in_sz[0]=p4DImage->getXDim();
    in_sz[1]=p4DImage->getYDim();
    in_sz[2]=p4DImage->getZDim();
    in_sz[3]=p4DImage->getCDim();

    V3DLONG sz0 = in_sz[0];  //M
    V3DLONG sz1 = in_sz[1];  //N
    V3DLONG sz2 = in_sz[2];  //P
    V3DLONG sz01 = sz0 * sz1;
    V3DLONG tol_sz = sz01 * sz2;

    cout<<"**********creat gray(.txt) and coordinate(.marker) file**********"<<endl;
    QList<ImageMarker> signalCoordinate_list;
    vector<float> signalValue;
    vector<ImageSignal> vec_imagesignal;
    ImageMarker * p_pt=0;
    for(V3DLONG j = 0; j < tol_sz; j++)
    {
//        if(data1d[j] > 18)
//        {
            V3DLONG signal_loc = j;
            ImageMarker signal_coordinate(signal_loc % sz0, signal_loc % sz01 / sz0, signal_loc / sz01);
            signalValue.push_back(data1d[j]);
            signalCoordinate_list.push_back(signal_coordinate);
       // }
    }
    for (int n=0;n<signalCoordinate_list.size(); n++)
    {
        ImageSignal imagesignal;
        p_pt = (ImageMarker *)(&(signalCoordinate_list.at(n)));
        imagesignal.x = p_pt->x;
        imagesignal.y = p_pt->y;
        imagesignal.z = p_pt->z;
        imagesignal.signal = signalValue[n];
        vec_imagesignal.push_back(imagesignal);
    }

    cout<<"vec_imagesignal = "<<vec_imagesignal.size()<<endl;
    export_TXT(vec_imagesignal,inimage+QString(".txt"));
    //writeMarker_file(p4DImage->getFileName()+QString(".marker"),signalCoordinate_list);

    QString fileOpenName = infiles[1];
    cout<<fileOpenName.toStdString()<<endl;
    NeuronTree nt;
    nt = readSWC_file(fileOpenName);
    cout<<"nt.size = "<<nt.listNeuron.size()<<endl;

    LandmarkList markerlist,tmp_markerlist,final_markerlist;
    NeuronTree nt_result;
    QList<NeuronSWC> nt_list,nt_result_list;

//    //mean shift by segments
//    Tree tree;
//    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
//    {
//        NeuronSWC S=nt.listNeuron[i];
//        Point1* pt= new Point1;
//        pt->x = S.x;
//        pt->y = S.y;
//        pt->z = S.z;
//        pt->r = S.r;
//        pt->type = S.type;
//        pt->p = NULL;
//        pt->childNum = 0;
//        tree.push_back(pt);

//    }
//    cout<<"tree.size="<<tree.size()<<endl;

//    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
//    {
//        if (nt.listNeuron[i].pn<0) continue;
//        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
//        tree[i]->p = tree[pid];
//        tree[pid]->childNum++;
//    }
//    //	printf("tree constructed.\n");
//    vector<Segment*> seg_list;
//    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
//    {
//        if (tree[i]->childNum!=1)//tip or branch point
//        {
//            Segment* seg = new Segment;
//            Point1* cur = tree[i];
//            do
//            {
//                seg->push_back(cur);
//                cur = cur->p;
//            }
//            while(cur && cur->childNum==1);
//            seg_list.push_back(seg);
//        }
//    }
//    cout<<"seg_list.size="<<seg_list.size()<<endl;
//    cout<<"tree="<<tree.size()<<endl;
//    for(V3DLONG i = 0;i < seg_list.size();i++)
//    {
//        for(V3DLONG j=0;j<seg_list[i]->size();j++)
//        {
//            markerlist.clear();
//            LocationSimple t;
//            Point1* pt= new Point1;
//            pt=seg_list[i]->at(j);
//            t.x = pt->x;
//            t.y = pt->y;
//            t.z = pt->z;
//            t.radius = pt->r;
//            t.color.a = 0;
//            t.color.b = 0;
//            t.color.g = 0;
//            t.color.r =0;
//            markerlist.push_back(t);
//            mean_shift_center(p4DImage->getRawData(),in_sz,markerlist,nt_list,0);
//            nt_result_list.append(nt_list);
//        }

//    }

//    //mean shift by points
//    vector<vector<V3DLONG> > childs;
//    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );

//    //getChildNum
//    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
//    {
//        V3DLONG par = nt.listNeuron[i].pn;
//        if (nt.listNeuron[i].pn<0) continue;
//        childs[nt.hashNeuron.value(par)].push_back(i);
//    }

//    for(V3DLONG i = 0;i < nt.listNeuron.size();i++)
//    {
//        markerlist.clear();
//        if (nt.listNeuron[i].pn<0)
//        {
//            nt_result_list.append(nt.listNeuron[i]);
//            continue;
//        }
//        int childNum = childs[i].size();
//        if(childNum == 0)
//        {
//            nt_result_list.append(nt.listNeuron[i]);
//            continue;
//        }

//        LocationSimple t;
//        t.x = nt.listNeuron[i].x;
//        t.y = nt.listNeuron[i].y;
//        t.z = nt.listNeuron[i].z;
//        t.radius = nt.listNeuron[i].r;
//        t.color.a = 0;
//        t.color.b = 0;
//        t.color.g = 0;
//        t.color.r =0;
//        markerlist.push_back(t);
//        V3DLONG n = nt.listNeuron[i].n;
//        V3DLONG pn = nt.listNeuron[i].pn;
//        mean_shift_center(p4DImage->getRawData(),in_sz,markerlist,nt_list,0,n,pn);
//        nt_result_list.append(nt_list);
//    }

//    cout<<"nt_result_list.size = "<<nt_result_list.size()<<endl;
//    nt_result.listNeuron = nt_result_list;
//    writeSWC_file(inimage+QString("_meanshift.swc"),nt_result);


//    v3d_msg("Checkout");

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
    QList<NeuronSWC> branch_swclist;
    NeuronTree branch_ntlist;
    QList<ImageMarker> branch_markerlist;
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        //childNum = childs[nt.listNeuron[i].pn].size();
        childNum = childs[i].size();
        if(childNum == 2)
            branch_swclist.push_back(nt.listNeuron[i]);
    }
    cout<<"branch_swclist = "<<branch_swclist.size()<<endl;
    branch_markerlist = swc_to_marker(branch_swclist);
    branch_ntlist.listNeuron = branch_swclist;
    //writeSWC_file(fileOpenName+QString("_branchpoints.swc"),branch_ntlist);
    //writeMarker_file(fileOpenName+QString("_branchpoints.marker"),branch_markerlist);

    NeuronTree choose_ntlist1,choose_ntlist2;
    QList<NeuronSWC> choose_swclist1,choose_swclist2;
    QList<ImageMarker> choose_markerlist1,choose_markerlist2;
    for(int i = 0;i <branch_swclist.size();i++)
    {
        choose_swclist1.clear();
        choose_markerlist1.clear();
        choose_swclist2.clear();
        choose_markerlist2.clear();
        V3DLONG par1 = childs[branch_swclist[i].n-1][0];
        V3DLONG par2 = childs[branch_swclist[i].n-1][1];
        cout<<"par1 = "<<par1<<"      par2 = "<<par2<<endl;

        choosePoint(nt,childs,par1,par2,choose_swclist1,choose_swclist2,15);
        cout<<"branchpoint "<<branch_swclist[i].n<<": choose_swclist1 = "<<choose_swclist1.size()<<"   choose_swclist2 = "<<choose_swclist2.size()<<endl;
        if(choose_swclist1.size()<5 || choose_swclist2.size()<5) continue;
        choose_ntlist1.listNeuron = choose_swclist1;
        choose_ntlist2.listNeuron = choose_swclist2;
        choose_markerlist1 = swc_to_marker(choose_swclist1);
        choose_markerlist2 = swc_to_marker(choose_swclist2);
        //writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.swc"),choose_ntlist1);
        //writeSWC_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.swc"),choose_ntlist2);
        //writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.marker"),choose_markerlist1);
        //writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.marker"),choose_markerlist2);

        cout<<"**********creat gray(.txt) and coordinate(.marker) file**********"<<endl;
        QList<ImageMarker> signalCoordinate_list1,signalCoordinate_list2;
        vector<V3DLONG> signalValue1,signalValue2;
        vector<ImageSignal> vec_imagesignal1,vec_imagesignal2;
        ImageMarker * p_pt1=0;
        ImageMarker * p_pt2=0;

        for(int k = 0;k < choose_swclist1.size();k++)
        {
            for(int s = -2;s <= 2;s++)
            {
                V3DLONG j= sz01*(choose_swclist1[k].z+s) +sz1*(choose_swclist1[k].y+s) + choose_swclist1[k].x+s;
                //V3DLONG signal_loc1 = j;
                //ImageMarker signal_coordinate1(signal_loc1 % sz0, signal_loc1 % sz01 / sz0, signal_loc1 / sz01);
                ImageMarker signal_coordinate1(choose_swclist1[k].x+s, choose_swclist1[k].y+s, choose_swclist1[k].z+s);
                signalValue1.push_back(int(data1d[j]));
                signalCoordinate_list1.push_back(signal_coordinate1);
            }
        }
        for (int n=0;n<signalCoordinate_list1.size(); n++)
        {
            ImageSignal imagesignal1;
            p_pt1 = (ImageMarker *)(&(signalCoordinate_list1.at(n)));
            imagesignal1.x = p_pt1->x;
            imagesignal1.y = p_pt1->y;
            imagesignal1.z = p_pt1->z;
            imagesignal1.signal = signalValue1[n];
            vec_imagesignal1.push_back(imagesignal1);
        }

        for(int k = 0;k < choose_swclist2.size();k++)
        {
            for(int s = -2;s <= 2;s++)
            {
                V3DLONG l= sz01*(choose_swclist2[k].z+s) +sz1*(choose_swclist2[k].y+s) + choose_swclist2[k].x+s;
                //V3DLONG signal_loc2 = l;
                //ImageMarker signal_coordinate2(signal_loc2 % sz0, signal_loc2 % sz01 / sz0, signal_loc2 / sz01);
                ImageMarker signal_coordinate2(choose_swclist2[k].x+s, choose_swclist2[k].y+s, choose_swclist2[k].z+s);
                signalValue2.push_back((data1d[l]));
                signalCoordinate_list2.push_back(signal_coordinate2);
            }
        }
        for (int n=0;n<signalCoordinate_list2.size(); n++)
        {
            ImageSignal imagesignal2;
            p_pt2 = (ImageMarker *)(&(signalCoordinate_list2.at(n)));
            imagesignal2.x = p_pt2->x;
            imagesignal2.y = p_pt2->y;
            imagesignal2.z = p_pt2->z;
            imagesignal2.signal = signalValue2[n];
            vec_imagesignal2.push_back(imagesignal2);
        }

        cout<<"vec_imagesignal1 = "<<vec_imagesignal1.size()<<endl;
        cout<<"vec_imagesignal2 = "<<vec_imagesignal2.size()<<endl;

        export_TXT(vec_imagesignal1,fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints1.txt"));
        export_TXT(vec_imagesignal2,fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_choosepoints2.txt"));
        writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_signalCoordinate1.marker"),signalCoordinate_list1);
        writeMarker_file(fileOpenName+QString("_branchpoint")+QString::number(branch_swclist[i].n)+QString("_signalCoordinate2.marker"),signalCoordinate_list2);


    }



    return true;
}



void mean_shift_center(unsigned char* data1d,V3DLONG sz_img[4], LandmarkList &LList,QList<NeuronSWC> &nt_list,int methodcode,V3DLONG n,V3DLONG pn)
//LandmarkList mean_shift_center(unsigned char* data1d,V3DLONG sz_img[4], LandmarkList &LList,QList<NeuronSWC> &nt_list,int methodcode)
{
    mean_shift_fun mean_shift_obj;
    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    int intype=1;
    //fun_obj.pushNewData<unsigned char>((unsigned char*)data1d, sz_img);
    if(intype==1)
    {
        mean_shift_obj.pushNewData<unsigned char>((unsigned char*)data1d, sz_img);
    }
    else if (intype == 2) //V3D_UINT16;
    {
        mean_shift_obj.pushNewData<unsigned short>((unsigned short*)data1d, sz_img);
        convert2UINT8_meanshift((unsigned short*)data1d, data1d, size_tmp);
    }
    else if(intype == 4) //V3D_FLOAT32;
    {
        mean_shift_obj.pushNewData<float>((float*)data1d, sz_img);
        convert2UINT8_meanshift((float*)data1d, data1d, size_tmp);
    }
    else
    {
        v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.",0);
        //return;
    }

    LandmarkList LList_new_center;
    vector<V3DLONG> poss_landmark;
     vector<float> mass_center;
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    //check parameter
    int windowradius=15;
    LList_new_center.clear();
    nt_list.clear();
    for (int j=0;j<poss_landmark.size();j++)
    {
        //append the original marker in LList_new_center
        LList[j].name="ori";
        LList[j].color.r=255; LList[j].color.g=LList[j].color.b=0;
        LList_new_center.append(LList.at(j));

        if (methodcode==2)
        {
            mass_center=mean_shift_obj.mean_shift_with_constraint(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1.0,mass_center[1]+1.0,mass_center[2]+1.0);
            tmp.color.r=0; tmp.color.g=170; tmp.color.b=255;
            tmp.name="ms_c";
            LList_new_center.append(tmp);
        }
        else
        {
            mass_center=mean_shift_obj.mean_shift_center(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            tmp.color.r=170; tmp.color.g=0; tmp.color.b=255;
            tmp.name="ms";
            LList_new_center.append(tmp);
        }
    }


    qDebug()<<"LList_new_center_size:"<<LList_new_center.size()<<endl;
    qDebug()<<"LList.size:"<<LList.size()<<endl;
    //return LList_new_center;

    for(V3DLONG k = 1; k < LList_new_center.size(); k++)
    {
        NeuronSWC cur;
        cur.n = n;
        cur.pn= pn;
        cur.x = 0.5*(LList_new_center.at(k).x+LList_new_center.at(k-1).x);
        cur.y = 0.5*(LList_new_center.at(k).y+LList_new_center.at(k-1).y);
        cur.z = 0.5*(LList_new_center.at(k).z+LList_new_center.at(k-1).z);
        cur.r = LList_new_center.at(k).radius;
        cur.type = 2;
        nt_list.push_back(cur);
    }
    cout<<"nt_list.size = "<<nt_list.size()<<endl;
    //system("mkdir meanshift_swc");

    //if (data1d!=0) {delete []data1d; data1d=0;}

}
bool export_TXT(vector<ImageSignal> &vec,QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    myfile<<"x\ty\tz\tgray"<<endl;
    ImageSignal * p_pt=0;
    for (int i=0;i<vec.size(); i++)
    {
        //then save
        p_pt = (ImageSignal *)(&(vec.at(i)));
        myfile << p_pt->x<<"\t"<<p_pt->y<<"\t"<<p_pt->z<<"\t"<<p_pt->signal<<endl;
    }

    file.close();
    cout<<"txt file "<<fileSaveName.toStdString()<<" has been generated, size: "<<vec.size()<<endl;
    return true;
}

