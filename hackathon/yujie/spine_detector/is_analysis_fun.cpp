#include "is_analysis_fun.h"
#include "common.h"

#define MAINWINNAME "ISquantifier_Proofread"
#define TRUNC_VIEW "ISquantifier_trunc"

is_analysis_fun::is_analysis_fun(V3DPluginCallback2 *cb, QStringList name_list)
{
    callback=cb;
    p_img1D=0;
    mask=0;
    image_trun=0;
    param.red_bgthr=90;
    param.green_bgthr=70;
    param.max_dis=40;
    param.min_voxel=4;
    param.halfwindowsize=30;
    basedir=name_list[2];
    swc_name=name_list[1];
    image_name=name_list[0];
}

void is_analysis_fun::run()
{
    obtain_mask();
    connected_components();
    stat_generate();
    visualize_image();
    create_proofread_panel();
}

void is_analysis_fun::obtain_mask()
{
    qDebug()<<"in obtain mask"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2];
//    V3DLONG count=0;
    V3DLONG page_size=sz_img[0]*sz_img[1]*sz_img[2];

    mask=new unsigned char [page_size*3];
    memset(mask,0,page_size*3);

    for(V3DLONG nid=0; nid<nt.listNeuron.size(); nid++){
        V3DLONG nx = nt.listNeuron.at(nid).x;
        V3DLONG ny = nt.listNeuron.at(nid).y;
        V3DLONG nz = nt.listNeuron.at(nid).z;
        if (nx<0 || nx>sz_img[0]-1 || ny<0 ||ny>sz_img[1]-1||nz<0 ||nz>sz_img[2]-1) //swc not in image
            continue;
        int disthr = nt.listNeuron.at(nid).radius+param.max_dis;
        for(V3DLONG x=MAX(nx-disthr,0); x<=MIN(nx+disthr,sz_img[0]-1); x++){
            for(V3DLONG y=MAX(ny-disthr,0); y<=MIN(ny+disthr,sz_img[1]-1); y++){
                for(V3DLONG z=MAX(nz-disthr,0); z<=MIN(nz+disthr,sz_img[2]-1); z++){
                    V3DLONG pos=xyz2pos(x,y,z,sz_img[0],sz_img[0]*sz_img[1]);
                    if (voxels_map.find(pos) != voxels_map.end())  //marked
                        continue;
                    mask[pos]=p_img1D[pos];
                    mask[pos+page_size]=p_img1D[pos+page_size];
                    if(p_img1D[pos]<param.red_bgthr){ //too dark, skip
                        continue;
                    }
                    if (p_img1D[pos+page_size]<param.green_bgthr)
                        continue;
                    voxels_map[pos]=true;

                }
            }
        }
    }

    mask_sz[0]=sz_img[0];
    mask_sz[1]=sz_img[1];
    mask_sz[2]=sz_img[2];
    mask_sz[3]=3;
    qDebug()<<"count:"<<voxels_map.size();

//    QString filename="mask1.v3draw";
//    simple_saveimage_wrapper(*callback,filename.toStdString().c_str(),mask,mask_sz,V3D_UINT8);
    if (p_img1D!=0)
    {
        delete[] p_img1D;
        p_img1D=0;
    }
}

void is_analysis_fun::connected_components()
{
    qDebug()<<"in connected components";
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    int label_id=1;

    map<V3DLONG, int> lookup;

    for(map<V3DLONG, bool>::iterator iter_map=voxels_map.begin(); iter_map!=voxels_map.end(); iter_map++)
    {
        if (voxels_map[iter_map->first]<=0)
            continue;
        //qDebug()<<iter_map->first;
        if (lookup[iter_map->first]>0)
            continue;

        vector<V3DLONG> seeds;
        seeds.clear();
        seeds.push_back(iter_map->first);
        lookup[iter_map->first]=label_id;
        int sid=0;
        while (sid<seeds.size())
        {
            //qDebug()<<"seeds size:"<<seeds.size();
            if (seeds[sid]-1>=0 && lookup[seeds[sid]-1]<=0 && voxels_map[seeds[sid]-1]>0)
            {
                seeds.push_back(seeds[sid]-1);
                lookup[seeds[sid]-1]=label_id;
            }
            if (seeds[sid]+1<size_page && lookup[seeds[sid]+1]<=0 && voxels_map[seeds[sid]+1]>0)
            {
                seeds.push_back(seeds[sid]+1);
                lookup[seeds[sid]+1]=label_id;
            }
            if (seeds[sid]-sz_img[0]>=0 && lookup[seeds[sid]-sz_img[0]]<=0 && voxels_map[seeds[sid]-sz_img[0]]>0)
            {
                seeds.push_back(seeds[sid]-sz_img[0]);
                lookup[seeds[sid]-sz_img[0]]=label_id;
            }
            if (seeds[sid]+sz_img[0]<size_page&& lookup[seeds[sid]+sz_img[0]]<=0 && voxels_map[seeds[sid]+sz_img[0]]>0)
            {
                seeds.push_back(seeds[sid]+sz_img[0]);
                lookup[seeds[sid]+sz_img[0]]=label_id;
            }
            if (seeds[sid]-z_offset>=0 && lookup[seeds[sid]-z_offset]<=0 && voxels_map[seeds[sid]-z_offset]>0)
            {
                seeds.push_back(seeds[sid]-z_offset);
                lookup[seeds[sid]-z_offset]=label_id;
            }
            if (seeds[sid]+z_offset<size_page && lookup[seeds[sid]+z_offset]<=0 && voxels_map[seeds[sid]+z_offset]>0)
            {
                seeds.push_back(seeds[sid]+z_offset);
                lookup[seeds[sid]+z_offset]=label_id;
            }
            sid++;
        }
        if (seeds.size()<param.min_voxel)
        {
            //need to make mask all zero??
            //qDebug()<<"small group:"<<seeds.size();
            continue;
        }

        GOI one_group(seeds,label_id);
        voxel_groups.push_back(one_group);
        label_id++;
        //qDebug()<<"i:"<<iter_map->first<<" seeds size:"<<seeds.size()<<label_id;
    }
    qDebug()<<"final groups size:"<<voxel_groups.size();
}

void is_analysis_fun::stat_generate()
{
    qDebug()<<"in stat_Generate";
    float sum_x,sum_y,sum_z;
    vector<V3DLONG> coord(3,0);
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    for (int i=0;i<voxel_groups.size();i++)
    {
        sum_x=sum_y=sum_z=0;
        qDebug()<<"i:"<<i<<" size:"<<voxel_groups[i].voxel.size();
        for (int j=0;j<voxel_groups[i].voxel.size();j++)
        {
            coord=pos2xyz(voxel_groups[i].voxel[j],sz_img[0],z_offset);
            sum_x+=coord[0];
            sum_y+=coord[1];
            sum_z+=coord[2];
        }
        LocationSimple tmp_lm;
        tmp_lm.x=sum_x/(float)voxel_groups[i].voxel.size()+1;
        tmp_lm.y=sum_y/(float)voxel_groups[i].voxel.size()+1;
        tmp_lm.z=sum_z/(float)voxel_groups[i].voxel.size()+1;
        tmp_lm.comments=QString::number(0).toStdString();

        int nearest_node=calc_nearest_node(nt,tmp_lm.x,tmp_lm.y,tmp_lm.z);
        voxel_groups[i].nearest_node=nearest_node;

        float min_dis=10e6;
        float dis;
        for (int j=0;j<voxel_groups[i].voxel.size();j++)
        {
            coord=pos2xyz(voxel_groups[i].voxel[j],sz_img[0],z_offset);
            dis=(coord[0]-tmp_lm.x-1)*(coord[0]-tmp_lm.x-1)+(coord[1]-tmp_lm.y-1)*(coord[1]-tmp_lm.y-1)
                +(coord[2]-tmp_lm.z-1)*(coord[2]-tmp_lm.z-1);
            if (dis<min_dis)
                min_dis=dis;
        }
        if (min_dis<nt.listNeuron[nearest_node].radius) //if on dendrite blue
        {
            voxel_groups[i].on_dendrite=true;
            tmp_lm.color.r=85;
            tmp_lm.color.g=170;
            tmp_lm.color.b=255;
        }
        else //if one spine yellow
        {
            tmp_lm.color.r=255;
            tmp_lm.color.g=255;
            tmp_lm.color.b=0;
        }
        LList_out.append(tmp_lm);
    }

}

void is_analysis_fun::visualize_image()
{
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    for (int i=0;i<voxel_groups.size();i++)
    {
        for (int j=0;j<voxel_groups[i].voxel.size();j++)
        {
            mask[voxel_groups[i].voxel[j]+2*size_page]=255;
        }
    }
    main_win =callback->newImageWindow(MAINWINNAME);
    Image4DSimple image4d;
    unsigned char *visual_copy=new unsigned char[size_page*3];
    memcpy(visual_copy,mask,size_page*3);
    image4d.setData(visual_copy,mask_sz[0],mask_sz[1],mask_sz[2],3,V3D_UINT8);
    callback->setImage(main_win,&image4d);
    callback->setLandmark(main_win,LList_out);
    callback->open3DWindow(main_win);
    callback->pushObjectIn3DWindow(main_win);
//    QString filename1="list.marker";
//    write_marker_file(filename1,LList_new);
//    QString filename="test1.v3draw";
//    simple_saveimage_wrapper(*callback,filename.toStdString().c_str(),mask,mask_sz,V3D_UINT8);
//    qDebug()<<"finished:"<<LList_new.size();
}

void is_analysis_fun::create_proofread_panel()
{
    qDebug()<<"in create proofread_panel";
    //mydialog=new QDialog();
    this->setWindowTitle("IS_proofreading");
    QGridLayout *layout2=new QGridLayout();

    QLabel *is_groups=new QLabel(QApplication::tr("IS groups"));
    layout2->addWidget(is_groups,0,0,1,6);
    QLabel *multiple=new QLabel(QApplication::tr("Press ctrl/shift to select multiple markers"));
    layout2->addWidget(multiple,1,0,1,6);

    list_markers=new QListWidget();
    layout2->addWidget(list_markers,2,0,8,4);
    list_markers->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list_markers->setFocusPolicy(Qt::NoFocus);

    QPushButton *btn_reject=new QPushButton("Reject");
    layout2->addWidget(btn_reject,2,4,1,2);
    QPushButton *set_loc_spine=new QPushButton("On spine");
    layout2->addWidget(set_loc_spine,3,4,1,2);
    QPushButton *set_loc_dendrite=new QPushButton("On dendrite");
    layout2->addWidget(set_loc_dendrite,4,4,1,2);

    QLabel *infobox=new QLabel("Info:");
    layout2->addWidget(infobox,13,0,1,2);
    edit_box = new QPlainTextEdit;
    edit_box->setReadOnly(true);
    edit_box->setFixedHeight(60);
    layout2->addWidget(edit_box,14,0,1,6);

    QPushButton *button_finish=new QPushButton("Finish proofreading");
    layout2->addWidget(button_finish,15,2,1,2);
    this->setLayout(layout2);

    for (int i=0;i<LList_out.size();i++)
    {
        if (voxel_groups[i].on_dendrite)
            list_markers->addItem("marker "+ QString::number(i+1)+ " on dendrite");
        else
            list_markers->addItem("marker "+ QString::number(i+1)+ " on spine");
    }
    list_markers->setCurrentItem(list_markers->item(0));
    //connect(list_markers,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(marker_doubleclicked()));
    connect(btn_reject,SIGNAL(clicked()),this,SLOT(reject_marker()));
    connect(set_loc_dendrite,SIGNAL(clicked()),this,SLOT(set_on_dendrite()));
    connect(set_loc_spine,SIGNAL(clicked()),this,SLOT(set_on_spine()));
    connect(button_finish,SIGNAL(clicked()),this,SLOT(finish_dialog()));
    this->setWindowTitle("IS_proofreading");

    qDebug()<<"mydialog finished building";
}

void is_analysis_fun::set_trunc_image()
{
    qDebug()<<"set_trun_Image_marker";
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    V3DLONG page_size=z_offset*sz_img[2];
    int mid=list_markers->currentRow();//start from zero--marker 1

    int halfwindowsize=param.halfwindowsize;
    //create bounding box
    V3DLONG x,y,z;
    x=LList_out[mid].x;
    y=LList_out[mid].y;
    z=LList_out[mid].z;

    if (x<halfwindowsize) sz[0]=x+halfwindowsize+1;
    else if (x+halfwindowsize>sz_img[0]) sz[0]=sz_img[0]-x+halfwindowsize;
    else sz[0]=2*halfwindowsize+1;

    if (y<halfwindowsize) sz[1]=y+halfwindowsize+1;
    else if (y+halfwindowsize>sz_img[1]) sz[1]=sz_img[1]-y+halfwindowsize;
    else sz[1]=2*halfwindowsize+1;

    if (z<halfwindowsize) sz[2]=z+halfwindowsize+1;
    else if (z+halfwindowsize>sz_img[2]) sz[2]=sz_img[2]-z+halfwindowsize;
    else sz[2]=2*halfwindowsize+1;

    sz[3]=3;

    if (image_trun!=0)
    {
        delete [] image_trun;
        image_trun=0;
    }
    image_trun=new unsigned char[sz[0]*sz[1]*sz[2]*sz[3]];
    memset(image_trun,0,sz[0]*sz[1]*sz[2]*sz[3]);
    qDebug()<<"image_trun allocated"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];
    //Store the area around the marker in image_trun
    x_start=MAX(0,x-halfwindowsize);
    y_start=MAX(0,y-halfwindowsize);
    z_start=MAX(0,z-halfwindowsize);
    x_end=MIN(sz_img[0],x+halfwindowsize+1);
    y_end=MIN(sz_img[1],y+halfwindowsize+1);
    z_end=MIN(sz_img[2],z+halfwindowsize+1);
    qDebug()<<"halfsize:"<<halfwindowsize<<" x,y,z:"<<x<<":"<<y<<":"<<z;
    qDebug()<<"sz0,sz1,sz2:"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];

    for (V3DLONG dx=x_start;dx<x_end;dx++){
        for (V3DLONG dy=y_start;dy<y_end;dy++){
            for (V3DLONG dz=z_start;dz<z_end;dz++){
                V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                V3DLONG pos1=xyz2pos(dx-x_start,dy-y_start,dz-z_start,sz[0],sz[0]*sz[1]);
                image_trun[pos1]=mask[pos];
                image_trun[pos1+sz[0]*sz[1]*sz[2]]=mask[pos+page_size];
                image_trun[pos1+2*sz[0]*sz[1]*sz[2]]=mask[pos+2*page_size];
            }
        }
    }
    qDebug()<<"finish setting up image";
}


void is_analysis_fun::adjust_LList_to_imagetrun()
{
    LList_adj.clear();
    for (int i=0;i<LList_out.size();i++)
    {
        LocationSimple tmp;
        tmp.x=LList_out[i].x-x_start;
        tmp.y=LList_out[i].y-y_start;
        tmp.z=LList_out[i].z-z_start;
        tmp.color.r=LList_out[i].color.r;
        tmp.color.g=LList_out[i].color.g;
        tmp.color.b=LList_out[i].color.b;
        tmp.comments=LList_out[i].comments;
        tmp.name=LList_out[i].name;
        LList_adj.append(tmp);
    }
    qDebug()<<"LList_adj set";
}


void is_analysis_fun::marker_doubleclicked()
{
    qDebug()<<"in marker_double clicked";
    open_main_win();
    set_trunc_image();
    adjust_LList_to_imagetrun();
    open_cur_win();
    int mid=list_markers->currentRow();

    TriviewControl * p_control = callback->getTriviewControl(curwin);
    p_control->setFocusLocation((long)LList_adj.at(mid).x-1,
                                (long)LList_adj.at(mid).y-1,(long)LList_adj.at(mid).z-1);
    p_control=callback->getTriviewControl(main_win);
    p_control->setFocusLocation((long)LList_out.at(mid).x-1,
                                (long)LList_out.at(mid).y-1,(long)LList_out.at(mid).z-1);

    unsigned char *reset_tmp =new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(reset_tmp,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(reset_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
}

void is_analysis_fun::open_main_win()
{
    qDebug()<<"open triview window";
    bool window_open_flag=false;
    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(callback->getImageName(list_triwin.at(i)).contains(MAINWINNAME))
        {
            window_open_flag=true;
            main_win=list_triwin[i];
            break;
        }
    }
    bool window_3d_flag=false;
    QList<V3dR_MainWindow*> main_3dwin=callback->getListAll3DViewers();
    for ( int i=0;i<main_3dwin.size();i++)
    {
        if (callback->getImageName(main_3dwin[i]).contains(MAINWINNAME))
        {
           window_3d_flag=true;
           break;
        }
    }
    if(!window_open_flag)
    {
        main_win = callback->newImageWindow(MAINWINNAME);
        unsigned char *image_input=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]];
        memcpy(image_input,mask,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        Image4DSimple image_main;
        image_main.setData(image_input,sz_img[0],sz_img[1],sz_img[2],sz_img[3],V3D_UINT8);
        callback->setImage(main_win,&image_main);
        callback->updateImageWindow(main_win);
        qDebug()<<"opening a new window!!";
        if (!window_3d_flag)
        {
            callback->open3DWindow(main_win);
            qDebug()<<"opening a 3d win";
        }

    }
}

void is_analysis_fun::open_cur_win()
{
    open_triview_window(TRUNC_VIEW,callback,image_trun,curwin,sz);
}

bool is_analysis_fun::check_cur_win()
{
    qDebug()<<"check curwin";
    bool window_open_flag=false;
    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(callback->getImageName(list_triwin.at(i)).contains(TRUNC_VIEW))
        {
            window_open_flag=true;
            curwin=list_triwin[i];
            break;
        }
    }
    return window_open_flag;
}

void is_analysis_fun::reject_marker()
{
    qDebug()<<"delete marker";
    open_main_win();
    bool cur_flag=false;
    //cur_flag=check_cur_win();

    //update landmarks color and status
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    qDebug()<<"this many selected:"<<sel_list.size();
    edit_box->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);

        if (cur_flag)
        {
            LList_adj[mid].color.g=LList_adj[mid].color.b=0;
            LList_adj[mid].color.r=255;
            LList_adj[mid].comments=QString::number(2).toStdString();
        }
        LList_out[mid].color.g=LList_out[mid].color.b=0;
        LList_out[mid].color.r=255;
        LList_out[mid].comments=QString::number(2).toStdString(); //rejected marker

        sel_list[i]->setText("marker " + QString::number(mid+1) + " rejected");
        edit_box->appendPlainText("marker: " + QString::number(mid+1) + " rejected");
    }

    qDebug()<<"before updating mainwin";
    //update mainwin
    callback->setLandmark(main_win,LList_out);
    callback->pushObjectIn3DWindow(main_win);
    qDebug()<<"1";
//    if (cur_flag)
//        callback->setLandmark(curwin,LList_adj);
//        callback->pushObjectIn3DWindow(curwin);
}

void is_analysis_fun::set_on_spine()
{
    qDebug()<<"set on spine";
    open_main_win();
    bool cur_flag=false;
    //cur_flag=check_cur_win();

    //update landmarks color and status
    //color for spine yellow 255 255 0
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    qDebug()<<"this many selected:"<<sel_list.size();
    edit_box->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);
//        if(cur_flag)
//        {
//            LList_adj[mid].color.g=LList_adj[mid].color.r=255;
//            LList_adj[mid].color.b=0;
//        }

        LList_out[mid].color.g=LList_out[mid].color.r=255;
        LList_out[mid].color.b=0;
        voxel_groups[mid].on_dendrite=false;

        sel_list[i]->setText("marker " + QString::number(mid+1) + " on spine");
        edit_box->appendPlainText("marker: " + QString::number(mid+1) + " are located on spine");
    }

    //update mainwin
    callback->setLandmark(main_win,LList_out);
//    callback->close3DWindow(main_win);
//    callback->open3DWindow(main_win);
    callback->pushObjectIn3DWindow(main_win);
//    if (cur_flag)
//        callback->setLandmark(curwin,LList_adj);
//        callback->pushObjectIn3DWindow(curwin);
}

void is_analysis_fun::set_on_dendrite()
{
    qDebug()<<"set on dendrite";
    open_main_win();
    //bool cur_flag=false;
    //cur_flag=check_cur_win();

    //update landmarks color and status
    //color for dendrite blue 85,170,255
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    qDebug()<<"this many selected:"<<sel_list.size();
    edit_box->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);
//        if (cur_flag)
//        {
//            LList_adj[mid].color.r=85;
//            LList_adj[mid].color.g=170;
//            LList_adj[mid].color.b=255;
//        }
        LList_out[mid].color.r=85;
        LList_out[mid].color.g=170;
        LList_out[mid].color.b=255;
        voxel_groups[mid].on_dendrite=true;
        sel_list[i]->setText("marker " + QString::number(mid+1) + " on dendrite");
        edit_box->appendPlainText("marker: " + QString::number(mid+1) + " are located on dendrite");
    }

    callback->setLandmark(main_win,LList_out);
    callback->pushObjectIn3DWindow(main_win);
//    if (cur_flag)
//        callback->setLandmark(curwin,LList_adj);
//        callback->pushObjectIn3DWindow(curwin);
}


void is_analysis_fun::finish_dialog()
{
    QMessageBox mybox;
    mybox.setText("Have you finished proofreading?");
    //mybox.setInformativeText("Only the accpeted markers will be kept");

    QPushButton *save_button = mybox.addButton(tr("Finish and save"),QMessageBox::ActionRole);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);

    mybox.setDefaultButton(save_button);
    mybox.exec();

     if (mybox.clickedButton() == save_button) {
         int final_landmarks_num;
         final_landmarks_num = finish_implement();
         QMessageBox mynewbox;
         QString info="After proofreading "+ QString::number(final_landmarks_num)+" IS were found\n";
         info+="The IS csv profile is saved at "+ csv_out_name;
         mynewbox.information(0,"IS quantifier",info,QMessageBox::Ok);
         return;

     } else if (mybox.clickedButton() == cancel_button) {
         return;
     }
     else if (mybox.clickedButton()== discard_button) {
         //need to close all image windows //check 3D window
         v3dhandleList list_triwin = callback->getImageWindowList();
         for(V3DLONG i=0; i<list_triwin.size(); i++){
             if(callback->getImageName(list_triwin.at(i)).contains(TRUNC_VIEW))
             {
                 callback->close3DWindow(list_triwin[i]);
             }
         }
         this->close();
         return;
     }
}

bool is_analysis_fun::csv_generate()
{
    //only get the image name
    QString baseName = image_name.section('/', -1);

    QString fileDefaultName=baseName.remove(".v3draw")+"_ISprofile.csv";
    QString completName=QDir(basedir).filePath(fileDefaultName);
    csv_out_name=QFileDialog::getSaveFileName(this,tr("Save csv"),completName,tr("Supported file: (*.csv)"),0,0);
    if (csv_out_name.isEmpty())
        return false;
    QFile file(csv_out_name);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&file);

    out<<"# "<<image_name<<endl;
    out<<"# "<<swc_name<<endl;
    out<<"# for location: on_dendrite=1; on_spine=0"<<endl;
    out<<"id,volume,nearest_node,location"<<endl;

    for (int kk=0;kk<voxel_groups.size();kk++)
    {
        QString tmp=QString::fromStdString(LList_out[kk].comments);
        if (tmp.contains("2"))  //delete
            continue;
        out<<kk+1<<","<<voxel_groups[kk].voxel.size()<<","<<voxel_groups[kk].nearest_node
          <<","<<voxel_groups[kk].on_dendrite<<endl;
    }
    file.close();
    qDebug()<<"voxel_group:"<<voxel_groups.size();
    return true;
}

int is_analysis_fun::finish_implement()
{
    open_main_win();
    //prepare csv
    if (!csv_generate())
        return 0;
    qDebug()<<"before closing window";
    //close dialog, close other 3D window
    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(!callback->getImageName(list_triwin.at(i)).contains(MAINWINNAME))
        {
            callback->close3DWindow(list_triwin[i]);
        }
    }
    this->close();
    qDebug()<<"closing dialog";
    //prep LList
    LandmarkList LList_new;
    for (int i=0;i<LList_out.size();i++)
    {
        QString tmp;
        tmp=QString::fromStdString(LList_out[i].comments);
        if (tmp.contains("2"))
            continue;
        else
            LList_new.append(LList_out[i]);
    }
    qDebug()<<"LList_new set:"<<LList_new.size();

    //open a final tri-view and 3D
    callback->setLandmark(main_win,LList_new);
    //callback->open3DWindow(main_win);
    callback->pushObjectIn3DWindow(main_win);

    return LList_new.size();

}
