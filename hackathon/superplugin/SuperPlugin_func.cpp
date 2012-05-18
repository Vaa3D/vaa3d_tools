/* SuperPlugin_func.cpp
 * This is SuperPlugin which uses multiple other plugins in a pipeline.
 * 2012-05-11 : by J Zhou
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "SuperPlugin_func.h"
#include <vector>
#include <iostream>
#include <cstdlib>

using namespace std;

const QString title = QObject::tr("This is SuperPlugin. ");


int parse_multi_plugin(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("Now you are invoking the example code. You can replace this part using your own code. Only the commandline version is available.");
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	//int i = 0;
	//int c = 0
	//Image4DSimple *p4DImage = callback.getImage(win_list[i]);
	//if(p4DImage->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	//V3DLONG sz[3];
	//sz[0] = p4DImage->getXDim();
	//sz[1] = p4DImage->getYDim();
	//sz[2] = p4DImage->getZDim();

	//unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

	//v3dhandle newwin;
	//if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		//newwin = callback.currentImageWindow();
	//else
		//newwin = callback.newImageWindow();

	//p4DImage->setData(inimg1d, sz[0], sz[1], sz[2], sz[3]);
	//callback.setImage(newwin, p4DImage);
	//callback.setImageName(newwin, QObject::tr("func1"));
	//callback.updateImageWindow(newwin);
	return 1;
}

bool parse_multi_plugin(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to SuperPlugin"<<endl;

	if(input.size() < 1 || output.size() != 1) return false;

     //input image, marker, atlas
	QString qs_filename_img_input="";
     QString qs_filename_img_output="";

     vector< QString > tar_plugin_paras; // paras for each called plugins

     // input paras style:
     // # is used to separate different plugin name and its -p paras
     // -i inputimg.raw -o outimg.raw -p + plugin1_x plugin1_f p_paras_plugin1 + plugin2_x plugin2_f p_paras_plugin2

     if (input.size()>=1)
     {
          // input file
          vector<char*> paras_infile = (*(vector<char*> *)(input.at(0).p));
          if(paras_infile.size() >= 1) qs_filename_img_input = paras_infile.at(0);

          // output file
          vector<char*> paras_outfile = (*(vector<char*> *)(output.at(0).p));
          if(paras_outfile.size() >= 1) qs_filename_img_output = paras_outfile.at(0);

          // parameters from -p for called plugin
          if(input.size()>=2)
          {
               vector<char*> paras_p = (*(vector<char*> *)(input.at(1).p));
               int i=0;
               while( i<paras_p.size() && !strcmp( paras_p.at(i++), "+") )  // is "+"
               {
                    QString cur_para;
                    // plugin -x para
                    cur_para.append(" -x ").append(paras_p.at(i++));
                    // plugin -f para
                    cur_para.append(" -f ").append(paras_p.at(i++));
                    // plugin -p paras
                    cur_para.append(" -p ");

                    while( i<paras_p.size() && strcmp(paras_p.at(i), "+") ) // not "+"
                    {
                         char* pa= paras_p.at(i++);
                         cur_para.append(" ").append(pa);
                    }

                    // push back paras for a plugin
                    tar_plugin_paras.push_back(cur_para);
                    cout<<"tar_plugin paras: "<< qPrintable(cur_para) <<endl;
               }
          }
	}
     else
     {
          v3d_msg("There are not enough parameters.\n", 0);
          return false;
     }

     // print out paras
     cout<<"Input image file: "<<qPrintable(qs_filename_img_input)<<endl;
     cout<<"Output image file: "<<qPrintable(qs_filename_img_output)<<endl;

     // link parameters
     for(V3DLONG i=0; i<tar_plugin_paras.size(); i++)
     {
          // para -o
          if( i == tar_plugin_paras.size() -1 )
               ( tar_plugin_paras.at(i).prepend(qs_filename_img_output) ).prepend(" -o ");
          else
               ( tar_plugin_paras.at(i).prepend( QObject::tr("temp%1.raw").arg(i) ) ).prepend(" -o ");

          // para -i
          if( i == 0 )
               ( tar_plugin_paras.at(i).prepend(qs_filename_img_input) ).prepend(" -i ");
          else
               ( tar_plugin_paras.at(i).prepend( QObject::tr("temp%1.raw").arg(i-1) ) ).prepend(" -i ");

          tar_plugin_paras.at(i).prepend("v3d ");
     }

     // run command
     for(V3DLONG i=0; i<tar_plugin_paras.size(); i++)
     {
          // Set up the command line which starts the external program
          cout<<"Now run plugin: "<< qPrintable( tar_plugin_paras.at(i) ) <<endl;
          system(qPrintable( tar_plugin_paras.at(i) ));
     }

     // delete temp files
#if defined(Q_OS_WIN)
     system("del temp*.raw");
#else
     system("rm temp*.raw");
#endif

     cout<<"Finish successfully!"<<endl;
	return true;
}


