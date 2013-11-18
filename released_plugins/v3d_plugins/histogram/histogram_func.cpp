/* histogram_func.cpp
 * Display histogram of the image
 * 2012-03-01 : by Jianlong Zhou
 * 2012-03-22 : change histogram comp method, by Yinan Wan 
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "histogram_func.h"
#include <vector>
#include <iostream>
#include "histogram_gui.h"
#include "stackutil.h"

using namespace std;

const QString title = QObject::tr("Histogram");

/*******************************************************
 * Split a string into string array
 * 1. args should be 0
 * 2. release args if not used any more
 *******************************************************/
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

#include <math.h>
#define INF 1.0e300

template <class T> bool getHistogram(const T * pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
{
	// init hist
	hist = QVector<int>(histscale, 0);

	for (V3DLONG i=0;i<datalen;i++)
	{
		V3DLONG ind = pdata1d[i]/max_value * histscale;
		//V3DLONG ind = pdata1d[i];
		hist[ind] ++;
	}

	return true;

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

	Image4DSimple *p4DImage = callback.getImage(curwin);
	if (p4DImage->getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Now we only support 8 bit image.\n");
		return -1;
	}

	//TODO add datatype judgment
	double max_value = 256;
	V3DLONG histscale = 256;
	QVector<QVector<int> > hist_vec;
	QStringList labelsLT;

	int nChannel = p4DImage->getCDim();
	V3DLONG sz[3];
	sz[0] = p4DImage->getXDim();
	sz[1] = p4DImage->getYDim();
	sz[2] = p4DImage->getZDim();

	for (int c=0;c<nChannel;c++)
	{
		unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);
		QVector<int> tmp;
		getHistogram(inimg1d, sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
		hist_vec.append(tmp);
		labelsLT.append(QString("channel %1").arg(c+1));
	}
	QString labelRB = QString("%1").arg(max_value);


	histogramDialog * dlg = new histogramDialog(hist_vec, labelsLT, labelRB, parent, QSize(500,150), QColor(50,50,50));
	dlg->setWindowTitle(QObject::tr("Histogram"));
	dlg->show();

	return 1;
}




bool compute(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to histogram"<<endl;
	if(output.size() != 1) return true;
	
	vector<char*>* inlist = (vector<char*>*) (input.at(0).p);
	if (inlist->size() != 1)
	{
		cerr<<"You must specify 1 input file!"<<endl;
		return false;
	}
	char * infile = inlist->at(0);
	cout<<"input file: "<<infile<<endl;
	unsigned char * inimg1d = NULL;
    V3DLONG sz[4];
	int datatype;
    if (!simple_loadimage_wrapper(callback, infile, inimg1d, sz, datatype))
	{
		cerr<<"faile to load image"<<endl;
		return false;
	}

	//Read the output file
	vector<char*>* outlist = (vector<char*>*) (output.at(0).p);
	if (outlist->size()!=1)
	{
		cerr<<"You must specify 1 output file!"<<endl;
		return false;
	}
	char* outfile = outlist->at(0);
	cout<<"output file: "<<outfile<<endl;
	
	if (datatype!=1)
	{
		v3d_msg("Now we only support 8 bit image.\n");
		return -1;
	}

	//TODO add datatype judgment
	double max_value = 256;
	V3DLONG histscale = 256;
	QVector<QVector<int> > hist_vec;

	int nChannel = sz[3];

	for (int c=0;c<nChannel;c++)
	{
		QVector<int> tmp;
		getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
		hist_vec.append(tmp);
	}

	//output histogram to csv file
	FILE *fp;
	fp = fopen(outfile, "w");
	for (int i=0;i<hist_vec.size();i++)
	{
		for (int j=0;j<hist_vec[i].size();j++)
			fprintf(fp, "%d,", hist_vec[i][j]);
		fprintf(fp,"\n");
	}
	fclose(fp);

	if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
	return true;

}

#undef INF
