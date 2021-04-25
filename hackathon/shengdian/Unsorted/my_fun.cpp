#include "my_fun.h"
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
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
void getTipComponent(QString inswc_file, QString outpath, int cropx, int cropy, int cropz)
{
    QDir path(outpath);
    if(!path.exists())
        path.mkpath(outpath);
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz = nt.listNeuron.size();
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    if(siz==0)
        return;
    //get the tips
    listNeuron=nt.listNeuron;
    QHash <long, long>  hashNeuron,hashchild;
    hashNeuron.clear(); hashchild.clear();
    //hashchild: value is #child of node, key is node id.
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        hashNeuron.insert(s.n,i);
        hashchild.insert(s.n,0);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        //get parent node
        if(s.parent>0)
        {
            //this node is not soma.
            hashchild[s.parent]+=1;
        }
    }
    QHash <long, long>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        //this node is tip node
        if(hcit.value()==0)
        {
            V3DLONG tipID=hcit.key();
            V3DLONG tipIndex=hashNeuron.value(tipID);
            NeuronSWC s = listNeuron[tipIndex];

            //this tip block boundary
            long start_x,start_y,start_z,end_x,end_y,end_z;
            start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
            end_x = s.x + cropx/2; /*if(end_x > in_zz[0]) end_x = in_zz[0];*/
            start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
            end_y = s.y + cropy/2;/*if(end_y > in_zz[1]) end_y = in_zz[1];*/
            start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
            end_z = s.z + cropz/2;/*if(end_z > in_zz[2]) end_z = in_zz[2];*/
            //get the raw cropped swc
            NeuronTree nt_corped_raw=NeuronTree();
            long count = 0;
            for(long i=0; i<siz; i++)
            {
                NeuronSWC thiss = listNeuron[i];
                if(thiss.x>=start_x&&thiss.x<end_x
                        &&thiss.y>=start_y&&thiss.y<end_y
                        &&thiss.z>=start_z&&thiss.z<end_z)
                {

//                    thiss.x-=start_x;
//                    thiss.y-=start_y;
//                    thiss.z-=start_z;
                    nt_corped_raw.listNeuron.push_back(thiss);
                    nt_corped_raw.hashNeuron.insert(thiss.n,count);
                    count++;
                }
            }
            //save to file
//            if(nt_corped_raw.listNeuron.size()>0)
//            {
//                QString tmpstr = "";
//                tmpstr.append("_x_").append(QString("%1").arg(s.x));
//                tmpstr.append("_y_").append(QString("%1").arg(s.y));
//                tmpstr.append("_z_").append(QString("%1").arg(s.z));
//                QString default_name = "RawTip"+tmpstr+".swc";
//                QString save_croped_path =outpath+"/"+default_name;
//                writeESWC_file(save_croped_path,nt_corped_raw);
//            }

            //if node is related to this tip node, keep it
            NeuronTree nt_corped_final=NeuronTree();
            long count_final = 0;
            //start from this tip, iterative push node into new swc neuron tree. tipID
            QHash <int, int> cropedHashNeuron=nt_corped_raw.hashNeuron;
            int crop_tipIndex=cropedHashNeuron.value(tipID);
            NeuronSWC start_tip = nt_corped_raw.listNeuron[crop_tipIndex];
            nt_corped_final.listNeuron.push_back(start_tip);
            nt_corped_final.hashNeuron.insert(start_tip.n,count_final);
            count_final++;
            //for the start tip, get its' parent node in this block
            V3DLONG stPid=start_tip.parent;
            while(stPid>0)
            {
                if(!cropedHashNeuron.contains(stPid))
                    break;
                int stP_Index=cropedHashNeuron.value(stPid);
                NeuronSWC stP = nt_corped_raw.listNeuron[stP_Index];
                nt_corped_final.listNeuron.push_back(stP);
                nt_corped_final.hashNeuron.insert(stP.n,count_final);
                count_final++;
                stPid=stP.parent;
            }
            //for all the tips in this cropped swc, get the brother tips
            for(long i=0; i<nt_corped_raw.listNeuron.size(); i++)
            {
                NeuronSWC crops = nt_corped_raw.listNeuron[i];
                //is this tip node?
                bool brotip=false;
//                if(/*hashchild.value(crops.n)==0*/true)
                if(true)
                {
                    //is parent node in nt_corped_final
                    //parent node must in cropedHashNeuron
                    V3DLONG scPid=crops.parent;
                    while(scPid>0)
                    {
                        if(!cropedHashNeuron.contains(scPid))
                            break;
                        if(nt_corped_final.hashNeuron.contains(scPid))
                        {
                            //this is a brother tips
                            brotip=true;
                            break;
                        }
                        int stP_Index=cropedHashNeuron.value(scPid);
                        NeuronSWC scP = nt_corped_raw.listNeuron[stP_Index];

                        scPid=scP.parent;
                    }
                }
                if(brotip)
                {
                    if(!nt_corped_final.hashNeuron.contains(crops.n))
                    {
                        nt_corped_final.listNeuron.push_back(crops);
                        nt_corped_final.hashNeuron.insert(crops.n,count_final);
                        count_final++;
                    }
                    V3DLONG scPid=crops.parent;
                    while(scPid>0)
                    {
                        if(!cropedHashNeuron.contains(scPid)||nt_corped_final.hashNeuron.contains(scPid))
                            break;
                        int stP_Index=cropedHashNeuron.value(scPid);
                        NeuronSWC stP = nt_corped_raw.listNeuron[stP_Index];
                        nt_corped_final.listNeuron.push_back(stP);
                        nt_corped_final.hashNeuron.insert(stP.n,count_final);
                        count_final++;
                        scPid=stP.parent;
                    }
                }
            }
            //
            //save to file
            if(nt_corped_final.listNeuron.size()>0)
            {
                QString tmpstr = "";
                tmpstr.append("_x_").append(QString("%1").arg(s.x));
                tmpstr.append("_y_").append(QString("%1").arg(s.y));
                tmpstr.append("_z_").append(QString("%1").arg(s.z));
                QString default_name = "Tip"+tmpstr+".swc";
                QString save_croped_path =outpath+"/"+default_name;
                writeESWC_file(save_croped_path,nt_corped_final);
            }
        }
    }
}
void getTipBlock(V3DPluginCallback2 &callback, string imgPath, QString inswc_file, QString outpath, int cropx, int cropy, int cropz)
{

    QDir path(outpath);
    if(!path.exists())
        path.mkpath(outpath);
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz = nt.listNeuron.size();
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    if(siz==0)
        return;
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    //get the tips
    listNeuron=nt.listNeuron;
    QHash <long, long>  hashNeuron,hashchild;
    hashNeuron.clear(); hashchild.clear();
    //hashchild: value is #child of node, key is node id.
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        hashNeuron.insert(s.n,i);
        hashchild.insert(s.n,0);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        //get parent node
        if(s.parent>0)
        {
            //this node is not soma.
            hashchild[s.parent]+=1;
        }
    }
    cout<<"Input crop size x="<<cropx<<";y="<<cropy<<";z="<<cropz<<endl;
    long tipTotal=0;int refinedBlockSize=16;
    QHash <long, long>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        //this node is tip node
        if(hcit.value()==0)
        {
            tipTotal++;
            V3DLONG tipID=hcit.key();
            V3DLONG tipIndex=hashNeuron.value(tipID);
            NeuronSWC s = listNeuron[tipIndex];
            //refine tip node to local max intensity pixel
            long refined_start_x,refined_start_y,refined_start_z,refined_end_x,refined_end_y,refined_end_z;
            refined_start_x = s.x - refinedBlockSize/2; if(refined_start_x<0) refined_start_x = 0;
            refined_end_x = s.x + refinedBlockSize/2; if(refined_end_x > in_zz[0]) refined_end_x = in_zz[0];
            refined_start_y =s.y - refinedBlockSize/2;if(refined_start_y<0) refined_start_y = 0;
            refined_end_y = s.y + refinedBlockSize/2;if(refined_end_y > in_zz[1]) refined_end_y = in_zz[1];
            refined_start_z = s.z - refinedBlockSize/2;if(refined_start_z<0) refined_start_z = 0;
            refined_end_z = s.z + refinedBlockSize/2;if(refined_end_z > in_zz[2]) refined_end_z = in_zz[2];
            V3DLONG *in_sz_refined = new V3DLONG[4];
            in_sz_refined[0] = refinedBlockSize;
            in_sz_refined[1] = refinedBlockSize;
            in_sz_refined[2] = refinedBlockSize;
            in_sz_refined[3]=in_zz[3];
            unsigned char * im_refined_block = 0;
            V3DLONG pagesz_refined;
            pagesz_refined = (refined_end_x-refined_start_x+1)*(refined_end_y-refined_start_y+1)*(refined_end_z-refined_start_z+1);
            try {im_refined_block = new unsigned char [pagesz_refined];}
            catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

            im_refined_block = callback.getSubVolumeTeraFly(imgPath,refined_start_x,refined_end_x,refined_start_y,refined_end_y,refined_start_z,refined_end_z);
            if(im_refined_block==NULL){
                continue;
            }
            V3DLONG thisx,thisy,thisz;
            thisx=s.x-refined_start_x;
            thisy=s.y-refined_start_y;
            thisz=s.z-refined_start_z;
            NeuronSWC s_refined=nodeRefine(im_refined_block,thisx,thisy,thisz,in_sz_refined,5);
            QString tmpstr_raw = "";
            tmpstr_raw.append("_rawx_").append(QString("%1").arg(s.x));
            tmpstr_raw.append("_rawy_").append(QString("%1").arg(s.y));
            tmpstr_raw.append("_rawz_").append(QString("%1").arg(s.z));

            s.x=refined_start_x+s_refined.x;
            s.y=refined_start_y+s_refined.y;
            s.z=refined_start_z+s_refined.z;
            if(im_refined_block) {delete []im_refined_block; im_refined_block = 0;}
            //get img block
            //read img
            long start_x,start_y,start_z,end_x,end_y,end_z;
            start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
            end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0];
            start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
            end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1];
            start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
            end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2];
//            cout<<"crop size x="<<start_x<<";y="<<start_y<<";z="<<start_z<<endl;
            V3DLONG *in_sz = new V3DLONG[4];
            in_sz[0] = cropx;
            in_sz[1] = cropy;
            in_sz[2] = cropz;
            in_sz[3]=in_zz[3];
            unsigned char * im_cropped = 0;
            V3DLONG pagesz;
            pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

            im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
            if(im_cropped==NULL){
                continue;
            }
            QString tmpstr = "";
            tmpstr.append("_x_").append(QString("%1").arg(s.x));
            tmpstr.append("_y_").append(QString("%1").arg(s.y));
            tmpstr.append("_z_").append(QString("%1").arg(s.z));
            QString default_name = "Img"+tmpstr+tmpstr_raw+".v3draw";
            QString save_path_img =outpath+"/"+default_name;
            simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
            if(im_cropped) {delete []im_cropped; im_cropped = 0;}
        }
    }
    cout<<"# Tip Block: "<<tipTotal<<endl;
    cout<<"Save to "<<outpath.toStdString()<<endl;
}
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size)
{
    //return the intensity of the input node
//    int outRadius=0;
    cout<<"---Node refine to the local maximal intensity----"<<endl;
    NeuronSWC out;
    V3DLONG maxIntensity=0;
    V3DLONG thisx,thisy,thisz;
    out.x=thisx=nodex;
    out.y=thisy=nodey;
    out.z=thisz=nodez;

    V3DLONG sz01 = sz[0] * sz[1];
//    cout<<"----------node new------------"<<endl;
    for(V3DLONG iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz++)
    {
        for( V3DLONG iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size+1)&&(iy<sz[1]);iy++)
        {
            for(V3DLONG ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size+1)&&(ix<sz[0]);ix++)
            {

                V3DLONG pos = iz*sz01 + iy * sz[0] + ix;
                V3DLONG thisIntensity=inimg1d[pos];
                if(thisIntensity>maxIntensity)
                {
//                    cout<<"update to intensity:"<<thisIntensity<<endl;
                    maxIntensity=thisIntensity;
                    out.x=ix;
                    out.y=iy;
                    out.z=iz;
                }
            }
        }
    }
    return out;
}
void getSomaBlock(V3DPluginCallback2 &callback, string imgPath, QString inapo_file, QString outpath, int cropx, int cropy, int cropz)
{
    QList <CellAPO> apolist=readAPO_file(inapo_file);
    if(apolist.size()==0)
        return;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
        end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
        end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
        end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2];
        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = cropx;
        in_sz[1] = cropy;
        in_sz[2] = cropz;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
        if(im_cropped==NULL){
            continue;
        }
        //below is croped img saving and marker saving
        QString tmpstr =s.name;
        tmpstr.append("-x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "ImgSoma_"+tmpstr+".v3draw";
        QString save_path_img =outpath+"/"+default_name;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
}
void getMarkerRadius(unsigned char *&inimg1d, long in_sz[], NeuronSWC& s)
{
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    double max_r = in_sz[0]/2;
    if (max_r > in_sz[1]/2) max_r = in_sz[1]/2;
    if (max_r > (in_sz[2])/2) max_r = (in_sz[2])/2;

    //get the background threshold
    double imgave,imgstd;
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
    mean_and_std(inimg1d,total_size,imgave,imgstd);
//    double td= (imgstd<10) ? 10:imgstd;
    double bkg_thresh= (imgave+0.5*imgstd)>20?20:(imgave+0.5*imgstd);
    cout<<"Img mean= "<<imgave<<" ,std="<<imgstd<<" ,thresh="<<bkg_thresh<<endl;
    int thisx,thisy,thisz;
    thisx=s.x;
    thisy=s.y;
    thisz=s.z;

    double total_num, background_num,total_intensity;
    double ir;
    total_intensity=0;
    for (ir=1.0; ir<=max_r; ir++)
    {
        total_num = background_num = 0;
        double dz, dy, dx;
        double zlower = -ir, zupper = +ir;
        for (dz= zlower; dz <= zupper; ++dz)
            for (dy= -ir; dy <= +ir; ++dy)
                for (dx= -ir; dx <= +ir; ++dx)
                {
                    total_num++;
                    double r = sqrt(dx*dx + dy*dy + dz*dz);
                    if (r>ir-1 && r<=ir)
                    {
                        V3DLONG i = thisx+dx;   if (i<0 || i>=in_sz[0]) goto end2;
                        V3DLONG j = thisy+dy;   if (j<0 || j>=in_sz[1]) goto end2;
                        V3DLONG k = thisz+dz;   if (k<0 || k>=in_sz[2]) goto end2;

                        if (inimg1d[k * sz01 + j * sz0 + i] <= bkg_thresh)
                        {
                            background_num++;
                            inimg1d[k * sz01 + j * sz0 + i]=0;
                            if ((background_num/total_num) > 0.01) goto end2;
                        }
                        else
                        {
                            total_intensity+=inimg1d[k * sz01 + j * sz0 + i];
                        }
                    }
                }
    }
end2:
    cout<<"radius="<<ir<<", totalnum="<<total_num<<endl;
    s.radius= float(ir);
    s.timestamp=total_num;
    s.tfresindex=total_intensity/total_num;
}
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath, QList<CellAPO> apolist, string outpath, int cropx, int cropy, int cropz)
{
    cout<<"Welcome into terafly block crop"<<endl;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    cout<<"Input crop size x="<<cropx<<";y="<<cropy<<";z="<<cropz<<endl;
    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
        end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
        end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
        end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2];
        cout<<"crop size x="<<start_x<<";y="<<start_y<<";z="<<start_z<<endl;
        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = cropx;
        in_sz[1] = cropy;
        in_sz[2] = cropz;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x)*(end_y-start_y)*(end_z-start_z);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
        if(im_cropped==NULL){
            continue;
        }
        QString tmpstr = "";
        tmpstr.append("_x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "Img"+tmpstr+".v3draw";
        QString save_path = QString::fromStdString(outpath);
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}
void getSWCIntensityInTerafly(V3DPluginCallback2 &callback, string imgPath, QString inswc_file)
{
    cout<<"Welcome into Intensity statistics"<<endl;
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();

    float intensityList[siz];
    float meanList[siz];
    float stdintensityList[siz];
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=1;
        intensityList[i]=meanList[i]=stdintensityList[i]=0;
        hashNeuron.insert(listNeuron[i].n,i);
    }
    V3DLONG *in_zz = 0;
        //read img
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    //for wirting out
    QString inswc_file_out = inswc_file + "_intensityStatistics.csv";
    ofstream fp(inswc_file_out.toStdString().c_str());
    if(!fp.is_open()){
        cout<<"Can't create file for out"<<endl;
        exit(1);
    }
    fp<<"Intensity,Mean,Std"<<endl;
    //start from here
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and radius=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.radius==1)
        {
            //get a block
            long start_x,start_y,start_z,end_x,end_y,end_z;
            long block_size=64;
            start_x = s.x - block_size; if(start_x<0) start_x = 0;
            end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
            start_y =s.y - block_size;if(start_y<0) start_y = 0;
            end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
            start_z = s.z - block_size;if(start_z<0) start_z = 0;
            end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

            V3DLONG *in_sz = new V3DLONG[4];
            in_sz[0] = end_x - start_x;
            in_sz[1] = end_y - start_y;
            in_sz[2] = end_z - start_z;
            in_sz[3]=in_zz[3];
            V3DLONG *sz;
            sz=in_sz;
            long sz0 = sz[0];
            long sz01 = sz[0] * sz[1];
            unsigned char * inimg1d = 0;
            inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            //            //version one
            int thisx,thisy,thisz;
            thisx=s.x-start_x;
            thisy=s.y-start_y;
            thisz=s.z-start_z;

            listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
//            cout<<"This node intensity is "<<listNeuron[i].radius<<endl;
            double imgave,imgstd;
            V3DLONG total_size=8*block_size;
            mean_and_std(inimg1d,total_size,imgave,imgstd);
//            cout<<"here"<<endl;
            intensityList[i]=listNeuron[i].level;
            meanList[i]=imgave;
            stdintensityList[i]=imgstd;
            cout<<intensityList[i]<<","<<imgave<<","<<imgstd<<endl;
            fp<<intensityList[i]<<","<<imgave<<","<<imgstd<<endl;

            if(inimg1d) {delete []inimg1d; inimg1d=0;}
        }
    }
    fp.close();
    //out swc file
    float intensityMean,imgMean,imgstdMean;
    intensityMean=0;
    imgMean=0;
    imgstdMean=0;
    for(V3DLONG i=0;i<siz;i++)
    {
        intensityMean+=intensityList[i];
        imgMean+=meanList[i];
        imgstdMean+=stdintensityList[i];
        listNeuron[i].radius=10*listNeuron[i].level/255;
    }
    intensityMean/=siz;
    imgMean/=siz;
    imgstdMean/=siz;
    QString tmpstr = "";
    tmpstr.append("_NodeIntensity_").append(QString("%1").arg(intensityMean));
    tmpstr.append("_ImgIntensityMean_").append(QString("%1").arg(imgMean));
    tmpstr.append("_ImgIntensityStd_").append(QString("%1").arg(imgstdMean));
    QString outswc_name =inswc_file+tmpstr+".eswc";
    writeESWC_file(outswc_name,nt);
    cout<<"Done:"<<endl;
}
void erosionImg(unsigned char *&inimg1d, long in_sz[], int kernelSize)
{
    /*Img processing: erosion
     *kernel size: default=3
     *revise the value of each pixels to it's local minimal
    */
    cout<<"Img processing: erosion"<<endl;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    long startx,starty,startz;
    long endx,endy,endz;
    cout<<"Input img size x="<<in_sz[0]<<";y="<<in_sz[1]<<";z="<<in_sz[2]<<endl;
    unsigned char * im_transfer = 0;
    V3DLONG pagesz;
    pagesz = sz01*sz0;
    try {im_transfer = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
//    im_transfer=inimg1d;
    //back up
    for(long ix=0;ix<in_sz[0];ix++)
    {
        for(long iy=0;iy<in_sz[1];iy++)
        {
            for(long iz=0;iz<in_sz[2];iz++)
            {
                im_transfer[iz * sz01 + iy * sz0 + ix]=inimg1d[iz * sz01 + iy * sz0 + ix];
            }
        }
    }
    cout<<"copy img pointer"<<endl;
    for(long ix=0;ix<in_sz[0];ix++)
    {
        for(long iy=0;iy<in_sz[1];iy++)
        {
            for(long iz=0;iz<in_sz[2];iz++)
            {
//                im_transfer[iz * sz01 + iy * sz0 + ix]=0;
                //get the local area
                startx=(ix-kernelSize>=0)?(ix-kernelSize):0;
                endx=(ix+kernelSize<in_sz[0])?(ix+kernelSize):in_sz[0];
                starty=(iy-kernelSize>=0)?(iy-kernelSize):0;
                endy=(iy+kernelSize<in_sz[1])?(iy+kernelSize):in_sz[1];
                startz=(iz-kernelSize>=0)?(iz-kernelSize):0;
                endz=(iz+kernelSize<in_sz[2])?(iz+kernelSize):in_sz[2];
                long tmpIntensity=im_transfer[iz * sz01 + iy * sz0 + ix];
                for(long kx=startx;kx<endx;kx++)
                {
                    for(long ky=starty;ky<endy;ky++)
                    {
                        for(long kz=startz;kz<endz;kz++)
                        {
                            long thistmpIntensity=im_transfer[kz * sz01 + ky * sz0 + kx];
                            tmpIntensity=(thistmpIntensity<tmpIntensity)?thistmpIntensity:tmpIntensity;

                        }
                    }
                }
                //erosion
                inimg1d[iz * sz01 + iy * sz0 + ix]=tmpIntensity;
            }
        }
    }
    if(im_transfer) {delete []im_transfer; im_transfer=0;}
}
void maskImg(V3DPluginCallback2 &callback, unsigned char *&inimg1d, QString outpath, long in_sz[], NeuronTree &nt, int maskRadius,int erosion_kernel_size)
{
    /*for all the pixels in the dst block*/
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    V3DLONG siz = listNeuron.size();
    if(siz<1) return;
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    int startx,starty,startz;
    int endx,endy,endz;
    cout<<"Input img size x="<<in_sz[0]<<";y="<<in_sz[1]<<";z="<<in_sz[2]<<endl;
    unsigned char * im_transfer = 0;
    V3DLONG pagesz;
    pagesz = sz01*sz0;
    try {im_transfer = new unsigned char [pagesz];}
    catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}
    for(int ix=0;ix<in_sz[0];ix++)
    {
        for(int iy=0;iy<in_sz[1];iy++)
        {
            for(int iz=0;iz<in_sz[2];iz++)
            {
                im_transfer[iz * sz01 + iy * sz0 + ix]=0;
            }
        }
    }

    for(int is=0;is<siz;is++)
    {
        int ix,iy,iz;
        NeuronSWC thiss = listNeuron[is];
        ix=int(thiss.x);
        iy=int(thiss.y);
        iz=int(thiss.z);
        startx=(ix-maskRadius>=0)?(ix-maskRadius):0;
        endx=(ix+maskRadius<in_sz[0])?(ix+maskRadius):in_sz[0];
        starty=(iy-maskRadius>=0)?(iy-maskRadius):0;
        endy=(iy+maskRadius<in_sz[1])?(iy+maskRadius):in_sz[1];
        startz=(iz-maskRadius>=0)?(iz-maskRadius):0;
        endz=(iz+maskRadius<in_sz[2])?(iz+maskRadius):in_sz[2];
        // for the surrounding area
        for(long iix=startx;iix<(endx);iix++)
        {
            for(long iiy=starty;iiy<(endy);iiy++)
            {
                for(long iiz=startz;iiz<(endz);iiz++)
                {
                    im_transfer[iiz * sz01 + iiy * sz0 + iix]=inimg1d[iiz * sz01 + iiy * sz0 + iix];

                }
            }
        }
    }
    //erosion
    if(erosion_kernel_size)
        erosionImg(im_transfer,in_sz,erosion_kernel_size);
    //save img
    simple_saveimage_wrapper(callback, outpath.toStdString().c_str(),(unsigned char *)im_transfer,in_sz,1);
    //release pointer
     if(im_transfer) {delete []im_transfer; im_transfer=0;}
}
