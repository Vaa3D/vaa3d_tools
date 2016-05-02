/* refextract.CPP
 * created by Yang Yu, Dec 16, 2011
 */
// update dofunc() interface by Jianlong Zhou, 2012-04-19

//

#ifndef __REFEXTRACT_SRC_CPP__
#define __REFEXTRACT_SRC_CPP__

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "refextract.h"

#include "basic_surf_objs.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"


#include "basic_landmark.h"
#include "basic_4dimage.h"

#include <iostream>
using namespace std;

#include <string.h>

#define INF 1e10


// free 1d pointer
template<class Tdata>
void freeMemory(Tdata *&p)
{
    if(p) {delete[] p; p=NULL;}
}

// Open a series of inputs
QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM";

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

// extract reference channel and convert it to 8bit data
template<class Tdata, class Tidx>
bool extconv(Tdata *p, Tidx sx, Tidx sy, Tidx sz, unsigned char *&pOutput)
{
    //
    if(p==NULL)
    {
        printf("\nError: inputs are invalid!\n");
        return false;
    }

    //
    Tidx pagesz = sx*sy*sz;

    //
    freeMemory<unsigned char>(pOutput);
    try
    {
        pOutput = new unsigned char [pagesz];
        memset(pOutput, 0, sizeof(unsigned char)*pagesz); // init
    }
    catch(...)
    {
        printf("Fail to allocate memory!\n");
        freeMemory<unsigned char>(pOutput);
        return false;
    }

    //
    double max_v=-INF;
    double min_v=INF;

    for(Tidx i=0; i<pagesz; i++)
    {
        Tdata val = p[i];

        if(max_v<val) max_v=val;
        if(min_v>val) min_v=val;
    }

    //
    max_v -= min_v;
    if(max_v==0)
    {
        printf("\nError: uniform data are not supported!\n");
        return false;
    }

    for(Tidx i=0; i<pagesz; i++)
    {
        pOutput[i] = 255*((double)p[i]-min_v)/max_v;
    }

    //
    return true;
}

//plugin interface
const QString title = "RefExtract";

Q_EXPORT_PLUGIN2(refExtract, RefExtractPlugin);

QStringList RefExtractPlugin::menulist() const
{
    return QStringList() << tr("RefExtract")
                         << tr("About");
}

void RefExtractPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("RefExtract"))
    {
    	//
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Reference Extraction Plugin %1 (Dec. 16, 2011) developed by Yang Yu. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"));
        return;
    }
}

void errorPrint()
{
    printf("\nUsage: v3d -x refExtract -f refExtract -i <input_image> -o <output_image> -p \"#c <refchannel> \"\n");
}

// plugin func
QStringList RefExtractPlugin::funclist() const
{
    return QStringList() << tr("refExtract")
                         << tr("ref2mip")
                         << tr("createTemplate")
                         << tr("help");
}

bool RefExtractPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
     if (func_name == tr("help"))
     {
          errorPrint();
          return true;
     }
     else if (func_name == tr("refExtract"))
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
        V3DLONG channel_ref = 0;

        QString qs_filename_img_input(infile);
        QString qs_filename_img_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

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
                                channel_ref = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
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


            QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();

            if(outfile)
            {
                qs_filename_img_output=qs_filename_output;
            }
            else
            {
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+"_8bit.v3draw";
            }

            // error check
            if(qs_filename_img_input==NULL || qs_filename_img_output==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint();
                return false;
            }
            if(channel_ref<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint();
                return false;
            }

        }

        //
        V3DLONG sz_relative[4];
        unsigned char* relative1d = 0;
        int datatype_tile = 0;
        if(QFile(QString(infile)).exists())
        {
            if (simple_loadimage_wrapper(callback,const_cast<char *>(infile), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",infile);
                return false;
            }
        }
        else
        {
            cout<<"The input file does not exist!"<<endl;
            return false;
        }

        //
        V3DLONG offset_c = channel_ref*sz_relative[0]*sz_relative[1]*sz_relative[2];

        unsigned char *pOutput = NULL;
        if(datatype_tile == V3D_UINT8)
        {
            if(extconv<unsigned char, V3DLONG>((unsigned char *)relative1d + offset_c, sz_relative[0], sz_relative[1], sz_relative[2], pOutput)!=true)
            {
                printf("Fail to call function extconv! \n");
                return false;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            if(extconv<unsigned short, V3DLONG>((unsigned short *)relative1d + offset_c, sz_relative[0], sz_relative[1], sz_relative[2], pOutput)!=true)
            {
                printf("Fail to call function extconv! \n");
                return false;
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

        // save
        if(qs_filename_img_output!=NULL)
        {
            sz_relative[3]=1;
            if(!simple_saveimage_wrapper(callback,qPrintable(qs_filename_img_output),pOutput,sz_relative,V3D_UINT8))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }

        //
        return true;
    }
    else if (func_name == tr("ref2mip"))
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
        V3DLONG channel_ref = 0;

        QString qs_filename_img_input(infile);
        QString qs_filename_img_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

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
                                channel_ref = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
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


            QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();

            if(outfile)
            {
                qs_filename_img_output=qs_filename_output;
            }
            else
            {
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+"_mip.tif";
            }

            // error check
            if(qs_filename_img_input==NULL || qs_filename_img_output==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint();
                return false;
            }
            if(channel_ref<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint();
                return false;
            }
        }

        //
        V3DLONG sz_relative[4];
        unsigned char* relative1d = 0;
        int datatype_tile = 0;
        if(QFile(QString(infile)).exists())
        {
            if (simple_loadimage_wrapper(callback,const_cast<char *>(infile), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",infile);
                return false;
            }
        }
        else
        {
            cout<<"The input file does not exist!"<<endl;
            return false;
        }

        //
        V3DLONG offset_c = channel_ref*sz_relative[0]*sz_relative[1]*sz_relative[2];

        unsigned char *pOutput = NULL;
        if(datatype_tile == V3D_UINT8)
        {
            if(extconv<unsigned char, V3DLONG>((unsigned char *)relative1d + offset_c, sz_relative[0], sz_relative[1], sz_relative[2], pOutput)!=true)
            {
                printf("Fail to call function extconv! \n");
                return false;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            if(extconv<unsigned short, V3DLONG>((unsigned short *)relative1d + offset_c, sz_relative[0], sz_relative[1], sz_relative[2], pOutput)!=true)
            {
                printf("Fail to call function extconv! \n");
                return false;
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

        // generate z-MIP images (.tif)

        for(V3DLONG j=0; j<sz_relative[1]; j++)
        {
            V3DLONG offset_j = j*sz_relative[0];
            for(V3DLONG i=0; i<sz_relative[0]; i++)
            {
                int val = 0;
                for(V3DLONG k=0; k<sz_relative[2]; k++)
                {
                    int curval = pOutput[ k*sz_relative[0]*sz_relative[1] + offset_j + i ];

                    if(val < curval) val = curval;
                }

                pOutput[offset_j + i] = val;
            }
        }


        // save
        if(qs_filename_img_output!=NULL)
        {
            sz_relative[2]=1;
            sz_relative[3]=1;
            if(!simple_saveimage_wrapper(callback,qPrintable(qs_filename_img_output),pOutput,sz_relative,V3D_UINT8))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }

        //
        if(pOutput) {delete []pOutput; pOutput=NULL;}

        //
        return true;
    }
    else if (func_name == tr("createTemplate"))
    {
        if(input.size()<1) // no inputs
        {
            //print Help info
            errorPrint();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_folder
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        //qDebug()<<"test ..."<<input.size()<<output.size();

        if(!input.empty())
        {
            if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
            if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
            if(!infilelist->empty()) { infile = infilelist->at(0); }
        }

        if(!output.empty())
        {
            if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
            if(!infilelist->empty()) { infile = infilelist->at(0); }
            if(!outfilelist->empty()) { outfile = outfilelist->at(0); }
        }

        // init
        V3DLONG channel_ref = 0;

        //qDebug()<<"running template creation ....";

        QStringList imgList = importSeriesFileList_addnumbersort(infile);
        QString qs_filename_img_input = imgList.at(0);
        QString qs_filename_img_output;

        qDebug()<<"one input file ... "<<qPrintable(qs_filename_img_input);

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

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
                                channel_ref = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
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

            if(channel_ref<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint();
                return false;
            }
        }

        if(outfile)
        {
            QString qs_filename_output=QString(outfile);
            //QString qs_pathname_output=QFileInfo(qs_filename_output).path();

            qs_filename_img_output=qs_filename_output;
        }
        else
        {
            QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
            QString qs_pathname_output=QFileInfo(qs_filename_img_input).path();

            qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+"_template.v3draw";
        }

        qDebug()<<"The output file ..."<<qPrintable(qs_filename_img_output);

        // error check
        if(qs_filename_img_input==NULL || qs_filename_img_output==NULL)
        {
            printf("\nERROR: invalid input file name (target or subject)!\n");
            errorPrint();
            return false;
        }

        //
        float *pOutput = NULL;
        V3DLONG szOutput[4];
        V3DLONG cnt = 1;
        V3DLONG totalplxs = 0;
        foreach (QString img_str, imgList)
        {
            string fn_image = img_str.toStdString();

            //loading files
            V3DLONG *szImg = 0;
            int datatypeImg = 0;
            unsigned char* pImg1d = 0;

            if(QFile(QString(fn_image.c_str())).exists())
            {
                if (simple_loadimage_wrapper(callback,const_cast<char *>(fn_image.c_str()), pImg1d, szImg, datatypeImg)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", fn_image.c_str());
                    return false;
                }
            }
            else
            {
                cout<<"The input file does not exist!"<<endl;
                return false;
            }

            if(cnt==1)
            {
                // assume all images are with the same size
                try
                {
                    //szOutput = new V3DLONG [4];
                    szOutput[0] = szImg[0];
                    szOutput[1] = szImg[1];
                    szOutput[2] = szImg[2];
                    szOutput[3] = szImg[3]; // 1

                    totalplxs = szOutput[0]*szOutput[1]*szOutput[2]*szOutput[3];

                    pOutput = new float [totalplxs];
                    for(V3DLONG i=0; i<totalplxs; i++)
                        pOutput[i] = pImg1d[i];

                }
                catch(...)
                {
                    cout<<"Fail to allocate memory!"<<endl;
                    return false;
                }
            }
            else
            {
                for(V3DLONG i=0; i<totalplxs; i++)
                    pOutput[i] += pImg1d[i];
            }

            //
            if(pImg1d) {delete []pImg1d; pImg1d=NULL;}
            if(szImg) {delete []szImg; szImg=NULL;}

            cnt++;
        }

        for(V3DLONG i=0; i<totalplxs; i++)
            pOutput[i] /= (float)cnt;

        // save
        if(qs_filename_img_output!=NULL)
        {
            if(!simple_saveimage_wrapper(callback,qPrintable(qs_filename_img_output),(unsigned char *)pOutput,szOutput,4))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }

        //
        if(pOutput) {delete []pOutput; pOutput=NULL;}

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


