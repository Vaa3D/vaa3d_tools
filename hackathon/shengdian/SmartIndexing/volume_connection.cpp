#include "volume_connection.h"
#include <fstream>
bool volume_signals(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For processing a volume, get the foreground voxels
     * input a dir, for all the tifs in dir
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    int hist_bins = 10; int to_8bit = 1;
    if(inparas.size() >= 1) hist_bins = atoi(inparas.at(0));
    if(inparas.size() >= 2) to_8bit = atoi(inparas.at(1));
   if(hist_bins<1)
       hist_bins=10;
//    char * inimg_file
    string inimg_file=  infiles[0];

    unsigned char * inimg1d = 0, *inimg1d_8bit =0;
    V3DLONG  in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    cout<<"data type = "<<datatype<<endl;
    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
    V3DLONG min_value=0,max_value=255;
    int hist_bin_voxel=int((max_value-min_value)/hist_bins);
    V3DLONG voxel_nums[hist_bins];
    for(int i=0;i<hist_bins;i++)
        voxel_nums[i]=0;
    double imgAve, imgStd;
    if(datatype>1&&to_8bit>0){
        cout<<"image convert to 8bit"<<endl;
        img_2_8bit(inimg1d,inimg1d_8bit,in_sz,datatype);
        mean_and_std(inimg1d_8bit, tol_sz, imgAve, imgStd);
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            int voxel_val=inimg1d_8bit[i];
            int voxel_range_i=int(voxel_val/hist_bin_voxel);
            voxel_nums[voxel_range_i]++;
        }
    }
    else{
        if(datatype==2)
            max_value=65535;
        else if(datatype>2)
            return false;
        hist_bin_voxel=int((max_value-min_value)/hist_bins);
        mean_and_std(inimg1d, tol_sz, imgAve, imgStd);
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            int voxel_val=inimg1d[i];
            int voxel_range_i=int(voxel_val/hist_bin_voxel);
            voxel_nums[voxel_range_i]++;
        }
    }

    QString outfile=(outfiles.size()>=1)?outfiles[0]:(QString::fromStdString(inimg_file)+"_intensity_hist_count.csv");
    QFile qf_anofile(outfile);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        cout<<"Cannot open file for writing!"<<endl;
        return false;
    }

    QTextStream out(&qf_anofile);
    QString img_basename=QFileInfo(QString::fromStdString(inimg_file)).baseName();
    qint64 img_size=QFileInfo(QString::fromStdString(inimg_file)).size();
    out<<"file,"<<img_basename<<endl;
    out<<"size,"<<img_size<<endl;
    out<<"mean_val,"<<QString::number(imgAve)<<endl;
    out<<"std_val,"<<QString::number(imgStd)<<endl;
    for(int i=0;i<hist_bins;i++)
        out<<"hist_"<<QString::number(i)<<","<<QString::number(voxel_nums[i])<<endl;
    delete [] inimg1d_8bit; inimg1d_8bit = 0;
    return true;
}
bool volume_filter(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For processing a volume, get the foreground voxels
     * 16-bit 3D volume :
     *
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    cout<<"Welcome to Volume filter !"<<endl;
    double bkg_thresh = 0; int to_8bit = 0;
    if(inparas.size() >= 1) bkg_thresh = atof(inparas.at(0));
    if(inparas.size() >= 2) to_8bit = atoi(inparas.at(1));

    char * inimg_file =  infiles[0];
    char * outimg_file = outfiles[0];

    unsigned char * inimg1d = 0, *inimg1d_8bit =0;
    V3DLONG  in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,inimg_file, inimg1d, in_sz, datatype)) {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    cout<<"data type = "<<datatype<<endl;
    img_2_8bit(inimg1d,inimg1d_8bit,in_sz,datatype);

    V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
    if(bkg_thresh<=0)
    {
        double imgAve, imgStd;
        mean_and_std(inimg1d_8bit, tol_sz, imgAve, imgStd);
        cout<<"imgAve = "<<imgAve<<" ; image std = "<<imgStd<<endl;
        bkg_thresh=MIN(MAX(imgAve+imgStd+15,30),100);
    }
    cout<<"bkg_thresh = "<<bkg_thresh<<endl;
//    outimg1d = new unsigned char[tol_sz];
    unsigned char  * outimg1d = 0;
    try {outimg1d = new unsigned char [tol_sz];}
    catch(...)  {cout<<"cannot allocate memory for thresholding image."<<endl; return false;}
    for(V3DLONG i = 0; i < tol_sz; i++)
    {
        outimg1d[i]=(inimg1d_8bit[i]>=bkg_thresh)?int(inimg1d_8bit[i]):0;
    }
    in_sz[3]=1;
    simple_saveimage_wrapper(callback,outimg_file, (unsigned char *)outimg1d, in_sz, 1);
    delete [] outimg1d; outimg1d = 0;
    return true;
}
bool terafly_vol_index(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){
    /* For a terafly volume in one resolution, index the related volumes in other resolution.
     * Input:
            * 1. terafly image path
            * 2. volume path
      * output dir
      * input para:
            * 1. volume res-level
            * 2. request res-level
       * Note:
            * 1. filename of terafly-volume is the coordinates of top-left-corner
            * 2. terafly-volume represents an index of virtual space in next-higher resolution.
            * 3. the volume-data-size is not equal with index-space.
            * 4. only volume size in highest-resolution represents index range of volume.
    */
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.size()<2){cerr<<"Input error: at least two input paras"<<endl; return false;}
    QString inimg_file=  infiles[0];
    string invol_file=  infiles[1];

    int vol_res = 3; int to_res = 0;
    if(inparas.size() >= 1) vol_res = atoi(inparas.at(0));
    if(inparas.size() >= 2) to_res = atoi(inparas.at(1));
    QString outfile_path=(outfiles.size()>=1)?outfiles[0]:(QString::fromStdString(invol_file)+".csv");
    QFile outfile(outfile_path);
    if(!outfile.open(QIODevice::WriteOnly))
    {
        cout<<"Cannot open file for writing!"<<endl;
        return false;
    }
    //parse terafly-data folder
    QDir inimg_path(inimg_file);
    if(!inimg_path.exists()){cerr<<"Input error: not an exist input folder"<<endl; return false;}
    inimg_path.setFilter(QDir::Dirs|QDir::NoSymLinks);
    inimg_path.setSorting(QDir::DirsFirst);
    QFileInfoList rawdirs=inimg_path.entryInfoList();
    QFileInfoList rawterafly_dirs, sorted_terafly_dirs;
    for(int i=0;i<rawdirs.size();i++){
        QFileInfo thisdir=rawdirs.at(i);
        if(thisdir.fileName()=="."|thisdir.fileName()=="..")
            continue;
        if(thisdir.fileName().left(3)=="RES")
            rawterafly_dirs.append(thisdir);
    }
    // sort input terafly-res-dirs : from highest res to lowest res
    for(int i=1;i<rawterafly_dirs.size();i++){
        for(int j=1;j<=(rawterafly_dirs.size()-i);j++){
            QFileInfo thisdir=rawterafly_dirs.at(j-1);
            QFileInfo nextdir=rawterafly_dirs.at(j);
            QString this_res_cords=thisdir.fileName();
            QString next_res_cords=nextdir.fileName();
            long coordx=this_res_cords.split("x").at(1).toLong();
            long next_coordx=next_res_cords.split("x").at(1).toLong();
            if(coordx>next_coordx){
                QFileInfo tmp_resdir=rawterafly_dirs.at(j-1);
                rawterafly_dirs[j-1]=rawterafly_dirs[j];
                rawterafly_dirs[j]=tmp_resdir;
            }
        }
    }
    for(int i=0;i<rawterafly_dirs.size();i++)
        sorted_terafly_dirs.append(rawterafly_dirs.at(rawterafly_dirs.size()-i-1));

    if(vol_res>sorted_terafly_dirs.size() -1 || vol_res < 0) {cerr<<"Input para error of inputing volume res-index"<<endl; return false;}
    if(to_res>sorted_terafly_dirs.size() -1 || to_res < 0) {cerr<<"Input para error of requesting volumes' res-index"<<endl; return false;}
    QStringList wholeSize=sorted_terafly_dirs.at(0).fileName().mid(4).split("x");
    V3DLONG wRES[3];
    wRES[1]=wholeSize.at(0).toLong();
    wRES[0]=wholeSize.at(1).toLong();
    wRES[2]=wholeSize.at(2).split(")").at(0).toLong();
    cerr<<"image size: "<<wRES[0]<<","<<wRES[1]<<","<<wRES[2]<<endl;

    //get volume virtual range
    /*
     * this step is for mapping filename with volume coordinates
     * The filename of TeraFly volume is volume's coordinates coded in 6 numbers.
    */
    int default_terafly_res_code=6;
    int res_code_num=MAX(QString::number(wRES[0]).length(),QString::number(wRES[1]).length());
    res_code_num=MAX(QString::number(wRES[2]).length(),res_code_num);
    QStringList invol_cords=QFileInfo(QString::fromStdString(invol_file)).baseName().split("_");
    V3DLONG volCs[3],volCe[3];
    volCs[0] =invol_cords.at(1).toLong();
    volCs[1]= invol_cords.at(0).toLong();
    volCs[2]= invol_cords.at(2).toLong();
    if(default_terafly_res_code>res_code_num)
        for(int s=0;s<3;s++)
            volCs[s]/=(10*(default_terafly_res_code-res_code_num));
    if(volCs[0]<0||volCs[0]>wRES[0]-1||
            volCs[1]<0||volCs[1]>wRES[1]-1||
            volCs[2]<0||volCs[2]>wRES[2]-1) {
        cerr<<"Error of input volumes' top-left coordinates"<<endl;
        return false;
    }
    cerr<<"vol start: "<<volCs[0]<<","<<volCs[1]<<","<<volCs[2]<<endl;

    /* get volume image size    */
    unsigned char * inimg1d = 0;
    V3DLONG  in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,(char*)invol_file.c_str(), inimg1d, in_sz, datatype)) {
        cerr<<"load image "<<invol_file<<" error!"<<endl;
        return false;
    }
    V3DLONG volX = in_sz[0]; V3DLONG volY = in_sz[1]; V3DLONG volZ = in_sz[2];
    cout<<"volume size: "<<volX<<","<<volY<<","<<volZ<<endl;
    delete [] inimg1d; inimg1d = 0;
    for(int s=0;s<3;s++)
        volCe[s]=volCs[s]+in_sz[s]*pow(2,vol_res);
    if(volCe[0]<0||volCe[0]>wRES[0]-1||
            volCe[1]<0||volCe[1]>wRES[1]-1||
            volCe[2]<0||volCe[2]>wRES[2]-1) {
        cerr<<"Error of input volumes' bottom-right coordinates"<<endl;
        return false;
    }
    cerr<<"vol end: "<<volCe[0]<<","<<volCe[1]<<","<<volCe[2]<<endl;
    //get related volumes of requesting resolution
    QFileInfo toRes_dir=sorted_terafly_dirs.at(to_res);
    QList < QList<V3DLONG> > to_vol_Lists;
    //for wRESi in [wRESX,wRESY,wRESZ]
    for(int s=0;s<3;s++)
    {
        // initialization
        V3DLONG wRESi=wRES[s];
        V3DLONG volis=volCs[s]; V3DLONG volie=volCe[s];
        QList<V3DLONG> to_vol_list; to_vol_list.clear();
        //get vol sizes
        V3DLONG volRes_dir_size = ceil(wRESi/(256*pow(2,to_res)));
//        cerr<<"volRes_dir_size: "<<volRes_dir_size<<","<<256*pow(2,to_res)<<endl;
        V3DLONG vol2_range=256*pow(2,to_res);
        V3DLONG vol1_num=volRes_dir_size;
        for(V3DLONG i=pow(2,to_res)*256-1;i>=(pow(2,to_res)-1)*256;i--){
            V3DLONG bro_nums=wRESi-volRes_dir_size*i;
            if(bro_nums>0){
                vol2_range=i;
                vol1_num=bro_nums;
                break;
            }
        }
        V3DLONG vol1_range=vol2_range+1;
        V3DLONG vol2_num=volRes_dir_size-vol2_num;
//        cerr<<"vol1: "<<vol1_range<<", number="<<vol1_num<<endl;
//        cerr<<"vol2: "<<vol2_range<<", number="<<vol2_num<<endl;
        //get start image name
        if(volis/vol1_range<vol1_num){
            // in vol1 range
            if(volie/vol1_range<=vol1_num)
                for(V3DLONG i=V3DLONG(volis/vol1_range);i<=V3DLONG(volie/vol1_range);i++)
                    to_vol_list.append(i*vol1_range);
            else{
                for(V3DLONG i=V3DLONG(volis/vol1_range);i<vol1_num;i++)
                    to_vol_list.append(i*vol1_range);
                for(V3DLONG i=0;i<=V3DLONG((volie-vol1_num*vol1_range)/vol2_range);i++)
                    to_vol_list.append(i*vol2_range+vol1_num*vol1_range);
            }
        }
        else{
            //all in vol2 range
            V3DLONG vol1_Len=vol1_num*vol1_range;
//            cerr<<"vol1_Len="<<vol1_Len<<", start ="<<V3DLONG((volis-vol1_Len)/vol2_range)<<endl;
            for(V3DLONG i=V3DLONG((volis-vol1_Len)/vol2_range);i<=V3DLONG((volie-vol1_Len)/vol2_range);i++)
                to_vol_list.append(i*vol2_range+vol1_Len);
        }
//        for(int l=0;l<to_vol_list.size();l++)
//            cout<<to_vol_list.at(l)<<",";
        to_vol_Lists.append(to_vol_list);
//        break;
    }

    /* map to real volume file
     * RES(YxXxZ)/Y0/X0/Y0_X0_Z0.tif
    */
    QStringList tovols;
    QList<V3DLONG> to_vol_Xlist=to_vol_Lists.at(0);
    QList<V3DLONG> to_vol_Ylist=to_vol_Lists.at(1);
    QList<V3DLONG> to_vol_Zlist=to_vol_Lists.at(2);
    for(V3DLONG iy=0;iy<to_vol_Ylist.size();iy++)
    {
        QString YD=QString::number(10*to_vol_Ylist.at(iy));
        YD="000000"+YD; YD=YD.right(6);
        for(V3DLONG ix=0;ix<to_vol_Xlist.size();ix++)
        {
            QString XD=QString::number(10*to_vol_Xlist.at(ix));
            XD="000000"+XD; XD=XD.right(6);
            for(V3DLONG iz=0;iz<to_vol_Zlist.size();iz++)
            {
                QString ZD=QString::number(10*to_vol_Zlist.at(iz));
                ZD="000000"+ZD; ZD=ZD.right(6);
                //file path
                QString tovol = inimg_file + "/" + toRes_dir.fileName() +"/" +YD +"/" +YD+"_"+XD +"/" +YD +"_" +XD +"_" +ZD + ".tif";
                QFileInfo tovol_file(tovol);
                if(tovol_file.isFile())
                    tovols.append(tovol);
            }
        }
    }
    // out to text file
    cout<<"Related volumes="<<tovols.size()<<endl;
    QTextStream out(&outfile);
    for(V3DLONG i=0;i<tovols.size();i++)
        out<<tovols.at(i)<<endl;
    return true;
}
// data type conversion
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt)
{
     if (!pre1d ||!pPost || imsz<=0 )
     {
          v3d_msg("Invalid parameters to converting().", 0);
          return;
     }

    Tpre *pPre = (Tpre *)pre1d;

    if(v3d_dt == V3D_UINT8)
    {
        Tpre max_v=0, min_v = 255;

        for(V3DLONG i=0; i<imsz; i++)
        {
            if(max_v<pPre[i]) max_v = pPre[i];
            if(min_v>pPre[i]) min_v = pPre[i];
        }
        max_v -= min_v;

        if(max_v>0)
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) 255*(double)(pPre[i] - min_v)/max_v;
        }
        else
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) pPre[i];
        }
    }
    else if(v3d_dt == V3D_UINT16)
    {
        Tpre max_v=0, min_v = 65535;

        for(V3DLONG i=0; i<imsz; i++)
        {
            if(max_v<pPre[i]) max_v = pPre[i];
            if(min_v>pPre[i]) min_v = pPre[i];
        }
        max_v -= min_v;

        if(max_v>0)
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) 65535*(double)(pPre[i] - min_v)/max_v;
        }
        else
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) pPre[i];
        }

    }
    else if(v3d_dt == V3D_FLOAT32)
    {
        for(V3DLONG i=0; i<imsz; i++)
            pPost[i] = (Tpost) pPre[i];
    }

}
bool img_2_8bit(unsigned char * & src, unsigned char * & data1d, V3DLONG * in_sz, int src_dt)
{
    V3DLONG	sz_sub = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
    try
    {
        data1d = new unsigned char [sz_sub];
    }
    catch(...)
    {
        printf("Error allocating memory. \n");
        return false;
    }

    if(src_dt == 1)
    {
        converting<unsigned char, unsigned char>((unsigned char *)src, data1d, sz_sub, V3D_UINT8);
    }
    else if(src_dt == 2)
    {
        converting<unsigned short, unsigned char>((unsigned short *)src, data1d, sz_sub, V3D_UINT8);
    }
    else if(src_dt == 4)
    {
        converting<float, unsigned char>((float *)src, data1d, sz_sub, V3D_UINT8);
    }
    return true;
}
