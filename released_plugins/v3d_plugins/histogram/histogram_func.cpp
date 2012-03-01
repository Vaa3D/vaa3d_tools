/* histogram_func.cpp
 * Display histogram of the image
 * 2012-03-01 : by Jianlong Zhou
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "histogram_func.h"
#include <vector>
#include <iostream>
#include "histogram_gui.h"

using namespace std;

const QString title = QObject::tr("Histogram");

/*******************************************************
 * Split a string into string array
 * 1. args should be 0
 * 2. release args if not used any more
 *******************************************************/
#define INF 1.0e300

template <class T> void getHistogram(const T * pdata1d, V3DLONG datalen, V3DLONG &histscale, vector<V3DLONG> &hist)
{
	// get min max
	T maxval=(T)0;
	T minval = (T) INF;
     for(V3DLONG i=0; i<datalen; i++)
     {
          if(pdata1d[i]>maxval) maxval=pdata1d[i];
          if(pdata1d[i]<minval) minval=pdata1d[i];
     }

     // init hist
     hist.clear();
     for(V3DLONG i=0; i<histscale; i++)
     {
          hist.push_back(0);
     }

     // divid data into histscale # segments
     T step = (maxval-minval+1)/histscale;

     for (V3DLONG i=0;i<datalen;i++)
     {
          V3DLONG ind = (V3DLONG) pdata1d[i]/step;
          hist.at(ind) = hist.at(ind)+1;
     }

}

int split(const char *paras, char ** &args)
{
    if(paras == 0) return 0;
    int argc = 0;
    int len = strlen(paras);
    int posb[2048];
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
int compute(V3DPluginCallback2 &callback, QWidget *parent)
{
     v3dhandle curwin;
     curwin=callback.currentImageWindow();
     if(!curwin)
     {
          v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
          return -1;
     }

     int c = 0;
     Image4DSimple *p4DImage = callback.getImage(curwin);
     if(p4DImage->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
     V3DLONG sz[3];
     sz[0] = p4DImage->getXDim();
     sz[1] = p4DImage->getYDim();
     sz[2] = p4DImage->getZDim();

     unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

     V3DLONG histscale=256;
     vector<V3DLONG> hist;
	getHistogram(inimg1d, sz[0]*sz[1]*sz[2], histscale, hist);

     HistogramDialog *histdialog = new HistogramDialog(parent);
     histdialog->histscale = histscale;
     histdialog->hist = hist;

     histdialog->show();



     // v3dhandle newwin;
     // if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
     //      newwin = callback.currentImageWindow();
     // else
     //      newwin = callback.newImageWindow();

     // p4DImage->setData(inimg1d, sz[0], sz[1], sz[2], sz[3]);
     // callback.setImage(newwin, p4DImage);
     // callback.setImageName(newwin, QObject::tr("compute"));
     // callback.updateImageWindow(newwin);
	return 1;
}




bool compute(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to compute"<<endl;
	if(input.size() != 2 || output.size() != 1) return true;
	char * paras = 0;
	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	cout<<"paras : "<<paras<<endl;

	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	cout<<"paras : "<<paras<<endl;

	return true;
}

#undef INF
