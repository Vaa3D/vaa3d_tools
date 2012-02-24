/* itiling.CPP
 * 2011-12-02: create this program by Yang Yu
 */


// 

#ifndef __ITILING_SRC_CPP__
#define __ITILING_SRC_CPP__

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "itiling.h"

//#include "y_imglib.h"
#include "y_imgvis.h"

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

#include <iostream>
using namespace std;

#include <string.h>


// template func
template<class Tdata, class Tidx>
bool imgtiling(Tdata *p, Tidx sx, Tidx sy, Tidx sz, Tidx sc, QString output, int datatype)
{
    // block
    Tidx bx=BLOCK_DIM_X;
    Tidx by=BLOCK_DIM_Y;
    Tidx bz=BLOCK_DIM_Z;
    Tidx bc=sc;
    
    Tdata *pBlock=NULL;
    
    Tidx szblock = bx*by*bz*bc;
    
    try {
        pBlock = new Tdata [szblock];
    } catch (...) {
        cout<<"Fail to allocate memory for block!"<<endl;
        return false;
    }
    
    Tidx offsetc = sx*sy*sz;
    Tidx offsetz = sx*sy;
    Tidx offsety = sx;
    
    Tidx offsetc_block = bx*by*bz;
    Tidx offsetz_block = bx*by;
    Tidx offsety_block = bx;
    
    //
    Tidx xn = sx/bx + 1;
    Tidx yn = sy/by + 1;
    Tidx zn = sz/bz + 1;
    
    for (Tidx k=0; k<zn; k++) 
    {
        for (Tidx j=0; j<yn; j++) 
        {
            for (Tidx i=0; i<xn; i++) 
            {
                // init
                memset(pBlock, 0, sizeof(Tdata)*szblock);
                
                // file name
		QString fn_image = output;
                fn_image = fn_image.append("X%1Y%2Z%3").arg(i).arg(j).arg(k);
		fn_image.append(IMGSUFFIX);
		qDebug()<<"output ..."<<fn_image<<i<<j<<k;
                
                //
                Tidx start_x = i*bx;
                Tidx end_x = start_x + bx;
                
                if(end_x>sx) end_x = sx;
                
                Tidx start_y = j*by;
                Tidx end_y = start_y + by;
                
                if(end_y>sy) end_y = sy;
                
                Tidx start_z = k*bz;
                Tidx end_z = start_z + bz;
                
                if(end_z>sz) end_z = sz;

		//
		if(end_x<start_x || end_y<start_y || end_z<start_z) continue;
                
                
                //
                for(Tidx kk=start_z; kk<end_z; kk++)
                {
                    Tidx offset_z_ori = kk*offsetz;
                    Tidx offset_z_blk = (kk-start_z)*offsetz_block;
                    
                    for(Tidx jj=start_y; jj<end_y; jj++)
                    {
                        Tidx offset_y_ori = offset_z_ori + jj*offsety;
                        Tidx offset_y_blk = offset_z_blk + (jj-start_y)*offsety_block;
                        
                        for(Tidx ii=start_x; ii<end_x; ii++)
                        {
                            Tidx offset_x_ori = offset_y_ori + ii;
                            Tidx offset_x_blk = offset_y_blk + ii-start_x;
                            
                            for(Tidx c=0; c<sc; c++)
                            {
                                Tidx offset_c_ori = offset_x_ori + c*offsetc;
                                Tidx offset_c_blk = offset_x_blk + c*offsetc_block;
                                
                                pBlock[offset_c_blk] = p[offset_c_ori];
                                
                            }
                            
                        }
                    }
                }
                
                // save block
                Tidx savesz[4];
                
                savesz[0] = bx;
                savesz[1] = by;
                savesz[2] = bz;
                savesz[3] = sc;
                
                //
		bool saved = false;

		for(Tidx idx=0; idx<szblock; idx++)
		{
			if(pBlock[idx]>=THRESH) {saved=true; break;}
		}

		if(saved != true) continue;

                if (saveImage(fn_image.toStdString().c_str(), (const unsigned char *)pBlock, savesz, datatype)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
            }
        } 
    }
    
    //
    if(pBlock){delete[] pBlock; pBlock=NULL;}
    
    //
    return true;
}

//plugin interface
const QString title = "Image Tiling";

Q_EXPORT_PLUGIN2(imageTiling, ITilingPlugin);

QStringList ITilingPlugin::menulist() const
{
    return QStringList() << tr("Image Tiling")
                         << tr("About");
}

void ITilingPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Image Tiling"))
    {
    	//
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Image Tiling Plugin %1 (Dec. 2, 2011) developed by Yang Yu and Hanchuan Peng. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"));
        return;
    }
}

// plugin func
QStringList ITilingPlugin::funclist() const
{
    return QStringList() << tr("itiling")  << tr("help");
}

bool ITilingPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("help"))
    {
        printf("\nUsage: v3d -x imageTiling.dylib -f itiling -i <input_image_folder> -o <output_image_file> \n");
        return true;
    }
    else if (func_name == tr("itiling"))
    {
        // subpixel translation registration based on pixel-level translation estimation
        if(input.size()<1) return false; // no inputs

        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty())
        {
            //print Help info
            printf("\nUsage: v3d -x imageTiling.dylib -f itiling -i <input_image_folder> -o <output_image_file> \n");
            return true;
        }

        char * infile = infilelist->at(0); // input_image_folder
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        QString m_InputFolder(infile);

        int channel1 = 0;
        bool img_show = true; // save stitching file

        bool success = false;

        ImagePixelType imgdatatype;
        V3DLONG cdim;

        // parsing parameters
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
                            if (!strcmp(key, "c"))
                            {
                                channel1 = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                img_show = atoi( argv[i+1] )?true:false;
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

            // error check
            if(channel1<0)
            {
                cout<<"illegal input parameters"<<endl;
                return false;
            }
            
            //free space
            if (argv) {delete []argv; argv=0;}
            if (myparas) {delete []myparas; myparas=0;}
        }

        // get stitch configuration
        QDir myDir(infile);

        // group stitch in subspace
        int start_t = clock();

        //
        int datatype_tile = 0; // assume all tiles with the same datatype
        
        V3DLONG *sz_relative = 0;
        unsigned char* relative1d = 0;
        if(QFile(QString(infile)).exists())
        {
            if (loadImage(const_cast<char *>(infile), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",infile);
                return false;
            }
        }
        else
        {
            cout<<"The input file is not supported!"<<endl;
            return false;
        }
        
        //
        if(datatype_tile == V3D_UINT8)
        {
            if(imgtiling<unsigned char, V3DLONG>((unsigned char *)relative1d, sz_relative[0], sz_relative[1], sz_relative[2], sz_relative[3], QString(outfile), datatype_tile)!=true);
            {
                printf("Fail to call function imgtiling! \n");
                return false;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            if(imgtiling<unsigned short, V3DLONG>((unsigned short *)relative1d, sz_relative[0], sz_relative[1], sz_relative[2], sz_relative[3], QString(outfile), datatype_tile)!=true);
            {
                printf("Fail to call function imgtiling! \n");
                return false;
            }

        }
        else if(datatype_tile == V3D_FLOAT32)
        {
            printf("The float type is not supported yet.\n");
            return false;
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        cout<<"time elapse ... "<<clock()-start_t<<endl;

        //
        return true;
    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
}

#endif


