/* image_blend_plugin.cpp
 * 
 * 2014-01-28 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "image_blend_plugin.h"
#include <fstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"
#include "../plugin_loader/v3d_plugin_loader.h"
#include "/local1/work/v3d_external/v3d_main/basic_c_fun/stackutil.h"


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
        <<tr("image_modulate")
        <<tr("image_align")
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
        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            return true;
        }

        char * output_file = ((vector<char*> *)(output.at(0).p))->at(0);

        vector<char*> * paralist;
        char * paras = NULL; // parameters
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters


        QString inimg_file1,inimg_file2;
        V3DLONG channel_input1 = 0;
        V3DLONG channel_input2 = 0;
        V3DLONG channel_output1 = 0;
        V3DLONG channel_output2 = 0;

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
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '^')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "i1"))
                            {
                                key++;
                                inimg_file1 = QString( argv[i+1] );
                                qDebug()<<"filename_img_1=["<<inimg_file1<<"]";
                                i++;
                            }
                            else if (!strcmp(key, "c1"))
                            {
                                key++;
                                channel_input1 = atoi( argv[i+1] ); // red 1 green 2 blue 3
                                qDebug()<<"input channel for filename_img_1 = "<<channel_input1<<"";
                                i++;
                            }
                            else if (!strcmp(key, "o1"))
                            {
                                key++;
                                channel_output1 = atoi( argv[i+1] ); // red 1 green 2 blue 3
                                qDebug()<<"output channel for filename_img_1 = "<<channel_output1<<"";
                                i++;
                            }
                            else if (!strcmp(key, "i2"))
                            {
                                key++;
                                inimg_file2 = QString( argv[i+1] );
                                qDebug()<<"filename_img_2=["<<inimg_file2<<"]";
                                i++;
                            }
                            else if (!strcmp(key, "c2"))
                            {
                                key++;
                                channel_input2 = atoi( argv[i+1] ); // red 1 green 2 blue 3
                                qDebug()<<"input channel for filename_img_2 = "<<channel_input2<<"";
                                i++;
                            }
                            else if (!strcmp(key, "o2"))
                            {
                                key++;
                                channel_output2 = atoi( argv[i+1] ); // red 1 green 2 blue 3
                                qDebug()<<"output channel for filename_img_2 ="<<channel_output2<<"";
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

        if(channel_output1 == channel_output2 || channel_output1 > 3 || channel_output2 > 3)
        {
            cerr<<"Invalid channel number for: "<<output_file<<" error!"<<endl;
            return false;
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

        if(channel_input1 > in_sz1[3] || channel_input1 < 1)
        {
            cerr<<"Invalid channel number for: "<<inimg_file1.toStdString()<<" error!"<<endl;
            if (image1) {delete []image1; image1=0;}
            return false;
        }
        unsigned char * image2 = 0;
        V3DLONG in_sz2[4];
        int datatype2;
        if(!simple_loadimage_wrapper(callback, inimg_file2.toStdString().c_str(), image2, in_sz2, datatype2))
        {
            cerr<<"load image2 "<<inimg_file2.toStdString()<<" error!"<<endl;
             if (image2) {delete []image2; image2=0;}
            return false;
        }
        if(channel_input2 > in_sz2[3] || channel_input2 < 1)
        {
            cerr<<"Invalid channel number for: "<<inimg_file2.toStdString()<<" error!"<<endl;
            if (image2) {delete []image2; image2=0;}
            return false;
        }

        if(in_sz1[0] != in_sz2[0] || in_sz1[1] != in_sz2[1] || in_sz1[2] != in_sz2[2])
        {
             v3d_msg("Two images have differnt size.");
             if (image1) {delete []image1; image1=0;}
             if (image2) {delete []image2; image2=0;}
             return false;
        }

        V3DLONG pagesz = in_sz1[0]*in_sz1[1]*in_sz1[2];
        V3DLONG in_sz[4];
        in_sz[0] = in_sz1[0];
        in_sz[1] = in_sz1[1];
        in_sz[2] = in_sz1[2];
        in_sz[3] = 3;
        unsigned char *data_blended = 0;
        try {data_blended = new unsigned char [pagesz*3];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}

        for(V3DLONG i = 0; i < pagesz*3; i++)
            data_blended[i] = 0;
        V3DLONG j = 0;
        for(V3DLONG i = (channel_output1-1)*pagesz; i < channel_output1*pagesz; i++)
        {
            data_blended[i] = image1[(channel_input1-1)*pagesz + j];
            j++;
        }
        j = 0;
        for(V3DLONG i = (channel_output2-1)*pagesz; i < channel_output2*pagesz; i++)
        {
            data_blended[i] = image2[(channel_input2-1)*pagesz + j];
            j++;
        }

        simple_saveimage_wrapper(callback, output_file, (unsigned char *)data_blended, in_sz, 1);

        if (image1) {delete []image1; image1=0;}
        if (image2) {delete []image2; image2=0;}
        if (data_blended) {delete []data_blended; data_blended=0;}

	}
    else if (func_name == tr("image_modulate"))
    {
        if (output.size() != 1) return false;
        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
       // char * inimg_enhanced = ((vector<char*> *)(input.at(1).p))->at(0);
        char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

       /* unsigned char * image1 = 0;
        V3DLONG in_sz1[4];
        int datatype1;
        if(!simple_loadimage_wrapper(callback, inimg_file, image1, in_sz1, datatype1))
        {
            if (image1) {delete []image1; image1=0;}
            return false;
        }*/

        unsigned char * image1 = 0;
        V3DLONG * in_sz1 = 0;


        int datatype1;
        if(!loadImage(inimg_file, image1, in_sz1, datatype1))
        {
            cerr<<"load image "<<inimg_file<<" error!"<<endl;
            return false;
        }



     /*   unsigned char * image2 = 0;
        V3DLONG in_sz2[4];
        int datatype2;
        if(!simple_loadimage_wrapper(callback, inimg_enhanced, image2, in_sz2, datatype2))
        {
            if (image2) {delete []image2; image2=0;}
            return false;
        }*/



        V3DLONG in_sz[4];
        in_sz[0] = in_sz1[0];
        in_sz[1] = in_sz1[1];
        in_sz[2] = in_sz1[2];
        in_sz[3] = 1;

        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz1[2];


       /* V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;

        arg.type = "random";std::vector<char*> args1;
        args1.push_back(inimg_enhanced); arg.p = (void *) & args1; input<< arg;
        arg.type = "random";std::vector<char*> args;
        args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("1"); arg.p = (void *) & args; input << arg;
        arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage.v3draw"); arg.p = (void *) & args2; output<< arg;

        QString full_plugin_name = "gaussian";
        QString func_name = "gf";

        callback.callPluginFunc(full_plugin_name,func_name, input,output);

        unsigned char * data1d_Gf = 0;
        int datatype;
        V3DLONG in_zz[4];

        simple_loadimage_wrapper(callback, "gfImage.v3draw", data1d_Gf, in_zz, datatype);
        remove("gfImage.v3draw");
        unsigned char* image2 = 0;
        image2 = new unsigned char [pagesz];

        double min,max;
        rescale_to_0_255_and_copy((float *)data1d_Gf,pagesz,min,max,image2);
        if(data1d_Gf) {delete []data1d_Gf; data1d_Gf =0;}*/

        unsigned char *data_blended = 0;
        try {data_blended = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}
        V3DLONG i = 0;

        V3DLONG N = in_sz1[0];
        V3DLONG M = in_sz1[1];
        V3DLONG P = in_sz1[2];
        printf("%d,%d,%d,%d,%d\n\n",N,M,P,in_sz1[3],datatype1);

        V3DLONG offsetc = 1*pagesz;
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                    data_blended[offsetk + offsetj + ix] = image1[offsetc+offsetk + offsetj + ix];
                    i++;
                }
            }
        }


       /* for(V3DLONG i = 0; i < pagesz; i++)
        {
           /* double f = 0;
            double f1 = 0;
            double f2 = 0;
            f1 = image1[i]/255.f;
            f2 = image2[i]/255.f;
            f = (f1 + f2) * (f1+f2);
            f = fabs(f);

            if (f>1) f = 1;

            data_blended[i] = f*255;
            data_blended[i] = 255 - image1[i];

        }*/




        simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data_blended, in_sz, datatype1);

        if (image1) {delete []image1; image1=0;}
     //   if (image2) {delete []image2; image2=0;}
        if (data_blended) {delete []data_blended; data_blended=0;}

    }
    else if (func_name == tr("image_align"))
    {

        if (output.size() != 1) return false;
        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
       // char * inimg_enhanced = ((vector<char*> *)(input.at(1).p))->at(0);
        char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

        unsigned char * image1 = 0;
        V3DLONG in_sz1[4];
        int datatype1;
        if(!simple_loadimage_wrapper(callback, inimg_file, image1, in_sz1, datatype1))
        {
            if (image1) {delete []image1; image1=0;}
            return false;
        }

      /*  unsigned char * image2 = 0;
        V3DLONG in_sz2[4];
        int datatype2;
        if(!simple_loadimage_wrapper(callback, inimg_enhanced, image2, in_sz2, datatype2))
        {
            if (image2) {delete []image2; image2=0;}
            return false;
        }*/



        V3DLONG in_sz[4];
        in_sz[0] = 18671;
        in_sz[1] = 12079;
        in_sz[2] = 1;
        in_sz[3] = 1;

        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];

        unsigned char *data_blended = 0;
        try {data_blended = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}
        for(int i = 0; i < pagesz; i++)
            data_blended[i] = 0;
        V3DLONG N = in_sz[0];
        V3DLONG M = in_sz[1];
        V3DLONG P = in_sz[2];
        double a11 = 0.989488343532377;
        double a12 = -0.14461264817971808;
        double a21 = 0.14461264817971808;
        double a22 = 0.989488343532377;
        double xshift = -2747.242615559955;
        double yshift = -1505.0768840073147;

        double det = a11*a22 - a12*a21;

        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {

                    V3DLONG ix_old = int((a22*ix - a21*iy)/det) - xshift;
                    V3DLONG iy_old = int((-a12*ix + a11*iy)/det) - yshift;
                    if(ix_old >=0 && ix_old < in_sz1[0] && iy_old >=0 && iy_old < in_sz1[1])
                    {
                        V3DLONG offsetj_old = iy_old*in_sz1[0];
                        data_blended[offsetk + offsetj + ix] = image1[offsetk + offsetj_old + ix_old];
                    }
                }

            }
        }



        simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data_blended, in_sz, 1);

        if (image1) {delete []image1; image1=0;}
      //  if (image2) {delete []image2; image2=0;}
        if (data_blended) {delete []data_blended; data_blended=0;}



    }
    else if (func_name == tr("help"))
	{
        cout<<"Usage : v3d -x blend_two_image -f image_blend -o <outimg_file> -p \"^i1 <inputimg1_file> ^c1 <inputimg1_channel> ^o1 <inputimg1_output_channel> ^i2 <inputimg2_file> ^c2 <inputimg2_channel> ^o2 <inputimg2_output_channel>\""<<endl;
        cout<<endl;
        cout<<"inputimg1_channel          the input channel value for inputimg1_file, start from 1"<<endl;
        cout<<"inputimg1_output_channel   the output channel value for inputimg1_file, start from 1,shoule no bigger than 3"<<endl;
        cout<<"inputimg2_channel          the input channel value for inputimg2_file, start from 1"<<endl;
        cout<<"inputimg2_output_channel   the output channel value for inputimg2_file, start from 1,shoule no bigger than 3"<<endl;
        cout<<endl;
        return true;
	}
	else return false;

	return true;
}


