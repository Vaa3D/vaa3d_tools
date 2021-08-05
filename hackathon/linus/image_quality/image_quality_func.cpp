/* image_quality_func.cpp
 * Obtain image quality features
 * 2021-08-04 : by Linus Manubens-Gil
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "image_quality_func.h"
#include <vector>
#include <iostream>
//#include "stackutil.h"

using namespace std;

// func converting
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt);

const QString title = QObject::tr("Image Quality");

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

    //TODO add datatype judgment in case someone wanted to compute in 16bit
	double max_value = 256;
	V3DLONG histscale = 256;
    QVector<QVector<int> > hist_vec;

	int nChannel = p4DImage->getCDim();
	V3DLONG sz[3];
	sz[0] = p4DImage->getXDim();
	sz[1] = p4DImage->getYDim();
	sz[2] = p4DImage->getZDim();

	for (int c=0;c<nChannel;c++)
	{
        unsigned char * subject1d = p4DImage->getRawDataAtChannel(c);

        // Code from datatype_convert plugin
        V3DLONG	sz_sub = sz[0]*sz[1]*sz[2]*sz[3];
        unsigned char * inimg1d = NULL;

        try
        {
            inimg1d = new unsigned char [sz_sub];
        }
        catch(...)
        {
            printf("Error allocating memory. \n");
            return -1;
        }

        // Data type conversion
        if (p4DImage->getDatatype()!=V3D_UINT8)
        {
            v3d_msg("Converting to 8 bit image to standardize results.\n");

    //        if(sub_dt == 1)
    //        {
    //            converting<unsigned char, unsigned char>((unsigned char *)subject1d, inimg1d, sz_sub, V3D_UINT8);
    //        }
            if(p4DImage->getDatatype()==V3D_UINT16)
            {
                converting<unsigned short, unsigned char>((unsigned short *)subject1d, inimg1d, sz_sub, V3D_UINT8);
            }
            else if(p4DImage->getDatatype()==V3D_FLOAT32)
            {
                converting<float, unsigned char>((float *)subject1d, inimg1d, sz_sub, V3D_UINT8);
            }

        }
        else
        {
            inimg1d = subject1d;
        }

        if (subject1d) {delete []subject1d; subject1d=NULL;}


		QVector<int> tmp;
		getHistogram(inimg1d, sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
        hist_vec.append(tmp);
	}

    //output histogram to csv file
    bool ok;
    QString outfile;
    outfile = QInputDialog::getText(0,"Output file","Write full path and filename of the output csv file",QLineEdit::Normal,"./hist.csv",&ok);
    if (!ok)
        outfile = "./hist.csv";
    QByteArray outfileba = outfile.toLocal8Bit();
    char *strout = outfileba.data();
    FILE *fp;
    fp = fopen(strout, "w");
    for (int i=0;i<hist_vec.size();i++)
    {
        for (int j=0;j<hist_vec[i].size();j++)
            fprintf(fp, "%d,", hist_vec[i][j]);
        fprintf(fp,"\n");
    }
    fclose(fp);

	return 1;
}




bool compute(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to image quality"<<endl;
	if(output.size() != 1) return true;
	
	vector<char*>* inlist = (vector<char*>*) (input.at(0).p);
	if (inlist->size() != 1)
	{
		cerr<<"You must specify 1 input file!"<<endl;
		return false;
	}
	char * infile = inlist->at(0);
	cout<<"input file: "<<infile<<endl;
    unsigned char * subject1d = NULL;
    V3DLONG sz[4];
	int datatype;
    if (!simple_loadimage_wrapper(callback, infile, subject1d, sz, datatype))
	{
        cerr<<"failed to load image"<<endl;
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

    // Code from datatype_convert plugin
    V3DLONG	sz_sub = sz[0]*sz[1]*sz[2]*sz[3];
    unsigned char * inimg1d = NULL;

    try
    {
        inimg1d = new unsigned char [sz_sub];
    }
    catch(...)
    {
        printf("Error allocating memory. \n");
        return -1;
    }

    // Data type conversion
	if (datatype!=1)
	{
        cout << "Converting to 8 bit image to standardize results.\n";

//        if(sub_dt == 1)
//        {
//            converting<unsigned char, unsigned char>((unsigned char *)subject1d, inimg1d, sz_sub, V3D_UINT8);
//        }
        if(datatype == 2)
        {
            converting<unsigned short, unsigned char>((unsigned short *)subject1d, inimg1d, sz_sub, V3D_UINT8);
        }
        else if(datatype == 4)
        {
            converting<float, unsigned char>((float *)subject1d, inimg1d, sz_sub, V3D_UINT8);
        }

	}
    else
    {
        inimg1d = subject1d;
    }

    if (subject1d) {delete []subject1d; subject1d=NULL;}

    //TODO add datatype judgment in case someone wanted to compute in 16bit
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

    //if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
	return true;

}



// func converting taken from datatype_cnvrt plugin
template <class Tpre, class Tpost>
void converting(void *pre1d, Tpost *pPost, V3DLONG imsz, ImagePixelType v3d_dt)
{
     if (!pre1d ||!pPost || imsz<=0 )
     {
          v3d_msg("Invalid parameters to converting().", 0);
          return;
     }

    Tpre *pPre = (Tpre *)pre1d;

    if(v3d_dt == V3D_UINT8)
    {
        Tpre max_v=0, min_v = 255;

        for(V3DLONG i=0; i<imsz; i++)
        {
            if(max_v<pPre[i]) max_v = pPre[i];
            if(min_v>pPre[i]) min_v = pPre[i];
        }
        max_v -= min_v;

        if(max_v>0)
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) 255*(double)(pPre[i] - min_v)/max_v;
        }
        else
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) pPre[i];
        }
    }
    else if(v3d_dt == V3D_UINT16)
    {
        Tpre max_v=0, min_v = 65535;

        for(V3DLONG i=0; i<imsz; i++)
        {
            if(max_v<pPre[i]) max_v = pPre[i];
            if(min_v>pPre[i]) min_v = pPre[i];
        }
        max_v -= min_v;

        if(max_v>0)
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) 65535*(double)(pPre[i] - min_v)/max_v;
        }
        else
        {
            for(V3DLONG i=0; i<imsz; i++)
                pPost[i] = (Tpost) pPre[i];
        }

    }
    else if(v3d_dt == V3D_FLOAT32)
    {
        for(V3DLONG i=0; i<imsz; i++)
            pPost[i] = (Tpost) pPre[i];
    }

}

#undef INF
