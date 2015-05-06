#include "spine_detector_dialog.h"
#include "v3d_message.h"
#include <vector>
#include "string"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <basic_landmark.h>


spine_detector_dialog::spine_detector_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3];
    create();
    //detect_obj.bubbles();
}

void spine_detector_dialog::spine_find()
{
    QString filename="ray_length.csv";
    FILE * fp = fopen(filename.toAscii(), "w");
    LandmarkList ray_coord,all_ray_coord;
    qDebug()<<"number of neuron nodes:"<<neuron.listNeuron.size();
    for (int i=0;i<neuron.listNeuron.size()-1;i++)
    {
        vector<float> ray_length(32,0);
        ray_length=detect_obj.ray_shoot_feature(neuron.listNeuron[i],neuron.listNeuron[i+1],ray_coord);
        all_ray_coord.append(ray_coord);
        for (int j=0;j<32;j++)
        {
            if (j==31) fprintf(fp,"%.2f\n",ray_length[j]);
          else
          fprintf(fp,"%.2f,",ray_length[j]);
        }
    }
    fclose(fp);

//    filename="ray_coord.csv";
//    fp = fopen(filename.toAscii(), "w");
//    qDebug()<<"size of all_ray_coord:"<<all_ray_coord.size();
//    for (int i=0;i<all_ray_coord.size();i++)
//    {
//        fprintf(fp,"%.2f,%.2f,%.2f\n",all_ray_coord.at(i).x,all_ray_coord.at(i).y,all_ray_coord.at(i).z);
//    }
//    fclose(fp);

    filename="all_ray_coord.swc";
    fp=fopen(filename.toAscii(),"wt");
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    for (int j=0;j<all_ray_coord.size();j++)
    {
        fprintf(fp, "%d %d %.lf %.lf %.lf %.1f %d\n",j+1,1,all_ray_coord.at(j).x,all_ray_coord.at(j).y,
                all_ray_coord.at(j).z,1.0,j+1);
    }
    fclose(fp);

    return;
}

void spine_detector_dialog::create()
{
    QGridLayout *mygridLayout = new QGridLayout;
    QLabel* label_load = new QLabel(QObject::tr("Load Image:"));
    mygridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit();
    edit_load->setText(""); edit_load->setReadOnly(true);
    mygridLayout->addWidget(edit_load,0,1,1,4);
    QPushButton *btn_load = new QPushButton("...");
    mygridLayout->addWidget(btn_load,0,5,1,1);

    QLabel* label_swc = new QLabel(QObject::tr("Load swc file:"));
    mygridLayout->addWidget(label_swc,1,0,1,1);
    edit_swc = new QLineEdit;
    edit_swc->setText(""); edit_swc->setReadOnly(true);
    mygridLayout->addWidget(edit_swc,1,1,1,4);
    QPushButton *btn_output = new QPushButton("...");
    mygridLayout->addWidget(btn_output,1,5,1,1);

    QPushButton *ok     = new QPushButton("OK");
    QPushButton *cancel = new QPushButton("Cancel");

    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(ok);
    hlayout->addWidget(cancel);

    mygridLayout->addLayout(hlayout,2,1,1,4,Qt::AlignHCenter);
    setLayout(mygridLayout);

    connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(loadImage()));
    connect(btn_output, SIGNAL(clicked()), this, SLOT(load_swc()));
    connect(this,SIGNAL(finished(int)),this,SLOT(dialoguefinish(int)));
}

void spine_detector_dialog::dialoguefinish(int)
{
    if (this->result()==QDialog::Accepted)
    {
        //preprosessing();
        distance_measure();
        //cell_segment();
//        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
//        V3DLONG page_size=sz_img[0]*sz_img[1]*sz_img[2];
//        V3DLONG y_offset=sz_img[0];
//        V3DLONG z_offset=sz_img[0]*sz_img[1];
////        unsigned char *all=new unsigned char [page_size];
////        memset(all,0,page_size);
////        V3DLONG number_voxels=detect_obj.extract_nonsphere(all); //all=1 skel region
//        //detect_obj.bubbles_no_gsdt(all);
//        //qDebug()<<"number of voxels:"<<number_voxels;

//        vector<V3DLONG> x_all,y_all,z_all;
//        unsigned char *image_out=new unsigned char[size_tmp];
//        memcpy(image_out,image1Dc_in,size_tmp);
//        //memset(image_out,0,size_tmp);
//        unsigned char *mask=new unsigned char[page_size];
//        memset(mask,0,page_size); //mask=1 skel, mask=0; others
//        long *label = new long[page_size];
//        memset(label,-1, page_size*sizeof(long));

//        qDebug()<<"neuron size:"<<neuron.listNeuron.size();
//        for (V3DLONG i=0;i<neuron.listNeuron.size();i++)
//        {
//            V3DLONG x=neuron.listNeuron.at(i).x;
//            V3DLONG y=neuron.listNeuron.at(i).y;
//            V3DLONG z=neuron.listNeuron.at(i).z;
//            V3DLONG r=neuron.listNeuron.at(i).r;
//            V3DLONG pos=xyz2pos(x,y,z,y_offset,z_offset);
//            int size=detect_obj.extract(x_all,y_all,z_all,pos,r,0.8);
//            float r_grow=detect_obj.skel_node_radius(pos);
//            qDebug()<<"i:"<<i<<" size"<<size;
//            //mask the growed areas
//            for(V3DLONG dx=MAX(x-r,0); dx<=MIN(sz_img[0],x+r); dx++){
//                for(V3DLONG dy=MAX(y-r,0); dy<=MIN(sz_img[1],y+r); dy++){
//                    for(V3DLONG dz=MAX(z-r,0); dz<=MIN(sz_img[2]-1,z+r); dz++){
//                        pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
//                        if (mask[pos]==1) continue; //mask>0 skeleton, need to delete
//                        double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
//                        if (tmp>r*r) continue;
//                        mask[pos]=1;
//                    }
//                }
//            }

//            for (int i=0;i<x_all.size();i++)
//            {
//                V3DLONG pos=xyz2pos(x_all[i],y_all[i],z_all[i],y_offset,z_offset);
//                mask[pos]=1;
//            }
//        }

//        //get the boundary points
////        vector<V3DLONG> neighbor(6,0);
////        unsigned char *mask1D=new unsigned char[page_size];
////        memset(mask1D,0,page_size);
////        for (V3DLONG i=0;i<page_size;i++)
////        {
////            if (image1Dc_in[i]>=80)
////            {
////                mask1D[i]=1;
////            }
////        }
////        unsigned char *boundary=new unsigned char[page_size];
////        memset(boundary,0,page_size);
////        for (V3DLONG i=0;i<page_size;i++)
////        {
////            if (mask1D[i]<=0) continue;
////            int sum_tmp=0; int count=0;
////            if (i-1>=0) {neighbor[0]=i-1; sum_tmp+=mask1D[neighbor[0]]; count++;}
////            if (i+1<page_size) {neighbor[1]=i+1; sum_tmp+=mask1D[neighbor[1]]; count++;}
////            if (i-y_offset>=0) {neighbor[2]=i-y_offset;sum_tmp+=mask1D[neighbor[2]]; count++;}
////            if (i+y_offset<page_size) {neighbor[3]=i+y_offset; sum_tmp+=mask1D[neighbor[3]]; count++;}
////            if (i-z_offset>=0) {neighbor[4]=i-z_offset; sum_tmp+=mask1D[neighbor[4]]; count++;}
////            if (i+z_offset<page_size) {neighbor[5]=i+z_offset; sum_tmp+=mask1D[neighbor[5]]; count++;}
////            if (sum_tmp<count)
////            {
////               boundary[i]=1;
////            }
////        }

//        //qDebug()<<"attached bubbles grown";
//        int bg_thr=80;

//        unsigned char *bubble_ind=new unsigned char [page_size];
//        memset(bubble_ind,0,page_size);
//        int rgb[3];
////        //grow directly without finding bubbles first
////        int count02=0;
////        for (V3DLONG i=0;i<page_size;i++)
////        {
////            if (all[i]>0 && mask[i]<=0)
////            {
////                int size=detect_obj.extract_nonoverlap(mask,label,x_all,y_all,z_all,i,0.4,i+1);
////                qDebug()<<"i:"<<i<<" size"<<x_all.size();
////                if (size<=5) {count02++; continue;}
////                GetColorRGB(rgb,i);
////                for (V3DLONG j=0;j<x_all.size();j++)
////                {

////                    V3DLONG pos=xyz2pos(x_all[j],y_all[j],z_all[j],y_offset,z_offset);

////                    image_out[pos]=rgb[0];
////                    image_out[pos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
////                    image_out[pos+2*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[2];
////                }
////            }
////        }

//        vector<V3DLONG> bubbles=detect_obj.bubbles_no_gsdt(mask);
//        qDebug()<<"bubble size:"<<bubbles.size();
//        int count25=0;int count02=0;
//        unsigned short *label_out=new unsigned short[page_size];
//        memset(label_out,0,page_size);
//        int marker=1;
//        for (V3DLONG i=0;i<bubbles.size();i++)
//        {
//            int size=detect_obj.extract_nonoverlap(mask,label,x_all,y_all,z_all,bubbles[i],0.3,marker);
//            //qDebug()<<"i:"<<i<<" size"<<x_all.size();
//            if (size<=1) {count02++; continue;}
//            //GetColorRGB(rgb,marker);
//            marker++;
//            for (V3DLONG j=0;j<x_all.size();j++)
//            {

//                V3DLONG pos=xyz2pos(x_all[j],y_all[j],z_all[j],y_offset,z_offset);
//                label_out[pos]=label[pos];
////                image_out[pos]=rgb[0];
////                image_out[pos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
//                image_out[pos+2*sz_img[0]*sz_img[1]*sz_img[2]]=255; //rgb[2];
//            }

//        }
//        qDebug()<<"max label:"<<marker;
////        for (V3DLONG j=0;j<page_size;j++)
////        {
////            if (mask[j]>0)
////            {
////                image_out[j]=0;
////                image_out[j+1*sz_img[0]*sz_img[1]*sz_img[2]]=0;
////                image_out[j+2*sz_img[0]*sz_img[1]*sz_img[2]]=0;
////            }
////        }

//        qDebug()<<"image_out set";

//        Image4DSimple image4d,image_label;
//        image4d.setData(image_out,sz_img[0],sz_img[1],sz_img[2],sz_img[3],V3D_UINT8);
//        //image_label.setData((unsigned char*)label_out,sz_img[0],sz_img[1],sz_img[2],1,V3D_UINT16);
//        //v3dhandle v3dhandle_label=callback->newImageWindow("label");
//        v3dhandle v3dhandle_color=callback->newImageWindow("test");
//        callback->setImage(v3dhandle_color, &image4d);
//        //callback->setImage(v3dhandle_label,&image_label);
//        callback->updateImageWindow(v3dhandle_color);
//        callback->open3DWindow(v3dhandle_color);

    }

    else{
        //reset image_data and neuron
        if (image1Dc_in!=0){
            delete [] image1Dc_in;
            image1Dc_in=0;
        }
    }
}

void spine_detector_dialog::loadMask()
{
    QString fileName;
    mask_data=0;
    //need to check if the size is the same as image
    fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
             QDir::currentPath(),QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    if (!fileName.isEmpty())
    {
        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), mask_data, mask_sz, mask_intype))
        {
            QMessageBox::information(0,"","load image "+fileName+" error!");
            return;
        }

        if (mask_sz[3]>3)
        {
            mask_sz[3]=3;
            QMessageBox::information(0,"","More than 3 channels were loaded."
                                     "The first 3 channel will be applied for analysis.");
            return;
        }

        V3DLONG size_tmp=mask_sz[0]*mask_sz[1]*mask_sz[2]*mask_sz[3];
        if(mask_intype!=1)
        {
            if (mask_intype == 2) //V3D_UINT16;
            {
                convert2UINT8((unsigned short*)mask_data, mask_data, size_tmp);
            }
            else if(mask_intype == 4) //V3D_FLOAT32;
            {
                convert2UINT8((float*)mask_data, mask_data, size_tmp);
            }
            else
            {
                QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
                return;
            }
        }
    }
    edit_swc->setText(fileName);
}

void spine_detector_dialog::cell_segment()
{
    qDebug()<<"mask data size:"<<mask_sz[0]<<":"<<mask_sz[3];
    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
//    unsigned char *leftover=new unsigned char[size_tmp];
//    V3DLONG count=0;
//    for (int i=0;i<size_tmp;i++)
//    {
//        leftover[i]=image1Dc_in[i]-mask_data[i];
//        if (leftover[i]>0) count++;
//    }
//    qDebug()<<"leftover set"<<count;
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];

    //detect_obj.pushNewData<unsigned char>((unsigned char*)leftover, sz_img);
    //qDebug()<<"data pushed to detect_obj";
    vector<MyMarker> bubbles=detect_obj.bubbles();

    vector<int> neuron_idx;
    LandmarkList far_points;
    vector<V3DLONG> far_points_ind;
    far_points_ind=detect_obj.distance_to_skel(bubbles,neuron,neuron_idx,far_points);
    qDebug()<<"far points size:"<<far_points.size();
//    unsigned char *image1Dc_out=new unsigned char[z_offset*sz_img[2]*3];
//    memset(image1Dc_out,0,z_offset*sz_img[2]*3*sizeof(unsigned char));
//    //start shape_extraction
//    vector<V3DLONG> x_all,y_all,z_all;
//    LandmarkList LList;
//    short *label = new short[z_offset*sz_img[2]];
//    memset(label,-1, z_offset*sz_img[2]*sizeof(short));
//    for (int i=0;i<bubbles.size();i++)
//    {
//        V3DLONG pos=xyz2pos(bubbles.at(i).x,bubbles.at(i).y,
//                            bubbles.at(i).z,y_offset,z_offset);
//        int size=detect_obj.extract(x_all,y_all,z_all,pos,i+1,label);
//        qDebug()<<"shape extract "<<i<<":"<<size;
//        if (size<=1) continue;
//        LocationSimple tmp;
//        tmp.x=bubbles.at(i).x+1;
//        tmp.y=bubbles.at(i).y+1;
//        tmp.z=bubbles.at(i).z+1;
//        LList.append(tmp);
//        int rgb[3];
//        GetColorRGB(rgb,i);
//        V3DLONG finalpos;
//        for (int j=0;j<x_all.size();j++)
//        {
//            finalpos=xyz2pos(x_all[j],y_all[j],z_all[j],y_offset,z_offset);
//            image1Dc_out[finalpos]=rgb[0];
//            image1Dc_out[finalpos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
//            image1Dc_out[finalpos+2*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[2];
//        }
//    }
//    qDebug()<<"LList size:"<<LList.size();
    Image4DSimple image4d;
    //unsigned char* image1Dc_input=memory_allocate_uchar1D(z_offset*sz_img[2]*3);
    //memcpy(image1Dc_input, image1Dc_out, z_offset*sz_img[2]*3*sizeof(unsigned char));
    image4d.setData(image1Dc_in,sz_img[0], sz_img[1], sz_img[2],3,V3D_UINT8);

    v3dhandle v3dhandle_color=callback->newImageWindow("test");
    callback->setImage(v3dhandle_color, &image4d);
    callback->setLandmark(v3dhandle_color, far_points);
    callback->updateImageWindow(v3dhandle_color);
    callback->pushObjectIn3DWindow(v3dhandle_color);

}

void spine_detector_dialog::preprosessing()
{
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    int search_length=40; int bg_thr=80;
    vector<V3DLONG> coord;
    unsigned char *mask=new unsigned char[size_page];
    memset(mask,0,size_page);
    //mask the interested area;
    for (V3DLONG i=0;i<neuron.listNeuron.size();i++)
    {
        float s_x=neuron.listNeuron.at(i).x;
        float s_y=neuron.listNeuron.at(i).y;
        float s_z=neuron.listNeuron.at(i).z;
        float radius=neuron.listNeuron[i].radius;
        float radius_new=radius+search_length;
        for(V3DLONG dx=MAX(s_x-radius_new,0); dx<=MIN(sz_img[0]-1,s_x+radius_new); dx++){
            for(V3DLONG dy=MAX(s_y-radius_new,0); dy<=MIN(sz_img[1]-1,s_y+radius_new); dy++){
                for(V3DLONG dz=MAX(s_z-radius_new,0); dz<=MIN(sz_img[2]-1,s_z+radius_new); dz++){
                    V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                        mask[pos]=1;
                }
            }
        }
    }

    for (V3DLONG i=0;i<size_page;i++)
    {
        if (mask[i]<=0) continue;
        coord=pos2xyz(i,y_offset,z_offset);
        float sum=0; float count=0;
        V3DLONG x=coord[0]; V3DLONG y=coord[1]; V3DLONG z=coord[2];
        for(V3DLONG dx=MAX(x-4,0); dx<=MIN(sz_img[0]-1,x+4); dx++){
            for(V3DLONG dy=MAX(y-4,0); dy<=MIN(sz_img[1]-1,y+4); dy++){
                for(V3DLONG dz=MAX(z-4,0); dz<=MIN(sz_img[2]-1,z+4); dz++){
                    V3DLONG pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    sum+=image1Dc_in[pos1];
                    count++;
                }
            }
        }
        float ave=sum/count;
        if (image1Dc_in[i]<ave) mask[i]=0;
    }

    for (V3DLONG i=0;i<size_page;i++)
    {
        if (mask[i]<=0) continue;
        coord=pos2xyz(i,y_offset,z_offset);
        int count=0;
        V3DLONG x=coord[0]; V3DLONG y=coord[1]; V3DLONG z=coord[2];
        for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_img[0]-1,x+1); dx++){
            for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_img[1]-1,y+1); dy++){
                for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_img[2]-1,z+1); dz++){
                    V3DLONG pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if (mask[pos1]>0) count++;
                }
            }
        }
        if (count<=0) mask[i]=0;
    }
    unsigned char *image_out=new unsigned char[size_page];
    for (V3DLONG i=0;i<size_page;i++)
    {
        if (mask[i]>0)
            image_out[i]=image1Dc_in[i];
    }

    Image4DSimple image4d;
    image4d.setData(image_out,sz_img[0],sz_img[1],sz_img[2],1,V3D_UINT8);
    v3dhandle main=callback->newImageWindow("test");
    callback->setImage(main,&image4d);
    callback->updateImageWindow(main);
    callback->open3DWindow(main);
}


void spine_detector_dialog::distance_measure()
{
    //qDebug()<<"in distance_measure";
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    float *bound_box=new float [size_page];
    float inf=0.001;
    for (V3DLONG i=0;i<size_page;i++)
        bound_box[i]=-3.;
    int *neuron_id=new int[size_page];
    for (V3DLONG i=0;i<size_page;i++)
        neuron_id[i]=-1;
    //unvisited -3
    //within neuron node radius bound_box=0;
    //between radius and radius_new, if not visited, bound_box=first dis value
    //if visited bound_box=smallest dis value
    int search_length=40; int bg_thr=120;
    for (V3DLONG i=0;i<neuron.listNeuron.size();i++)
    {
        float s_x=neuron.listNeuron.at(i).x;
        float s_y=neuron.listNeuron.at(i).y;
        float s_z=neuron.listNeuron.at(i).z;
        float radius=neuron.listNeuron[i].radius;
        float radius_new=radius+search_length;
        for(V3DLONG dx=MAX(s_x-radius_new,0); dx<=MIN(sz_img[0]-1,s_x+radius_new); dx++){
            for(V3DLONG dy=MAX(s_y-radius_new,0); dy<=MIN(sz_img[1]-1,s_y+radius_new); dy++){
                for(V3DLONG dz=MAX(s_z-radius_new,0); dz<=MIN(sz_img[2]-1,s_z+radius_new); dz++){
                    V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if (ABS(bound_box[pos])<inf) continue; //bound_box[pos]==0
                    if (image1Dc_in[pos]<bg_thr) {bound_box[pos]=-1;neuron_id[pos]=-1;continue;}
                    float tmp=(dx-s_x)*(dx-s_x)+(dy-s_y)*(dy-s_y)
                         +(dz-s_z)*(dz-s_z);
                    //if (tmp>radius_new*radius_new) {continue;}
                    if (tmp<=radius*radius)
                    {
                        bound_box[pos]=0;
                        neuron_id[pos]=-1;
                        continue;
                    }
                    if (ABS(bound_box[pos]+3)<inf) //unvisited
                    {
                        bound_box[pos]=sqrt(tmp)-radius;
                        neuron_id[pos]=i;
                    }
                    else if (bound_box[pos]>0 &&(sqrt(tmp)-radius)<bound_box[pos])
                    {
                        bound_box[pos]=sqrt(tmp)-radius;
                        neuron_id[pos]=i;
                    }

                }
            }
        }
    }

    unsigned char *visual_data=new unsigned char [size_page*3];
    memcpy(visual_data,image1Dc_in,size_page);  
    qDebug()<<"bound_box finished:"<<sz_img[3];
    //adaptive_thres(neuron_id,bound_box);
//    for (V3DLONG i=0;i<size_page;i++)
//    {
//        if (bound_box[i]>=0)
//            visual_data[i+size_page]=255;
//    }

    //search each bound_box==-2 points for extremes
    //1)farthest dis among 26 nbs
    //2)more nbs in 5*5*5 than 3*3*3
    vector<V3DLONG> coord(3,0);
    multimap<float,V3DLONG> order_map;
    for (V3DLONG i=0;i<size_page;i++)
    {
        if (bound_box[i]<=3) continue;
        int count_3nb=0; int count_5nb=0;
        coord=pos2xyz(i,y_offset,z_offset);
        V3DLONG x=coord[0]; V3DLONG y=coord[1]; V3DLONG z=coord[2];
        for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_img[0]-1,x+1); dx++){
            for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_img[1]-1,y+1); dy++){
                for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_img[2]-1,z+1); dz++){
                    V3DLONG pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if (bound_box[pos1]>bound_box[i]) {goto break1;}
                    if (bound_box[pos1]>=0) count_3nb++;
                }
            }
        }
        for(V3DLONG dx=MAX(x-2,0); dx<=MIN(sz_img[0]-1,x+2); dx++){
            for(V3DLONG dy=MAX(y-2,0); dy<=MIN(sz_img[1]-1,y+2); dy++){
                for(V3DLONG dz=MAX(z-2,0); dz<=MIN(sz_img[2]-1,z+2); dz++){
                    V3DLONG pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if (bound_box[pos1]>=0) count_5nb++;
                }
            }
        }
        if (count_3nb==count_5nb||count_3nb<=4)
        {
            bound_box[i]=-1;
            neuron_id[i]=-1;
            continue;
        }
        //build map starting from the farthest distance
        order_map.insert(pair<float,V3DLONG>(bound_box[i],i));
    break1: {}
    }
    qDebug()<<"order map size:"<<order_map.size();


//    int count0_2=0; int count2_5=0; int count5_10=0; int count10=0;
//    multimap<float,V3DLONG>::reverse_iterator it = order_map.rbegin();
//    while (it!=order_map.rend())
//    {
//        V3DLONG i=it->second;
////        if (bound_box[i]>0&&bound_box[i]<=2) count0_2++;
////        else if(bound_box[i]>2&&bound_box[i]<=5) count2_5++;
////        else if(bound_box[i]>5&&bound_box[i]<=10) count5_10++;
////        else if(bound_box[i]>10) count10++;
//        visual_data[i+size_page]=255;
//        it++;
//    }
//    qDebug()<<"count0_2:"<<count0_2<<":"<<count2_5<<":"<<count5_10<<":"
//           <<count10;
    unsigned char *label=new unsigned char[size_page];
    memset(label,0,size_page);
//    for (V3DLONG i=0;i<size_page;i++)
//    {
//        if (bound_box[i]==0)
//            label[i]=1;
//    }
    multimap<float,V3DLONG>::reverse_iterator it = order_map.rbegin();
    V3DLONG spine_id=2;

//    QString fname="all.marker";
//    FILE *fp1=fopen(fname.toAscii(),"wt");
//    fprintf(fp1,"##x,y,z,radius,shape,name,comment,color_r,color_g,color_b\n");
    while(it!= order_map.rend())
    {
        V3DLONG tmp=it->second;
        if (label[tmp]>=2) {qDebug()<<"seed used"; it++; continue;}
        coord=pos2xyz(tmp,y_offset,z_offset);
        vector<V3DLONG> temp_vec;
        temp_vec=detect_obj.spine_grow(bound_box,label,tmp,40,spine_id,3000,30);
        qDebug()<<"spine_id:"<<spine_id<<":"<<bound_box[tmp];
        //spine_id++;
        //fprintf(fp1,"%d,%d,%d,1,1,"","",255,255,255\n",coord[0],coord[1],coord[2]);
        for (V3DLONG i=0;i<temp_vec.size();i++)
            visual_data[temp_vec[i]+1*size_page]=255;
        it++;
    }

    //fclose(fp1);

//    QString filename="2.marker";
//    QList<ImageMarker> LList=readMarker_file(filename);
//    for (int i=0;i<LList.size();i++)
//    {
//      qDebug()<<"i:__________"<<i;
//      V3DLONG tmp=xyz2pos(LList[i].x+0.5,LList[i].y+0.5,LList[i].z+0.5,y_offset,z_offset);
//      if (bound_box[tmp]<=0) {qDebug()<<"seeds not qualified";continue;}
//      temp_vec=detect_obj.spine_grow(bound_box,label,tmp,35,spine_id,2000,40);
//      spine_id++;
//      for (V3DLONG i=0;i<temp_vec.size();i++)
//          visual_data[temp_vec[i]+2*size_page]=255;
//      it++;
//    }

    if (neuron_id!=0) {delete[] neuron_id; neuron_id=0;}
    unsigned short *new_label=new unsigned short[size_page];
    memset(new_label,0,size_page*sizeof(unsigned short));
    int label_marker=1;
    detect_obj.connect_comp(label,new_label,label_marker,bound_box,2000,30,40);
    if (label!=0) {delete[] label; label=0;}
    if (bound_box!=0) {delete[] bound_box; bound_box=0;}
    //memcpy(visual_data+2*size_page,image1Dc_in,size_page);

    for (V3DLONG i=0;i<size_page;i++)
    {
        if (new_label[i]>0)
            visual_data[i+2*size_page]=image1Dc_in[i];
    }

      //filename="test.v3draw";
      //simple_saveimage_wrapper(*callback,filename.toAscii(),visual_data,sz_img,1);
    QString test_fname="newlabel.v3draw";
    V3DLONG sz[4]={sz_img[0],sz_img[1],sz_img[2],1};
    simple_saveimage_wrapper(*callback,test_fname.toAscii(),(unsigned char*)new_label,sz,2);
    Image4DSimple image4d;
    image4d.setData(visual_data,sz_img[0],sz_img[1],sz_img[2],3,V3D_UINT8);
    //image4d.setData((unsigned char*)new_label,sz_img[0],sz_img[1],sz_img[2],1,V3D_UINT16);
    v3dhandle main=callback->newImageWindow("test");
    callback->setImage(main,&image4d);
    callback->updateImageWindow(main);
    callback->open3DWindow(main);

}

void spine_detector_dialog::adaptive_thres(int *neuron_id,float *bound_box)
{
    qDebug()<<"in adaptive thresh";
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    float tolerance=0.001;
    float hisum,losum,dif,threshold,average,prevt;
    V3DLONG count,hicount,locount;
    vector<V3DLONG> coord(3,0);
    vector<float> thresh_array;
    vector<V3DLONG> value;
    for (V3DLONG i=0;i<neuron.listNeuron.size();i++)
    {
        float s_x=neuron.listNeuron.at(i).x;
        float s_y=neuron.listNeuron.at(i).y;
        float s_z=neuron.listNeuron.at(i).z;
        float radius=neuron.listNeuron[i].radius;
        float radius_new=radius+40;
        hisum=0.; count=0;
        for(V3DLONG dx=MAX(s_x-radius_new,0); dx<=MIN(sz_img[0]-1,s_x+radius_new); dx++){
            for(V3DLONG dy=MAX(s_y-radius_new,0); dy<=MIN(sz_img[1]-1,s_y+radius_new); dy++){
                for(V3DLONG dz=MAX(s_z-radius_new,0); dz<=MIN(sz_img[2]-1,s_z+radius_new); dz++){
//                    if (ABS(dx-s_x)<=radius||ABS(dy-s_y)<=radius
//                            ||ABS(dz-s_z)<=radius) continue;
                    V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    hisum+=image1Dc_in[pos];
                    count++;
                    value.push_back(pos);
                }
            }
        }
        if (count==0) {qDebug()<<"no voxels found";continue;}
        average=hisum/count;
        threshold=average;
        dif=tolerance+1.0;
        while(dif>tolerance)
        {
            prevt=threshold;
            hisum=losum=0.0;
            hicount=locount=0;
            for (int k=0;k<count;k++)
            {
                if (image1Dc_in[value[k]]<prevt)
                {
                    losum+=image1Dc_in[value[k]];
                    locount++;
                }
                else
                {
                    hisum+=image1Dc_in[value[k]];
                    hicount++;
                }
            }
            if (locount==0)
            {
                threshold = image1Dc_in[value[0]];
            }
            else
            {
                threshold=(hisum/hicount+losum/locount)/2.0;
            }
            dif = ABS(prevt-threshold);
        }
        thresh_array.push_back(threshold);
        neuron.listNeuron[i].type=threshold;
        value.clear();
        //qDebug()<<"neuron:"<<i<<":"<<"thresh:"<<threshold;
    }
//    QString filename="neurontree.swc";
//    writeSWC_file(filename.toAscii(),neuron);
    //thresholding for each voxel
    for (V3DLONG k=0;k<size_page;k++)
    {
        if (bound_box[k]<=0) continue;
        float dis1,dis2,dis3;
        coord=pos2xyz(k,y_offset,z_offset);
        int node_id=neuron_id[k];
        //qDebug()<<"k:"<<k<<" node_id:"<<node_id;
        dis2=neuron.listNeuron[node_id].r+bound_box[k];
        if (node_id==0) dis1=10000;
        else
        {
            float sx=neuron.listNeuron[node_id-1].x;
            float sy=neuron.listNeuron[node_id-1].y;
            float sz=neuron.listNeuron[node_id-1].z;
            float tmp_dis=(coord[0]-sx)*(coord[0]-sx)+(coord[1]-sy)*
                    (coord[1]-sy)+(coord[2]-sz)*(coord[2]-sz);
            dis1=sqrt(tmp_dis);
        }

        if (node_id==neuron.listNeuron.size()-1) dis3=10000;
        else
        {
            float sx=neuron.listNeuron[node_id+1].x;
            float sy=neuron.listNeuron[node_id+1].y;
            float sz=neuron.listNeuron[node_id+1].z;
            float tmp_dis=(coord[0]-sx)*(coord[0]-sx)+(coord[1]-sy)*
                    (coord[1]-sy)+(coord[2]-sz)*(coord[2]-sz);
            dis3=sqrt(tmp_dis);
        }
        if (dis1>=dis3) threshold=(dis3*thresh_array[node_id]+
                                  dis2*thresh_array[node_id+1])/(dis3+dis2);
        else if (dis1<dis3) threshold=(dis1*thresh_array[node_id]+
                                       dis2*thresh_array[node_id-1])/(dis1+dis2);
        if (image1Dc_in[k]<threshold)
        {
            bound_box[k]=-1;
            neuron_id[k]=-1;
        }
        //qDebug()<<"i:"<<image1Dc_in[k]<<" thresh:"<<threshold;
            //bound_box[value[k]]=-1; //means bg
    }

}

void spine_detector_dialog::load_swc()
{
    QString filename;
    filename = QFileDialog::getOpenFileName(0, 0,"","Supported file (*.swc)" ";;Neuron structure(*.swc)",0,0);

    if(filename.isEmpty())
    {
        v3d_msg("You don't have any SWC file open in the main window.");
        return;
    }
    NeuronSWC *p_cur=0;

    neuron = readSWC_file(filename);
    for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
    {
        p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
        if (p_cur->r<0)
        {
            v3d_msg("You have illeagal radius values. Check your data.");
            return;
        }
     }
    edit_swc->setText(filename);
}


void spine_detector_dialog::loadImage()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
             QDir::currentPath(),QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    if (!fileName.isEmpty())
    {
        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
            QMessageBox::information(0,"","load image "+fileName+" error!");
            return;
        }
        if (sz_img[3]>3)
        {
            sz_img[3]=3;
            QMessageBox::information(0,"","More than 3 channels were loaded."
                                     "The first 3 channel will be applied for analysis.");
            return;
        }



        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        if(intype==1)
        {
            detect_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
        }
        else if (intype == 2) //V3D_UINT16;
        {
            detect_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
            convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else if(intype == 4) //V3D_FLOAT32;
        {
            detect_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
            convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else
        {
            QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
            return;
        }
        edit_load->setText(fileName);
    }
}

bool spine_detector_dialog::load_image()
{
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback->getListAll3DViewers();

    if (v3dhandleList_current.size()==0){
        v3d_msg("Please open image and select markers");
        return false;
    }
    else if (v3dhandleList_current.size()==1)
    {
        curwin=v3dhandleList_current[0];
    }
    else if (v3dhandleList_current.size()>1)
    {
        QStringList items;
        int i;
        for (i=0; i<v3dhandleList_current.size(); i++)
            items << callback->getImageName(v3dhandleList_current[i]);

        for (i=0; i<cur_list_3dviewer.count(); i++)
        {
            QString curname = callback->getImageName(cur_list_3dviewer[i]).remove("3D View [").remove("]");
            bool b_found=false;
            for (int j=0; j<v3dhandleList_current.size(); j++)
                if (curname==callback->getImageName(v3dhandleList_current[j]))
                {
                    b_found=true;
                    break;
                }

            if (!b_found)
                items << callback->getImageName(cur_list_3dviewer[i]);
        }
        qDebug()<<"Number of items:"<<items.size();

        QDialog *mydialog=new QDialog;
        QComboBox *combo=new QComboBox();
        combo->insertItems(0,items);
        QLabel *label_win=new QLabel;
        label_win->setText("You have multiple windows open, please select one image:");
        QGridLayout *layout= new QGridLayout;
        layout->addWidget(label_win,0,0,1,1);
        layout->addWidget(combo,1,0,4,1);
        QPushButton *button_d_ok=new QPushButton;
        button_d_ok->setText("Ok");
        button_d_ok->setFixedWidth(100);
        QPushButton *button_d_cancel=new QPushButton;
        button_d_cancel->setText("Cancel");
        button_d_cancel->setFixedWidth(100);
        QHBoxLayout *box=new QHBoxLayout;
        box->addWidget(button_d_ok,Qt::AlignCenter);
        box->addWidget(button_d_cancel,Qt::AlignCenter);
        layout->addLayout(box,5,0,1,1);
        connect(button_d_ok,SIGNAL(clicked()),mydialog,SLOT(accept()));
        connect(button_d_cancel,SIGNAL(clicked()),mydialog,SLOT(reject()));
        mydialog->setLayout(layout);
        mydialog->exec();
        if (mydialog->result()==QDialog::Accepted)
        {
            int tmp=combo->currentIndex();
            curwin=v3dhandleList_current[tmp];
        }
        else
        {
            v3d_msg("You have not selected a window");
            return false;
        }
    }

    //Get the image info
    Image4DSimple* p4DImage = callback->getImage(curwin);
    if (!p4DImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return false;
    }

    sz_img[0]=p4DImage->getXDim();
    sz_img[1]=p4DImage->getYDim();
    sz_img[2]=p4DImage->getZDim();
    sz_img[3]=1;

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]; //since we only get one channel
    V3DLONG cid=0; //needs to put in the para setting
    image1Dc_in = p4DImage->getRawDataAtChannel(cid);
    ImagePixelType pixeltype = p4DImage->getDatatype();

    if(pixeltype==1)//V3D_UNIT8
    {
        detect_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
    }

    else if (pixeltype == 2) //V3D_UINT16;
    {
        detect_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(pixeltype == 4) //V3D_FLOAT32;
    {
        detect_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
        convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
       QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
       return false;
    }
    //LOAD markers

    LList_in.clear();
    LList_in = callback->getLandmark(curwin);
    return true;
}



void spine_detector_dialog::convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    unsigned short max_v=0, min_v = 255;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}

void spine_detector_dialog::convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    float* pPre = (float*)pre1d;
    float max_v=0, min_v = 65535;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}

void spine_detector_dialog::core()
{

    if (!load_image()) return;
    detect_obj.bubbles();
//    vector<V3DLONG> poss_landmark;
//    vector<float> ray_vector(48,0);
//    poss_landmark.clear();
//    poss_landmark=landMarkList2poss(LList_in, sz_img[0], sz_img[0]*sz_img[1]);
//    V3DLONG page_size=sz_img[0]*sz_img[1]*sz_img[2];
//    int bg_thr=12;

//    //GENERATE GSDT
////    float *phi=0;
////    fastmarching_dt(image1Dc_in,phi,sz_img[0],sz_img[1],sz_img[2],3,bg_thr);

////    float min_val = phi[0], max_val = phi[0];

////    float *outimg1d_tmp = new float[page_size*sizeof(float)];

////    for(V3DLONG i = 0; i < page_size; i++)
////    {
////        if(phi[i] == INF) continue;
////        min_val = MIN(min_val, phi[i]);
////        max_val = MAX(max_val, phi[i]);
////    }
////    max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;

////    for(V3DLONG i = 0; i < page_size; i++)
////    {
////        if(phi[i] == INF) outimg1d_tmp[i] = 0;
////        else if(phi[i] ==0) outimg1d_tmp[i] = 0;
////        else
////        {
////            outimg1d_tmp[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
////            outimg1d_tmp[i] = MAX(outimg1d_tmp[i], 1);
////        }
////    }

////    if (phi!=0) {delete phi; phi=0;}

//    //generate ray vectors
//    QString filename="spines_ray.csv";
//    FILE *fp=fopen(filename.toAscii(),"w");

//    for (int j=0;j<poss_landmark.size();j++)
//    {
//        ray_vector=detect_obj.ray_shoot_vector(poss_landmark[j]);
////        double distance=detect_obj.calc_mean_shift_center(poss_landmark[j],15,outimg1d_tmp,sz_img,2);
////        fprintf(fp,"%.2f,",distance);
//        for (int i=0;i<48;i++)
//        {
//          if (i==47) fprintf(fp,"%.2f\n",ray_vector[i]);
//          else
//          fprintf(fp,"%.2f,",ray_vector[i]);
//        }
//    }
//    fclose(fp);
}

void spine_detector_dialog::count_neighbors(unsigned char *boundary,unsigned char *single)
{
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    V3DLONG size_page=sz_img[0]*sz_img[2]*sz_img[2];
    int count_nb=0; V3DLONG count_total=0;
    vector<V3DLONG> coord;
    for (V3DLONG i=0;i<size_page;i++)
    {
        count_nb=0;
        if (boundary[i]>0)
        {
            coord=pos2xyz(i, y_offset, z_offset);
            V3DLONG x=coord[0];V3DLONG y=coord[1];V3DLONG z=coord[2];
            int count=0;
            for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_img[0]-1,x+1); dx++){
                for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_img[1]-1,y+1); dy++){
                    for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_img[2]-1,z+1); dz++){
                        V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                        if (image1Dc_in[pos]>80) count_nb++;
                    }
                }
            }
        if (count_nb<1) {single[i]=255;count_total++;}
        }
    }
    qDebug()<<"count_total:"<<count_total;

}
