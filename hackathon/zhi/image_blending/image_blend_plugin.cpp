/* image_blend_plugin.cpp
 * 
 * 2014-01-28 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "image_blend_plugin.h"
#include <fstream>
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(image_blend, image_blend);
 
QStringList image_blend::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList image_blend::funclist() const
{
	return QStringList()
		<<tr("image_blend")
		<<tr("help");
}

void image_blend::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
	{
        v3d_msg(tr("This plugin blends two images. This plugin is designed for command line use ONLY right now. "
			"Developed by Zhi Zhou, 2014-01-28"));
	}
}

bool image_blend::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("image_blend"))
	{
      /*  if (output.size() != 1) return false;
        if (input.size() != 2) return false;

        char * inimg_file1 = ((vector<char*> *)(input.at(0).p))->at(0);
        char * inimg_file2 = ((vector<char*> *)(input.at(1).p))->at(0);

        char * output_file = ((vector<char*> *)(output.at(0).p))->at(0);*/

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
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

        char * output_file = ((vector<char*> *)(output.at(0).p))->at(0);

        QString inimg_file1,inimg_file2;
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
                                inimg_file1 = QString( argv[i+1] );
                                qDebug()<<"filename_img_1=["<<inimg_file1<<"]";
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                //channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                inimg_file2 = QString( argv[i+1] );
                                qDebug()<<"filename_img_2=["<<inimg_file2<<"]";
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                              //  channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
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
        }

        unsigned char * image1 = 0;
        V3DLONG in_sz1[4];
        int datatype1;
        if(!simple_loadimage_wrapper(callback, inimg_file1.toStdString().c_str(), image1, in_sz1, datatype1))
        {
            cerr<<"load image1 "<<inimg_file1.toStdString()<<" error!"<<endl;
            if (image1) {delete []image1; image1=0;}
            return false;
        }
        V3DLONG pagesz1 = in_sz1[0]*in_sz1[1]*in_sz1[2];

        unsigned char * image2 = 0;
        V3DLONG in_sz2[4];
        int datatype2;
        if(!simple_loadimage_wrapper(callback, inimg_file2.toStdString().c_str(), image2, in_sz2, datatype2))
        {
            cerr<<"load image2 "<<inimg_file2.toStdString()<<" error!"<<endl;
             if (image2) {delete []image2; image2=0;}
            return false;
        }

        V3DLONG pagesz2 = in_sz2[0]*in_sz2[1]*in_sz2[2];

        if(pagesz1 != pagesz2)
        {
             v3d_msg("Two images have differnt size.");
             if (image1) {delete []image1; image1=0;}
             if (image2) {delete []image2; image2=0;}
             return false;
        }

        unsigned char *data_blended = 0;
        try {data_blended = new unsigned char [pagesz1*3];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}
        for(V3DLONG i = 0; i < pagesz1*3; i++)
        {
            if(i < pagesz1)
                data_blended[i] = image1[i];
            else if(i < 2*pagesz1)
                data_blended[i] = image2[i-pagesz1];
            else
                data_blended[i] = 0;
        }
        in_sz1[3] = 3;
        simple_saveimage_wrapper(callback, output_file, (unsigned char *)data_blended, in_sz1, 1);

        if (image1) {delete []image1; image1=0;}
        if (image2) {delete []image2; image2=0;}
        if (data_blended) {delete []data_blended; data_blended=0;}

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

