/*
 *  NEURON_TRACING.cpp
 *
 *  Created by Yang, Jinzhu, on 12/15/10.
 *  Last update: by PHC, 20110826. Remove some of the absolute path
 *
 * This program uses Yinan Wan's neuron sorting code and Zongcai Ruan's radius estimation code
 * Last edit: 2012-01-25
 *
 */

#include "Neuron_tracing.h"
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_format_converter.h"

#include <deque>
#include <algorithm>
#include <functional>
#include<math.h>
#include <unistd.h>

#include "parser.h"
#include "stackutil.h"
#include "mg_utilities.h"
#include "mg_image_lib.h"
#include "NeuronSegmentation.h"
#include "NeuronEnhancementFilter.h"

#define BACKGROUND 0
#define DISOFENDS 0 
#define MASK -100 
#define VOID 1000000000

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
#ifndef __MERGE_NEURON_TRACING_ALGORITHMS__
Q_EXPORT_PLUGIN2(NEURON_TRACING,NeuronPlugin);
#endif


#ifndef __MERGE_NEURON_TRACING_ALGORITHMS__
//plugin funcs

const QString title = "SimpleTracing 0.92 (2012-03-19)";

QStringList NeuronPlugin::menulist() const
{
    return QStringList()
	<< tr("Tracing")
	<< tr("Segmentation")
	<< tr("Help")
	<< tr("-------------------")
	<< tr("Other method 1: ray_shooting")
	<< tr("Other method 2: dfs");
}
void NeuronPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Other method 2: dfs"))
	{
		NeuronTracing_Ray_D(callback, parent);
	}
	else if (menu_name == tr("Other method 1: ray_shooting"))
	{
		NeuronTracing_Ray_cast(callback, parent);
	}
	else if(menu_name == tr("Tracing"))
	{
		Tracing_DistanceField_entry_func(callback, parent);
	}
	else if(menu_name == tr("Segmentation"))
	{
		bool b_binarization = true;
		Neuron_segment_entry_func(callback,parent);
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg(title + " A Vaa3D plugin for tracing neurons using several local search methods. "
				   "Developed by Jinzhu Yang & Hanchuan Peng, 2011");
	}
}

QStringList NeuronPlugin::funclist() const
{
	return QStringList()
	<<tr("enhance")
	<<tr("segment")
	<<tr("tracing")
	<<tr("help")
	<<tr("ray_shooting")
	<<tr("dfs")
	;
}

bool NeuronPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (func_name==tr("tracing"))
	{
		Tracing_DistanceField_entry_func(input,output);
		return true;
	}
	else if (func_name==tr("segment"))
	{
		Neuron_segment_entry_func(input,output, true);
		return true;
	}
	else if (func_name==tr("enhance"))
	{
		Neuron_segment_entry_func(input,output, false);
		return true;
	}
	else if (func_name==tr("ray_shooting"))
	{
		Tracing_Ray_SWC(input,output);
		return true;
	}
	else if (func_name==tr("dfs"))
	{
		Tracing_Ball_SWC(input,output);
		return true;
	}
	else if (func_name==tr("help"))
	{
		cout<< qPrintable(title) <<endl<<endl;
		cout<<"-f enhance      : Enhance the line-like structures"<<endl;
		cout<<"-f segment      : Generates the segmentation mask"<<endl;
		cout<<"-f tracing      : SimpleTracing for producing a SWC neuron reconstruction"<<endl;
		cout<<"-f help         : Print this message"<<endl;
		cout<<"-f ray_shooting : Additional tracing method 1"<<endl;
		cout<<"-f dfs          : Additional tracing method 2"<<endl;
	    cout<<"demo:vaa3d -x <path_to_this_dll or the unique sub-string> -f <one of the above options> -i <input_image_file> -o <out_swc_file>"<<endl;
		return true;
	}
	return false;
}
#endif

template <class T1, class T2> bool assign_val(T1 *dst, T2 *src, V3DLONG total_num)
{
	if (!dst || !src || total_num<=0) return false;
	for (V3DLONG i=0; i<total_num; i++) 
		dst[i] = src[i];
	return true;
}

template <class T1, class T2> bool assign_val_condition(T1 *dst, T2 *src, V3DLONG total_num, T2 condition_val, T1 condition_yes_val, T1 condition_no_val)
{
	if (!dst || !src || total_num<=0) return false;
	for (V3DLONG i=0; i<total_num; i++) 
		dst[i] = (src[i]==condition_val) ? condition_yes_val : condition_no_val;
	return true;
}

template <class T1, class T2> bool do_seg(T1 * inputData,
                               T2 * outputData,
                               V3DLONG sx, V3DLONG sy, V3DLONG sz, 
                               int & iVesCnt, bool b_binarization=true)
{
    bool res=false;
    if (!inputData || !outputData || sx<=0 || sy<=0 || sz<=0)
        return res;
    
	short *internal_input = 0;
	V3DLONG channel_sz = sx*sy*sz;
    
	try
	{
		internal_input = new short[channel_sz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in do_seg().");
        res = false;
        goto Label_exit_do_seg;
	}
	
    assign_val(internal_input, inputData, channel_sz);
    
    if(!do_seg(internal_input, sx, sy, sz, iVesCnt, b_binarization))
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        res = false;
        goto Label_exit_do_seg;
    }	
	
	printf("Line/Vessle count = [%ld]\n", iVesCnt);
	
    if (b_binarization)
        assign_val_condition(outputData, internal_input, channel_sz, (short int)(-1), (T2)0, (T2)255);
    else
    {
        for (V3DLONG i=0;i<channel_sz;i++)
            outputData[i] = (T2)(internal_input[i]);
    }
    
    res = true; 
    
Label_exit_do_seg:
    if (internal_input) {delete []internal_input; internal_input=0;}
    return res;
}

bool do_seg(short *pData, V3DLONG sx, V3DLONG sy, V3DLONG sz, int & iVesCnt, bool b_binarization=true)
{
    if (!pData || sx<=0 || sy<=0 || sz<=0)
        return false;
    
	int iSigma[1] = {1};
	
	//vesselness
	float fA = 0.5;
	float fB = 0.5;
    float fC = 1.0*10*5;
	
	//volume filter
	bool bVolFilter = true;
    
	NeuronSeg m_CerebroSeg;
	m_CerebroSeg.Neuron_Segmentation(pData, iSigma, 
									  sy, sx, sz,
									  fA, fB, fC,  //parameters for the line-likelihood
									  1,NULL,
									  80, //ignore connected components that size smaller than this threshold 
                                      iVesCnt,//220 ,3 30,1.300.3//40.3／／20.3 50 .3//20 .3
									  b_binarization, 
									  bVolFilter);	
    return true;
}

void Neuron_segment_entry_func(const V3DPluginArgList & input, V3DPluginArgList & output, bool b_binarization)
{
    if (input.size()<=0 || output.size()<=0)
        return;
	char * infile = (*(vector<char*> *)(input.at(0).p)).at(0);
    char * outfile = (*(vector<char*> *)(output.at(0).p)).at(0);
    if (!infile || !outfile) 
        return;
    
	cout<<"infile : "<<infile<<endl;
    cout<<"outfile : "<<outfile<<endl;
	
	unsigned char * inimg1d = 0; V3DLONG * in_sz = 0; int datatype;
	loadImage(infile, inimg1d, in_sz, datatype);
    
	V3DLONG channel_sz = in_sz[0]*in_sz[1]*in_sz[2];
    V3DLONG outsz[4];
    
    //segmentation
    
    int iVesCnt = 1, i;
    bool res;
	unsigned char * m_OutImgData = 0;
    
	try
	{
		m_OutImgData = new  unsigned char[channel_sz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        goto Label_exit_Neuron_segment_entry_func;
	}
	
    switch (datatype)
    {
        case 1:
            res = do_seg(inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 2:
            res = do_seg((short int *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 4:
            res = do_seg((float *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        default:
            break;
    }
    
    if(!res)
    {
        cerr<<"The enhancement/segmentation of foreground fails."<<endl;
        goto Label_exit_Neuron_segment_entry_func;
    }	
	
	
    //save mask file
    for (i=0;i<3;i++) outsz[i]=in_sz[i]; outsz[3]=1;	
	saveImage(outfile, m_OutImgData, outsz, 1);
    
    //clear memory
Label_exit_Neuron_segment_entry_func:
    if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;} //fix this memory leak on 2012-01-28. by PHC
    if (inimg1d) {delete []inimg1d; inimg1d=0;}
    if (in_sz) {delete []in_sz; in_sz=0;} //do not forget
}

void Neuron_segment_entry_func(V3DPluginCallback &callback, QWidget *parent)///segment
{
	v3dhandle curwin = callback.currentImageWindow();
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
    
    unsigned char * inimg1d = subject->getRawData(); 
    V3DLONG in_sz[4]; 
    in_sz[0] = subject->getXDim(); in_sz[1] = subject->getYDim(); in_sz[2] = subject->getZDim(); in_sz[3] = subject->getCDim();
    int datatype = subject->getDatatype();
	
    V3DLONG channel_sz = in_sz[0]*in_sz[1]*in_sz[2];
    V3DLONG outsz[4];
    
    //segmentation
    
    int iVesCnt = 1, i;
    bool res;
	unsigned char * m_OutImgData = 0;
    
	try
	{
		m_OutImgData = new  unsigned char[channel_sz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
	}
	
	bool b_binarization = true;
    switch (datatype)
    {
        case 1:
            res = do_seg(inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 2:
            res = do_seg((short int *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 4:
            res = do_seg((float *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        default:
            break;
    }
    
    if(!res)
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
    }	
    
	////////////////////////////////////////
	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)m_OutImgData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);		
	callback.setImageName(newwin, QString("Neuron Seg"));
	callback.updateImageWindow(newwin);
	//Clear();
    
    return;    
}

void NeuronPlugin::NeuronTracing_Ray_cast(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	int start_t = clock(); // record time point
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}

    unsigned char * inimg1d = subject->getRawData();
    V3DLONG sx = subject->getXDim();
    V3DLONG sy = subject->getYDim();
    V3DLONG sz = subject->getZDim();
	V3DLONG pagesz_sub = sx*sy*sz;

	m_OiImgWidth = sx;
	m_OiImgHeight =sy;
	
    unsigned char * pData;
    try
    {
        pData = new  unsigned char[pagesz_sub];
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        return;
    }
    int iVesCnt = 1;
    bool b_binarization = true;
    do_seg(inimg1d, pData, sx, sy, sz, iVesCnt, b_binarization);

	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG x,y,z,index;
	
	V3DLONG imagesize = sy*sx;
    V3DLONG *apsInput=0;
	try
	{
        apsInput = new V3DLONG[sx*sy*sz];
        memset(apsInput, 0, sx*sy*sz * sizeof(V3DLONG));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (apsInput) {delete []apsInput; apsInput=0;}
		return;
	}
	SpacePoint_t orgPoint;
	orgPoint.m_x = -1;
	orgPoint.m_y = -1;
	orgPoint.m_z = -1;
	
    //?? what is the goal of this search?? a max-filter? But the code does not seem to be correct
	for (V3DLONG k=0; k<sz; k++)
	{
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				index = k * imagesize + j * sx + i;
				int max =0;
				for(int m = 0; m < 3; m++)
				{
					for(int n = 0; n < 9; n++)
					{
						z = k + nDz[m];
						y = j + nDy[n];
						x = i + nDx[n];
						if (z < sz && y < sy && x<sx && x>0 && y>0 && z>0)
						{
                            if (pData[ z * imagesize + y * sx + x] > max)
							{
								max = pData[ z * sx*sy + y * sx + x];
                                //m=3;
								//printf("max=%ld m=%ld\n",max,m);
							}
                        }
                        else
						{
							m = 3;
						}
					}
				}
				apsInput[index]=max;
			}
		}
	}

    if(pData) {delete []pData; pData = 0;}

    bool flag = 1;
	for (V3DLONG k=0; k<sz; k++)
	{
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if (apsInput[k*sx*sy+j*sx+i] == 255)
				{
					apsInput[k*sx*sy+j*sx+i] = 255;
                    if(flag == 1)
                    {
                        orgPoint.m_x = i + 1;
                        orgPoint.m_y = j + 1;
                        orgPoint.m_z = k + 1;
                        flag =0;
                    }


				}
				else 
				{
					apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
				}
			}
		}
	}

    SetImageInfo1D(apsInput,sz,sx,sy);

    printf("x=%ld y=%ld z=%ld\n",orgPoint.m_x,orgPoint.m_y,orgPoint.m_z);


//	LandmarkList  Landmark;
//	Landmark = callback.getLandmark(curwin);
//    V3DLONG m_x=0, m_y=0, m_z=0;
//    if (Landmark.size()>0)
//    {
//        m_x= Landmark.at(0).x;
//        m_y= Landmark.at(0).y;
//        m_z= Landmark.at(0).z;
//    }
//    printf("x=%ld y=%ld z=%ld\n",m_x,m_y,m_z);
//    orgPoint.m_x = m_x;
//	orgPoint.m_y = m_y;
//	orgPoint.m_z = m_z;

	Set_Seed(orgPoint);
	Initialize1D();
	Set_local_DFB();
	
    try
	{
		m_ppsMaskData = new unsigned char [m_iImgWidth * m_iImgHeight*m_iImgCount]; 
		
		memset(m_ppsMaskData, 0, m_iImgWidth * m_iImgHeight*m_iImgCount * sizeof(unsigned char ));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_ppsMaskData) {delete []m_ppsMaskData; m_ppsMaskData=0;}
		return;
	}
    /*unsigned char * m_OutImgData;
	try {
		m_OutImgData = new  unsigned char[m_iImgCount * m_iImgWidth * m_iImgHeight];
		
		memset(m_OutImgData, 0, m_ulVolumeSize * sizeof(unsigned char));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
		return;
    }*/

	/////first extract	
	vector<SpacePoint_t> centerpath;
	
	centerpath = ExtractCenterpath_Ray_Cast(orgPoint);
	
	centerpathall.push_back(centerpath);
	
	PathMask(centerpath);
	
    SpacePoint_t direc_point;
	
	SpacePoint_t swcpoint;
	
	int direc = 0;
	
	double r = 0;
		
	vector<SpacePoint_t> Branchpoint;
	
	Branchpoint = BranchDetect2(centerpath); 
	
	NeuronSWC v;
    listNeuron.clear();
	
	for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	{
		if(jj==0)
		{
			v.n	= jj+1;
			v.type = 2;
			v.x = centerpath.at(jj).m_x;
			v.y = centerpath.at(jj).m_y;
			v.z = centerpath.at(jj).m_z;
			v.r = centerpath.at(jj).r;
			v.pn  = jj-1;
			//v.seg_id = jj;
			//v.nodeinseg_id =jj;
			listNeuron.append(v);
		}
		else 
		{
			v.n	= jj+1;
			v.type = 2;
			v.x = centerpath.at(jj).m_x;
			v.y = centerpath.at(jj).m_y;
			v.z = centerpath.at(jj).m_z;
			v.r = centerpath.at(jj).r;
			v.pn  = jj;
			//	v.seg_id = jj;
			//	v.nodeinseg_id =jj;
			listNeuron.append(v);
		}
	}
	
	//centerpathall.push_back(centerpath);
	
	count=0;
	V3DLONG num = 0;
	SpacePoint_t tempPoint;
	
	for(V3DLONG jj = 0; jj < Branchpoint.size(); jj++)
	{

		NueronTree2(Branchpoint.at(jj),Branchpoint.at(jj).node_id);
		//NueronTree2(tempPoint,jj);
		
	}
	
	SS.on = true;
	SS.listNeuron = listNeuron;
	SS.name = "neu_tracing_R";
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                        QFileInfo(callback.getImageName(curwin)).fileName() + "_R.swc",
                                                            QObject::tr("Supported file (*.swc)"
                                                                        ";;Neuron structure	(*.swc)"
                                                                        ));

    writeSWC_file(fileSaveName,SS);

	/////////
    /*Image4DSimple p4DImage;
	p4DImage.setData(m_OutImgData, m_iImgWidth, m_iImgHeight, m_iImgCount-2, 1, V3D_UINT8);
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);		
	callback.setImageName(newwin, QString("Neuron tracing_R"));
	callback.updateImageWindow(newwin);
    Clear();*/
    return;
}
void NeuronPlugin::NeuronTracing_Ray_D(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	
	int start_t = clock(); // record time point
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
    unsigned char * inimg1d = subject->getRawData();
    V3DLONG sx = subject->getXDim();
	V3DLONG sy = subject->getYDim();
	V3DLONG sz = subject->getZDim();
	V3DLONG pagesz_sub = sx*sy*sz;
	m_OiImgWidth = sx;
	m_OiImgHeight =sy;
	
    unsigned char * pData;
    try
    {
        pData = new  unsigned char[pagesz_sub];
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        return;
    }
    int iVesCnt = 1;
    bool b_binarization = true;
    do_seg(inimg1d, pData, sx, sy, sz, iVesCnt, b_binarization);


	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG x,y,z,index;
	
	V3DLONG imagesize = sy*sx;
	
    V3DLONG *apsInput;
	try
	{
        apsInput = new V3DLONG[sx*sy*sz];
        memset(apsInput, 0, sx*sy*sz * sizeof(V3DLONG));
		
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (apsInput) {delete []apsInput; apsInput=0;}
		return;
	}
	SpacePoint_t orgPoint;
	orgPoint.m_x = -1;
	orgPoint.m_y = -1;
	orgPoint.m_z = -1;	
	
	for (V3DLONG k=0; k<sz; k++)
	{
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				index = k * imagesize + j * sx + i;
				int max =0;
				for(int m = 0; m < 3; m++)
				{
					for(int n = 0; n < 9; n++)
					{
						z = k + nDz[m];
						y = j + nDy[n];
						x = i + nDx[n];
						if (z < sz && y < sy && x<sx && x>0 && y>0 && z>0)
						{
							if(pData[ z * imagesize + y * sx + x] > max)
							{
								max = pData[ z * sx*sy + y * sx + x];
								m=3;
								//printf("max=%ld m=%ld\n",max,m);
							}
						}else
						{
							m = 3;
							
						}
					}
				}
				apsInput[index]=max;
			}
		}
    }

    if(pData) {delete []pData; pData = 0;}

    bool flag = 1;
    V3DLONG m_x,m_y,m_z;

	for (V3DLONG k=0; k<sz; k++)
	{
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if (apsInput[k*sx*sy+j*sx+i] == 255)
				{
                    apsInput[k*sx*sy+j*sx+i] = 255;
                    if(flag == 1)
                    {
                        m_x = i + 1;
                        m_y = j + 1;
                        m_z = k + 1;
                        flag =0;
                    }


					
				}
				else 
				{
					apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
				}
			}
		}
    }


    SetImageInfo1D(apsInput,sz,sx,sy); //temp modify
 /*   LandmarkList  Landmark;
    Landmark = callback.getLandmark(curwin);
    if (Landmark.size()<1)
    {
        v3d_msg("Landmark has not been set in the image. Quit.");
        return;
    }

    V3DLONG m_x= Landmark.at(0).x;
    V3DLONG m_y= Landmark.at(0).y;
    V3DLONG m_z= Landmark.at(0).z;*/
	
    ImageMarker S;
    S.x = m_x;
    S.y = m_y;
    S.z = m_z;
    S.on = true;
    listMarker.append(S);
	
    if(apsInput[m_z*sx*sy+m_y*sx+m_x] == BACKGROUND)
    {
        return;
    }
    printf("x=%ld y=%ld z=%ld\n",m_x,m_y,m_z);
	
    orgPoint.m_x = m_x;
    orgPoint.m_y = m_y;
    orgPoint.m_z = m_z;
	
	Initialize1D();
	
	Set_Seed(orgPoint);
	
	//Initialize1D();
	
	Set_local_DFB();
	
	Set_local_DFS();
	
	try
	{
		m_ppsMaskData = new unsigned char [m_iImgCount * m_iImgWidth * m_iImgHeight]; 
		
		memset(m_ppsMaskData, 0, m_iImgCount * m_iImgWidth * m_iImgHeight * sizeof(unsigned char ));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_ppsMaskData) {delete []m_ppsMaskData; m_ppsMaskData=0;}
		return;
	}
    /*unsigned char * m_OutImgData;
	try {
		m_OutImgData = new  unsigned char[m_iImgCount * m_iImgWidth * m_iImgHeight];
		
		memset(m_OutImgData, 0, m_iImgCount * m_iImgWidth * m_iImgHeight * sizeof(unsigned char));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
		return;
    }*/
	try
	{
		m_piMskDFS = new V3DLONG[m_ulVolumeSize]; 
		memset(m_piMskDFS, 0, m_ulVolumeSize * sizeof(V3DLONG));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piMskDFS) {delete []m_piMskDFS; m_piMskDFS=0;}
		return;
	}
	/////first extract	
	vector<SpacePoint_t> centerpath;
	
	vector<SpacePoint_t> Rcenterpath;
	
	centerpath.clear();
	
	Rcenterpath.clear();
	
	centerpath = ExtractCenterpath_Ray_D(orgPoint,orgPoint);
	
	//PathMask(Rcenterpath);
	
	//centerpath = Rivise_centerpath(Rcenterpath);
	
	SpacePoint_t direc_point ;
	
	SpacePoint_t swcpoint;
	
	int direc = 0;
	
	double r = 0;
	
	QString file;
	
	
//	for(V3DLONG j = 0; j < centerpath.size(); j++)
//	{
//		x = centerpath.at(j).m_x;
//		y = centerpath.at(j).m_y;
//		z = centerpath.at(j).m_z;
//		m_OutImgData[z* m_iImgSize + y * m_iImgWidth + x]=255;
//		printf("rrrr=%lf rayyy=%ld\n",centerpath.at(j).r,centerpath.at(j).max_ray);
//	}
//	QString curImageName = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/ray cast and distance/tracing1.swc";
//	
//	SaveSwcfile(centerpath,curImageName);
	
	centerpathall.push_back(centerpath);
	
	PathMask(centerpath);
	
	NeuronSWC v;
    listNeuron.clear();

	for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	{
		v.n	= jj+1;
		v.type = 2;
		v.x = centerpath.at(jj).m_x;
		v.y = centerpath.at(jj).m_y;
		v.z = centerpath.at(jj).m_z;
		v.r = centerpath.at(jj).r;
		if(jj==0)
		{
			v.pn  = jj-1;
		}else 
		{
			v.pn  = jj;
		}
		listNeuron.append(v);
	}	
	
	vector<SpacePoint_t> Branchpoint;
	
	Branchpoint = BranchDetect2(centerpath);
	
	count=0;
	
	SpacePoint_t tempPoint;
	
	for(V3DLONG jj = 0; jj <  Branchpoint.size(); jj++)
	{
	     x = Branchpoint.at(jj).m_x;
         y = Branchpoint.at(jj).m_y;
         z = Branchpoint.at(jj).m_z;
    //	m_OutImgData[z* m_iImgSize + y * m_iImgWidth + x]=255;
		m_Markerpoint.push_back(Branchpoint.at(jj));
		NueronTree_Ray_D(Branchpoint.at(jj),(Branchpoint.at(jj).node_id));
	}
	
	SS.on = true;
	SS.listNeuron = listNeuron;
	SS.name = "neu_tracing_D";
  //  QString filename = "neu_tracing_D.swc";
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                        QFileInfo(callback.getImageName(curwin)).fileName() + "_D.swc",
                                                            QObject::tr("Supported file (*.swc)"
                                                                        ";;Neuron structure	(*.swc)"
                                                                        ));
    writeSWC_file(filename,SS);
	/////////////////////////////////////
//    centerpathall.push_back(centerpath);
//    for(V3DLONG jj = 0; jj< centerpathall.size(); jj++)
//    {
//        //		file.sprintf("%d",jj);
//        //		QString filepath = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/ray cast and distance/";
//        //		QString curImageName1 = filepath + file + ".swc";
//        //		SaveSwcfile(centerpathall.at(jj),curImageName1);
//        for(V3DLONG ii = 0; ii<centerpathall.at(jj).size(); ii++)
//        {
//            x = centerpathall.at(jj).at(ii).m_x;
//            y= centerpathall.at(jj).at(ii).m_y;
//            z = centerpathall.at(jj).at(ii).m_z;
//            m_OutImgData[z* m_iImgSize + y * m_iImgWidth + x]=255;
//            //printf("r=%lf ray=%ld\n",centerpathall.at(jj).at(ii).r,centerpathall.at(jj).at(ii).max_ray);
//        }
//    }
//    QHash <int, int>  hashNeuron;
//    //listNeuron.clear();
//    hashNeuron.clear();
//    V3DLONG tt =0;
//    for(V3DLONG i = 0; i < centerpathall.size(); i++)
//    {
		
//        int size = centerpathall.at(i).size();
//        //printf("siz=%ld\n",size);
//        for(V3DLONG j = 0; j < size; j++)
//        {
//            tt++;
//            x = centerpathall.at(i).at(j).m_x;
//            y = centerpathall.at(i).at(j).m_y;
//            z = centerpathall.at(i).at(j).m_z;
//            double r = 0;
//            r = centerpathall.at(i).at(j).r;
//            //swcpath.push_back(tempoint);
//            if(j==0)
//            {
//                v.n	= tt;
//                v.type = 2;
//                v.x = x;
//                v.y = y;
//                v.z = z;
//                v.r = r;
//                v.pn  = -1;
//                //v.seg_id = centerpath.at(jj).seg_id;
//                //v.nodeinseg_id =jj;
//                listNeuron.append(v);
//                hashNeuron.insert(v.n, listNeuron.size()-1);
				
//            }else
//            {
//                v.n	= tt;
//                v.type = 2;
//                v.x = x;
//                v.y = y;
//                v.z = z;
//                v.r = r;
//                v.pn  = tt-1;
//                //v.seg_id = centerpath.at(jj).seg_id;
//                //v.nodeinseg_id =jj;
//                listNeuron.append(v);
//                hashNeuron.insert(v.n, listNeuron.size()-1);
//            }
//        }
//    }
	//	QString filename;
	//	SS.on = true;
	//	SS.listNeuron = listNeuron;
	//	SS.name = "neu_tracing";
	//	filename = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/ray cast and distance/neu_tracing1.swc";
	//writeSWC_file(filename,SS);	
	
	//	QList<NeuronSWC> lN;
	//	NeuronTree       NS;
	//	int rootid;
	//	rootid = VOID;
	//	if (SortSWC(SS,lN,rootid))
	//	{
	//		printf("sortswcccccccccccccccccccc\n");
	//		NS.on = true;
	//		NS.listNeuron = lN;
	//		NS.name = "neu_tracing_R_D";
	//		filename = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/ray cast and distance/tracing.swc";
	//		writeSWC_file(filename,NS);
	//	}
	
	
	//	for (V3DLONG k=0; k<m_iImgCount; k++)
	//	{
	//		for (V3DLONG j=0; j<m_iImgHeight; j++)
	//		{
	//			for (V3DLONG i=0; i<m_iImgWidth; i++)
	//			{
	//				m_ppsMaskData[k*m_iImgWidth*m_iImgHeight + j*m_iImgWidth + i]=0;
	//				
	//			}
	//		}
	//	}
	//	
	//
//    QString markfile = "markfile.marker";
//    writeMarker_file(markfile, listMarker);
//    /////////
//    Image4DSimple p4DImage;
//    p4DImage.setData(m_ppsMaskData, m_iImgWidth, m_iImgHeight, m_iImgCount-2, 1, V3D_UINT8);
//    v3dhandle newwin;
//    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
//        newwin = callback.currentImageWindow();
//    else
//        newwin = callback.newImageWindow();
//    callback.setImage(newwin, &p4DImage);
//    callback.setImageName(newwin, QString("Neuron tracing_RD"));
//    callback.updateImageWindow(newwin);
	//Clear();
    return;
}



void NeuronPlugin::Dilation3D(unsigned char *apsInput, unsigned char *aspOutput, V3DLONG sz, V3DLONG sy, V3DLONG sx)
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG i,j,k,x,y,z,index;
	V3DLONG imagesize = sy*sx;
	bool end = true;
	printf("x=%ld y=%ld z=%ld\n",sx,sy,sz);
	
	for (V3DLONG k=0; k<sz; k++)
	{
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				end = false;
				index = k * imagesize + j * sx + i;
				int max =0;
				for(int m = 0; m < 3; m++)
				{
					for(int n = 0; n < 9; n++)
					{
						z = k + nDz[m];
						y = j + nDy[n];
						x = i + nDx[n];
						if (z < sz && y < sy && x<sx && x>0 && y>0 && z>0)
						{
							if(apsInput[ z * imagesize + y * sx + x] > max)
							{
								max = apsInput[ z * sx*sy + y * sx + x];
								m=3;
								//printf("max=%ld m=%ld\n",max,m);
							}
						}else
						{
							m = 3;
							
						}
					}
				}
				aspOutput[index]=max;
			}
		}
	}
	
}


bool NeuronPlugin::Tracing_Ray_SWC(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to Ray_shooting Tracing"<<endl;
	//if(input.size() != 2 || output.size() != 1) return false;
//	char * paras = 0;
//	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
//	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	
//	for(int i = 0; i < strlen(paras); i++)
//	{
//		if(paras[i] == '#') paras[i] = '-';
//	}
//	string optstrings[] = {"-inimg:","-inmarker:", "-outswc:"};
//	ParaParser parser(string(paras), optstrings, sizeof(optstrings)/sizeof(string));
//	string inimgfile = parser.get_optarg("-inimg");
//	string inmarkerfile = parser.get_optarg("-inmarker");
//	string outswcfile = parser.get_optarg("-outswc");
//	cout<<"parser : -inimg "<<inimgfile<<" -inmarker "<<inmarkerfile<<" -outswc "<<outswcfile<<endl;
//	if(parser.error()) {parser.print_error(); return false;}

    char * inimgfile = (*(vector<char*> *)(input.at(0).p)).at(0);
    char * outswcfile = (*(vector<char*> *)(output.at(0).p)).at(0);

	unsigned char * inimg1d = 0;
	V3DLONG * sz = 0;
	int datatype;
    if(!loadImage(inimgfile, inimg1d, sz, datatype)){cerr<<"unable to load image"<<endl; return false;}
    NeuronTree nt= NeuronTracing_Ray_SWC(inimg1d, sz[0], sz[1], sz[2]);
    if(!writeSWC_file(outswcfile, nt)){cerr<<"unable to write swc"<<endl; return false;}
	return true;
}

bool NeuronPlugin::Tracing_Ball_SWC(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Neurontracing_jinzhu1"<<endl;
	//if(input.size() != 2 || output.size() != 1) return false;
//	char * paras = 0;
//	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
//	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	
//	for(int i = 0; i < strlen(paras); i++)
//	{
//		if(paras[i] == '#') paras[i] = '-';
//	}
//	string optstrings[] = {"-inimg:","-inmarker:", "-outswc:"};
//	ParaParser parser(string(paras), optstrings, sizeof(optstrings)/sizeof(string));
//	string inimgfile = parser.get_optarg("-inimg");
//	string inmarkerfile = parser.get_optarg("-inmarker");
//	string outswcfile = parser.get_optarg("-outswc");
//	cout<<"parser : -inimg "<<inimgfile<<" -inmarker "<<inmarkerfile<<" -outswc "<<outswcfile<<endl;
//	if(parser.error()) {parser.print_error(); return false;}

    char * inimgfile = (*(vector<char*> *)(input.at(0).p)).at(0);
    char * outswcfile = (*(vector<char*> *)(output.at(0).p)).at(0);

    unsigned char * inimg1d = 0;
    V3DLONG * sz = 0;
    int datatype;
    if(!loadImage(inimgfile, inimg1d, sz, datatype)){cerr<<"unable to load image"<<endl; return false;}
    NeuronTree nt= NeuronTracing_Rollerball_SWC(inimg1d, sz[0], sz[1], sz[2]);
    if(!writeSWC_file(outswcfile, nt)){cerr<<"unable to write swc"<<endl; return false;}
	return true;
}

bool NeuronPlugin::Neuron_Seg(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Neurontracing_jinzhu1"<<endl;
	//if(input.size() != 2 || output.size() != 1) return false;
	char * paras = 0;
	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	
	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	string optstrings[] = {"-inimg:","-inmarker:", "-outswc:"};
	ParaParser parser(string(paras), optstrings, sizeof(optstrings)/sizeof(string));
	string inimgfile = parser.get_optarg("-inimg"); 
	string inmarkerfile = parser.get_optarg("-inmarker");
	string outswcfile = parser.get_optarg("-outswc");
	cout<<"parser : -inimg "<<inimgfile<<" -inmarker "<<inmarkerfile<<" -outswc "<<outswcfile<<endl;
	if(parser.error()) {parser.print_error(); return false;}
	unsigned char * inimg1d = 0;
	V3DLONG * sz = 0;
	int datatype;
	if(!loadImage((char*)inimgfile.c_str(), inimg1d, sz, datatype)){cerr<<"unable to load image"<<endl; return false;}
	QList<ImageMarker> markerlist = readMarker_file(QString(inmarkerfile.c_str()));
	ImageMarker first_marker = markerlist.at(0);
    NeuronTree nt= NeuronTracing_Rollerball_SWC(inimg1d, sz[0], sz[1], sz[2]);
	if(!writeSWC_file(QString(outswcfile.c_str()), nt)){cerr<<"unable to write swc"<<endl; return false;}
	return true;
}

void NeuronPlugin::SetImageInfo1D(V3DLONG* data, V3DLONG count, V3DLONG width, V3DLONG height)
{
	m_ppsOriData1D = data;
	
	m_iImgCount = count;
	
	m_iImgWidth = width;
	
	m_iImgHeight = height;
	
	printf("count=%d w=%d h=%d\n",m_iImgCount,m_iImgWidth,m_iImgHeight);
}

void NeuronPlugin::Set_Seed(SpacePoint_t seed)
{
	m_sptSeed = seed;	
}


bool NeuronPlugin::Tracing_DistanceField_entry_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Now invoke SimpeTracing..."<<endl;
	
    char * inimgfile = (*(vector<char*> *)(input.at(0).p)).at(0);
    char * outswcfile = (*(vector<char*> *)(output.at(0).p)).at(0);
    //char * paras = (*(vector<char*> *)(input.at(1).p)).at(0); //here need to add the channel info later, noted by PHC, 2012-01-25
	
	unsigned char * inimg1d = 0;
	
	V3DLONG * sz = 0;
	int datatype;
	
	V3DLONG ch_tracing=0; 	//here need to add the channel info later, noted by PHC, 2012-01-25
	
	if(!loadImage(inimgfile, inimg1d, sz, datatype))
	{
		cerr<<"unable to load image ["<< inimgfile << "]"<<endl;
		return false;
	}
	
	NeuronTree nt;
	switch (datatype) {
		case 1:
			nt = NeuronTracing_Distance_SWC((unsigned char *)inimg1d, sz[0], sz[1], sz[2], sz[3], ch_tracing, 0,0,0); 
			break;
		case 2:
			nt = NeuronTracing_Distance_SWC((unsigned short int *)inimg1d, sz[0], sz[1], sz[2], sz[3], ch_tracing, 0,0,0); 
			break;
		case 4:
			nt = NeuronTracing_Distance_SWC((float *)inimg1d, sz[0], sz[1], sz[2], sz[3], ch_tracing, 0,0,0); 
			break;
		default:
			cerr<<"Unsupported data type detected. Do nothing."<<endl;
			return false;
	} 
	
	if (sz) {delete []sz; sz=0;} //not delete sz[] maybe a bug of many Vaa3D code, by PHC, 2012-02-06
    nt.name = "Simple_Tracing";
	if (!writeSWC_file(QString(outswcfile), nt))
	{
		cerr<<"unable to write swc"<<endl; 
		return false;
	}
    //Clear();
	return true;
}

void NeuronPlugin::Tracing_DistanceField_entry_func(V3DPluginCallback &callback, QWidget *parent) 
{
	v3dhandle curwin = callback.currentImageWindow();
	Image4DSimple* subject = callback.getImage(curwin);
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	V3DLONG ch_tracing = 0;
	NeuronTree  RS;
	switch (subject->getDatatype())
	{
		case 1:
			RS = NeuronTracing_Distance_SWC(subject->getRawData(), 
												subject->getXDim(), subject->getYDim(), subject->getZDim(), 
												subject->getCDim(),  
												ch_tracing, 0, 0, 0);
			break;
		case 2:
			RS = NeuronTracing_Distance_SWC((unsigned short int *)(subject->getRawData()), 
											subject->getXDim(), subject->getYDim(), subject->getZDim(), 
											subject->getCDim(),  
											ch_tracing, 0, 0, 0);
			break;
		case 4:
			RS = NeuronTracing_Distance_SWC((float *)(subject->getRawData()), 
											subject->getXDim(), subject->getYDim(), subject->getZDim(), 
											subject->getCDim(),  
											ch_tracing, 0, 0, 0);
			break;
		default:
			cerr<<"Unsupported data type detected. Do nothing."<<endl;
			return;
	} 
	
	
	// now save SWC
	QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
														QFileInfo(callback.getImageName(curwin)).fileName() + "_ST.swc",
															QObject::tr("Supported file (*.swc)"
																		";;Neuron structure	(*.swc)"
																		));
	writeSWC_file(fileSaveName,RS);
}



template <class T> NeuronTree NeuronPlugin::NeuronTracing_Distance_SWC(T *apsInput, 
                                                                       V3DLONG sx, V3DLONG sy, V3DLONG sz, V3DLONG sc, 
                                                                       V3DLONG ch_tracing, V3DLONG mx ,V3DLONG my, V3DLONG mz)
{
	NeuronTree  RS;

	if (!apsInput || sx<=0 || sy<=0 || sz<=0 || sc<=0 || ch_tracing<0 || ch_tracing>=sc || mx<0 || my<0 || mz<0 || mx>=sx || my>=sy || mz>=sz)
	{
		cerr << "The parameters to the SimpleTracing() function are invalid." <<endl;
		return RS;
	}
	
	V3DLONG channel_sz = sx*sy*sz;
	
	//allocate memory 
	
	short *pData = 0;
	unsigned char * m_OutImgData = 0;
	V3DLONG *aspOutput = 0;

	try
	{
		pData = new short[channel_sz];
		memset(pData, 0, channel_sz * sizeof(short));

		m_OutImgData = new unsigned char[channel_sz];
		memset(m_OutImgData, 0, channel_sz * sizeof(unsigned char));

		aspOutput = new V3DLONG [channel_sz];
		memset(aspOutput, 0, channel_sz * sizeof(V3DLONG));
		
		m_ppsMaskData = new unsigned char [(sz+2) * sy * sx]; 
		memset(m_ppsMaskData, 0, (sz+2) * sy * sx * sizeof(unsigned char ));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in NeuronTracing_Distance_SWC().");
		if (pData) {delete []pData; pData=0;}
		if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
		if (aspOutput) {delete []aspOutput; aspOutput=0;}
		if (m_ppsMaskData) {delete []m_ppsMaskData; m_ppsMaskData=0;}
		return RS;
	}
	
	//force data conversion
	
	V3DLONG i,j,k,z;
	assign_val(pData, apsInput, channel_sz);
	
	//auto segment 
    
    int iVesCnt = 1;
    if(!do_seg(pData, sx, sy, sz, iVesCnt))
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        return RS;
    }
	
	printf("ivesCnt = %ld\n",iVesCnt);

	assign_val_condition(m_OutImgData, pData, channel_sz, short(-1), (unsigned char)0, (unsigned char)255);

	//Dilation3D(m_OutImgData,m_DmgData,sz, sy, sx);
	
	assign_val_condition(aspOutput, m_OutImgData, channel_sz, (unsigned char)(255), (V3DLONG)(255), (V3DLONG)(BACKGROUND));
	
	//Dilation3D(m_OutImgData,aspOutput,sz, sy, sx);
	
	//v3d_msg("segmatation end.");
	
	printf("seg end \n");
	
	SpacePoint_t *orgPoint = new SpacePoint_t[iVesCnt];
	
	QString filename;
	
	V3DLONG num=0;
	V3DLONG blog = 0;
	V3DLONG tt=0;
    listNeuron.clear();

	
	for (i=0; i<iVesCnt; i++)
	{
		orgPoint[i].m_x = orgPoint[i].m_y = orgPoint[i].m_z = -1;
		GetOrgPoint(pData, sz, sy, sx, i, orgPoint[i]);
		if (orgPoint[i].m_x == -1) 
			continue;
		
		printf("ivescnt=%ld orgx=%ld orgy=%ld org=%ld ",i,orgPoint[i].m_x,orgPoint[i].m_y,orgPoint[i].m_z);
		
		SetImageInfo1D(aspOutput,sz,sx,sy);
		
		Set_Seed(orgPoint[i]);
		
		Initialize1D();
		
		Set_local_DFB();
		
		Set_local_DFS();
		
		if(!m_vdfsptEndPoint.size())
			SearchEndPoints();
		
		int num = m_vdfsptEndPoint.size();
		
		m_vvsptCenterpaths.clear();
		
		for(int j = 0; j < num; j++)
		{			
			ExtractSingleCenterpath(m_vdfsptEndPoint.at(j));
		}
		
		vector<SpacePoint_t> branch;
		vector<SpacePoint_t> swcpath;
		SpacePoint_t tempoint;
		
		int size = 0;
		
		size = m_vvsptCenterpaths.size();
		
		printf("sizcount=%ld ",size);
		
		V3DLONG x,y,z;
		NeuronSWC v;
		QHash <int, int>  hashNeuron;
		hashNeuron.clear();
		
		blog++;
		//printf("blog=%ld\n",blog);
		
		if(blog==1)
		{
			for(V3DLONG ii = 0; ii < m_vvsptCenterpaths.size(); ii++)
			{
				branch = m_vvsptCenterpaths.at(ii);
				
				size = branch.size();
				printf("siz=%ld\n",size);
				if (size==0) 
				{
					continue;
				}
				for(V3DLONG jj = 0; jj < size; jj++)
				{
					tt++;
					num++;
					x = branch.at(jj).m_x;
					y = branch.at(jj).m_y;
					z = branch.at(jj).m_z;
					double r = 0;
					r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
										 x, y, z, 1, false);
					v.n	= tt;
					v.type = 2;
					v.x = x;
					v.y = y;
					v.z = z;
					v.r = r;
					v.pn  = (jj==0) ? -1 : tt-1;
					
					//v.seg_id = centerpath.at(jj).seg_id;
					//v.nodeinseg_id =jj;
					listNeuron.append(v);
					hashNeuron.insert(v.n, listNeuron.size()-1);
				}
			}
		}
		else 
		{
			for(V3DLONG ii = 0; ii < m_vvsptCenterpaths.size(); ii++)
			{
				//tt = num;
				
				printf("tt=%ld\n",tt);
				
				branch = m_vvsptCenterpaths.at(ii);
				size = branch.size();
				printf("siz=%ld\n",size);
				if (size==0) 
				{
					continue;
				}
				for(V3DLONG jj = 0; jj < size; jj++)
				{
					tt++;
					num++;
					x = branch.at(jj).m_x;
					y = branch.at(jj).m_y;
					z = branch.at(jj).m_z;
					double r = 0;
					r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
										 x, y, z, 1, false);
					v.n	= tt;
					v.type = 2;
					v.x = x;
					v.y = y;
					v.z = z;
					v.r = r;
					v.pn  = (jj==0) ? -1 : tt-1;
					//v.seg_id = centerpath.at(jj).seg_id;
					//v.nodeinseg_id =jj;
					listNeuron.append(v);
					hashNeuron.insert(v.n, listNeuron.size()-1);
				}
			}
		}
		Clear();
	}
	
	//fuse and sort individual neuron pieces
	
	SS.on = true;
	SS.listNeuron = listNeuron;
	SS.name = "ST_neuron";
	
	double thres = 10; //10; //20;//20,1000 //change to 1000 for testing? Not working. by PHC, 2012-01-25
	QList<NeuronSWC> lN;
	NeuronTree       NS;
	int rootid;
	rootid = VOID;
	if (SortSWC(SS.listNeuron,lN,rootid,thres))
	{
		printf("sortswc\n");
		NS.on = true;
		NS.listNeuron = lN;
		NS.name = "ST_neuron_sorted";
	}

	// save the reconstruction to a file
	Output_SWC(NS.listNeuron,RS);
	
	//clean memory
Label_exit:
	if (orgPoint) { delete []orgPoint; orgPoint=0;}
	if (m_ppsMaskData) {	delete []m_ppsMaskData;m_ppsMaskData = NULL;}
	if (pData) {delete []pData; pData=0;}
	if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
	if (aspOutput) {delete []aspOutput; aspOutput=0;}

	return RS;
}

void NeuronPlugin::GetOrgPoint(short *apsInput, V3DLONG iImgLayer, V3DLONG iImgHei, V3DLONG iImgWid, V3DLONG cnt, SpacePoint_t &pt)
{
	V3DLONG i, j, k, n;
	
	n=0;
	for (k = 0; k < iImgLayer; k++)
	{
		for (j = 0; j < iImgHei; j++)
		{
			for (i = 0; i < iImgWid; i++, n++)
			{
				if(apsInput[n]==cnt)
				{
					pt.m_x = i;
					pt.m_y = j;
					pt.m_z = k;
					
					//printf("i=%ld j=%ld k=%ld\n",i,j,k);
					return ;
				}
			}
		}
	}
}

NeuronTree NeuronPlugin::NeuronTracing_Ray_SWC(unsigned char *inimg1d, V3DLONG sx, V3DLONG sy, V3DLONG sz)
{

    SS.listNeuron.clear();
    V3DLONG pagesz_sub = sx*sy*sz;

    m_OiImgWidth = sx;
    m_OiImgHeight =sy;

    unsigned char * pData;
    try
    {
        pData = new  unsigned char[pagesz_sub];
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        return SS;
    }
    int iVesCnt = 1;
    bool b_binarization = true;
    do_seg(inimg1d, pData, sx, sy, sz, iVesCnt, b_binarization);

    static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
    static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
    static int nDz[] = {-1,0,1};

    V3DLONG x,y,z,index;

    V3DLONG imagesize = sy*sx;
    V3DLONG *apsInput=0;
    try
    {
        apsInput = new V3DLONG[sx*sy*sz];
        memset(apsInput, 0, sx*sy*sz * sizeof(V3DLONG));
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Distance Transform.");
        if (apsInput) {delete []apsInput; apsInput=0;}
        return SS;
    }
    SpacePoint_t orgPoint;
    orgPoint.m_x = -1;
    orgPoint.m_y = -1;
    orgPoint.m_z = -1;
    for (V3DLONG k=0; k<sz; k++)
    {
        for (V3DLONG j=0; j<sy; j++)
        {
            for (V3DLONG i=0; i<sx; i++)
            {
                index = k * imagesize + j * sx + i;
                int max =0;
                for(int m = 0; m < 3; m++)
                {
                    for(int n = 0; n < 9; n++)
                    {
                        z = k + nDz[m];
                        y = j + nDy[n];
                        x = i + nDx[n];
                        if (z < sz && y < sy && x<sx && x>0 && y>0 && z>0)
                        {
                            if (pData[ z * imagesize + y * sx + x] > max)
                            {
                                max = pData[ z * sx*sy + y * sx + x];
                                //m=3;
                                //printf("max=%ld m=%ld\n",max,m);
                            }
                        }
                        else
                        {
                            m = 3;
                        }
                    }
                }
                apsInput[index]=max;
            }
        }
    }

    if(pData) {delete []pData; pData = 0;}

    bool flag = 1;
    for (V3DLONG k=0; k<sz; k++)
    {
        for (V3DLONG j=0; j<sy; j++)
        {
            for (V3DLONG i=0; i<sx; i++)
            {
                if (apsInput[k*sx*sy+j*sx+i] == 255)
                {
                    apsInput[k*sx*sy+j*sx+i] = 255;
                    if(flag == 1)
                    {
                        orgPoint.m_x = i + 1;
                        orgPoint.m_y = j + 1;
                        orgPoint.m_z = k + 1;
                        flag =0;
                    }


                }
                else
                {
                    apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
                }
            }
        }
    }

    SetImageInfo1D(apsInput,sz,sx,sy);

    printf("x=%ld y=%ld z=%ld\n",orgPoint.m_x,orgPoint.m_y,orgPoint.m_z);

	Set_Seed(orgPoint);
	Initialize1D();
	Set_local_DFB();
	
	try
	{
		m_ppsMaskData = new unsigned char [m_iImgWidth * m_iImgHeight*m_iImgCount]; 
		
		memset(m_ppsMaskData, 0, m_iImgWidth * m_iImgHeight*m_iImgCount * sizeof(unsigned char ));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_ppsMaskData) {delete []m_ppsMaskData; m_ppsMaskData=0;}
		//return;
	}
	/////first extract	
	vector<SpacePoint_t> centerpath;
	
	centerpath = ExtractCenterpath_Ray_Cast(orgPoint);
	
	centerpathall.push_back(centerpath);
	
	PathMask(centerpath);
	
	SpacePoint_t direc_point ;
	
	SpacePoint_t swcpoint;
	
	int direc = 0;
	
	double r = 0;
	
	vector<SpacePoint_t> Branchpoint;
	
	Branchpoint = BranchDetect2(centerpath); 
	
	NeuronSWC v;
    listNeuron.clear();

	
	for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	{
		if(jj==0)
		{
			v.n	= jj+1;
			v.type = 2;
			v.x = centerpath.at(jj).m_x;
			v.y = centerpath.at(jj).m_y;
			v.z = centerpath.at(jj).m_z;
			v.r = centerpath.at(jj).r;
			v.pn  = jj-1;
			//v.seg_id = jj;
			//v.nodeinseg_id =jj;
			listNeuron.append(v);
		}
		else 
		{
			v.n	= jj+1;
			v.type = 2;
			v.x = centerpath.at(jj).m_x;
			v.y = centerpath.at(jj).m_y;
			v.z = centerpath.at(jj).m_z;
			v.r = centerpath.at(jj).r;
			v.pn  = jj;
			//	v.seg_id = jj;
			//	v.nodeinseg_id =jj;
			listNeuron.append(v);
		}
	}
	
	//centerpathall.push_back(centerpath);
	
	count=0;
	V3DLONG num = 0;
	SpacePoint_t tempPoint;
	
	for(V3DLONG jj = 0; jj < Branchpoint.size(); jj++)
	{
		NueronTree2(Branchpoint.at(jj),Branchpoint.at(jj).node_id);
		
	}
	SS.on = true;
	SS.listNeuron = listNeuron;
    SS.name = "Ray_shooting Tracing";
	return SS;
		
}
NeuronTree NeuronPlugin::NeuronTracing_Rollerball_SWC(unsigned char *inimg1d, V3DLONG sx, V3DLONG sy, V3DLONG sz)
{
	
    V3DLONG pagesz_sub = sx*sy*sz;
    m_OiImgWidth = sx;
    m_OiImgHeight =sy;

    unsigned char * pData;
    try
    {
        pData = new  unsigned char[pagesz_sub];
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        return SS;
    }
    int iVesCnt = 1;
    bool b_binarization = true;
    do_seg(inimg1d, pData, sx, sy, sz, iVesCnt, b_binarization);


    static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
    static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
    static int nDz[] = {-1,0,1};

    V3DLONG x,y,z,index;

    V3DLONG imagesize = sy*sx;

    V3DLONG *apsInput;
    try
    {
        apsInput = new V3DLONG[sx*sy*sz];
        memset(apsInput, 0, sx*sy*sz * sizeof(V3DLONG));

    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Distance Transform.");
        if (apsInput) {delete []apsInput; apsInput=0;}
        return SS;
    }
    SpacePoint_t orgPoint;
    orgPoint.m_x = -1;
    orgPoint.m_y = -1;
    orgPoint.m_z = -1;

    for (V3DLONG k=0; k<sz; k++)
    {
        for (V3DLONG j=0; j<sy; j++)
        {
            for (V3DLONG i=0; i<sx; i++)
            {
                index = k * imagesize + j * sx + i;
                int max =0;
                for(int m = 0; m < 3; m++)
                {
                    for(int n = 0; n < 9; n++)
                    {
                        z = k + nDz[m];
                        y = j + nDy[n];
                        x = i + nDx[n];
                        if (z < sz && y < sy && x<sx && x>0 && y>0 && z>0)
                        {
                            if(pData[ z * imagesize + y * sx + x] > max)
                            {
                                max = pData[ z * sx*sy + y * sx + x];
                                m=3;
                                //printf("max=%ld m=%ld\n",max,m);
                            }
                        }else
                        {
                            m = 3;

                        }
                    }
                }
                apsInput[index]=max;
            }
        }
    }

    if(pData) {delete []pData; pData = 0;}

    bool flag = 1;
    V3DLONG m_x,m_y,m_z;

    for (V3DLONG k=0; k<sz; k++)
    {
        for (V3DLONG j=0; j<sy; j++)
        {
            for (V3DLONG i=0; i<sx; i++)
            {
                if (apsInput[k*sx*sy+j*sx+i] == 255)
                {
                    apsInput[k*sx*sy+j*sx+i] = 255;
                    if(flag == 1)
                    {
                        m_x = i + 1;
                        m_y = j + 1;
                        m_z = k + 1;
                        flag =0;
                    }



                }
                else
                {
                    apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
                }
            }
        }
    }


    SetImageInfo1D(apsInput,sz,sx,sy); //temp modify

    ImageMarker S;
    S.x = m_x;
    S.y = m_y;
    S.z = m_z;
    S.on = true;
    listMarker.append(S);

    printf("x=%ld y=%ld z=%ld\n",m_x,m_y,m_z);

    orgPoint.m_x = m_x;
    orgPoint.m_y = m_y;
    orgPoint.m_z = m_z;

	
	Initialize1D();
	
	Set_Seed(orgPoint);
	
	//Initialize1D();
	
	Set_local_DFB();
	
	Set_local_DFS();
	
	try
	{
		m_ppsMaskData = new unsigned char [m_iImgCount * m_iImgWidth * m_iImgHeight]; 
		
		memset(m_ppsMaskData, 0, m_iImgCount * m_iImgWidth * m_iImgHeight * sizeof(unsigned char ));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_ppsMaskData) {delete []m_ppsMaskData; m_ppsMaskData=0;}
		//return;
	}

	/////first extract	
	vector<SpacePoint_t> centerpath;
	
	vector<SpacePoint_t> Rcenterpath;
	
	centerpath.clear();
	
	Rcenterpath.clear();
	
	centerpath = ExtractCenterpath_Ray_D(orgPoint,orgPoint);
	
	//PathMask(Rcenterpath);
	
	//centerpath = Rivise_centerpath(Rcenterpath);
	
	SpacePoint_t direc_point ;
	
	SpacePoint_t swcpoint;
	
	int direc = 0;
	
	double r = 0;
	
	QString file;
		
	centerpathall.push_back(centerpath);
	
	PathMask(centerpath);
	
	NeuronSWC v;
    listNeuron.clear();

	
	for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	{
		v.n	= jj+1;
		v.type = 2;
		v.x = centerpath.at(jj).m_x;
		v.y = centerpath.at(jj).m_y;
		v.z = centerpath.at(jj).m_z;
		v.r = centerpath.at(jj).r;
		if(jj==0)
		{
			v.pn  = jj-1;
		}else 
		{
			v.pn  = jj;
		}
		listNeuron.append(v);
	}	
	
	vector<SpacePoint_t> Branchpoint;
	
	Branchpoint = BranchDetect2(centerpath);
	
	count=0;
	
	SpacePoint_t tempPoint;
	
	for(V3DLONG jj = 0; jj <  Branchpoint.size(); jj++)
	{
	    V3DLONG x = Branchpoint.at(jj).m_x;
        V3DLONG y = Branchpoint.at(jj).m_y;
        V3DLONG z = Branchpoint.at(jj).m_z;
		m_Markerpoint.push_back(Branchpoint.at(jj));
		NueronTree_Ray_D(Branchpoint.at(jj),(Branchpoint.at(jj).node_id));
	}
	
	SS.on = true;
	SS.listNeuron = listNeuron;
    SS.name = "Rollerball_Tracing";
	//QString filename = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/ray cast and distance/neu_tracing.swc";
	//writeSWC_file(filename,SS);	
    return SS;
}

void NeuronPlugin::SearchEndPoints()
{
	
	int i, j, k, l;
	int x, y, z;
	int index, index_nei;
	bool endp;
	DFBPoint_t point_d;
	DFSPoint_t point_l;
	DFSPoint_t last;
	int count1;
	int count2;
	vector<int> len;
	vector<DFSPoint_t> temp;
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	int size = m_vdfbptSurface.size();
	for(int num = 0; num < size; ++num)
	{
		point_d = m_vdfbptSurface.at(num);
		i = point_d.m_z;
		j = point_d.m_y;
		k = point_d.m_x;
		
		index = m_iImgSize * i + m_iImgWidth * j + k;
		l = m_piDFS[index];		
		endp = true;
		count1 = 0;
		count2 = 0;
		
		// 26
		for(int m = 0; m < 3; ++m)
			for(int n = 0; n < 9; ++n)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					if(index != index_nei)
					{
						
						++count1;
						
						if(l <= m_piDFS[index_nei])
						{
							endp = false;
						}
						
						if(l >= m_piDFS[index_nei])
						{
							++count2;
						}
					}
				}
			}
		if(endp)
		{
			point_l.m_x = k;
			point_l.m_y = j;
			point_l.m_z = i;	
			point_l.m_l = l;
			m_vdfsptEndPoint.push_back(point_l);	
			len.push_back(l);
		}
		else
		{
			if(count1 == count2)
			{
				if(m_vdfsptEndPoint.empty())
				{
					point_l.m_x = k;
					point_l.m_y = j;
					point_l.m_z = i;	
					point_l.m_l = l;
					m_vdfsptEndPoint.push_back(point_l);
					len.push_back(l);
				}
				else
				{
					last = m_vdfsptEndPoint.at(m_vdfsptEndPoint.size() - 1);
					if(sqrt(1.0*(last.m_x - k) * (last.m_x - k) + (last.m_y - j) * (last.m_y - j)
							+ (last.m_z - i) * (last.m_z - i)) > DISOFENDS)
					{										
						point_l.m_x = k;
						point_l.m_y = j;
						point_l.m_z = i;	
						point_l.m_l = l;
						m_vdfsptEndPoint.push_back(point_l);	
						len.push_back(l);
					}
				}
			}
		}
	}
	
	//size = len.size();
	//	stable_sort(len.begin(), len.end(), greater<int>());
	//	for(i = 0; i < size; ++i)
	//	{
	//		for(int j = 0; j < size; ++j)
	//		{
	//			if(len.at(i) == m_vdfsptEndPoint.at(j).m_l)
	//			{
	//				temp.push_back(m_vdfsptEndPoint.at(j));
	//				break;
	//			}
	//		}
	//	}
	//	m_vdfsptEndPoint.clear();
	//	m_vdfsptEndPoint = temp;
	
}

void NeuronPlugin::ExtractSingleCenterpath(DFSPoint_t endP)
{	
	int x = 0;
	int y = 0;
	int z = 0;
	
	int i = 0;
	int j = 0;
	int k = 0;
	
    SpacePoint_t point;
	point.m_x = endP.m_x;
	point.m_y = endP.m_y;
	point.m_z = endP.m_z;
	
	int max_d = 0;
	int index = 0;
	int index_nei = 0;
	bool end = false;
	vector<SpacePoint_t> branch;
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	//if(m_ppsMaskData[m_iImgSize * endP.m_z + m_iImgWidth * endP.m_y + endP.m_x] !=0)
	//return;
	
	while(!end)
	{	
		end = true;
		
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		
		max_d = 0;	
        index = m_iImgSize * i + m_iImgWidth * j + k;
		
		for(int m = 0; m < 3; ++m)
			for(int n = 0; n < 9; ++n)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];	
				//index_nei = m_iImgSize * z + m_iImgWidth * y + x;
				
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND )
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					// 
					if(m_piDFS[index_nei]!= LINETAG && m_ppsMaskData[index_nei]==0)
					{
						if(m_piDFS[index_nei] < m_piDFS[index])
						{
							if(m_piDFB[index_nei] > max_d)
							{
								end = false;
								max_d = m_piDFB[index_nei];
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
							}	
						}
					}
					else
					{
						m = 3;
						end = true;
						break;
					}
				}	
			}
		
		if(!end)
		{
			double r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
										point.m_x, point.m_y, point.m_z, 1, false);
			point.r = r;
			branch.push_back(point);
		}
	}
	vector<SpacePoint_t> temp;
	int size = branch.size();
	
	if(size >2) 
	{
		for(int i = size - 1; i >= 0; --i)
		{
		 	point = branch.at(i);
			m_piDFS[point.m_z * m_iImgSize + point.m_y * m_iImgWidth + point.m_x] = LINETAG;
			--point.m_z;
			temp.push_back(point);
 		}
		branch.clear();
		branch = temp;
		m_vvsptCenterpaths.push_back(branch);
	}
	
	vector<SpacePoint_t> Rbranch;
	Rbranch.clear();
	Rbranch = Rivise_centerpath(branch);
	PathMask(Rbranch);
	
	//int x = 0;
	//	int y = 0;
	//	int z = 0;
	//	
	//	int i = 0;
	//	int j = 0;
	//	int k = 0;
	//	
	//    SpacePoint_t point;
	//	SpacePoint_t tmpoint;
	//	point.m_x = endP.m_x;
	//	point.m_y = endP.m_y;
	//	point.m_z = endP.m_z;
	//    
	//	
	//	int max_d = 0;
	//	int index = 0;
	//	int index_nei = 0;
	//	bool end = false;
	//	vector<SpacePoint_t> branch;
	//	
	//	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	//	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	//	static int nDz[] = {-1,0,1};
	//	
	//	
	//	while(!end)
	//	{	
	//		end = true;
	//		
	//		tmpoint.m_x = point.m_x;
	//		tmpoint.m_y = point.m_y;
	//		tmpoint.m_z = point.m_z;
	//		
	//		
	//		printf("px=%ld py=%ld pz=%ld \n",tmpoint.m_x,tmpoint.m_y,tmpoint.m_z);
	//		
	//		i = point.m_z;
	//		j = point.m_y;
	//		k = point.m_x;
	//		
	//		max_d = 0;	
	//        index = m_iImgSize * i + m_iImgWidth * j + k;
	//		
	//		for(int m = 0; m < 3;  ++m)
	//		{
	//			for(int n = 0; n < 9; ++n)
	//			{
	//				z = i + nDz[m];
	//				y = j + nDy[n];
	//				x = k + nDx[n];	
	//				
	//				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
	//				{
	//					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
	//				
	//					if(m_piDFB[index_nei] >max_d)
	//					{
	//						end = false;
	//						max_d = m_piDFB[index_nei];
	//						point.m_x = x;
	//						point.m_y = y;
	//						point.m_z = z;
	//						//printf("tx=%ld ty=%ld tz=%ld \n",point.m_x,point.m_y,point.m_z);
	//						printf("maxd=%ld \n",max_d);
	//					}	
	//
	//				}	
	//			}
	//		}
	//		if(point.m_x == tmpoint.m_x && point.m_y == tmpoint.m_y && point.m_z == tmpoint.m_z)
	//		{
	//		    end = true;
	//			point.m_z = point.m_z-1;
	//			
	//			printf("x=%ld y=%ld z=%ld \n",point.m_x,point.m_y,point.m_z);
	//		}
	//		
	//		
	//		
	//		if(!end)
	//		{
	//			branch.push_back(point);
	//		}
	//	}
	//	
	//	m_vvsptCenterpaths.push_back(branch);	
	//	
	////	vector<SpacePoint_t> temp;
	////	int size = branch.size();
	////	if(size > m_iMinLength)
	////	{
	////		for(int i = size - 1; i >= 0; --i)
	////		{
	////		 	point = branch.at(i);
	////			m_piDFS[point.m_z * m_iImgSize + point.m_y * m_iImgWidth + point.m_x] = LINETAG;
	////			--point.m_z;
	////			temp.push_back(point);
	//// 		}
	////		branch.clear();
	////		branch = temp;
	////		m_vvsptCenterpaths.push_back(branch);
	////	}
}

vector<SpacePoint_t> NeuronPlugin::Rivise_centerpath(vector<SpacePoint_t> centerpath)
{
	vector<SpacePoint_t> Rivisepash;
	Rivisepash.clear();
	
	SpacePoint_t masspoint;
	SpacePoint_t tpoint;
	double d = 0;
	double r =0;
	V3DLONG x,y,z,ir;
	for(V3DLONG jj = 0; jj < centerpath.size(); jj++)
	{
		x = centerpath.at(jj).m_x;
		y = centerpath.at(jj).m_y;
		z = centerpath.at(jj).m_z;
		ir = centerpath.at(jj).r;
		
		//printf("x=%ld y=%ld z=%ld\n",x,y,z);
		int maxr = 0;
		for (V3DLONG cz= z-r; cz <= z+r; cz++)
		{
			for (V3DLONG cy= y-r; cy <= y+r; cy++)
			{
				for (V3DLONG cx= x-r; cx <= x+r; cx++)
				{
					if (cz > m_iImgCount-2 ){continue;}
					if (cy > m_iImgHeight){continue;}
					if (cx > m_iImgWidth) {continue;}
					if (cz < 0 ){continue;}
					if (cy < 0 ){continue;}
					if (cx < 0) {continue;}
					if(m_ppsImgData[cz][m_iImgWidth * cy + cx] != BACKGROUND)
					{
						r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
											 cx, cy, cz, 1, false);
						if(maxr<r)
						{
							maxr = r;
							tpoint.m_x = cx;
							tpoint.m_y = cy;
							tpoint.m_z = cz;
							tpoint.r = r;
						}						
					}
					else 
					{
						continue;						
					}
					//if(m_ppsMaskPoint[cz*m_iImgWidth*m_iImgHeight+cy*m_iImgWidth+cx]==0)
					{
						Rivisepash.push_back(tpoint);
						//	m_ppsMaskPoint[cz*m_iImgWidth*m_iImgHeight+cy*m_iImgWidth+cx]==255;
					}
					
				}
			}
		}
	}
	
	return Rivisepash;
}

SpacePoint_t NeuronPlugin::Rivise_point(SpacePoint_t Rpoint)
{
	SpacePoint_t masspoint;
	SpacePoint_t tpoint;
	double d = 0;
	double r = 0;
	V3DLONG x,y,z,ir;
	
	x = Rpoint.m_x;
	y = Rpoint.m_y;
	z = Rpoint.m_z;
	ir = Rpoint.r;
	
	masspoint = Center_mass(Rpoint);
	
	d = sqrt((masspoint.m_x-x)*(masspoint.m_x-x)+(masspoint.m_y-y)*(masspoint.m_y-y)+(masspoint.m_z-z)*(masspoint.m_z-z));
	printf("d1=%lf\n",d);
	while (d!=0)
	{
		tpoint = Center_mass(masspoint);
		d = sqrt((masspoint.m_x-tpoint.m_x)*(masspoint.m_x-tpoint.m_x)+(masspoint.m_y-tpoint.m_y)*(masspoint.m_y-tpoint.m_y)
				 +(masspoint.m_z-tpoint.m_z)*(masspoint.m_z-tpoint.m_z));
		printf("d=%lf\n",d);
		masspoint.m_x = tpoint.m_x;
		masspoint.m_y = tpoint.m_y;
		masspoint.m_z = tpoint.m_z;
		
		//		r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
		//							 masspoint.m_x, masspoint.m_y, masspoint.m_z, 1, false);
		//		masspoint.r = r;
	}
	masspoint.r = ir;
	printf("x=%ld y=%ld z=%ld\n",x,y,z);
	
	printf("mx=%ld my=%ld mz=%ld\n",masspoint.m_x,masspoint.m_y,masspoint.m_z);
	//m_piMskDFS[masspoint.m_z*m_iImgCount+masspoint.m_y*m_iImgWidth+masspoint.m_x]=255;
	//masspoint.r = ir;
	// masspoint.ray_value[26] = 
	//Rivisepash.push_back(masspoint);
	//		
	//		if(m_piMskDFS[masspoint.m_z*m_iImgCount+masspoint.m_y*m_iImgWidth+masspoint.m_x]!=255)
	//		{
	//			masspoint.r = ir;
	//			Rivisepash.push_back(masspoint);
	//		}else 
	//		{
	//			masspoint.m_x = x;
	//			masspoint.m_y = y;
	//			masspoint.m_z = z;
	//			masspoint.r = ir;
	//			Rivisepash.push_back(masspoint);
	//		}
	//		m_piMskDFS[masspoint.m_z*m_iImgCount+masspoint.m_y*m_iImgWidth+masspoint.m_x]=255;
	
	
	return masspoint;
	
}

SpacePoint_t NeuronPlugin::Center_mass(SpacePoint_t point)
{
	V3DLONG x,y,z,xi,yi,zi;
	SpacePoint_t mpoint;
	double ir;
	x = point.m_x;
	y = point.m_y;
	z = point.m_z;
	ir = point.r;
	xi=yi=zi=0;
	V3DLONG sum =0;
	V3DLONG m = 0;
	printf("x=%ld y=%ld z=%ld ir=%lf\n",x,y,z,ir);
	
	for (V3DLONG cz= z-ir; cz <= z+ir; cz++)
	{
		for (V3DLONG cy= y-ir; cy <= y+ir; cy++)
		{
			for (V3DLONG cx= x-ir; cx <= x+ir; cx++)
			{
				if (cz > m_iImgCount-2 ){continue;}
				if (cy > m_iImgHeight){continue;}
				if (cx > m_iImgWidth) {continue;}
				if (cz < 0 ){continue;}
				if (cy < 0 ){continue;}
				if (cx < 0) {continue;}
				if (m_ppsImgData[cz][m_iImgWidth * cy + cx] != BACKGROUND) 
				{
					m=1;
					xi = xi+cx;
					yi = yi+cy;
					zi = zi+cz;
					sum=sum+1;
				}//else 
				//				{
				//					m=0;
				//				}
				//				sum=sum+m;
				//                xi=xi+cx*m;
				//				yi=yi+cy*m;
				//				zi=zi+cz*m;
			}
		}
	}
	printf("xi=%ld yi=%ld zi=%ld sum=%ld\n",xi,yi,zi,sum);
	if(sum ==0)
	{
		mpoint.m_x = x;
		mpoint.m_y = y;
		mpoint.m_z = z;
		mpoint.r = ir;
	}else 
	{
		mpoint.m_x = xi/sum;
		mpoint.m_y = yi/sum;
		mpoint.m_z = zi/sum;
		mpoint.r = ir;
	}
	//	printf("x=%ld y=%ld z=%ld\n",x,y,z);
	
	//	printf("mx=%ld my=%ld mz=%ld\n",mpoint.m_x,mpoint.m_y,mpoint.m_z);
	
	return mpoint;
}

vector<SpacePoint_t> NeuronPlugin::BranchDetect2(vector<SpacePoint_t> centerpath)
{
	SpacePoint_t tempPoint;
	SpacePoint_t direc_point;
	int seg = centerpathall.size();
	printf("seg==========================%ld\n",seg);
	int direc;
	vector<SpacePoint_t> branch_centerpath;
	//for(V3DLONG jj = centerpath.size()-1 ; jj>=0; jj--)
	for(V3DLONG jj = 0; jj < centerpath.size(); jj++)
	{
		double r = centerpath.at(jj).r;
		//direc_point =  Raycasting3(centerpath.at(jj));
		direc_point =  Raycasting2(centerpath.at(jj));
		direc = Getmaxderection(direc_point);
		direc_point.max_ray = direc_point.ray_value[direc];
		direc_point.max_direct=direc;
		direc_point.node_id = jj;
		direc_point.r = r;
		direc_point.seg_id = seg;
		printf("branch_r=%ld direc=%ld branch_ray=%ld\n",r,direc,direc_point.max_ray);
		printf("seg==========================%ld\n",seg);
		printf("bx=%ld by=%ld bz=%ld bmax=%ld\n",direc_point.m_x,direc_point.m_y,direc_point.m_z,direc_point.max_ray);
		if(direc_point.max_ray > 0)//4
		{
			branch_centerpath.push_back(direc_point);
			//m_ppsMaskBranch[direc_point.m_z*m_iImgWidth*m_iImgHeight + direc_point.m_y*m_iImgWidth + direc_point.m_x]=255;
			printf("bx=%ld by=%ld bz=%ld bmax=%ld\n",direc_point.m_x,direc_point.m_y,direc_point.m_z,direc_point.max_ray);
		}
	}
	return branch_centerpath;
}

QHash<V3DLONG, V3DLONG> NeuronPlugin::ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT) 
{
	QHash<V3DLONG, V3DLONG> cp;
	for (V3DLONG i=0;i<neurons.size(); i++)
		if (neurons.at(i).pn==-1) cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
		else cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), idlist.indexOf(LUT.value(neurons.at(i).pn))); 
	return cp;
}

QHash<V3DLONG, V3DLONG> NeuronPlugin::getUniqueLUT(QList<NeuronSWC> &neurons)
{
	QHash<V3DLONG,V3DLONG> LUT;
	for (V3DLONG i=0;i<neurons.size();i++)
	{
		V3DLONG j;
		for (j=0;j<i;j++)
		{
			if (neurons.at(i).x==neurons.at(j).x && neurons.at(i).y==neurons.at(j).y && neurons.at(i).z==neurons.at(j).z)		break;
		}
		
		LUT.insertMulti(neurons.at(i).n,j);
	}
	return (LUT);
}


void NeuronPlugin::DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group)
{
	numbered[node] = *group;
	neworder[*id] = node;
	(*id)++;
	for (V3DLONG v=0;v<siz;v++)
		if (!numbered[v] && matrix[v][node])
		{
			DFS(matrix, neworder, v, id, siz,numbered,group);
		}
}

double NeuronPlugin::computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
	double xx = s1.x-s2.x;
	double yy = s1.y-s2.y;
	double zz = s1.z-s2.z;
	return (xx*xx+yy*yy+zz*zz);
}

bool NeuronPlugin::SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres)
{
	//seems there are a few memory leaks in this function. I did a quick fix. by PHC, 2012-01-25
	
	//create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
	QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);
	
	//create a new id list to give every different neuron a new id		
	QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
	
	//create a child-parent table, both child and parent id refers to the index of idlist
	QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);
	
	V3DLONG siz = idlist.size();
	
	V3DLONG i;
	bool** matrix = new bool*[siz];
	for (i = 0;i<siz;i++)
	{
		matrix[i] = new bool[siz];
		for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
	}
	
	//generate the adjacent matrix for undirected matrix
	for (i = 0;i<siz;i++)
	{
		QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
		for (V3DLONG j=0;j<parentSet.size();j++)
		{
			V3DLONG v2 = (V3DLONG) (parentSet.at(j));
			if (v2==-1) continue;
			matrix[i][v2] = true;
			matrix[v2][i] = true;
		}
	}
	
	//do a DFS for the the matrix and re-allocate ids for all the nodes
	V3DLONG root;
	if (newrootid==VOID)
	{
		for (V3DLONG i=0;i<neurons.size();i++)
			if (neurons.at(i).pn==-1){
				root = idlist.indexOf(LUT.value(neurons.at(i).n));
				break;
			}
	}
	else
	{
		root = idlist.indexOf(LUT.value(newrootid));
		
		if (LUT.keys().indexOf(newrootid)==-1)
		{
			v3d_msg("The new root id you have chosen does not exist in the SWC file.");
			return(false);
		}
	}
	
	
	V3DLONG* neworder = new V3DLONG[siz];
	int* numbered = new int[siz];
	for (i=0;i<siz;i++) numbered[i] = 0;
	
	V3DLONG id[] = {0};
	
	int group[] = {1};
	DFS(matrix,neworder,root,id,siz,numbered,group);
	
	while (*id<siz)
	{
		V3DLONG iter;
		(*group)++;
		for (iter=0;iter<siz;iter++)
			if (numbered[iter]==0) break;
		DFS(matrix,neworder,iter,id,siz,numbered,group);
	}
	
	
	//find the point in non-group 1 that is nearest to group 1, 
	//include the nearest point as well as its neighbors into group 1, until all the nodes are connected
	while((*group)>1)
	{
		double min = VOID;
		double dist2 = 0;
		int mingroup = 1;
		V3DLONG m1,m2;
		for (V3DLONG ii=0;ii<siz;ii++){
			if (numbered[ii]==1)
				for (V3DLONG jj=0;jj<siz;jj++)
					if (numbered[jj]!=1)
					{
						dist2 = computeDist2(neurons.at(idlist.at(ii)),neurons.at(idlist.at(jj)));
						if (dist2<min)
						{
							min = dist2;
							mingroup = numbered[jj];
							m1 = ii;
							m2 = jj;
						}
					}
		}
		for (i=0;i<siz;i++)
			if (numbered[i]==mingroup)
				numbered[i] = 1;
		if (min<=thres*thres)
		{
			matrix[m1][m2] = true;
			matrix[m2][m1] = true;
		}
		(*group)--;
	}
	
	id[0] = 0;
	for (i=0;i<siz;i++)
	{
		numbered[i] = 0;
		neworder[i]= VOID;
	}
	
	*group = 1;
	
	V3DLONG new_root=root;
	V3DLONG offset=0;
	while (*id<siz)
	{
		V3DLONG cnt = 0;
		DFS(matrix,neworder,new_root,id,siz,numbered,group);
		(*group)++;
		NeuronSWC S;
		S.n = offset+1;
		S.pn = -1;
		V3DLONG oripos = idlist.at(new_root);
		S.x = neurons.at(oripos).x;
		S.y = neurons.at(oripos).y;
		S.z = neurons.at(oripos).z;
		S.r = neurons.at(oripos).r;
		S.type = neurons.at(oripos).type;
		result.append(S);
		cnt++;
		
		for (V3DLONG ii=offset+1;ii<(*id);ii++)
		{
			for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
			{
				if (neworder[ii]!=VOID && neworder[jj]!=VOID && matrix[neworder[ii]][neworder[jj]] ) 
				{
					NeuronSWC S;
					S.n = ii+1;
					S.pn = jj+1;
					V3DLONG oripos = idlist.at(neworder[ii]);
					S.x = neurons.at(oripos).x;
					S.y = neurons.at(oripos).y;
					S.z = neurons.at(oripos).z;
					S.r = neurons.at(oripos).r;
					S.type = neurons.at(oripos).type;
					result.append(S);
					cnt++;
				}
			}
		}
		for (new_root=0;new_root<siz;new_root++)
			if (numbered[new_root]==0) break;
		offset += cnt;
	}
	
	if ((*id)<siz) {
		v3d_msg("Error!");
		return false;
	}
	
	//try to free space by PHC, 2012-01-25
	if (neworder) {delete []neworder; neworder=0;}
	if (numbered) {delete []numbered; numbered=0;}
	if (matrix)
	{
		for (i = 0;i<siz;i++)
			if (matrix[i]) {delete matrix[i]; matrix[i]=0;}
		if (matrix) {delete []matrix; matrix=0;}
	}	
	
	//
	return true;
}

bool NeuronPlugin::Output_SWC(QList<NeuronSWC> & neurons, NeuronTree & result)
{
	vector<NeuronTree> swc;
	
	swc.clear();
	
	NeuronTree ts;
	
	QList <NeuronSWC>  list;
	
	int t=0;
	NeuronSWC v;
	int max = 0;
	for (V3DLONG i=0;i<neurons.size();i++)
	{
		if (neurons.at(i).pn==-1)
		{
			list.clear();
			v.n = neurons.at(i).n;
			v.pn = neurons.at(i).pn;
			v.x = neurons.at(i).x;
			v.y = neurons.at(i).y;
			v.z = neurons.at(i).z;
			v.r = neurons.at(i).r;
			v.type = 2;
		    list.append(v);
			int t = i+1;
			//printf("t=%ld\n",t);
			bool bmax = true;
			int n =0;
			while ((neurons.at(t).pn != -1) && bmax) 
			{
				v.n = neurons.at(t).n;
				v.pn = neurons.at(t).pn;
				v.x = neurons.at(t).x;
				v.y = neurons.at(t).y;
				v.z = neurons.at(t).z;
				v.r = neurons.at(t).r;
				v.type = 2;
				list.append(v);
				t++;
				n++;
				//printf("t=============%ld\n",t);
				//printf("size=====%ld\n",neurons.size());
				if(t > neurons.size()-1)
				{
					t = neurons.size()-1;
					bmax = false;
				}
			}
			if(n >max)
			{
				max = n;
				result.on = true;
				result.listNeuron = list;
				result.name = "neu_tracing_distance_result";
			}
			
		}
	}
	
	if (result.listNeuron.size()>0)
	{
		return true;
	}else 
	{
		return false;
		
	}
}

void NeuronPlugin::Initialize1D()
{	
	m_iImgSize = m_iImgWidth * m_iImgHeight;
	
	m_ppsImgData = new V3DLONG*[m_iImgCount + 2];
	
	m_psTemp = new V3DLONG[m_iImgSize];
	memset(m_psTemp, BACKGROUND, m_iImgSize * sizeof(V3DLONG));
	m_ppsImgData[0] = m_psTemp;
	
	for(V3DLONG c = 0; c < m_iImgCount; ++c)
	{
		m_ppsImgData[c + 1] = &m_ppsOriData1D[c*m_iImgSize];
	}
	
	m_psTemp = new V3DLONG[m_iImgSize];
	
	memset(m_psTemp, BACKGROUND, m_iImgSize * sizeof(V3DLONG));
	
	m_ppsImgData[m_iImgCount + 1] = m_psTemp;
	
	m_iImgCount += 2;
	
	m_ulVolumeSize = m_iImgCount * m_iImgSize;
	
	printf("m_iImgCount=%d w=%d h=%d\n",m_iImgCount,m_iImgWidth,m_iImgHeight);
}

void NeuronPlugin::Clear()
{
	if(m_piDFB)
	{
		delete []m_piDFB;
		m_piDFB = NULL;
	}
	
	if(m_piDFS)
	{
		delete []m_piDFS;
		m_piDFS = NULL;
	}
	if(m_ppsImgData[0])
	{
		delete []m_ppsImgData[0];
		m_ppsImgData[0] = NULL;
	}
	if(m_ppsImgData[m_iImgCount - 1])
	{
		delete []m_ppsImgData[m_iImgCount - 1];
		m_ppsImgData[m_iImgCount - 1] = NULL;
	}
	if(m_ppsImgData)
	{
		delete []m_ppsImgData;
		m_ppsImgData = NULL;
	}
	
	m_iImgWidth = 0;
	
	m_iImgHeight = 0;
	
	m_iImgSize = 0;
	
	m_iImgCount = 0;
	
	m_ulVolumeSize = 0;
	
	m_vdfsptEndPoint.clear();
	
	m_vdfbptSurface.clear();
	
	m_vsptCenterpath.clear();
}	

void NeuronDialog::update()
{
	//get current data
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;
		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
}

void NeuronPlugin::Set_local_DFB()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	V3DLONG count = 0;
	V3DLONG index = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG k = 0;
	
	bool find;
    DFBPoint_t point;
	
	deque<DFBPoint_t> surface;
	
	try
	{
		m_piDFB = new V3DLONG[m_ulVolumeSize]; 
		memset(m_piDFB, BACKGROUND, m_ulVolumeSize * sizeof(V3DLONG));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piDFB) {delete []m_piDFB; m_piDFB=0;}
		return;
	}
	//printf("countnew=%d\n",m_iImgCount);
	
	for(i = 0; i < m_iImgCount; i++)
	{
		for(j = 0; j < m_iImgHeight; j++)
		{
			for(k = 0; k < m_iImgWidth; k++)
			{
				if(m_ppsImgData[i][m_iImgWidth * j + k] != BACKGROUND)
				{
					count = 0;
					index = m_iImgSize * i + m_iImgWidth * j + k;
					find = false;
					// ’“26
					for(int m = 0; m < 3; ++m)
					{
						for(int n = 0; n < 9; ++n)
						{
							z = i + nDz[m];
							y = j + nDy[n];
							x = k + nDx[n];
							++count;
							if(m_ppsImgData[z][m_iImgWidth * y + x] == BACKGROUND)
							{
								find = true;
								switch(count)
								{
										// 6 faces
									case 5:
									case 11:
									case 13:
									case 15:
									case 17:
									case 23:                          
										m_piDFB[index] = 3;									
										break;
										// 12 edges
									case 2:
									case 4:
									case 6:
									case 8:
									case 10:
									case 12:
									case 16:
									case 18:
									case 20:
									case 22:
									case 24:
									case 26:
										if(m_piDFB[index] == BACKGROUND || m_piDFB[index] == 5)
											m_piDFB[index] = 4;									
										break;
										// 8 vertexes
									case 1:
									case 3:
									case 7:
									case 9:
									case 19:
									case 21:
									case 25:
									case 27:
										if(m_piDFB[index] == BACKGROUND)
											m_piDFB[index] = 5;											
										break;
									default:
										break;
								}
							}
						}
					}
					if(find)
					{
						point.m_x = k;
						point.m_y = j;
						point.m_z = i;
						point.m_d = m_piDFB[index];
						surface.push_back(point);
						m_vdfbptSurface.push_back(point);
					}
				}
			}
		}
	}
	
	// 
	int d = 0;
	int temp_d = 0;
	int index_nei = 0;
	
	while(!surface.empty())
	{	
        point = surface.front();
		surface.pop_front();
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		d = point.m_d;
		count = 0;
		// ’“26
		for(int m = 0; m < 3; m++)
			for(int n = 0; n < 9; n++)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				
				count++;
				if (z >= m_iImgCount){continue;}
				if (y >= m_iImgHeight){continue;}
				if (x >= m_iImgWidth) {continue;}
				if (z < 0 ) {continue;}
				if ( y < 0 ){continue;}
				if ( x < 0) {continue;}
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					switch(count)
					{
							// 6 faces
						case 5:
						case 11:
						case 13:
						case 15:
						case 17:
						case 23:
							temp_d = d + 3;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								if(temp_d < m_piDFB[index_nei])
								{
									m_piDFB[index_nei] = temp_d;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_d = temp_d;
									surface.push_back(point);
								}
							}
							else
							{
								m_piDFB[index_nei] = temp_d;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_d = temp_d;
								surface.push_back(point);
							}
							break;
							// 12 edges
						case 2:
						case 4:
						case 6:
						case 8:
						case 10:
						case 12:
						case 16:
						case 18:
						case 20:
						case 22:
						case 24:
						case 26:				
							temp_d = d + 4;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								if(temp_d < m_piDFB[index_nei])
								{
									m_piDFB[index_nei] = temp_d;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_d = temp_d;
									surface.push_back(point);
								}
							}
							else
							{
								m_piDFB[index_nei] = temp_d;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_d = temp_d;
								surface.push_back(point);
							}
							break;
							// 8 vertexes
						case 1:
						case 3:
						case 7:
						case 9:
						case 19:
						case 21:
						case 25:
						case 27:
							temp_d = d + 5;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								if(temp_d < m_piDFB[index_nei])
								{
									m_piDFB[index_nei] = temp_d;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_d = temp_d;
									surface.push_back(point);
								}
							}
							else
							{
								m_piDFB[index_nei] = temp_d;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_d = temp_d;
								surface.push_back(point);
							}
							break;
						default:
							break;
					}
				}
			}
	}
	
	///////////////////////////
	//	for(i = 0; i < m_iImgCount; i++)
	//	{
	//		for(j = 0; j < m_iImgHeight; j++)
	//		{	for(k = 0; k < m_iImgWidth; k++)
	//			{
	//				if (m_piDFB[i*m_iImgSize+j*m_iImgWidth+k] > -1) 
	//				{
	//					// printf("m_piDFB=%d\n",m_piDFB[i*m_iImgSize+j*m_iImgWidth+k]);
	//					
	//				}
	//			}
	//		}
	//	}
	//	
}

void NeuronPlugin::Set_local_DFS()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	V3DLONG count = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG k = 0;
	V3DLONG l = 0;
    DFSPoint_t point;
	deque<DFSPoint_t> dfs;
	
	V3DLONG temp_l = 0;
	V3DLONG index_nei = 0;
	
	point.m_x = m_sptSeed.m_x;
	point.m_y = m_sptSeed.m_y;
	point.m_z = m_sptSeed.m_z + 1;
	point.m_l = 0;
	dfs.push_back(point);
	try
	{
		m_piDFS = new V3DLONG[m_ulVolumeSize]; 
		memset(m_piDFS, BACKGROUND, m_ulVolumeSize * sizeof(V3DLONG));
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
		return;
	}
	// dis(S) = 0;
	m_piDFS[m_iImgSize * point.m_z + m_iImgWidth * point.m_y + point.m_x] = 0;
	while(!dfs.empty())
	{	
        point = dfs.front();
		dfs.pop_front();
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		l = point.m_l;
		count = 0;
		// ’“26
		for(int m = 0; m < 3; m++)
			for(int n = 0; n < 9; n++)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				if (z >= m_iImgCount-2){continue;}
				if (y >= m_iImgHeight){continue;}
				if (x >= m_iImgWidth) {continue;}
				if (z <= 0 ) {continue;}
				if ( y <= 0 ){continue;}
				if ( x <= 0) {continue;}
				++count;
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					switch(count)
					{
							// 6 faces
						case 5:
						case 11:
						case 13:
						case 15:
						case 17:
						case 23:
							temp_l = l + 1;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									//printf("temp=%d m_pidfs=%d\n",temp_l,m_piDFS[index_nei]);
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								//	printf("tem=%d",temp_l);
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							// 12 edges
						case 2:
						case 4:
						case 6:
						case 8:
						case 10:
						case 12:
						case 16:
						case 18:
						case 20:
						case 22:
						case 24:
						case 26:				
							temp_l = l + 2;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							// 8 vertexes
						case 1:
						case 3:
						case 7:
						case 9:
						case 19:
						case 21:
						case 25:
						case 27:
							temp_l = l + 3;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
						default:
							break;
					}
				}
			}
	}
	//	for(i = 0; i < m_iImgCount; i++)
	//	{
	//		for(j = 0; j < m_iImgHeight; j++)
	//		{
	//			for(k = 0; k < m_iImgWidth; k++)
	//			{
	//				if (m_piDFS[i*m_iImgSize+j*m_iImgWidth+k] > -1) 
	//				{
	//					printf("m_piDFS=%d\n",m_piDFS[i*m_iImgSize+j*m_iImgWidth+k]);
	//				}
	//			}
	//		}
	//	}	
}

vector<SpacePoint_t> NeuronPlugin::ExtractCenterpath_Ray_Cast(SpacePoint_t points)//bak
{
	//	vector<SpacePoint_t> centerpath;
	//	
	//	SpacePoint_t direc_point;
	//	
	//	SpacePoint_t Ncentepoint;
	//	
	//	SpacePoint_t centerpoint;
	//	
	//	SpacePoint_t swcpoint;
	//	int direc = 0;
	//	double r = 0;
	//	
	//	Ncentepoint.m_x = points.m_x;
	//	Ncentepoint.m_y = points.m_y;
	//	Ncentepoint.m_z = points.m_z;
	//	centerpath.push_back(Ncentepoint);
	//	centerpoint.max_ray = 100;
	//	
	//	printf("nx=%ld ny=%ld nz=%ld \n",Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
	//
	//	if (points.m_z > m_iImgCount-2){return centerpath;}
	//	if (points.m_y > m_iImgHeight){return centerpath;}
	//	if (points.m_x > m_iImgWidth) {return centerpath;}
	//	if (points.m_z < 0 ) {return centerpath;}
	//	if ( points.m_y < 0 ){return centerpath;}
	//	if ( points.m_x < 0) {return centerpath;}
	//	if(m_ppsImgData[points.m_z][m_iImgWidth * points.m_y + points.m_x] == BACKGROUND)
	//	{
	//		printf("ex=%ld ey=%ld ez=%ld \n",Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
	//		return centerpath;
	//	}
	//    bool search = true;
	//	while (centerpoint.max_ray > 0) //6
	//	{
	//		//printf("nx=%ld ny=%ld nz=%ld \n", Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
	//		
	//		//printf("x=%ld y=%ld z=%ld \n", centerpoint.m_x,centerpoint.m_y,centerpoint.m_z);
	//		
	//		if(m_ppsImgData[Ncentepoint.m_z][m_iImgWidth * Ncentepoint.m_y + Ncentepoint.m_x] == BACKGROUND)
	//		{
	//			//search=false;
	//			return centerpath;
	//		}
	//		direc_point = Raycasting2(Ncentepoint);
	//	    
	//		direc = Getmaxderection(direc_point);
	//		
	//		centerpoint = GetCenterPiont(Ncentepoint,direc);
	//		
	//		centerpoint.max_ray = direc_point.ray_value[direc];
	//		
	//		printf("direc=%ld\n",direc);
	//		
	//		printf("ray=%ld\n",direc_point.ray_value[direc]);
	//		
	//		r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
	//							 centerpoint.m_x, centerpoint.m_y, centerpoint.m_z, 1, false);	
	//		
	//		printf("DB=%ld r=%lf\n",centerpoint.sd,r);
	//		
	//		Ncentepoint = Coumpere_ball_center(centerpoint, direc, r);
	//		
	//		if(Ncentepoint.m_x == centerpoint.m_x && Ncentepoint.m_y == centerpoint.m_y && Ncentepoint.m_z == centerpoint.m_z)
	//		{
	//			//search=false;
	//			return centerpath;
	//		}
	//		swcpoint.r = r;
	//		swcpoint.m_x = centerpoint.m_x;
	//		swcpoint.m_y = centerpoint.m_y;
	//		swcpoint.m_z = centerpoint.m_z;
	//		swcpoint.max_ray = centerpoint.max_ray;
	//		if(swcpoint.max_ray > 0)//8
	//			centerpath.push_back(swcpoint);
	//		//printf("next_drc=%ld next_r=%lf next_x=%ld next_y=%ld next_z=%ld next_ray=%ld \n", direc,r,centerpoint.m_x,centerpoint.m_y,centerpoint.m_z,direc_point.ray[direc]);	
	//	}
	//	
	//	//PathMask(centerpath);
	//	
	//	return centerpath;
	vector<SpacePoint_t> centerpath;
	
	SpacePoint_t direc_point;
	
	SpacePoint_t Ncentepoint;
	
	SpacePoint_t centerpoint;
	
	SpacePoint_t swcpoint;
	
	int direc = 0;
	
	double r = 0;
	
	Ncentepoint.m_x = points.m_x;
	Ncentepoint.m_y = points.m_y;
	Ncentepoint.m_z = points.m_z;
	
	r = fitRadiusPercent_cast(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
							  Ncentepoint.m_x, Ncentepoint.m_y, Ncentepoint.m_z, 1, false);	
	Ncentepoint.r = r;
	
	centerpath.push_back(Ncentepoint);
	
	centerpoint.max_ray = 100;
	
	printf("nx=%ld ny=%ld nz=%ld \n",Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
	
	if(m_ppsImgData[points.m_z][m_iImgWidth * points.m_y + points.m_x] == BACKGROUND)
	{
		printf("ex=%ld ey=%ld ez=%ld \n",Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
		return centerpath;
	}
	while (centerpoint.max_ray > 1) //6
	{
		//printf("nx=%ld ny=%ld nz=%ld \n", Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
		
		//printf("x=%ld y=%ld z=%ld \n", centerpoint.m_x,centerpoint.m_y,centerpoint.m_z);
		
		direc_point = Raycasting2(Ncentepoint);
	    
		direc = Getmaxderection(direc_point);
		
		centerpoint = GetCenterPiont(Ncentepoint,direc);
		
		centerpoint.max_ray = direc_point.ray_value[direc];
		
		printf("direc=%ld\n",direc);
		
		printf("ray=%ld\n",direc_point.ray_value[direc]);
		
		r = fitRadiusPercent_cast(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
								  centerpoint.m_x, centerpoint.m_y, centerpoint.m_z, 1, false);	
		//		r = EstimateRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
		//								  centerpoint.m_x, centerpoint.m_y, centerpoint.m_z, 1, false);
		centerpoint.r = r;
		
		//PointMask(centerpoint);
		
		printf("DB=%ld r=%lf\n",centerpoint.sd,r);
		
		Ncentepoint = Coumpere_ball_center(centerpoint, direc, r);
		
		//		if(Ncentepoint.m_x > m_iImgWidth && Ncentepoint.m_y > m_iImgHeight && Ncentepoint.m_z > m_iImgCount && Ncentepoint.m_x <0 && Ncentepoint.m_y <0 && Ncentepoint.m_z <0)
		//		{
		//			return centerpath;
		//		}
		swcpoint.r = r;
		swcpoint.m_x = centerpoint.m_x;
		swcpoint.m_y = centerpoint.m_y;
		swcpoint.m_z = centerpoint.m_z;
		swcpoint.max_ray = centerpoint.max_ray;	
		printf("centerray=%ld\n",centerpoint.max_ray);
		//if(swcpoint.max_ray > 2)//8
		centerpath.push_back(swcpoint);
		
		if(Ncentepoint.m_x == centerpoint.m_x && Ncentepoint.m_y == centerpoint.m_y && Ncentepoint.m_z == centerpoint.m_z)
		{
			return centerpath;
		}
		//		if(m_ppsMaskData[Ncentepoint.m_z*m_iImgWidth*m_iImgHeight + Ncentepoint.m_y*m_iImgWidth + Ncentepoint.m_x]==255)
		//		{
		//			printf("255555555555555555555555555\n");
		//			return centerpath;
		//		}
		//printf("next_drc=%ld next_r=%lf next_x=%ld next_y=%ld next_z=%ld next_ray=%ld \n", direc,r,centerpoint.m_x,centerpoint.m_y,centerpoint.m_z,direc_point.ray[direc]);	
	}
	return centerpath;
}


void NeuronPlugin::SaveSwcTree_cast(vector<SpacePoint_t> centerpath,V3DLONG pn_id,V3DLONG node_count)
{
	QString file;
	QHash <int, int>  hashNeuron;
	//listNeuron.clear();
	hashNeuron.clear();
	NeuronSWC v;
	int temp = centerpathall.size();
	//	if(temp == 1)
	//	{
	//		for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	//		{
	//			if(jj==0)
	//			{
	//				v.n	= jj+node_count+1;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = pn_num;
	//				//v.seg_id = jj;
	//				//v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//			}
	//			else 
	//			{
	//				v.n	= jj+node_count+1;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = jj+node_count;
	//				//	v.seg_id = jj;
	//				//	v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//			}
	//		}
	//	}
	//	else 
	{
		for(V3DLONG jj = 1; jj< centerpath.size(); jj++)
		{
			//			if(jj==0)
			//			{
			//				v.n	= pn_num;
			//				v.type = 2;
			//				v.x = centerpath.at(jj).m_x;
			//				v.y = centerpath.at(jj).m_y;
			//				v.z = centerpath.at(jj).m_z;
			//				v.r = centerpath.at(jj).r;
			//				v.pn  = -1;
			//				//v.seg_id = jj;
			//				//v.nodeinseg_id =jj;
			//				listNeuron.append(v);
			//				hashNeuron.insert(v.n, listNeuron.size()-1);
			//			}
			if(jj==1)
			{
				v.n	= jj + node_count;
				v.type = 2;
				v.x = centerpath.at(jj).m_x;
				v.y = centerpath.at(jj).m_y;
				v.z = centerpath.at(jj).m_z;
				v.r = centerpath.at(jj).r;
				v.pn  = pn_id+1;
				v.seg_id = centerpath.at(jj).seg_id;
				//v.nodeinseg_id =jj;
				listNeuron.append(v);
				hashNeuron.insert(v.n, listNeuron.size()-1);
				
			}
			else 
			{
				v.n	= jj + node_count;
				v.type = 2;
				v.x = centerpath.at(jj).m_x;
				v.y = centerpath.at(jj).m_y;
				v.z = centerpath.at(jj).m_z;
				v.r = centerpath.at(jj).r;
				v.pn  = jj+node_count -1;
				v.seg_id = centerpath.at(jj).seg_id;
				//	v.seg_id = jj;
				//	v.nodeinseg_id =jj;
				listNeuron.append(v);
				hashNeuron.insert(v.n, listNeuron.size()-1);
			}
		}
		
	}
	
	//	if(temp == 1)
	//	{
	//		for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	//		{
	//			if(jj==0)
	//			{
	//				v.n	= jj;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = -1;
	//				//v.seg_id = jj;
	//				//v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//			}
	//			else 
	//			{
	//				v.n	= jj;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = jj-1;
	//				//	v.seg_id = jj;
	//				//	v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//			}
	//		}
	//	}
	//	else 
	//	{
	//		for(V3DLONG jj = 0; jj< centerpath.size(); jj++)
	//		{
	//			if(jj==0)
	//			{
	//				v.n	= pn_num;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = -1;
	//				//v.seg_id = jj;
	//				//v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//			}if(jj==1)
	//			{
	//				v.n	= jj + node_count;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = pn_num;
	//				//v.seg_id = jj;
	//				//v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//				
	//			}
	//			else 
	//			{
	//				v.n	= jj + node_count;
	//				v.type = 2;
	//				v.x = centerpath.at(jj).m_x;
	//				v.y = centerpath.at(jj).m_y;
	//				v.z = centerpath.at(jj).m_z;
	//				v.r = centerpath.at(jj).r;
	//				v.pn  = jj+node_count-1;
	//				//	v.seg_id = jj;
	//				//	v.nodeinseg_id =jj;
	//				listNeuron.append(v);
	//				hashNeuron.insert(v.n, listNeuron.size()-1);
	//			}
	//		}
	//		
	//		
	//	}
	//	SS.on = true;
	//	SS.listNeuron = listNeuron;
	//	SS.hashNeuron = hashNeuron;
	//	SS.name = "neu_tracing";
	//	QString filename = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/neu_tracing.swc";
	//	writeSWC_file(filename,SS);
}

void NeuronPlugin::NueronTree2(SpacePoint_t point,int n)
{
	vector<SpacePoint_t> Branchpoint;
	vector<SpacePoint_t> centerpath;
	Branchpoint.clear();
	centerpath.clear();
	
	V3DLONG num = 0;
	V3DLONG nodeid = 0;
	
	int direc = point.max_direct;
	
	printf("direc=%ld sx=%ld sy=%ld sz=%ld point.r=%lf\n",direc,point.m_x,point.m_y,point.m_z,point.r);
	
	SpacePoint_t Ncentepoint = Coumpere_ball_center(point, direc, point.r);
	
	printf("nsx=%ld nsy=%ld nsz=%ld \n",Ncentepoint.m_x,Ncentepoint.m_y,Ncentepoint.m_z);
	
	//	if(m_ppsMaskData[Ncentepoint.m_z*m_iImgWidth*m_iImgHeight + Ncentepoint.m_y*m_iImgWidth + Ncentepoint.m_x]==255)
	//	{
	//		return;
	//	}
	//	printf("DB=%ld r=%lf\n",centerpoint.sd,r);
	
	centerpath = ExtractCenterpath_Ray_Cast(Ncentepoint);
	//centerpath.insert(centerpath.begin(),point);
	
	if(centerpath.size()>2)//4
	{   
		//printf("xxx=%ld yyyy=%ld zzzz=%ld\n",tempPoint.m_x,tempPoint.m_y,tempPoint.m_z);
		
		for(V3DLONG jj = 0; jj<centerpathall.size();jj++)
		{
			num = num + centerpathall.at(jj).size(); 
		}
		
		if(point.seg_id == 1)
		{
			centerpath.insert(centerpath.begin(),point);
			
			SaveSwcTree_cast(centerpath,n,num);
			
			centerpath.erase(centerpath.begin());
			
			centerpathall.push_back(centerpath);
		}else
		{
			for(V3DLONG ii = 0; ii<point.seg_id-1; ii++)
			{
				nodeid += centerpathall.at(ii).size();
			}
			centerpath.insert(centerpath.begin(),point);
			
			printf("branchnum++++++++++++++++++++++++++=%ld n=%ld nodeid=%ld\n",num,n,nodeid);
			
			printf("xxx=%ld yyyy=%ld zzzz=%ld\n",point.m_x,point.m_y,point.m_z);
			
			SaveSwcTree_cast(centerpath,n+nodeid,num);
			
			centerpath.erase(centerpath.begin());
			
			centerpathall.push_back(centerpath);
		}
		
		//			QString file;
		//			file.sprintf("%d",n);
		//			QString filepath = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/ray cast/";
		//			QString curImageName1 = filepath + file + ".swc";
		//			SaveSwcfile(centerpath,curImageName1);
		//			printf("CCCCCCCCCCCCCCCCCCCCCCCCCCCC");
		Branchpoint = BranchDetect2(centerpath);
		
		if(Branchpoint.size()>0)
		{
			for(V3DLONG ii = 0 ; ii< Branchpoint.size(); ii++)
			{
				//NueronTree2(Branchpoint.at(ii),(Branchpoint.at(ii).node_id+num));
				NueronTree2(Branchpoint.at(ii),Branchpoint.at(ii).node_id);
				
			}
		}else 
		{
			return;
		}
	}
	
}

vector<SpacePoint_t> NeuronPlugin::ExtractCenterpath_Ray_D(SpacePoint_t Cpoints,SpacePoint_t NSpoint)
{	
	int x = 0;
	int y = 0;
	int z = 0;
	
	int i = 0;
	int j = 0;
	int k = 0;
	
	SpacePoint_t point;
	point.m_x = NSpoint.m_x;
	point.m_y = NSpoint.m_y;
	point.m_z = NSpoint.m_z;
	
	int max_d = 0;
	int index = 0;
	int index_nei = 0;
    int ted = 0;
	bool end = false;
	bool single = false;
	int  num=0;
	
	vector<SpacePoint_t> branch;
	
	branch.clear();
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	double r = 0;
	double d = 0;
	
	if (point.m_z >= m_iImgCount-2){return branch;}
	if (point.m_y >= m_iImgHeight){return branch;}
	if (point.m_x >= m_iImgWidth) {return branch;}
	if (point.m_z <= 0 ) {return branch;}
	if (point.m_y <= 0 ){return branch;}
	if (point.m_x <= 0) {return branch;}
	
	//	r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
	//						 Cpoints.m_x, Cpoints.m_y, Cpoints.m_z, 1, false);
	//	Cpoints.r = r;
	//	branch.push_back(Cpoints);
	
	r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
						 point.m_x, point.m_y, point.m_z, 1, false);
	//	r = EstimateRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
	//						 point.m_x, point.m_y, point.m_z, 1, false);
	point.r = r;
	branch.push_back(point);
	
	if(m_ppsImgData[point.m_z][m_iImgWidth * point.m_y + point.m_x] == BACKGROUND)
	{
		return branch;
	}
	//
	//	while(!end)
	//	{	
	//		end = true;
	//		//printf("px=%ld py=%ld pz=%ld \n",tmpoint.m_x,tmpoint.m_y,tmpoint.m_z);
	//		i = point.m_x;
	//		j = point.m_y;
	//		k = point.m_z;
	//		max_d = 0;	
	//		index = m_iImgSize * k + m_iImgWidth * j + i;
	//		int num = 0;
	//		for(int m = 0; m < 3;  m++)
	//		{
	//			for(int n = 0; n < 9; n++)
	//			{
	//				z = k + nDz[m];
	//				y = j + nDy[n];
	//				x = i + nDx[n];	
	//				if (z > m_iImgCount){continue;}
	//				if (y > m_iImgHeight){continue;}
	//				if (x > m_iImgWidth) {continue;}
	//				if (z < 0 ) {continue;}
	//				if (y < 0 ){continue;}
	//				if (x < 0) {continue;}
	//				//	
	//				index_nei = m_iImgSize * z + m_iImgWidth * y + x;
	//				if(index_nei > m_ulVolumeSize || index > m_ulVolumeSize)
	//					continue;
	//				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND && m_ppsMaskData[z * m_iImgSize + m_iImgWidth * y + x] == 0)
	//				{
	//					//index_nei = m_iImgSize * z + m_iImgWidth * y + x;
	//						if(m_piDFS[index_nei] > m_piDFS[index])
	//						{
	//							if(m_piDFB[index_nei] >max_d)
	//							{
	//								end = false;
	//								max_d = m_piDFB[index_nei];
	//								point.m_x = x;
	//								point.m_y = y;
	//								point.m_z = z;
	//
	//							}
	//						}
	//				}
	//
	//			}
	//		}
	//
	//		
	//		if(!end)
	//		{
	//			r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
	//								 point.m_x, point.m_y, point.m_z, 1, false);
	//			
	////			r = EstimateRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
	////								 point.m_x, point.m_y, point.m_z, 1, false);
	//			point.r = r;
	//			//SpacePoint_t Tpoint;
	////			
	////			Tpoint = Rivise_point(point);
	//			if(m_ppsImgData[point.m_z][m_iImgWidth * point.m_y + point.m_x] != BACKGROUND )
	//			{
	//				branch.push_back(point);
	//				
	//			}
	//		}
	//		
	//	}
	
	
	while(!end)
	{	
		end = true;
		//printf("px=%ld py=%ld pz=%ld \n",tmpoint.m_x,tmpoint.m_y,tmpoint.m_z);
		i = point.m_x;
		j = point.m_y;
		k = point.m_z;
		max_d = 0;	
		index = m_iImgSize * k + m_iImgWidth * j + i;
		//	int num = 0;
		for(int m = 0; m < 3;  m++)
		{
			for(int n = 0; n < 9; n++)
			{
				z = k + nDz[m];
				y = j + nDy[n];
				x = i + nDx[n];	
				//if (z > m_iImgCount){continue;}
				//				if (y > m_iImgHeight){continue;}
				//				if (x > m_iImgWidth) {continue;}
				//				if (z < 0 ) {continue;}
				//				if (y < 0 ){continue;}
				//				if (x < 0) {continue;}
				//	
				
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND && m_ppsMaskData[z * m_iImgSize + m_iImgWidth * y + x] == 0)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					//if(m_ppsMaskData[index_nei] == 0)
					if(m_piDFS[index_nei]!= LINETAG && m_ppsMaskData[index_nei]!=255)
					{
						if(m_piDFS[index_nei] > m_piDFS[index])
						{
							if(m_piDFB[index_nei] > max_d)
							{
								end = false;
								max_d = m_piDFB[index_nei];
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
							}	
						}
					}
					else
					{
						single = true;
						m = 3;
						end = true;
						break;
					}
				}else {
					num++;
				}
				
			}
		}
		
		if(!end)
		{
			r = fitRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
								 point.m_x, point.m_y, point.m_z, 1, false);
			
			//			r = EstimateRadiusPercent(m_ppsImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 255, 0,
			//								 point.m_x, point.m_y, point.m_z, 1, false);
			point.r = r;
			branch.push_back(point);
		}
		
	}
	PathMask(branch);
	vector<SpacePoint_t> temp;//
	int size = branch.size();
	if(size > 2)
	{
		for(int i = 0; i <size ; i++)
		{
		 	point = branch.at(i);
			m_piDFS[point.m_z * m_iImgSize + point.m_y * m_iImgWidth + point.m_x] = LINETAG;
			--point.m_z;
			temp.push_back(point);
 		}
		branch.clear();
		branch = temp;
		m_vvsptCenterpaths.push_back(branch);
	}
	//	PathMask(branch);
	//	for(int i = 0; i < branch.size();i++)
	//	{
	//		z= branch.at(i).m_z;
	//		x= branch.at(i).m_x;
	//		y= branch.at(i).m_y;
	//		index_nei = m_iImgSize * z + m_iImgWidth * y + x;
	//		m_piMskDFS[index_nei]=255;
	//		
	//	}
	
	printf("num=%ld\n",num);
	
	if(!single )
	{
		return branch;
		
	}
	else 
	{
		branch.clear();
		return branch;
		
	}
	///return branch;
	
}

void NeuronPlugin::NueronTree_Ray_D(SpacePoint_t point,int n)
{
	
	//	Set_Seed(point);
	//    if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
	//	Set_local_DFS();
	
	vector<SpacePoint_t> Branchpoint;
	vector<SpacePoint_t> centerpath;
	vector<SpacePoint_t> Rcenterpath;
	vector<SpacePoint_t> Pcenterpath;
	Branchpoint.clear();
	centerpath.clear();
	Rcenterpath.clear();
	Pcenterpath.clear();
	V3DLONG num=0;
	V3DLONG num1=0;
	V3DLONG nodeid = 0;
	
	int direc = Getmaxderection(point);
	
	//	if(point.ray_value[direc]==0)
	//	{
	//		return;
	//	}
	
	printf("direcNext=%ld\n",direc);
	
	SpacePoint_t Npoint = Coumpere_Next_Point(point,direc,point.r);
	
	//	if(m_ppsImgData[Npoint.m_z][m_iImgWidth * Npoint.m_y + Npoint.m_x] == BACKGROUND)
	//	{
	//		return ;
	//	}
	//Rcenterpath = ExtractCenterpath_Ray_D(point,Npoint);
	
	centerpath = ExtractCenterpath_Ray_D(point,Npoint); 
	
	bool mask = PathMask(centerpath);
	
	if (centerpath.size()==0) 
	{
		return;
	}
	
	V3DLONG x,y,z;
	
	double rr;
	
	SpacePoint_t tp;
	
	//	for(V3DLONG ii = 0; ii<Rcenterpath.size();ii++)
	//	{
	//		tp.m_x = Rcenterpath.at(ii).m_x;
	//		tp.m_y = Rcenterpath.at(ii).m_y;
	//		tp.m_z = Rcenterpath.at(ii).m_z;
	//		tp.r = Rcenterpath.at(ii).r;
	//		if(m_ppsImgData[tp.m_z][m_iImgWidth * tp.m_y + tp.m_x] != BACKGROUND)
	//			Pcenterpath.push_back(tp);
	//	}
	//	for(V3DLONG ii = 0; ii<centerpath.size();ii++)
	//	{
	//		tp.m_x = centerpath.at(ii).m_x;
	//		tp.m_y = centerpath.at(ii).m_y;
	//		tp.m_z = centerpath.at(ii).m_z;
	//		tp.r = centerpath.at(ii).r;
	//		if(m_ppsImgData[tp.m_z][m_iImgWidth * tp.m_y + tp.m_x] != BACKGROUND)
	//			Pcenterpath.push_back(tp);
	//	}
	//centerpath = Rivise_centerpath(Pcenterpath);
	
    PathMask(centerpath);
	
	if(centerpath.size()>2)//4
	{  
		centerpathall.push_back(centerpath);
		
		QString file;
		file.sprintf("%d",n);
		QString filepath = "/Users/yangj13/work/v3d_internal/v3d_2.0/yang_jinzhu/";
		QString curImageName1 = filepath + file + ".swc";
		//SaveSwcfile(centerpath,curImageName1);
		printf("CCCCCCCCCCCCCCCCCCCCCCCCCCCC");
		
		ImageMarker S;
		S.x = point.m_x;
		S.y = point.m_y;
		S.z = point.m_z;
		S.on = true;
		listMarker.append(S);
		//for(V3DLONG jj = 0; jj<centerpathall.size();jj++)
		//		{
		//			num += centerpathall.at(jj).size(); 
		//		}
		//		
		//		SaveSwcTree(centerpath,node_id,num);//first save swc file
		//		
		//		PathMask(centerpath);
		
		//PathMask();//
		
		for(V3DLONG jj = 0; jj<centerpathall.size();jj++)
		{
			num = num + centerpathall.at(jj).size(); 
		}
		
		if(point.seg_id == 1)
		{
			centerpath.insert(centerpath.begin(),point);
			
			SaveSwcTree_cast(centerpath,n,num);
			
			centerpath.erase(centerpath.begin());
			
			centerpathall.push_back(centerpath);
		}else
		{
			for(V3DLONG ii = 0; ii<point.seg_id-1; ii++)
			{
				nodeid += centerpathall.at(ii).size();
			}
			centerpath.insert(centerpath.begin(),point);
			
			printf("branchnum++++++++++++++++++++++++++=%ld n=%ld nodeid=%ld\n",num,n,nodeid);
			
			printf("xxx=%ld yyyy=%ld zzzz=%ld\n",point.m_x,point.m_y,point.m_z);
			
			SaveSwcTree_cast(centerpath,n+nodeid,num);
			
			centerpath.erase(centerpath.begin());
			
			centerpathall.push_back(centerpath);
		}
		
		
		Branchpoint = BranchDetect2(centerpath);
		
		if(Branchpoint.size()>0)
		{
			for(V3DLONG ii = 0 ; ii< Branchpoint.size(); ii++)
				//for(V3DLONG ii = Branchpoint.size()-1 ; ii>=0; ii--)
			{
				m_Markerpoint.push_back(Branchpoint.at(ii));
				//				Set_Seed(Branchpoint.at(ii));
				//				if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
				//				Set_local_DFS();
				//NueronTree(Branchpoint.at(ii),(Branchpoint.at(ii).node_id+centerpath.size()));
				NueronTree_Ray_D(Branchpoint.at(ii),(Branchpoint.at(ii).node_id));
			}
		}else 
		{
			return;
		}
		
		//centerpathall.push_back(centerpath);
	}
}

SpacePoint_t NeuronPlugin::Coumpere_ball_center(SpacePoint_t points, int direct, double r)
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	V3DLONG x = points.m_x;
	V3DLONG y = points.m_y;
	V3DLONG z = points.m_z;
	V3DLONG x1=0; 
	V3DLONG y1=0; 
	V3DLONG z1=0;
	SpacePoint_t center;
	double rr = 0;
	r = 1.5*r;
	switch(direct)
	{
		case 0:
			x1 = x +nDx[0];
			y1 = y +nDy[0];
			z1 = z +nDz[0];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255) 
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[0];
				y1 = y1 +nDy[0];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 1:
			x1 = x +nDx[1];
			y1 = y +nDy[1];
			z1 = z +nDz[0];
	        rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[1];
				y1 = y1 +nDy[1];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}			
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
			
		case 2:
			x1 = x +nDx[2];
			y1 = y +nDy[2];
			z1 = z +nDz[0];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[2];
				y1 = y1 +nDy[2];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
			
		case 3:
			x1 = x +nDx[3];
			y1 = y +nDy[3];
			z1 = z +nDz[0];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[3];
				y1 = y1 +nDy[3];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 4:
			x1 = x +nDx[4];
			y1 = y +nDy[4];
			z1 = z +nDz[0];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r ) 
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[4];
				y1 = y1 +nDy[4];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				//printf("x1=%ld y1=%ld z1=%ld x=%ld y=%ld z=%ld \n",x1,y1,z1,x,y,z);
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 5:
			x1 = x +nDx[5];
			y1 = y +nDy[5];
			z1 = z +nDz[0];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[5];
				y1 = y1 +nDy[5];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 6:
			x1 = x +nDx[6];
			y1 = y +nDy[6];
			z1 = z +nDz[0];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[6];
				y1 = y1 +nDy[6];
				z1 = z1 +nDz[0];
                rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 7:
			x1 = x +nDx[7];
			y1 = y +nDy[7];
			z1 = z +nDz[0];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[7];
				y1 = y1 +nDy[7];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 8:
			x1 = x +nDx[8];
			y1 = y +nDy[8];
			z1 = z +nDz[0];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[8];
				y1 = y1 +nDy[8];
				z1 = z1 +nDz[0];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 9:
			x1 = x +nDx[0];
			y1 = y +nDy[0];
			z1 = z +nDz[1];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[0];
				y1 = y1 +nDy[0];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 10:
			x1 = x +nDx[1];
			y1 = y +nDy[1];
			z1 = z +nDz[1];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[1];
				y1 = y1 +nDy[1];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 11:
			x1 = x +nDx[2];
			y1 = y +nDy[2];
			z1 = z +nDz[1];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[2];
				y1 = y1 +nDy[2];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 12:
			x1 = x +nDx[3];
			y1 = y +nDy[3];
			z1 = z +nDz[1];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[3];
				y1 = y1 +nDy[3];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 13:
			x1 = x +nDx[4];
			y1 = y +nDy[4];
			z1 = z +nDz[1];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[4];
				y1 = y1 +nDy[4];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 14:
			x1 = x +nDx[5];
			y1 = y +nDy[5];
			z1 = z +nDz[1];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[5];
				y1 = y1 +nDy[5];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 15:
			x1 = x +nDx[6];
			y1 = y +nDy[6];
			z1 = z +nDz[1];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[6];
				y1 = y1 +nDy[6];
				z1 = z1 +nDz[1];
				
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 16:
			x1 = x +nDx[7];
			y1 = y +nDy[7];
			z1 = z +nDz[1];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[7];
				y1 = y1 +nDy[7];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 17:
			x1 = x +nDx[8];
			y1 = y +nDy[8];
			z1 = z +nDz[1];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[8];
				y1 = y1 +nDy[8];
				z1 = z1 +nDz[1];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;				
		case 18:
			x1 = x +nDx[0];
			y1 = y +nDy[0];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[0];
				y1 = y1 +nDy[0];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 19:
			x1 = x +nDx[1];
			y1 = y +nDy[1];
			z1 = z +nDz[2];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[1];
				y1 = y1 +nDy[1];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 20:
			x1 = x +nDx[2];
			y1 = y +nDy[2];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[2];
				y1 = y1 +nDy[2];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 21:
			x1 = x +nDx[3];
			y1 = y +nDy[3];
			z1 = z +nDz[2];
		    rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[3];
				y1 = y1 +nDy[3];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 22:
			x1 = x +nDx[4];
			y1 = y +nDy[4];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[4];
				y1 = y1 +nDy[4];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 23:
			x1 = x +nDx[5];
			y1 = y +nDy[5];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[5];
				y1 = y1 +nDy[5];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 24:
			x1 = x +nDx[6];
			y1 = y +nDy[6];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[6];
				y1 = y1 +nDy[6];
				z1 = z1 +nDz[2];
				
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 25:
			x1 = x +nDx[7];
			y1 = y +nDy[7];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				
				x1 = x1 +nDx[7];
				y1 = y1 +nDy[7];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				
			}
			
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		case 26:
			x1 = x +nDx[8];
			y1 = y +nDy[8];
			z1 = z +nDz[2];
			rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			while (rr < r && m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]!=255)
			{
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				x1 = x1 +nDx[8];
				y1 = y1 +nDy[8];
				z1 = z1 +nDz[2];
				rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
			}
			center.m_x = x1;
			center.m_y = y1;
			center.m_z = z1;
			break;
		default:
			break;
	}
	
	if (center.m_z >= m_iImgCount)
	{
		printf("centerz=%ld\n",center.m_z);
		printf("centerzzzz>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"); 
		center.m_z = m_iImgCount ;
		
	}
	if (center.m_y >=m_iImgHeight)
	{
		printf("centery=%ld\n",center.m_y);
		printf("centeryyyy>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		center.m_y = m_iImgHeight;
	}
	if (center.m_x >= m_iImgWidth)
	{
		printf("centerx=%ld\n",center.m_x);
		printf("centerxxxxx>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		center.m_x = m_iImgWidth;
	}
	if (center.m_z <= 0 )
	{
		printf("centerz=%ld\n",center.m_z);
		printf("centerz<<<<<<<<<<<<<<<<<<<<<<000\n");
		center.m_z=0;
	}
	if (center.m_y <= 0 )
	{
		printf("centery=%ld\n",center.m_y);
		printf("centery<<<<<<<<<<<<<0000\n");
		center.m_y=0;}
	if (center.m_x <= 0) 
	{
		printf("centerx=%ld\n",center.m_x);
		printf("centerx<<<<<<<<<<<<<<<<<<<<<0\n");
		center.m_x=0;
	}
	if(m_ppsMaskData[z1*m_iImgWidth*m_iImgHeight + y1*m_iImgWidth + x1]==255)
	{
		printf("center25555555555555555555555555555555555\n");
	}
	
	return center;
	
}

SpacePoint_t NeuronPlugin::Coumpere_Next_Point(SpacePoint_t points, int direct, double r)
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG x = points.m_x;
	V3DLONG y = points.m_y;
	V3DLONG z = points.m_z;
	V3DLONG x1=0; 
	V3DLONG y1=0; 
	V3DLONG z1=0;
	SpacePoint_t center;
	double rr = 0;
	x1=x;
	y1=y;
	z1=z;
	switch(direct)
	{
		case 0:
			x1 = x +nDx[0];
			y1 = y +nDy[0];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND) 
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				
				//rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[0];
				y1 = y1 +nDy[0];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 1:
			x1 = x +nDx[1];
			y1 = y +nDy[1];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[1];
				y1 = y1 +nDy[1];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 2:
			x1 = x +nDx[2];
			y1 = y +nDy[2];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[2];
				y1 = y1 +nDy[2];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 3:
			x1 = x +nDx[3];
			y1 = y +nDy[3];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[3];
				y1 = y1 +nDy[3];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 4:
			x1 = x +nDx[4];
			y1 = y +nDy[4];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[4];
				y1 = y1 +nDy[4];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
				//printf("x1=%ld y1=%ld z1=%ld x=%ld y=%ld z=%ld \n",x1,y1,z1,x,y,z);
			}
			break;
		case 5:
			x1 = x +nDx[5];
			y1 = y +nDy[5];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[5];
				y1 = y1 +nDy[5];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 6:
			x1 = x +nDx[6];
			y1 = y +nDy[6];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[6];
				y1 = y1 +nDy[6];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 7:
			x1 = x +nDx[7];
			y1 = y +nDy[7];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[7];
				y1 = y1 +nDy[7];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 8:
			x1 = x +nDx[8];
			y1 = y +nDy[8];
			z1 = z +nDz[0];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[8];
				y1 = y1 +nDy[8];
				z1 = z1 +nDz[0];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 9:
			x1 = x +nDx[0];
			y1 = y +nDy[0];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[0];
				y1 = y1 +nDy[0];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 10:
			x1 = x +nDx[1];
			y1 = y +nDy[1];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[1];
				y1 = y1 +nDy[1];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 11:
			x1 = x +nDx[2];
			y1 = y +nDy[2];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[2];
				y1 = y1 +nDy[2];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 12:
			x1 = x +nDx[3];
			y1 = y +nDy[3];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[3];
				y1 = y1 +nDy[3];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 13:
			x1 = x +nDx[4];
			y1 = y +nDy[4];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[4];
				y1 = y1 +nDy[4];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 14:
			x1 = x +nDx[5];
			y1 = y +nDy[5];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[5];
				y1 = y1 +nDy[5];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 15:
			x1 = x +nDx[6];
			y1 = y +nDy[6];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[6];
				y1 = y1 +nDy[6];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 16:
			x1 = x +nDx[7];
			y1 = y +nDy[7];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[7];
				y1 = y1 +nDy[7];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 17:
			x1 = x +nDx[8];
			y1 = y +nDy[8];
			z1 = z +nDz[1];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[8];
				y1 = y1 +nDy[8];
				z1 = z1 +nDz[1];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;				
		case 18:
			x1 = x +nDx[0];
			y1 = y +nDy[0];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//		rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[0];
				y1 = y1 +nDy[0];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 19:
			x1 = x +nDx[1];
			y1 = y +nDy[1];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[1];
				y1 = y1 +nDy[1];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 20:
			x1 = x +nDx[2];
			y1 = y +nDy[2];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[2];
				y1 = y1 +nDy[2];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 21:
			x1 = x +nDx[3];
			y1 = y +nDy[3];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[3];
				y1 = y1 +nDy[3];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 22:
			x1 = x +nDx[4];
			y1 = y +nDy[4];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//		rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[4];
				y1 = y1 +nDy[4];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 23:
			x1 = x +nDx[5];
			y1 = y +nDy[5];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[5];
				y1 = y1 +nDy[5];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 24:
			x1 = x +nDx[6];
			y1 = y +nDy[6];
			z1 = z +nDz[2];
			
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[6];
				y1 = y1 +nDy[6];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 25:
			x1 = x +nDx[7];
			y1 = y +nDy[7];
			z1 = z +nDz[2];
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[7];
				y1 = y1 +nDy[7];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		case 26:
			x1 = x +nDx[8];
			y1 = y +nDy[8];
			z1 = z +nDz[2];
			while (m_ppsMaskData[z1 * m_iImgSize + m_iImgWidth * y1 + x1] ==255 && m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
			{
				if (z1 > m_iImgCount-2){z1=m_iImgCount-2;}
				if (y1 > m_iImgHeight){y1=m_iImgHeight;}
				if (x1 > m_iImgWidth) {x1=m_iImgWidth;}
				if (z1 < 0 ){z=0;}
				if (y1 < 0 ){y=0;}
				if (x1 < 0) {x=0;}
				//	rr = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y) + (z1-z)*(z1-z));
				x1 = x1 +nDx[8];
				y1 = y1 +nDy[8];
				z1 = z1 +nDz[2];
				center.m_x = x1;
				center.m_y = y1;
				center.m_z = z1;
			}
			break;
		default:
			break;
	}
	if (center.m_z > m_iImgCount-2){center.m_z = m_iImgCount-2;}
	if (center.m_y > m_iImgHeight){center.m_y = m_iImgHeight;}
	if (center.m_x > m_iImgWidth) {center.m_x = m_iImgWidth;}
	if (center.m_z < 0 ){center.m_z = 0;}
	if (center.m_y < 0 ){center.m_y = 0;}
	if (center.m_x < 0) {center.m_x = 0;}
	//	V3DLONG tx,ty,tz;
	//	V3DLONG b = m_piDFB[center.m_z * m_iImgSize + m_iImgWidth * center.m_y + center.m_x];
	//	for(int kk=-3;kk<4;kk++)
	//	{
	//		for(int m=-3;m<4;m++)
	//		{
	//			for(int n=-3; n<4; ++n)
	//			{
	//				tz = center.m_z + kk;
	//				ty = center.m_y + m;
	//				tx = center.m_x + n;
	//				int index = tz * m_iImgSize + m_iImgWidth * ty + tx;
	//				if((tx<0)||(ty<0)||(tx>m_iImgWidth)||(ty>m_iImgHeight)|| (tz > m_iImgCount))
	//					continue;
	//				if(m_ppsImgData[tz][m_iImgWidth * ty + tx] != BACKGROUND && m_piDFS[index]!=MASK )
	//				{
	//					if(m_piDFB[index]>b)
	//					{
	//						b =m_piDFB[index];
	//						center.m_x = tx;
	//						center.m_y = ty;
	//						center.m_z = tz;
	//					}
	//				}
	//			}
	//		}
	//	}
	
	return center;
	
}

SpacePoint_t NeuronPlugin::Raycasting2(SpacePoint_t startpoint)
{
	int x = 0;
	int y = 0;
	int z = 0;
	
	int i = 0;
	int j = 0;
	int k = 0;
	
	//direc_pointSpacedirec_point_t direc_point;
	
	int max_d = 0;
	int index = 0;
	int index_nei = 0;
    int ted = 0;
	bool end = false;
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	i = startpoint.m_x;
	j = startpoint.m_y;
	k = startpoint.m_z;
	
	//printf("x=%ld y=%ld z=%ld \n",i,j,k);
	
	index = m_iImgSize * k + m_iImgWidth * j + i;
	
	SpacePoint_t direc_point;
	int count = 0;
	
	for(int m = 0; m < 3;  ++m)
	{
		for(int n = 0; n < 9; ++n)
		{
			z = k + nDz[m];
			y = j + nDy[n];
			x = i + nDx[n];
			direc_point.ray_value[count]=0;
			count++;
			if(m == 1 && n==4)
			{
				direc_point.ray_value[count-1]=0;
				direc_point.m_x = startpoint.m_x;
				direc_point.m_y = startpoint.m_y;
				direc_point.m_z = startpoint.m_z;
				int  direct = count-1;
				//printf("direct=%ld ray_value=%ld\n",direct,direc_point.ray_value[count-1]);
			}
			else
			{
				//				z = z+nDz[m];
				//				y = y+nDy[n];
				//				x = x+nDx[n];
				for(int s = 0; s<50; s++)
				{
					z = z+nDz[m];
					y = y+nDy[n];
					x = x+nDx[n];
					if (z > m_iImgCount-2){s=51;continue;}
					if (y > m_iImgHeight){s=51;continue;}
					if (x > m_iImgWidth) {s=51;continue;}
					if (z < 0 ){s=51; continue;}
					if (y < 0 ){s=51; continue;}
					if (x < 0 ){s=51; continue;}
					if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
					{
						if(m_ppsMaskData[z * m_iImgSize + m_iImgWidth * y + x] != 255)
						{
							direc_point.ray_value[count-1]++;
							
						}
						
					}else 
					{
						s=51;
					}
					
					
					//					if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND && m_ppsMaskData[z * m_iImgSize + m_iImgWidth * y + x] != 255 )
					//					{
					//						//z = z+nDz[m];
					//						//y = y+nDy[n];
					//						//x = x+nDx[n];
					//						direc_point.ray_value[count-1]++;
					//						//m_ppsMaskBranch[z * m_iImgSize + m_iImgWidth * y + x]=255;
					//					}
				}
				
			}
			//printf("x=%ld y=%ld z=%ld \n",x,y,z);
		}
		
	}
	direc_point.m_x = startpoint.m_x;
	direc_point.m_y = startpoint.m_y;
	direc_point.m_z = startpoint.m_z;
	direc_point.r = startpoint.r;
	return direc_point;
	//	branch.push_back(direc_point);
}

bool NeuronPlugin::PathMask(vector<SpacePoint_t> centerpath)
{
	V3DLONG x,y,z;
	double ir;
	bool mask = false;
	for(V3DLONG jj = 0; jj < centerpath.size(); jj++)
	{
		x = centerpath.at(jj).m_x;
		y = centerpath.at(jj).m_y;
		z = centerpath.at(jj).m_z;
		ir = centerpath.at(jj).r;
		
		for (V3DLONG cz= z-ir; cz <= z+ir; cz++)
		{
			for (V3DLONG cy= y-ir; cy <= y+ir; cy++)
			{
				for (V3DLONG cx= x-ir; cx <= x+ir; cx++)
				{
					if (cz > m_iImgCount-2 ){continue;}
					if (cy > m_iImgHeight){continue;}
					if (cx > m_iImgWidth) {continue;}
					if (cz < 0 ){continue;cz = 0;}
					if (cy < 0 ){continue;cy = 0;}
					if (cx < 0) {continue;cx = 0;}
					m_ppsMaskData[cz*m_iImgWidth*m_iImgHeight + cy*m_iImgWidth + cx]=255;
					//if(m_ppsMaskData[cz*m_iImgWidth*m_iImgHeight + cy*m_iImgWidth + cx]==255)
					//					{
					//						mask = true;
					//						
					//					}else 
					//					{
					//						m_ppsMaskData[cz*m_iImgWidth*m_iImgHeight + cy*m_iImgWidth + cx]=255;
					//						
					//					}
				}
			}
		}
	}
	return mask;
	
	//	for (V3DLONG cz= 0; cz <m_iImgCount; cz++)
	//	{
	//		for (V3DLONG cy= 0; cy <m_iImgHeight; cy++)
	//		{
	//			for (V3DLONG cx= 0; cx < m_iImgWidth; cx++)
	//			{
	//	
	//				if(m_ppsMaskData[cz*m_iImgWidth*m_iImgHeight + cy*m_iImgWidth + cx]==255)
	//				{
	//					m_piDFB[cz*m_iImgWidth*m_iImgHeight + cy*m_iImgWidth + cx]=MASK;
	//					m_piDFS[cz*m_iImgWidth*m_iImgHeight + cy*m_iImgWidth + cx]=MASK;
	//					
	//				}
	//				
	//			}
	//		}
	//	}
	
}
SpacePoint_t NeuronPlugin::GetCenterPiont(SpacePoint_t startpoint,int direction)
{
	int x = 0;
	int y = 0;
	int z = 0;
	
	int i = 0;
	int j = 0;
	int k = 0;
	
	SpacePoint_t point;
	point.m_x = startpoint.m_x;
	point.m_y = startpoint.m_y;
	point.m_z = startpoint.m_z;
	point.sd = 0;
	
	int max_d = 0;
	int index = 0;
	int index_nei = 0;
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	k = point.m_z;
	j = point.m_y;
	i = point.m_x;
	index = m_iImgSize * i + m_iImgWidth * j + k;
	max_d = 0;	
	
	//	for(int m = 0; m < 3;  ++m)
	//	{
	//		for(int n = 0; n < 9; ++n)
	//		{
	//			z = k + nDz[m];
	//			y = j + nDy[n];
	//			x = i + nDx[n];	
	//			if (z >= m_iImgCount-2){z = m_iImgCount-2;}
	//			if (y > m_iImgHeight){y = m_iImgHeight;}
	//			if (x > m_iImgWidth) {x = m_iImgWidth;}
	//			if (z < 0 ) {z = 0;}
	//			if ( y < 0 ){y = 0;}
	//			if ( x < 0) {x = 0;}
	//			if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND )//&& m_piDFS[index_nei] >m_piDFS[index] )//&& m_ppsMaskData[z * m_iImgSize + m_iImgWidth * y + x] == 0)
	//			{
	//				index_nei = m_iImgSize * z + m_iImgWidth * y + x;
	//				
	//				if(m_piDFS[index_nei] > m_piDFS[index])
	//				{
	//					if(m_piDFB[index_nei] >max_d)// && m_piDFS[index_nei] > m_piDFS[index])
	//					{
	//						max_d = m_piDFB[index_nei];
	//						point.m_x = x;
	//						point.m_y = y;
	//						point.m_z = z;
	//						//point.r = max_d;
	//						point.mark = 1;
	//						point.sd = m_piDFB[index_nei];
	//						//printf("tx=%ld ty=%ld tz=%ld \n",point.m_x,point.m_y,point.m_z);
	//						//printf("maxd=%ld \n",max_d );
	//					}	
	//				}
	//			}	
	//		}
	//	}
	
	for(int m = 0; m < 3;  ++m)
	{
		for(int n = 0; n < 9; ++n)
		{
			z = k + nDz[m];
			y = j + nDy[n];
			x = i + nDx[n];	
			if (z >= m_iImgCount-2){z = m_iImgCount-2;}
			if (y > m_iImgHeight){y = m_iImgHeight;}
			if (x > m_iImgWidth) {x = m_iImgWidth;}
			if (z < 0 ) {z = 0;}
			if ( y < 0 ){y = 0;}
			if ( x < 0) {x = 0;}
			
			index_nei = m_iImgSize * z + m_iImgWidth * y + x;
			if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND && m_ppsMaskData[index_nei] != 255)// && m_piDFS[index_nei] >m_piDFS[index])
			{
				if(m_piDFB[index_nei] >max_d)
				{
					max_d = m_piDFB[index_nei];
					point.m_x = x;
					point.m_y = y;
					point.m_z = z;
					point.sd = max_d;
					point.mark = 1;
					//printf("tx=%ld ty=%ld tz=%ld \n",point.m_x,point.m_y,point.m_z);
				}
				
			}	
			//	else {
			//				continue;
			//			}
			
		}
	}
	return point;
	
}
int NeuronPlugin::Getmaxderection(SpacePoint_t startpoint)
{
	int temp = 0;
	int direc = 0;
	for(int i = 0; i < 26; i++)
	{
		if(startpoint.ray_value[i]>temp)
		{
			temp=startpoint.ray_value[i];
			direc = i;
		}
	}
	return direc;
}
double NeuronPlugin::fitRadiusPercent_cast(V3DLONG **img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
										   float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitRadiusPercent() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset
	
	//printf("x=%lf y=%lf z=%lf\n",x,y,z);
	double max_r = dim0/2;
	if (max_r > dim1/2) max_r = dim1/2;
	if (!b_est_in_xyplaneonly)
	{
		if (max_r > (dim2*zthickness)/2) max_r = (dim2*zthickness)/2;
	}
	//max_r = bound_r;
	
	double total_num, background_num;
	double ir;
	for (ir=1; ir<=max_r; ir++)
	{
		total_num = background_num = 0;
		
		double dz, dy, dx;
		double zlower = -ir/zthickness, zupper = +ir/zthickness;
		if (b_est_in_xyplaneonly)
			zlower = zupper = 0;
		for (dz= zlower; dz <= zupper; ++dz)
			for (dy= -ir; dy <= +ir; ++dy)
				for (dx= -ir; dx <= +ir; ++dx)
				{
					total_num++;
					
					double r = sqrt(dx*dx + dy*dy + dz*dz);
					if (r>ir-1 && r<=ir)
					{
						V3DLONG i = x+dx;	if (i<0 || i>=dim0) goto end;
						V3DLONG j = y+dy;	if (j<0 || j>=dim1) goto end;
						V3DLONG k = z+dz;	if (k<0 || k>=dim2) goto end;
						
						//m_ppsMaskData[k*m_iImgWidth*m_iImgHeight+j*m_iImgWidth+i]=255;					
						
						if (img3d[k][j*m_iImgWidth+i] < imgTH)
						{
							background_num++;
							
							if ((background_num/total_num) > 0.009)	goto end; //change 0.01 to 0.001 on 100104// 0.009
						}
					}
				}
	}
end:
	//m_ppsMaskData[(V3DLONG)z*m_iImgWidth*m_iImgHeight + (V3DLONG)y*m_iImgWidth + (V3DLONG)x]=255;
	for (V3DLONG cz= z-ir; cz <= z+ir; cz++)
	{
		for (V3DLONG cy= y-ir; cy <= y+ir; cy++)
		{
			for (V3DLONG cx= x-ir; cx <= x+ir; cx++)
			{
				if (cz >= m_iImgCount ){continue;}
				if (cy >= m_iImgHeight){continue;}
				if (cx >= m_iImgWidth) {continue;}
				if (cz <= 0 ){continue;}
				if (cy <= 0 ){continue;}
				if (cx <= 0) {continue;}
				if(m_ppsMaskData[(V3DLONG)cz*m_iImgWidth*m_iImgHeight + (V3DLONG)cy*m_iImgWidth + (V3DLONG)cx]!=255 && m_ppsImgData[cz][m_iImgWidth * cy + cx] != BACKGROUND )
					//printf("cx=%ld cy = %ld cz=%ld \n",cx,cy,cz);
					m_ppsMaskData[(V3DLONG)cz*m_iImgWidth*m_iImgHeight + (V3DLONG)cy*m_iImgWidth + (V3DLONG)cx]=255;
				
			}
		}
	}
	return ir;
}
double NeuronPlugin::fitRadiusPercent(V3DLONG **img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
									  float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitRadiusPercent() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset
	
	//printf("x=%lf y=%lf z=%lf\n",x,y,z);
	double max_r = dim0/2;
	if (max_r > dim1/2) max_r = dim1/2;
	if (!b_est_in_xyplaneonly)
	{
		if (max_r > (dim2*zthickness)/2) max_r = (dim2*zthickness)/2;
	}
	//max_r = bound_r;
	
	double total_num, background_num;
	double ir;
	for (ir=1; ir<=max_r; ir++)
	{
		total_num = background_num = 0;
		
		double dz, dy, dx;
		double zlower = -ir/zthickness, zupper = +ir/zthickness;
		if (b_est_in_xyplaneonly)
			zlower = zupper = 0;
		for (dz= zlower; dz <= zupper; ++dz)
			for (dy= -ir; dy <= +ir; ++dy)
				for (dx= -ir; dx <= +ir; ++dx)
				{
					total_num++;
					
					double r = sqrt(dx*dx + dy*dy + dz*dz);
					if (r>ir-1 && r<=ir)
					{
						V3DLONG i = x+dx;	if (i<0 || i>=dim0) goto end;
						V3DLONG j = y+dy;	if (j<0 || j>=dim1) goto end;
						V3DLONG k = z+dz;	if (k<0 || k>=dim2) goto end;
						
						//m_ppsMaskData[k*m_iImgWidth*m_iImgHeight+j*m_iImgWidth+i]=255;					
						
						if (img3d[k][j*m_iImgWidth+i] < imgTH)
						{
							background_num++;
							
							if ((background_num/total_num) > 0.009)	goto end; //change 0.01 to 0.001 on 100104// 0.009
						}
					}
				}
	}
end:
	//	m_ppsMaskData[(V3DLONG)z*m_iImgWidth*m_iImgHeight + (V3DLONG)y*m_iImgWidth + (V3DLONG)x]=255;
	//	for (V3DLONG cz= z-ir; cz <= z+ir; cz++)
	//	{
	//		for (V3DLONG cy= y-ir; cy <= y+ir; cy++)
	//		{
	//			for (V3DLONG cx= x-ir; cx <= x+ir; cx++)
	//			{
	//				if (cz >= m_iImgCount ){continue;cz = m_iImgCount;}
	//				if (cy >= m_iImgHeight){continue;cy = m_iImgHeight;}
	//				if (cx >= m_iImgWidth) {continue;cx = m_iImgWidth;}
	//				if (cz <= 0 ){cz = 0;}
	//				if (cy <= 0 ){cy = 0;}
	//				if (cx <= 0) {cx = 0;}
	//				if(m_ppsMaskData[(V3DLONG)cz*m_iImgWidth*m_iImgHeight + (V3DLONG)cy*m_iImgWidth + (V3DLONG)cx]!=255 && m_ppsImgData[cz][m_iImgWidth * cy + cx] != BACKGROUND )
	//				//printf("cx=%ld cy = %ld cz=%ld \n",cx,cy,cz);
	//				m_ppsMaskData[(V3DLONG)cz*m_iImgWidth*m_iImgHeight + (V3DLONG)cy*m_iImgWidth + (V3DLONG)cx]=255;
	//
	//			}
	//		}
	//	}
	return ir;
}
