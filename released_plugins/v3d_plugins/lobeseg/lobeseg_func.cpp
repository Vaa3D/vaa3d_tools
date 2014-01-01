/* lobeseg_func.cpp
 * This is a lobeseg plugin
 * 2011-06-20 : by Hang Xiao
 */

#include "stackutil.h"
#include <v3d_interface.h>
#include "v3d_message.h"
#include "lobeseg_func.h"
#include "lobeseg_gui.h"

#include "lobeseg_main/lobeseg.h"
#include "../../../v3d_main/worm_straighten_c/bdb_minus.h"

#ifdef WIN32
#include "getopt.h"
#endif

#include <iostream>
using namespace std;
extern char *optarg;
extern int optind, opterr;

const QString title = QObject::tr("Lobeseg Plugin");

int split(const char *paras, char ** &args)
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
            posb[argc++]=i;
        }
        else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') &&
                (myparas[i] != ' ' && myparas[i] != '\t'))
        {
            posb[argc++] = i;
        }
    }

    args = new char*[argc];
    for(int i = 0; i < argc; i++)
    {
        args[i] = myparas + posb[i];
    }

    for(int i = 0; i < len; i++)
    {
        if(myparas[i]==' ' || myparas[i]=='\t')myparas[i]='\0';
    }
    return argc;
}

int lobeseg_two_sides(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	TwoSidesDialog dialog(callback, parent);
	if(dialog.exec() == QDialog::Rejected) return -1;
	dialog.update();
	int i = dialog.i;
	int c = dialog.channel;
	double alpha = dialog.alpha;
	double beta = dialog.beta;
	double gamma = dialog.gamma;
	int nloops = dialog.nloops;
	int radius = dialog.radius;

	Image4DSimple * image = callback.getImage(win_list[i]);
	if(image->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	unsigned char * inimg1d = image->getRawData();
	V3DLONG sz[4];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();
	sz[3] = image->getCDim();
	unsigned char * outimg1d = new unsigned char[sz[0] * sz[1] * sz[2] * (sz[3] + 1)];
	for(V3DLONG i = 0 ; i < sz[0] * sz[1] * sz[2] * sz[3]; i++) outimg1d[i] = inimg1d[i];
	for(V3DLONG i =  sz[0] * sz[1] * sz[2] * sz[3] ; i < sz[0] * sz[1] * sz[2] * (sz[3] + 1); i++) outimg1d[i] = 0;

	int in_channel_no = c;
	int out_channel_no = 1;

	BDB_Minus_ConfigParameter mypara;
	mypara.f_image = alpha;
	mypara.f_smooth = beta;
	mypara.f_length = gamma;
	mypara.nloops = nloops;
	mypara.radius = radius;
	mypara.radius_x = radius;
	mypara.radius_y = radius;
	mypara.TH = 0.1;

	if(!do_lobeseg_bdbminus(inimg1d, sz, image->getDatatype(), outimg1d, in_channel_no, out_channel_no, mypara))
	{
		v3d_msg("lobeseg two sides error!");
		return -1;
	}

	Image4DSimple * p4DImage = new Image4DSimple();
	p4DImage->setData((unsigned char*)outimg1d, sz[0], sz[1], sz[2], sz[3] + 1, image->getDatatype());

	v3dhandle newwin;
	if(dialog.is_newwin)
		newwin = callback.newImageWindow();
	else
		newwin = callback.currentImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("lobeseg_two_sides"));
	callback.updateImageWindow(newwin);

	return 1;
}

int lobeseg_one_side_only(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}

	OneSideOnlyDialog dialog(callback, parent);
	if(dialog.exec() == QDialog::Rejected) return -1;
	dialog.update();
	int i = dialog.i;
	int c = dialog.channel;
	double alpha = dialog.alpha;
	double beta = dialog.beta;
	double gamma = dialog.gamma;
	int nloops = dialog.nloops;
	int radius = dialog.radius;
	int x0 = dialog.x0;
	int y0 = dialog.y0;
	int x1 = dialog.x1;
	int y1 = dialog.y1;
	int keep_which = dialog.keep_which;
	int nctrls = dialog.nctrls;
	bool is_surf = dialog.is_surf;

	Image4DSimple * image = callback.getImage(win_list[i]);
	if(image->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	unsigned char * inimg1d = image->getRawData();
	V3DLONG sz[4];
	sz[0] = image->getXDim();
	sz[1] = image->getYDim();
	sz[2] = image->getZDim();
	sz[3] = image->getCDim();
	unsigned char * outimg1d = new unsigned char[sz[0] * sz[1] * sz[2] * (sz[3] + 1)];
	for(V3DLONG i = 0 ; i < sz[0] * sz[1] * sz[2] * sz[3]; i++) outimg1d[i] = inimg1d[i];
	for(V3DLONG i =  sz[0] * sz[1] * sz[2] * sz[3] ; i < sz[0] * sz[1] * sz[2] * (sz[3] + 1); i++) outimg1d[i] = 0;
	int in_channel_no = c;
	int out_channel_no = 1;

	BDB_Minus_ConfigParameter mypara;
	mypara.f_image = alpha;
	mypara.f_smooth = beta;
	mypara.f_length = gamma;
	mypara.nloops = nloops;
	mypara.radius = radius;
	mypara.radius_x = radius;
	mypara.radius_y = radius;
	mypara.TH = 0.1;

	if(!do_lobeseg_bdbminus_onesideonly(inimg1d, sz, image->getDatatype(), outimg1d, in_channel_no, out_channel_no, mypara, x0, y0, x1, y1, keep_which, nctrls, is_surf))
	{
		v3d_msg("lobeseg two sides error!");
		return -1;
	}

	Image4DSimple * p4DImage = new Image4DSimple();
	p4DImage->setData((unsigned char*)outimg1d, sz[0], sz[1], sz[2], sz[3] + 1, image->getDatatype());

	v3dhandle newwin;
	if(dialog.is_newwin)
		newwin = callback.newImageWindow();
	else
		newwin = callback.currentImageWindow();

	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("lobeseg_one_side_only"));
	callback.updateImageWindow(newwin);

	return 1;
}

void printHelp()
{
    printf("\nUsage: vaa3d -p <lobeseg_plugin_name> -n lobeseg -i <input_image_file> -o <output_image_file> -par \"#c <channalNo_reference> #A <alpha: image force> #B <beta: length force> #G <gamma: smoothness force> #n <nloop> #w <local win radius> [#s #p <position> #k <direction> #N <num_of_controls>]\"\n");
    printf("\t -i <input_image_file>              input 3D image (tif, or Hanchuan's RAW or LSM). \n");
    //printf("\t -p <prior control pt file>         input prior location of control points (note: the order will matter!). If unspecified, then randomly initialized. \n");
    printf("\t -o <output_image_file>             output image where the third channel is a mask indicating the regions. \n");
    printf("\t #c <channalNo_reference>           the ID of channel for processing (starting from 0). If unspecified, then initialized as 0.\n");
    printf("\t #A <alpha>                         the alpha coefficient for the image force. Default = 1.0.\n");
    printf("\t #B <beta>                          the beta coefficient for the length force. Default = 0.5.\n");
    printf("\t #G <gamma>                         the gamma coefficient for the smoothness force Default = 0.5.\n");
    printf("\t #n <nloop>                         the number of maximum loops of optimization. If unspecified, then initialized as 500.\n");
    printf("\t #w <local win radius>              the radius of local window for center of mass estimation. The real win size is 2*radius+1. The default radius = 20. \n");
    printf("\t \n");
    printf("\t [#h]                               print this message.\n");
    printf("\t [#s]                               lobe segmentation on one side, -p -k -N parameters is needed.\n");
    printf("\t [#p]                               set the relative initial position, format X1xY1+X2xY2, e.g. 30x0+100x50, top left is the origin.\n");
    printf("\t [#k]                               specifiy which direction will be kept, left or right.\n");
    printf("\t [#N]                               the number of control points.\n");
    printf("\t [#S]                               output the seperating surface.\n");
    printf("\n");
    printf("Demo :\t v3d -x liblobeseg_debug.dylib -f lobeseg -i input.tif -o output.tif -p \"#s #N 20 #p 30x0+100x30 #k left #S\"\n");
    printf("Version: 0.91 (Copyright: Hanchuan Peng)\n");
    return;
}

bool lobeseg(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	if(input.size() == 1) {printHelp(); return true;}
	vector<char*> * infiles = (vector<char*> *)(input.at(0).p);
	vector<char*> * paralist = (vector<char*> *)(input.at(1).p);
	vector<char*> * outfiles = output.empty() ? (new vector<char*>()) : (vector<char*> *)(output.at(0).p);
	if(infiles->empty() || outfiles->empty()) {printHelp(); return true;}

	cout<<"============== Welcome to lobeseg function ================="<<endl;
	//char infile[500] = "";
	//char outfile[500] = "";
	cout<<"input.size() = "<<input.size()<<endl;
	cout<<"output.size() = "<<output.size()<<endl;
	char * infile = (*(vector<char*> *)(input.at(0).p)).at(0);
	char * paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	char * outfile = (*(vector<char*> *)(output.at(0).p)).at(0);

	cout<<"infile : "<<infile<<endl;
	cout<<"outfile : "<<outfile<<endl;
	cout<<"paras : " <<paras<<endl;

	char * err_str = new char[500];
	strcpy(err_str, "");

	int in_channel_no = 0;
	int out_channel_no = 1;
	double alpha = 1.0;
	double beta  = 0.5;
	double gamma = 0.5;
	int nloops = 500;
	int radius = 20;
	int x0 = 30;
	int y0 = 0;
	int x1 = 100;
	int y1 = 50;
	int keep_which = 0; //left
	int nctrls = 20;
	bool is_surf = false;

	bool single_side = false;

	V3DPluginArgItem item;
	item.type = "error string";
	item.p = err_str;
	output<<item;

	char ** argv;
	//int argc = split((char *) input.at(0).p , argv);
	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	cout<<"paras = \""<<paras<<"\""<<endl;
	int argc = split(paras, argv);
	
	char optstring[] = "hsSi:o:c:A:B:G:n:p:k:N:";
	int c;
	optind = 0;
    while((c = getopt_new(argc, argv, optstring)) != -1)
	{
		switch(c)
		{
			case 'h' : printHelp(); return true;
			case 's' : single_side = true; cout<<"single_side = true"<<endl; break;
			case 'S' : is_surf = true; break;
			case 'p' : 
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -p");
						   printf( "Found illegal or NULL parameter for the option -p");
						   return false;
					   }
					   else
					   {
						   string pos(optarg);
						   if(pos.find('+') == string::npos || pos.find_first_of('+') != pos.find_last_of('+') || pos.find('x') == string::npos || pos.find_first_of('x') == pos.find_last_of('x'))
						   {
							   strcpy(err_str, "Found illegal or NULL parameter for the option -p");
							   printf( "Found illegal or NULL parameter for the option -p");
							   return false;
						   }
						   else
						   {
							   string str_x0 = pos.substr(0, pos.find_first_of('x'));
							   string str_y0 = pos.substr(pos.find_first_of('x') + 1, pos.find_first_of('+') - pos.find_first_of('x') -1);
							   string str_x1 = pos.substr(pos.find_first_of('+') + 1, pos.find_last_of('x') - pos.find_first_of('+') - 1);
							   string str_y1 = pos.substr(pos.find_last_of('x') + 1, pos.length() - pos.find_last_of('x') -1);
							   x0 = atoi(str_x0.c_str());
							   y0 = atoi(str_y0.c_str());
							   x1 = atoi(str_x1.c_str());
							   y1 = atoi(str_y1.c_str());
							   if(x0 < 0 || x0 > 100 || y0 < 0 || y0 > 100 || x1 < 0 || x1 > 100 || y1 < 0 || y1 > 100)
							   {
								   strcpy(err_str, "Found illegal or NULL parameter for the option -p");
								   printf( "Found illegal or NULL parameter for the option -p");
								   return false;
							   }
						   }
					   }
					   break;
			case 'k':
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -k");
						   printf( "Found illegal or NULL parameter for the option -k");
						   return false;
					   }
					   else
					   {
						   if(strcmp(optarg,"left") == 0 || strcmp(optarg, "Left") == 0) keep_which = 0;
						   else if(strcmp(optarg,"right") == 0 || strcmp(optarg, "Right") == 0) keep_which = 1;
						   else {
							   strcpy(err_str, "Found illegal or NULL parameter for the option -k");
							   printf( "Found illegal or NULL parameter for the option -k");
							   return false;
						   };
					   }
					   break;
			case 'N' : 
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -N");
						   printf( "Found illegal or NULL parameter for the option -N");
						   return false;
					   }
					   else
					   {
						   //cout<<" number of control points : "<<optarg<<endl;
						   nctrls = atoi(optarg);
						   if(nctrls<=0)
						   {
							   strcpy(err_str, "Found illegal or NULL parameter for the option -N");
							   printf( "Found illegal or NULL parameter for the option -N");
							   return false;
						   }
					   }
					   break;
			case 'i':
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -i");
						   printf( "Found illegal or NULL parameter for the option -i");
						   return false;
					   }
					   strcpy(infile , optarg);
					   break;
			case 'o' :
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -o");
						   printf( "Found illegal or NULL parameter for the option -o");
						   return false;
					   }
					   strcpy(outfile, optarg);
					   break;
			case 'c' :
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -c");
						   printf( "Found illegal or NULL parameter for the option -c");
						   return false;
					   }
					   in_channel_no = atoi (optarg);
					   if (in_channel_no < 0)
					   {
						   strcpy(err_str, "Illegal channel. Not found! It must be >=0.");
						   printf( "Illegal channel. Not found! It must be >=0.");
						   return false;
					   }
					   break;
			case 'A' :
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -A");
						   printf( "Found illegal or NULL parameter for the option -A");
						   return false;
					   }
					   alpha = atof (optarg);
					   if (alpha<0)
					   {
						   strcpy(err_str,"alpha must not be less than than 0.");
						   printf("alpha must not be less than than 0.");
						   return false;
					   }
					   break;
			case 'B' :
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -B");
						   printf( "Found illegal or NULL parameter for the option -B");
						   return false;
					   }
					   beta = atof (optarg);
					   if (beta<0)
					   {
						   strcpy(err_str,"beta must not be less than than 0.");
						   printf("beta must not be less than than 0.");
						   return false;
					   }
					   break;
			case 'G' :
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -G");
						   printf( "Found illegal or NULL parameter for the option -G");
						   return false;
					   }
					   gamma = atof (optarg);
					   if (gamma<0)
					   {
						   strcpy(err_str,"gamma must not be less than than 0.");
						   printf("gamma must not be less than than 0.");
						   return false;
					   }
					   break;
			case 'n' :
					   if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					   {
						   strcpy(err_str, "Found illegal or NULL parameter for the option -n");
						   printf( "Found illegal or NULL parameter for the option -n");
						   return false;
					   }
					   nloops = atoi (optarg);
					   if (nloops < 1)
					   {
						   strcpy(err_str, "The number of loop must be >= 1.");
						   printf( "The number of loop must be >= 1.");
						   return false;
					   }
					   break;
			case '?' :
					   sprintf(err_str, "Unknow option '-%c' or incomplete argument lists.", optopt);
						return false;
		}
	}
	cout<<"parameter analysis over!"<<endl;
	BDB_Minus_ConfigParameter mypara;
	mypara.f_image = alpha;
	mypara.f_smooth = beta;
	mypara.f_length = gamma;
	mypara.nloops = nloops;
	mypara.radius = radius;
	mypara.radius_x = radius;
	mypara.radius_y = radius;
	mypara.TH = 0.1;
	cout<<"alpha = "<<alpha<<endl;
	cout<<"beta = "<<beta<<endl;
	cout<<"gamma = "<<gamma<<endl;
	cout<<"nloops = "<<nloops<<endl;
	cout<<"single_side = "<<single_side<<endl;
	cout<<"is_surf = "<<is_surf<<endl;
	cout<<"x0 = "<<x0<<endl;
	cout<<"y0 = "<<y0<<endl;
	cout<<"x1 = "<<x1<<endl;
	cout<<"y1 = "<<y1<<endl;


	unsigned char * inimg1d = 0;
	V3DLONG *sz=0;
	int datatype;
	loadImage(infile, inimg1d, sz, datatype);

	unsigned char * outimg1d = new unsigned char[sz[0] * sz[1] * sz[2] * (sz[3] + 1)];
	for(V3DLONG i = 0 ; i < sz[0] * sz[1] * sz[2] * sz[3]; i++) outimg1d[i] = inimg1d[i];
	for(V3DLONG i =  sz[0] * sz[1] * sz[2] * sz[3] ; i < sz[0] * sz[1] * sz[2] * (sz[3] + 1); i++) outimg1d[i] = 0;

	if(!single_side)
	{
		cout<<"do two sides lobeseg"<<endl;
		if (!do_lobeseg_bdbminus(inimg1d, sz, datatype, outimg1d, in_channel_no, out_channel_no, mypara))
		{
			strcpy(err_str, "Fail to do two sides lobe segmentation.");
			printf( "Fail to do two sides lobe segmentation.");
			return false;
		}
	}
	else
	{
		cout<<"do one side lobeseg"<<endl;
		if (!do_lobeseg_bdbminus_onesideonly(inimg1d, sz, datatype, outimg1d, in_channel_no, out_channel_no, mypara, x0, y0, x1, y1, keep_which,nctrls, is_surf))
		{
			strcpy(err_str, "Fail to do one side lobe segmentation.");
			printf( "Fail to do one side lobe segmentation.");
			return false;
		}
	}
	sz[3]=sz[3]+1;
	saveImage(outfile, outimg1d,sz, datatype);
	return true;
}


