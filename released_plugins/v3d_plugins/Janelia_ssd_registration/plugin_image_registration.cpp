//plugin_image_registration.cpp
//by Lei Qu 2011-04-08
// Add command line interface and multiple datatype support by Yang Yu Dec. 8, 2011
// 20120410 by PHC. some refinement.

#include "plugin_image_registration.h"

#include <QtGui>
#include <math.h>
#include <stdlib.h>
#include "stackutil.h"
#include "common/q_imresize.cpp"
#include "common/q_convolve.h"
#include "histogram_matching/q_histogram_matching.h"

#include "q_paradialog_rigidaffine.h"
#include "q_registration_common.h"
#include "q_rigidaffine_registration.h"

Q_EXPORT_PLUGIN2(plugin_image_registration, ImageRegistrationPlugin);

const QString title = "ImageRegistrationPlugin demo";

void RigidAffineRegistration(V3DPluginCallback &callback, QWidget *parent,const int i_regtype);
void FFDNonrigidRegistration(V3DPluginCallback &callback, QWidget *parent);
void releasememory_rigidaffine(unsigned char *&,unsigned char *&,unsigned char *&,unsigned char *&,double *&,double *&,unsigned char *&);
void releasememory_nonrigid_FFD(long *&,long *&,unsigned char *&,unsigned char *&,unsigned char *&,unsigned char *&,double *&,double *&,unsigned char *&);

QStringList ImageRegistrationPlugin::menulist() const
{
    return QStringList()
            << tr("rigid registration...")
            << tr("About");
}

QString version_str = "0.93";

void ImageRegistrationPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name==tr("rigid registration..."))
    {
        RigidAffineRegistration(callback, parent,0);
    }
    else if (menu_name==tr("About"))
    {
        v3d_msg(QString("This is a 3D SSD registration program developed in Hanchuan Peng lab. Version %1.").arg(version_str));
    }
}

void RigidAffineRegistration(V3DPluginCallback &callback, QWidget *parent,const int i_regtype)
{
    CParaDialog_rigidaffine DLG_rigidaffine(callback,parent);
    if(DLG_rigidaffine.exec()!=QDialog::Accepted)
        return;

    for(long ind_file=0;ind_file<DLG_rigidaffine.m_qsSelectedFiles_sub.size();ind_file++)
    {
        QString qs_filename_input=DLG_rigidaffine.m_qsSelectedFiles_sub[ind_file];
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_filename_output=DLG_rigidaffine.lineEdit_img_sub2tar->text();
        QString qs_pathname_output=QFileInfo(qs_filename_output).path();

        QString qs_filename_img_sub=qs_filename_input;
        QString qs_filename_img_sub2tar,qs_filename_swc_grid;
        if(DLG_rigidaffine.m_qsSelectedFiles_sub.size()>1)
        {
            qs_filename_img_sub2tar=qs_pathname_output+"/"+qs_basename_input+".tif";
            qs_filename_swc_grid=qs_pathname_output+"/"+qs_basename_input+".swc";
        }
        else
        {
            qs_filename_img_sub2tar=DLG_rigidaffine.lineEdit_img_sub2tar->text();
            qs_filename_swc_grid=DLG_rigidaffine.lineEdit_swc_grid->text();
        }

        QString qs_filename_img_tar=DLG_rigidaffine.lineEdit_img_tar->text();
        long l_refchannel=DLG_rigidaffine.lineEdit_refchannel->text().toLong()-1;
        long l_downsampleratio=DLG_rigidaffine.lineEdit_downsampleratio->text().toLong();
        bool b_histomatch=DLG_rigidaffine.checkBox_histogrammatching->isChecked();
        bool b_alignedges=DLG_rigidaffine.checkBox_alignedges->isChecked();
        bool b_gausmooth=DLG_rigidaffine.groupBox_Gaussian->isChecked();
        long l_gauradius=DLG_rigidaffine.lineEdit_Gau_radius->text().toLong();
        double d_gausigma=DLG_rigidaffine.lineEdit_Gau_sigma->text().toDouble();

        CParas_reg paras;
        paras.i_regtype=i_regtype;
        paras.b_alignmasscenter=1;
        paras.l_iter_max=DLG_rigidaffine.lineEdit_iter_max->text().toLong();
        paras.d_step_inimultiplyfactor=DLG_rigidaffine.lineEdit_step_multiplyfactor->text().toDouble();
        paras.d_step_annealingratio=DLG_rigidaffine.lineEdit_step_annealingrate->text().toDouble();
        paras.d_step_min=DLG_rigidaffine.lineEdit_step_min->text().toDouble();

        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        if(i_regtype==0) printf(">>Rigid registration\n");
        if(i_regtype==1) printf(">>Affine registration\n");
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        printf(">>input parameters:\n");
        printf(">>  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
        printf(">>  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
        printf(">>  input reference channel:      %ld\n",l_refchannel);
        printf(">>  input downsample ratio:       %ld\n",l_downsampleratio);
        printf(">>-------------------------\n");
        printf(">>output parameters:\n");
        printf(">>  output sub2tar image:         %s\n",qPrintable(qs_filename_img_sub2tar));
        printf(">>  output meshgrid apo:          %s\n",qPrintable(qs_filename_swc_grid));
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

        if(i_regtype!=0 && i_regtype!=1)
        {
            printf("ERROR: invalid i_regtype!\n");
            return;
        }
        if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
        {
            printf("ERROR: invalid input file name (target or subject)!\n");
            return;
        }
        if(l_refchannel<0 || l_refchannel>=4)
        {
            printf("ERROR: invalid reference channel (should start from 1 and less than 4)!\n");
            return;
        }

        long sz_img_tar_input[4],sz_img_sub_input[4];
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;
        unsigned char *p_img8u_tar=0,*p_img8u_sub=0;
        double *p_img64f_tar=0,*p_img64f_sub=0;
        unsigned char *p_img8u_sub_warp=0;

        printf("1. Read target and subject image. \n");
        int datatype_tar_input=0;
        if(!simple_loadimage_wrapper(callback,(char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            if (p_img_tar_input) {delete []p_img_tar_input; p_img_tar_input=0;}
            if (p_img_sub_input) {delete []p_img_sub_input; p_img_sub_input=0;}
            return;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!simple_loadimage_wrapper(callback,(char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            if (p_img_tar_input) {delete []p_img_tar_input; p_img_tar_input=0;}
            if (p_img_sub_input) {delete []p_img_sub_input; p_img_sub_input=0;}
            return;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(datatype_tar_input!=datatype_sub_input || (datatype_tar_input!=1
                                                      // && datatype_tar_input!=2
                                                      // && datatype_tar_input!=4
                                                      )) //by PHC 20111123
        {
            //printf("ERROR: input target and subject image have different datatype or datatype is not uint8/uint16/float32!\n");
            printf("ERROR: input target and subject image have different datatype or datatype is not uint8!\n");
            releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
            return;
        }
        if(sz_img_tar_input[3]<l_refchannel+1 || sz_img_sub_input[3]<l_refchannel+1)
        {
            printf("ERROR: invalid reference channel!\n");
            releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
            return;
        }

        printf("2. Extract reference channel. \n");
        {
            unsigned char *p_img_1c=0;
            if(!q_extractchannel(p_img_tar_input,sz_img_tar_input,l_refchannel,p_img_1c))
            {
                printf("ERROR: q_extractchannel() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            if(p_img8u_tar) 			{delete []p_img8u_tar;			p_img8u_tar=0;}
            p_img8u_tar=p_img_1c;		p_img_1c=0;
            if(p_img_tar_input) 		{delete []p_img_tar_input;		p_img_tar_input=0;}
            if(!q_extractchannel(p_img_sub_input,sz_img_sub_input,l_refchannel,p_img_1c))
            {
                printf("ERROR: q_extractchannel() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
            p_img8u_sub=p_img_1c;		p_img_1c=0;
            if(qs_filename_img_sub2tar==NULL)
                if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
        }

        printf("3. Downsample 1c images and make them have the same size. \n");
        long sz_img[4]={sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],1};
        double d_downsample_ratio_tar[3],d_downsample_ratio_sub[3];
        {
            for(int i=0;i<3;i++)
            {
                sz_img[i]=sz_img_tar_input[i]/double(l_downsampleratio)+0.5;
                d_downsample_ratio_tar[i]=sz_img_tar_input[i]/double(sz_img[i]);
                d_downsample_ratio_sub[i]=sz_img_sub_input[i]/double(sz_img[i]);
            }

            printf("\t>>downsample target image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img[0],sz_img[1],sz_img[2]);
            if(sz_img_tar_input[0]!=sz_img[0] || sz_img_tar_input[1]!=sz_img[1] || sz_img_tar_input[2]!=sz_img[2])
            {
                unsigned char *p_img_tmp=0;
                long sz_img_old[4]={sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],1};
                if(!q_imresize_3D(p_img8u_tar,sz_img_old,0,sz_img,p_img_tmp))
                {
                    printf("ERROR: q_imresize_3D() return false!\n");
                    releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                    return;
                }
                if(p_img8u_tar) 			{delete []p_img8u_tar;			p_img8u_tar=0;}
                p_img8u_tar=p_img_tmp;	p_img_tmp=0;
            }

            printf("\t>>downsample subject image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img[0],sz_img[1],sz_img[2]);
            if(sz_img_sub_input[0]!=sz_img[0] || sz_img_sub_input[1]!=sz_img[1] || sz_img_sub_input[2]!=sz_img[2])
            {
                unsigned char *p_img_tmp=0;
                long sz_img_old[4]={sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],1};
                if(!q_imresize_3D(p_img8u_sub,sz_img_old,0,sz_img,p_img_tmp))
                {
                    printf("ERROR: q_imresize_3D() return false!\n");
                    releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                    return;
                }
                if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
                p_img8u_sub=p_img_tmp;	p_img_tmp=0;
            }
        }

        long l_npixels=sz_img[0]*sz_img[1]*sz_img[2];

        if(b_histomatch)
        {
            printf("[optional] Match the histogram of subject to that of target. \n");
            unsigned char *p_img_tmp=0;

            if(!q_histogram_matching_1c(p_img8u_tar,sz_img,
                                        p_img8u_sub,sz_img,
                                        p_img_tmp))
            {
                printf("ERROR: q_histogram_matching_1c() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
            p_img8u_sub=p_img_tmp;	p_img_tmp=0;
        }

        printf("4. Convert image data from uint8 to double and scale to [0~1]. \n");
        {
            p_img64f_tar=new double[l_npixels]();
            p_img64f_sub=new double[l_npixels]();
            if(!p_img64f_tar || !p_img64f_sub)
            {
                printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }

            long l_maxintensity_tar,l_maxintensity_sub;
            l_maxintensity_tar=l_maxintensity_sub=255;
            for(long i=0;i<l_npixels;i++)
            {
                p_img64f_tar[i]=p_img8u_tar[i]/(double)l_maxintensity_tar;
                p_img64f_sub[i]=p_img8u_sub[i]/(double)l_maxintensity_sub;
            }

            if(p_img8u_tar) 			{delete []p_img8u_tar;			p_img8u_tar=0;}
            if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
        }

        if(b_alignedges)
        {
            printf("[optional] Generate gradient images. \n");
            double *p_img64f_gradnorm=0;
            //target
            if(!q_gradientnorm(p_img64f_tar,sz_img,1,p_img64f_gradnorm))
            {
                printf("ERROR: q_gradientnorm() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
            p_img64f_tar=p_img64f_gradnorm; p_img64f_gradnorm=0;
            //subject
            if(!q_gradientnorm(p_img64f_sub,sz_img,1,p_img64f_gradnorm))
            {
                printf("ERROR: q_gradientnorm() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
            p_img64f_sub=p_img64f_gradnorm; p_img64f_gradnorm=0;
        }

        if(b_gausmooth)
        {
            printf("[optional] Gaussian smooth input target and subject images. \n");
            long l_kenelradius=l_gauradius;
            double d_sigma=d_gausigma;
            vector<double> vec1D_kernel;
            if(!q_kernel_gaussian_1D(l_kenelradius,d_sigma,vec1D_kernel))
            {
                printf("ERROR: q_kernel_gaussian_1D() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }

            printf("\tsmoothing target image.\n");
            if(!q_convolve_img64f_3D_fast(p_img64f_tar,sz_img,vec1D_kernel))
            {
                printf("ERROR: q_convolve64f_3D_fast() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            printf("\tsmoothing subject image.\n");
            if(!q_convolve_img64f_3D_fast(p_img64f_sub,sz_img,vec1D_kernel))
            {
                printf("ERROR: q_convolve64f_3D_fast() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
        }


        //------------------------------------------------------------------------------------------------------------------------------------
        if(i_regtype==0) printf("5. Enter rigid registration iteration: \n");
        if(i_regtype==1) printf("5. Enter affine registration iteration: \n");

        vector< vector< vector< vector<double> > > > vec4D_grid;
        if(!q_rigidaffine_registration(paras,p_img64f_tar,p_img64f_sub,sz_img,vec4D_grid))
        {
            printf("ERROR: q_affine_registration_SSD() return false!\n");
            releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
            return;
        }
        if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
        if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}

        for(long x=0;x<2;x++)
            for(long y=0;y<2;y++)
                for(long z=0;z<2;z++)
                {
                    vec4D_grid[y][x][z][0]=(vec4D_grid[y][x][z][0]+1)*d_downsample_ratio_tar[0]-1;
                    vec4D_grid[y][x][z][1]=(vec4D_grid[y][x][z][1]+1)*d_downsample_ratio_tar[1]-1;
                    vec4D_grid[y][x][z][2]=(vec4D_grid[y][x][z][2]+1)*d_downsample_ratio_tar[2]-1;

                    //qDebug()<<"y x z "<<y<<x<<z<<" ... "<<vec4D_grid[y][x][z][0]<<vec4D_grid[y][x][z][1]<<vec4D_grid[y][x][z][2];
                }

        if(qs_filename_swc_grid!=NULL)
        {
            printf(">> Save deformed grid to swc file:[%s] \n",qPrintable(qs_filename_swc_grid));
            if(!q_rigidaffine_savegrid_swc(vec4D_grid,sz_img_tar_input,qPrintable(qs_filename_swc_grid)))
            {
                printf("ERROR: q_savegrid_swc() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
        }

        if(qs_filename_img_sub2tar!=NULL)
        {
            printf(">> Warp subject image according to the grid and save. \n");
            printf("\t>> resize the input subject image to the same size as target. \n");
            unsigned char *p_img_sub_resize=0;
            if(!q_imresize_3D(p_img_sub_input,sz_img_sub_input,0,sz_img_tar_input,p_img_sub_resize))
            {
                printf("ERROR: q_imresize_3D() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }

            printf("\t>> warped subject image according to deformed grid. \n");
            if(!q_rigidaffine_warpimage_baseongrid(paras.i_regtype,p_img_sub_resize,sz_img_tar_input,vec4D_grid,p_img8u_sub_warp))
            {
                printf("ERROR: q_warpimage_baseongrid() return false!\n");
                if(p_img_sub_resize) 			{delete []p_img_sub_resize;			p_img_sub_resize=0;}
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
            if(p_img_sub_resize) 			{delete []p_img_sub_resize;			p_img_sub_resize=0;}

            printf("\t>> Save warped subject image to file:[%s] \n",qPrintable(qs_filename_img_sub2tar));
            
            qDebug()<<"p_img8u_sub_warp="<<p_img8u_sub_warp;
            qDebug()<<sz_img_tar_input[0] << " " <<sz_img_tar_input[1] << " " <<sz_img_tar_input[2] << " " <<sz_img_tar_input[3]; 
            if(!simple_saveimage_wrapper(callback,qPrintable(qs_filename_img_sub2tar),p_img8u_sub_warp,sz_img_tar_input,1))
            {
                printf("ERROR: q_save64f01_image() return false!\n");
                releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
                return;
            }
        }

        printf(">>Free memory\n");
        releasememory_rigidaffine(p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);

    }
    printf(">>Program exit success!\n\n");
}

void releasememory_rigidaffine(unsigned char *&p_img_tar_input,unsigned char *&p_img_sub_input,
                               unsigned char *&p_img8u_tar,unsigned char *&p_img8u_sub,
                               double *&p_img64f_tar,double *&p_img64f_sub,
                               unsigned char *&p_img8u_output_sub)
{
    if(p_img_tar_input) 	{delete []p_img_tar_input;		p_img_tar_input=0;}
    if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
    if(p_img8u_tar) 		{delete []p_img8u_tar;			p_img8u_tar=0;}
    if(p_img8u_sub) 		{delete []p_img8u_sub;			p_img8u_sub=0;}
    if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
    if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
    if(p_img8u_output_sub)  {delete []p_img8u_output_sub;	p_img8u_output_sub=0;}
   // if(sz_img_tar_input) 	{delete []sz_img_tar_input;		sz_img_tar_input=0;}
   // if(sz_img_sub_input) 	{delete []sz_img_sub_input;		sz_img_sub_input=0;}
    printf("Release all memory done!\n");
}

// Add command line interface by Yang Yu Dec. 8, 2011

template<class Tdata>
void freeMemory(Tdata *&p)
{
    if(p) {delete[] p; p=NULL;}
}

template<class T1, class T2>
void freeMemory2(T1 *&p1, T2 *&p2)
{
    if(p1) {delete[] p1; p1=NULL;}
    if(p2) {delete[] p2; p2=NULL;}
}

// sampling image
template <class Tdata, class Tidx>
bool iSampler(Tdata *p, Tidx *subDims, Tdata *&pOutput, Tidx *tarDims, Tidx dimc)
{
    if(p==NULL || subDims==NULL || tarDims==NULL)
    {
        printf("\nError: The inputs for iSampler function are invalid!\n");
        return false;
    }

    // change x,y,z dims and keep color channels
    double arr_resize_ratio[3];
    arr_resize_ratio[0]=(double)(tarDims[0])/(double)(subDims[0]);
    arr_resize_ratio[1]=(double)(tarDims[1])/(double)(subDims[1]);
    arr_resize_ratio[2]=(double)(tarDims[2])/(double)(subDims[2]);

    //
    bool b_isint=1;
    Tdata tmp=1.1;
    if((tmp-1)>0.01) b_isint=0;

    //
    Tidx pagesz = tarDims[0]*tarDims[1]*tarDims[2];
    Tidx totalplxs = tarDims[3]*pagesz;

    freeMemory<Tdata>(pOutput);
    try
    {
        pOutput = new Tdata [totalplxs];
        memset(pOutput, 0, sizeof(Tdata)*totalplxs); // init
    }
    catch(...)
    {
        printf("Fail to allocate memory!\n");
        freeMemory<Tdata>(pOutput);
        return false;
    }

    //
    Tidx offset_z_input = subDims[1]*subDims[0];
    Tidx offset_z_output = tarDims[1]*tarDims[0];

    for(Tidx z=0;z<tarDims[2];z++)
    {
        for(Tidx y=0;y<tarDims[1];y++)
        {
            for(Tidx x=0;x<tarDims[0];x++)
            {
                double cur_pos[3];//x,y,z
                cur_pos[0]=(x+1)/arr_resize_ratio[0]-1;
                cur_pos[1]=(y+1)/arr_resize_ratio[1]-1;
                cur_pos[2]=(z+1)/arr_resize_ratio[2]-1;

                Tidx x_s,x_b,y_s,y_b,z_s,z_b;
                x_s=(Tidx)floor(cur_pos[0]);		x_b=(Tidx)ceil(cur_pos[0]);
                y_s=(Tidx)floor(cur_pos[1]);		y_b=(Tidx)ceil(cur_pos[1]);
                z_s=(Tidx)floor(cur_pos[2]);		z_b=(Tidx)ceil(cur_pos[2]);
                x_s=(x_s<0)?0:x_s;
                y_s=(y_s<0)?0:y_s;
                z_s=(z_s<0)?0:z_s;
                x_b=(x_b>subDims[0]-1)?(subDims[0]-1):x_b;
                y_b=(y_b>subDims[1]-1)?(subDims[1]-1):y_b;
                z_b=(z_b>subDims[2]-1)?(subDims[2]-1):z_b;

                double l_w,r_w,t_w,b_w;
                l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
                t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
                double u_w,d_w;
                u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

                for(long c=0;c<dimc;c++)
                {
                    Tidx idx = c*pagesz + z*offset_z_output + y*tarDims[0] + x;
                    Tidx offset_c_input = c*subDims[0]*subDims[1]*subDims[2];

                    double higher_slice;
                    higher_slice=t_w*(l_w*p[offset_c_input + z_s*offset_z_input + y_s*subDims[0] + x_s]+r_w*p[offset_c_input + z_s*offset_z_input + y_s*subDims[0] + x_b])+
                            b_w*(l_w*p[offset_c_input + z_s*offset_z_input + y_b*subDims[0] + x_s]+r_w*p[offset_c_input + z_s*offset_z_input + y_b*subDims[0] + x_b]);
                    double lower_slice;
                    lower_slice =t_w*(l_w*p[offset_c_input + z_b*offset_z_input + y_s*subDims[0] + x_s]+r_w*p[offset_c_input + z_b*offset_z_input + y_s*subDims[0] + x_b])+
                            b_w*(l_w*p[offset_c_input + z_b*offset_z_input + y_b*subDims[0] + x_s]+r_w*p[offset_c_input + z_b*offset_z_input + y_b*subDims[0] + x_b]);

                    if(b_isint)
                        pOutput[idx]=u_w*higher_slice+d_w*lower_slice+0.5;
                    else
                        pOutput[idx]=u_w*higher_slice+d_w*lower_slice;
                }
            }
        }
    }

    return true;
}

// Normalize image to double [0 1]
template <class Tdata, class Tidx>
bool iNormalizer(Tdata *p, Tidx sz, double *&pOutput)
{
    if(p==NULL || sz<=0)
    {
        printf("\nError: The inputs for iNormalizer function are invalid!\n");
        return false;
    }

    freeMemory<double>(pOutput);
    try
    {
        pOutput = new double [sz];
        memset(pOutput, 0, sizeof(Tdata)*sz); // init
    }
    catch(...)
    {
        printf("Fail to allocate memory!\n");
        freeMemory<double>(pOutput);
        return false;
    }

    Tdata max_v = p[0]; //note that already verified sz>=1
    Tdata min_v = p[0]; //note that already verified sz>=1
    for(V3DLONG i=1;i<sz;i++)
    {
        if (p[i]>max_v)
            max_v = p[i];
        else if (p[i]<min_v)
            min_v = p[i];
    }

    max_v -= min_v;
    
    if (max_v==0)
    {
        v3d_msg("You data consists of all-the-same values. You don't need registration, do you??\n", 0);
        freeMemory<double>(pOutput);
        return false;
    }
    
    for(long i=0;i<sz;i++)
    {
        pOutput[i] = (double(p[i]) - min_v)/double(max_v);
    }

    return true;
}

//
template<class Tdata, class Tidx>
bool iInterpolater(Tdata *p, Tidx *sz_img_input,
                   const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
                   Tdata *&pOutput)
{
    if(p==NULL || sz_img_input==NULL)
    {
        printf("\nError: The inputs for iInterpolater function are invalid!\n");
        return false;
    }

    if(V3DLONG(vec4D_grid_int.size()) != V3DLONG(sz_img_input[1]) || 
            V3DLONG(vec4D_grid_int[0].size()) != V3DLONG(sz_img_input[0]) ||
            V3DLONG(vec4D_grid_int[0][0].size()) != V3DLONG(sz_img_input[2]) ||
            V3DLONG(vec4D_grid_int[0][0][0].size()) != 3)
    {
        printf("\nERROR: Invalid input grid size, it should be same as input image size!\n");
        return false;
    }

    //
    bool b_isint=1;
    Tdata tmp=(Tdata)1.1;
    if((tmp-1)>0.01) b_isint=0;

    //
    Tidx pagesz = sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
    Tidx offset_z_input = sz_img_input[1]*sz_img_input[0];

    freeMemory<Tdata>(pOutput);
    V3DLONG totalplxs = sz_img_input[3]*pagesz;
    try
    {
        pOutput = new Tdata [totalplxs];
        memset(pOutput, 0, totalplxs); // init
    }
    catch(...)
    {
        printf("Fail to allocate memory!\n");
        freeMemory<Tdata>(pOutput);
        return false;
    }

    for(Tidx x=0;x<sz_img_input[0];x++)
        for(Tidx y=0;y<sz_img_input[1];y++)
            for(Tidx z=0;z<sz_img_input[2];z++)
            {
                double cur_pos[3];
                cur_pos[0]=vec4D_grid_int[y][x][z][0];
                cur_pos[1]=vec4D_grid_int[y][x][z][1];
                cur_pos[2]=vec4D_grid_int[y][x][z][2];

                if(fabs(cur_pos[0])<1e-10)	cur_pos[0]=0.0;
                if(fabs(cur_pos[1])<1e-10)	cur_pos[1]=0.0;
                if(fabs(cur_pos[2])<1e-10)	cur_pos[2]=0.0;
                if(cur_pos[0]<0 || cur_pos[0]>sz_img_input[0]-1 || cur_pos[1]<0 || cur_pos[1]>sz_img_input[1]-1 || cur_pos[2]<0 || cur_pos[2]>sz_img_input[2]-1)
                {
                    continue;
                }

                Tidx x_s,x_b,y_s,y_b,z_s,z_b;
                x_s=(Tidx)floor(cur_pos[0]);		x_b=(Tidx)ceil(cur_pos[0]);
                y_s=(Tidx)floor(cur_pos[1]);		y_b=(Tidx)ceil(cur_pos[1]);
                z_s=(Tidx)floor(cur_pos[2]);		z_b=(Tidx)ceil(cur_pos[2]);

                double l_w,r_w,t_w,b_w;
                l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
                t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
                double u_w,d_w;
                u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

                for(Tidx c=0;c<sz_img_input[3];c++)
                {
                    Tidx offset_c_input = c*pagesz;
                    Tidx idx = offset_c_input + z*offset_z_input + y*sz_img_input[0] + x;

                    double higher_slice;
                    higher_slice=t_w*(l_w*p[offset_c_input + z_s*offset_z_input + y_s*sz_img_input[0] + x_s]+r_w*p[offset_c_input + z_s*offset_z_input + y_s*sz_img_input[0] + x_b])+
                            b_w*(l_w*p[offset_c_input + z_s*offset_z_input + y_b*sz_img_input[0] + x_s]+r_w*p[offset_c_input + z_s*offset_z_input + y_b*sz_img_input[0] + x_b]);
                    double lower_slice;
                    lower_slice =t_w*(l_w*p[offset_c_input + z_b*offset_z_input + y_s*sz_img_input[0] + x_s]+r_w*p[offset_c_input + z_b*offset_z_input + y_s*sz_img_input[0] + x_b])+
                            b_w*(l_w*p[offset_c_input + z_b*offset_z_input + y_b*sz_img_input[0] + x_s]+r_w*p[offset_c_input + z_b*offset_z_input + y_b*sz_img_input[0] + x_b]);

                    if(b_isint)
                        pOutput[idx] = Tdata(u_w*higher_slice+d_w*lower_slice+0.5);
                    else
                        pOutput[idx] = Tdata(u_w*higher_slice+d_w*lower_slice);
                }

            }

    //
    return true;
}

//
template <class Tdata, class Tidx>
bool iWarper(Tdata *p,Tidx *sz_img,
             const vector< vector< vector< vector<double> > > > &vec4D_grid,
             Tdata *&pOutput)
{
    if(p==NULL || sz_img==NULL || vec4D_grid.size()==0)
    {
        printf("\nError: The inputs for iWarper function are invalid!\n");
        return false;
    }

    //
    vector< vector< vector< vector<double> > > > vec4D_grid_int;
    if(!q_rigidaffine_grid2field_3D(0,vec4D_grid,sz_img,sz_img,vec4D_grid_int))
    {
        printf("ERROR: q_rigidaffine_grid2field_3D() return false!\n");
        return false;
    }

    //
    if(!iInterpolater<Tdata>(p,sz_img,vec4D_grid_int,pOutput))
    {
        printf("ERROR: iInterpolater() return false!\n");
        freeMemory2<Tdata, Tdata>(p, pOutput);
        return false;
    }

    //
    return true;
}

template <class Tdata, class Tidx>
bool saturateImage(Tdata *&p, Tidx sx, Tidx sy, Tidx sz, float percentage)
{
    // error check
    if(!p || percentage<0 || percentage>1)
    {
        qDebug()<<"Invalid input!"<<endl;
        return false;
    }

    //
    Tidx pagesz = sx*sy*sz;

    Tdata maxori = 0, minori = 0;
    for(Tidx i=0; i<pagesz; i++)
    {
        if(p[i]>maxori)
            maxori = p[i];
        if(p[i]<minori)
            minori = p[i];
    }

    Tdata MAXV=maxori;
    if(sizeof(Tdata)==1)
    {
        if(MAXV<255) MAXV=255; // 8 bit
    }
    else if(sizeof(Tdata)==2)
    {
        if(MAXV<4096) MAXV=4096; // 12 bit 
    }

    Tidx NBIN = maxori+1;

    //Look Up Table
    Tdata *LUT = NULL;
    Tidx *hist = NULL;
    Tidx *accuhist = NULL;

    try
    {
        LUT = new Tdata [NBIN];
        hist = new Tidx [NBIN];
        accuhist = new Tidx [NBIN];
    }
    catch(...)
    {
        qDebug()<<"Fail to allocate memory for LUT";
        return false;
    }

    for(Tidx i=0; i<NBIN; i++)
    {
        LUT[i]=0;
        hist[i]=0;
        accuhist[i]=0;
    }

    Tidx curv;
    //compute histogram
    for(Tidx i=0; i<pagesz; i++)
    {
        curv = p[i];
        hist[curv]++;
    }
    //statistic accumulated histogram
    for(Tidx i=0; i<NBIN; i++)
    {
        for(Tidx j=0; j<=i; j++)
        {
            accuhist[i] += hist[j];
        }
    }

    //compute histogram satuate percentage (e.g. 98%)
    Tdata ileft = minori, iright = ileft;
    double sflag = 0;
    while(sflag<percentage)
    {
        sflag = double(accuhist[iright++])/double(pagesz);
    }
    iright--;

    //compute LUT
    for (Tidx i=ileft; i<=iright; i++)
    {
        LUT[i] = (Tdata)(MAXV*double(i-ileft)/double(iright-ileft));
    }

    //output
    for (Tidx i=0;i<pagesz;i++)
    {
        if(p[i]<ileft) p[i] = 0;
        else if(p[i]>iright) p[i] = MAXV;
        else
        {
            Tidx curv = p[i];
            p[i] = LUT[curv];
        }
    }

    if (LUT) {delete []LUT; LUT=0;}
    if (hist) {delete []hist; hist=0;}
    if (accuhist) {delete []accuhist; accuhist=0;}

    return true;
}

void errorPrint()
{
    printf("\nUsage: vaa3d -x imagereg.dylib (or equivalent dll name) "
    		"-f rigidreg -o <output_warped_image> "
    		"-p \"^t <template.img> ^ct <refchannel_template> "
    		"     ^s <source.img> ^cs <refchannel_source> "
    		"     ^ds <downsample_ratio> ^n <max_iters> "
    		"     [^m <step_inimultiplyfactor> ^a <step_annealing_rate> ^ms <min_step>]\"\n");
    printf("Reference channel numbers must start from 1.\n");
    v3d_msg(QString("This is a 3D SSD registration program developed in Hanchuan Peng lab. Version %1.").arg(version_str), 0);
}

// function call

//bool wrapperLoadImage(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype)
//{
//	v3d_msg("wrapperLoadImage", 0);
	
//	if (!imgSrcFile || data1d || sz)
//	{
//		v3d_msg("wrong input to wrapperLoadImage().",0);
//		return false;
//	}
	
//	Image4DSimple *tmpimg=new Image4DSimple;
//	tmpimg->loadImage(imgSrcFile);
//	if (!(tmpimg->valid()))
//	{
//		v3d_msg(QString("Fail to load image [%1]").arg(imgSrcFile),0);
//		return false;
//	}
	
//    // The following few lines are to avoid disturbing the existing code below
    
//    data1d = tmpimg->getRawData();
//    datatype=tmpimg->getDatatype();
//    sz=new V3DLONG[4];
//    sz[0]=tmpimg->getXDim();
//    sz[1]=tmpimg->getYDim();
//    sz[2]=tmpimg->getZDim();
//    sz[3]=tmpimg->getCDim();
    
//    return true;
//}

QStringList ImageRegistrationPlugin::funclist() const
{
    return QStringList() << tr("rigidreg") << tr("help");
}

bool ImageRegistrationPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("rigidreg"))
    {
        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        QString qs_filename_img_sub, qs_filename_img_tar;
        QString qs_filename_img_sub2tar,qs_filename_swc_grid;

        long l_downsampleratio=4;

        CParas_reg rrparas;
        rrparas.i_regtype=0;
        rrparas.b_alignmasscenter=1;
        rrparas.l_iter_max=500;
        rrparas.d_step_inimultiplyfactor=5;
        rrparas.d_step_annealingratio=0.95;
        rrparas.d_step_min=0.01;

        // parsing parameters
        qDebug() << "infile=["<<infile<<"]";
        qDebug() << "outfile=["<<outfile<<"]";
        qDebug() << "para=["<<paras<<"]";
        
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[2048];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                //qDebug()<<"argv"<<i<<"["<<argv[i]<<"]";
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '^')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                qDebug()<<"qs_filename_img_tar=["<<qs_filename_img_tar<<"]";
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                qDebug()<<"qs_filename_img_sub=["<<qs_filename_img_sub<<"]";
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "ds"))
                            {
                                key++;
                                l_downsampleratio = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "n"))
                            {
                                rrparas.l_iter_max = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                rrparas.d_step_inimultiplyfactor = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "a"))
                            {
                                rrparas.d_step_annealingratio = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ms"))
                            {
                                key++;
                                rrparas.d_step_min = atof( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }

                }
            }
            
            if (argv) {delete []argv; argv=0;}

            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();

            qs_filename_swc_grid=qs_pathname_output+"/"+qs_basename_input+".swc";

            if(outfile)
            {
                qs_filename_img_sub2tar=qs_filename_output;
            }
            else
            {
                qs_filename_img_sub2tar=qs_pathname_output+"/"+qs_basename_input+".v3draw";
            }

            // error check
            if(qs_filename_img_tar==NULL)
            {
                printf("\nERROR: invalid input target file name!\n");
                errorPrint();
                return false;
            }
            if(qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input subject file name!\n");
                errorPrint();
                return false;
            }
            if(channel_ref_sub<0)
            {
                printf("\nERROR: invalid reference channel for the subject image! Assume R(1)G(2)B(3) ...!\n");
                errorPrint();
                return false;
            }
            if(channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel for the target image! Assume R(1)G(2)B(3) ...!\n");
                errorPrint();
                return false;
            }

            printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            if(rrparas.i_regtype==0) printf(">>Rigid registration\n");
            if(rrparas.i_regtype==1) printf(">>Affine registration\n");
            printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            printf(">>input parameters:\n");
            printf(">>  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
            printf(">>  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
            printf(">>  input ref channel of sub:     %ld\n",channel_ref_sub+1);
            printf(">>  input ref channel of tar:     %ld\n",channel_ref_tar+1);
            printf(">>  input downsample ratio:       %ld\n",l_downsampleratio);
            printf(">>-------------------------\n");
            printf(">>output parameters:\n");
            printf(">>  output sub2tar image:         %s\n",qPrintable(qs_filename_img_sub2tar));
            // printf(">>  output meshgrid apo:          %s\n",qPrintable(qs_filename_swc_grid));
            printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

        }

        V3DLONG sz_img_tar_input[4], sz_img_sub_input[4];
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;
        double *p_img64f_tar=0,*p_img64f_sub=0;
        unsigned char *p_img_sub_warp=0;

        int process_step = 0;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Read target and subject images. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        int datatype_tar_input=0;

        if(!simple_loadimage_wrapper(callback,(char *)qPrintable(qs_filename_img_tar),
        		p_img_tar_input,
        		sz_img_tar_input,
        		datatype_tar_input))
        {
            if (p_img_tar_input) {delete []p_img_tar_input; p_img_tar_input=0;}
            if (p_img_sub_input) {delete []p_img_sub_input; p_img_sub_input=0;}
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!simple_loadimage_wrapper(callback,(char *)qPrintable(qs_filename_img_sub),
        		p_img_sub_input,
        		sz_img_sub_input,
        		datatype_sub_input))
        {
            if (p_img_tar_input) {delete []p_img_tar_input; p_img_tar_input=0;}
            if (p_img_sub_input) {delete []p_img_sub_input; p_img_sub_input=0;}
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");
            if (p_img_tar_input) {delete []p_img_tar_input; p_img_tar_input=0;}
            if (p_img_sub_input) {delete []p_img_sub_input; p_img_sub_input=0;}

            errorPrint();

            return false;
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Downsample the subject and the target to the same dimensions. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        V3DLONG sz_img[4]={sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],1};

        double d_downsample_ratio_tar[3],d_downsample_ratio_sub[3];
        for(int i=0;i<3;i++)
        {
            // 2D case
            if(sz_img_tar_input[i]==1)
            {
                sz_img[i]=1;
            }
            else
            {
                sz_img[i]=(V3DLONG)((double)sz_img_tar_input[i]/(double)(l_downsampleratio) + 0.5);
            }


            d_downsample_ratio_tar[i]=double(sz_img_tar_input[i])/double(sz_img[i]);
            d_downsample_ratio_sub[i]=double(sz_img_sub_input[i])/double(sz_img[i]);
        }

        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG offsets_sub = channel_ref_sub*sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_tar = channel_ref_tar*sz_img_tar_input[0]*sz_img_tar_input[1]*sz_img_tar_input[2];

        if(sz_img_tar_input[0]!=sz_img[0] || sz_img_tar_input[1]!=sz_img[1] || sz_img_tar_input[2]!=sz_img[2])
        {
            switch (datatype_tar_input) {
            case 1:
            {
                if(!iSampler<unsigned char, V3DLONG>(p_img_tar_input+offsets_tar, sz_img_tar_input, p_tar, sz_img, sz_img[3]))
                {
                    printf("\nError: fail to call iSampler function!\n");
                    return false;
                }
            }
            break;

            case 2:
            {
                unsigned short int *pTmp = NULL;

                if(!iSampler<unsigned short int, V3DLONG>((unsigned short int *)p_img_tar_input+offsets_tar, sz_img_tar_input, pTmp, sz_img, sz_img[3]))
                {
                    printf("\nError: fail to call iSampler function!\n");
                    return false;
                }
                p_tar = (unsigned char *)pTmp; pTmp = NULL;
            }
            break;

            case 4:
            {
                float *pTmp = NULL;

                if(!iSampler<float, V3DLONG>((float *)p_img_tar_input+offsets_tar, sz_img_tar_input, pTmp, sz_img, sz_img[3]))
                {
                    printf("\nError: fail to call iSampler function!\n");
                    return false;
                }
                p_tar = (unsigned char *)pTmp; pTmp = NULL;
            }
            break;

            default:
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                //need to free memory here?? potential bug
                return false;
            }
            }
        }
        else
        {
            p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;
        }

        if(sz_img_sub_input[0]!=sz_img[0] || sz_img_sub_input[1]!=sz_img[1] || sz_img_sub_input[2]!=sz_img[2])
        {
            switch (datatype_sub_input) {
            case 1:
            {
                if(!iSampler<unsigned char, V3DLONG>(p_img_sub_input+offsets_sub, sz_img_sub_input, p_sub, sz_img, sz_img[3]))
                {
                    printf("\nError: fail to call iSampler function!\n");
                    return false;
                }
            }
            break;

            case 2:
            {
                unsigned short int *pTmp = NULL;

                if(!iSampler<unsigned short int, V3DLONG>((unsigned short int *)p_img_sub_input+offsets_sub, sz_img_sub_input, pTmp, sz_img, sz_img[3]))
                {
                    printf("\nError: fail to call iSampler function!\n");
                    return false;
                }
                p_sub = (unsigned char *)pTmp; pTmp = NULL;
            }
            break;

            case 4:
            {
                float *pTmp = NULL;

                if(!iSampler<float, V3DLONG>((float *)p_img_sub_input+offsets_sub, sz_img_sub_input, pTmp, sz_img, sz_img[3]))
                {
                    printf("\nError: fail to call iSampler function!\n");
                    return false;
                }
                p_sub = (unsigned char *)pTmp; pTmp = NULL;
            }
            break;

            default:
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
            }
        }
        else
        {
            p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Saturate the reference of the subject image. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------
        V3DLONG pagesz_img = sz_img[0]*sz_img[1]*sz_img[2];
        switch (datatype_sub_input)
        {
        case 1:
        {
            if(!saturateImage<unsigned char, V3DLONG>(p_sub, sz_img[0], sz_img[1], sz_img[2], 0.98))
            {
                printf("\nError: fail to call saturateImage function!\n");
                return false;
            }
        }
        break;

        case 2:
        {
            unsigned short *pTmp = (unsigned short *)p_sub;

            if(!saturateImage<unsigned short, V3DLONG>(pTmp, sz_img[0], sz_img[1], sz_img[2], 0.98))
            {
                printf("\nError: fail to call saturateImage function!\n");
                return false;
            }
        }
        break;

        case 4:
        {
            qDebug()<<"32bit not supported!";
        }
        break;

        default:
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Convert data to double type and scale to [0~1]. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------
        switch (datatype_tar_input) {
        case 1:
        {
            if(!iNormalizer<unsigned char, V3DLONG>(p_tar, pagesz_img, p_img64f_tar))
            {
                printf("\nError: fail to call iNormalizer function!\n");
                return false;
            }
        }
        break;

        case 2:
        {
            if(!iNormalizer<unsigned short int, V3DLONG>((unsigned short int*)p_tar, pagesz_img, p_img64f_tar))
            {
                printf("\nError: fail to call iNormalizer function!\n");
                return false;
            }
        }
        break;

        case 4:
        {
            if(!iNormalizer<float, V3DLONG>((float *)p_tar, pagesz_img, p_img64f_tar))
            {
                printf("\nError: fail to call iNormalizer function!\n");
                return false;
            }
        }
        break;

        default:
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
        break;
        }
        freeMemory<unsigned char>(p_tar);

        //
        switch (datatype_sub_input) {
        case 1:
        {
            if(!iNormalizer<unsigned char, V3DLONG>(p_sub, pagesz_img, p_img64f_sub))
            {
                printf("\nError: fail to call iNormalizer function!\n");
                return false;
            }
        }
        break;

        case 2:
        {
            if(!iNormalizer<unsigned short int, V3DLONG>((unsigned short int*)p_sub, pagesz_img, p_img64f_sub))
            {
                printf("\nError: fail to call iNormalizer function!\n");
                return false;
            }
        }
        break;

        case 4:
        {
            if(!iNormalizer<float, V3DLONG>((float *)p_sub, pagesz_img, p_img64f_sub))
            {
                printf("\nError: fail to call iNormalizer function!\n");
                return false;
            }
        }
        break;

        default:
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
        }
        freeMemory<unsigned char>(p_sub);

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Obtain gradient surfaces of the subject and the target. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        double *p_img64f_gradnorm=NULL;

        if(!q_gradientnorm(p_img64f_tar,sz_img,1,p_img64f_gradnorm))
        {
            printf("\nERROR: q_gradientnorm() return false!\n");
            return false;
        }
        freeMemory<double>(p_img64f_tar);
        p_img64f_tar=p_img64f_gradnorm;

        p_img64f_gradnorm=NULL;
        if(!q_gradientnorm(p_img64f_sub,sz_img,1,p_img64f_gradnorm))
        {
            printf("\nERROR: q_gradientnorm() return false!\n");
            return false;
        }
        freeMemory<double>(p_img64f_sub);
        p_img64f_sub=p_img64f_gradnorm;
        p_img64f_gradnorm=NULL;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Rigid registration iteration: \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // rigid registration
        size_t start_t = clock();

        vector< vector< vector< vector<double> > > > vec4D_grid;
        if(!q_rigidaffine_registration(rrparas,p_img64f_tar,p_img64f_sub,sz_img,vec4D_grid))
        {
            printf("\nERROR: q_rigidaffine_registration() return false!\n");
            return false;
        }
        freeMemory<double>(p_img64f_tar);
        freeMemory<double>(p_img64f_sub);

        for(long x=0;x<2;x++)
            for(long y=0;y<2;y++)
                for(long z=0;z<2;z++)
                {
                    vec4D_grid[y][x][z][0]=(vec4D_grid[y][x][z][0]+1)*d_downsample_ratio_tar[0]-1;
                    vec4D_grid[y][x][z][1]=(vec4D_grid[y][x][z][1]+1)*d_downsample_ratio_tar[1]-1;
                    vec4D_grid[y][x][z][2]=(vec4D_grid[y][x][z][2]+1)*d_downsample_ratio_tar[2]-1;
                }

        size_t t_rigidreg = clock() - start_t;
        printf("\n Time elapse %f seconds for rigid registration!\n", float(t_rigidreg)/1.0e6f);

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Save the output. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        if(qs_filename_img_sub2tar!=NULL)
        {
            printf(">> Warp subject image according to the grid and save. \n");

            unsigned char *p_img_sub_resize=NULL;
            V3DLONG sz_img_output[4] = {sz_img_tar_input[0], sz_img_tar_input[1], sz_img_tar_input[2], sz_img_sub_input[3]};

            if(sz_img_sub_input[0]!=sz_img_output[0] || sz_img_sub_input[1]!=sz_img_output[1] || sz_img_sub_input[2]!=sz_img_output[2])
            {
                printf("\t>> resize the input subject image to the same size as target. \n");

                if(datatype_sub_input == 1)
                {
                    if(!iSampler<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input, p_img_sub_resize, sz_img_output, sz_img_sub_input[3]))
                    {
                        printf("\nError: fail to call iSampler function!\n");
                        return false;
                    }
                }
                else if(datatype_sub_input == 2)
                {
                    unsigned short *pTmp = NULL;

                    if(!iSampler<unsigned short, V3DLONG>((unsigned short *)p_img_sub_input, sz_img_sub_input, pTmp, sz_img_output, sz_img_sub_input[3]))
                    {
                        printf("\nError: fail to call iSampler function!\n");
                        return false;
                    }
                    p_img_sub_resize = (unsigned char *)pTmp; pTmp = NULL;
                }
                else if(datatype_sub_input == 4)
                {
                    float *pTmp = NULL;

                    if(!iSampler<float, V3DLONG>((float *)p_img_sub_input, sz_img_sub_input, pTmp, sz_img_output, sz_img_sub_input[3]))
                    {
                        printf("\nError: fail to call iSampler function!\n");
                        return false;
                    }
                    p_img_sub_resize = (unsigned char *)pTmp; pTmp = NULL;
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return false;
                }
            }
            else
            {
                p_img_sub_resize=p_img_sub_input;
            }

            printf("\t>> warped subject image according to deformed grid. \n");
            if(datatype_sub_input == 1)
            {
                if(!iWarper<unsigned char>(p_img_sub_resize,sz_img_output,vec4D_grid,p_img_sub_warp))
                {
                    printf("ERROR: iInterpolater() return false!\n");
                    freeMemory2<unsigned char, unsigned char>(p_img_sub_resize, p_img_sub_warp);
                    return false;
                }
                if(!simple_saveimage_wrapper(callback,qPrintable(qs_filename_img_sub2tar),p_img_sub_resize,sz_img_output,datatype_sub_input))
                {
                    printf("ERROR: saveImage() return false!\n");
                    return false;
                }
            }
            else if(datatype_sub_input == 2)
            {
                unsigned short *pTmp = NULL;

                if(!iWarper<unsigned short>((unsigned short *)p_img_sub_resize,sz_img_output,vec4D_grid,pTmp))
                {
                    printf("ERROR: iInterpolater() return false!\n");
                    freeMemory2<unsigned char, unsigned char>(p_img_sub_resize, p_img_sub_warp);
                    return false;
                }
                p_img_sub_warp = (unsigned char *)pTmp; pTmp = NULL;
            }
            else if(datatype_sub_input == 4)
            {
                float *pTmp = NULL;

                if(!iWarper<float>((float *)p_img_sub_resize,sz_img_output,vec4D_grid,pTmp))
                {
                    printf("ERROR: iInterpolater() return false!\n");
                    freeMemory2<unsigned char, unsigned char>(p_img_sub_resize, p_img_sub_warp);
                    return false;
                }
                p_img_sub_warp = (unsigned char *)pTmp; pTmp = NULL;
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
            freeMemory<unsigned char>(p_img_sub_resize);

            printf("\t>> Save warped subject image to file:[%s] \n",qPrintable(qs_filename_img_sub2tar));
            if(!simple_saveimage_wrapper(callback,qPrintable(qs_filename_img_sub2tar),p_img_sub_warp,sz_img_output,datatype_sub_input))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }

        }

        //
        freeMemory<unsigned char>(p_img_sub_warp);
        freeMemory2<unsigned char, unsigned char>(p_img_tar_input, p_img_sub_input); //a crash happen here why?? PHC 20120410. also the image is all black for 2D SSD, why. Also a crash for do_menu for 2D case too. need debug.

        printf("\n Time elapse %f seconds for warping!\n", float(clock() - t_rigidreg)/1.0e6f);

        //
        return true;
    }
    else //    if (func_name == tr("help"))
    {
        errorPrint();
        return true;
    }
    
    return false;
}
