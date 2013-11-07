/* channel_split_plugin.cpp
 * 2012-04-11 Hanchuan Peng
 */


#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <iostream>

#include "v3d_message.h"
#include "stackutil.h"

#include "channel_split_plugin.h"

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(channelsplit, ChannelSplitPlugin)


void processImage(V3DPluginCallback2 &callback, QWidget *parent, const QString & menu_name);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
template <class T> bool extract_a_channel(T* data1d, V3DLONG *sz, V3DLONG c, void * &outimg);

const QString title = QObject::tr("Channel Split Plugin");
QStringList ChannelSplitPlugin::menulist() const
{
    return QStringList() 
    << tr("Split Channels") 
    << tr("Extract One Channel") 
    << tr("About");
}

void ChannelSplitPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Split Channels") || menu_name == tr("Extract One Channel"))
	{
		processImage(callback,parent, menu_name);
	}
    else if (menu_name == tr("About"))
    {
         v3d_msg("This plugin is developed by Hanchuan Peng as a simple example for splitting or extracting color channels of an image.");
    }
}

QStringList ChannelSplitPlugin::funclist() const
{
	return QStringList()
    <<tr("split_extract_channels")
    <<tr("help");
}


bool ChannelSplitPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (func_name == tr("split_extract_channels"))
	{
        return processImage(callback, input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage 1: vaa3d -x channelsplit -f split_extract_channels -i <inimg_file> -o <outimg_file> -p <channel>"<<endl;
		cout<<"channel                  the input channel value, default -1 (all channels) and start from 0 (first channel)"<<endl;
		cout<<"This plugin is developed by Hanchuan Peng as a simple example for splitting or extracting color channels of an image."<<endl;
		cout<<endl;
		return true;
	}
}

bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	if(input.size() < 1 || output.size() != 1) 
        return false;

	V3DLONG c = -1;
    if (input.size()>=2)
    {
        vector<char*> paras = *(vector<char*> *)(input.at(1).p);
        if(paras.size() >= 1) c = atoi(paras.at(0));
    }
    if (c<0)
    {
        v3d_msg("You explicitly or implicitly specified a channel number < 0, which means split-all-channels. \n",0);
    }
        
	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"channel = "<<c<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

	bool b_res=false;
    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    void* outimg = 0;
    V3DLONG cb, ce, k;
    
	int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl; 
        return false;
    }
    
    if (c>=in_sz[3])
    {
        v3d_msg("The specified channel is not valid. Do nothing.", 0);
        goto Label_exit;
    }
    
    if (c<0) {cb=0; ce=in_sz[3]-1;}
    else {cb = ce = c;}
    
    for (k=cb; k<=ce; k++)
    {
        printf("\n... processing [%d] channel now...\n", k);
        switch(datatype)
        {
            case 1: b_res = extract_a_channel(data1d, in_sz, k, outimg); break;
            case 2: b_res = extract_a_channel((unsigned short int *)data1d, in_sz, k, outimg); break;
            case 4: b_res = extract_a_channel((float *)data1d, in_sz, k, outimg); break;
            default:
            b_res = false;
            v3d_msg("Right now this plugin supports only UINT8/UINT16/FLOAT32 data. Do nothing."); goto Label_exit;
        }

        // save image
        V3DLONG oldc = in_sz[3]; in_sz[3]=1;
        if (cb==ce)
            simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)outimg, in_sz, datatype);
        else
            simple_saveimage_wrapper(callback,
                                     qPrintable(QString("").setNum(k).prepend("_C").prepend(outimg_file).append(".v3draw")),
                                     (unsigned char *)outimg, in_sz, datatype);
        
        //
        in_sz[3] = oldc; //this sentence is important
        
        printf("b_res=[%s]\n", (b_res)?"true":"false");
    }

Label_exit:
     if (data1d) {delete []data1d; data1d=0;}
     //should never delete outimg here, as it is just a wrapper. by PHC 20131105
     return b_res;
}


void processImage(V3DPluginCallback2 &callback, QWidget *parent, const QString & menu_name)
{
    v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        v3d_msg("You don't have any image open in the main window.");
		return;
	}

    Image4DSimple* p4DImage = callback.getImage(curwin);
	if (!p4DImage)
	{
		v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}
    
    unsigned char* data1d = p4DImage->getRawData();

    V3DLONG sc = p4DImage->getCDim();

	//input
    bool b_res;
	bool ok1;
	V3DLONG c=-1;

    if (menu_name==QString("Split Channels"))
        c = -1;
    else if (menu_name==QString("Extract One Channel"))
    {
        if (sc>1) //only need to ask if more than one channel
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel # (0, 1... for the 1st, 2nd... channels; -1 for all channels):",
                                             -1, -1, sc-1, 1, &ok1);
            if (!ok1)
                return;
        }
    }
    
    V3DLONG in_sz[4];
    in_sz[0] = p4DImage->getXDim(); 
    in_sz[1] = p4DImage->getYDim(); 
    in_sz[2] = p4DImage->getZDim(); 
    in_sz[3] = sc;

    //
    V3DLONG cb, ce, k;
    if (c<0) {cb=0; ce=sc-1;}
    else {cb = ce = c;}
    
    void * outimg=0;
    for (k=cb; k<=ce; k++)
    {
        switch(p4DImage->getDatatype())
        {
            case V3D_UINT8: 
                b_res = extract_a_channel(data1d, in_sz, k, outimg); 
                break;
            case V3D_UINT16: 
                b_res = extract_a_channel((unsigned short int *)data1d, in_sz, k, outimg); 
                break;
            case V3D_FLOAT32: 
                b_res = extract_a_channel((float *)data1d, in_sz, k, outimg); 
                break;
            default: b_res = false; v3d_msg("Right now this plugin supports only UINT8/UINT16/FLOAT32 data. Do nothing."); return;
        }

        //display
        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->createImage(in_sz[0], in_sz[1], in_sz[2], 1, p4DImage->getDatatype());        
        memcpy(new4DImage->getRawData(), (unsigned char *)outimg, new4DImage->getTotalBytes());
        
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.setImageName(newwin, QString("").setNum(k).prepend("_C").prepend(p4DImage->getFileName()));
        callback.updateImageWindow(newwin);
    }
}



template <class T> bool extract_a_channel(T* data1d, V3DLONG *sz, V3DLONG c, void* &outimg)
{
    if (!data1d || !sz || c<0 || c>=sz[3])
    {
        printf("problem: c=[%ld] sz=[%p] szc=[%ld], data1d=[%p]\n",c, sz, sz[3], data1d);
        return false;
    }
    
    outimg = (void *) (data1d + c*sz[0]*sz[1]*sz[2]);

//    printf("ok c=[%ld]\n",c);
    return true;
}




