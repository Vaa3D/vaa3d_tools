/* image_quality_func.cpp
 * Obtain image quality features
 * 2021-08-04 : by Linus Manubens-Gil
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "image_quality_func.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <numeric>
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

template <class T> bool getVec(const T * pdata1d, V3DLONG datalen, vector<int> &Vec)
{
    int i;
    // create vector of intensities
    for (i = 0; i < datalen; i++) {
        Vec.push_back(pdata1d[i]);
    }
    return true;
}

//template <class T> bool getHistogram(const T * pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
bool getHistogram(vector<int> pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
{
	// init hist
	hist = QVector<int>(histscale, 0);

	for (V3DLONG i=0;i<datalen;i++)
	{
        V3DLONG ind = pdata1d.at(i)/max_value * histscale;
		//V3DLONG ind = pdata1d[i];
		hist[ind] ++;
	}

	return true;

}

//template <class T> bool getStats(const T * pdata1d, V3DLONG datalen, int &minint, int &maxint, float &meanint, float &medianint)
bool getStats(vector<int> pdata1d, V3DLONG datalen, int &minint, int &maxint, double &meanint, double &medianint, double &madint, double &stdevint)
{
    // init
    maxint = int(0);
    minint = int(255);
    meanint = float(0);
    medianint = float(0);
    stdevint = float(0);

    // for median
    vector<int> intvec;
    int i;
    // Sort the numbers using pointers
    for (i = 0; i < datalen; i++) {
        intvec.push_back(pdata1d[i]);
    }
    sort(intvec.begin(),intvec.end());
    // print the numbers
//    for (i = 0; i < datalen; i++)
//        printf("%d ", intvec.at(i));

    for (V3DLONG i=0;i<datalen;i++)
    {
        if(pdata1d[i]>maxint)
        {
            maxint = pdata1d[i];
        }
        if(pdata1d[i]<minint)
        {
            minint = pdata1d[i];
        }
        meanint += pdata1d[i];
    }
    meanint /= datalen;

    double sqsum = inner_product(pdata1d.begin(),pdata1d.end(),pdata1d.begin(),0);
    stdevint = sqrt(sqsum/datalen - meanint*meanint);

    if (datalen % 2 != 0)
    {
        medianint = intvec.at(datalen / 2);
    }
    else
    {
        medianint = (double)(intvec.at((datalen - 1) / 2) + intvec.at(datalen / 2)) / 2.0;
    }

    vector<float> mdevvec;
    float dev;
    for (i = 0; i < datalen; i++) {
        dev= intvec.at(i)-medianint;
        if(dev < 0)dev *= -1;
        mdevvec.push_back(dev);
    }
    sort(mdevvec.begin(),mdevvec.end());
    if (datalen % 2 != 0)
    {
        madint = mdevvec.at(datalen / 2);
    }
    else
    {
        madint = (double)(mdevvec.at((datalen - 1) / 2) + mdevvec.at(datalen / 2)) / 2.0;
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
    QString  imgname = callback.getImageName(curwin);

    //TODO add datatype judgment in case someone wanted to compute in 16bit
	double max_value = 256;
	V3DLONG histscale = 256;
    QVector<QVector<int> > hist_vec;
    QVector<double> FocusScore_vec;
    QVector<int> MinIntensity_vec;
    QVector<int> MaxIntensity_vec;
    QVector<double> MeanIntensity_vec;
    QVector<double> MedianIntensity_vec;
    QVector<double> MADIntensity_vec;
    QVector<double> StdIntensity_vec;
    QVector<double> PercentMinimal_vec;
    QVector<double> PercentMaximal_vec;
    QVector<double> ThreshOtsu_vec;
    QVector<double> SNRmean_vec;
    QVector<double> CNRmean_vec;
    QVector<double> SNRotsu_vec;
    QVector<double> CNRotsu_vec;

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


        int  maxint,minint;
        double meanint,medianint,madint,stdevint,pcmin,pcmax,focusscore;
        vector<int> intvec;
        getVec(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], intvec);
        if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
        //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
        getStats(intvec, intvec.size(), minint, maxint, meanint, medianint, madint, stdevint);
        cout << maxint << "\t" << minint << "\t" << meanint << "\n";
        QVector<int> tmphist;
        //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
        getHistogram(intvec, intvec.size(), max_value, histscale, tmphist);
        pcmin = tmphist.at(minint)*100/(sz[0]*sz[1]*sz[2]);
        pcmax = tmphist.at(maxint)*100/(sz[0]*sz[1]*sz[2]);
        hist_vec.append(tmphist);

        MaxIntensity_vec.append(maxint);
        MinIntensity_vec.append(minint);
        MeanIntensity_vec.append(meanint);
        MedianIntensity_vec.append(medianint);
        MADIntensity_vec.append(madint);
        StdIntensity_vec.append(stdevint);
        PercentMinimal_vec.append(pcmin);
        PercentMaximal_vec.append(pcmax);

        // Get Focus Score
        focusscore = stdevint*stdevint/meanint;
        FocusScore_vec.append(focusscore);

        // Get Otsu threshold
        double var_max, sum, sumB, q1, q2, u1, u2, sigma2, ThreshOtsu;
        var_max = sum = sumB = q1 = q2 = u1 = u2 = sigma2 = ThreshOtsu = 0;
        for(int i=0; i<tmphist.size(); i++)
        {
            sum += i*double(tmphist.at(i));
        }
        for(int t=0; t<tmphist.size(); t++)
        {
            q1 += double(tmphist.at(t));
            if(q1>0)
            {
                q2 = sz[0]*sz[1]*sz[2]-q1;
                sumB += t*double(tmphist.at(t));
                u1 = sumB/q1;
                u2 = (sum-sumB)/q2;
                sigma2 = q1*q2*(u1-u2)*(u1-u2);
                if(sigma2 > var_max)
                {
                    ThreshOtsu = t;
                    var_max =sigma2;
                }
            }
        }
        cout << "Otsu threshold:\t" << ThreshOtsu << "\n";
        ThreshOtsu_vec.append(ThreshOtsu);

        // Get SNR and CNR using mean intensity and Otsu as threshold for background
        double SNRmean,CNRmean,SNRotsu,CNRotsu;

        vector<int> otsunoise,otsusignal;
        double meanotsunoise,meanotsusignal,stdevotsu,sqsum;
        vector<int> meannoise,meansignal;
        double avmeannoise,avmeansignal,stdevmean;
        for(int i=0; i<sz[0]*sz[1]*sz[2]; i++)
        {
            if(intvec.at(i)<ThreshOtsu)
            {
                otsunoise.push_back(intvec.at(i));
                meanotsunoise += intvec.at(i);
            }
            else
            {
                otsusignal.push_back(intvec.at(i));
                meanotsusignal += intvec.at(i);
            }

            if(intvec.at(i)<meanint)
            {
                meannoise.push_back(intvec.at(i));
                avmeannoise += intvec.at(i);
            }
            else
            {
                meansignal.push_back(intvec.at(i));
                avmeansignal += intvec.at(i);
            }
        }
        meanotsunoise = meanotsunoise/otsunoise.size();
        meanotsusignal = meanotsusignal/otsusignal.size();
        sqsum = inner_product(otsunoise.begin(),otsunoise.end(),otsunoise.begin(),0);
        stdevotsu = sqrt(sqsum/sz[0]*sz[1]*sz[2] - meanotsunoise*meanotsunoise);

        avmeannoise = avmeannoise/meannoise.size();
        avmeansignal = avmeansignal/meansignal.size();
        sqsum = inner_product(meannoise.begin(),meannoise.end(),meannoise.begin(),0);
        stdevmean = sqrt(sqsum/sz[0]*sz[1]*sz[2] -avmeannoise*avmeannoise);

        SNRmean = avmeansignal/stdevmean;
        CNRmean = (maxint-meanint)/stdevmean;
        SNRotsu = meanotsusignal/stdevotsu;
        CNRotsu = (maxint-ThreshOtsu)/stdevotsu;

        SNRmean_vec.append(SNRmean);
        CNRmean_vec.append(CNRmean);
        SNRotsu_vec.append(SNRotsu);
        CNRotsu_vec.append(CNRotsu);

        //cout << tmphist.at(0) << "\n";
        cout << "minint\tmaxint\tmeanint\tmedint\tMADint\tStdint\tpcmin\tpcmax\tSNRmean\t\tCNRmean\t\tSNRotsu\t\tCNRotsu\t\tFocusScore\n";
        cout << minint << "\t" << maxint << "\t" << meanint << "\t" << medianint << "\t" << madint << "\t" << stdevint << "\t" << pcmin << "\t" << pcmax << "\t" << SNRmean << "\t" << CNRmean << "\t" << SNRotsu << "\t" << CNRotsu << "\t" << focusscore << "\n";
    }

    //output histogram to csv file
    bool ok;
    QString outfile;
    outfile = QInputDialog::getText(0,"Output file","Write full path and filename of the output csv file",QLineEdit::Normal,"./ImageQuality.csv",&ok);
    if (!ok)
        outfile = "./ImageQuality.csv";
    QByteArray outfileba = outfile.toLocal8Bit();
    char *strout = outfileba.data();
    FILE *fp;
    fp = fopen(strout, "w");
    QString header = "Image_name,Channel,MinIntensity,MaxIntensity,MeanIntensity,MedianIntensity,MADIntensity,StdIntensity,PercentMinimal,PercentMaximal,ThresholdOtsu,SNR_mean,CNR_mean,SNR_otsu,CNR_otsu,FocusScore\n";
    fprintf(fp, "%s", header.toStdString().c_str());
    for (int i=0;i<hist_vec.size();i++)
    {
        fprintf(fp, "%s,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", imgname.toStdString().c_str(),i+1,MinIntensity_vec.at(i),MaxIntensity_vec.at(i),MeanIntensity_vec.at(i),MedianIntensity_vec.at(i),MADIntensity_vec.at(i),StdIntensity_vec.at(i),PercentMinimal_vec.at(i),PercentMaximal_vec.at(i),ThreshOtsu_vec.at(i),SNRmean_vec.at(i),CNRmean_vec.at(i),SNRotsu_vec.at(i),CNRotsu_vec.at(i),FocusScore_vec.at(i));
    }
//    for (int i=0;i<hist_vec.size();i++)
//    {
//        for (int j=0;j<hist_vec[i].size();j++)
//            fprintf(fp, "%d,", hist_vec[i][j]);
//        fprintf(fp,"\n");
//    }
    fclose(fp);

	return 1;
}




bool compute(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to image quality"<<endl;
    //if(output.size() != 1) return true;
	
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
    char* outfile;
    if(output.size()==1)
    {
        vector<char*>* outlist = (vector<char*>*) (output.at(0).p);
        if (outlist->size()>1)
        {
            cerr<<"You must specify 1 output file!"<<endl;
            return false;
        }
        outfile = outlist->at(0);
    }
    else
    {
        outfile = "ImageQuality.csv";
    }
    QString outfileapp = QString(infile) + "." + QString(outfile);
    cout<<"\noutput file: "<<outfileapp.toStdString().c_str()<<endl;

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
    cout << "\nConverting to 8 bit image to standardize results.\n";

    if(datatype == 1)
    {
        converting<unsigned char, unsigned char>((unsigned char *)subject1d, inimg1d, sz_sub, V3D_UINT8);
    }
    if(datatype == 2)
    {
        converting<unsigned short, unsigned char>((unsigned short *)subject1d, inimg1d, sz_sub, V3D_UINT8);
    }
    else if(datatype == 4)
    {
        converting<float, unsigned char>((float *)subject1d, inimg1d, sz_sub, V3D_UINT8);
    }

    if (subject1d) {delete []subject1d; subject1d=NULL;}

    //TODO add datatype judgment in case someone wanted to compute in 16bit
	double max_value = 256;
	V3DLONG histscale = 256;
	QVector<QVector<int> > hist_vec;
    QVector<double> FocusScore_vec;
    QVector<int> MinIntensity_vec;
    QVector<int> MaxIntensity_vec;
    QVector<double> MeanIntensity_vec;
    QVector<double> MedianIntensity_vec;
    QVector<double> MADIntensity_vec;
    QVector<double> StdIntensity_vec;
    QVector<double> PercentMinimal_vec;
    QVector<double> PercentMaximal_vec;
    QVector<double> ThreshOtsu_vec;
    QVector<double> SNRmean_vec;
    QVector<double> CNRmean_vec;
    QVector<double> SNRotsu_vec;
    QVector<double> CNRotsu_vec;

	int nChannel = sz[3];

    vector<int> intvec;
    getVec(inimg1d, nChannel*sz[0]*sz[1]*sz[2], intvec);
    if (inimg1d) {delete []inimg1d; inimg1d=NULL;}

    if(nChannel == 1)
    {
        int  maxint,minint;
        double meanint,medianint,madint,stdevint,pcmin,pcmax,focusscore;

        //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
        getStats(intvec, sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint, madint, stdevint);
        QVector<int> tmphist;
        //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
        getHistogram(intvec, sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
        pcmin = tmphist.at(minint)*100/(sz[0]*sz[1]*sz[2]);
        pcmax = tmphist.at(maxint)*100/(sz[0]*sz[1]*sz[2]);
        hist_vec.append(tmphist);
        MaxIntensity_vec.append(maxint);
        MinIntensity_vec.append(minint);
        MeanIntensity_vec.append(meanint);
        MedianIntensity_vec.append(medianint);
        MADIntensity_vec.append(madint);
        StdIntensity_vec.append(stdevint);
        PercentMinimal_vec.append(pcmin);
        PercentMaximal_vec.append(pcmax);

        // Get Focus Score
        focusscore = stdevint*stdevint/meanint;
        FocusScore_vec.append(focusscore);

        // Get Otsu threshold
        double var_max, sum, sumB, q1, q2, u1, u2, sigma2, ThreshOtsu;
        var_max = sum = sumB = q1 = q2 = u1 = u2 = sigma2 = ThreshOtsu = 0;
        for(int i=0; i<tmphist.size(); i++)
        {
            sum = sum + i*double(tmphist.at(i));
        }
        for(int t=0; t<tmphist.size(); t++)
        {
            q1 = q1 + double(tmphist.at(t));
            //cout << q1 << " ";
            if(q1>0)
            {
                q2 = sz[0]*sz[1]*sz[2]-q1;
                //cout << q2 << " ";
                sumB = sumB + double(t)*double(tmphist.at(t));
                u1 = sumB/q1;
                //cout << u1 << " ";
                u2 = (sum-sumB)/q2;
                sigma2 = q1*q2*(u1-u2)*(u1-u2);
                if(sigma2 > var_max)
                {
                    ThreshOtsu = t;
                    var_max =sigma2;
                }
            }
        }
        cout << "Otsu threshold:\t" << ThreshOtsu << "\n";
        ThreshOtsu_vec.append(ThreshOtsu);

        // Get SNR and CNR using mean intensity and Otsu as threshold for background
        double SNRmean,CNRmean,SNRotsu,CNRotsu;

        vector<int> otsunoise,otsusignal;
        double meanotsunoise,meanotsusignal,stdevotsu,sqsum;
        vector<int> meannoise,meansignal;
        double avmeannoise,avmeansignal,stdevmean;
        for(int i=0; i<sz[0]*sz[1]*sz[2]; i++)
        {
            if(intvec.at(i)<ThreshOtsu)
            {
                otsunoise.push_back(intvec.at(i));
                meanotsunoise += intvec.at(i);
            }
            else
            {
                otsusignal.push_back(intvec.at(i));
                meanotsusignal += intvec.at(i);
            }

            if(intvec.at(i)<meanint)
            {
                meannoise.push_back(intvec.at(i));
                avmeannoise += intvec.at(i);
            }
            else
            {
                meansignal.push_back(intvec.at(i));
                avmeansignal += intvec.at(i);
            }
        }
        meanotsunoise = meanotsunoise/otsunoise.size();
        meanotsusignal = meanotsusignal/otsusignal.size();
        sqsum = inner_product(otsunoise.begin(),otsunoise.end(),otsunoise.begin(),0);
        stdevotsu = sqrt(sqsum/sz[0]*sz[1]*sz[2] - meanotsunoise*meanotsunoise);

        avmeannoise = avmeannoise/meannoise.size();
        avmeansignal = avmeansignal/meansignal.size();
        sqsum = inner_product(meannoise.begin(),meannoise.end(),meannoise.begin(),0);
        stdevmean = sqrt(sqsum/sz[0]*sz[1]*sz[2] -avmeannoise*avmeannoise);

        SNRmean = avmeansignal/stdevmean;
        CNRmean = (maxint-meanint)/stdevmean;
        SNRotsu = meanotsusignal/stdevotsu;
        CNRotsu = (maxint-ThreshOtsu)/stdevotsu;

        SNRmean_vec.append(SNRmean);
        CNRmean_vec.append(CNRmean);
        SNRotsu_vec.append(SNRotsu);
        CNRotsu_vec.append(CNRotsu);

        //cout << tmphist.at(0) << "\n";
        cout << "minint\tmaxint\tmeanint\tmedint\tMADint\tStdint\tpcmin\tpcmax\tSNRmean\t\tCNRmean\t\tSNRotsu\t\tCNRotsu\t\tFocusScore\n";
        cout << minint << "\t" << maxint << "\t" << meanint << "\t" << medianint << "\t" << madint << "\t" << stdevint << "\t" << pcmin << "\t" << pcmax << "\t" << SNRmean << "\t" << CNRmean << "\t" << SNRotsu << "\t" << CNRotsu << "\t" << focusscore << "\n";

    }
    else
    {
        for (int c=0;c<nChannel;c++)
        {
            int  maxint,minint;
            double meanint,medianint,madint,stdevint,pcmin,pcmax,focusscore;

            //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
            vector<int> subvec = {intvec.begin()+c*sz[0]*sz[1]*sz[2],intvec.begin()+(c+1)*sz[0]*sz[1]*sz[2]};
            getStats(subvec, sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint, madint, stdevint);
            QVector<int> tmphist;
            //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
            getHistogram(subvec, sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
            pcmin = tmphist.at(minint)*100/(sz[0]*sz[1]*sz[2]);
            pcmax = tmphist.at(maxint)*100/(sz[0]*sz[1]*sz[2]);
            hist_vec.append(tmphist);
            MaxIntensity_vec.append(maxint);
            MinIntensity_vec.append(minint);
            MeanIntensity_vec.append(meanint);
            MedianIntensity_vec.append(medianint);
            MADIntensity_vec.append(madint);
            StdIntensity_vec.append(stdevint);
            PercentMinimal_vec.append(pcmin);
            PercentMaximal_vec.append(pcmax);

            // Get Focus Score
            focusscore = stdevint*stdevint/meanint;
            FocusScore_vec.append(focusscore);

            // Get Otsu threshold
            double var_max, sum, sumB, q1, q2, u1, u2, sigma2, ThreshOtsu;
            var_max = sum = sumB = q1 = q2 = u1 = u2 = sigma2 = ThreshOtsu = 0;
            for(int i=0; i<tmphist.size(); i++)
            {
                sum += i*tmphist.at(i);
            }
            for(int t=0; t<tmphist.size(); t++)
            {
                q1 += tmphist.at(t);
                if(q1>0)
                {
                    q2 = sz[0]*sz[1]*sz[2]-q1;
                    sumB += t*tmphist.at(t);
                    u1 = sumB/q1;
                    u2 = (sum-sumB)/q2;
                    sigma2 = q1*q2*(u1-u2)*(u1-u2);
                    if(sigma2 > var_max)
                    {
                        ThreshOtsu = t;
                        var_max =sigma2;
                    }
                }
            }
            cout << "Otsu threshold:\t" << ThreshOtsu << "\n";
            ThreshOtsu_vec.append(ThreshOtsu);

            // Get SNR and CNR using mean intensity and Otsu as threshold for background
            double SNRmean,CNRmean,SNRotsu,CNRotsu;

            vector<int> otsunoise,otsusignal;
            double meanotsunoise,meanotsusignal,stdevotsu,sqsum;
            vector<int> meannoise,meansignal;
            double avmeannoise,avmeansignal,stdevmean;
            for(int i=0; i<sz[0]*sz[1]*sz[2]; i++)
            {
                if(subvec.at(i)<ThreshOtsu)
                {
                    otsunoise.push_back(subvec.at(i));
                    meanotsunoise += subvec.at(i);
                }
                else
                {
                    otsusignal.push_back(subvec.at(i));
                    meanotsusignal += subvec.at(i);
                }

                if(subvec.at(i)<meanint)
                {
                    meannoise.push_back(subvec.at(i));
                    avmeannoise += subvec.at(i);
                }
                else
                {
                    meansignal.push_back(subvec.at(i));
                    avmeansignal += subvec.at(i);
                }
            }
            meanotsunoise = meanotsunoise/otsunoise.size();
            meanotsusignal = meanotsusignal/otsusignal.size();
            sqsum = inner_product(otsunoise.begin(),otsunoise.end(),otsunoise.begin(),0);
            stdevotsu = sqrt(sqsum/sz[0]*sz[1]*sz[2] - meanotsunoise*meanotsunoise);

            avmeannoise = avmeannoise/meannoise.size();
            avmeansignal = avmeansignal/meansignal.size();
            sqsum = inner_product(meannoise.begin(),meannoise.end(),meannoise.begin(),0);
            stdevmean = sqrt(sqsum/sz[0]*sz[1]*sz[2] -avmeannoise*avmeannoise);

            SNRmean = avmeansignal/stdevmean;
            CNRmean = (maxint-meanint)/stdevmean;
            SNRotsu = meanotsusignal/stdevotsu;
            CNRotsu = (maxint-ThreshOtsu)/stdevotsu;

            SNRmean_vec.append(SNRmean);
            CNRmean_vec.append(CNRmean);
            SNRotsu_vec.append(SNRotsu);
            CNRotsu_vec.append(CNRotsu);

            //cout << tmphist.at(0) << "\n";
            cout << "minint\tmaxint\tmeanint\tmedint\tMADint\tStdint\tpcmin\tpcmax\tSNRmean\t\tCNRmean\t\tSNRotsu\t\tCNRotsu\t\tFocusScore\n";
            cout << minint << "\t" << maxint << "\t" << meanint << "\t" << medianint << "\t" << madint << "\t" << stdevint << "\t" << pcmin << "\t" << pcmax << "\t" << SNRmean << "\t" << CNRmean << "\t" << SNRotsu << "\t" << CNRotsu << "\t" << focusscore << "\n";

        }
    }

	//output histogram to csv file
	FILE *fp;
    fp = fopen(outfileapp.toStdString().c_str(), "w");
    QString header = "Image_name,Channel,MinIntensity,MaxIntensity,MeanIntensity,MedianIntensity,MADIntensity,StdIntensity,PercentMinimal,PercentMaximal,ThresholdOtsu,SNR_mean,CNR_mean,SNR_otsu,CNR_otsu,FocusScore\n";
    fprintf(fp, "%s", header.toStdString().c_str());
    for (int i=0;i<hist_vec.size();i++)
    {
        fprintf(fp, "%s,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", infile,i+1,MinIntensity_vec.at(i),MaxIntensity_vec.at(i),MeanIntensity_vec.at(i),MedianIntensity_vec.at(i),MADIntensity_vec.at(i),StdIntensity_vec.at(i),PercentMinimal_vec.at(i),PercentMaximal_vec.at(i),ThreshOtsu_vec.at(i),SNRmean_vec.at(i),CNRmean_vec.at(i),SNRotsu_vec.at(i),CNRotsu_vec.at(i),FocusScore_vec.at(i));
    }
//	for (int i=0;i<hist_vec.size();i++)
//	{
//		for (int j=0;j<hist_vec[i].size();j++)
//			fprintf(fp, "%d,", hist_vec[i][j]);
//		fprintf(fp,"\n");
//	}
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
