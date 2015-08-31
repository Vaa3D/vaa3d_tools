/* Authors: Ching-Wei Wang (cweiwang@mail.ntust.edu.tw), M. Hilmil Muchtar Aditya Pradana, Cheng-Ta Huang
 * Institute: Medical Image Research Center, National Taiwan University of Science & Technology
 * Web: http://www-o.ntust.edu.tw/~cweiwang/
 * License: This plugin uses the same license as Vaa3D.
 * This method is called "Ensemble Neuron Tracer", which combines multiple segmentation results by three simple methods adapted from APP2[citation 1].
 * Citation 1:  Hang Xiao, Hanchuan Peng, APP2: automatic tracing of 3D neuron morphology based on hierarchical pruning of a gray-weighted image distance-tree. Bioinformatics 29(11): 1448-1454 (2013)
 */
 
#include "v3d_message.h"
#include <vector>
#include <algorithm>
#include <string>
#include "basic_surf_objs.h"

#include "EnsembleNeuronTracerBasic.h"

#include "vn_imgpreprocess.h"
#include "fastmarching_dt.h"
#include "fastmarching_tree.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"

Q_EXPORT_PLUGIN2(APP2_ported, APP2_ported);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList APP2_ported::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList APP2_ported::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void APP2_ported::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("Authors: Ching-Wei Wang(cweiwang@mail.ntust.edu.tw), M. Hilmil Muchtar Aditya Pradana, Cheng-Ta Huang\nInstitute: Medical Image Research Center, National Taiwan University of Science & Technology\nWeb: http://www-o.ntust.edu.tw/~cweiwang/\nLicense: This plugin uses the same license as Vaa3D."));
	}
}

bool APP2_ported::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN
		printf("**** Usage of CWlab_method1_version1 tracing **** \n");
		printf("vaa3d -x CWlab_method1_version1 -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;
	return true;
}

vector<MyMarker*> combining(vector<MyMarker*> main, vector<MyMarker*> adding);

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
	Image4DSimple* p4DImage;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        if(p4DImage->getDatatype()!=V3D_UINT8)
        {
            QMessageBox::information(0, "", "Please convert the image to be UINT8 and try again!");
            return;
        }
        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();
		unsigned char * inimg1d = p4DImage->getRawDataAtChannel(-1);
        bool ok1;
        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel", "Enter channel NO:", 1, 1, sc, 1, &ok1);
        }
        if(!ok1) return;
        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;
        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }

        if(datatype !=1)
        {
            fprintf (stderr, "Please convert the image to be UINT8 and try again!\n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
	
	int se;
    V3DLONG pagesz = N*M*P;
    unsigned char *data1d_1ch;
    try {data1d_1ch = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for data1d_1ch."); return;}

    for(V3DLONG i = 0; i < pagesz; i++)
        data1d_1ch[i] = data1d[i+(c-1)*pagesz];

    Image4DSimple * p4dImageNew = 0;
    p4dImageNew = new Image4DSimple;

    if(!p4dImageNew->createImage(N,M,P,1, V3D_UINT8))
        return;

    memcpy(p4dImageNew->getRawData(), data1d_1ch, pagesz);

    unsigned char * indata1d = p4dImageNew->getRawDataAtChannel(0);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BW Function
	
	V3DLONG tb = in_sz[0]*in_sz[1]*in_sz[2]*p4DImage->getUnitBytes();
	
	unsigned char * nm = NULL;
	try {
		nm = new unsigned char [tb];
	}
	catch (...) {
		throw("Fail to allocate memory in Image Thresholding plugin.");
	}
	
	unsigned char * imdilate = NULL;
	try {
		imdilate = new unsigned char [tb];
	}
	catch (...) {
		throw("Fail to allocate memory in Image Thresholding plugin.");
	}
	
	vector<MyMarker*> outswc[3];
	float * phi = 0;
	vector<MyMarker *> outtree[3];
	V3DLONG max_loc = 0;
	double max_val = -1;
	
	for (V3DLONG i=0;i<tb;i++)
	{
		if (indata1d[i]>=1) nm[i] = 255;
		else nm[i] = 0;
	}
	
	in_sz[0] = N;
	in_sz[1] = M;
	in_sz[2] = P;
	in_sz[3] = sc;
	int j = 0;
	
	QString swc_name = PARA.inimg_file + "_Cwlab_ver1.swc";
	QByteArray temp = swc_name.toLatin1();
	char* path = temp.data();
//	ofstream file(path);
	
//	file << "#Supervisor: Prof Ching-Wei Wang (cweiwang@mail.ntust.edu.tw)\n#Authors: Ching-Wei Wang, M. Hilmil Muchtar Aditya Pradana (hilmil.041@gmail.com), Cheng-Ta Huang (chengta.huang@mail.ntust.edu.tw)\n#Institute: Medical Image Research Center, National Taiwan University of Science & Technology\n#Web: http://www-o.ntust.edu.tw/~cweiwang/\n#License: This plugin uses the same license as Vaa3D.\n";
	
	for (int iteration=0;iteration<3;iteration++)
	{
		in_sz[0] = N;
		in_sz[1] = M;
		in_sz[2] = P;
		in_sz[3] = sc;
		se = 1; // strell

		if (iteration!=0)
		{
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Closing
			// Create 3D image
			// dilate
			for (V3DLONG i=0;i<tb;i++)
			{
				if (nm[i]==255)
				{
					imdilate[i] = 255;
					for (V3DLONG j=0;j<se;j++)
					{
						for (V3DLONG k=0;k<se;k++)
						{
							for (V3DLONG l=0;l<se;l++)
							{
								if ((((i+(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))-(1*(l+1)))>=0 && (((i+(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))+(1*(l+1)))>=0 && ((i+(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))-(1*(l+1)))>=0 && ((i+(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))+(1*(l+1)))>=0 && (((i-(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))-(1*(l+1)))>=0 && (((i-(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))+(1*(l+1)))>=0 && ((i-(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))-(1*(l+1)))>=0 && ((i-(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))+(1*(l+1)))>=0)
								{
									//back  //bottom mid //left
									imdilate[((i+(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))-(1*(l+1))] = 255;
									//back  //bottom mid //right
									imdilate[((i+(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))+(1*(l+1))] = 255;
									//back  //top mid //left
									imdilate[(i+(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))-(1*(l+1))] = 255;
									//back  //top mid //right
									imdilate[(i+(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))+(1*(l+1))] = 255;
									
									//front //bottom mid //left
									imdilate[((i-(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))-(1*(l+1))] = 255;
									//front //bottom mid //right
									imdilate[((i-(in_sz[0]*in_sz[1]*j))+(in_sz[0]*(k+1)))+(1*(l+1))] = 255;
									//from //top mid //left
									imdilate[(i-(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))-(1*(l+1))] = 255;
									//from //top mid //right
									imdilate[(i-(in_sz[0]*in_sz[1]*(j+1))-(in_sz[0]*(k+1)))+(1*(l+1))] = 255;
								}
							}
						}
					}
				}
				else
				{
					imdilate[i] = 0;
				}
			}
		}
		else if (iteration==2)
		{
			//erode
			for (V3DLONG i=0;i<tb;i++)
			{
				//front //right //top
		//		imdilate[i-(in_sz[0]*in_sz[1]*se)-se-(se*in_sz[0])]
				//front //right //bottom
		//		imdilate[i-(in_sz[0]*in_sz[1]*se)-se+(se*in_sz[0])]
				
				//front //left //top
		//		imdilate[i-(in_sz[0]*in_sz[1]*se)+se-(se*in_sz[0])]
				//front //left //bottom
		//		imdilate[i-(in_sz[0]*in_sz[1]*se)+se+(se*in_sz[0])]
				
				//back //right //top
		//		imdilate[i+(in_sz[0]*in_sz[1]*se)-se-(se*in_sz[0])]
				//back //right //bottom
		//		imdilate[i+(in_sz[0]*in_sz[1]*se)-se+(se*in_sz[0])]
				
				//back //left //top
		//		imdilate[i+(in_sz[0]*in_sz[1]*se)+se-(se*in_sz[0])]
				//back //left //bottom
		//		imdilate[i+(in_sz[0]*in_sz[1]*se)+se+(se*in_sz[0])]
		
				if (nm[i]!=imdilate[i])
				{
					if ((i-(in_sz[0]*in_sz[1]*(se-1))-(se-1)-((se-1)*in_sz[0]))>=0 && (i-(in_sz[0]*in_sz[1]*se)-se+(se*in_sz[0]))>=0 && (i-(in_sz[0]*in_sz[1]*se)+se-(se*in_sz[0]))>=0 && (i-(in_sz[0]*in_sz[1]*se)+se+(se*in_sz[0]))>=0 && (i+(in_sz[0]*in_sz[1]*se)-se-(se*in_sz[0]))>=0 && (i+(in_sz[0]*in_sz[1]*se)-se+(se*in_sz[0]))>=0 && (i+(in_sz[0]*in_sz[1]*se)+se-(se*in_sz[0]))>=0 && (i+(in_sz[0]*in_sz[1]*se)+se+(se*in_sz[0])))
					{
						if(imdilate[i-(in_sz[0]*in_sz[1]*(se-1))-(se-1)-((se-1)*in_sz[0])]!=255 || imdilate[i-(in_sz[0]*in_sz[1]*se)-se+(se*in_sz[0])]!=255 || imdilate[i-(in_sz[0]*in_sz[1]*se)+se-(se*in_sz[0])]!=255 || imdilate[i-(in_sz[0]*in_sz[1]*se)+se+(se*in_sz[0])]!=255 || imdilate[i+(in_sz[0]*in_sz[1]*se)-se-(se*in_sz[0])]!=255 || imdilate[i+(in_sz[0]*in_sz[1]*se)-se+(se*in_sz[0])]!=255 || imdilate[i+(in_sz[0]*in_sz[1]*se)+se-(se*in_sz[0])]!=255 || imdilate[i+(in_sz[0]*in_sz[1]*se)+se+(se*in_sz[0])]!=255)
						{
							nm[i] = 0;
						}
						else
						{
							nm[i] = 255;
						}
					}
				}
			}
		}
		if (iteration==0 || iteration==2)
		{
			indata1d = nm;
		}
		else if (iteration==1)
		{
			indata1d = imdilate;
		}
		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		in_sz[3] = 1;
		double dfactor_xy = 1, dfactor_z = 1;

		if (in_sz[0]<=256 && in_sz[2]<=256 && in_sz[2]<=256)
		{
			dfactor_z = dfactor_xy = 1;
		}
		else if (in_sz[0] >= 2*in_sz[2] || in_sz[1] >= 2*in_sz[2])
		{
			if (in_sz[2]<=256)
			{
				double MM = in_sz[0];
				if (MM<in_sz[1]) MM=in_sz[1];
				dfactor_xy = MM / 256.0;
				dfactor_z = 1;
			}
			else
			{
				double MM = in_sz[0];
				if (MM<in_sz[1]) MM=in_sz[1];
				if (MM<in_sz[2]) MM=in_sz[2];
				dfactor_xy = dfactor_z = MM / 256.0;
			}
		}
		else
		{
			double MM = in_sz[0];
			if (MM<in_sz[1]) MM=in_sz[1];
			if (MM<in_sz[2]) MM=in_sz[2];
			dfactor_xy = dfactor_z = MM / 256.0;
		}
		
		printf("dfactor_xy=%5.3f\n", dfactor_xy);
		printf("dfactor_z=%5.3f\n", dfactor_z);

		if (dfactor_z>1 || dfactor_xy>1)
		{
			v3d_msg("enter ds code", 0);

			V3DLONG out_sz[4];
			
			unsigned char * outimg=0;
			if (!downsampling_img_xyz( indata1d, in_sz, dfactor_xy, dfactor_z, outimg, out_sz))
				return;
			
			Image4DSimple * p4dImageNew2 = 0;
			p4dImageNew2 = new Image4DSimple;
			
			p4dImageNew2->setData(outimg, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);

			indata1d = p4dImageNew2->getRawDataAtChannel(0);
			in_sz[0] = p4dImageNew2->getXDim();
			in_sz[1] = p4dImageNew2->getYDim();
			in_sz[2] = p4dImageNew2->getZDim();
			in_sz[3] = p4dImageNew2->getCDim();
		}

		cout<<"Start detecting cellbody"<<endl;
		float * phi = 0;
		vector<MyMarker> inmarkers;
		
		fastmarching_dt_XY(indata1d, phi, in_sz[0], in_sz[1], in_sz[2],2, 10);
		V3DLONG sz0 = in_sz[0];
		V3DLONG sz1 = in_sz[1];
		V3DLONG sz2 = in_sz[2];
		V3DLONG sz01 = sz0 * sz1;
		V3DLONG tol_sz = sz01 * sz2;
	
		for(V3DLONG i = 0; i < tol_sz; i++)
		{
			if(phi[i] > max_val)
			{
				max_val = phi[i];
				max_loc = i;
			}
		}
		MyMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);
		inmarkers.push_back(max_marker);

		cout<<"======================================="<<endl;
		cout<<"Construct the neuron tree"<<endl;

		v3d_msg("8bit", 0);

		fastmarching_tree(inmarkers[0], indata1d, outtree[iteration], in_sz[0], in_sz[1], in_sz[2], 2, 10, false);
		cout<<"======================================="<<endl;

		//save a copy of the ini tree
		cout<<"Save the initial unprunned tree"<<endl;
		vector<MyMarker*> & inswc = outtree[iteration];
		
		if (1)
		{
			V3DLONG tmpi;

			vector<MyMarker*> tmpswc;
			for (tmpi=0; tmpi<inswc.size(); tmpi++)
			{
				MyMarker * curp = new MyMarker(*(inswc[tmpi]));
				tmpswc.push_back(curp);

				if (dfactor_xy>1) inswc[tmpi]->x *= dfactor_xy;
				inswc[tmpi]->x += (0);
				if (dfactor_xy>1) inswc[tmpi]->x += dfactor_xy/2;

				if (dfactor_xy>1) inswc[tmpi]->y *= dfactor_xy;
				inswc[tmpi]->y += (0);
				if (dfactor_xy>1) inswc[tmpi]->y += dfactor_xy/2;

				if (dfactor_z>1) inswc[tmpi]->z *= dfactor_z;
				inswc[tmpi]->z += (0);
				if (dfactor_z>1)  inswc[tmpi]->z += dfactor_z/2;
			}
			
			for (tmpi=0; tmpi<inswc.size(); tmpi++)
			{
				inswc[tmpi]->x = tmpswc[tmpi]->x;
				inswc[tmpi]->y = tmpswc[tmpi]->y;
				inswc[tmpi]->z = tmpswc[tmpi]->z;
			}

			for(tmpi = 0; tmpi < tmpswc.size(); tmpi++)
				delete tmpswc[tmpi];
			tmpswc.clear();
		}

		cout<<"Pruning neuron tree"<<endl;

		v3d_msg("start to use happ.\n", 0);
		happ(inswc, outswc[iteration], indata1d, in_sz[0], in_sz[1], in_sz[2],10, 5, 0.3333);

		if (p4dImageNew) {delete p4dImageNew; p4dImageNew=0;} //free buffe

		inmarkers[0].x *= dfactor_xy;
		inmarkers[0].y *= dfactor_xy;
		inmarkers[0].z *= dfactor_z;

		for(V3DLONG i = 0; i < outswc[iteration].size(); i++)
		{
			if (dfactor_xy>1) outswc[iteration][i]->x *= dfactor_xy;
			outswc[iteration][i]->x += 0;
			if (dfactor_xy>1) outswc[iteration][i]->x += dfactor_xy/2;

			if (dfactor_xy>1) outswc[iteration][i]->y *= dfactor_xy;
			outswc[iteration][i]->y += 0;
			if (dfactor_xy>1) outswc[iteration][i]->y += dfactor_xy/2;

			if (dfactor_z>1) outswc[iteration][i]->z *= dfactor_z;
			outswc[iteration][i]->z += 0;
			if (dfactor_z>1)  outswc[iteration][i]->z += dfactor_z/2;

			outswc[iteration][i]->radius *= dfactor_xy; //use xy for now
		}
	}
	
	double real_thres = 40;
	int method_radius_est = 2;
	V3DLONG szOriginalData[4] = {N, M, P, 1};
	
	//Combine image
	for (int a=0;a<3;a++)
	{
		for(V3DLONG i = 0; i < outswc[a].size(); i++)
		{
			outswc[a][i]->radius = markerRadius(data1d_1ch, szOriginalData, *(outswc[a][i]), real_thres, method_radius_est);
		}
	}
	
/*	QString swc_name2 = PARA.inimg_file + "_CWLab_1.swc";
    QString swc_name3 = PARA.inimg_file + "_CWLab_2.swc";
    QString swc_name4 = PARA.inimg_file + "_CWLab_3.swc";
	saveSWC_file(swc_name2.toStdString(), outswc[0]);
	saveSWC_file(swc_name3.toStdString(), outswc[1]);
	saveSWC_file(swc_name4.toStdString(), outswc[2]);*/
	

	vector<MyMarker*> result;
	vector<MyMarker*> result1;

	result1 = combining(outswc[0], outswc[1]);
	result = combining(result1, outswc[2]);


	for(V3DLONG i = 0; i < result.size(); i++)
	{
		result[i]->radius = markerRadius(data1d_1ch, szOriginalData, *(result[i]), real_thres, method_radius_est);
	}
	
	//Output
	QString swc_name1 = PARA.inimg_file + "_EnsembleNeuronTracerBasic.swc";
	saveSWC_file(swc_name1.toStdString(), result);
	
	if(phi){delete [] phi; phi = 0;}
	if(data1d_1ch){delete []data1d_1ch; data1d_1ch = 0;}

	if(!bmenu)
	{
		if(data1d) {delete []data1d; data1d = 0;}
	}

	v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name1.toStdString().c_str()),bmenu);

	return;
}

struct location {
	int x;
	int y;
	int z;
};

struct maps {
	location main;
	location adding;
};

vector<MyMarker*> combining(vector<MyMarker*> main, vector<MyMarker*> adding)
{
	vector<maps> dataLocation;
	vector<MyMarker*> result;
	vector<MyMarker*> root;
	result = main;
	maps insert;
	MyMarker* addingCheck;
	MyMarker* check;
	location ins1;
	location ins2;
	V3DLONG size = result.size();
	V3DLONG idx;
	double minimum;
	double temp;
	double Threshold = 2;
	int x, y, z;
	bool flag;

	for(V3DLONG i=0;i<adding.size();i++)
	{
		minimum = 1000.0;
		for(V3DLONG j=0;j<main.size();j++)
		{
			temp = sqrt(double(((main[j]->x - adding[i]->x)*(main[j]->x - adding[i]->x))+((main[j]->y - adding[i]->y)*(main[j]->y - adding[i]->y))+((main[j]->z - adding[i]->z)*(main[j]->z - adding[i]->z))));
			if (temp<minimum)
			{
				minimum = temp;
				x = main[j]->x;
				y = main[j]->y;
				z = main[j]->z;
			}
		}
		if (minimum<=Threshold)
		{
			ins1.x = x;
			ins1.y = y;
			ins1.z = z;
			
			ins2.x = adding[i]->x;
			ins2.y = adding[i]->y;
			ins2.z = adding[i]->z;
			insert.main = ins1;
			insert.adding = ins2;
			dataLocation.push_back(insert);
		}
		else
		{
			// gak ada di map
			ins1.x = x;
			ins1.y = y;
			ins1.z = z;
			
			ins2.x = adding[i]->x;
			ins2.y = adding[i]->y;
			ins2.z = adding[i]->z;
			insert.main = ins1;
			insert.adding = ins2;
			dataLocation.push_back(insert);
			result.push_back(adding[i]);
		}
	}
	int count = 0;
	for (V3DLONG i=size;i<result.size(); i++)
	{
		idx = 0;
		flag = true;
		for(V3DLONG j=0;j<result.size();j++)
		{
			if (result[i]->parent == result[j])
			{
				flag = false;
			}
		}
		if (flag)
		{
			minimum = 1000.0;
			for(V3DLONG j=0;j<result.size();j++)
			{
				if (i!=j)
				{
					temp = sqrt(double(((result[i]->x-result[j]->x)*(result[i]->x-result[j]->x)) + ((result[i]->y-result[j]->y)*(result[i]->y-result[j]->y)) + ((result[i]->z-result[j]->z)*(result[i]->z-result[j]->z))));
					if (temp<minimum)
					{
						minimum = temp;
						idx = j;
					}
				}
			}
			result[i]->parent = result[idx];
		}
	}
	
	return result;
}