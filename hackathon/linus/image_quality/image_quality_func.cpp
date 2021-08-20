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
template <class T> bool rc(T* data1d, V3DLONG *sz, V3DLONG c, double apercent);
template <class T> bool scaleintensity(T *img, V3DLONG sz[4], V3DLONG channo, double lower_th, double higher_th, double target_min, double target_max);


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

template <class T> bool getVec(const T * pdata1d, V3DLONG datalen, vector<uint8_t> &Vec)
{
    int i;
    // create vector of intensities
    for (i = 0; i < datalen; i++) {
        Vec.push_back(pdata1d[i]);
    }
    return true;
}

template <class T> bool getVecSWC(const T * pdata1d, V3DLONG mysz[4], int c, QList<NeuronSWC> neuron, vector<uint8_t> &Vec)
{
    V3DLONG id;
    // create vector of intensities
    for (id = 0; id < neuron.size(); id++) {
        V3DLONG nodex = neuron.at(id).x;
        V3DLONG nodey = neuron.at(id).y;
        V3DLONG nodez = neuron.at(id).z;
        struct XYZ treep = XYZ(nodex,nodey,nodez);
        Vec.push_back(pdata1d[V3DLONG(c*mysz[0]*mysz[1]*mysz[2] + treep.z*mysz[0]*mysz[1]+treep.y*mysz[0]+treep.x)]);
        //cout<<"\n\n";
        //cout<<uint(pdata1d[V3DLONG(treep.z*mysz[0]*mysz[1]+treep.y*mysz[0]+treep.x)])<<" ";
        //cout<<uint(Vec.at(Vec.size()-1))<<" ";
    }
    return true;
}

//template <class T> bool getHistogram(const T * pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
bool getHistogram(vector<uint8_t> pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
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
bool getStats(vector<uint8_t> pdata1d, V3DLONG datalen, int &minint, int &maxint, double &meanint, double &medianint, double &madint, double &stdevint)
{
    // init
    maxint = int(0);
    minint = int(255);
    meanint = float(0);
    medianint = int(0);
    stdevint = float(0);

    // for median
//    vector<int> intvec;
//    int i;
//    // Sort the numbers using pointers
//    for (i = 0; i < datalen; i++) {
//        intvec.push_back(pdata1d[i]);
//    }
    // print the numbers
//    for (i = 0; i < datalen; i++)
//        printf("%d ", intvec.at(i));

    for (V3DLONG i=0;i<datalen;i++)
    {
        int ival = pdata1d.at(i);
        if(ival>maxint)
        {
            maxint = ival;
        }
        if(ival<minint)
        {
            minint = ival;
        }
        meanint += ival;
    }
    meanint /= datalen;

    cout << "Starting Stdev calculation\n";
    for (V3DLONG i=0;i<datalen;i++)
    {
        stdevint += (pdata1d.at(i) - meanint)*(pdata1d.at(i) - meanint);
    }
    stdevint /= datalen;
    stdevint = sqrt(stdevint);
//    double sqsum = inner_product(pdata1d.begin(),pdata1d.end(),pdata1d.begin(),0);
//    stdevint = sqrt(sqsum/datalen - meanint*meanint);

    cout << "Starting Median calculation\n";
    sort(pdata1d.begin(),pdata1d.end());
    if (datalen % 2 != 0)
    {
        medianint = pdata1d.at(datalen / 2);
    }
    else
    {
        //cout << "even\n";
        //cout << datalen << "\n";
        medianint = int(lrint((double) pdata1d.at(datalen/2 - 1) + (double) pdata1d.at(datalen/2))/2);//int(lrint((((double) pdata1d.at(datalen - 1)) / 2 + ((double) pdata1d.at(datalen)) / 2) / 2.0));
    }

    cout << "Starting MAD calculation\n";
    vector<uint8_t> mdevvec;
    uint8_t dev;
    for (int i = 0; i < datalen; i++) {
        dev= pdata1d.at(i)-medianint;
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
        madint = int(round((double)(mdevvec.at((datalen - 1) / 2) + mdevvec.at(datalen / 2)) / 2.0));
    }
    mdevvec.clear();

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
    V3DLONG sz[4];
	sz[0] = p4DImage->getXDim();
	sz[1] = p4DImage->getYDim();
	sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    for (V3DLONG c=0;c<nChannel;c++)
	{
        unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

        // Code from datatype_convert plugin
//        V3DLONG	sz_sub = sz[0]*sz[1]*sz[2]*sz[3];
//        unsigned char * inimg1d = NULL;

//        try
//        {
//            inimg1d = new unsigned char [sz_sub];
//        }
//        catch(...)
//        {
//            printf("Error allocating memory. \n");
//            return -1;
//        }

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
                //converting<unsigned short, unsigned char>((unsigned short *)subject1d, inimg1d, sz_sub, V3D_UINT8);
                rc((unsigned short int *)inimg1d, sz, c, 0.01);
            }
            else if(p4DImage->getDatatype()==V3D_FLOAT32)
            {
                rc((float *)inimg1d, sz, c, 0.01);
                //converting<float, unsigned char>((float *)subject1d, inimg1d, sz_sub, V3D_UINT8);
            }

        }
//        else
//        {
//            inimg1d = subject1d;
//        }
//        if (subject1d) {delete []subject1d; subject1d=NULL;}


        cout << "\nComputing statistics\n";
        int  maxint,minint;
        double meanint,medianint,madint,stdevint,pcmin,pcmax,focusscore;
        vector<uint8_t> intvec;
        getVec(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], intvec);
        if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
        //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
        getStats(intvec, intvec.size(), minint, maxint, meanint, medianint, madint, stdevint);
        cout << maxint << "\t" << minint << "\t" << meanint << "\n";
        QVector<int> tmphist;
        //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
        getHistogram(intvec, intvec.size(), max_value, histscale, tmphist);
        pcmin = double(tmphist.at(minint))*100/double(sz[0]*sz[1]*sz[2]);
        pcmax = double(tmphist.at(maxint))*100/double(sz[0]*sz[1]*sz[2]);
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

        vector<uint8_t> otsunoise;//,otsusignal;
        double meanotsunoise,meanotsusignal,stdevotsu;//,sqsum;
        meanotsunoise=meanotsusignal=stdevotsu=0;
        vector<uint8_t> meannoise;//,meansignal;
        double avmeannoise,avmeansignal,stdevmean;
        avmeannoise=avmeansignal=stdevmean=0;
        for(int i=0; i<sz[0]*sz[1]*sz[2]; i++)
        {
            if(intvec.at(i)<ThreshOtsu)
            {
                otsunoise.push_back(intvec.at(i));
                meanotsunoise += intvec.at(i);
            }
            else
            {
                //otsusignal.push_back(intvec.at(i));
                meanotsusignal += intvec.at(i);
            }

            if(intvec.at(i)<meanint)
            {
                meannoise.push_back(intvec.at(i));
                avmeannoise += intvec.at(i);
            }
            else
            {
                //meansignal.push_back(intvec.at(i));
                avmeansignal += intvec.at(i);
            }
        }
        meanotsunoise = meanotsunoise/otsunoise.size();
        meanotsusignal = meanotsusignal/(intvec.size() - otsunoise.size());
        for (V3DLONG i=0;i<otsunoise.size();i++)
        {
            stdevotsu += (otsunoise.at(i) - meanotsunoise)*(otsunoise.at(i) - meanotsunoise);
        }
        stdevotsu /= otsunoise.size();
        stdevotsu = sqrt(stdevotsu);
//        sqsum = inner_product(otsunoise.begin(),otsunoise.end(),otsunoise.begin(),0);
//        stdevotsu = sqrt(sqsum/sz[0]*sz[1]*sz[2] - meanotsunoise*meanotsunoise);

        avmeannoise = avmeannoise/meannoise.size();
        avmeansignal = avmeansignal/(intvec.size() - meannoise.size());
        for (V3DLONG i=0;i<meannoise.size();i++)
        {
            stdevmean += (meannoise.at(i) - avmeannoise)*(meannoise.at(i) - avmeannoise);
        }
        stdevmean /= meannoise.size();
        stdevmean = sqrt(stdevmean);
        /*sqsum = inner_product(meannoise.begin(),meannoise.end(),meannoise.begin(),0);
        stdevmean = sqrt(sqsum/sz[0]*sz[1]*sz[2] -avmeannoise*avmeannoise);*/

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
        fprintf(fp, "%s,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", imgname.toStdString().c_str(),i+1,MinIntensity_vec.at(i),MaxIntensity_vec.at(i),MeanIntensity_vec.at(i),MedianIntensity_vec.at(i),MADIntensity_vec.at(i),StdIntensity_vec.at(i),PercentMinimal_vec.at(i),PercentMaximal_vec.at(i),ThreshOtsu_vec.at(i),SNRmean_vec.at(i),CNRmean_vec.at(i),SNRotsu_vec.at(i),CNRotsu_vec.at(i),FocusScore_vec.at(i));
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
    if (inlist->size() != 1 && inlist->size() != 2)
	{
        cerr<<"You must specify 1 or 2 input files!"<<endl;
		return false;
	}
	char * infile = inlist->at(0);
    cout<<"input image file: "<<infile<<endl;

//    QList<NeuronSWC> neuron;
    vector<QList<NeuronSWC> > nt_list;
    if (inlist->size() == 1)
    {
        QList<NeuronSWC> tmp;
        nt_list.push_back(tmp);
    }
    if (inlist->size() == 2)
    {
        char * inswc = inlist->at(1);
        cout<<"input SWC file: "<<inswc<<endl;

        // Get Neuron
    //    NeuronTree nt;
        QString fileOpenName = QString(inswc);
        //cout<<fileOpenName.toStdString().c_str()<<endl;

        if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC"))
        {
            QList<NeuronSWC> tmp = readSWC_file(fileOpenName).listNeuron;
            nt_list.push_back(tmp);
        }
        else if (fileOpenName.endsWith(".ano") || fileOpenName.endsWith(".ANO"))
        {
            P_ObjectFileType linker_object;
            if (!loadAnoFile(fileOpenName,linker_object))
            {
                cout<<"Error in reading the linker file."<<endl;
                return false;
            }
            QStringList nameList = linker_object.swc_file_list;
            V3DLONG neuronNum = nameList.size();
            for (V3DLONG i=0;i<neuronNum;i++)
            {
                QList<NeuronSWC> tmp = readSWC_file(nameList.at(i)).listNeuron;
                nt_list.push_back(tmp);
            }
        }
        else
        {
            cout<<"The reconstruction file type you specified is not supported."<<endl;
            return false;
        }
    }
    unsigned char * inimg1d = NULL;
    //unsigned char * subject1d = NULL;
    V3DLONG sz[4];
	int datatype;
    //if (!simple_loadimage_wrapper(callback, infile, subject1d, sz, datatype))
    if (!simple_loadimage_wrapper(callback, infile, inimg1d, sz, datatype))
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
        if(inlist->size()==1)
        {
            outfile = "ImageQuality.csv";
        }
        if(inlist->size()==2)
        {
            outfile = "ImageQualitySWC.csv";
        }
    }
    vector<QString> outfileapp;
    if(inlist->size()==1)
    {
        outfileapp.push_back(QString(infile) + "." + QString(outfile));
        cout<<"\noutput file: "<<outfileapp.at(0).toStdString().c_str()<<endl;
    }
    if(inlist->size()==2)
    {
        char * inswc = inlist->at(1);
        QString fileOpenName = QString(inswc);
        if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC"))
        {
            char * inswc = inlist->at(1);
            outfileapp.push_back(QString(inswc) + "." + QString(outfile));
            cout<<"\noutput file: "<<outfileapp.at(0).toStdString().c_str()<<endl;
        }
        else if (fileOpenName.endsWith(".ano") || fileOpenName.endsWith(".ANO"))
        {
            P_ObjectFileType linker_object;
            if (!loadAnoFile(fileOpenName,linker_object))
            {
                cout<<"Error in reading the linker file."<<endl;
                return false;
            }
            QStringList nameList = linker_object.swc_file_list;
            V3DLONG neuronNum = nameList.size();
            for (V3DLONG i=0;i<neuronNum;i++)
            {
                outfileapp.push_back(nameList.at(i) + "." + QString(outfile));
                cout<<"\noutput file: "<<outfileapp.at(i).toStdString().c_str()<<endl;
            }
        }
    }

//    // Code from datatype_convert plugin
//    V3DLONG	sz_sub = sz[0]*sz[1]*sz[2]*sz[3];
//    unsigned char * inimg1d = NULL;

//    try
//    {
//        inimg1d = new unsigned char [sz_sub];
//    }
//    catch(...)
//    {
//        printf("Error allocating memory. \n");
//        return -1;
//    }


    // Data type conversion

    int nChannel = sz[3];

    for (V3DLONG c=0;c<nChannel;c++)
    {
        if(datatype == 2)
        {
            cout << "\nConverting to 8 bit image to standardize results.\n";
            rc((unsigned short int *)inimg1d, sz, c, 0.01);
        }
        else if(datatype == 4)
        {
            cout << "\nConverting to 8 bit image to standardize results.\n";
            rc((float *)inimg1d, sz, c, 0.01);
        }
    }

//    if(datatype == 1)
//    {
//        converting<unsigned char, unsigned char>((unsigned char *)subject1d, inimg1d, sz_sub, V3D_UINT8);
//    }
//    if(datatype == 2)
//    {
//        //converting<unsigned short, unsigned char>((unsigned short *)subject1d, inimg1d, sz_sub, V3D_UINT8);
//    }
//    else if(datatype == 4)
//    {
//        //converting<float, unsigned char>((float *)subject1d, inimg1d, sz_sub, V3D_UINT8);
//    }

    //if (subject1d) {delete []subject1d; subject1d=NULL;}

    V3DLONG iters;
    if(inlist->size()==1) iters=1;
    if(inlist->size()==2) iters=nt_list.size();
    cout << nt_list.size() << "\n";
    for(V3DLONG n=0; n<iters; n++)
    {
        cout << "Iteration "<<n <<"\n";
        QList<NeuronSWC> neuron;
        if(inlist->size()==2) neuron = nt_list.at(n);
        //TODO add datatype judgment in case someone wanted to compute in 16bit
        cout << "\nInitializing variables\n";
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

        cout << "\nConverting 1D data to vector\n";
        V3DLONG mysize=0;
        vector<uint8_t> intvec;
        if(inlist->size()==1)
        {
            mysize=sz[0]*sz[1]*sz[2];
            intvec.reserve(nChannel*sz[0]*sz[1]*sz[2]);
            getVec(inimg1d, nChannel*sz[0]*sz[1]*sz[2], intvec);
        }
        if(inlist->size()==2)
        {
            int c=0;
            //cout<<neuron.size();
            mysize=neuron.size();
            intvec.reserve(neuron.size());
            getVecSWC(inimg1d, sz, c, neuron, intvec);
        }
        if (inlist->size()==1 && inimg1d) {delete []inimg1d; inimg1d=NULL;}

        cout << "\nComputing statistics\n";
        if(nChannel == 1)
        {
            int  maxint,minint;
            double meanint,medianint,madint,stdevint,pcmin,pcmax,focusscore;

            //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
            getStats(intvec, mysize, minint, maxint, meanint, medianint, madint, stdevint);
            QVector<int> tmphist;
            //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
            getHistogram(intvec, mysize, max_value, histscale, tmphist);
            pcmin = double(tmphist.at(minint))*100/double(sz[0]*sz[1]*sz[2]);
            pcmax = double(tmphist.at(maxint))*100/double(sz[0]*sz[1]*sz[2]);
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

            vector<uint8_t> otsunoise;//,otsusignal;
            double meanotsunoise,meanotsusignal,stdevotsu;//,sqsum;
            meanotsunoise=meanotsusignal=stdevotsu=0;
            vector<uint8_t> meannoise;//,meansignal;
            double avmeannoise,avmeansignal,stdevmean;
            avmeannoise=avmeansignal=stdevmean=0;
            for(int i=0; i<mysize; i++)
            {
                if(intvec.at(i)<ThreshOtsu)
                {
                    otsunoise.push_back(intvec.at(i));
                    meanotsunoise += intvec.at(i);
                }
                else
                {
                    //otsusignal.push_back(intvec.at(i));
                    meanotsusignal += intvec.at(i);
                }

                if(intvec.at(i)<meanint)
                {
                    meannoise.push_back(intvec.at(i));
                    avmeannoise += intvec.at(i);
                }
                else
                {
                    //meansignal.push_back(intvec.at(i));
                    avmeansignal += intvec.at(i);
                }
            }
            meanotsunoise = meanotsunoise/otsunoise.size();
            meanotsusignal = meanotsusignal/(intvec.size() - otsunoise.size());
            for (V3DLONG i=0;i<otsunoise.size();i++)
            {
                stdevotsu += (otsunoise.at(i) - meanotsunoise)*(otsunoise.at(i) - meanotsunoise);
            }
            stdevotsu /= otsunoise.size();
            stdevotsu = sqrt(stdevotsu);
    //        sqsum = inner_product(otsunoise.begin(),otsunoise.end(),otsunoise.begin(),0);
    //        stdevotsu = sqrt(sqsum/sz[0]*sz[1]*sz[2] - meanotsunoise*meanotsunoise);

            avmeannoise = avmeannoise/meannoise.size();
            avmeansignal = avmeansignal/(intvec.size() - meannoise.size());
            for (V3DLONG i=0;i<meannoise.size();i++)
            {
                stdevmean += (meannoise.at(i) - avmeannoise)*(meannoise.at(i) - avmeannoise);
            }
            stdevmean /= meannoise.size();
            stdevmean = sqrt(stdevmean);
            /*sqsum = inner_product(meannoise.begin(),meannoise.end(),meannoise.begin(),0);
            stdevmean = sqrt(sqsum/sz[0]*sz[1]*sz[2] -avmeannoise*avmeannoise);*/

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
                vector<uint8_t> subvec;

                //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
                if(inlist->size()==1)
                {
                    vector<uint8_t> subvec = {intvec.begin()+c*sz[0]*sz[1]*sz[2],intvec.begin()+(c+1)*sz[0]*sz[1]*sz[2]};
                }
                if(inlist->size()==1)
                {
                    getVecSWC(inimg1d, sz, c, neuron, subvec);
                }
                getStats(subvec, mysize, minint, maxint, meanint, medianint, madint, stdevint);
                QVector<int> tmphist;
                //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
                getHistogram(subvec, mysize, max_value, histscale, tmphist);
                pcmin = double(tmphist.at(minint))*100/double(sz[0]*sz[1]*sz[2]);
                pcmax = double(tmphist.at(maxint))*100/double(sz[0]*sz[1]*sz[2]);
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

                vector<uint8_t> otsunoise;//,otsusignal;
                double meanotsunoise,meanotsusignal,stdevotsu;//,sqsum;
                meanotsunoise=meanotsusignal=stdevotsu=0;
                vector<uint8_t> meannoise;//,meansignal;
                double avmeannoise,avmeansignal,stdevmean;
                avmeannoise=avmeansignal=stdevmean=0;
                for(int i=0; i<mysize; i++)
                {
                    if(subvec.at(i)<ThreshOtsu)
                    {
                        otsunoise.push_back(subvec.at(i));
                        meanotsunoise += (double)subvec.at(i);
                    }
                    else
                    {
                        //otsusignal.push_back(intvec.at(i));
                        meanotsusignal += (double)subvec.at(i);
                    }

                    if(subvec.at(i)<meanint)
                    {
                        meannoise.push_back(subvec.at(i));
                        avmeannoise += (double)subvec.at(i);
                    }
                    else
                    {
                        //meansignal.push_back(intvec.at(i));
                        avmeansignal += (double)subvec.at(i);
                    }
                }
                meanotsunoise = meanotsunoise/(double)otsunoise.size();
                meanotsusignal = meanotsusignal/((double)subvec.size() - (double)otsunoise.size());
                for (V3DLONG i=0;i<otsunoise.size();i++)
                {
                    stdevotsu += (otsunoise.at(i) - meanotsunoise)*(otsunoise.at(i) - meanotsunoise);
                }
                stdevotsu /= otsunoise.size();
                stdevotsu = sqrt(stdevotsu);
        //        sqsum = inner_product(otsunoise.begin(),otsunoise.end(),otsunoise.begin(),0);
        //        stdevotsu = sqrt(sqsum/sz[0]*sz[1]*sz[2] - meanotsunoise*meanotsunoise);

                avmeannoise = avmeannoise/(double)meannoise.size();
                avmeansignal = avmeansignal/((double)subvec.size() - (double)meannoise.size());
                for (V3DLONG i=0;i<meannoise.size();i++)
                {
                    stdevmean += (meannoise.at(i) - avmeannoise)*(meannoise.at(i) - avmeannoise);
                }
                stdevmean /= meannoise.size();
                stdevmean = sqrt(stdevmean);
                /*sqsum = inner_product(meannoise.begin(),meannoise.end(),meannoise.begin(),0);
                stdevmean = sqrt(sqsum/sz[0]*sz[1]*sz[2] -avmeannoise*avmeannoise);*/

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
        fp = fopen(outfileapp.at(n).toStdString().c_str(), "w");
        QString header = "Image_name,Channel,MinIntensity,MaxIntensity,MeanIntensity,MedianIntensity,MADIntensity,StdIntensity,PercentMinimal,PercentMaximal,ThresholdOtsu,SNR_mean,CNR_mean,SNR_otsu,CNR_otsu,FocusScore\n";
        fprintf(fp, "%s", header.toStdString().c_str());
        for (int i=0;i<hist_vec.size();i++)
        {
            fprintf(fp, "%s,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", infile,i+1,MinIntensity_vec.at(i),MaxIntensity_vec.at(i),MeanIntensity_vec.at(i),MedianIntensity_vec.at(i),MADIntensity_vec.at(i),StdIntensity_vec.at(i),PercentMinimal_vec.at(i),PercentMaximal_vec.at(i),ThreshOtsu_vec.at(i),SNRmean_vec.at(i),CNRmean_vec.at(i),SNRotsu_vec.at(i),CNRotsu_vec.at(i),FocusScore_vec.at(i));
        }
    //	for (int i=0;i<hist_vec.size();i++)
    //	{
    //		for (int j=0;j<hist_vec[i].size();j++)
    //			fprintf(fp, "%d,", hist_vec[i][j]);
    //		fprintf(fp,"\n");
    //	}
        fclose(fp);

        //if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
    }
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


// Taken from rescale_and_convert plugin
template <class T> bool rc(T* data1d, V3DLONG *sz, V3DLONG c, double apercent)
//apercent is the percentage of signal that should be upper/lower saturation
{
    if (!data1d || !sz || c<0 || c>=sz[3])
    {
        printf("problem: c=[%ld] sz=[%p] szc=[%ld], data1d=[%p]\n",c, sz, sz[3], data1d);
        return false;
    }

    V3DLONG channelsz = sz[0]*sz[1]*sz[2];
    T * outimg = (data1d + c*channelsz);

    if (apercent<0 || apercent>=0.5)
    {
        v3d_msg(QString("Your percentage parameter [%1] is wrong. Must be bwteen 0 and 0.5.\n").arg(apercent));
        return false;
    }

    V3DLONG i;
    double minval, maxval;
    minval = maxval = double(outimg[0]);
    for (i=1; i<channelsz; i++)
    {
        if (minval > outimg[i]) minval = outimg[i];
        else if (maxval < outimg[i]) maxval = outimg[i];
    }

    if (sizeof(T) == 4) //V3D_FLOAT32, or any other 32bit data; for float data , first rescale it to [0, 4095]
    {
        if (!scaleintensity(data1d, sz, c, minval, maxval, double(0), double(4095)))
            return false;
        else
        {
            minval = 0;
            maxval = 4095;
        }
    }

    {
        V3DLONG maxvv = ceil(maxval+1); //this should be safe now as the potential FLOAT32 data has been rescaled

        qDebug() << "ch =" << c << " maxvv=" << maxvv;

        double *hist = 0;
        try
        {
            hist = new double [maxvv];
        }
        catch (...)
        {
            qDebug() << "fail to allocate"; return false;
            v3d_msg("Fail to allocate memory.\n");
            return false;
        }

        for (i=0;i<maxvv;i++)
        {
            hist[i] = 0;
        }

        //find the histogram
        for (i=0;i<channelsz;i++)
            hist[V3DLONG(outimg[i])] += 1;

        qDebug() << "Histogram computed.";

        //compute the CDF
        for (i=1;i<maxvv;i++)
        {
            hist[i] += hist[i-1];
        }

        for (i=0;i<maxvv;i++)
        {
            hist[i] /= hist[maxvv-1];
        }

        //now search for the intensity thresholds
        double lowerth, upperth; lowerth = upperth = 0;
        for (i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
        {
            if (hist[i]<apercent && hist[i+1]>apercent)
                lowerth = i;
            if (hist[i]<1-apercent && hist[i+1]>1-apercent)
                upperth = i;
        }

        v3d_msg(QString("channel=%1 lower th=%2 upper th=%3").arg(c).arg(lowerth).arg(upperth), 0);

        //real rescale of intensity
        scaleintensity(data1d, sz, c, lowerth, upperth, double(0), double(255));

        //free space
        if (hist) {delete []hist; hist=0;}
    }

    return true;
}

template <class T> bool scaleintensity(T *img, V3DLONG sz[4], V3DLONG channo, double lower_th, double higher_th, double target_min, double target_max)
//map the value linear from [lower_th, higher_th] to [target_min, target_max]
{
    if (!img || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || channo>=sz[3]) //channo < 0 is reserved to mean all channels
    {
        v3d_msg("Invalid parameters in scaleintensity();\n");
        return false;
    }

    double t;
    if (lower_th>higher_th) {t=lower_th; lower_th=higher_th; higher_th=t;}
    if (target_min>target_max) {t=target_min; target_min=target_max; target_max=t;}

    double rate = (higher_th==lower_th) ? 1 : (target_max-target_min)/(higher_th-lower_th); //if the two th vals equal, then later-on t-lower_th will be 0 anyway

    V3DLONG i,j,k,c;

    V3DLONG channelPageSize = sz[0]*sz[1]*sz[2];
    switch ( sizeof(T) )
    {
        case 1:
        case 2:
        case 4:
            for (c=0;c<sz[3];c++)
            {
                if (channo>=0 && c!=channo)
                    continue;
                T *data = img + c*channelPageSize;
                for (i=0;i<channelPageSize;i++)
                {
                    t = data[i];
                    if (t>higher_th) t=higher_th;
                    else if (t<lower_th) t=lower_th;
                    data[i] = (T)((t - lower_th)*rate + target_min);
                }
            }
            break;

        default:
            v3d_msg("invalid datatype in scaleintensity();\n", 0);
            return false;
    }

    return true;
}




#undef INF
