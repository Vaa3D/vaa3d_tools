//principal skeleton detection plugin
//by Lei Qu
//2009-12-29

// add dofunc() interface by Jianlong Zhou. 2012-05-02

#include <QtGui>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include "plugin_principalskeleton_detection.h"
#include "q_principalskeleton_detection.h"
#include "../../basic_c_fun/stackutil.h"
#include "../../basic_c_fun/basic_surf_objs.h"
#include "../../worm_straighten_c/spline_cubic.h"
#include "q_neurontree_segmentation.h"
#include "q_skeletonbased_warp_sub2tar.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(principalskeleton_detection, PrincipalSkeletonDetectionPlugin)

void PrincipalSkeletonDetection(V3DPluginCallback2 &callback, QWidget *parent);
void SkeletonBasedImgWarp(V3DPluginCallback2 &callback, QWidget *parent);
void OpenDownloadPage(QWidget *parent);
bool readDomain_file(const QString &qs_filename,
					 vector< vector<long> > &vecvec_domain_length_ind,vector<double> &vec_domain_length_weight,
					 vector< vector<long> > &vecvec_domain_smooth_ind,vector<double> &vec_domain_smooth_weight);
bool q_cubicSplineMarker(const QList<ImageMarker> &ql_marker,QList<ImageMarker> &ql_marker_cubicspline);
bool q_saveSkeleton2swcFile_cubicspline(const QList<ImageMarker> &ql_cptpos,const vector< vector<long> > &vecvec_domain_cptind,
										const QString &qs_filename_swc_cubicspline_skeleton_output);

bool do_PrincipalSkeletonDetection(unsigned char *p_img_input, long *sz_img_input, QString &qs_filename_marker_ini, //input
     QString &qs_filename_domain, int &n_index_channel, double &d_stopthresh,  //input
     QList<ImageMarker> &ql_cptpos_output, vector< vector<long> > &vecvec_domain_smooth_ind); //output

bool PrincipalSkeletonDetection(const V3DPluginArgList & input, V3DPluginArgList & output);

bool SkeletonBasedImgWarp(const V3DPluginArgList & input, V3DPluginArgList & output);

bool do_ImgWarp(QString &qs_filename_img_sub, QString &qs_filename_mak_sub, QString &qs_filename_img_tar,
     QString &qs_filename_mak_tar, QString &qs_filename_domain,
     unsigned char* &newdata1d, V3DLONG* &out_sz);


const QString title = "Principal Skeleton Detection demo";
QStringList PrincipalSkeletonDetectionPlugin::menulist() const
{
    return QStringList()
	<< tr("detect prinipcal skeleton...")
	<< tr("warp subject image to target by aligning their skeletons...")
	<< tr("open test data and demo web page")
	<< tr("about this plugin")
	;
}

void PrincipalSkeletonDetectionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if(menu_name==tr("detect prinipcal skeleton..."))
	{
		PrincipalSkeletonDetection(callback, parent);
	}
	else if(menu_name==tr("warp subject image to target by aligning their skeletons..."))
	{
		SkeletonBasedImgWarp(callback, parent);
	}
	else if(menu_name==tr("open test data and demo web page"))
	{
		OpenDownloadPage(parent);
	}
	else if(menu_name==tr("about this plugin"))
	{
        qDebug("about");
        QString msg = QString("version %1 Detecting the pricipal skeleton of an image object (2009-Aug-14): this tool is developed by Lei Qu.").arg(getPluginVersion(), 1, 'f', 1);
		QMessageBox::information(parent, "Version info", msg);
	}
}


QStringList PrincipalSkeletonDetectionPlugin::funclist() const
{
	return QStringList()
		<<tr("detect")
          <<tr("warp")
		<<tr("help");
}


bool PrincipalSkeletonDetectionPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("detect"))
	{
		return PrincipalSkeletonDetection(input, output);
     }
     if (func_name == tr("warp"))
	{
		return SkeletonBasedImgWarp(input, output);
     }
	else if(func_name == tr("help"))
	{
          cout<<"Usage : v3d -x skeleton -f detect -i <inimg_file> <iniskele_file> <domain_file> -o <deformskele_file> <cubicswc_file> -p <channel> <stop_thresh>"<<endl;
		cout<<endl;
		cout<<"inimg_file     name of input image file"<<endl;
          cout<<"iniskele_file  name of input initial skeleton file (.marker)"<<endl;
          cout<<"domain_file    name of input domain definition file (.domain)"<<endl;

          cout<<"deformskele_file    name of output deformed skeleton control points in a marker file (.marker)"<<endl;
          cout<<"cubicswc_file       name of output cubic-spline smoothed skeleton swc file (.swc)"<<endl;

		cout<<"channel        the input channel value, default 2 and start from 0"<<endl;
		cout<<"stop_thresh    stop threshold, default 0.01"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x skeleton -f detect -i input.raw iniskele_file.marker domain_file.domain -o deformskele.marker cubicswc.swc -p 2 0.01"<<endl;
		cout<<endl;

          cout<<"Usage : v3d -x skeleton -f warp -i <subimg_file> <submak_file> <tarimg_file> <tarmak_file> <domain_file> -o <outimg_file>"<<endl;
		cout<<endl;
		cout<<"subimg_file     file name of subject image"<<endl;
          cout<<"submak_file     file name of subject skeleton (.marker)"<<endl;
          cout<<"tarimg_file     file name of target image"<<endl;
          cout<<"tarmak_file     file name of target skeleton (.marker)"<<endl;
          cout<<"domain_file     file name of domain definition (.domain)"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x skeleton -f warp -i subimg.raw submak.marker tarimg.raw tarmak.marker domain_file.domain -o outimg.raw"<<endl;
		cout<<endl;
		return true;
	}


QString qs_filename_img_sub = NULL;
     QString qs_filename_mak_sub = NULL;
	QString qs_filename_img_tar = NULL;
     QString qs_filename_mak_tar = NULL;
	QString qs_filename_domain  = NULL;
     QString filename_img_out = NULL;


}



void OpenDownloadPage(QWidget *parent)
{
    bool b_openurl_worked;
    b_openurl_worked=QDesktopServices::openUrl(QUrl("http://penglab.janelia.org/proj/principal_skeleton/supp/supp_index.htm"));
    if (! b_openurl_worked)
        QMessageBox::warning(parent,
							 "Error opening download page", // title
							 "Please browse to\n"
							 "http://penglab.janelia.org/proj/principal_skeleton/supp/supp_index.htm\n"
							 "to download the test data for this plugin");

}

bool PrincipalSkeletonDetection(const V3DPluginArgList & input, V3DPluginArgList & output)
{
     cout<<"Welcome to Principal Skeleton Detection"<<endl;

     if (output.size() != 1) return false;
	//read arguments
	QString qs_filename_input_img		     =NULL;
	QString qs_filename_marker_ini		=NULL;
	QString qs_filename_domain    		=NULL;
	QString filename_marker_out   		=NULL;
     QString filename_swc_out           	=NULL;

     int n_index_channel=2;
	double d_stopthresh=0.01;

     if (input.size()>=2)
     {
          // input files
          vector<char*> paras_infile = (*(vector<char*> *)(input.at(0).p));
          if(paras_infile.size() <3)
          {
               v3d_msg("There are not enough input files.", 0);
               return false;
          }
          if(paras_infile.size() >= 1) qs_filename_input_img = paras_infile.at(0);
          if(paras_infile.size() >= 2) qs_filename_marker_ini = paras_infile.at(1);
          if(paras_infile.size() >= 3) qs_filename_domain = paras_infile.at(2);
          // output files
          vector<char*> paras_outfile = (*(vector<char*> *)(output.at(0).p));
          if(paras_outfile.size() <2)
          {
               v3d_msg("There are not enough output files.", 0);
               return false;
          }
          if(paras_outfile.size() >= 1) filename_marker_out = paras_outfile.at(0);
          if(paras_outfile.size() >= 2) filename_swc_out = paras_outfile.at(1);

          // input -p
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) n_index_channel = atoi(paras.at(0));
          if(paras.size() >= 2) d_stopthresh = atof(paras.at(1));
     }



     cout<<"qs_filename_input_img:  "<<qPrintable(qs_filename_input_img)<<endl;
     cout<<"qs_filename_marker_ini: "<<qPrintable(qs_filename_marker_ini)<<endl;

     unsigned char *p_img_input=0;
	long *sz_img_input=0;
	int datatype;
	if(!loadImage((char*)qPrintable(qs_filename_input_img), p_img_input, sz_img_input, datatype))
     {
          cerr<<"load image "<< qPrintable(qs_filename_input_img) <<" error!"<<endl;
          return false;
     }

     cout<<"run here!"<<endl;
     // do skeleton detection
     QList<ImageMarker> ql_cptpos_output;
     vector< vector<long> > vecvec_domain_smooth_ind;
     if(! do_PrincipalSkeletonDetection(p_img_input, sz_img_input, qs_filename_marker_ini,
               qs_filename_domain, n_index_channel, d_stopthresh,  // input
               ql_cptpos_output, vecvec_domain_smooth_ind)) //output
     {
          v3d_msg("Error in doing skeleton detection.");
          return false;
     }

     // output
     writeMarker_file(filename_marker_out,ql_cptpos_output);
     q_saveSkeleton2swcFile_cubicspline(ql_cptpos_output,vecvec_domain_smooth_ind,filename_swc_out);

     // free memory
     if(p_img_input){delete []p_img_input; p_img_input=0;}
     if(sz_img_input){delete []sz_img_input; sz_img_input=0;}
     return true;

}

bool do_PrincipalSkeletonDetection(unsigned char *p_img_input, long *sz_img_input, QString &qs_filename_marker_ini,
     QString &qs_filename_domain, int &n_index_channel, double &d_stopthresh,  // input
     QList<ImageMarker> &ql_cptpos_output, vector< vector<long> > &vecvec_domain_smooth_ind) //output
{

	//------------------------------------------------------------------------------------------------------------------------------------

	//read initial marker file (initial skeleton control points position definition)
	QList<ImageMarker> ql_cptpos_input;
	ql_cptpos_input=readMarker_file(qs_filename_marker_ini);
	printf("\t>>read marker file [%s] complete.\n",qPrintable(qs_filename_marker_ini));
    if(ql_cptpos_input.isEmpty())
    {
         v3d_msg("read nothing from input skeleotn control points definition file.");
         printf("ERROR: read nothing from input skeleotn control points definition file.\n");
         return false;
    }
    for(long i=0;i<ql_cptpos_input.size();i++)
         printf("\t\tcpt[%ld]=[%.2f,%.2f,%.2f]\n",i,ql_cptpos_input[i].x,ql_cptpos_input[i].y,ql_cptpos_input[i].z);

    //read domain file (include domain definition and corresponding weight definition)
    vector< vector<long> > vecvec_domain_length_ind;//vecvec_domain_smooth_ind;	//the index of control point of each domain is refer to the corresponding marker file
    vector<double> vec_domain_length_weight,vec_domain_smooth_weight;
    if(!readDomain_file(qs_filename_domain,
						vecvec_domain_length_ind,vec_domain_length_weight,
						vecvec_domain_smooth_ind,vec_domain_smooth_weight))
    {
         v3d_msg("readDomain_file() return false!");
         printf("ERROR: readDomain_file() return false!\n");
         return false;
    }
    printf("\t>>read domain file [%s] complete.\n",qPrintable(qs_filename_domain));
    printf("\t\tdomain - length constraint:\n");
    for(unsigned long i=0;i<vecvec_domain_length_ind.size();i++)
    {
         printf("\t\tweight=%.2f;\tcontol points index=[",vec_domain_length_weight[i]);
         for(unsigned long j=0;j<vecvec_domain_length_ind[i].size();j++)
              printf("%ld,",vecvec_domain_length_ind[i][j]);
         printf("]\n");
    }
    printf("\t\tdomain - smooth constraint:\n");
    for(unsigned long i=0;i<vecvec_domain_smooth_ind.size();i++)
    {
    	printf("\t\tweight=%.2f;\tcontol points index=[",vec_domain_smooth_weight[i]);
    	for(unsigned long j=0;j<vecvec_domain_smooth_ind[i].size();j++)
    		printf("%ld,",vecvec_domain_smooth_ind[i][j]);
    	printf("]\n");
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    //generate MIP 2d image
    printf("\t>>generate MIP image ...\n");
    unsigned char *p_img_MIP=0;
    p_img_MIP=new unsigned char[sz_img_input[0]*sz_img_input[1]*sz_img_input[3]];
    if(!p_img_MIP)
    {
         v3d_msg("Fail to allocate memory for the MIP image!");
         printf("ERROR:Fail to allocate memory for the MIP image. \n");
         return false;
    }

	long pgsz1=sz_img_input[0];
	long pgsz2=sz_img_input[0]*sz_img_input[1];
	long pgsz3=sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
	unsigned char u_MIP_rgb[3];
	for(long y=0;y<sz_img_input[1];y++)
		for(long x=0;x<sz_img_input[0];x++)
		{
			u_MIP_rgb[0]=u_MIP_rgb[1]=u_MIP_rgb[2]=0;
			for(long z=0;z<sz_img_input[2];z++)
				for(long c=0;c<sz_img_input[3];c++)
				{
					long index=pgsz3*c+pgsz2*z+pgsz1*y+x;
					if(p_img_input[index]>u_MIP_rgb[c])
						u_MIP_rgb[c]=p_img_input[index];
				}

			for(long c=0;c<sz_img_input[3];c++)
			{
				long index_MIP=pgsz2*c+pgsz1*y+x;
				p_img_MIP[index_MIP]=u_MIP_rgb[c];
			}
		}

	//------------------------------------------------------------------------------------------------------------------------------------
	//extract the reference channel
	//	int n_index_channel=2;
    printf("\t>>extract the reference [%d]th channel from MIP image: ...\n",n_index_channel);

    unsigned char *p_img_MIP_ref=0;
    p_img_MIP_ref=new unsigned char[sz_img_input[0]*sz_img_input[1]];
    if(!p_img_MIP_ref)
    {
         v3d_msg("Fail to allocate memory for reference image!");
         printf("ERROR: fail to allocate memory for reference image!\n");
         if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
         return false;
    }

	for(long y=0;y<sz_img_input[1];y++)
		for(long x=0;x<sz_img_input[0];x++)
		{
			long index_MIP=pgsz2*n_index_channel+pgsz1*y+x;
			long index_MIP_1c=pgsz1*y+x;
			p_img_MIP_ref[index_MIP_1c]=p_img_MIP[index_MIP];
		}

	//------------------------------------------------------------------------------------------------------------------------------------
	//downsample 1 channel MIP image to given size (for speed) and modify the coordinate of skeleton accordingly
	double d_ratio_sample=2;//d_ratio_sample=sz_ori/sz_sample
	long sz_img_sample[2];	//[0]:width, [1]:height
	sz_img_sample[0]=sz_img_input[0]/d_ratio_sample+0.5;
	sz_img_sample[1]=sz_img_input[1]/d_ratio_sample+0.5;

	printf("\t>>resize image(to [w=%ld,h=%ld]) and skeleton ...\n",sz_img_sample[0],sz_img_sample[1]);
	//downsample 1 channel MIP image
    unsigned char *p_img_sample=0;
    p_img_sample=new unsigned char[sz_img_sample[0]*sz_img_sample[1]];
    if(!p_img_sample)
    {
         v3d_msg("Fail to allocate memory for sample image!");
         printf("ERROR: fail to allocate memory for sample image!\n");
         if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
         if(p_img_MIP_ref) 	{delete []p_img_MIP_ref;	p_img_MIP_ref=0;}
         return false;
    }

	for(long y=0;y<sz_img_sample[1];y++)
		for(long x=0;x<sz_img_sample[0];x++)
		{
			long x_o=x*d_ratio_sample+0.5;
			long y_o=y*d_ratio_sample+0.5;
			x_o=x_o<0?0:x_o;	x_o=x_o>=sz_img_input[0]?sz_img_input[0]-1:x_o;
			y_o=y_o<0?0:y_o;	y_o=y_o>=sz_img_input[1]?sz_img_input[1]-1:y_o;

			long index_o=sz_img_input[0]*y_o+x_o;
			long index_s=sz_img_sample[0]*y+x;
			p_img_sample[index_s]=p_img_MIP_ref[index_o];
		}

	QList<ImageMarker> ql_cptpos_resize(ql_cptpos_input);
	for(long i=0;i<ql_cptpos_input.size();i++)
	{
		ql_cptpos_resize[i].x/=d_ratio_sample;
		ql_cptpos_resize[i].y/=d_ratio_sample;
		ql_cptpos_resize[i].z/=d_ratio_sample;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//principal skeleton detection
    //data structure convert
    vector<point3D64F> vec_cptpos_input,vec_cptpos_output;
    for(long i=0;i<ql_cptpos_input.size();i++)
    {
    	point3D64F temp;
    	temp.x=ql_cptpos_resize[i].x;
    	temp.y=ql_cptpos_resize[i].y;
    	temp.z=ql_cptpos_resize[i].z;
    	vec_cptpos_input.push_back(temp);
    }

    //principal skeleton detection parameters
    PSDParas paras_input;
    paras_input.l_maxitertimes=500;
    paras_input.d_stopiter_threshold=0.01;
    paras_input.d_foreground_treshold=0.5;
    paras_input.l_diskradius_openning=7;
    paras_input.l_diskradius_closing=7;

    //do principal skeleton detection
    if(!q_principalskeleton_detection(
									  p_img_sample,sz_img_sample,
									  vec_cptpos_input,
									  vecvec_domain_length_ind,vec_domain_length_weight,
									  vecvec_domain_smooth_ind,vec_domain_smooth_weight,
									  paras_input,
									  vec_cptpos_output))
    {
         v3d_msg("q_principalskeleton_detection() return false!");
         printf("ERROR:q_principalskeleton_detection() return false!\n");
         if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
         if(p_img_MIP_ref) 	{delete []p_img_MIP_ref;	p_img_MIP_ref=0;}
         if(p_img_sample) 	{delete []p_img_sample;		p_img_sample=0;}
         return false;
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    //data structure convert
    //QList<ImageMarker>
    ql_cptpos_output = ql_cptpos_input;
	for(unsigned long i=0;i<vec_cptpos_output.size();i++)
	{
		ql_cptpos_output[i].x=vec_cptpos_output[i].x*d_ratio_sample;
		ql_cptpos_output[i].y=vec_cptpos_output[i].y*d_ratio_sample;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>Free memory\n");
	if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
	if(p_img_MIP_ref) 	{delete []p_img_MIP_ref;	p_img_MIP_ref=0;}
	if(p_img_sample) 	{delete []p_img_sample;		p_img_sample=0;}

	printf(">>Program exit successful!\n");
	return true;

}

void PrincipalSkeletonDetection(V3DPluginCallback2 &callback, QWidget *parent)
{
	//------------------------------------------------------------------------------------------------------------------------------------
	//get image
	Image4DSimple *p_img4dsimple=0;
	unsigned char *p_img_input=0;
	long sz_img_input[4];

	v3dhandleList h_wndlist=callback.getImageWindowList();
	if(h_wndlist.size()<1)
	{
		QMessageBox::information(0,title,QObject::tr("Need at least 1 image."));
		return;
	}
	else
	{
		p_img4dsimple=callback.getImage(callback.currentImageWindow());
		p_img_input=p_img4dsimple->getRawData();
		sz_img_input[0]=p_img4dsimple->getXDim();
		sz_img_input[1]=p_img4dsimple->getYDim();
		sz_img_input[2]=p_img4dsimple->getZDim();
		sz_img_input[3]=p_img4dsimple->getCDim();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//get initial marker and domain definition filename
	QString qs_filename_marker_ini,qs_filename_domain;
	int n_index_channel=2;
	double d_stopthresh=0.01;
	ParaDialog_PSDetection d(parent);
	if(d.exec()==QDialog::Accepted)
	{
		qs_filename_marker_ini=d.getFilename_mak_ini();
		qs_filename_domain=d.getFilename_domain();
		n_index_channel=d.refChannelLineEdit->text().toInt();
		d_stopthresh=d.refChannelLineEdit->text().toDouble();
	}
	else
		return;


     // do skeleton detection
     QList<ImageMarker> ql_cptpos_output;
     vector< vector<long> > vecvec_domain_smooth_ind;
     if(! do_PrincipalSkeletonDetection(p_img_input, sz_img_input, qs_filename_marker_ini,
               qs_filename_domain, n_index_channel, d_stopthresh,  // input
               ql_cptpos_output, vecvec_domain_smooth_ind)) //output
     {
          v3d_msg("Error in doing skeleton detection.");
          return;
     }


	//output deformation results to files
	QGroupBox *saveOptionGroup=new QGroupBox(parent);
	saveOptionGroup->setTitle(QObject::tr("Save option"));
	QCheckBox *saveskeleotn2markerfileCheckBox=new QCheckBox(saveOptionGroup);
	QCheckBox *saveskeleton2swcfileCheckBox=new QCheckBox(saveOptionGroup);
	saveskeleotn2markerfileCheckBox->setText(QObject::tr("save deformed skeleton control points to marker file"));
	saveskeleton2swcfileCheckBox->setText(QObject::tr("save cubic-spline smoothed skeleton to swc file"));

	QVBoxLayout *saveOptionLayout=new QVBoxLayout(saveOptionGroup);
	saveOptionLayout->addWidget(saveskeleotn2markerfileCheckBox);
	saveOptionLayout->addWidget(saveskeleton2swcfileCheckBox);

	QHBoxLayout *savecancelLayout=new QHBoxLayout;
	QPushButton* save=new QPushButton("Save");
	QPushButton* cancel=new QPushButton("Cancel");
	savecancelLayout->addWidget(save);
	savecancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(saveOptionGroup);
	mainLayout->addWidget(new QLabel(QObject::tr("Note: suffix will be added automatically\n(*.marker, *.swc)")));
	mainLayout->addLayout(savecancelLayout);

	QDialog dialog_showsave(parent);
	dialog_showsave.setWindowTitle(QObject::tr("Output Option"));
	dialog_showsave.setLayout(mainLayout);
	dialog_showsave.connect(save,  SIGNAL(clicked()),&dialog_showsave,SLOT(accept()));
	dialog_showsave.connect(cancel,SIGNAL(clicked()),&dialog_showsave,SLOT(reject()));

	if(dialog_showsave.exec()==QDialog::Accepted)
	{
		QString filename_nosuffix,filename_marker_out,filename_swc_out;
		filename_nosuffix=QFileDialog::getSaveFileName(parent,QString(QObject::tr("Only input filename (without suffix)")));
		filename_marker_out=filename_nosuffix+QString(".marker");
		filename_swc_out=filename_nosuffix+QString(".swc");

		if(saveskeleotn2markerfileCheckBox->isChecked())
		{
			writeMarker_file(filename_marker_out,ql_cptpos_output);
		}
		if(saveskeleton2swcfileCheckBox->isChecked())
		{
			q_saveSkeleton2swcFile_cubicspline(ql_cptpos_output,vecvec_domain_smooth_ind,filename_swc_out);
		}

		if(saveskeleotn2markerfileCheckBox->isChecked() && !saveskeleton2swcfileCheckBox->isChecked())
			QMessageBox::information(0,title,QObject::tr("save to:\n\n>>%1\n\ncomplete!").arg(filename_marker_out));
		else if(!saveskeleotn2markerfileCheckBox->isChecked() && saveskeleton2swcfileCheckBox->isChecked())
			QMessageBox::information(0,title,QObject::tr("save to:\n\n>>%1\n\ncomplete!").arg(filename_swc_out));
		else if(saveskeleotn2markerfileCheckBox->isChecked() && saveskeleton2swcfileCheckBox->isChecked())
			QMessageBox::information(0,title,QObject::tr("save to:\n\n>>%1\n>>%2\n\ncomplete!").arg(filename_marker_out).arg(filename_swc_out));
	}
	else
		return;


	//------------------------------------------------------------------------------------------------------------------------------------

	printf(">>Program exit successful!\n");
	return;
}

//************************************************************************************************************************************
//paradialog for principal skeleton detection
ParaDialog_PSDetection::ParaDialog_PSDetection(QWidget *parent):QDialog(parent)
{
	filePathLineEdit_mak_ini=new QLineEdit(QObject::tr("choose initial skeleton file here (*.marker)"));
	filePathLineEdit_domain=new QLineEdit(QObject::tr("choose domain definition file here (*.domain)"));
	filePathLineEdit_mak_ini->setFixedWidth(300);
	filePathLineEdit_domain->setFixedWidth(300);

	refChannelLabel=new QLabel(QObject::tr("reference channel:"));
	stopThreshLabel=new QLabel(QObject::tr("stop threshold:"));
	foregroundRatioLabel=new QLabel(QObject::tr("foreground ratio:"));
	refChannelLineEdit=new QLineEdit(QObject::tr("2"));
	stopThreshLineEdit=new QLineEdit(QObject::tr("0.01"));
	foregroundRatioLineEdit=new QLineEdit(QObject::tr("0.5"));

	QPushButton *button1=new QPushButton(QObject::tr("..."));
	QPushButton *button2=new QPushButton(QObject::tr("..."));
	QPushButton *ok=new QPushButton("OK");	ok->setDefault(true);
	QPushButton *cancel=new QPushButton("Cancel");

	connect(button1,SIGNAL(clicked()),this,SLOT(openFileDialog_mak_ini()));
	connect(button2,SIGNAL(clicked()),this,SLOT(openFileDialog_domain()));
	connect(ok,	    SIGNAL(clicked()),this,SLOT(accept()));
	connect(cancel, SIGNAL(clicked()),this,SLOT(reject()));

	QGroupBox *fileChooseGroup=new QGroupBox(parent);
	fileChooseGroup->setTitle(QObject::tr("Choose initial skeleton and domain definition file:"));
	QGroupBox *paraGroup=new QGroupBox(parent);
	paraGroup->setTitle(QObject::tr("Parameters:"));

	QGridLayout *fileChooseLayout=new QGridLayout(fileChooseGroup);
	fileChooseLayout->addWidget(filePathLineEdit_mak_ini,0,0);
	fileChooseLayout->addWidget(button1,0,1);
	fileChooseLayout->addWidget(filePathLineEdit_domain,1,0);
	fileChooseLayout->addWidget(button2,1,1);

	QGridLayout *paraLayout=new QGridLayout(paraGroup);
	paraLayout->addWidget(refChannelLabel,0,0);
	paraLayout->addWidget(refChannelLineEdit,0,1);
	paraLayout->addWidget(stopThreshLabel,1,0);
	paraLayout->addWidget(stopThreshLineEdit,1,1);

	QHBoxLayout *okcancelLayout=new QHBoxLayout;
	okcancelLayout->addWidget(ok);
	okcancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(fileChooseGroup);
	mainLayout->addWidget(paraGroup);
	mainLayout->addLayout(okcancelLayout);
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(QObject::tr("Choose initial skeleton and domain definition files"));
	setLayout(mainLayout);
}
void ParaDialog_PSDetection::openFileDialog_mak_ini()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Open initial skeleton file (*.marker)"));
	d.setNameFilter("Marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_mak_ini->setText(selectedFile);
	}
}
void ParaDialog_PSDetection::openFileDialog_domain()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Open domain definition file (*.domain)"));
	d.setNameFilter("Domain file (*.domain)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_domain->setText(selectedFile);
	}
}
QString ParaDialog_PSDetection::getFilename_mak_ini()
{
	return filePathLineEdit_mak_ini->text();
}
QString ParaDialog_PSDetection::getFilename_domain()
{
	return filePathLineEdit_domain->text();
}

//************************************************************************************************************************************
//read domain definition for principal skeleton detection
bool readDomain_file(const QString &qs_filename,
					 vector< vector<long> > &vecvec_domain_length_ind,vector<double> &vec_domain_length_weight,
					 vector< vector<long> > &vecvec_domain_smooth_ind,vector<double> &vec_domain_smooth_weight)
{
	vecvec_domain_length_ind.clear();	vec_domain_length_weight.clear();
	vecvec_domain_smooth_ind.clear();	vec_domain_smooth_weight.clear();

	QFile qf(qs_filename);
	if(!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		printf("ERROR: open file [%s] fail.\n",qPrintable(qs_filename));
		return false;
	}

	long k=0;
    while(!qf.atEnd())
    {
		char curline[2000];
        qf.readLine(curline,sizeof(curline));
		k++;
		{
			if(curline[0]=='#' || curline[0]=='\n' || curline[0]=='\0') continue;

			QStringList qsl=QString(curline).trimmed().split(",");
			int qsl_count=qsl.size();
			if(qsl_count<=3)
			{
				printf("WARNING: invalid format found in line %ld.\n",k);
				continue;
			}

			if(qsl[1].trimmed().toLower()=="length")
			{
				vec_domain_length_weight.push_back(qsl[2].toDouble());

				vector<long> vec_domain_length_ind;
				for(long i=3;i<qsl.size();i++)
					vec_domain_length_ind.push_back(qsl[i].toLong());
				vecvec_domain_length_ind.push_back(vec_domain_length_ind);
			}
			else if(qsl[1].trimmed().toLower()=="smooth")
			{
				vec_domain_smooth_weight.push_back(qsl[2].toDouble());

				vector<long> vec_domain_smooth_ind;
				for(long i=3;i<qsl.size();i++)
					vec_domain_smooth_ind.push_back(qsl[i].toLong());
				vecvec_domain_smooth_ind.push_back(vec_domain_smooth_ind);
			}
			else
				printf("WARNING: unknown constraint type found in line %ld.\n",k);
		}
    }

    return true;
}

//save cubic spline interpolated skeleton to swc file
bool q_saveSkeleton2swcFile_cubicspline(const QList<ImageMarker> &ql_cptpos,const vector< vector<long> > &vecvec_domain_cptind,const QString &qs_filename_swc_cubicspline_skeleton_output)
{
	//check parameters
	if(ql_cptpos.isEmpty())
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: input skeleton position array is empty.\n");
		return false;
	}
	if(vecvec_domain_cptind.size()==0)
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: input domain definition is empty.\n");
		return false;
	}
	if(qs_filename_swc_cubicspline_skeleton_output.length()==0)
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: output file name is empty.\n");
		return false;
	}

	//reorgnize the skeleton into branches according to the domain defintion
	vector< QList<ImageMarker> > vec_ql_brances;
	for(unsigned long i=0;i<vecvec_domain_cptind.size();i++)
	{
		QList<ImageMarker> ql_brances;
		ImageMarker im;
		for(unsigned long j=0;j<vecvec_domain_cptind[i].size();j++)
		{
			long index=vecvec_domain_cptind[i][j];
			im.x=ql_cptpos[index].x;
			im.y=ql_cptpos[index].y;
			im.z=ql_cptpos[index].z;

			ql_brances.push_back(im);
		}
		vec_ql_brances.push_back(ql_brances);
	}

	//cubic spline interpolate every branch respectively
	vector< QList<ImageMarker> > vec_ql_brances_cpline;
	for(unsigned long i=0;i<vec_ql_brances.size();i++)
	{
		QList<ImageMarker> ql_brances_cpline;
		if(vec_ql_brances[i].size()==2)
		{
			ql_brances_cpline.push_back(vec_ql_brances[i][0]);
			ql_brances_cpline.push_back(vec_ql_brances[i][1]);
		}
		else if(vec_ql_brances[i].size()>2)
		{
			if(!q_cubicSplineMarker(vec_ql_brances[i],ql_brances_cpline))
			{
				printf("ERROR: q_cubicSplineMarker() return false.\n");
				return false;
			}
		}
		vec_ql_brances_cpline.push_back(ql_brances_cpline);
	}

	//save to swc file
	NeuronTree nt_skeleton;
	NeuronSWC ns_marker;
	long index=0;
	for(unsigned long i=0;i<vec_ql_brances_cpline.size();i++)
	{
		for(long j=0;j<vec_ql_brances_cpline[i].size();j++)
		{
			index++;

			ns_marker.n=index;					//index
			ns_marker.r=2;						//radius
			ns_marker.x=vec_ql_brances_cpline[i][j].x;	//x
			ns_marker.y=vec_ql_brances_cpline[i][j].y;	//y
			if(j==0)							//parent index
				ns_marker.pn=-1;
			else
				ns_marker.pn=index-1;

			nt_skeleton.listNeuron.push_back(ns_marker);
		}
	}
	writeSWC_file(qs_filename_swc_cubicspline_skeleton_output,nt_skeleton);

	return true;
}

//cubic spline interpolate given marker series
bool q_cubicSplineMarker(const QList<ImageMarker> &ql_marker,QList<ImageMarker> &ql_marker_cubicspline)
{
	//check parameters
	if(ql_marker.isEmpty())
	{
		printf("ERROR: q_cubicSplieMarker: input ql_marker is empty.\n");
		return false;
	}
	if(!ql_marker_cubicspline.isEmpty())
	{
		printf("WARNING: q_cubicSplieMarker: ouput ql_marker_cubicspline is not empty, previoud contents will be deleted.\n");
		ql_marker_cubicspline.clear();
		return false;
	}

	//estimate the cubic spline parameters for given markers
	parameterCubicSpline **cpara=0;
	double *xpos=0, *ypos=0, *zpos=0;
	long NPoints=ql_marker.size();
	xpos=new double[NPoints];
	ypos=new double[NPoints];
	zpos=new double[NPoints];
	if(!xpos || !ypos || !zpos)
	{
		printf("ERROR: q_cubicSplieMarker: Fail to allocate memory for cubic splin control points.\n");
		if(xpos) {delete []xpos; xpos=0;}
		if(ypos) {delete []ypos; ypos=0;}
		if(zpos) {delete []zpos; zpos=0;}
		return false;
	}
	for(int i=0;i<NPoints;i++)
	{
		xpos[i]=ql_marker.at(i).x;
		ypos[i]=ql_marker.at(i).y;
		zpos[i]=ql_marker.at(i).z;
	}
	cpara=est_cubic_spline_2d(xpos,ypos,NPoints,false);

	//cubic spline interpolate the head and butt markers(find all the interpolated locations on the backbone (1-pixel spacing))
	double *cp_x=0,*cp_y=0,*cp_z=0,*cp_alpha=0;
	V3DLONG cutPlaneNum=0;
	if(!interpolate_cubic_spline(cpara,2,cp_x,cp_y,cp_z,cp_alpha,cutPlaneNum))
	{
		printf("ERROR: q_cubicSplieMarker: interpolate_cubic_spline() return false! \n");
		if(xpos) {delete []xpos; xpos=0;}
		if(ypos) {delete []ypos; ypos=0;}
		if(zpos) {delete []zpos; zpos=0;}
		if(cp_x) {delete []cp_x; cp_x=0;}
		if(cp_y) {delete []cp_y; cp_y=0;}
		if(cp_z) {delete []cp_z; cp_z=0;}
		if(cp_alpha) {delete []cp_alpha; cp_alpha=0;}
		if(cpara) //delete the cubic spline parameter data structure
		{
			for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
			delete []cpara; cpara=0;
		}
		return false;
	}

	//fill output structure
	ImageMarker imgmarker;
	for(long i=1;i<cutPlaneNum;i++)//should skip the first point since it is wrong!
	{
		imgmarker.x=cp_x[i];
		imgmarker.y=cp_y[i];
		ql_marker_cubicspline.push_back(imgmarker);
	}

	//free memory
	if(xpos) {delete []xpos; xpos=0;}
	if(ypos) {delete []ypos; ypos=0;}
	if(zpos) {delete []zpos; zpos=0;}
	if(cp_x) {delete []cp_x; cp_x=0;}
	if(cp_y) {delete []cp_y; cp_y=0;}
	if(cp_z) {delete []cp_z; cp_z=0;}
	if(cp_alpha) {delete []cp_alpha; cp_alpha=0;}
	if(cpara) //delete the cubic spline parameter data structure
	{
		for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
		delete []cpara; cpara=0;
	}

	return true;
}


//************************************************************************************************************************************
//paradialog for principal skeleton detection
ParaDialog_PSWarping::ParaDialog_PSWarping(QWidget *parent):QDialog(parent)
{
	filePathLineEdit_img_sub=new QLineEdit(QObject::tr("choose subject image file here (*.lsm,*.raw,*.tif)"));
	filePathLineEdit_mak_sub=new QLineEdit(QObject::tr("choose subject skeleton file here (*.marker)"));
	filePathLineEdit_img_tar=new QLineEdit(QObject::tr("choose target image file here (*.lsm,*.raw,*.tif)"));
	filePathLineEdit_mak_tar=new QLineEdit(QObject::tr("choose target skeleton file here (*.marker)"));
	filePathLineEdit_domain=new QLineEdit(QObject::tr("choose domain definition file here (*.domain)"));
	filePathLineEdit_domain->setFixedWidth(400);

	QPushButton *button1=new QPushButton(QObject::tr("..."));
	QPushButton *button2=new QPushButton(QObject::tr("..."));
	QPushButton *button3=new QPushButton(QObject::tr("..."));
	QPushButton *button4=new QPushButton(QObject::tr("..."));
	QPushButton *button5=new QPushButton(QObject::tr("..."));
	QPushButton *ok=new QPushButton("OK");	ok->setDefault(true);
	QPushButton *cancel=new QPushButton("Cancel");

	connect(button1,SIGNAL(clicked()),this,SLOT(openFileDialog_img_sub()));
	connect(button2,SIGNAL(clicked()),this,SLOT(openFileDialog_mak_sub()));
	connect(button3,SIGNAL(clicked()),this,SLOT(openFileDialog_img_tar()));
	connect(button4,SIGNAL(clicked()),this,SLOT(openFileDialog_mak_tar()));
	connect(button5,SIGNAL(clicked()),this,SLOT(openFileDialog_domain()));
	connect(ok,	    SIGNAL(clicked()),this,SLOT(accept()));
	connect(cancel, SIGNAL(clicked()),this,SLOT(reject()));

	QGroupBox *fileChooseGroup=new QGroupBox(parent);

	QGridLayout *fileChooseLayout=new QGridLayout(fileChooseGroup);
	fileChooseLayout->addWidget(filePathLineEdit_img_sub,0,0);
	fileChooseLayout->addWidget(button1,0,1);
	fileChooseLayout->addWidget(filePathLineEdit_mak_sub,1,0);
	fileChooseLayout->addWidget(button2,1,1);
	fileChooseLayout->addWidget(filePathLineEdit_img_tar,2,0);
	fileChooseLayout->addWidget(button3,2,1);
	fileChooseLayout->addWidget(filePathLineEdit_mak_tar,3,0);
	fileChooseLayout->addWidget(button4,3,1);
	fileChooseLayout->addWidget(filePathLineEdit_domain,4,0);
	fileChooseLayout->addWidget(button5,4,1);

	QHBoxLayout *okcancelLayout=new QHBoxLayout;
	okcancelLayout->addWidget(ok);
	okcancelLayout->addWidget(cancel);

	QVBoxLayout *mainLayout=new QVBoxLayout;
	mainLayout->addWidget(fileChooseGroup);
	mainLayout->addLayout(okcancelLayout);
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(QObject::tr("Choose files for principal skeleton based warping"));
	setLayout(mainLayout);
}
void ParaDialog_PSWarping::openFileDialog_img_sub()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Select subject image file (*.lsm,*.raw,*.tif)"));
	d.setNameFilter("image file (*.lsm *.raw *.tif)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_img_sub->setText(selectedFile);
	}
}
void ParaDialog_PSWarping::openFileDialog_mak_sub()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Select subject skeleton file (*.marker)"));
	d.setNameFilter("Marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_mak_sub->setText(selectedFile);
	}
}
void ParaDialog_PSWarping::openFileDialog_img_tar()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Select target image file (*.lsm,*.raw,*.tif)"));
	d.setNameFilter("image file (*.lsm *.raw *.tif)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_img_tar->setText(selectedFile);
	}
}
void ParaDialog_PSWarping::openFileDialog_mak_tar()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Select target skeleton file (*.marker)"));
	d.setNameFilter("Marker file (*.marker)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_mak_tar->setText(selectedFile);
	}
}
void ParaDialog_PSWarping::openFileDialog_domain()
{
	QFileDialog d(this);
	d.setWindowTitle(tr("Open domain definition file (*.domain)"));
	d.setNameFilter("Domain file (*.domain)");
	if(d.exec())
	{
		QString selectedFile=(d.selectedFiles())[0];
		filePathLineEdit_domain->setText(selectedFile);
	}
}



bool SkeletonBasedImgWarp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
     cout<<"Welcome to Skeleton Warp"<<endl;

     if (output.size() != 1) return false;
	//read arguments
     QString qs_filename_img_sub = NULL;
     QString qs_filename_mak_sub = NULL;
	QString qs_filename_img_tar = NULL;
     QString qs_filename_mak_tar = NULL;
	QString qs_filename_domain  = NULL;
     QString filename_img_out = NULL;

     if (input.size()>=1) // because there is no -p, so >=1
     {
          // input files
          vector<char*> paras_infile = (*(vector<char*> *)(input.at(0).p));
          if(paras_infile.size() <5)
          {
               v3d_msg("There are not enough input files.", 0);
               return false;
          }
          if(paras_infile.size() >= 1) qs_filename_img_sub = paras_infile.at(0);
          if(paras_infile.size() >= 2) qs_filename_mak_sub = paras_infile.at(1);
          if(paras_infile.size() >= 3) qs_filename_img_tar = paras_infile.at(2);
          if(paras_infile.size() >= 4) qs_filename_mak_tar = paras_infile.at(3);
          if(paras_infile.size() >= 5) qs_filename_domain  = paras_infile.at(4);

          // output files
          vector<char*> paras_outfile = (*(vector<char*> *)(output.at(0).p));
          if(paras_outfile.size() >= 1) filename_img_out = paras_outfile.at(0);
     }

     cout<<"subject image:" << qPrintable(qs_filename_img_sub) <<endl;
     cout<<"subject skeleton:" << qPrintable(qs_filename_mak_sub) <<endl;
     cout<<"target image:" << qPrintable(qs_filename_img_tar) <<endl;
     cout<<"target skeleton:" << qPrintable(qs_filename_mak_tar) <<endl;
     cout<<"domain definition:" << qPrintable(qs_filename_domain) <<endl;
     cout<<"output image:" << qPrintable(filename_img_out) <<endl;


     // do_warp
     unsigned char* newdata1d = 0;
     V3DLONG* sz_img_tar = 0;
     if(! do_ImgWarp(qs_filename_img_sub, qs_filename_mak_sub, qs_filename_img_tar, qs_filename_mak_tar, qs_filename_domain,
               newdata1d, sz_img_tar))
     {
          return false;
     }
     // output image
     saveImage((char*)qPrintable(filename_img_out), (unsigned char *)newdata1d, sz_img_tar, 1); // UINT8

     if(newdata1d){delete []newdata1d; newdata1d=0;}
     if(sz_img_tar){delete []sz_img_tar; sz_img_tar=0;}
     return true;
}


void SkeletonBasedImgWarp(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString qs_filename_img_sub,qs_filename_mak_sub;
	QString qs_filename_img_tar,qs_filename_mak_tar;
	QString qs_filename_domain;

	ParaDialog_PSWarping d(parent);
	if(d.exec()==QDialog::Accepted)
	{
		qs_filename_img_sub=d.filePathLineEdit_img_sub->text();
		qs_filename_mak_sub=d.filePathLineEdit_mak_sub->text();
		qs_filename_img_tar=d.filePathLineEdit_img_tar->text();
		qs_filename_mak_tar=d.filePathLineEdit_mak_tar->text();
		qs_filename_domain=d.filePathLineEdit_domain->text();
	}
	else
		return;

     // do warp
     unsigned char* newdata1d = 0;
     V3DLONG* sz_img_tar = 0;
     if(! do_ImgWarp(qs_filename_img_sub, qs_filename_mak_sub, qs_filename_img_tar, qs_filename_mak_tar, qs_filename_domain,
               newdata1d, sz_img_tar))
     {
          return;
     }

     // output
     v3dhandle newwin=callback.newImageWindow();
	Image4DSimple tmp;
     tmp.setData(newdata1d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3],V3D_UINT8);
	callback.setImage(newwin,&tmp);
	callback.updateImageWindow(newwin);
}


bool do_ImgWarp(QString &qs_filename_img_sub, QString &qs_filename_mak_sub, QString &qs_filename_img_tar,
     QString &qs_filename_mak_tar, QString &qs_filename_domain,
     unsigned char* &newdata1d, V3DLONG* &out_sz)
{
	if(qs_filename_img_sub.isEmpty()) {v3d_msg("ERROR: Invalid subject image file name!\n"); return false;}
	if(qs_filename_mak_sub.isEmpty()) {v3d_msg("ERROR: Invalid subject skeleton file name!\n"); return false;}
	if(qs_filename_img_tar.isEmpty()) {v3d_msg("ERROR: Invalid target image file name!\n"); return false;}
	if(qs_filename_mak_tar.isEmpty()) {v3d_msg("ERROR: Invalid target skeleton file name!\n"); return false;}
	if(qs_filename_domain.isEmpty()) {v3d_msg("ERROR: Invalid domain file name!\n"); return false;}

     double d_ctlpt2node_ratio_alongbranch=2.0;
	long l_nctrlpt_cuttingplane=5;
	long l_cuttingplane_width=300;

	//------------------------------------------------------------------------------------------------------------------------------------
	//read target image and prinicpal skeleton marker file
	printf(">>read target image and prinicpal skeleton swc file ...\n");
	//read target image
	unsigned char *p_img_tar=0;
	V3DLONG *sz_img_tar=0;
	int datatype_tar=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar,sz_img_tar,datatype_tar))
	{
		v3d_msg("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
		if(p_img_tar) 		{delete []p_img_tar;		p_img_tar=0;}
		if(sz_img_tar)		{delete []sz_img_tar;		sz_img_tar=0;}
		return false;
	}
	printf("\t>>read image [%s] complete.\n",qPrintable(qs_filename_img_tar));

	//read target prinicpal skeleton position marker file
	QList<ImageMarker> ql_cptpos_tar;
	ql_cptpos_tar=readMarker_file(qs_filename_mak_tar);
	printf("\t>>read marker file [%s] complete.\n",qPrintable(qs_filename_mak_tar));
    if(ql_cptpos_tar.isEmpty())
    {
    	v3d_msg("ERROR: read nothing from input skeleotn control points definition file.\n");
    	return false;
    }
    for(long i=0;i<ql_cptpos_tar.size();i++)
    	printf("\t\tcpt[%ld]=[%.2f,%.2f,%.2f]\n",i,ql_cptpos_tar[i].x,ql_cptpos_tar[i].y,ql_cptpos_tar[i].z);

	//------------------------------------------------------------------------------------------------------------------------------------
	//read subject image and principal skeleton marker file
	printf(">>read subject image and prinicpal skeleton swc file ...\n");
	//read subject image
	unsigned char *p_img_sub=0;
	V3DLONG *sz_img_sub=0;
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub,sz_img_sub,datatype_sub))
	{
		v3d_msg("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
		if(p_img_tar) 		{delete []p_img_tar;		p_img_tar=0;}
		if(p_img_sub) 		{delete []p_img_sub;		p_img_sub=0;}
		if(sz_img_sub)		{delete []sz_img_sub;		sz_img_sub=0;}
		if(sz_img_tar)		{delete []sz_img_tar;		sz_img_tar=0;}
		return false;
	}
	printf("\t>>read image [%s] complete.\n",qPrintable(qs_filename_img_tar));

	//read subject prinicpal skeleton position marker file
	QList<ImageMarker> ql_cptpos_sub;
	ql_cptpos_sub=readMarker_file(qs_filename_mak_sub);
	printf("\t>>read marker file [%s] complete.\n",qPrintable(qs_filename_img_sub));
    if(ql_cptpos_sub.isEmpty())
    {
    	v3d_msg("ERROR: read nothing from input skeleotn control points definition file.\n");
    	return false;
    }
    for(long i=0;i<ql_cptpos_sub.size();i++)
    	printf("\t\tcpt[%ld]=[%.2f,%.2f,%.2f]\n",i,ql_cptpos_sub[i].x,ql_cptpos_sub[i].y,ql_cptpos_sub[i].z);

    //------------------------------------------------------------------------------------------------------------------------------------
    //read domain file (include domain definition and corresponding weight definition)
    vector< vector<long> > vecvec_domain_length_ind,vecvec_domain_smooth_ind;	//the index of control point of each domain is refer to the corresponding marker file
    vector<double> vec_domain_length_weight,vec_domain_smooth_weight;
    if(!readDomain_file(qs_filename_domain,
    		vecvec_domain_length_ind,vec_domain_length_weight,
    		vecvec_domain_smooth_ind,vec_domain_smooth_weight))
    {
    	v3d_msg("ERROR: readDomain_file() return false!\n");
    	return false;
    }
    printf("\t>>read domain file [%s] complete.\n",qPrintable(qs_filename_domain));
    printf("\t\tdomain - length constraint:\n");
    for(unsigned long i=0;i<vecvec_domain_length_ind.size();i++)
    {
    	printf("\t\tweight=%.2f;\tcontol points index=[",vec_domain_length_weight[i]);
    	for(unsigned long j=0;j<vecvec_domain_length_ind[i].size();j++)
    		printf("%ld,",vecvec_domain_length_ind[i][j]);
    	printf("]\n");
    }
    printf("\t\tdomain - smooth constraint:\n");
    for(unsigned long i=0;i<vecvec_domain_smooth_ind.size();i++)
    {
    	printf("\t\tweight=%.2f;\tcontol points index=[",vec_domain_smooth_weight[i]);
    	for(unsigned long j=0;j<vecvec_domain_smooth_ind[i].size();j++)
    		printf("%ld,",vecvec_domain_smooth_ind[i][j]);
    	printf("]\n");
    }

	//------------------------------------------------------------------------------------------------------------------------------------
	//reorganize the nodes for the standardization (according to the definition of smooth constraint domain)
	vector< QList<ImageMarker> > vec_ql_branchcpt_tar,vec_ql_branchcpt_sub;
	for(unsigned long i=0;i<vecvec_domain_smooth_ind.size();i++)
	{
		QList<ImageMarker> ql_branchcpt_tar,ql_branchcpt_sub;
		for(unsigned long j=0;j<vecvec_domain_smooth_ind[i].size();j++)
		{
			ImageMarker im_marker;
			long l_cptind=vecvec_domain_smooth_ind[i][j];

			im_marker.x=ql_cptpos_tar[l_cptind].x;
			im_marker.y=ql_cptpos_tar[l_cptind].y;
			im_marker.z=ql_cptpos_tar[l_cptind].z;
			ql_branchcpt_tar.push_back(im_marker);

			im_marker.x=ql_cptpos_sub[l_cptind].x;
			im_marker.y=ql_cptpos_sub[l_cptind].y;
			im_marker.z=ql_cptpos_sub[l_cptind].z;
			ql_branchcpt_sub.push_back(im_marker);
		}
		vec_ql_branchcpt_tar.push_back(ql_branchcpt_tar);
		vec_ql_branchcpt_sub.push_back(ql_branchcpt_sub);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//warp subject image to target image
	printf(">>warp subject image to target image based on given principal skeleton ...\n");
	unsigned char *p_img_sub2tar=0;
	vector<Coord2D64F_SL> vec_controlpoint_tar,vec_controlpoint_sub;

	if(!q_skeletonbased_sub2tar(
			p_img_tar,sz_img_tar,
			p_img_sub,sz_img_sub,
			vec_ql_branchcpt_tar,vec_ql_branchcpt_sub,
			d_ctlpt2node_ratio_alongbranch,l_nctrlpt_cuttingplane,l_cuttingplane_width,
			p_img_sub2tar,
			vec_controlpoint_tar,vec_controlpoint_sub))
    {
    	v3d_msg("ERROR: q_skeletonbased_sub2tar() return false.\n");
    	if(p_img_tar) 		{delete []p_img_tar;		p_img_tar=0;}
    	if(p_img_sub) 		{delete []p_img_sub;		p_img_sub=0;}
    	if(p_img_sub2tar) 	{delete []p_img_sub2tar;	p_img_sub2tar=0;}
    	if(sz_img_sub)		{delete []sz_img_sub;		sz_img_sub=0;}
    	if(sz_img_tar)		{delete []sz_img_tar;		sz_img_tar=0;}
    	return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//align mess center in z dir
	long l_refchannel=0;
	unsigned char *p_img_shift=new unsigned char[sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2]*sz_img_tar[3]]();
	if(!p_img_shift)
	{
		printf("ERROR: Fail to allocate memory for sub2tar warpped image.\n");
    	if(p_img_tar) 		{delete []p_img_tar;		p_img_tar=0;}
    	if(p_img_sub) 		{delete []p_img_sub;		p_img_sub=0;}
    	if(p_img_sub2tar) 	{delete []p_img_sub2tar;	p_img_sub2tar=0;}
    	if(sz_img_sub)		{delete []sz_img_sub;		sz_img_sub=0;}
    	if(sz_img_tar)		{delete []sz_img_tar;		sz_img_tar=0;}
		return false;
	}

	//find the masscenter of target and sub2tar warped image
	double d_masscenter_z_tar=0,d_masscenter_z_sub2tar=0,d_intensitysum_tar=0,d_intensitysum_sub2tar=0;
	unsigned char ****p_img_tar_4d=0,****p_img_shift_4d=0,****p_img_sub2tar_4d=0;
	if(!new4dpointer(p_img_tar_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3],p_img_tar) ||
	   !new4dpointer(p_img_shift_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3],p_img_shift) ||
	   !new4dpointer(p_img_sub2tar_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3],p_img_sub2tar))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
    	if(p_img_tar) 		{delete []p_img_tar;		p_img_tar=0;}
    	if(p_img_sub) 		{delete []p_img_sub;		p_img_sub=0;}
    	if(p_img_sub2tar) 	{delete []p_img_sub2tar;	p_img_sub2tar=0;}
		if(p_img_shift) 	{delete []p_img_shift;	p_img_shift=0;}
    	if(sz_img_sub)		{delete []sz_img_sub;		sz_img_sub=0;}
    	if(sz_img_tar)		{delete []sz_img_tar;		sz_img_tar=0;}
		if(p_img_tar_4d) 		{delete4dpointer(p_img_tar_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);}
		if(p_img_shift_4d) 		{delete4dpointer(p_img_shift_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);}
		if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);}
		return false;
	}
	for(long z=0;z<sz_img_tar[2];z++)
		for(long y=0;y<sz_img_tar[1];y++)
			for(long x=0;x<sz_img_tar[0];x++)
			{
				d_intensitysum_tar+=p_img_tar_4d[l_refchannel][z][y][x];
				d_masscenter_z_tar+=z*p_img_tar_4d[l_refchannel][z][y][x];

				d_intensitysum_sub2tar+=p_img_sub2tar_4d[l_refchannel][z][y][x];
				d_masscenter_z_sub2tar+=z*p_img_sub2tar_4d[l_refchannel][z][y][x];
			}
	d_masscenter_z_tar/=d_intensitysum_tar;
	d_masscenter_z_sub2tar/=d_intensitysum_sub2tar;

	//align the mass center of sub2tar image to that of target image
	long l_offset_z=d_masscenter_z_tar-d_masscenter_z_sub2tar+0.5;
	for(long z=0;z<sz_img_tar[2];z++)
	{
		long z_sub2tar=z-l_offset_z;
		if(z_sub2tar>=sz_img_tar[2])	z_sub2tar=sz_img_tar[2]-1;
		if(z_sub2tar<0)					z_sub2tar=0;

		for(long y=0;y<sz_img_tar[1];y++)
			for(long x=0;x<sz_img_tar[0];x++)
				for(long c=0;c<sz_img_tar[3];c++)
					p_img_shift_4d[c][z][y][x]=p_img_sub2tar_4d[c][z_sub2tar][y][x];
	}

	//swap pointer
	if(p_img_sub2tar) 	{delete []p_img_sub2tar;	p_img_sub2tar=0;}
	p_img_sub2tar=p_img_shift;	p_img_shift=0;

	if(p_img_tar_4d) 		{delete4dpointer(p_img_tar_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);}
	if(p_img_shift_4d) 		{delete4dpointer(p_img_shift_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);}
	if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);}



     //------------------------------------------------------------------------------------------------------------------------------------
	//push warped image to v3d
	unsigned long l_npixel_s=sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2]*sz_img_tar[3];
	//unsigned char*
     newdata1d=new unsigned char[l_npixel_s]();
	memcpy(newdata1d,p_img_sub2tar,l_npixel_s);
     out_sz = sz_img_tar;


//	printf(">>save subject to target warped image ...\n");
//	saveImage(p_filename_img_sub2tar,(unsigned char *)p_img_sub2tar,sz_img_tar,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>free memory ...\n");
	if(p_img_tar) 		{delete []p_img_tar;		p_img_tar=0;}
	if(p_img_sub) 		{delete []p_img_sub;		p_img_sub=0;}
	if(p_img_sub2tar) 	{delete []p_img_sub2tar;	p_img_sub2tar=0;}
     //if(sz_img_tar)		{delete []sz_img_tar;		sz_img_tar=0;}
	if(sz_img_sub)		{delete []sz_img_sub;		sz_img_sub=0;}

	printf("Program exit successful!\n");
     return true;

}
