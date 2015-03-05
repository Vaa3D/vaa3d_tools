#include "filter_dialog.h"
#include "v3d_message.h"
#include <vector>
#include <math.h>
#include "string"
#include "sstream"
#include <iostream>
#include <cstdlib>
#include <fstream>

// lroundf() is gcc-specific --CMB
#ifdef _MSC_VER
inline long lroundf(float num) { return static_cast<long>(num > 0 ? num + 0.5f : ceilf(num - 0.5f)); }
#endif

filter_dialog::filter_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image_data=0;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3];
    intype=0;
    create();
}

void filter_dialog::create()
{
    mygridLayout = new QGridLayout();
    QLabel* label_load = new QLabel(QObject::tr("Load Image:"));
    mygridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit();
    edit_load->setText(""); edit_load->setReadOnly(true);
    mygridLayout->addWidget(edit_load,0,1,1,4);
    QPushButton *btn_load = new QPushButton("...");
    mygridLayout->addWidget(btn_load,0,5,1,1);

    QLabel* label_swc = new QLabel(QObject::tr("Load swc file:"));
    mygridLayout->addWidget(label_swc,1,0,1,1);
    edit_swc = new QLineEdit();
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

void filter_dialog::dialoguefinish(int)
{
    if (this->result()==QDialog::Accepted)
    {
        swc_filter_image();
    }

    else{
        v3d_msg("No files are loaded");
        //reset image_data and neuron
        if (image_data!=0){
            delete [] image_data;
            image_data=0;
        }

    }
}

void filter_dialog::load_swc()
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
        if (p_cur->r<=0)
        {
            v3d_msg("You have illeagal radius values. Check your data.");
            return;
        }
     }
    edit_swc->setText(filename);
}


void filter_dialog::loadImage()
{
    QString fileName;
    image_data=0;

    fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
             QDir::currentPath(),QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    if (!fileName.isEmpty())
    {
        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image_data, sz_img, intype))
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
        if(intype!=1)
        {
            if (intype == 2) //V3D_UINT16;
            {
                convert2UINT8((unsigned short*)image_data, image_data, size_tmp);
            }
            else if(intype == 4) //V3D_FLOAT32;
            {
                convert2UINT8((float*)image_data, image_data, size_tmp);
            }
            else
            {
                QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
                return;
            }
        }
        edit_load->setText(fileName);
    }
}



void filter_dialog::swc_filter_image()
{
    double x_min,x_max,y_min,y_max,z_min,z_max;
    x_min=x_max=y_min=y_max=z_min=z_max=0;
    V3DLONG sx,sy,sz;
    unsigned char *pImMask = 0;
    V3DLONG nx,ny,nz; //the image size
    nx=sz_img[0]; ny=sz_img[1]; nz=sz_img[2];

    BoundNeuronCoordinates(neuron,x_min,x_max,y_min,y_max,z_min,z_max);
    sx=x_max;
    sy=y_max;
    sz=z_max;
    V3DLONG stacksz = sx*sy*sz;
    pImMask = new unsigned char [stacksz];
    memset(pImMask,0,stacksz*sizeof(unsigned char));
    ComputemaskImage(neuron, pImMask, sx, sy, sz);
    unsigned char * image_filter=new unsigned char[nx*ny*nz*sz_img[3]];
    memcpy(image_filter,image_data,nx*ny*nz*sz_img[3]*sizeof(unsigned char));

    for (V3DLONG k1 = 0; k1 < sz; k1++){
        for(V3DLONG j1 = 0; j1 < sy; j1++){
            for(V3DLONG i1 = 0; i1 < sx; i1++){
                 if ((i1>nx-1)||(j1>ny-1)||(k1>nz-1)) continue;
                 if (pImMask[k1*sx*sy + j1*sx +i1]>0) continue;

                 image_filter[k1*nx*ny + j1*nx +i1]=0;
                 if (sz_img[3]>1){
                 image_filter[nx*ny*nz+k1*nx*ny + j1*nx +i1]=0;
                 }
                 if (sz_img[3]>2){
                 image_filter[2*nx*ny*nz+k1*nx*ny + j1*nx +i1]=0;
                 }

             }
         }
     }
    Image4DSimple tmp,tmp_out;
    tmp.setData(image_data, nx, ny, nz, sz_img[3], V3D_UINT8);
    tmp_out.setData(image_filter,nx,ny,nz,sz_img[3],V3D_UINT8);
    v3dhandle newwin = callback->newImageWindow();
    v3dhandle newwin_out=callback->newImageWindow();
    callback->setImage(newwin, &tmp);
    callback->setImageName(newwin, QString("Input_swc_filter"));
    callback->setImage(newwin_out,&tmp_out);
    callback->setImageName(newwin_out, QString("Ouput_swc_filter"));
    callback->updateImageWindow(newwin);
    callback->updateImageWindow(newwin_out);
    callback->open3DWindow(newwin);
    callback->open3DWindow(newwin_out);

    if (pImMask!=0) {delete []pImMask; pImMask=0;}
}



QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
    QHash<V3DLONG, V3DLONG> neuron_id_table;
    for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
        neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
    return neuron_id_table;
}

void BoundNeuronCoordinates(NeuronTree & neuron, double & output_xmin,double & output_xmax,
                            double & output_ymin,double & output_ymax,double & output_zmin,double & output_zmax)
{
    NeuronSWC *p_cur = 0;
    for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
    {
        p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
        if (ii==0)
        {
            output_xmin = p_cur->x - p_cur->r;
            output_ymin = p_cur->y - p_cur->r;
            output_zmin = p_cur->z - p_cur->r;
            output_xmax = p_cur->x + p_cur->r;
            output_ymax = p_cur->y + p_cur->r;
            output_zmax = p_cur->z + p_cur->r;
        }
        else
        {
            output_xmin = (p_cur->x - p_cur->r < output_xmin) ? (p_cur->x - p_cur->r) : output_xmin;
            output_ymin = (p_cur->y - p_cur->r < output_ymin) ? (p_cur->y - p_cur->r) : output_ymin;
            output_zmin = (p_cur->z - p_cur->r < output_zmin) ? (p_cur->z - p_cur->r) : output_zmin;

            output_xmax = (p_cur->x + p_cur->r > output_xmax) ? (p_cur->x + p_cur->r) : output_xmax;
            output_ymax = (p_cur->y + p_cur->r > output_ymax) ? (p_cur->y + p_cur->r) : output_ymax;
            output_zmax = (p_cur->z + p_cur->r > output_zmax) ? (p_cur->z + p_cur->r) : output_zmax;
        }
    }

    return;
}

void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz)
{
    NeuronSWC *p_cur = 0;
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = sx*sy;

    qDebug()<<"neuron size:"<<neurons.listNeuron.size();
    int count=0;
    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        V3DLONG i,j,k;
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
        xs = p_cur->x;
        ys = p_cur->y;
        zs = p_cur->z;
        rs = p_cur->r;

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
                    if (pImMask[ind]>0) continue;
                    if (ind>sx*sy*sz-1) continue;
                    double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
                    double dt = sqrt(norms10);
                    if(dt <=rs || dt<=1) pImMask[ind] = 255;
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
        if (idex1>sx*sy*sz-1) continue;
         pImMask[idex1] = 255;

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
            if (idex>sx*sy*sz-1) continue;
            pImMask[idex] = 255;
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
                    if (ind1>sx*sy*sz-1) continue;
                    if (dist <= rr || dist<=1)
                    {
                        pImMask[ind1] = 255;
                    }
                }
            }
        }
        count++;
        qDebug()<<"count:"<<count;
    }

}

void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
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

void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
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
