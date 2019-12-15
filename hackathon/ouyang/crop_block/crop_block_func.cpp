#include "crop_block_func.h"

#include "../../../../vaa3d_tools/hackathon/ouyang/ML_get_samples/tip_main.cpp"
#include "../../../../vaa3d_tools/hackathon/yimin/old_vr_codes/Vaa3DVR(ver 0.1)/vrminimal/matrix.h"
//#include "basic_surf_objs.cpp"
using namespace std;


void crop_bt_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir=outfiles.at(0);
    QString output_dir_axon=outfiles.at(1);//needed only when seperate axon and dendrite nodes
    QString output_apo;

    XYZ block_size=XYZ(100,100,100);

    int branch_or_tip_node=atoi(inparas.at(0));//1:branch;2:tip;3:both
    int tip_type=atoi(inparas.at(1));//1:seperate axon and dendrite;2:not seperate
    bool condition=false;
    if(branch_or_tip_node==3) condition=true;


    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();


    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_branch plugin\n");
    NeuronTree sorted_tree = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }
    if(!output_dir_axon.endsWith("/")){
        output_dir_axon = output_dir_axon+"/";
    }

    // Find branch and points
    QList<int> tip_point_list = get_tips(sorted_tree, false);
    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_point_list.size()<<endl;
    QList<int> branch_point_list = get_branch(sorted_tree);
    cout<<"=========================================================Number_of_branch_points\t"<<qPrintable(swc_file)<<"\t"<<branch_point_list.size()<<endl;
    QList<int> branch_and_tip_point_list;
    branch_and_tip_point_list.append(tip_point_list);
    branch_and_tip_point_list.append(branch_point_list);
    // Crop branch points-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;

    printf("welcome to use get_termial\n");
    if(branch_or_tip_node==1 || condition)
    {
        for(int i=0; i<branch_point_list.size(); i++){
            int tipnum=i;
            NeuronSWC node = sorted_tree.listNeuron.at(branch_point_list.at(i));
            if(node.type > 5) continue;

            block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
            crop_block.name = QString::number(i);
            XYZ tip=XYZ(node.x, node.y, node.z);
            // crop swc
            QString num_cnt=QString("%1").arg(i);
            QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);

            // create a tip-centered block
            if(tip_type == 2){  //get branch points from both dendrite and axon

                QString output_swc = output_dir+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 3){  //only get branch points from dendrite


                QString output_swc = output_dir+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 2){  //only get branch points from axon


                QString output_swc = output_dir_axon+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
        }
    }
    if(branch_or_tip_node==2 || condition)
    {
        for(int i=0; i<tip_point_list.size(); i++){
            int tipnum=i;
            NeuronSWC node = sorted_tree.listNeuron.at(tip_point_list.at(i));
            if(node.type > 5) continue;

            block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
            crop_block.name = QString::number(i);
            XYZ tip=XYZ(node.x, node.y, node.z);
            // crop swc
            QString num_cnt=QString("%1").arg(i);
            QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);

            // create a tip-centered block
            if(tip_type == 2){  //get tip points from both dendrite and axon

                QString output_swc = output_dir+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 3){  //only get tip points from dendrite


                QString output_swc = output_dir+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 2){  //only get tip points from axon


                QString output_swc = output_dir_axon+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
        }
    }
    return;
}

void crop_defined_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);//raw image
    QString swc_folder=infiles.at(1);//all swc of same brain folder
    QString original_swc_folder=infiles.at(2);//terafly swc for croppping swc block
    NeuronTree sorted_tree = readSWC_file(original_swc_folder);
    XYZ block_size=XYZ(50,50,50);//default 50x50x50
    int crop_all_in_input_swc=atoi(inparas.at(0));//1 for crop all swc nodes,0 is constrain
    if (inparas.size() >= 3 )
    {
        int tmp1=atoi(inparas.at(0));
        int tmp2=atoi(inparas.at(1));
        int tmp3=atoi(inparas.at(2));
        crop_all_in_input_swc=atoi(inparas.at(3));
        if (tmp1 != 0 && tmp2 != 0 && tmp3 != 0)
        {
            block_size=XYZ(tmp1,tmp2,tmp3);
            printf("block size is set to:[%dx%dx%d]",tmp1,tmp2,tmp3);
        }
        else
            v3d_msg("The parameter of block size is not valid, the program will use default value of [50x50x50]",0);
    }
    if(!swc_folder.endsWith("/")){
        swc_folder = swc_folder+"/";
    }


    QDir dir_tip(swc_folder);

    QStringList namelist=dir_tip.entryList();

    QStringList all_swc;
    for(int i=0;i<namelist.size();++i)
    {
        if(namelist.at(i).endsWith("swc")) all_swc.push_back(namelist.at(i));
        else if(namelist.at(i).endsWith("eswc")) all_swc.push_back(namelist.at(i));
    }

    QStringList list=all_swc.at(0).split("_");
    QString block_folder_name_a=list.at(0)+"_all_block_after_pruning"+"/";
    QString block_folder_name_b=list.at(0)+"_all_block_before_pruning"+"/";
    QString block_folder_name_refined=list.at(0)+"_all_block_refined"+"/";

    QString dir_output_block_a=swc_folder+block_folder_name_a;
    QString dir_output_block_b=swc_folder+block_folder_name_b;
    QString dir_output_block_refined=swc_folder+block_folder_name_refined;
//    QDir dir_block_a(dir_output_block_a);
//    if(!dir_block_a.exists() && crop_all_in_input_swc)
//    {
//        dir_block_a.mkdir(dir_output_block_a);
//    }
//    else
//    {
//        dir_block_a.setFilter(QDir::Files);
//        int fileCount = dir_block_a.count();
//        for (int i = 0; i < fileCount; i++)
//            dir_block_a.remove(dir_block_a[i]);
//    }

    if(crop_all_in_input_swc==1)
    {
        QDir dir_block_b(dir_output_block_b);
        if(!dir_block_b.exists() && crop_all_in_input_swc)
        {
            dir_block_b.mkdir(dir_output_block_b);
        }
        else
        {
            dir_block_b.setFilter(QDir::Files);
            int fileCount = dir_block_b.count();
            for (int i = 0; i < fileCount; i++)
                dir_block_b.remove(dir_block_b[i]);
        }
    }

    if(crop_all_in_input_swc==0)
    {
        QDir dir_block_refined(dir_output_block_refined);
        if(!dir_block_refined.exists())
        {
            dir_block_refined.mkdir(dir_output_block_refined);
        }
        else
        {
            dir_block_refined.setFilter(QDir::Files);
            int fileCount = dir_block_refined.count();
            for (int i = 0; i < fileCount; i++)
                dir_block_refined.remove(dir_block_refined[i]);
        }
    }
    // Crop branch points-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    printf("welcome to use get_termial\n");
    for(int i=0; i<all_swc.size(); i++)
    {
        cout<<"000000000000000000000000000000000000000"<<endl;
        QString flag1=all_swc.at(i).split(".")[0];
        NeuronTree pruned_swc=readSWC_file(swc_folder+all_swc.at(i));
        if (crop_all_in_input_swc){
            for(int j=0;j<pruned_swc.listNeuron.size();j++)
            {
                QString num_cnt=QString("%1").arg(j);
                NeuronSWC node = pruned_swc.listNeuron.at(j);
                QString r=QString("%1").arg(node.r);
                block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
                QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                crop_block.name = QString::number(i);

                QString output_image = flag1+"_r"+r+cordinates;
                //crop_img(image_file, crop_block, dir_output_block_b, callback,output_image);
                // crop swc
                QString output_swc = dir_output_block_b+output_image+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                // crop image
                {
                    //QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, dir_output_block_b, callback,output_image);
                }
            }
        }
        else {
            for(int j=0;j<pruned_swc.listNeuron.size();j++)
            {
                NeuronSWC node = pruned_swc.listNeuron.at(j);
                if(node.r>=8)
                {
                    QString num_cnt=QString("%1").arg(j);
                    QString r=QString("%1").arg(node.r);
                    block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
                    QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                    crop_block.name = QString::number(i);

                    QString output_image = flag1+"_r"+r+cordinates;
                    //crop_img(image_file, crop_block, dir_output_block_refined, callback,output_image);
                    // crop swc
                    QString output_swc = dir_output_block_refined+output_image+"_croped.eswc";
                    if(crop_swc_cuboid(pruned_swc, output_swc, crop_block))
                    // crop image
                    {
                        crop_img(image_file, crop_block, dir_output_block_refined, callback,output_image);
                    }



               }
            }
        }
    }
    return;
}



QList<int> get_branch(NeuronTree nt){
    // whether a node is a branch;
    QList<int> ori_branch_list;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)>1){ori_branch_list.append(i);}
    }

    cout<<"the number of branch nodes:"<<ori_branch_list.size()<<endl;

    return(ori_branch_list);
}



bool writeMarker_file(const QString & filename, LandmarkList & listMarker)
{
    FILE * fp = fopen(filename.toLatin1(), "wt");
    if (!fp)
    {
        return false;
    }

    fprintf(fp, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");

    for (int i=0;i<listMarker.size(); i++)
    {
        LocationSimple  p_pt;
        p_pt = listMarker[i];
        fprintf(fp, "%5.3f, %5.3f, %5.3f, %d, %d, %s, %s, %d,%d,%d\n",
                // 090617 RZC: marker file is 1-based
                p_pt.x,
                p_pt.y,
                p_pt.z,
                int(p_pt.radius), p_pt.shape,
                //qPrintable(p_pt.name), qPrintable(p_pt.comment),
                p_pt.color.r,p_pt.color.g,p_pt.color.b);
    }

    fclose(fp);
    return true;
}

int CheckSWC(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString inimg_file = callback.getPathTeraFly();
    QString file_name = inimg_file;
    QFileInfo info(file_name);

    //*************the default size of block;
    int BoxX = 90;
    int BoxY = 90;
    int BoxZ = 90;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    QGridLayout * layout = new QGridLayout();

    //*************set spinbox
    QSpinBox * block_X = new QSpinBox();
    block_X->setRange(0,5000000);      //???
    block_X->setValue(BoxX);  //???

    QSpinBox * block_Y = new QSpinBox();
    block_Y->setRange(0,5000000);      //???
    block_Y->setValue(BoxY);  //???

    QSpinBox * block_Z = new QSpinBox();
    block_Z->setRange(0,5000000);      //???
    block_Z->setValue(BoxZ);  //???

    layout->addWidget(new QLabel("X of block"),0,0);
    layout->addWidget(block_X, 0,1,1,5);

    layout->addWidget(new QLabel("Y of block"),1,0);
    layout->addWidget(block_Y, 1,1,1,5);

    layout->addWidget(new QLabel("Z of block"),2,0);
    layout->addWidget(block_Z, 2,1,1,5);


    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
    if(dialog->exec() != QDialog::Accepted)
    {
        if (dialog)
        {
                delete dialog;
                dialog=0;
                cout<<"delete dialog"<<endl;
        }
        return -1;
    }

    //***************get the dialog return values
    BoxX = block_X->value(); //   block的大小
    BoxY = block_Y->value();
    BoxZ = block_Z->value();

    if (dialog) {delete dialog; dialog=0;}

    V3DLONG *sz = new V3DLONG[4];
    callback.getDimTeraFly(inimg_file.toStdString(),sz);
    cout<<"sz[0]:"<<sz[0]<<" sz[1]:"<<sz[1]<<" sz[2]:"<<sz[2]<<endl;


    int num_along_x=ceil((float)sz[0]/BoxX);
    int num_along_y=ceil((float)sz[1]/BoxY);
    int num_along_z=ceil((float)sz[2]/BoxZ);
    int NumberOfBlock=num_along_x*num_along_y*num_along_z; // block的总个数
    cout<<"num along x is "<< num_along_x << endl;
    cout<<"num along y is "<< num_along_y << endl;
    cout<<"num along z is "<< num_along_z << endl;
    cout<<"NumberOfBlock is "<< NumberOfBlock << endl;

    std::set <TBlock> blcok_flag;  //set容器可以不重复地添加元素


    vector<MyMarker*> outMarker; outMarker.clear();

    QStringList swc_list;

    NeuronTree nt = callback.getSWCTeraFly();
    int swcInWindows=0;
    if(nt.listNeuron.size()!=0)
    {   swcInWindows=1;}
    else
    {
        swcInWindows=0;
        swc_list=QFileDialog::getOpenFileNames(0,"Choose ESWC or SWC","D:\\",QObject::tr("*.swc"));
        if(swc_list.isEmpty()){return 0;}
        int input_as_swc;//0 for eswc, 1 for swc
        if(swc_list.at(0).endsWith(".eswc"))
        {   input_as_swc=0;}
        else{   input_as_swc=1;}
        if(input_as_swc)
        {
            for(int i=0;i<swc_list.size();i++)
            {
                if(!swc_list.at(i).endsWith(".swc"))
                {
                    swc_list.removeAt(i);
                }
            }
            for(int i=0;i<swc_list.size();i++)
            {
                cout<<swc_list.at(i).toUtf8().data()<<endl;
            }

            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarker*> temp_swc;temp_swc.clear();
                readSWC_file(swc_list.at(i).toStdString(),temp_swc);
                cout<<"i :"<<i<<" temp_swc.size:"<<temp_swc.size()<<endl;
                for(int j=0;j<temp_swc.size();j++)
                {

                    TBlock temT; //定义临时存放标记的变量

                    temT.tx = ceil(temp_swc.at(j)->x/BoxX);
                    temT.ty = ceil(temp_swc.at(j)->y/BoxY);
                    temT.tz = ceil(temp_swc.at(j)->z/BoxZ);
                    temT.t = num_along_x*temT.ty*(temT.tz-1)+num_along_x*(temT.ty-1)+temT.tx; //当前点对应的block标号

                    if(temT.t >= NumberOfBlock){  cout<<"flase,exceed NumberOfBlock";}
                    else
                    {
                       blcok_flag.insert(temT); //把swc对应的blcok标号记下
                    }
                }
            }
        }
        else
        {
            //0 for eswc
            for(int i=0;i<swc_list.size();i++)
            {
                if(!swc_list.at(i).endsWith(".swc"))
                {
                    swc_list.removeAt(i);
                }
            }
            for(int i=0;i<swc_list.size();i++)
            {
                cout<<swc_list.at(i).toUtf8().data()<<endl;
            }

            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarkerX*> temp_eswc;temp_eswc.clear();
                readESWC_file(swc_list.at(i).toStdString(),temp_eswc);
                cout<<"i :"<<i<<" temp_eswc.size:"<<temp_eswc.size()<<endl;
                for(int j=0;j<temp_eswc.size();j++)
                {
                    TBlock temT; //定义临时存放标记的变量

                    temT.tx = ceil(temp_eswc.at(j)->x/BoxX);
                    temT.ty = ceil(temp_eswc.at(j)->y/BoxY);
                    temT.tz = ceil(temp_eswc.at(j)->z/BoxZ);
                    temT.t = num_along_x*temT.ty*(temT.tz-1)+num_along_x*(temT.ty-1)+temT.tx; //当前点对应的block标号

                    if(temT.t >= NumberOfBlock){  cout<<"flase,exceed NumberOfBlock";}
                    else
                    {
                       blcok_flag.insert(temT); //把swc对应的blcok标号记下
                    }
                }
            }

        }

     }


    // 找block的中心点
    vector<MyMarker> Block_Markerset;
    MyMarker Block_Marker;
    LandmarkList curlist;
    LocationSimple s;
    for(set<TBlock>::iterator it = blcok_flag.begin(); it!= blcok_flag.end(); it++)//遍历有swc的block
    {

        Block_Marker.x=((double)(*it).tx-0.5)*BoxX;
        Block_Marker.y=((double)(*it).ty-0.5)*BoxY;
        Block_Marker.z=((double)(*it).tz-0.5)*BoxZ;
        Block_Markerset.push_back(Block_Marker);
//        cout<< "Block_Marker.t  " << (*it).t  ;
//        cout<< "  Block_Marker.x  " << Block_Marker.x  ;
//        cout<< "  Block_Marker.y  " << Block_Marker.y  ;
//        cout<< "  Block_Marker.z  " << Block_Marker.z << endl ;
        s.x= Block_Marker.x;
        s.y= Block_Marker.y;
        s.z= Block_Marker.z;
        s.radius=1;
        s.color = random_rgba8(255);
        curlist << s;
    }

    v3d_msg(QString("save %1 markers").arg(curlist.size()),0);

    QString outimg_dir = "";
    QString default_name = info.baseName()+"_for_sub.marker";
    outimg_dir = QFileDialog::getExistingDirectory(0,
                                  "Choose a dir to save file " );
    QString outimg_file = outimg_dir + "\\" + default_name;
    writeMarker_file(outimg_file,curlist);


    return 1;
}
bool readSWC_file(string swc_file, vector<MyMarker*> & swc)
{//debug by guochanghao, fix for reading failure after 7 values
    ifstream ifs(swc_file.c_str());

    if(ifs.fail())
    {
        cout<<"open swc file : "<< swc_file <<" error"<<endl;
        return false;
    }

    map<int, MyMarker*> marker_map;
    map<MyMarker*, int> parid_map;
    string line;
    float ave_count_each_line_num = 0.0;
    int num_valid_line = 0;
    while(getline(ifs, line))
    {
        int count_each_line_num = 0;
        stringstream ss(line);
        string tmp;
        bool skipToNextLine = false;

        while(getline(ss,tmp,' '))
        {
            if(tmp.at(0) == '#'){skipToNextLine=true; break;}
//            cout<<tmp<<" ";
            count_each_line_num++;
        }
        if(skipToNextLine){continue;}
        num_valid_line++;
        ave_count_each_line_num+=count_each_line_num;
//        cout<<line<<": "<<count_each_line_num<<endl;
    }
    ave_count_each_line_num/=num_valid_line;
    cout<<"ave_count_each_line_num:"<<ave_count_each_line_num<<endl;
    if(ave_count_each_line_num-(int)ave_count_each_line_num==0)
    {qDebug("Each line has the same number of parameters."); ifs.close();}
    else
    {qDebug("ave_count_each_line_num is not integer."); ifs.close(); return false;}


    ifs.open(swc_file.c_str());
    while(ifs.good())
    {
        if(ifs.peek() == '#'){ifs.ignore(1000,'\n'); continue;}
        MyMarker *  marker = new MyMarker;
        int my_id = -1 ; ifs >> my_id;
        if(my_id == -1) break;
        if(marker_map.find(my_id) != marker_map.end())
        {
            cerr<<"Duplicate Node. This is a graph file. Please read it as a graph."<<endl; return false;
        }
        marker_map[my_id] = marker;

        ifs>> marker->type;
        ifs>> marker->x;
        ifs>> marker->y;
        ifs>> marker->z;
        ifs>> marker->radius;

        int par_id = -1; ifs >> par_id;
        parid_map[marker] = par_id;
        if(1)
        {//readSWC_file这个函数直接读swc文件有问题，后面seg_id等不读。跳过。
            int temp_for_;
            for(int i=ave_count_each_line_num-7; i>0; i--)
            {
                ifs>>temp_for_;
            }
        }
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
    return true;
}


