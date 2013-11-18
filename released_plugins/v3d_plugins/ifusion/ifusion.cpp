/* ifusion.cpp
 * 2011-08-31: the program is created by Yang Yu
 */

#include <QtGui>

#include <cmath>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <iostream>

//#include "basic_surf_objs.h"
//#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

//#include "mg_utilities.h"
//#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

#include "../istitch/y_imglib.h"

#include "ifusion.h"

using namespace std;

//
// image normalization and image blending
//
// inormalizer: normalizing and linear blending
// iblender: linear blending
//

//
Q_EXPORT_PLUGIN2(ifusion, ImageFusionPlugin);

//plugin
const QString title = "Image Fusion";

//
class AdjustPara
{
public:
    void init(V3DLONG n)
    {
        a=NULL;
        b=NULL;
        
        try 
        {
            a = new float [n];
            b = new float [n];
            
            memset(a, 0, sizeof(float)*n);
            memset(b, 0, sizeof(float)*n);
        } 
        catch (...) 
        {
            printf("Fail to allocate memory for a and b!\n");
            
            clean();
            
            return;
        }
    }
    
    void clean()
    {
        if(a){delete []a; a=NULL;}
        if(b){delete []b; b=NULL;}
    }
    
public:
    float *a, *b; // r, g, b, ref
    V3DLONG num_tile;
};

// normalizing func
template<class Tdata>
bool computeAdjustPara(Tdata *f, Tdata *g, V3DLONG szimg, float &a, float &b)
{
    //
    a=0.0; b=0.0;
    
    //
//    float sumfg=0.0;
//    float sumgg=0.0;
    
    float meanf=0.0;
    float meang=0.0;
    
    float N = (float) szimg;
    float coef = sqrt( (N-1.0)/N );
    
    for (V3DLONG i=0; i<szimg; i++) 
    {
        float valf=(float)f[i];
        float valg=(float)g[i];
        
        meanf += valf;
        meang += valg;
        
//        sumfg += valf*valg;
//        sumgg += valg*valg;
    }
    meanf /= N;
    meang /= N;
    
    // least square
    // a might be negative, this will introduce some errors, will fix it later
//    a = (sumfg - (float)szimg*meanf*meang)/(sumgg - (float)szimg*meang*meang);
//    b = meanf - a*meang;


    // use mean and standard deviation to solve a and b
    // in this way, a will always be positive
    float stdf=0.0, stdg=0.0;
    for (V3DLONG i=0; i<szimg; i++) 
    {
        float valf=(float)f[i];
        float valg=(float)g[i];
        
        stdf += (valf-meanf)*(valf-meanf);
        stdg += (valg-meang)*(valg-meang);
    }
    stdf /= (N-1.0);
    stdg /= (N-1.0);

    stdf = sqrt(stdf);
    stdg = sqrt(stdg);

    a = stdf/stdg*coef;
    b = meanf - a*meang;
    
    return true;
}

// normalizing func
template<class Tdata>
bool findHighestIntensityLevelTile(Tdata *f, Tdata *g, V3DLONG szimg, V3DLONG &tilen)
{
    float meanf=0.0;
    float meang=0.0;
    
    float N = (float) szimg;
    float coef = sqrt( (N-1.0)/N );
    
    for (V3DLONG i=0; i<szimg; i++) 
    {
        float valf=(float)f[i];
        float valg=(float)g[i];
        
        meanf += valf;
        meang += valg;
    }
    meanf /= N;
    meang /= N;
    
    
    qDebug()<<"mean value ..."<<meanf<<meang;
    
    return true;
}

// reconstruct tiles into one stitched image
template <class Tdata> 
bool ireconstructingwnorm(Tdata *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, Tdata intensityrange, QList<AdjustPara> adjparaList,V3DPluginCallback2 &callback)
{    
    //
    V3DLONG vx = vim.sz[0];
    V3DLONG vy = vim.sz[1];
    V3DLONG vz = vim.sz[2];
    V3DLONG vc = vim.sz[3];
    
    V3DLONG pagesz = vx*vy*vz;
    V3DLONG imgsz = pagesz*vc;
    
    float *pTmp = NULL;
    try {
        pTmp = new float [imgsz];
        memset(pTmp, 0.0, sizeof(float)*imgsz);
    } catch (...) {
        printf("Fail to allocate memory for pTmp!\n");
        if(pTmp){delete []pTmp; pTmp=NULL;}
        return false;
    }
    
    // fusion
    string fn;
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        // loading relative imagg files
        V3DLONG sz_relative[4];
        int datatype_relative = 0;
        unsigned char* relative1d = 0;
        
        //
        if(ii==0) 
        {   
            fn = vim.lut[ii].fn_img;
        }
        else 
        {
            QString curPath = QFileInfo(QString(vim.lut[0].fn_img.c_str())).path();;
            
            fn = curPath.append("/").append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
        }
        
        //
        if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
            return false;
        }
        V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
        
        //
        V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
        V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];
        
        V3DLONG x_start = (0 > tile2vi_xs) ? 0 : tile2vi_xs;
        V3DLONG x_end = (vx-1 < tile2vi_xe) ? vx-1 : tile2vi_xe;
        V3DLONG y_start = (0 > tile2vi_ys) ? 0 : tile2vi_ys;
        V3DLONG y_end = (vy-1 < tile2vi_ye) ? vy-1 : tile2vi_ye;
        V3DLONG z_start = (0 > tile2vi_zs) ? 0 : tile2vi_zs;
        V3DLONG z_end = (vz-1 < tile2vi_ze) ? vz-1 : tile2vi_ze;
        
        x_end++;
        y_end++;
        z_end++;
        
        //suppose all tiles with same color dimensions
        if(rc>vc)
            rc = vc;

        V3DLONG itile;
        for(itile=0; itile<adjparaList.size(); itile++)
        {
            if(ii==adjparaList.at(itile).num_tile) break;
        }
        
        //
        Tdata *prelative = (Tdata *)relative1d;
        
        for(V3DLONG c=0; c<rc; c++)
        {
            V3DLONG o_c = c*vx*vy*vz;
            V3DLONG o_r_c = c*rx*ry*rz;
            for(V3DLONG k=z_start; k<z_end; k++)
            {
                V3DLONG o_k = o_c + k*vx*vy;
                V3DLONG o_r_k = o_r_c + (k-z_start)*rx*ry;
                
                for(V3DLONG j=y_start; j<y_end; j++)
                {
                    V3DLONG o_j = o_k + j*vx;
                    V3DLONG o_r_j = o_r_k + (j-y_start)*rx;
                    for(V3DLONG i=x_start; i<x_end; i++)
                    {
                        V3DLONG idx = o_j + i;
                        V3DLONG idx_r = o_r_j + (i-x_start);
                        
                        float val = (float)(prelative[idx_r]);

                        if(itile<adjparaList.size())
                        {
                            val = val*adjparaList.at(itile).a[c] + adjparaList.at(itile).b[c]; // normalizing
                        }
                        
                        //
                        float coef;
                        if(!computeWeights<REAL>(vim, i, j, k, ii, coef) )
                        {
                            printf("Fail to call function computeWeights!\n");
                            return false;
                        }
                        
                        pTmp[idx] += val*coef; // linear blending                        
                    }
                }
            }
        }
        
        //de-alloc
        if(relative1d) {delete []relative1d; relative1d=0;}
      //  if(sz_relative) {delete []sz_relative; sz_relative=0;}
    }
    
    float minval, maxval;
    for(V3DLONG c=0; c<vc; c++) 
    {
        V3DLONG offsets = c*pagesz;
        
        minval=1e9;
        maxval=-1e9;
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            float val=pTmp[idx];
            
            if(minval>val) minval = val;
            if(maxval<val) maxval = val;
        }
        maxval -= minval;
        
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            pVImg[idx] = (Tdata) (intensityrange * (pTmp[idx] - minval)/maxval);
        }
    }
    
    //de-alloc
    if(pTmp) {delete []pTmp; pTmp=NULL;}
    
    return true;
}

// reconstruct tiles into one stitched image
template <class Tdata> 
bool ireconstructing(Tdata *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, Tdata intensityrange,V3DPluginCallback2 &callback)
{    
    //
    V3DLONG vx = vim.sz[0];
    V3DLONG vy = vim.sz[1];
    V3DLONG vz = vim.sz[2];
    V3DLONG vc = vim.sz[3];
    
    V3DLONG pagesz = vx*vy*vz;
    V3DLONG imgsz = pagesz*vc;
    
    float *pTmp = NULL;
    try {
        pTmp = new float [imgsz];
        memset(pTmp, 0.0, sizeof(float)*imgsz);
    } catch (...) {
        printf("Fail to allocate memory for pTmp!\n");
        if(pTmp){delete []pTmp; pTmp=NULL;}
        return false;
    }
    
    // fusion
    string fn;
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        // loading relative imagg files
        V3DLONG sz_relative[4];
        int datatype_relative = 0;
        unsigned char* relative1d = 0;
        
        //
        if(ii==0) 
        {   
            fn = vim.lut[ii].fn_img;
        }
        else 
        {
            QString curPath = QFileInfo(QString(vim.lut[0].fn_img.c_str())).path();;
            
            fn = curPath.append("/").append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
        }
        
        //
        if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
            return false;
        }
        V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
        
        //
        V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
        V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];
        
        V3DLONG x_start = (0 > tile2vi_xs) ? 0 : tile2vi_xs;
        V3DLONG x_end = (vx-1 < tile2vi_xe) ? vx-1 : tile2vi_xe;
        V3DLONG y_start = (0 > tile2vi_ys) ? 0 : tile2vi_ys;
        V3DLONG y_end = (vy-1 < tile2vi_ye) ? vy-1 : tile2vi_ye;
        V3DLONG z_start = (0 > tile2vi_zs) ? 0 : tile2vi_zs;
        V3DLONG z_end = (vz-1 < tile2vi_ze) ? vz-1 : tile2vi_ze;
        
        x_end++;
        y_end++;
        z_end++;
        
        //suppose all tiles with same color dimensions
        if(rc>vc)
            rc = vc;
        
        //
        Tdata *prelative = (Tdata *)relative1d;
        
        for(V3DLONG c=0; c<rc; c++)
        {
            V3DLONG o_c = c*vx*vy*vz;
            V3DLONG o_r_c = c*rx*ry*rz;
            for(V3DLONG k=z_start; k<z_end; k++)
            {
                V3DLONG o_k = o_c + k*vx*vy;
                V3DLONG o_r_k = o_r_c + (k-z_start)*rx*ry;
                
                for(V3DLONG j=y_start; j<y_end; j++)
                {
                    V3DLONG o_j = o_k + j*vx;
                    V3DLONG o_r_j = o_r_k + (j-y_start)*rx;
                    for(V3DLONG i=x_start; i<x_end; i++)
                    {
                        V3DLONG idx = o_j + i;
                        V3DLONG idx_r = o_r_j + (i-x_start);
                        
                        float val = (float)(prelative[idx_r]);
                        
                        //
                        float coef;
                        if(!computeWeights<REAL>(vim, i, j, k, ii, coef) )
                        {
                            printf("Fail to call function computeWeights!\n");
                            return false;
                        }
                        
                        pTmp[idx] += (Tdata)(val*coef); // linear blending
                        
                    }
                }
            }
        }
        
        //de-alloc
        if(relative1d) {delete []relative1d; relative1d=0;}
      //  if(sz_relative) {delete []sz_relative; sz_relative=0;}
    }
    
    float minval, maxval;
    for(V3DLONG c=0; c<vc; c++) 
    {
        V3DLONG offsets = c*pagesz;
        
        minval=1e9;
        maxval=-1e9;
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            float val=pTmp[idx];
            
            if(minval>val) minval = val;
            if(maxval<val) maxval = val;
        }
        maxval -= minval;
        
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            pVImg[idx] = (Tdata) (intensityrange * (pTmp[idx] - minval)/maxval);
        }
    }
    
    //de-alloc
    if(pTmp) {delete []pTmp; pTmp=NULL;}
    
    return true;
}


// funcs
QStringList ImageFusionPlugin::funclist() const
{
    return QStringList() << "iblender"<<"inormalizer";
}

bool ImageFusionPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (func_name == tr("iblender")) // linear blending
    {
        // parsing parameters
        if(input.size()<1) return false; // no inputs
        
        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty()) 
        {
            //print Help info
            printf("\nUsage: v3d -x ifusion.dylib -f iblender -i <folder> -o <output_image> -p \"#s <save_blending_result zero(false)/nonzero(true)>\"\n");
            
            return true;
        }        
        
        char * infile = infilelist->at(0); // input images
        char * paras = NULL; // parameters
        char * outfile = NULL; // outputs
        
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        
        bool b_saveimage = true; // save the blended image by default
        
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t') 
                    myparas[i]='\0';
            }
            
            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];
                    
                    qDebug()<<">>key ..."<<key;
                    
                    if (*key == '#')
                    {
                        while(*++key)
                        {                            
                            if (!strcmp(key, "s"))
                            {                                
                                b_saveimage = (atoi( argv[i+1] ))?true:false;                                
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                            
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                    
                }
            }
        }
        
        QString blendImageName;
        
        // get stitch configuration        
        QDir myDir(infile);
        QStringList list = myDir.entryList(QStringList("*.tc"));
        
        if(list.size()!=1)
        {
            printf("Must have only one stitching configuration file!\n");
            return false;
        }
        
        if(!outfile) 
            blendImageName = QString(infile).append("/stitched.v3draw");
        else
            blendImageName = QString(outfile);
        
        if(QFileInfo(blendImageName).suffix().toUpper() != "V3DRAW")
        {
            blendImageName.append(".v3draw"); // force to save as .v3draw file
        }
        
        // load config
        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;
        
        QString tcfile = QString(infile).append("/").append(list.at(0));
        
        if( !vim.y_load(tcfile.toStdString()) )
        {
            printf("Wrong stitching configuration file to be load!\n");
            return false;
        }
        
        // do blending
        V3DLONG vx, vy, vz, vc;

        vx = vim.sz[0]; //
        vy = vim.sz[1];
        vz = vim.sz[2];
        vc = vim.sz[3];

        V3DLONG pagesz_vim = vx*vy*vz*vc;
        
        int datatype_tile = 0; // assume all tiles with the same datatype
        for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
        {
            if(ii>0) break; // get datatype
            
            // load tile
            V3DLONG sz_relative[4];
            unsigned char* relative1d = 0;
            
            QString curPath = QFileInfo(tcfile).path();;
            
            string fn = curPath.append("/").append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
            
            vim.lut[ii].fn_img = fn; // absolute path
            
            if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
                return -1;
            }
            
            // de-alloca
            if(relative1d) {delete []relative1d; relative1d=NULL;}
        }
        if(datatype_tile == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;

            try
            {
                pVImg = new unsigned char [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory for pVImg.\n");
                if(pVImg) {delete []pVImg; pVImg=NULL;}
                return false;
            }

            //
            bool success = ireconstructing<unsigned char>((unsigned char *)pVImg, vim, 255,callback);
            if(!success)
            {
                printf("Fail to call function ireconstructing! \n");
                return false;
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (simple_saveimage_wrapper(callback,blendImageName.toStdString().c_str(), (unsigned char *)pVImg, vim.sz, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;

            try
            {
                pVImg = new unsigned short [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory for pVImg.\n");
                if(pVImg) {delete []pVImg; pVImg=NULL;}
                return false;
            }

            //
            bool success = ireconstructing<unsigned short>((unsigned short *)pVImg, vim, 4095,callback);
            if(!success)
            {
                printf("Fail to call function ireconstructing! \n");
                return false;
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (simple_saveimage_wrapper(callback,blendImageName.toStdString().c_str(), (unsigned char *)pVImg, vim.sz, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }

        }
        else if(datatype_tile == V3D_FLOAT32)
        {
            // current not supported
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
    }
    else if (func_name == tr("inormalizer")) // normalizing and linear blending
    {
        // parsing parameters
        if(input.size()<1) return false; // no inputs
        
        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty()) 
        {
            //print Help info
            printf("\nUsage: v3d -x ifusion.dylib -f inormalizer -i <folder> -o <output_image> -p \"#s <save_blending_result zero(false)/nonzero(true)>\"\n");
            
            return true;
        }        
        
        char * infile = infilelist->at(0); // input images
        char * paras = NULL; // parameters
        char * outfile = NULL; // outputs
        
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        
        bool b_saveimage = true; // save the blended image by default
        
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t') 
                    myparas[i]='\0';
            }
            
            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];
                    
                    qDebug()<<">>key ..."<<key;
                    
                    if (*key == '#')
                    {
                        while(*++key)
                        {                            
                            if (!strcmp(key, "s"))
                            {                                
                                b_saveimage = (atoi( argv[i+1] ))?true:false;                                
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                            
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                    
                }
            }
        }
        
        QString blendImageName;
        
        // get stitch configuration        
        QDir myDir(infile);
        QStringList list = myDir.entryList(QStringList("*.tc"));
        
        if(list.size()!=1)
        {
            printf("Must have only one stitching configuration file!\n");
            return false;
        }
        
        if(!outfile) 
            blendImageName = QString(infile).append("/normalized_stitched.v3draw");
        else
            blendImageName = QString(outfile);
        
        if(QFileInfo(blendImageName).suffix().toUpper() != "V3DRAW")
        {
            blendImageName.append(".v3draw"); // force to save as .v3draw file
        }
        
        // load config
        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;
        
        QString tcfile = QString(infile).append("/").append(list.at(0));
        
        if( !vim.y_load(tcfile.toStdString()) )
        {
            printf("Wrong stitching configuration file to be load!\n");
            return false;
        }
        
        // do blending
        V3DLONG vx, vy, vz, vc;
        
        vx = vim.sz[0];
        vy = vim.sz[1];
        vz = vim.sz[2];
        vc = vim.sz[3];
        
        V3DLONG pagesz_vim = vx*vy*vz*vc;
        
        int datatype_tile = 0; // assume all tiles with the same datatype
        for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
        {
            if(ii>0) break; // get datatype
            
            // load tile
            V3DLONG sz_relative[4];
            unsigned char* relative1d = 0;
            
            QString curPath = QFileInfo(tcfile).path();
            
            string fn = curPath.append("/").append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
            
            vim.lut[ii].fn_img = fn; // absolute path
            
            if (simple_loadimage_wrapper(callback, const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
                return false;
            }
            
            // de-alloca
            if(relative1d) {delete []relative1d; relative1d=NULL;}
        }
        
        // the first tile chosen to be a reference image, all other tile will be normalized to this reference image
        QList<V3DLONG> normList, tileList;
        QList<AdjustPara> adjparaList;
        
        //normList<<0; // choose the first tile as the reference to normalize
        // find the highest mean value of overlap region as the reference tile
        V3DLONG reftile=0;
        QList<V3DLONG> aList, bList;
        aList<<0;
        for(V3DLONG ii=1; ii<vim.number_tiles; ii++)
        {
            bList.append(ii);
        }
        while (!bList.isEmpty()) 
        {
            // pick one from tileList
            V3DLONG j=bList.at(0);
            bList.pop_front();
            
            V3DLONG i;
            
            V3DLONG jx_s, jy_s, jz_s, jx_e, jy_e, jz_e;
            V3DLONG ix_s, iy_s, iz_s, ix_e, iy_e, iz_e;
            
            jx_s = vim.lut[j].start_pos[0] + vim.min_vim[0];
            jy_s = vim.lut[j].start_pos[1] + vim.min_vim[1];
            jz_s = vim.lut[j].start_pos[2] + vim.min_vim[2];
            
            jx_e = vim.lut[j].end_pos[0] + vim.min_vim[0];
            jy_e = vim.lut[j].end_pos[1] + vim.min_vim[1];
            jz_e = vim.lut[j].end_pos[2] + vim.min_vim[2];
            
            bool findoverlap = false;
            for (V3DLONG ii=0; ii<aList.size(); ii++) 
            {
                // pick one from normList
                i=aList.at(ii);
                
                ix_s = vim.lut[i].start_pos[0] + vim.min_vim[0];
                iy_s = vim.lut[i].start_pos[1] + vim.min_vim[1];
                iz_s = vim.lut[i].start_pos[2] + vim.min_vim[2];
                
                ix_e = vim.lut[i].end_pos[0] + vim.min_vim[0];
                iy_e = vim.lut[i].end_pos[1] + vim.min_vim[1];
                iz_e = vim.lut[i].end_pos[2] + vim.min_vim[2];
                
                // find whether they have common overlap region
                if(  ( (ix_s>=jx_s && ix_s<=jx_e) || (ix_e>=jx_s && ix_e<=jx_e) || (jx_s>=ix_s && jx_s<=ix_e) || (jx_e>=ix_s && jx_e<=ix_e) )
                   && ( (iy_s>=jy_s && iy_s<=jy_e) || (iy_e>=jy_s && iy_e<=jy_e) || (jy_s>=iy_s && jy_s<=iy_e) || (jy_e>=iy_s && jy_e<=iy_e) )
                   && ( (iz_s>=jz_s && iz_s<=jz_e) || (iz_e>=jz_s && iz_e<=jz_e) || (jz_s>=iz_s && jz_s<=iz_e) || (jz_e>=iz_s && jz_e<=iz_e) ) )
                {
                    findoverlap = true;
                    break;
                }
            }
            
            // find a and b
            if(findoverlap)
            {
                // load image f
                V3DLONG sz_f[4];
                int datatype_f = 0;
                unsigned char* f1d = 0;
                
                string fn;
                
                // j <-> f
                if(j==0) 
                {   
                    fn = vim.lut[j].fn_img;
                }
                else 
                {
                    QString curPath = QFileInfo(QString(vim.lut[0].fn_img.c_str())).path();;
                    
                    fn = curPath.append("/").append( QString(vim.lut[j].fn_img.c_str()) ).toStdString();
                }
                
                //
                if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), f1d, sz_f, datatype_f)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",fn.c_str());
                    return false;
                }
                V3DLONG fx=sz_f[0], fy=sz_f[1], fz=sz_f[2], fc=sz_f[3];
                
                // load image g
                V3DLONG sz_g[4];
                int datatype_g = 0;
                unsigned char* g1d = 0;
                
                // i <-> g
                if(i==0) 
                {   
                    fn = vim.lut[i].fn_img;
                }
                else 
                {
                    QString curPath = QFileInfo(QString(vim.lut[0].fn_img.c_str())).path();;
                    
                    fn = curPath.append("/").append( QString(vim.lut[i].fn_img.c_str()) ).toStdString();
                }
                
                //
                if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), g1d, sz_g, datatype_g)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",fn.c_str());
                    return false;
                }
                V3DLONG gx=sz_g[0], gy=sz_g[1], gz=sz_g[2], gc=sz_g[3];
                
                // overlap image from f and g
                V3DLONG start_x = qMax(jx_s, ix_s);
                V3DLONG start_y = qMax(jy_s, iy_s);
                V3DLONG start_z = qMax(jz_s, iz_s);
                
                V3DLONG end_x = qMin(jx_e, ix_e);
                V3DLONG end_y = qMin(jy_e, iy_e);
                V3DLONG end_z = qMin(jz_e, iz_e);
                
                V3DLONG dimxol= end_x-start_x+1;
                V3DLONG dimyol= end_y-start_y+1;
                V3DLONG dimzol= end_z-start_z+1;
                
                V3DLONG pagesz_ol = dimxol*dimyol*dimzol;
                
                float *fol1d = NULL;
                float *gol1d = NULL;
                
                try 
                {
                    // suppose fc = gc = vc
                    fol1d = new float [fc*pagesz_ol];
                    gol1d = new float [gc*pagesz_ol];
                } 
                catch (...) 
                {
                    printf("Fail to allocate memory for fol1d and gol1d.\n");
                    
                    if (fol1d) {delete []fol1d; fol1d=NULL;}
                    if (gol1d) {delete []gol1d; gol1d=NULL;}
                    
                    return false;
                }
                
                for(V3DLONG c=0; c<fc; c++)
                {
                    V3DLONG offsets_fc = c*fx*fy*fz;
                    V3DLONG offsets_gc = c*gx*gy*gz;
                    
                    V3DLONG offsets_olc = c*pagesz_ol;
                    
                    for(V3DLONG z=start_z; z<end_z; z++)
                    {
                        V3DLONG offsets_fz = offsets_fc + (z-jz_s)*fx*fy;
                        V3DLONG offsets_gz = offsets_gc + (z-iz_s)*gx*gy;
                        
                        V3DLONG offsets_olz = offsets_olc + (z-start_z)*dimxol*dimyol;
                        
                        for(V3DLONG y=start_y; y<end_y; y++)
                        {
                            V3DLONG offsets_fy = offsets_fz + (y-jy_s)*fx;
                            V3DLONG offsets_gy = offsets_gz + (y-iy_s)*gx;
                            
                            V3DLONG offsets_oly = offsets_olz + (y-start_y)*dimxol;
                            
                            for(V3DLONG x=start_x; x<end_x; x++)
                            {
                                V3DLONG idx = offsets_oly + x - start_x;
                                
                                fol1d[idx] = f1d[offsets_fy + (x-jx_s)];
                                gol1d[idx] = g1d[offsets_gy + (x-ix_s)];
                            }
                        }
                    }
                }
                // de-alloc
                if (f1d) {delete []f1d; f1d=NULL;}
                if (g1d) {delete []g1d; g1d=NULL;}
                
                //                
                for(V3DLONG c=0; c<fc; c++)
                {
                    V3DLONG offsets_olc = c*pagesz_ol;
                    
                    float a,b;
                    V3DLONG tilen;
                    
                    if(findHighestIntensityLevelTile<float>(fol1d+offsets_olc, gol1d+offsets_olc, pagesz_ol, tilen)!=true)
                    {
                        printf("Fail to call function computeAdjustPara! \n");
                        return false;
                    }
                    
                    qDebug()<<"color ..."<<c;
                }
                
                // de-alloc
                if (fol1d) {delete []fol1d; fol1d=NULL;}
                if (gol1d) {delete []gol1d; gol1d=NULL;}
                
                //
                aList.push_back(j);
            }
            else
            {
                bList.push_back(j);
            }
        }
        
        normList.append(reftile);
        
        for(V3DLONG ii=1; ii<vim.number_tiles; ii++)
        {
            tileList.append(ii);
            
            AdjustPara adjPara;
            
            adjPara.init(vc);
            adjPara.num_tile=ii;
            
            adjparaList.push_back(adjPara);
        }
        
        // consider linear model f=a*g+b find a and b
        while (!tileList.isEmpty()) 
        {
            // pick one from tileList
            V3DLONG j=tileList.at(0);
            tileList.pop_front();
            
            V3DLONG i;
            
            V3DLONG jx_s, jy_s, jz_s, jx_e, jy_e, jz_e;
            V3DLONG ix_s, iy_s, iz_s, ix_e, iy_e, iz_e;
            
            jx_s = vim.lut[j].start_pos[0] + vim.min_vim[0];
            jy_s = vim.lut[j].start_pos[1] + vim.min_vim[1];
            jz_s = vim.lut[j].start_pos[2] + vim.min_vim[2];
            
            jx_e = vim.lut[j].end_pos[0] + vim.min_vim[0];
            jy_e = vim.lut[j].end_pos[1] + vim.min_vim[1];
            jz_e = vim.lut[j].end_pos[2] + vim.min_vim[2];
            
            bool findoverlap = false;
            for (V3DLONG ii=0; ii<normList.size(); ii++) 
            {
                // pick one from normList
                i=normList.at(ii);
                
                ix_s = vim.lut[i].start_pos[0] + vim.min_vim[0];
                iy_s = vim.lut[i].start_pos[1] + vim.min_vim[1];
                iz_s = vim.lut[i].start_pos[2] + vim.min_vim[2];
                
                ix_e = vim.lut[i].end_pos[0] + vim.min_vim[0];
                iy_e = vim.lut[i].end_pos[1] + vim.min_vim[1];
                iz_e = vim.lut[i].end_pos[2] + vim.min_vim[2];
                
                // find whether they have common overlap region
                if(  ( (ix_s>=jx_s && ix_s<=jx_e) || (ix_e>=jx_s && ix_e<=jx_e) || (jx_s>=ix_s && jx_s<=ix_e) || (jx_e>=ix_s && jx_e<=ix_e) )
                   && ( (iy_s>=jy_s && iy_s<=jy_e) || (iy_e>=jy_s && iy_e<=jy_e) || (jy_s>=iy_s && jy_s<=iy_e) || (jy_e>=iy_s && jy_e<=iy_e) )
                   && ( (iz_s>=jz_s && iz_s<=jz_e) || (iz_e>=jz_s && iz_e<=jz_e) || (jz_s>=iz_s && jz_s<=iz_e) || (jz_e>=iz_s && jz_e<=iz_e) ) )
                {
                    findoverlap = true;
                    break;
                }
            }
            
            // find a and b
            if(findoverlap)
            {
                // load image f
                V3DLONG sz_f[4];
                int datatype_f = 0;
                unsigned char* f1d = 0;
                
                string fn;
                
                // j <-> f
                if(j==0) 
                {   
                    fn = vim.lut[j].fn_img;
                }
                else 
                {
                    QString curPath = QFileInfo(QString(vim.lut[0].fn_img.c_str())).path();;
                    
                    fn = curPath.append("/").append( QString(vim.lut[j].fn_img.c_str()) ).toStdString();
                }
                
                //
                if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), f1d, sz_f, datatype_f)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",fn.c_str());
                    return false;
                }
                V3DLONG fx=sz_f[0], fy=sz_f[1], fz=sz_f[2], fc=sz_f[3];
                
                // load image g
                V3DLONG sz_g[4];
                int datatype_g = 0;
                unsigned char* g1d = 0;
                
                // i <-> g
                if(i==0) 
                {   
                    fn = vim.lut[i].fn_img;
                }
                else 
                {
                    QString curPath = QFileInfo(QString(vim.lut[0].fn_img.c_str())).path();;
                    
                    fn = curPath.append("/").append( QString(vim.lut[i].fn_img.c_str()) ).toStdString();
                }
                
                //
                if (simple_loadimage_wrapper(callback,const_cast<char *>(fn.c_str()), g1d, sz_g, datatype_g)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",fn.c_str());
                    return false;
                }
                V3DLONG gx=sz_g[0], gy=sz_g[1], gz=sz_g[2], gc=sz_g[3];
                
                // overlap image from f and g
                V3DLONG start_x = qMax(jx_s, ix_s);
                V3DLONG start_y = qMax(jy_s, iy_s);
                V3DLONG start_z = qMax(jz_s, iz_s);
                
                V3DLONG end_x = qMin(jx_e, ix_e);
                V3DLONG end_y = qMin(jy_e, iy_e);
                V3DLONG end_z = qMin(jz_e, iz_e);
                
                V3DLONG dimxol= end_x-start_x+1;
                V3DLONG dimyol= end_y-start_y+1;
                V3DLONG dimzol= end_z-start_z+1;
                
                V3DLONG pagesz_ol = dimxol*dimyol*dimzol;

                float *fol1d = NULL;
                float *gol1d = NULL;
                
                try 
                {
                    // suppose fc = gc = vc
                    fol1d = new float [fc*pagesz_ol];
                    gol1d = new float [gc*pagesz_ol];
                } 
                catch (...) 
                {
                    printf("Fail to allocate memory for fol1d and gol1d.\n");

                    if (fol1d) {delete []fol1d; fol1d=NULL;}
                    if (gol1d) {delete []gol1d; gol1d=NULL;}
                    
                    return false;
                }
                
                for(V3DLONG c=0; c<fc; c++)
                {
                    V3DLONG offsets_fc = c*fx*fy*fz;
                    V3DLONG offsets_gc = c*gx*gy*gz;
                    
                    V3DLONG offsets_olc = c*pagesz_ol;
                    
                    for(V3DLONG z=start_z; z<end_z; z++)
                    {
                        V3DLONG offsets_fz = offsets_fc + (z-jz_s)*fx*fy;
                        V3DLONG offsets_gz = offsets_gc + (z-iz_s)*gx*gy;
                        
                        V3DLONG offsets_olz = offsets_olc + (z-start_z)*dimxol*dimyol;
                        
                        for(V3DLONG y=start_y; y<end_y; y++)
                        {
                            V3DLONG offsets_fy = offsets_fz + (y-jy_s)*fx;
                            V3DLONG offsets_gy = offsets_gz + (y-iy_s)*gx;
                            
                            V3DLONG offsets_oly = offsets_olz + (y-start_y)*dimxol;
                            
                            for(V3DLONG x=start_x; x<end_x; x++)
                            {
                                V3DLONG idx = offsets_oly + x - start_x;
                                
                                fol1d[idx] = f1d[offsets_fy + (x-jx_s)];
                                gol1d[idx] = g1d[offsets_gy + (x-ix_s)];
                            }
                        }
                    }
                }
                // de-alloc
                if (f1d) {delete []f1d; f1d=NULL;}
                if (g1d) {delete []g1d; g1d=NULL;}
                
                //
                V3DLONG ii;
                for (ii=0; ii<adjparaList.size(); ii++) 
                {
                    if(j==adjparaList.at(ii).num_tile) break;
                }
                
                for(V3DLONG c=0; c<fc; c++)
                {
                    V3DLONG offsets_olc = c*pagesz_ol;
                    
                    float a,b;
                    
                    if(computeAdjustPara<float>(fol1d+offsets_olc, gol1d+offsets_olc, pagesz_ol, a, b)!=true)
                    {
                        printf("Fail to call function computeAdjustPara! \n");
                        return false;
                    }
                    
                    adjparaList.at(ii).a[c] = a;
                    adjparaList.at(ii).b[c] = b;
                }
                
                // de-alloc
                if (fol1d) {delete []fol1d; fol1d=NULL;}
                if (gol1d) {delete []gol1d; gol1d=NULL;}
                
                //
                normList.push_back(j);
            }
            else
            {
                tileList.push_back(j);
            } 
        }
        // de-alloc
//        while (!adjparaList.isEmpty()) 
//        {
//            adjparaList.at(0).clean();
//            adjparaList.pop_front();
//        }
        
        if(datatype_tile == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;
            
            try
            {
                pVImg = new unsigned char [pagesz_vim];
                
                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory for pVImg.\n");
                
                if(pVImg) {delete []pVImg; pVImg=NULL;}
                
                return false;
            }
            
            //
            if(ireconstructingwnorm<unsigned char>((unsigned char *)pVImg, vim, 255, adjparaList,callback)!=true)
            {
                printf("Fail to call function ireconstructing! \n");
                return false;
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (simple_saveimage_wrapper(callback,blendImageName.toStdString().c_str(), (unsigned char *)pVImg, vim.sz, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;
            
            try
            {
                pVImg = new unsigned short [pagesz_vim];
                
                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory for pVImg.\n");
                
                if(pVImg) {delete []pVImg; pVImg=NULL;}
                
                return false;
            }
            
            //
            if(ireconstructingwnorm<unsigned short>((unsigned short *)pVImg, vim, 4095, adjparaList,callback)!=true)
            {
                printf("Fail to call function ireconstructing! \n");
                return false;
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (simple_saveimage_wrapper(callback,blendImageName.toStdString().c_str(), (unsigned char *)pVImg, vim.sz, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
            
        }
        else if(datatype_tile == V3D_FLOAT32)
        {
            // current not supported
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
        
    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
    
    return true;
}

// menu
QStringList ImageFusionPlugin::menulist() const
{
    return QStringList() << tr("Image Fusion")
                         << tr("About");
}

void ImageFusionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Image Fusion"))
    {
        ImageFusionDialog dialog(callback, parent, NULL);
        if (dialog.exec()!=QDialog::Accepted)
            return;
        
        QString m_InputFolder = dialog.foldername;
        
        if ( !QFile::exists(m_InputFolder) )
        {
            cout<<"Folder does not exist!"<<endl;
            return;
        }
        
        // call fusion dofunc 
        V3DPluginArgItem arg;
        V3DPluginArgList pluginfunc_input;
        V3DPluginArgList pluginfunc_output;
        
        vector<char*> fileList;
        vector<char*> paraList;
        
        fileList.clear();
        paraList.clear();
        
        QByteArray bytes = m_InputFolder.toAscii();
        
        fileList.push_back(bytes.data());
        
        paraList.push_back("#s 0");
        
        arg.type = ""; arg.p = (void *)(&fileList); pluginfunc_input << arg;
        arg.type = ""; arg.p = (void *)(&paraList); pluginfunc_input << arg;
        
        bool success = dofunc("iblender", pluginfunc_input, pluginfunc_output, callback, parent);
        
        if(!success)
        {
            QMessageBox::information(parent, "Warning: Image Fusion", QString("Fail to run image fusion program."));
            return;
        }
        
        // output
        V3DLONG *metaImg = (V3DLONG *)(pluginfunc_output.at(1).p);

        V3DLONG sx = metaImg[0];
        V3DLONG sy = metaImg[1];
        V3DLONG sz = metaImg[2];

        V3DLONG colordim = metaImg[3];
        V3DLONG datatype = metaImg[4];

        // show result in v3d
        if(datatype == V3D_UINT8)
        {
            //
            unsigned char* data1d = (unsigned char *)(pluginfunc_output.at(0).p);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_UINT8); //

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched Image");
            callback.updateImageWindow(newwin);

        }
        else if(datatype == V3D_UINT16)
        {
            //
            unsigned short* data1d = (unsigned short *)(pluginfunc_output.at(0).p);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_UINT16); //

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched Image");
            callback.updateImageWindow(newwin);
        }
        else if(datatype == V3D_FLOAT32)
        {
            // current not support
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return;
        }
        
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Image fusion. Version %1 (August 31, 2011) developed by Yang Yu and Sean Murphy. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()));
        return;
    }
}

