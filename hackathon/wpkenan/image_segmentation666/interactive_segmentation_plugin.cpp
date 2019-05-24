 #include "v3d_message.h"
#include <vector>
#include <map>
#include <iostream>
#include <QString>
#include <math.h>
#include "interactive_segmentation_plugin.h"
#include "NeuronEnhancementFilter.h"
#include "NeuronSegmentation.h"

using namespace std;

Q_EXPORT_PLUGIN2(image_segmentation,image_segmentation);
 
QStringList image_segmentation::menulist() const
{
	return QStringList() 
		<<tr("interactive_segmentation")
		<<tr("about");
}

QStringList image_segmentation::funclist() const
{
    return QStringList()
        <<tr("interactive_segmentation")
		<<tr("about");
}


void image_segmentation::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("interactive_segmentation"))
	{
        image_IO(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will show mean distance"));
	}
}

template <class T1, class T2> bool assign_val(T1 *dst, T2 *src, V3DLONG total_num)
{/*
    if (!dst || !src || total_num<=0) return false;

    for (V3DLONG i=0; i<total_num; i++){
        if(src[i]>30)
            dst[i] = src[i];
        else
            dst[i] = 0;
    }
    return true;*/
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
	cout << "*********************in_seg**************************" << endl;
	cout << __LINE__ << endl;
	cout << !inputData << !outputData << (sx<=0) << (sy<=0) << (sz<=0) << endl;
    bool res=false;
    if (!inputData || !outputData || sx<=0 || sy<=0 || sz<=0)
	{
		cout << __LINE__ << endl;
		return res;
	}
		//cout << !inputData << !outputData << (sx<=0) << (sy<=0) << (sz<=0) << endl;
		
	cout << "out" << endl;
	cout << __LINE__ << endl;

    short *internal_input = 0;
    V3DLONG channel_sz = sx*sy*sz;
	cout << __LINE__ << endl;
    try
    {
		cout << __LINE__ << endl;
        internal_input = new short[channel_sz];
		cout << __LINE__ << endl;
    }
    catch (...)
    {
		cout << __LINE__ << endl;
        v3d_msg("Fail to allocate memory in do_seg().");
        res = false;
        goto Label_exit_do_seg;
    }

    assign_val(internal_input, inputData, channel_sz);

    //cout<<"####################\n"<<endl;

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

bool image_segmentation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("gmm_classification"))
    {
        //gmm();

    }
	
	else return false;

	return true;
}


void image_IO(V3DPluginCallback2 &callback, QWidget *parent){
	cout << "hell" << endl;
	v3dhandle curwin = callback.currentImageWindow();
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, "", QObject::tr("No image is open."));
		return;
	}
    
    unsigned char * inimg1d = subject->getRawData(); 
    V3DLONG in_sz[4]; 
    in_sz[0] = subject->getXDim(); in_sz[1] = subject->getYDim(); in_sz[2] = subject->getZDim(); in_sz[3] = subject->getCDim();
    int datatype = subject->getDatatype();
	
    V3DLONG channel_sz = in_sz[0]*in_sz[1]*in_sz[2];

    //cout<<"channel sz="<<channel_sz<<endl;// okay
    V3DLONG outsz[4];
    
    //segmentation
    
    int iVesCnt = 1,i;
    bool res;
	unsigned char * m_OutImgData = 0;
	unsigned char * m_OutImgData2 = 0;
    
	try
	{
		m_OutImgData = new  unsigned char[channel_sz];
		m_OutImgData2 = new  unsigned char[channel_sz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
	}
	
	bool b_binarization = true;

    cout<<"datatype=\n"<<datatype<<endl;//=1 okay

    switch (datatype)
    {
        case 1:
            cout<<"-------------------\n"<<endl;
            res = do_seg(inimg1d, m_OutImgData,in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
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
        cerr<<"The segmentation of foreground fails.-------------"<<endl;
        if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
    }	
    
	////////////////////////////////////////
	for(int i=0;i<channel_sz;i++){
		m_OutImgData2[i] = inimg1d[i] + m_OutImgData[i];
	}
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


bool do_seg(short *pData, V3DLONG sx, V3DLONG sy, V3DLONG sz, int & iVesCnt, bool b_binarization=true)
{


    if (!pData || sx<=0 || sy<=0 || sz<=0)
        return false;
    
	int iSigma[1] = {1};
	
	//vesselness
	float fA = 0.5;
	float fB = 0.5;
    float fC = 1.0*10*5;

    cout<<"fA=\n"<<fA<<endl;
	
	//volume filter
	bool bVolFilter = true;
    

    //cout<<"+++++++++++++++++++++++++++\n"<<endl;
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
