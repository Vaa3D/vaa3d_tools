/* Opencv_example_func.cpp
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */

#include "LCM_boost_func.h"

//#define INTENSITY_DISTANCE_METHOD 0

using namespace std;
using namespace cv;
using namespace cv::ml;

#define debug_on 0

#define debug_on1 0

#define debug_on2 0

#define debug_on3 0

#define debug_on4 0

#define debug_on5 0



const QString title = QObject::tr("Load Image And SWC");

#define GET_CURRENT_IMAGE_AND_SWC(inimg1d, in_sz, nt) \
{\
	v3dhandle curwin = callback.currentImageWindow(); \
	Image4DSimple *p4DImage = callback.getImage(curwin); \
	nt = callback.getSWC(curwin);\
	inimg1d = p4DImage->getRawData();\
	in_sz = new V3DLONG[4]; \
	in_sz[0] = p4DImage->getXDim(); \
	in_sz[1] = p4DImage->getYDim();\
	in_sz[2] = p4DImage->getZDim();\
	in_sz[3] = p4DImage->getCDim();\
	ImagePixelType datatype = p4DImage->getDatatype();\
}

#define SHOW_OUT_IMAGE_AND_SWC(outimg1d, out_sz, nt, title) \
{\
	Image4DSimple * p4DImage = new Image4DSimple();\
	p4DImage->setData(outimg1d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);\
	v3dhandle newwin = callback.newImageWindow();\
	callback.setImage(newwin, p4DImage);\
	callback.setImageName(newwin, QObject::tr(title));\
	callback.setSWC(newwin, nt);\
	callback.open3DWindow(newwin);\
	callback.getView3DControl(newwin)->updateWithTriView();\
}

string file_type(string para)
{
    cout << " enter the file_type " << endl;

	int pos = para.find_last_of(".");

	cout << (double) pos << endl;

	if(pos == string::npos) return string("");
	else return para.substr(pos, para.size() - pos);
}

bool Opencv_example(V3DPluginCallback2 &callback, QWidget *parent)
{
	if(callback.getImageWindowList().empty()) return QMessageBox::information(0, title, QObject::tr("No image is open."));

	unsigned char * data1d = 0;
	V3DLONG * in_sz = 0;
	NeuronTree nt;

	v3dhandle curwin = callback.currentImageWindow();

	Image4DSimple *p4DImage = callback.getImage(curwin);

	data1d = p4DImage->getRawData();

	in_sz = new V3DLONG[4];

	in_sz[0] = p4DImage->getXDim();

	in_sz[1] = p4DImage->getYDim();

	in_sz[2] = p4DImage->getZDim();

	in_sz[3] = p4DImage->getCDim();

	ImagePixelType datatype = p4DImage->getDatatype();


    // shrink the iamge if possible

    double * zv = new double[in_sz[2]];

    double * yv = new double[in_sz[1]];

    double * xv = new double[in_sz[0]];

    for(int i = 0; i < (int)in_sz[2]; i++)
        zv[i] = 0;

    for(int i = 0; i < (int)in_sz[1]; i++)
        yv[i] = 0;

    for(int i = 0; i < (int)in_sz[0]; i++)
        xv[i] = 0;

    cout << "Initialise " << endl;

    for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
    {

        V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
        for(V3DLONG iy = 0; iy <  in_sz[1]; iy++)
        {
            V3DLONG offsetj = iy*in_sz[0];
            for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
            {

                unsigned char PixelValue = data1d[offsetk + offsetj + ix];

                double p = (double)PixelValue;


                xv[ix] = xv[ix] + p;

                yv[iy] = yv[iy] + p;

                zv[iz] = zv[iz] + p;

            }
        }

    }


      for(int i = 0; i < (int)in_sz[2]; i++)
      {

        if(zv[i] > 1000)
            zv[i] = 1000;

     //   zv[i] = zv[i] / (double)(sz[0] * sz[1]);

        //cout << "zv "  << i << " is " << zv[i] << endl;
       }


    for(int i = 0; i < (int)in_sz[1]; i++)
    {

            if(yv[i] > 1000)
            yv[i] = 1000;

       // yv[i] = yv[i] / (do)uble)(sz[0] * sz[2]);

      //cout << "yv is " << yv[i] << endl;

    }

    for(int i = 0; i < (int)in_sz[0]; i++)
    {

        //xv[i] = xv[i] / (double)(sz[1] * sz[2]);
            if(xv[i] > 1000)
            xv[i] = 1000;


    }



    double rng[1] = {1000};

    int bx[2];

    int by[2];

    int bz[2];

    input_boundary(xv, in_sz[0],rng, bx, 5);

    cout << "....................................................." << endl;

    cout << "x1 and x2 is " << bx[0] << "  " << bx[1] << endl;


    input_boundary(yv, in_sz[1],rng, by, 5);

    cout << "....................................................." << endl;

    cout << "y1 and y2 is " << by[0] << "  " << by[1] << endl;

    input_boundary(zv, in_sz[2],rng, bz, 3);

    cout << "....................................................." << endl;

    cout << "z1 and z2 is " << bz[0] << "  " << bz[1] << endl;


    int img_sz_new[3];

    img_sz_new[0] = bx[1] - bx[0];

    img_sz_new[1] = by[1] - by[0];

    img_sz_new[2] = bz[1] - bz[0];


    Mat image = Mat(3,img_sz_new,CV_8UC1,Scalar::all(0));


    for(int z = 0; z < image.size[2]; z++)
    {


        V3DLONG offsetk = (V3DLONG)(z + bz[0])*in_sz[1]*in_sz[0];

         for(int y = 0; y < image.size[1]; y ++)
         {

            V3DLONG offsetj = (V3DLONG)(y + by[0])*in_sz[0];

            for(int x = 0; x < image.size[0]; x++)
            {

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                unsigned char PixelValue = data1d[offsetk + offsetj + (V3DLONG)(x + bx[0])];

                image.at<uchar>(v3) = PixelValue;


            }

         }

    }

    delete [] data1d;

    char *tmp_nm = new char[100];

    sprintf(tmp_nm,"tmp_cache_img.v3draw");

    saveMat(image,tmp_nm);

    Mat conf_img;

    //uchar * conf_img1;

    //image.release();

    // now use either the multiscale enhancement or the GWDT to collect
    // the base image

    int t1,t2;


    char * dataset = new char[100];

    cout << "......................" << endl;

    cout << "Start the base method" << endl;

    if(1)
    {
        multiscaleEhance(callback, tmp_nm, conf_img);


        t1 = 20;

        t2 = 5;


        //t1 = 80;

        //t2 = 20;

        sprintf(dataset,"OPFmEh");


    }else
    {
        fastMarch(tmp_nm, conf_img);

        t1 = 7;

        t2 = 3;

        sprintf(dataset,"OPFFM");

    }

    cout << "Complete the base method" << endl;

    Mat seg_img;

    LCM_boost(image, conf_img,seg_img,t1,t2,dataset);

    cout << "complete the LCM " << endl;

    // output the result into the harddisk

    sprintf(tmp_nm,"neuron_segmentation.v3draw");

    saveMat(seg_img,tmp_nm);

    // trace the image

    sprintf(tmp_nm,"neuron_swc.swc");

    trace_img(seg_img, image, tmp_nm);

    cout << "The traced result has been saved in neuron_swc.swc" << endl;

    //v3d_msg(tr("The traced result has been saved in neuron_swc.swc"));

    //v3d_msg(tr("This plugin will boost the performance of the existing methods"));

    //nt = readSWC_file((char *)tmp_nm);

/*

    uchar * show1d = new uchar[in_sz[0] * in_sz[1] * in_sz[2]];

    for(int z = 0; z < image.size[2]; z++)
    {

        V3DLONG offsetk = (V3DLONG)z*in_sz[1]*in_sz[0];

         for(int y = 0; y < image.size[1]; y ++)
         {

            V3DLONG offsetj = (V3DLONG)y*in_sz[0];

            for(int x = 0; x < image.size[0]; x++)
            {

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                unsigned char PixelValue = image.at<uchar>(v3);

                show1d[offsetk + offsetj + (V3DLONG)x] = PixelValue;

            }

         }

    }



	V3DLONG in_sz1[4];

	in_sz1[0] = (V3DLONG)image.size[0];

	in_sz1[1] = (V3DLONG)image.size[1];

	in_sz1[2] = (V3DLONG)image.size[2];

	in_sz1[3] = 1;



        // start visualising the image

    v3dhandle newwin = callback.newImageWindow();

    Image4DSimple *p4DImage_new;

    p4DImage_new->setData(show1d, in_sz1[0], in_sz1[1], in_sz1[2], 1, p4DImage->getDatatype());

    callback.setImage(newwin, p4DImage_new);

    callback.setImageName(newwin, QObject::tr("Boosted Image"));

   // callback.setSWC(newwin, nt);

    callback.updateImageWindow(newwin);

    delete []  show1d;
*/

/*

	int img_sz[3];

	for(int i = 0; i < 3; i++)
        img_sz[i] = (int)in_sz[i];

    Mat label_img = Mat(3,img_sz,CV_32S,Scalar::all(0));

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = (int)in_sz[i];

    image = Mat(3,sz,CV_8UC1,Scalar::all(0));

    for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
    {

        V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
        for(V3DLONG iy = 0; iy <  in_sz[1]; iy++)
        {
            V3DLONG offsetj = iy*in_sz[0];
            for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
            {

                unsigned char PixelValue = inimg[offsetk + offsetj + ix];

                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                image.at<uchar>(v3) = PixelValue;

            }
        }

    }

    delete [] inimg;

    delete [] in_sz;


    Mat conf_img;

    // multiscaleEhance(callback, infile, conf_img);



    // process the labelling image

    cout << "Start processing  the image" << endl;

    LCM_boost(label_img, );

    cout << "Complete processing  the image" << endl;


    Mat show_img;

    label_img.convertTo(show_img,CV_8UC1);



*/

    return true;
}

#define READ_IMAGE_AND_SWC_FROM_INPUT(inimg1d, in_sz, nt) \
{\
	if(input.empty()) return false;\
	vector<char*> in_str = *((vector<char*> *)(input.at(0).p));\
	if(in_str.size() < 2){cerr<<"Error: no enough input."<<endl; return false;}\
	string inimg_file, inswc_file;\
	string ftp1 = file_type(in_str[0]);\
	string ftp2 = file_type(in_str[1]);\
	if(ftp1 == ".raw" || ftp1 == ".tif" || ftp1 == ".tiff" || ftp1 == ".lsm") inimg_file = in_str[0];\
	else {if(ftp1 == ".swc") inswc_file = in_str[0];}\
	if(ftp2 == ".raw" || ftp2 == ".tif" || ftp2 == ".tiff" || ftp2 == ".lsm") inimg_file = in_str[1];\
	else {if(ftp2 == ".swc") inswc_file = in_str[1];}\
	if(inimg_file == "" || inswc_file == "") {cerr<<"Error : either image file or swc file is missing."<<endl; return false;}\
	int datatype;\
	loadImage((char*) inimg_file.c_str(), inimg1d, in_sz, datatype);\
	nt = readSWC_file((char*)inswc_file.c_str());\
}

int input_boundary(double * xv, int sz, double rng[], int bnd[], int bf)
{

    double *x1, *x2;

    x1 = find_first_of(xv,xv + sz,rng,rng + 1);

    x2 = find_end(xv,xv + sz,rng,rng + 1);

    //int x1i,x2i;

    bnd[0] = distance(xv,x1);

    bnd[0] = bnd[0] - bf;

    bnd[0] = max(bnd[0],0);

    bnd[1] = distance(xv,x2);


    bnd[1] = bnd[1] + bf;

    bnd[1] = min(bnd[1], sz - 1);



    //bnd[0] = x1i;


}

int saveImg(float *img_ptr, int img_sz[], char * fileName_string)
{

	V3DLONG *sz = new V3DLONG[4];

	for(int i =0; i < 3; i++)
		sz[i] = (V3DLONG)img_sz[i];

	sz[3] = 1;

	int datatype = 1;

	int img_pg = img_sz[0] * img_sz[1] * img_sz[2];

	uchar * show_img = new uchar[img_pg];

	for(int i = 0; i < img_pg; i++)
	{
		if(img_ptr[i] > 254)
			show_img[i] = 254;
		else
			show_img[i] = (uchar)img_ptr[i];

	}

	saveImage(fileName_string, show_img, sz, datatype);

	delete [] show_img;




}


int saveMat(cv::Mat image, char * fileName_string)
{
        V3DLONG *sz = new V3DLONG[4];

        for(int i =0; i < 3; i++)
            sz[i] = (V3DLONG)image.size[i];

         sz[3] = 1;

         int datatype = 1;

         int img_pg = sz[0] * sz[1] * sz[2];

         uchar * show_img = new uchar[img_pg];

     for(V3DLONG iz = 0; iz < sz[2]; iz++)
    {

        V3DLONG offsetk = iz*sz[1]*sz[0];
        for(V3DLONG iy = 0; iy <  sz[1]; iy++)
        {
            V3DLONG offsetj = iy* sz[0];
            for(V3DLONG ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                unsigned char PixelValue = image.at<uchar>(v3);

                show_img[offsetk + offsetj + ix] = PixelValue;

            }
        }

    }

    saveImage(fileName_string, show_img, sz, datatype);

    delete [] show_img;


}


int trace_img(Mat seg_img, Mat image, char * outfile_swc)
{
    V3DLONG *sz = new V3DLONG[4];

    for(int i =0; i < 3; i++)
        sz[i] = (V3DLONG)seg_img.size[i];

    int img_sz[3];

    for(int i =0; i < 3; i++)
        img_sz[i] = seg_img.size[i];


    sz[3] = 1;

    int datatype = 1;

    int img_pg = img_sz[0] * img_sz[1] * img_sz[2];

    uchar * show_img = new uchar[img_pg];


    // subsititute the filled part with the average value

    // first get the average value

    int n_base = 0;

    int mean_base = 0;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {

        int offsetk = iz*img_sz[1]*img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy* img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {


                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                unsigned char PixelValue = seg_img.at<uchar>(v3);

                if(PixelValue > 150)
                {
                    n_base ++;

                    mean_base = mean_base + (int)image.at<uchar>(v3);

                }

            }
        }

    }

    mean_base = mean_base / n_base;

    cout << "The mean value of image is " << mean_base << endl;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {

        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {


                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                unsigned char PixelValue = seg_img.at<uchar>(v3);
                //unsigned char PV = image.at<uchar>(v3);


                if(PixelValue > 10)
                {

                    if(PixelValue > 150)
                    {
                        //show_img[offsetk + offsetj + ix] = image.at<uchar>(v3);

                        show_img[offsetk + offsetj + ix] = (uchar)mean_base;
                    }
                    else
                    {
                        show_img[offsetk + offsetj + ix] = (uchar)mean_base;

                    }
                }
                else
                {

                    show_img[offsetk + offsetj + ix] = 0;

                }

            }
        }

    }

    // extract the swc file from the segmented image

    vector<MyMarker *> outtree;

    cout<<"Start detecting cellbody"<<endl;

    float * phi = 0;
    vector<MyMarker> inmarkers;

    fastmarching_dt_XY(show_img, phi, seg_img.size[0], seg_img.size[1], seg_img.size[2],2, 10);

    int in_sz[3];

    in_sz[0] = sz[0];

    in_sz[1] = sz[1];

    in_sz[2] = sz[2];


    V3DLONG sz0 = sz[0];
    V3DLONG sz1 = sz[1];
    V3DLONG sz2 = sz[2];
    V3DLONG sz01 = sz0 * sz1;
    V3DLONG tol_sz = sz01 * sz2;

    V3DLONG max_loc = 0;
    double max_val = phi[0];
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

    fastmarching_tree(inmarkers[0], show_img, outtree, sz[0], sz[1], sz[2], 2, 10, false);
    cout<<"======================================="<<endl;


    //save a copy of the constructed tree
    cout<<"Save the reconstruced tree"<<endl;
    vector<MyMarker*> & inswc = outtree;

    double dfactor_xy = 1, dfactor_z = 1;


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

        int sz_swc = inswc.size();

        cout << sz_swc << endl;


        //saveSWC_file(outfile_swc, inswc);

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


    inmarkers[0].x *= dfactor_xy;

    inmarkers[0].y *= dfactor_xy;

    inmarkers[0].z *= dfactor_z;


    vector<MyMarker*> outswc;

    happ(inswc, outswc, show_img, sz[0], sz[1], sz[2],10, 5, 0.3);
  //  v3d_msg("start to use happ.\n", 0);
    //happ(inswc, outswc, show_img, in_sz[0], in_sz[1], in_sz[2],10, 5, 0.3333);

 //   if (p4dImageNew) {delete p4dImageNew; p4dImageNew=0;} //free buffe

    inmarkers[0].x *= dfactor_xy;
    inmarkers[0].y *= dfactor_xy;
    inmarkers[0].z *= dfactor_z;


    for(V3DLONG i = 0; i < outswc.size(); i++)
    {
        if (dfactor_xy>1) outswc[i]->x *= dfactor_xy;
        outswc[i]->x += 0;
        if (dfactor_xy>1) outswc[i]->x += dfactor_xy/2;

        if (dfactor_xy>1) outswc[i]->y *= dfactor_xy;
        outswc[i]->y += 0;
        if (dfactor_xy>1) outswc[i]->y += dfactor_xy/2;

        if (dfactor_z>1) outswc[i]->z *= dfactor_z;
        outswc[i]->z += 0;
        if (dfactor_z>1)  outswc[i]->z += dfactor_z/2;

        outswc[i]->radius *= dfactor_xy; //use xy for now
    }

    //re-estimate the radius using the original image
    double real_thres = 40;


   V3DLONG szOriginalData[4] = {sz0,sz1,sz2, 1};

    int method_radius_est = 2;
    for(V3DLONG i = 0; i < outswc.size(); i++)
    {
        //printf(" node %ld of %ld.\n", i, outswc.size());
        outswc[i]->radius = markerRadius(show_img, szOriginalData, *(outswc[i]), real_thres, method_radius_est);
    }

   saveSWC_file(outfile_swc, outswc);


   delete [] show_img;

   //delete [] phi;


     if(phi){delete [] phi; phi = 0;}
    for(V3DLONG i = 0; i < outtree.size(); i++) delete outtree[i];
    outtree.clear();

  //  if(data1d_1ch){delete []data1d_1ch; data1d_1ch = 0;}


//    delete [] phi;

    return 1;




}


int roi_img(cv::Mat &image,cv::Mat image1)
{

    double * zv = new double[image1.size[2]];

    double * yv = new double[image1.size[1]];

    double * xv = new double[image1.size[0]];

    for(int i = 0; i < image1.size[2]; i++)
        zv[i] = 0;

    for(int i = 0; i < image1.size[1]; i++)
        yv[i] = 0;

    for(int i = 0; i < image1.size[0]; i++)
        xv[i] = 0;

    for(int z = 0; z < image1.size[2]; z++)
    {

         for(int y = 0; y < image1.size[1]; y ++)
         {

            for(int x = 0; x < image1.size[0]; x++)
            {

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                double p = (double)image1.at<uchar>(v3);

                xv[x] = xv[x] + p;

                yv[y] = yv[y] + p;

                zv[z] = zv[z] + p;


            }

         }

    }


    for(int i = 0; i < image1.size[0]; i++)
    {
    	if(xv[i] < 1000)
    		xv[i] = 0;
    	else
    		xv[i] = 1000;

    }


    for(int i = 0; i < image1.size[1]; i++)
    {
    	if(yv[i] < 1000)
    		yv[i] = 0;
    	else
    		yv[i] = 1000;

    }


    for(int i = 0; i < image1.size[2]; i++)
    {
    	if(zv[i] < 1000)
    		zv[i] = 0;
    	else
    		zv[i] = 1000;

    }


    double rng[1] = {1000};

    int bx[2];

    int by[2];

    int bz[2];

    input_boundary(xv, image1.size[0],rng, bx, 5);

    input_boundary(yv, image1.size[1],rng, by, 5);

    input_boundary(zv, image1.size[2],rng, bz, 3);

    delete [] xv;

    delete [] yv;

    delete [] zv;



    int img_sz_new[3];

    img_sz_new[0] = bx[1] - bx[0];

    img_sz_new[1] = by[1] - by[0];

    img_sz_new[2] = bz[1] - bz[0];




   // cout << "bx is " << bx[0] << " " << bx[1] << endl;

   // cout << "by is " << by[0] << " " << by[1] << endl;

  //  cout << "bz is " << bz[0] << " " << bz[1] << endl;

    image = Mat(3,img_sz_new,CV_8UC1,Scalar::all(0));

    for(int z = 0; z < image.size[2]; z++)
    {

         for(int y = 0; y < image.size[1]; y ++)
         {

            for(int x = 0; x < image.size[0]; x++)
            {

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                int v3ip[3];

                v3ip[0] = x + bx[0];

                v3ip[1] = y + by[0];

                v3ip[2] = z + bz[0];

                image.at<uchar>(v3) = image1.at<uchar>(v3ip);


            }

         }

    }

    return 1;

}

//./vaa3d -x Opencv_example -f Opencv_example -i B5.v3draw -o tmp_tst.v3draw tmp_tst.swc

// call the function to batch process  the whole data
bool Batch_Process(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	unsigned char * inimg1d = 0;


	if(input.empty()) return false;


    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);

        if (inlist->size() != 1)
        {
                cout<<"You must specify 1 input file!"<<endl;
                return -1;
        }

    char * infile = inlist->at(0);

    cout << "The input is " << infile << endl;


    for(int i_img = 1; i_img < 51; i_img ++)
    {


        ostringstream convert;

    	convert << i_img;

    	string im_file = infile;

    	im_file = "/media/gulin/E402023602020DEC/Data/BigN/" + im_file + convert.str() + ".v3draw";

    	string roi_fn = infile;

    	roi_fn = "/media/gulin/E402023602020DEC/Data/Roi/roi_" + roi_fn + convert.str() + ".v3draw";

        string save_seg_fn = infile;

     	save_seg_fn = "/media/gulin/E402023602020DEC/Data/Seg/seg_" + save_seg_fn + convert.str() + ".v3draw";


        string save_swc_fn = infile;

     	save_swc_fn = "/media/gulin/E402023602020DEC/Data/SWC/swc_" + save_swc_fn + convert.str() + ".swc";



    //	string base_fn1 = infile;

    //	base_fn1 = "/media/gulin/E402023602020DEC/Data/mE/mE_" + base_fn1 + convert.str() + ".v3draw";

    //	string base_fn2 = infile;

    //	base_fn2 = "/media/gulin/E402023602020DEC/Data/FM/FM_" + base_fn2 + convert.str() + ".v3draw";

    //	string roi_fn = infile;

    //	roi_fn = "/media/gulin/E402023602020DEC/Data/Roi/roi_" + roi_fn + convert.str() + ".v3draw";

    //	string sav_mat = infile;

    //	sav_mat = "/media/gulin/E402023602020DEC/Data/Mat_sav/mat_" + sav_mat + convert.str() + ".yml";


        Mat image1;

        loadMat(image1, (char*)im_file.c_str());


     	Mat image;

    	roi_img(image,image1);

        image1.release();


        saveMat(image,(char*)roi_fn.c_str());

        Mat conf_img;

        int t1,t2;

        char * dataset = new char[100];

        cout << "......................" << endl;

        cout << "Start the base method" << endl;

        int img_pg = image.size[0] * image.size[1];

        if(1)
        {
            if(img_pg > (300 * 250))
            {
                t1 = 50;

                t2 = 5;

            }else
            {

                if(img_pg > (200 * 200))
                    {
                        t1 = 20;

                        t2 = 5;

                    }
                    else
                    {
                        t1 = 10;

                        t2 = 5;

                    }

            }

            multiscaleEhance(callback, (char*)roi_fn.c_str(), conf_img);


            sprintf(dataset,"OPFmEh");


        }
        else
        {
            fastMarch((char*)roi_fn.c_str(), conf_img);

            t1 = 7;

            t2 = 3;

            sprintf(dataset,"OPFFM");

        }


        cout << "Complete the base method" << endl;

        Mat seg_img;

        LCM_boost(image, conf_img,seg_img,t1,t2,dataset);

        cout << "complete the LCM " << endl;

        // output the result into the harddisk

        saveMat(seg_img,(char*)save_seg_fn.c_str());

        // trace the image

        trace_img(seg_img, image, (char*)save_swc_fn.c_str());

       // cin.get();


    }

	return true;
}


bool Opencv_example(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	unsigned char * inimg1d = 0;


	if(input.empty()) return false;


    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);

        if (inlist->size() != 1)
        {
                cout<<"You must specify 1 input file!"<<endl;
                return -1;
        }

    char * infile = inlist->at(0);


        vector<char*>* outlist = (vector<char*>*)(output.at(0).p);

        if (outlist->size() != 2)
        {
                cout<<"You must specify 2 output file!"<<endl;
                return -1;
        }

        char * outfile = outlist->at(0);

        char * outfile_swc = outlist->at(1);

        cout<<"output file: "<<outfile<<endl;




    Mat image1;

    loadMat(image1, infile);

    // shrink the iamge if possible

    double * zv = new double[image1.size[2]];

    double * yv = new double[image1.size[1]];

    double * xv = new double[image1.size[0]];

    for(int i = 0; i < image1.size[2]; i++)
        zv[i] = 0;

    for(int i = 0; i < image1.size[1]; i++)
        yv[i] = 0;

    for(int i = 0; i < image1.size[0]; i++)
        xv[i] = 0;

    cout << "Initialise " << endl;


    for(int z = 0; z < image1.size[2]; z++)
    {

         for(int y = 0; y < image1.size[1]; y ++)
         {

            for(int x = 0; x < image1.size[0]; x++)
            {

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                double p = (double)image1.at<uchar>(v3);

                xv[x] = xv[x] + p;

                yv[y] = yv[y] + p;

                zv[z] = zv[z] + p;


            }

         }

    }


      for(int i = 0; i < image1.size[2]; i++)
      {

        if(zv[i] > 1000)
            zv[i] = 1000;

     //   zv[i] = zv[i] / (double)(sz[0] * sz[1]);

        //cout << "zv "  << i << " is " << zv[i] << endl;
       }


    for(int i = 0; i < image1.size[1]; i++)
    {

            if(yv[i] > 1000)
            yv[i] = 1000;

       // yv[i] = yv[i] / (do)uble)(sz[0] * sz[2]);

      //cout << "yv is " << yv[i] << endl;

    }

    for(int i = 0; i < image1.size[0]; i++)
    {

        //xv[i] = xv[i] / (double)(sz[1] * sz[2]);
            if(xv[i] > 1000)
            xv[i] = 1000;


    }



    double rng[1] = {1000};

    int bx[2];

    int by[2];

    int bz[2];

    input_boundary(xv, image1.size[0],rng, bx, 5);

    cout << "....................................................." << endl;

    cout << "x1 and x2 is " << bx[0] << "  " << bx[1] << endl;


    input_boundary(yv, image1.size[1],rng, by, 5);

    cout << "....................................................." << endl;

    cout << "y1 and y2 is " << by[0] << "  " << by[1] << endl;

    input_boundary(zv, image1.size[2],rng, bz, 3);

    cout << "....................................................." << endl;

    cout << "z1 and z2 is " << bz[0] << "  " << bz[1] << endl;


    int img_sz_new[3];

    img_sz_new[0] = bx[1] - bx[0];

    img_sz_new[1] = by[1] - by[0];

    img_sz_new[2] = bz[1] - bz[0];



    Mat image = Mat(3,img_sz_new,CV_8UC1,Scalar::all(0));


    for(int z = 0; z < image.size[2]; z++)
    {

         for(int y = 0; y < image.size[1]; y ++)
         {

            for(int x = 0; x < image.size[0]; x++)
            {

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                int v3ip[3];

                v3ip[0] = x + bx[0];

                v3ip[1] = y + by[0];

                v3ip[2] = z + bz[0];

                image.at<uchar>(v3) = image1.at<uchar>(v3ip);


            }

         }

    }

    image1.release();


    char *tmp_nm = new char[100];

    sprintf(tmp_nm,"tmp_cache_img.v3draw");


    saveMat(image,tmp_nm);

 /*


    loadMat(image1,infile);

    cout << "............................" << endl;


    cout << image1.size[0] << endl;

    cout << image.size[0] << endl;


    cout << image1.size[1] << endl;

    cout << image.size[1] << endl;


    cout << image1.size[2] << endl;

    cout << image.size[2] << endl;

    double sum_value = (double)std::accumulate(image.begin<uchar>(), image.end<uchar>(), 0.0);

    cout << "sum value is " << sum_value << endl;


    sum_value = (double)std::accumulate(image1.begin<uchar>(), image1.end<uchar>(), 0.0);

    cout << "sum value is " << sum_value << endl;


*/


    Mat conf_img;

    //uchar * conf_img1;

    //image.release();

    // now use either the multiscale enhancement or the GWDT to collect
    // the base image

    int t1,t2;


    char * dataset = new char[100];

    cout << "......................" << endl;

    cout << "Start the base method" << endl;

    if(1)
    {
        multiscaleEhance(callback, tmp_nm, conf_img);


        t1 = 20;

        t2 = 5;


        //t1 = 80;

        //t2 = 20;

        sprintf(dataset,"OPFmEh");


    }else
    {
        fastMarch(tmp_nm, conf_img);

        t1 = 7;

        t2 = 3;

        sprintf(dataset,"OPFFM");

    }

    cout << "Complete the base method" << endl;

    Mat seg_img;

    LCM_boost(image, conf_img,seg_img,t1,t2,dataset);

    cout << "complete the LCM " << endl;

    // output the result into the harddisk

    saveMat(seg_img,outfile);

    // trace the image

    trace_img(seg_img, image, outfile_swc);

	return true;
}

int get_main_branch(cv::Mat image, cv::Mat &seg_img, int t1, int & base_t)
{

	int img_sz[3];

	img_sz[0] = (int)image.size[0];

	img_sz[1] = (int)image.size[1];

	img_sz[2] = (int)image.size[2];

	uchar t1c = (uchar)t1;

	for(int iz = 0; iz < img_sz[2]; iz++)
	{

		for(int iy = 0; iy <  img_sz[1]; iy++)
		{

			for(int ix = 0; ix < img_sz[0]; ix++)
			{


				int v3[3];

				v3[0] = ix;

				v3[1] = iy;

				v3[2] = iz;

				uchar PixelValue = image.at<uchar>(v3);

				if(PixelValue > t1c)
				{

					seg_img.at<uchar>(v3) = 254;

				}

			}
		}

	}

//	cout << "Ready to do the labelling" << endl;

	Mat label_img;

	int max_label;

	max_label = bwconnmp_img3(seg_img,label_img,254);

//	cout << "There are " << max_label << " components in the base image" << endl;

	// clean the small pixels

	// find how many labels are there in the image

	vector<vector<int> > PixelIdxListx(max_label);

	vector<vector<int> > PixelIdxListy(max_label);

	vector<vector<int> > PixelIdxListz(max_label);

	bwconnmp(label_img,PixelIdxListx,PixelIdxListy,PixelIdxListz);

	label_img.release();

	//int base_t = 10;

	int n_mb = 0;

	int *npl = new int[max_label];

	for(int i = 0; i < max_label; i++)
        npl[i] = PixelIdxListx[i].size();

	sort(npl,npl + max_label);

    int base_i = 0;

    base_i = max(max_label - 20,0);

	base_t = max(npl[base_i],10);


	cout << "The base threshold is " << base_t << endl;

    delete [] npl;

	for(int i = 0; i < max_label; i++)
	{

		int n_p = PixelIdxListx[i].size();

		if(n_p < base_t)
		{

			for(int j = 0; j < n_p; j ++)
			{
				int v3[3];

				v3[0] = PixelIdxListx[i][j];

				v3[1] = PixelIdxListy[i][j];

				v3[2] = PixelIdxListz[i][j];

				seg_img.at<uchar>(v3) = 0;

			}

		}
		else
			n_mb ++;


	}

	return n_mb;

}


int filter_small_comp(cv::Mat &seg_img, int thres)
{

	int img_sz[3];

	img_sz[0] = (int)seg_img.size[0];

	img_sz[1] = (int)seg_img.size[1];

	img_sz[2] = (int)seg_img.size[2];

	//uchar t1c = (uchar)t1;

    Mat label_img;

	int max_label;

	max_label = bwconnmp_img4(seg_img,label_img,1);

//	cout << "There are " << max_label << " components in the base image" << endl;

	// clean the small pixels

	// find how many labels are there in the image

	vector<vector<int> > PixelIdxListx(max_label);

	vector<vector<int> > PixelIdxListy(max_label);

	vector<vector<int> > PixelIdxListz(max_label);

	bwconnmp(label_img,PixelIdxListx,PixelIdxListy,PixelIdxListz);

	label_img.release();

	int base_t = 100;

	int n_mb = 0;

	for(int i = 0; i < max_label; i++)
	{

		int n_p = PixelIdxListx[i].size();

		if(n_p < thres)
		{

			for(int j = 0; j < n_p; j ++)
			{
				int v3[3];

				v3[0] = PixelIdxListx[i][j];

				v3[1] = PixelIdxListy[i][j];

				v3[2] = PixelIdxListz[i][j];

				seg_img.at<uchar>(v3) = 0;

			}

		}
		else
			n_mb ++;


	}

	return n_mb;

}



int find_t2_fragments(cv::Mat image, cv::Mat &seg_img, int t2, float dt){

    cout << "Enter the find_t2_fragments" << endl;

	int sz[3];

	for(int i = 0; i < 3; i++)
		sz[i] = image.size[i];

	uchar t2c = (uchar)t2;

	float * dist_t1 = 0;

	cout << "Ready to do the bwdist" << endl;

	bwdist(seg_img,dist_t1,100,dt + 1);

	cout << "Complete the bwdist" << endl;

	for(int iz = 0; iz < sz[2]; iz++)
	{

		int offsetk = iz * sz[1] * sz[0];
		for(int iy = 0; iy <  sz[1]; iy++)
		{
			int offsetj = iy * sz[0];
			for(int ix = 0; ix < sz[0]; ix++)
			{


				int v3[3];

				v3[0] = ix;

				v3[1] = iy;

				v3[2] = iz;

				float dist_b = dist_t1[offsetk + offsetj + ix];

				uchar PixelValue = image.at<uchar>(v3);

				uchar SegValue = seg_img.at<uchar>(v3);

				if((PixelValue > t2c) & ( dist_b > dt ) & (SegValue < 1))
				{
					seg_img.at<uchar>(v3) = 80;

				}

			}

		}
	}

	delete [] dist_t1;

    return 1;

}

int collect_idx_c(Mat seg_img, vector<trip_idx> &Pseg, uchar tc)
{

	int img_sz[3];

	for(int i = 0;  i < 3;  i++)
		img_sz[i] = seg_img.size[i];

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
      //  cout << "iz is " << iz << endl;
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                uchar PixelValue = seg_img.at<uchar>(v3);

                if(PixelValue == tc)
                {
                    Pseg.push_back(trip_idx(ix,iy,iz));
                }

            }

        }
    }
}

int collect_idx_int(Mat seg_img, vector<trip_idx> &Pseg, int tc)
{

	int img_sz[3];

	for(int i = 0;  i < 3;  i++)
		img_sz[i] = seg_img.size[i];

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
      //  cout << "iz is " << iz << endl;
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                int PixelValue = seg_img.at<int>(v3);

                if(PixelValue == tc)
                {
                    Pseg.push_back(trip_idx(ix,iy,iz));
                }

            }

        }
    }
}


int max_component(cv:: Mat label_img)
{

    int * np;

    int max_label = count_n_labels_img(label_img,np);

            //   cout << "Finish counting the histogram" << endl;

            // n_sum = accumulate

    int max_scc = 0;

    if(max_label > 0)
    {
    int * npmaxp = max_element(np, np + max_label);

    max_scc = distance(np,npmaxp);

    max_scc = max_scc + 1;

    }


    delete [] np;

   // delete npmaxp;

    return max_scc;


}

int merge_base_img(Mat image, Mat &seg_img, vector<int> & comp_list)
{

	int img_sz[3];

	for(int i = 0;  i < 3;  i++)
		img_sz[i] = image.size[i];



	/*

    // label the base components

    // apply a modest expansion in an attempt to connect the base
    cout << "Enter the merge_base" << endl;

  //  t3 = 5;

    uchar t3c = 5;

    int dt = 3;

    float dtf = (float)dt;

    float *dist_t1 = 0;

    bwdist(seg_img,dist_t1,10,5);


    //cout << "image size is " << img_sz[0] << img_sz[1] << img_sz[2] << endl;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
      //  cout << "iz is " << iz << endl;
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                float dist_b = dist_t1[offsetk + offsetj + ix];

                uchar PixelValue = image.at<uchar>(v3);

                if((PixelValue > t3c) & ( dist_b < dtf))
                {
                    seg_img.at<uchar>(v3) = 254;

                }

            }

        }
    }

    delete [] dist_t1;


    */

    filter_small_comp(seg_img,6);

    Mat label_base_img;

    int max_label;


    cout << "Begin bwconnmp_img3" << endl;

    max_label = bwconnmp_img3(seg_img,label_base_img,254);

    cout << "There are " << max_label << " components in the base image" << endl;


    // count the number of individual base fragment

    int * np;

    count_n_labels_img(label_base_img,np);

    for(int i = 0; i < max_label; i++)
        cout << "np " << i << " is " << np[i] << endl;


    int n_sum = accumulate(np, np + max_label,0);

    cout << "There are " << n_sum << " pixels in the base image" << endl;


   // delete [] np;


    int max_scc;

    int *max_scc_p = max_element(np, np + max_label);

    max_scc = distance(np,max_scc_p);

    vector<trip_idx> Pseg;

    collect_idx_int(label_base_img, Pseg, max_scc + 1);

    double prop_cc = (double)(*max_scc_p) / (double)n_sum;


    cout << "The largest base represents " << prop_cc * 100 << "%" << endl;

    delete [] np;


    // attempt to connect the largest components if possible

    int * npmaxp;

    int target_tip[3];

    int source_tip[3];


    int stopsigh = 0;

    int n_comp = 1;



    while((prop_cc > 0.1) & (prop_cc < 2))
    {

        ////cin.get();

        if(debug_on > 0)
            cin.get();

        cout << "Start growing the base" << endl;

        int nc = grow_base_img(image, label_base_img, seg_img, max_scc + 1, target_tip,15);

        cout << "nc is " << nc << endl;

        //if(target_tip > 0)



        int target;


        if(nc > 0)
            target = label_base_img.at<int>(target_tip);
        else
            target = 0;


        cout << "The target is " << target << endl;

        if(nc > 0)
        {
            collect_idx_c(seg_img, Pseg, 128);

            collect_idx_int(label_base_img, Pseg, target);

        }

      //  if()

        cout << "1384" << endl;

        ////cin.get();

        if(debug_on > 0)
            cin.get();


        int * tx;

        int * ty;

        int * tz;

        int n_target;

        if(target > 0)
           n_target = count_x_labels_img(label_base_img, target, tx, ty, tz);
           else
            n_target = 0;


                /*

        int n_target = count_x_labels_img(label_base_img, target, tx, ty, tz);


        for(int i = 0; i < 100; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target old " << i << " is " << label_base_img.at<int>(v3) << endl;

        }


*/

     //   ////cin.get();


        cout << "Complete growing the base" << endl;

        filter_small_comp(seg_img,6);

        //cout << "nc is " << nc << endl;

        label_base_img.release();

        max_label = bwconnmp_img4(seg_img,label_base_img, 30);



        if(max_label == 1)
            break;



       cout << "The base has " << max_label <<  " components " << endl;

       // int n_target = 0;

       /*


        for(int i = 0; i < n_target; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }

       ////cin.get();


        */



        // revise the label image

        target = max_component(label_base_img);

        cout << "Biggest component is " << target << endl;

        cout << "1432" << endl;

        cout << "PSeg size is " << Pseg.size() << endl;

        for(int i = 0; i < Pseg.size(); i++)
        {

            int v3[3];

            v3[0] = Pseg[i].x;

            v3[1] = Pseg[i].y;

            v3[2] = Pseg[i].z;

            label_base_img.at<int>(v3) = target;

            //   cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        cout << "1411" << endl;

        if(debug_on > 0)
            cin.get();

        /*

        for(int i = 0; i < n_target; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        ////cin.get();

        */
        if(nc > 0)
        {

            delete [] tx;

            delete [] ty;

            delete [] tz;

        }

        cout << "1527" << endl;

        cout << "Now the base has " << max_label << " components" << endl;

       // cout << "The target is " << label_base_img.at<int>(target_tip) << endl;

        if(debug_on > 0)
            cin.get();
      //  ////cin.get();

        //cout << "..................2..............." << endl;

      //  ////cin.get();

    //  cout << "1450" << endl;



        if((nc == 0) && (max_label > 0))
        {

            stopsigh = 1;

            n_comp ++;


            cout << " Attempt to connect the second largest componnets" << endl;

            count_n_labels_img(label_base_img,np);

            if(debug_on > 0)
            {
            for(int i = 0; i < max_label; i++)
                cout << "np " << i << " is "  <<  np [i] << endl;

            }

            npmaxp = max_element(np, np + max_label);

            max_scc = distance(np,npmaxp);

            max_scc = max_component(label_base_img);


            cout << "The maximal label is " << max_scc << endl;

            cout << "The component has " << *npmaxp << endl;

            cout << 1582 << endl;

            if(debug_on > 0)
                cin.get();

          //  np[max_scc] = 0;

         //   int n_rest = accumulate(np, np + max_label,0);

            delete [] np;

         //   if(n_rest < 1)
         //       break;

            // know out the largest components

            int n_seg = 0;

            for(int iz = 0; iz < label_base_img.size[2]; iz++)
            {
                for(int iy = 0; iy <  label_base_img.size[1]; iy++)
                {
                    for(int ix = 0; ix < label_base_img.size[0]; ix++)
                    {

                        int v3[3];

                        v3[0] = ix;

                        v3[1] = iy;

                        v3[2] = iz;

                        int label = label_base_img.at<int>(v3);

                        if(label == max_scc)
                        {

                            seg_img.at<uchar>(v3) = 20;

                            n_seg ++;

                        }


                    }

                }
            }


            comp_list.push_back(Pseg.size());

            Pseg.clear();


            cout << "knock out " << n_seg << " Pixels" << endl;

          //  ////cin.get();

        }

        label_base_img.release();



        max_label = bwconnmp_img4(seg_img, label_base_img, 30);

        target = max_component(label_base_img);

        cout << "The target is " << target << endl;

        cout << "The Pseg has " << Pseg.size() << endl;



        for(int i = 0; i < Pseg.size(); i++)
        {

            int v3[3];

            v3[0] = Pseg[i].x;

            v3[1] = Pseg[i].y;

            v3[2] = Pseg[i].z;

            label_base_img.at<int>(v3) = target;

         //   cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        cout << "The current base has " << max_label << " components" << endl;

        cout << 1678 << endl;

      //  if(stopsigh > 0)
            //cin.get();


        if(max_label > 0)
        {

            //   cout << "Ready to count the histogram" << endl;

            count_n_labels_img(label_base_img,np);

            //   cout << "Finish counting the histogram" << endl;

            // n_sum = accumulate

            npmaxp = max_element(np, np + max_label);

            max_scc = distance(np,npmaxp);

            cout << "Now the current base is of label " << max_scc << endl;

            cout << "Current base has " << *npmaxp << endl;

            if(Pseg.size() == 0)
                collect_idx_int(label_base_img, Pseg, max_scc + 1);

            //  if(!(*npmaxp) == (max_scc + 1))
            //    ////cin.get();

            cout << 1709 << endl;

            //if(stopsigh > 0)
                //cin.get();

            //////cin.get();

            for(int i = 0; i < max_label; i++)
                cout << "np " << i << " is "  << np[i] << endl;



            cout << 1721 << endl;

            ////cin.get();

            int target_label;

            target_label  = label_base_img.at<int>(target_tip);

            cout << "The target tip is of label " << target_label << endl;


            cout << 1732 << endl;

            //if(stopsigh > 0)
                //cin.get();

            prop_cc = (double)(*npmaxp) / (double)n_sum;

            delete [] np;
        }
        else
            prop_cc = 0;



      //  if(stopsigh > 0)
        //    //cin.get();

        cout << "The current base represents " << prop_cc * 100 << "%" << endl;

        cout << "The current base has  " << prop_cc * n_sum << " pixels" << endl;



    }

    //cout << "Out of the loop" << endl;

   // //cin.get();

    // get the dim componnet back

    int n_seg = 0;


    for(int iz = 0; iz < seg_img.size[2]; iz++)
    {
        for(int iy = 0; iy <  seg_img.size[1]; iy++)
        {
            for(int ix = 0; ix < seg_img.size[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                uchar pV = seg_img.at<uchar>(v3);

                if(pV > 0)
                {

                    seg_img.at<uchar>(v3) = 254;

                    n_seg ++;

                }


            }

        }
    }

    cout << "Retrive " << n_seg << " pixels";

    return n_comp;

}




int merge_base(Mat image, Mat &seg_img, vector<int> & comp_list)
{

	int img_sz[3];

	for(int i = 0;  i < 3;  i++)
		img_sz[i] = image.size[i];

    // label the base components

    // apply a modest expansion in an attempt to connect the base
    cout << "Enter the merge_base" << endl;

  //  t3 = 5;

    uchar t3c = 5;

    int dt = 1;

    float dtf = (float)dt;

    float *dist_t1 = 0;

    bwdist(seg_img,dist_t1,10,5);


    //cout << "image size is " << img_sz[0] << img_sz[1] << img_sz[2] << endl;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
      //  cout << "iz is " << iz << endl;
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                float dist_b = dist_t1[offsetk + offsetj + ix];

                uchar PixelValue = image.at<uchar>(v3);

                if((PixelValue > t3c) & ( dist_b < dtf))
                {
                    seg_img.at<uchar>(v3) = 254;

                }

            }

        }
    }

    delete [] dist_t1;

    filter_small_comp(seg_img,6);

    Mat label_base_img;

    int max_label;


    cout << "Begin bwconnmp_img3" << endl;

    max_label = bwconnmp_img3(seg_img,label_base_img,254);

    cout << "There are " << max_label << " components in the base image" << endl;


    // count the number of individual base fragment

    int * np;

    count_n_labels_img(label_base_img,np);

    for(int i = 0; i < max_label; i++)
        cout << "np " << i << " is " << np[i] << endl;


    int n_sum = accumulate(np, np + max_label,0);

    cout << "There are " << n_sum << " pixels in the base image" << endl;


   // delete [] np;


    int max_scc;

    int *max_scc_p = max_element(np, np + max_label);

    max_scc = distance(np,max_scc_p);

    vector<trip_idx> Pseg;

    collect_idx_int(label_base_img, Pseg, max_scc + 1);

    double prop_cc = (double)(*max_scc_p) / (double)n_sum;


    cout << "The largest base represents " << prop_cc * 100 << "%" << endl;

    delete [] np;


    // attempt to connect the largest components if possible

    int * npmaxp;

    int target_tip[3];

    int source_tip[3];


    int stopsigh = 0;

    int n_comp = 1;



    while(prop_cc > 0.1)
    {

       // //cin.get();

        cout << "Start growing the base" << endl;

        int nc = grow_base(label_base_img, seg_img, max_scc + 1, target_tip,5);

        cout << "nc is " << nc << endl;

        //if(target_tip > 0)



        int target;


        if(nc > 0)
            target = label_base_img.at<int>(target_tip);
        else
            target = 0;


        cout << "  1948 The target is " << target << endl;



        ////cin.get();


        if(nc > 0)
        {
            collect_idx_c(seg_img, Pseg, 128);

            collect_idx_int(label_base_img, Pseg, target);

        }



      //  //cin.get();


        int * tx;

        int * ty;

        int * tz;

        int n_target;

        if(target > 0)
           n_target = count_x_labels_img(label_base_img, target, tx, ty, tz);
           else
            n_target = 0;


                /*

        int n_target = count_x_labels_img(label_base_img, target, tx, ty, tz);


        for(int i = 0; i < 100; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target old " << i << " is " << label_base_img.at<int>(v3) << endl;

        }


*/

     //   //cin.get();


        cout << "Complete growing the base" << endl;

        filter_small_comp(seg_img,6);

        //cout << "nc is " << nc << endl;

        label_base_img.release();

        max_label = bwconnmp_img4(seg_img,label_base_img, 30);

       cout << "The base has " << max_label <<  " components " << endl;

       // int n_target = 0;

       /*


        for(int i = 0; i < n_target; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }

       //cin.get();


        */

        // revise the label image

        target = max_component(label_base_img);

        cout << "Biggest component is " << target << endl;

        cout << "1432" << endl;



        for(int i = 0; i < Pseg.size(); i++)
        {

            int v3[3];

            v3[0] = Pseg[i].x;

            v3[1] = Pseg[i].y;

            v3[2] = Pseg[i].z;

            label_base_img.at<int>(v3) = target;

            //   cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }


     //   cout << "1456" << endl;

        /*

        for(int i = 0; i < n_target; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        //cin.get();

        */
        if(nc > 0)
        {

            delete [] tx;

            delete [] ty;

            delete [] tz;

        }

      //  cout << "Now the base has " << max_label << " components" << endl;

       // cout << "The target is " << label_base_img.at<int>(target_tip) << endl;


      //  //cin.get();

        //cout << "..................2..............." << endl;

      //  //cin.get();

    //  cout << "1450" << endl;



        if((nc == 0) && (max_label > 0))
        {

            stopsigh = 1;

            n_comp ++;


            cout << " Attempt to connect the second largest componnets" << endl;

            count_n_labels_img(label_base_img,np);

           // for(int i = 0; i < max_label; i++)
             //   cout << "np " << i << " is "  <<  np [i] << endl;


            npmaxp = max_element(np, np + max_label);

            max_scc = distance(np,npmaxp);

            max_scc = max_component(label_base_img);


            cout << "The maximal label is " << max_scc << endl;

            cout << "The component has " << *npmaxp << endl;

       //     //cin.get();

          //  np[max_scc] = 0;

         //   int n_rest = accumulate(np, np + max_label,0);

            delete [] np;

         //   if(n_rest < 1)
         //       break;

            // know out the largest components

            int n_seg = 0;

            for(int iz = 0; iz < label_base_img.size[2]; iz++)
            {
                for(int iy = 0; iy <  label_base_img.size[1]; iy++)
                {
                    for(int ix = 0; ix < label_base_img.size[0]; ix++)
                    {

                        int v3[3];

                        v3[0] = ix;

                        v3[1] = iy;

                        v3[2] = iz;

                        int label = label_base_img.at<int>(v3);

                        if(label == max_scc)
                        {

                            seg_img.at<uchar>(v3) = 20;

                            n_seg ++;

                        }


                    }

                }
            }


            comp_list.push_back(Pseg.size());

            Pseg.clear();


            cout << "knock out " << n_seg << " Pixels" << endl;

          //  //cin.get();

        }

        label_base_img.release();



        max_label = bwconnmp_img4(seg_img, label_base_img, 30);

        target = max_component(label_base_img);

        cout << "The target is " << target << endl;

        cout << "The Pseg has " << Pseg.size() << endl;



        for(int i = 0; i < Pseg.size(); i++)
        {

            int v3[3];

            v3[0] = Pseg[i].x;

            v3[1] = Pseg[i].y;

            v3[2] = Pseg[i].z;

            label_base_img.at<int>(v3) = target;

         //   cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        cout << "The current base has " << max_label << " components" << endl;


        //if(stopsigh > 0)
         //   //cin.get();

        if(max_label > 0)
        {

            //   cout << "Ready to count the histogram" << endl;

            count_n_labels_img(label_base_img,np);

            //   cout << "Finish counting the histogram" << endl;

            // n_sum = accumulate

            npmaxp = max_element(np, np + max_label);

            max_scc = distance(np,npmaxp);

            cout << "Now the current base is of label " << max_scc << endl;

            cout << "Current base has " << *npmaxp << endl;

            if(Pseg.size() == 0)
                collect_idx_int(label_base_img, Pseg, max_scc + 1);

            //  if(!(*npmaxp) == (max_scc + 1))
            //    //cin.get();

           // if(stopsigh > 0)
               // //cin.get();

            ////cin.get();

           // for(int i = 0; i < max_label; i++)
            //    cout << "np " << i << " is "  << np[i] << endl;


            int target_label;

            target_label  = label_base_img.at<int>(target_tip);

            cout << "The target tip is of label " << target_label << endl;


          //  if(stopsigh > 0)
            //    //cin.get();

            prop_cc = (double)(*npmaxp) / (double)n_sum;

            delete [] np;
        }
        else
            prop_cc = 0;

      //  if(stopsigh > 0)
        //    //cin.get();

        cout << "The current base represents " << prop_cc * 100 << "%" << endl;

        cout << "The current base has  " << prop_cc * n_sum << " pixels" << endl;



    }

    //cout << "Out of the loop" << endl;

   // //cin.get();

    // get the dim componnet back

    int n_seg = 0;


    for(int iz = 0; iz < seg_img.size[2]; iz++)
    {
        for(int iy = 0; iy <  seg_img.size[1]; iy++)
        {
            for(int ix = 0; ix < seg_img.size[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                uchar pV = seg_img.at<uchar>(v3);

                if(pV > 0)
                {

                    seg_img.at<uchar>(v3) = 254;

                    n_seg ++;

                }


            }

        }
    }

    cout << "Retrive " << n_seg << " pixels";

    return n_comp;

}

int merge_base1(Mat image, Mat &seg_img, vector<int> & comp_list, vector<trip_idx> & base_idx, int large_flag)
{

	int img_sz[3];

	for(int i = 0;  i < 3;  i++)
		img_sz[i] = image.size[i];

    // label the base components

    // apply a modest expansion in an attempt to connect the base
    cout << "Enter the merge_base" << endl;

  //  t3 = 5;
    uchar t3c;




    if(large_flag > 1000)
        t3c = 100;
    else
    {
        if(large_flag > 500)
            t3c = 50;
        else
        {
            if(large_flag > 100)
                t3c = 20;
            else
                t3c = 5;
        }


    }


    int dt = 1;

    float dtf = (float)dt;

    float *dist_t1 = 0;

    bwdist(seg_img,dist_t1,10,5);


    //cout << "image size is " << img_sz[0] << img_sz[1] << img_sz[2] << endl;

    if(large_flag < 300)
    {

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
      //  cout << "iz is " << iz << endl;
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                float dist_b = dist_t1[offsetk + offsetj + ix];

                uchar PixelValue = image.at<uchar>(v3);

                if((PixelValue > t3c) & ( dist_b < dtf))
                {
                    seg_img.at<uchar>(v3) = 254;

                }

            }

        }
    }

    }

    delete [] dist_t1;

    filter_small_comp(seg_img,6);

    Mat label_base_img;

    int max_label;


    cout << "Begin bwconnmp_img3" << endl;

    max_label = bwconnmp_img3(seg_img,label_base_img,254);

    cout << "There are " << max_label << " components in the base image" << endl;


    // count the number of individual base fragment

    int * np;

    count_n_labels_img(label_base_img,np);

    for(int i = 0; i < max_label; i++)
        cout << "np " << i << " is " << np[i] << endl;


    int n_sum = accumulate(np, np + max_label,0);

    cout << "There are " << n_sum << " pixels in the base image" << endl;


   // delete [] np;


    int max_scc;

    int *max_scc_p = max_element(np, np + max_label);

    max_scc = distance(np,max_scc_p);

    vector<trip_idx> Pseg;



    // represent the filled pixels
    vector<trip_idx> P_sub;


    collect_idx_int(label_base_img, Pseg, max_scc + 1);

    double prop_cc = (double)(*max_scc_p) / (double)n_sum;


    cout << "The largest base represents " << prop_cc * 100 << "%" << endl;

    delete [] np;


    // attempt to connect the largest components if possible

    int * npmaxp;

    int target_tip[3];

    int source_tip[3];


    int stopsigh = 0;

    int n_comp = 1;



    while(prop_cc > 0.1)
    {

       // //cin.get();

        cout << "Start growing the base" << endl;

        int nc = grow_base(label_base_img, seg_img, max_scc + 1, target_tip,5);

        cout << "nc is " << nc << endl;





        //if(target_tip > 0)



        int target;


        if(nc > 0)
            target = label_base_img.at<int>(target_tip);
        else
            target = 0;


        cout << "  1948 The target is " << target << endl;



        ////cin.get();


        if(nc > 0)
        {
            collect_idx_c(seg_img, Pseg, 128);

            collect_idx_int(label_base_img, Pseg, target);

        }



      //  //cin.get();


        int * tx;

        int * ty;

        int * tz;

        int n_target;

        if(target > 0)
           n_target = count_x_labels_img(label_base_img, target, tx, ty, tz);
           else
            n_target = 0;


                /*

        int n_target = count_x_labels_img(label_base_img, target, tx, ty, tz);


        for(int i = 0; i < 100; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target old " << i << " is " << label_base_img.at<int>(v3) << endl;

        }


*/

     //   //cin.get();


        cout << "Complete growing the base" << endl;

        filter_small_comp(seg_img,6);

        //cout << "nc is " << nc << endl;

        label_base_img.release();

        max_label = bwconnmp_img4(seg_img,label_base_img, 30);

       cout << "The base has " << max_label <<  " components " << endl;

       // int n_target = 0;

       /*


        for(int i = 0; i < n_target; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }

       //cin.get();


        */

        // revise the label image

        target = max_component(label_base_img);

        cout << "Biggest component is " << target << endl;

        cout << "1432" << endl;



        for(int i = 0; i < Pseg.size(); i++)
        {

            int v3[3];

            v3[0] = Pseg[i].x;

            v3[1] = Pseg[i].y;

            v3[2] = Pseg[i].z;

            label_base_img.at<int>(v3) = target;

            //   cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }


     //   cout << "1456" << endl;

        /*

        for(int i = 0; i < n_target; i++)
        {

            int v3[3];

            v3[0] = tx[i];

            v3[1] = ty[i];

            v3[2] = tz[i];

            cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        //cin.get();

        */
        if(nc > 0)
        {

            delete [] tx;

            delete [] ty;

            delete [] tz;

        }

      //  cout << "Now the base has " << max_label << " components" << endl;

       // cout << "The target is " << label_base_img.at<int>(target_tip) << endl;


      //  //cin.get();

        //cout << "..................2..............." << endl;

      //  //cin.get();

    //  cout << "1450" << endl;



        if((nc == 0) && (max_label > 0))
        {

            stopsigh = 1;

            n_comp ++;


            cout << " Attempt to connect the second largest componnets" << endl;

            count_n_labels_img(label_base_img,np);

           // for(int i = 0; i < max_label; i++)
             //   cout << "np " << i << " is "  <<  np [i] << endl;


            npmaxp = max_element(np, np + max_label);

            max_scc = distance(np,npmaxp);

            max_scc = max_component(label_base_img);


            cout << "The maximal label is " << max_scc << endl;

            cout << "The component has " << *npmaxp << endl;

       //     //cin.get();

          //  np[max_scc] = 0;

         //   int n_rest = accumulate(np, np + max_label,0);

            delete [] np;

         //   if(n_rest < 1)
         //       break;


              // record the filled pixels
             collect_idx_c(seg_img, P_sub, 128);


            // knock out the largest components

            int n_seg = 0;

            for(int iz = 0; iz < label_base_img.size[2]; iz++)
            {
                for(int iy = 0; iy <  label_base_img.size[1]; iy++)
                {
                    for(int ix = 0; ix < label_base_img.size[0]; ix++)
                    {

                        int v3[3];

                        v3[0] = ix;

                        v3[1] = iy;

                        v3[2] = iz;

                        int label = label_base_img.at<int>(v3);

                        if(label == max_scc)
                        {

                            seg_img.at<uchar>(v3) = 20;

                            n_seg ++;

                        }


                    }

                }
            }


            comp_list.push_back(Pseg.size());

            for(int i = 0; i < Pseg.size(); i++)
            {

                base_idx.push_back(Pseg[i]);


            }




            Pseg.clear();


            cout << "knock out " << n_seg << " Pixels" << endl;

          //  //cin.get();

        }

        label_base_img.release();



        max_label = bwconnmp_img4(seg_img, label_base_img, 30);

        target = max_component(label_base_img);

        cout << "The target is " << target << endl;

        cout << "The Pseg has " << Pseg.size() << endl;



        for(int i = 0; i < Pseg.size(); i++)
        {

            int v3[3];

            v3[0] = Pseg[i].x;

            v3[1] = Pseg[i].y;

            v3[2] = Pseg[i].z;

            label_base_img.at<int>(v3) = target;

         //   cout << "target new" << i << " is " << label_base_img.at<int>(v3) << endl;

        }



        cout << "The current base has " << max_label << " components" << endl;


        //if(stopsigh > 0)
         //   //cin.get();

        if(max_label > 0)
        {

            //   cout << "Ready to count the histogram" << endl;

            count_n_labels_img(label_base_img,np);

            //   cout << "Finish counting the histogram" << endl;

            // n_sum = accumulate

            npmaxp = max_element(np, np + max_label);

            max_scc = distance(np,npmaxp);

            cout << "Now the current base is of label " << max_scc << endl;

            cout << "Current base has " << *npmaxp << endl;

            if(Pseg.size() == 0)
                collect_idx_int(label_base_img, Pseg, max_scc + 1);

            //  if(!(*npmaxp) == (max_scc + 1))
            //    //cin.get();

           // if(stopsigh > 0)
               // //cin.get();

            ////cin.get();

           // for(int i = 0; i < max_label; i++)
            //    cout << "np " << i << " is "  << np[i] << endl;


            int target_label;

            target_label  = label_base_img.at<int>(target_tip);

            cout << "The target tip is of label " << target_label << endl;


          //  if(stopsigh > 0)
            //    //cin.get();

            prop_cc = (double)(*npmaxp) / (double)n_sum;

            delete [] np;
        }
        else
            prop_cc = 0;



      //  if(stopsigh > 0)
        //    //cin.get();

        cout << "The current base represents " << prop_cc * 100 << "%" << endl;

        cout << "The current base has  " << prop_cc * n_sum << " pixels" << endl;



    }

    // record the filled pixels
    collect_idx_c(seg_img, P_sub, 128);


    //cout << "Out of the loop" << endl;

   // //cin.get();

    // get the dim componnet back

    int n_seg = 0;


    for(int iz = 0; iz < seg_img.size[2]; iz++)
    {
        for(int iy = 0; iy <  seg_img.size[1]; iy++)
        {
            for(int ix = 0; ix < seg_img.size[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                uchar pV = seg_img.at<uchar>(v3);

                if(pV > 0)
                {

                    seg_img.at<uchar>(v3) = 254;

                    n_seg ++;

                }


            }

        }
    }

    for(int i = 0; i < P_sub.size();i++)
    {

        int v3[3];

        v3[0] = P_sub[i].x;

        v3[1] = P_sub[i].y;

        v3[2] = P_sub[i].z;

        seg_img.at<uchar>(v3) = 128;

    }

    cout << "Retrive " << n_seg << " pixels";

    return n_comp;

}





int grow_base(cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip, float min_dist1)
{

    float *dist_t = 0;

    float *dist_s = 0;

  //  int target_tip[3];

    //float min_dist;

    cout << "Ready to to do the bwdist2" << endl;

    float min_dist = bwdist2(label_img, dist_t, source, target_tip, min_dist1);




    cout << "The minimal distance for tareget is " << min_dist << endl;

    int np = 0;

    if(min_dist < (min_dist1 + 0.01))
    {


        cout << "target_tip is " << target_tip[0] << " " << target_tip[1] << " " << target_tip[2] << " " << endl;

        cout << " label image is of size " << label_img.size[0] << " " << label_img.size[1] << " " << label_img.size[2] << " " << endl;

        int target = label_img.at<int>(target_tip);

        cout << "The merged base is of label " << target << endl;

        // expand the target tip

        expand_tip(seg_img,target_tip);

        int target_tip1[3];

        bwdist2(label_img, dist_s, target, target_tip1, min_dist1);

        // expand the target tip1

        expand_tip(seg_img,target_tip);

        expand_tip(seg_img,target_tip1);

        if(debug_on4)
        {



            string patial_seg_file = "/media/gulin/E402023602020DEC/Cache/BigN/expand_illu.v3draw";

            char * patial_seg_file_char = (char *)patial_seg_file.c_str();

            //sprintf(base_file, " partial_segmentation.v3draw");

            saveMat(seg_img,patial_seg_file_char);

            cin.get();


        }




        min_dist = min((float)(min_dist + 0.1), min_dist1);


        int img_sz[3];

        for(int i = 0; i < 3; i++)
            img_sz[i] = label_img.size[i];
        for(int iz = 0; iz < img_sz[2]; iz++)
        {
            int offsetk = iz * img_sz[1] * img_sz[0];
            for(int iy = 0; iy <  img_sz[1]; iy++)
            {
                int offsetj = iy * img_sz[0];
                for(int ix = 0; ix < img_sz[0]; ix++)
                {

                    int v3[3];

                    v3[0] = ix;

                    v3[1] = iy;

                    v3[2] = iz;

                    float dt = dist_t[offsetk + offsetj + ix];

                    float ds = dist_s[offsetk + offsetj + ix];

                    int label = label_img.at<uchar>(v3);

                    if(label == source)
                    {
                        seg_img.at<uchar>(v3) = 254;

                       // np ++;

                    }

                    if(label == target)
                    {
                        seg_img.at<uchar>(v3) = 254;

                       // np ++;

                    }
                    //      if(pV == 0)
                    //    {

                    if((dt <= min_dist) & (ds <= min_dist))
                    {
                        seg_img.at<uchar>(v3) = 128;
                        np++;

                    }
                    //  }

                }

            }
        }

        cout << "Fill " << np << " pixels" << endl;

    }

    delete [] dist_t;

    delete [] dist_s;



    return np;

 }


int grow_base_img(cv::Mat image, cv::Mat label_img, cv::Mat &seg_img, int source, int target_tip[], float min_dist1)
{

    float *dist_t = 0;

    float *dist_s = 0;

    float *dist_t1 = 0;

  //  int target_tip[3];

    //float min_dist;

    cout << "Ready to to do the bwdist2" << endl;

    float min_dist = bwdist2(label_img, dist_t1, source, target_tip, min_dist1);

    delete [] dist_t1;

    //delete [] dist_t;

    cout << "The minimal distance for tareget is " << min_dist << endl;

    int np = 0;

    if(min_dist < (min_dist1 + 0.01))
    {


        cout << "target_tip is " << target_tip[0] << " " << target_tip[1] << " " << target_tip[2] << " " << endl;

        cout << " label image is of size " << label_img.size[0] << " " << label_img.size[1] << " " << label_img.size[2] << " " << endl;

        int target = label_img.at<int>(target_tip);

        cout << "The merged base is of label " << target << endl;

        int tmp_t_tip1[3];

        int tmp_t_tip2[3];

        float min_dist_t;

        float min_dist_s;


        min_dist_s = bwdist_img(image, label_img, dist_t, source, target, tmp_t_tip1,(min_dist + 1) * 255);

        min_dist_t = bwdist_img(image, label_img, dist_s, target, source, tmp_t_tip2, (min_dist + 1) * 255);

        // expand the two remote tips

        expand_tip(seg_img,tmp_t_tip1);

        expand_tip(seg_img,tmp_t_tip2);




       // min_dist = min((float)(min_dist + 0.1), min_dist1);

       cout << "min_dist_s is " << min_dist_s << endl;

       cout << "min_dist_t is " << min_dist_t << endl;

       float min_dist_st = max(min_dist_s,min_dist_t);

       min_dist = min_dist + 0.1;


        int img_sz[3];

        for(int i = 0; i < 3; i++)
            img_sz[i] = label_img.size[i];
        for(int iz = 0; iz < img_sz[2]; iz++)
        {
            int offsetk = iz * img_sz[1] * img_sz[0];
            for(int iy = 0; iy <  img_sz[1]; iy++)
            {
                int offsetj = iy * img_sz[0];
                for(int ix = 0; ix < img_sz[0]; ix++)
                {

                    int v3[3];

                    v3[0] = ix;

                    v3[1] = iy;

                    v3[2] = iz;

                    float dt = dist_t[offsetk + offsetj + ix];

                    float ds = dist_s[offsetk + offsetj + ix];

                    int label = label_img.at<uchar>(v3);

                    if(label == source)
                    {
                        seg_img.at<uchar>(v3) = 254;

                       // np ++;

                    }

                    if(label == target)
                    {
                        seg_img.at<uchar>(v3) = 254;

                       // np ++;

                    }
                    //      if(pV == 0)
                    //    {

                    if((dt <= min_dist_s) && (ds <= min_dist_t))
                    {
                       // cout << "The filled image is " << (int)image.at<uchar>(v3) << endl;

                        double ddt = (double)dist_t1[offsetk + offsetj + ix];

                        //if((ddt < (double)(min_dist1 + 2)) & (ddt > 0))
                          //    cout << "The filled image is " << ddt << endl;

                        seg_img.at<uchar>(v3) = 128;
                        np++;

                    }
                    //  }

                }

            }
        }

        cout << "min_dist is " << min_dist << endl;

        cout << "Fill " << np << " pixels" << endl;

    }

   // delete [] dist_t1;

    delete [] dist_t;

    delete [] dist_s;




    return np;

 }


int expand_tip(cv::Mat &seg_img,int target_tip[])
{

    int img_sz[3];

    for(int i = 0; i < 3; i++)
        img_sz[i] = seg_img.size[i];

    for(int ix = -1; ix < 2; ix++)
    {
        for(int iy = -1; iy < 2; iy++)
        {

            for(int iz = - 1; iz < 2; iz ++)
            {
                int x = ix + target_tip[0];

                int y = iy + target_tip[1];

                int z = iz + target_tip[2];

                x = min(x,img_sz[0]);

                y = min(y,img_sz[1]);

                z = min(z,img_sz[2]);

                x = max(x,1);

                y = max(y,1);

                z = max(z,1);

                int v3[3];

                v3[0] = x;

                v3[1] = y;

                v3[2] = z;

                if(seg_img.at<uchar>(v3) < 1)
                    seg_img.at<uchar>(v3) = 128;

            }

        }



    }

    return 1;

}



 int grow_base1(cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip, float min_dist1)
{

    float *dist_t = 0;

    float *dist_s = 0;

  //  int target_tip[3];

    //float min_dist;

    cout << "Ready to to do the bwdist2" << endl;

    float min_dist = bwdist2(label_img, dist_t, source, target_tip, min_dist1);

    cout << "The minimal distance for tareget is " << min_dist << endl;

    int np = 0;

    if(min_dist < (min_dist1 + 0.01))
    {


        cout << "target_tip is " << target_tip[0] << " " << target_tip[1] << " " << target_tip[2] << " " << endl;

        cout << " label image is of size " << label_img.size[0] << " " << label_img.size[1] << " " << label_img.size[2] << " " << endl;

        int target = label_img.at<int>(target_tip);

        cout << "The merged base is of label " << target << endl;

        bwdist1(label_img, dist_s, target, min_dist1);

        //min_dist = min(min_dist + 0.1, min_dist1);

        int img_sz[3];

        for(int i = 0; i < 3; i++)
            img_sz[i] = label_img.size[i];
        for(int iz = 0; iz < img_sz[2]; iz++)
        {
            int offsetk = iz * img_sz[1] * img_sz[0];
            for(int iy = 0; iy <  img_sz[1]; iy++)
            {
                int offsetj = iy * img_sz[0];
                for(int ix = 0; ix < img_sz[0]; ix++)
                {

                    int v3[3];

                    v3[0] = ix;

                    v3[1] = iy;

                    v3[2] = iz;

                    float dt = dist_t[offsetk + offsetj + ix];

                    float ds = dist_s[offsetk + offsetj + ix];

                    int label = label_img.at<uchar>(v3);

                    if(label == source)
                    {
                        seg_img.at<uchar>(v3) = 254;

                        //np ++;

                    }

                    if(label == target)
                    {
                        seg_img.at<uchar>(v3) = 254;

                        //np ++;

                    }
                    //      if(pV == 0)
                    //    {

                    if((dt <= min_dist1) & (ds <= min_dist1))
                    {
                        seg_img.at<uchar>(v3) = 254;
                        np++;

                    }
                    //  }

                }

            }
        }

        cout << "Fill " << np << " pixels" << endl;

    }

    delete [] dist_t;

    delete [] dist_s;









    return np;

 }



int grow_base_adv(cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip)
{

    int img_sz[3];

    for(int i = 0; i < 3; i++)
        img_sz[i] = label_img.size[i];

    //Mat seg_img1;

  // seg_img1 = Mat(3,img_sz,CV_8UC1,Scalar::all(0));

    int nf;

    float min_dist = 5;

    nf = grow_base(label_img, seg_img, source, target_tip, min_dist);

    if(nf > 0)
    {
        int target = label_img.at<int>(target_tip);

        int * tx;

        int * ty;

        int * tz;


        int * sx;

        int * sy;

        int * sz;

        int n_target = count_x_labels_img(label_img, target, tx, ty, tz);

        int n_source = count_x_labels_img(label_img, source, sx, sy, sz);

        Mat label_img1;

        // check the connectivity

       for(float con_dist = 5; con_dist < 20; con_dist ++)
        {

            label_img1.release();

            bwconnmp_img4(seg_img,label_img1,20);

           // seg_img1.release();

            int v3[3];

            v3[0] = sx[0];

            v3[1] = sy[0];

            v3[2] = sz[0];

            int source_new = label_img1.at<int>(v3);

            for(int i = 0; i < 100 ; i++)
            {

                v3[0] = tx[i];

                v3[1] = ty[i];

                v3[2] = tz[i];


                cout << "The source is " << label_img1.at<int>(v3) << endl;
            }

             cout << "New base is " << source_new << endl;


            int isc  = isconnect(label_img1, n_target, tx,ty,tz,source_new);

            cout << "There are " << isc << " not connected pixels" << endl;

            cout << "Now the connecting dist is " << (double)con_dist << endl;

            //cin.get();

            if( isc < 1)
            {

                label_img1.release();

                if(con_dist > 5)
                    nf = grow_base1(label_img, seg_img, source, target_tip, con_dist);
                else
                    nf = grow_base(label_img, seg_img, source, target_tip, con_dist);

                cout << "Connect the link with energy " << (double)con_dist << endl;

                break;

            }else
                nf = grow_base1(label_img, seg_img, source, target_tip, con_dist);

        }

        label_img1.release();

        delete [] tx;

        delete [] ty;

        delete [] tz;

        delete [] sx;

        delete [] sy;

        delete [] sz;

    }

    return nf;

 }

int isconnect(cv::Mat label_img, int nt, int *tx,int *ty,int *tz, int source)
{

    int isc = 0;

    for(int i = 0; i < nt; i++)
    {

        int v3[3];

         v3[0] = tx[i];

         v3[1] = ty[i];

         v3[2] = tz[i];

         int tl = label_img.at<int>(v3);

         if(tl!=source)
           isc ++;

    }

    return isc;

}






   //if(seg_img2[tip_seed] > 0)
     //   seg_img2[tip_seed] =






int count_merged_base(cv::Mat seg_img,cv::Mat label_base_img, int * base_merge_list)
{

	int max_label;

	Mat label_img;

    max_label = bwconnmp_img3(seg_img,label_img,254);

	vector<vector<int> > Px(max_label);

	vector<vector<int> > Py(max_label);

	vector<vector<int> > Pz(max_label);

	bwconnmp(label_img,Px,Py,Pz);

    for(int i = 0; i < max_label; i++)
    {
    	int n_p = Px[i].size();

    	for(int j = 0; j < n_p; j ++)
    	{
    		int v3[3];

    		v3[0] = Px[i][j];

    		v3[1] = Py[i][j];

    		v3[2] = Pz[i][j];

    		int l = label_img.at<int>(v3);

    		if(l > 0)
    			base_merge_list[l - 1] = i;

    	}

    }

    return max_label;

}

int force_merge(cv::Mat image, cv::Mat &seg_img, vector<int> base_list,vector<trip_idx> base_idx)
{

    int img_sz[3];

    for(int i = 0; i < 3; i++)
        img_sz[i] = image.size[i];

    Mat label_img;

    label_img = Mat(3,img_sz,CV_32S,Scalar::all(0));

    int bi = 0;

    int * base_offset = new int[base_list.size()];




    for(int i = 0; i < base_list.size(); i++)
    {

        base_offset[i] = bi;

        for(int j = 0; j < base_list[i]; j++)
        {
            int v3[3];

            v3[0] = base_idx[bi].x;

            v3[1] = base_idx[bi].y;

            v3[2] = base_idx[bi].z;

            label_img.at<int>(v3) = (i + 1);

            bi++;

        }


    }

    int n_b = base_list.size();

    int * cc_list = new int[n_b];

    for(int i = 0; i < n_b; i++)
        cc_list[i] = base_list[i];




    int * mt_ip = max_element(cc_list,cc_list + n_b);

    int mt_i = distance(cc_list,mt_ip) + 1;


    cout << "The current main trunck is " << mt_i << endl;


    //max_component(label_img);

    int n_sum = base_idx.size();



    double prop_cc = (double)base_list[mt_i - 1] / (double) n_sum;

    cout << "The main trunk now contains " << prop_cc * 100 << "%" << endl;

    vector<trip_idx> P_sub;



    int num_b = n_b;




    while((prop_cc > 0.1) & (num_b > 1))
    {



        if(debug_on1 > 0)
            cin.get();

        cout << "Start growing the base" << endl;

        int target_tip[3];

        // adaptively change the distance

        int nc = grow_base_img(image, label_img, seg_img, mt_i, target_tip,15);

        //if(target_tip > 0)

        int target;




        if(nc > 0)
        {
            target = label_img.at<int>(target_tip);

            cout << "Connect to the label " << target << endl;

            cout << "The label has " << cc_list[target - 1] << endl;


            if(debug_on1 > 0)
                cin.get();

            //cc_list[mt_i - 1] = cc_list[mt_i - 1] + cc_list[target - 1] + n_c;


            /*
            for(int i = 0; i < base_list[i]; i++)
            {

                int v3[3];

                v3[0] = base_idx[base_offset[target - 1] + i].x;

                v3[1] = base_idx[base_offset[target - 1] + i].y;

                v3[2] = base_idx[base_offset[target - 1] + i].z;

                label_img.at<int>(v3) = mt_i;



            }

            */
            collect_idx_c(seg_img,P_sub,128);



            for(int iz = 0; iz < img_sz[2]; iz++)
            {
                for(int iy = 0; iy <  img_sz[1]; iy++)
                {
                    for(int ix = 0; ix < img_sz[0]; ix++)
                    {

                        int v3[3];

                        v3[0] = ix;

                        v3[1] = iy;

                        v3[2] = iz;


                        if(label_img.at<int>(v3) == target)
                        {
                            label_img.at<int>(v3) = mt_i;

                            cc_list[mt_i - 1] ++;

                        }

                        if(seg_img.at<uchar>(v3) == 128)
                        {


                            label_img.at<int>(v3) = mt_i;

                            cc_list[mt_i - 1] ++;


                            seg_img.at<uchar>(v3) = 129;

                        }
                    }


                }
            }


            cc_list[target - 1] = 0;

            num_b --;


        }else{

            target = 0;

            cout << "Fails to connect to any base" << endl;

            cc_list[mt_i - 1] = 0;


            mt_ip = max_element(cc_list,cc_list + n_b);

            mt_i = distance(cc_list,mt_ip) + 1;

            prop_cc = (double)base_list[mt_i - 1] / (double) n_sum;

            cout << "The main trunk now contains " << prop_cc * 100 << "%" << endl;

            if(debug_on1 > 0)
                cin.get();



            num_b --;

        }

    }

    delete [] cc_list;

    delete [] base_offset;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                if(seg_img.at<uchar>(v3) > 10)
                {

                    seg_img.at<uchar>(v3) = 254;

                }
            }


        }
    }

    // change the subsititued pixels into 128

    for(int i = 0; i < P_sub.size();i++)
    {
        int v3[3];

        v3[0] = P_sub[i].x;

        v3[1] = P_sub[i].y;

        v3[2] = P_sub[i].z;

        seg_img.at<uchar>(v3) = 128;

    }




}



int check_connect(cv::Mat &seg_img)
{
   // Mat label_base;

    Mat label_t2;

    cout << "Ready to do the bwconnmp_img3" << endl;

    int n_t2 = bwconnmp_img3(seg_img,label_t2,80);

	cout << "Complete the bwconnmp on the images" << endl;

	cout << "There are " << n_t2 <<  " components in the image" << endl;

	// find how many labels are there in the image

	vector<vector<int> > PList_t2_x(n_t2);

	vector<vector<int> > PList_t2_y(n_t2);

	vector<vector<int> > PList_t2_z(n_t2);

    bwconnmp(label_t2,PList_t2_x,PList_t2_y,PList_t2_z);


    label_t2.release();

    cout << "Release the label_t2 " << endl;





    int n_t12 = bwconnmp_img4(seg_img,label_t2,50);

 	vector<vector<int> > PList_t12_x(n_t12);

	vector<vector<int> > PList_t12_y(n_t12);

	vector<vector<int> > PList_t12_z(n_t12);

	bwconnmp(label_t2,PList_t12_x,PList_t12_y,PList_t12_z);

    int * label_t2_list = new int[n_t2];

    for(int i = 0; i < n_t2; i++)
    {
        int v3[3];

        v3[0] = PList_t2_x[i][0];

        v3[1] = PList_t2_y[i][0];

        v3[2] = PList_t2_z[i][0];

        label_t2_list[i] = label_t2.at<int>(v3);

    }

	label_t2.release();

	int * np;

	int n_t1 = bwconnmp_img3(seg_img,label_t2,254);

	count_n_labels_img(label_t2, np);

    double * p_seg = new double[n_t1];

    for(int i = 0; i < n_t1; i++)
    {
        p_seg[i] = (double)np[i];

    }

    delete [] np;

    double n_sum = accumulate(p_seg,p_seg + n_t2,0);

    for(int i = 0; i < n_t2; i++)
        p_seg[i] = p_seg[i] / n_sum;


    double * p_segt1 = new double[n_t1];

	int * pt1 = new int[n_t1];

	int * t12_c = new int[n_t12];

	double * t12_cp = new double[n_t12];

    //int * seg_cn[] = new int[n_];

	for(int i = 0; i < n_t12; i++)
	{
        for(int j = 0; j < n_t1; j++)
        {
            pt1[j] = 0;

            p_segt1[j] = 0;

        }

        for(int j = 0; j < PList_t12_x[i].size(); j ++)
        {
            int v3[3];

            v3[0] = PList_t12_x[i][j];

            v3[1] = PList_t12_y[i][j];

            v3[2] = PList_t12_z[i][j];

            int label = label_t2.at<int>(v3);

            if(label > 0)
            {
                pt1[label - 1]  = 1;

                p_segt1[label - 1] = p_seg[label - 1];

            }


        }

        t12_c[i] = accumulate(pt1,pt1 + n_t1,0);

        t12_cp[i] = accumulate(p_segt1,p_segt1 + n_t1,0);

	}

	label_t2.release();

	int * pt2 = new int[n_t2];

	double * pt12_cp = new double[n_t2];

	for(int i = 0; i < n_t2; i ++)
    {
        pt2[i] = t12_c[label_t2_list[i]];

        pt12_cp[i] = t12_cp[label_t2_list[i]];

        cout << " The pt2 is " << pt2[i] << endl;

        cout << " The prop for pt2 is " << pt12_cp[i] << endl;

	}

	cin.get();

	delete [] t12_c;

	delete [] t12_cp;

	delete [] p_segt1;

	delete [] pt1;

	delete [] pt2;

	delete [] pt12_cp;


}



int LCM_boost(Mat input_img, Mat image, Mat & seg_img,int t1, int t2,char * dataset)
{
       // at first load the image into opencv

	int img_sz[3];

	img_sz[0] = (int)image.size[0];

	img_sz[1] = (int)image.size[1];

	img_sz[2] = (int)image.size[2];

	seg_img = Mat(3,img_sz,CV_8UC1,Scalar::all(0));

	V3DLONG sz[3];

	for(int i = 0; i < 3; i++)
		sz[i] = (V3DLONG)img_sz[i];

	int img_page = img_sz[0] * img_sz[1] * img_sz[2];

	cout << "t1 is " << t1 << endl;

	cout << "t2 is " << t2 << endl;


//////////////////////////////////////////////////////////////
	// save the partial segmetnation
	if(0)
	{

		for(V3DLONG iz = 0; iz < sz[2]; iz++)
		{

			V3DLONG offsetk = iz*sz[1]*sz[0];

			for(V3DLONG iy = 0; iy <  sz[1]; iy++)
			{
				V3DLONG offsetj = iy* sz[0];
				for(V3DLONG ix = 0; ix < sz[0]; ix++)
				{


					int v3[3];

					v3[0] = (int)ix;

					v3[1] = (int)iy;

					v3[2] = (int)iz;

					uchar PixelValue = image.at<uchar>(v3);

					if(PixelValue > 20)
					{

						seg_img.at<uchar>(v3) = 254;

					}

				}
			}

		}

		//char * base_file = new char[100];

		string patial_seg_file = "/media/gulin/E402023602020DEC/Cache/BigN/partial_seg.v3draw";

		char * patial_seg_file_char = (char *)patial_seg_file.c_str();

		//sprintf(base_file, " partial_segmentation.v3draw");

		saveMat(seg_img,patial_seg_file_char);

	}


	/////////////////////////////////////////////////////////////////////////////////////

	// first get the image t1

	uchar t1c = (uchar) t1;

	uchar t2c = (uchar) t2;

	//int t1_sum = 0;

	int n_base;

	int base_thres;

	//cin.get();

	n_base  = get_main_branch(image, seg_img, t1,base_thres);

	// determine the connecting branch

 	find_t2_fragments(image, seg_img, t2, 0);

    cout << "Complete t2 seeds" << endl;

//	check_connect(seg_img);


	cout << "Base image contains " << n_base << " main branches" << endl;

	cout << "start doing bwdist" << endl;

    //cin.get();

//	float * dist_t1 = 0;

//	bwdist(seg_img,dist_t1,10,17);

//	cout << "save the distance mapping" << endl;

//	cout << "complete bwdist" << endl;


//	string dist_mapping_file = "/media/gulin/E402023602020DEC/Cache/BigN/dist_imgs.v3draw";

//	char * dist_mapping_file_char =  (char *)dist_mapping_file.c_str();

//	saveImg(dist_t1,img_sz,dist_mapping_file_char);


	cout << "Start merging the base" << endl;

   // cin.get();


//    vector<int> base_list;

//    merge_base(image,seg_img,base_list);

 //   merge_base(image,seg_img,base_list);

    vector<int> base_list;

    vector<trip_idx> base_idx;

    merge_base1(image,seg_img,base_list, base_idx,base_thres);

    force_merge(image,seg_img,base_list,base_idx);


    cout << "The base idx has " << base_idx.size() << endl;

    cout << "There are " << base_list.size() << " in the label image"<< endl;

    string base_file = "/media/gulin/E402023602020DEC/Cache/BigN/base_seg.v3draw";

	char * base_file_char = (char *)base_file.c_str();

    saveMat(seg_img,base_file_char);

    float dt = 3;

 //	find_t2_fragments(image, seg_img, t2, dt);

 	//Mat features_t2;

    //retrieve_low_confidence(input_img, image,  seg_img, dataset);





 	 	/*






	cout << "Ready to do the bwconnmp on the images" << endl;

	//   assign the minor pixels in T1 image as the seeds

	max_label = bwconnmp_img3(seg_img,label_img,1);

	cout << "Complete the bwconnmp on the images" << endl;

	cout << max_label << endl;

	// find how many labels are there in the image

	vector<vector<int> > PixelIdxListx(max_label);

	vector<vector<int> > PixelIdxListy(max_label);

	vector<vector<int> > PixelIdxListz(max_label);

	bwconnmp(label_img,PixelIdxListx,PixelIdxListy,PixelIdxListz);

	float * PLmd = new float[max_label];

	int * base_pl = new int[max_label];





	for(int i = 0; i < max_label; i++)
	{

		int np = PixelIdxListx[i].size();

		cout << np << endl;

		PLmd[i] = 0;

		for(int j = 0; j < np; j++)
		{

			V3DLONG ix = (V3DLONG) PixelIdxListx[i][j];

			V3DLONG iy = (V3DLONG) PixelIdxListy[i][j];

			V3DLONG iz = (V3DLONG) PixelIdxListz[i][j];


			V3DLONG offsetk = iz*sz[1]*sz[0];

			V3DLONG offsetj = iy* sz[0];

			float dist_b = dist_t1[offsetk + offsetj + ix];

			PLmd[i] = PLmd[i] + dist_b;

		}

		cout << "The sum distance is  " << PLmd[i] << endl;

		if(PLmd[i] < 1)
			base_pl[i] = 1;
		else
			base_pl[i] = 0;

	}

	//  delete [] dist_t1;

	// be very cautious the 0 of PixelIdxListx is label 1
	cout << "start the determine_bounding_box on the images" << endl;

	cout << max_label << endl;
	// find the bounding box for individual component

	int *bnd = new int[max_label * 2 * 3];

	determine_bounding_box(bnd,PixelIdxListx, PixelIdxListy, PixelIdxListz);

	cout << "Complete the determine_bounding_box on the images" << endl;


	int base_thrs = 100;

	int hist_sz[4];

	hist_sz[0] = 3;

	hist_sz[1] = 3;

	hist_sz[2] = 3;

	hist_sz[3] = max_label;


	Mat hist1 = Mat(4,hist_sz,CV_32S, Scalar::all(0));

	cout << "Start HOG" << endl;

	collect_hog(label_img,hist1,base_thrs,PixelIdxListx, PixelIdxListy, PixelIdxListz);

	cout << "Finish HOG" << endl;

	label_img.release();


	int hist_page = 3 * 3 * 3;

	// get the local features

	Mat centre_cc1 = Mat(max_label,3,CV_32F,Scalar::all(0));

	int cb_sz[4];

	cb_sz[0] = 7 * 2 + 1;

	cb_sz[1] = 7 * 2 + 1;

	cb_sz[2] = 3 * 2 + 1;

	cb_sz[3] = max_label;

	Mat cub1 = Mat(4,cb_sz,CV_8UC1,Scalar::all(0));


	cout << "Start Local Features" << endl;

	local_ftrs(input_img,centre_cc1, cub1,PixelIdxListx,PixelIdxListy,PixelIdxListz);

	cout << "Finish Local Features" << endl;

	// calculate the minimal distance to the main base and their features

	vector <int>frag_list;

	vector <int>base_list;


	Mat frag_p1 = Mat(3,max_label,CV_32S,Scalar::all(0));

	Mat base_p1 = Mat(3,max_label,CV_32S,Scalar::all(0));


	double *curvature_l = new double[max_label];

	int *min_dist1 = new int[max_label];

	cout << "start pre-linking the components" << endl;

	cout << "max Label is " << max_label << endl;


	//link_ftrs(min_dist1,curvature_l, base_thrs, bnd, frag_p1,base_p1, PixelIdxListx, PixelIdxListy,PixelIdxListz);

	link_ftrs1(min_dist1,curvature_l, base_pl, bnd, frag_p1,base_p1, PixelIdxListx, PixelIdxListy,PixelIdxListz);

	cout << "complete pre-linking the components" << endl;

	int cub_page;

	cub_page = (2 * cb_sz[0] + 1) * (2 * cb_sz[1] + 1) * (2 * cb_sz[2] + 1);

	//double feature_cc[max_label][4 + cub_page + hist_page];

	Mat feature_cc(max_label,6 + cub_page + hist_page,CV_32F);

	// fill the features for individual component

	cout << "start filling" << endl;

	fill_features(feature_cc,PixelIdxListx, bnd,min_dist1, curvature_l,hist1,cub1);

	cout << "complete filling" << endl;

	// now starts to apply the learned random forest

	int n_samp = feature_cc.rows;

	cout << n_samp << endl;

	// apply the random forest to test the image
	double * resp_tst = new double[n_samp + 100];

	for(int i = 0; i < n_samp; i++)
		resp_tst[i] = 0;


	//  v3d_msg(" Start classifying");

	cout << "Start classifying" << endl;

	LCM_classify(feature_cc, resp_tst,dataset);

	cout << "Complete classifying" << endl;

	//rt_test(feature_cc, resp_tst);

	int * fila_frag = new int[n_samp];

	for(int i = 0; i < feature_cc.rows; i++)
	{
		fila_frag[i] = resp_tst[i] > 0 ? 1 : 0;

		if(base_pl[i] > 0)
			fila_frag[i] = 2;
		else{
			if(feature_cc.at<float>(i,0) >5)
				fila_frag[i] = 1;

		}


	}

	for(int i = 0; i < feature_cc.rows; i++)
	{
		// cout << fila_frag[i] << endl;

		cout << "The classified result " << i << " is " << resp_tst[i] << endl;

	}

	vector<vector<trip_idx> > Pathxyz(max_label);

	for(int i = 0; i < feature_cc.rows; i++)
	{


		cout << i << endl;

		cout << feature_cc.at<float>(i,0) << endl;

		cout << fila_frag[i] << endl;

	}


	// attempt to connect the individual components

	// index of fila_frag is different from that of the labe_img

	cout << "start recovering the path" << endl;





	// recover_component(image,label_img,fila_frag,Pathxyz,PixelIdxListx,PixelIdxListy,PixelIdxListz);

	cout << "complete recovering the path" << endl;

	// form the segmented image

	seg_img.release();

	seg_img = Mat(3,img_sz,CV_8UC1,Scalar::all(0));

	cout << "max_label is " << max_label << endl;

	for(int i = 0; i < max_label; i++)
	{

		//   cout << "fila_frag " << i << " is " << fila_frag[i] << endl;

		if(fila_frag[i] == 1)
		{
			int n_p = Pathxyz[i].size();

			//       cout << "n_p is " << n_p << endl;

			//     cout <<  " Reconstruct Seed " << i << endl;

			for(int j = 0; j < n_p; j ++)
			{
				int v3[3];

				v3[0] = Pathxyz[i][j].x;

				v3[1] = Pathxyz[i][j].y;

				v3[2] = Pathxyz[i][j].z;

				seg_img.at<uchar>(v3) = 254;

				//cout << i << "  " << j << endl;

			}


		}

	}





	uchar t3c = 5;

	dt = 15;

	for(V3DLONG iz = 0; iz < sz[2]; iz++)
	{

		V3DLONG offsetk = iz*sz[1]*sz[0];
		for(V3DLONG iy = 0; iy <  sz[1]; iy++)
		{
			V3DLONG offsetj = iy* sz[0];
			for(V3DLONG ix = 0; ix < sz[0]; ix++)
			{


				int v3[3];

				v3[0] = (int)ix;

				v3[1] = (int)iy;

				v3[2] = (int)iz;

				// cout << ix << endl;

				// cout << iy << endl;

				// cout << iz << endl;

				uchar PixelValue = image.at<uchar>(v3);

				if(PixelValue > t1c)
					seg_img.at<uchar>(v3) = 254;


				//uchar PixelValue = image.at<uchar>(v3);

				PixelValue = input_img.at<uchar>(v3);

				float dist_b = dist_t1[offsetk + offsetj + ix];



				if((PixelValue > t3c) & ( dist_b < dt ))
					seg_img.at<uchar>(v3) = 254;


			}

		}
	}

	//	delete [] dist_t1;

	label_img.release();

	max_label = bwconnmp_img3(seg_img,label_img,254);

	cout << "Complete the bwconnmp on the images" << endl;

	cout << max_label << endl;

	// find how many labels are there in the image

	vector<vector<int> > PixelIdxListx2(max_label);

	vector<vector<int> > PixelIdxListy2(max_label);

	vector<vector<int> > PixelIdxListz2(max_label);

	bwconnmp(label_img,PixelIdxListx2,PixelIdxListy2,PixelIdxListz2);


	delete [] fila_frag;

	fila_frag = new int[max_label];


	// clean the tiny fragments

	for(int i = 0; i < max_label; i++)
	{
		int np = PixelIdxListx2[i].size();

		//  cout << "np of base " << i << " is " << np << endl;

		fila_frag[i] = 0;

		if(np > 1000)
			fila_frag[i] = 2;
		else{

			if(np > 10)
				fila_frag[i] = 1;

		}


		if(np < 10)
		{

			for(int j = 0; j < np ; j++)
			{

				int v3[3];

				v3[0] =  PixelIdxListx2[i][j];

				v3[1] =  PixelIdxListy2[i][j];

				v3[2] =  PixelIdxListz2[i][j];

				seg_img.at<uchar>(v3) = 0;

			}

		}



	}

	vector<vector<trip_idx> > Pathxyz1(max_label);

	cout << "start recovering the path" << endl;

	// cv::FileStorage file("Mat_img.ext", cv::FileStorage::WRITE);

	//  file << "image" <<image;

	//  file << "label_img" <<label_img;


	//recover_component(image,label_img,fila_frag,Pathxyz1,PixelIdxListx2,PixelIdxListy2,PixelIdxListz2);

	cout << "complete recovering the path" << endl;

	//  seg_img.release();

	//  seg_img = Mat(3,img_sz,CV_8UC1,Scalar::all(0));


	int se[3];

	se[0] = 3;

	se[1] = 3;

	se[2] = 3;

	int * dil_v = new int[se[0] * se[1] * se[2] * 3];


	for(int i = 0; i < max_label; i++)
	{

		if(fila_frag[i] == 1)
		{
			int n_p = Pathxyz1[i].size();

			//cout << "n_p is " << n_p << endl;

			for(int j = 0; j < n_p ; j++)
			{

				int v3[3];

				v3[0] =  PixelIdxListx2[i][j];

				v3[1] =  PixelIdxListy2[i][j];

				v3[2] =  PixelIdxListz2[i][j];

				seg_img.at<uchar>(v3) = 254;

			}



		}

	}

	cout << "ready to the post process" << endl;

	dt = 3;

	t3c = 5;

	for(V3DLONG iz = 0; iz < sz[2]; iz++)
	{

		V3DLONG offsetk = iz*sz[1]*sz[0];
		for(V3DLONG iy = 0; iy <  sz[1]; iy++)
		{
			V3DLONG offsetj = iy* sz[0];
			for(V3DLONG ix = 0; ix < sz[0]; ix++)
			{


				int v3[3];

				v3[0] = (int)ix;

				v3[1] = (int)iy;

				v3[2] = (int)iz;

				// cout << ix << endl;

				// cout << iy << endl;

				// cout << iz << endl;

				uchar PixelValue1 = seg_img.at<uchar>(v3);


				//uchar PixelValue = image.at<uchar>(v3);

				uchar PixelValue = input_img.at<uchar>(v3);

				float dist_b = dist_t1[offsetk + offsetj + ix];


				if( ( dist_b < dt ))
					seg_img.at<uchar>(v3) = 128;


			}

		}
	}




	  delete [] dil_v;



	  // draw the base image

	  label_img.release();

	  delete [] dist_t1;


	  delete [] resp_tst;

	  delete [] bnd;

	  delete [] curvature_l;

	  delete [] min_dist1;

	  delete [] fila_frag;

*/

	  return 1;

}

int retrieve_low_confidence(cv::Mat input_img, cv::Mat image, cv::Mat & seg_img, char * dataset)
{


	cout << "Ready to do the bwconnmp on the images" << endl;

	Mat label_img;

	int n_t2 = bwconnmp_img4(seg_img,label_img,100);

	cout << "Complete the bwconnmp on the images" << endl;

	cout << "There are " << n_t2 <<  " components in the image" << endl;

	// find how many labels are there in the image

	vector<vector<int> > PList_t2_x(n_t2);

	vector<vector<int> > PList_t2_y(n_t2);

	vector<vector<int> > PList_t2_z(n_t2);

	bwconnmp(label_img,PList_t2_x,PList_t2_y,PList_t2_z);

	//float * PLmd = new float[n_t2];

	int * base_pl = new int[n_t2];

	for(int i = 0; i < n_t2; i++)
	{

        int np = PList_t2_x[i].size();

        cout << np << endl;

        int mean_seg = 0;

		for(int j = 0; j < np; j++)
		{

			int ix = PList_t2_x[i][j];

			int iy = PList_t2_y[i][j];

			int iz = PList_t2_z[i][j];

			int v3[3];

			v3[0] = ix;

			v3[1] = iy;

			v3[2] = iz;

            mean_seg = mean_seg + (int)seg_img.at<uchar>(v3);

		}

		mean_seg = mean_seg / np;


        if(mean_seg > 120)
            base_pl[i] = 1;
        else
            base_pl[i] = 0;

	}



	// be very cautious the 0 of PixelIdxListx is label 1
	cout << "start the determine_bounding_box on the images" << endl;

	// find the bounding box for individual component

	int *bnd = new int[n_t2 * 2 * 3];

	determine_bounding_box(bnd,PList_t2_x, PList_t2_y, PList_t2_z);

	cout << "Complete the determine_bounding_box on the images" << endl;

	int hist_sz[4];

	hist_sz[0] = 3;

	hist_sz[1] = 3;

	hist_sz[2] = 3;

	hist_sz[3] = n_t2;


	Mat hist1 = Mat(4,hist_sz,CV_32S, Scalar::all(0));

	cout << "Start HOG" << endl;

	collect_hog1(label_img,hist1,base_pl,PList_t2_x, PList_t2_y, PList_t2_z);

	cout << "Finish HOG" << endl;

	label_img.release();

	int hist_page = 3 * 3 * 3;


	// get the local features

	Mat centre_cc1 = Mat(n_t2,3,CV_32F,Scalar::all(0));

	int cb_sz[4];

	cb_sz[0] = 1 * 2 + 1;

	cb_sz[1] = 1 * 2 + 1;

	cb_sz[2] = 1 * 2 + 1;

	cb_sz[3] = n_t2;

	Mat cub1 = Mat(4,cb_sz,CV_8UC1,Scalar::all(0));

	cout << "Start Local Features" << endl;

	local_ftrs(input_img,centre_cc1, cub1,PList_t2_x,PList_t2_y,PList_t2_z);

	cout << "Finish Local Features" << endl;

	// calculate the minimal distance to the main base and their features

	//vector <int>frag_list;

    //	vector <int>base_list;

	Mat frag_p1 = Mat(3,n_t2,CV_32S,Scalar::all(0));

	Mat base_p1 = Mat(3,n_t2,CV_32S,Scalar::all(0));

	double *curvature_l = new double[n_t2];

	int *min_dist1 = new int[n_t2];

	cout << "start pre-linking the components" << endl;

	cout << "label image has " << n_t2  << " components" << endl;

	//link_ftrs(min_dist1,curvature_l, base_thrs, bnd, frag_p1,base_p1, PixelIdxListx, PixelIdxListy,PixelIdxListz);

	link_ftrs1(min_dist1,curvature_l, base_pl, bnd, frag_p1, base_p1, PList_t2_x, PList_t2_y, PList_t2_z);

	cout << "complete pre-linking the components" << endl;

	int cub_page;

	cub_page = (2 * cb_sz[0] + 1) * (2 * cb_sz[1] + 1) * (2 * cb_sz[2] + 1);

	//double feature_cc[max_label][4 + cub_page + hist_page];

    Mat feature_cc = Mat(n_t2,6 + cub_page + hist_page,CV_32F);

	// fill the features for individual component

	cout << "start filling" << endl;

	fill_features(feature_cc,PList_t2_x, bnd,min_dist1, curvature_l,hist1,cub1);

	cout << "complete filling" << endl;

	// now starts to apply the learned random forest

	int n_samp = n_t2;

	cout << n_samp << endl;

	// apply the random forest to test the image
	double * resp_tst = new double[n_samp];

	for(int i = 0; i < n_samp; i++)
		resp_tst[i] = 1;


	//  v3d_msg(" Start classifying");


	cout << "Start classifying" << endl;

	//LCM_classify(feature_cc, resp_tst,dataset);

	cout << "Complete classifying" << endl;

	//rt_test(feature_cc, resp_tst);

    /*

	int * fila_frag = new int[n_samp];

	for(int i = 0; i < feature_cc.rows; i++)
	{
		fila_frag[i] = resp_tst[i] > 0 ? 1 : 0;

		if(base_pl[i] > 0)
			fila_frag[i] = 2;
		else{
			if(feature_cc.at<float>(i,0) >5)
				fila_frag[i] = 1;

		}


	}

	*/



	for(int i = 0; i < n_t2; i++)
	{

		//   cout << "fila_frag " << i << " is " << fila_frag[i] << endl;

		if((resp_tst[i] < 0)&(base_pl[i] < 0))
		{
			int n_p = PList_t2_x[i].size();

			//       cout << "n_p is " << n_p << endl;

			//     cout <<  " Reconstruct Seed " << i << endl;

			for(int j = 0; j < n_p; j ++)
			{
				int v3[3];

				v3[0] = PList_t2_x[i][j];

				v3[1] = PList_t2_y[i][j];

				v3[2] = PList_t2_z[i][j];

				seg_img.at<uchar>(v3) = 0;

			}


		}

	}






}





int imdilate_v(int v3[], int *dil_v,int img_sz[],int sz[])
{

    int mv[3];

     mv[0] = (sz[0] + 1) / 2;

     mv[1] = (sz[1] + 1) / 2;

     mv[2] = (sz[2] + 1) / 2;

     int of;


     int x,y,z;

     for(int i = 0; i < sz[0]; i ++)
     {

        for(int j = 0; j < sz[1]; j ++)
        {
            for(int k = 0; k < sz[2]; k ++)
            {
                of = (i * sz[1] * sz[2] + j * sz[2] + k) * 3;

                x = v3[0] - mv[0] + 1 + i;

                y = v3[1] - mv[1] + 1 + j;

                z = v3[2] - mv[2] + 1 + k;

                x = max(0,x);

                x = min(x,img_sz[0] - 1);

                y = max(0,y);

                y = min(y,img_sz[1] - 1);


                z = max(0,z);

                z = min(z,img_sz[2] - 1);


                dil_v[of] = x;

                dil_v[of + 1] = y;

                dil_v[of + 2] = z;


            }

        }

     }

}

int count_n_labels_img(Mat label_img, int * &np )
{
    int max_label = 0;

    //cout << " Before doing max_element "  << endl;



    max_label = (int)*(max_element(label_img.begin<int>(),label_img.end<int>()));

    if(max_label < 1)
        return max_label;

    //cout << "The max label is " << max_label << endl;

    int img_sz[3];

    for(int i = 0; i < 3; i++)
        img_sz[i] = label_img.size[i];

    np = new int[max_label];

    for(int i = 0; i < max_label; i++)
        np[i] = 0;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                int label = label_img.at<int>(v3);

                if(label > 0)
                    np[label - 1] ++;

            }

        }
    }


    return max_label;

}

int count_x_labels_img(Mat label_img, int target, int * &tx, int * &ty, int * &tz)
{
   // int max_label = 0;

    //cout << " Before doing max_element "  << endl;

    //max_label = (int)*(max_element(label_img.begin<int>(),label_img.end<int>()));

    //if(max_label < 1)
      //  return max_label;

    //cout << "The max label is " << max_label << endl;

    int img_sz[3];

    for(int i = 0; i < 3; i++)
        img_sz[i] = label_img.size[i];


    int * np;

    count_n_labels_img(label_img,np);


        int n_target = np[target - 1];



        tx = new int[n_target];

        ty = new int[n_target];

        tz = new int[n_target];


        delete [] np;



   // np = new int[max_label];

//    for(int i = 0; i < max_label; i++)
  //      np[i] = 0;

    int it = 0;

    for(int iz = 0; iz < img_sz[2]; iz++)
    {
        int offsetk = iz * img_sz[1] * img_sz[0];
        for(int iy = 0; iy <  img_sz[1]; iy++)
        {
            int offsetj = iy * img_sz[0];
            for(int ix = 0; ix < img_sz[0]; ix++)
            {

                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                int label = label_img.at<int>(v3);

                if(label == target)
                {

                    tx[it] = ix;

                    ty[it] = iy;

                    tz[it] = iz;


                    it++;

                }

            }

        }
    }


    return n_target;

}




int count_labels_img(Mat label_img)
{
	  int max_label = 0;

	  max_label = (int)*(max_element(label_img.begin<uchar>(),label_img.end<uchar>()));

	  return max_label;

}

int bwconnmp_img(Mat input_img, Mat &label_img)
{
	// collect the PixelIdxList for individual component

	int im_sz[3];


	for(int i = 0; i < 3; i++)
		im_sz[i] = input_img.size[i];


	int *img = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	for(int k = 0; k < im_sz[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < im_sz[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < im_sz[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

				int lc = (int)input_img.at<uchar>(v3);

				if(lc > 0)
					lc = 1;

				img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = lc;

			}


		}

	}

	int component_num;

	int * c_label = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	component_num = i4block_components(im_sz[0], im_sz[1], im_sz[2], img, c_label);

	delete [] img;

	label_img = Mat(3,im_sz,CV_32S,Scalar::all(0));


	for(int k = 0; k < label_img.size[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < label_img.size[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < label_img.size[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

				//int lc = int(label_img.at<uchar>(v3));

				label_img.at<int>(v3) = c_label[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]];

			}

		}

	}

	delete [] c_label;

	return component_num;

}



int bwconnmp_img2(Mat input_img, int * &c_label, uchar sv)
{
	// collect the PixelIdxList for individual component

	int im_sz[3];


	for(int i = 0; i < 3; i++)
		im_sz[i] = input_img.size[i];


	int *img = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	for(int k = 0; k < im_sz[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < im_sz[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < im_sz[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

				uchar lc = input_img.at<uchar>(v3);

				if(lc == sv)
					img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = 1;
                else
                    img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = 0;


			}


		}

	}

	int component_num;

	c_label = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	component_num = i4block_components(im_sz[0], im_sz[1], im_sz[2], img, c_label);

	delete [] img;

	return component_num;

}

int bwconnmp_img3(Mat input_img, Mat &label_img, uchar cv)
{
	// collect the PixelIdxList for individual component

	int im_sz[3];


	for(int i = 0; i < 3; i++)
		im_sz[i] = input_img.size[i];


	int *img = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	for(int k = 0; k < im_sz[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < im_sz[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < im_sz[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

                uchar lc = input_img.at<uchar>(v3);

				if(lc == cv)
					img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = 1;
                else
                    img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = 0;

			}


		}

	}

	int component_num;

	int * c_label = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	component_num = i4block_components(im_sz[0], im_sz[1], im_sz[2], img, c_label);

	delete [] img;

	label_img = Mat(3,im_sz,CV_32S,Scalar::all(0));


	for(int k = 0; k < label_img.size[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < label_img.size[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < label_img.size[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

				//int lc = int(label_img.at<uchar>(v3));

				label_img.at<int>(v3) = c_label[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]];

			}

		}

	}

	delete [] c_label;

	return component_num;

}


int bwconnmp_img4(Mat input_img, Mat &label_img, uchar cv)
{
	// collect the PixelIdxList for individual component

	int im_sz[3];


	for(int i = 0; i < 3; i++)
		im_sz[i] = input_img.size[i];

 //   cout << "img_sz is " << im_sz[0] << im_sz[1] << im_sz[2] << endl;

	int *img = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	for(int k = 0; k < im_sz[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < im_sz[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < im_sz[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

                uchar lc = input_img.at<uchar>(v3);


				if(lc > cv){



					img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = 1;

                }
                else
                    img[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]] = 0;

			}


		}

	}

	int sum_img = accumulate(img,img + im_sz[0] * im_sz[1] * im_sz[2],0);

	cout << "sum of the image in the bwconnomp 4 is " << sum_img <<  endl;

//	int n_img = (int)accumulate(img,img + im_sz[0] * im_sz[1] * im_sz[2], 0);

//	cout << "The sum of the image is " << n_img << endl;

	int component_num;

	int * c_label = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	component_num = i4block_components(im_sz[0], im_sz[1], im_sz[2], img, c_label);

	delete [] img;

	label_img = Mat(3,im_sz,CV_32S,Scalar::all(0));

	sum_img = 0;


	for(int k = 0; k < label_img.size[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < label_img.size[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < label_img.size[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

				//int lc = int(label_img.at<uchar>(v3));

				label_img.at<int>(v3) = c_label[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]];

				//int lc = c_label[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]];

				//if(lc > 0)
                  //  sum_img ++;




			}

		}

	}

    sum_img = accumulate(c_label,c_label + im_sz[0] * im_sz[1] * im_sz[2],0);

	cout << "sum of the label is " << sum_img << endl;





	delete [] c_label;

	return component_num;

}


/*

int bwconnmp_img1(int * input_img, Mat &label_img, int im_sz[])
{
	// collect the PixelIdxList for individual component


	int component_num;

	int * c_label = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	component_num = i4block_components(im_sz[0], im_sz[1], im_sz[2], img, c_label);

	delete [] img;

	label_img = Mat(3,im_sz,CV_32S,Scalar::all(0));


	for(int k = 0; k < label_img.size[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < label_img.size[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < label_img.size[0]; i ++)
			{

				int v3[3];

				v3[0] = i;

				v3[1] = j;

				v3[2] = k;

				//int lc = int(label_img.at<uchar>(v3));

				label_img.at<int>(v3) = c_label[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]];

			}

		}

	}

	delete [] c_label;

	return component_num;

}


*/


int bwconnmp(Mat label_img,vector<vector<int> > &PixelIdxListx,vector<vector<int> > &PixelIdxListy,vector<vector<int> > &PixelIdxListz)
{
		  // collect the PixelIdxList for individual component

	  for(int i = 0; i < label_img.size[0]; i++)
	  {
		  //uchar* rp = label_img.ptr<uchar>(i);

		  for(int j = 0; j < label_img.size[1] ; j ++)
		  {
			  //int offsetj = j * label_img.size[2];

			  for(int k = 0; k < label_img.size[2]; k ++)
			  {

				  int v3[3];

				  v3[0] = i;

				  v3[1] = j;

				  v3[2] = k;

				  int lc = (int)label_img.at<int>(v3);

			  	  if(lc > 0)
			  	  {

				  	  PixelIdxListx[lc - 1].push_back(i);

				  	  PixelIdxListy[lc - 1].push_back(j);

				  	  PixelIdxListz[lc - 1].push_back(k);

			  	  }

			  }


		  }

	  }

	  return 0;

}



int bwconnmp1(int * label_img,vector<vector<int> > &PixelIdxListx,vector<vector<int> > &PixelIdxListy,vector<vector<int> > &PixelIdxListz, V3DLONG sz[])
{
		  // collect the PixelIdxList for individual component


    for(V3DLONG iz = 0; iz < sz[2]; iz++)
    {

        V3DLONG offsetk = iz*sz[1]*sz[0];
        for(V3DLONG iy = 0; iy <  sz[1]; iy++)
        {
            V3DLONG offsetj = iy* sz[0];
            for(V3DLONG ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                int lc = label_img[offsetk + offsetj + ix];

			  	  if(lc > 0)
			  	  {

				  	  PixelIdxListx[lc - 1].push_back((int)ix);

				  	  PixelIdxListy[lc - 1].push_back((int)iy);

				  	  PixelIdxListz[lc - 1].push_back((int)iz);

			  	  }

            }
        }

    }

  return 1;

}

int determine_bounding_box(int *bnd,vector<vector<int> > PixelIdxListx,vector<vector<int> > PixelIdxListy,vector<vector<int> > PixelIdxListz)
{

	int max_label = PixelIdxListx.size();

	  for(int i = 0; i < max_label; i++)
	  {

        int n_cc = PixelIdxListx[i].size();

       // cout << i << "   " << n_cc << max_label << endl;

        if(n_cc > 0)
        {

        bnd[i * 2] = (int)*min_element(PixelIdxListx[i].begin(),PixelIdxListx[i].end());

        bnd[i * 2 + 1] = (int)*max_element(PixelIdxListx[i].begin(),PixelIdxListx[i].end());


        bnd[i * 2 + max_label * 2] = (int)*min_element(PixelIdxListy[i].begin(),PixelIdxListy[i].end());

        bnd[i * 2 + 1 + max_label * 2] = (int)*max_element(PixelIdxListy[i].begin(),PixelIdxListy[i].end());


        bnd[i * 2 + max_label * 4] = (int)*min_element(PixelIdxListz[i].begin(),PixelIdxListz[i].end());

        bnd[i * 2 + 1 + max_label * 4] = (int)*max_element(PixelIdxListz[i].begin(),PixelIdxListz[i].end());
        }else
        {

            bnd[i * 2] = 0;

            bnd[i * 2 + 1] = 0;

            bnd[i * 2 + max_label * 2]  = 0;

            bnd[i * 2 + 1 + max_label * 2]  = 0;

            bnd[i * 2 + max_label * 4]  = 0;

            bnd[i * 2 + 1 + max_label * 4]  = 0;


        }

        //cout << i << endl;


	  }

	  return 1;

}

int collect_hog(Mat label_img,Mat & hist1,int base_thrs,vector<vector<int> > PixelIdxListx,vector<vector<int> > PixelIdxListy,vector<vector<int> > PixelIdxListz)
{

	int max_label = PixelIdxListx.size();



	int *bnd = new int[max_label * 2 * 3];

	determine_bounding_box(bnd,PixelIdxListx, PixelIdxListy, PixelIdxListz);


	//initialise the histogram of the fragment

	  for(int l = 0; l < max_label; l++)
	  {

	      int n_cc = PixelIdxListz[l].size();

	      if(n_cc < base_thrs)
	      {
	    	  for(int i = bnd[2 * l]; i < bnd[2 * l + 1]; i++)
	    	  {

	    		  for(int j = bnd[2 * l + max_label * 2]; j < bnd[2 * l + 1 + max_label * 2] ; j ++)
	    		  {

	    			  for(int k = bnd[2 * l + max_label * 4]; k < bnd[2 * l + 1 + max_label * 4]; k ++)
	    			  {

	    				  int g3d[4];

	    				  int v3[3];

	    				  //uchar* rp = label_img.ptr<uchar>(i);

	    				  //int offsetj = j * label_img.channels();

	    				  v3[0] = i;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  int p1 = label_img.at<int>(v3);

	    				  p1 = (p1 == (l + 1)) ? 1: 0;

	    				  // gradient in the Z direction

	    				  v3[2] = k + 1;

	    				  int p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[2] = p1 - p2 + 1;

	    				  // gradient in the Y direction

	    				  v3[0] = i;

	    				  v3[1] = j + 1;

	    				  v3[2] = k;

	    				  p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[1] = p1 - p2 + 1;

	    				  // gradient in the X direction

	    				  v3[0] = i + 1;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[0] = p1 - p2 + 1;

	    				  g3d[3] = l;

	    				  int h_bin = hist1.at<int>(g3d);

	    				  h_bin ++;

	    				  hist1.at<int>(g3d) = h_bin;

	    				  //hist[g3d[0]][g3d[1]][g3d[2]][l] ++;


	    			  }


	    		  }

	    	  }

	      }

	  }

	  delete [] bnd;

	  // reset the count for null as zero

	  for(int k = 0; k < max_label; k++)
	  {
		  //hist[1][1][1][k] = 0;

		  int g3d[4];

		  g3d[0] = 1;

		  g3d[1] = 1;

		  g3d[2] = 1;

		  g3d[3] = k;

		  hist1.at<int>(g3d) = 0;

	  }


	  return 0;


}


int collect_hog1(Mat label_img,Mat & hist1,int * base_list,vector<vector<int> > PixelIdxListx,vector<vector<int> > PixelIdxListy,vector<vector<int> > PixelIdxListz)
{

	int max_label = PixelIdxListx.size();

	int *bnd = new int[max_label * 2 * 3];

	determine_bounding_box(bnd,PixelIdxListx, PixelIdxListy, PixelIdxListz);


	//initialise the histogram of the fragment

	  for(int l = 0; l < max_label; l++)
	  {

	      int n_cc = PixelIdxListz[l].size();

	      if(base_list[l] < 1)
	      {
	    	  for(int i = bnd[2 * l]; i < bnd[2 * l + 1]; i++)
	    	  {

	    		  for(int j = bnd[2 * l + max_label * 2]; j < bnd[2 * l + 1 + max_label * 2] ; j ++)
	    		  {

	    			  for(int k = bnd[2 * l + max_label * 4]; k < bnd[2 * l + 1 + max_label * 4]; k ++)
	    			  {

	    				  int g3d[4];

	    				  int v3[3];

	    				  //uchar* rp = label_img.ptr<uchar>(i);

	    				  //int offsetj = j * label_img.channels();

	    				  v3[0] = i;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  int p1 = label_img.at<int>(v3);

	    				  p1 = (p1 == (l + 1)) ? 1: 0;

	    				  // gradient in the Z direction

	    				  v3[2] = k + 1;

	    				  int p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[2] = p1 - p2 + 1;

	    				  // gradient in the Y direction

	    				  v3[0] = i;

	    				  v3[1] = j + 1;

	    				  v3[2] = k;

	    				  p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[1] = p1 - p2 + 1;

	    				  // gradient in the X direction

	    				  v3[0] = i + 1;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[0] = p1 - p2 + 1;

	    				  g3d[3] = l;

	    				  int h_bin = hist1.at<int>(g3d);

	    				  h_bin ++;

	    				  hist1.at<int>(g3d) = h_bin;

	    				  //hist[g3d[0]][g3d[1]][g3d[2]][l] ++;


	    			  }


	    		  }

	    	  }

	      }

	  }

	  delete [] bnd;

	  // reset the count for null as zero

	  for(int k = 0; k < max_label; k++)
	  {
		  //hist[1][1][1][k] = 0;

		  int g3d[4];

		  g3d[0] = 1;

		  g3d[1] = 1;

		  g3d[2] = 1;

		  g3d[3] = k;

		  hist1.at<int>(g3d) = 0;

	  }


	  return 1;


}


/*

int collect_hog1(int * label_img,Mat & hist1,int base_thrs,vector<vector<int> > PixelIdxListx,vector<vector<int> > PixelIdxListy,vector<vector<int> > PixelIdxListz,int sz[])
{

	int max_label = PixelIdxListx.size();



	int *bnd = new int[max_label * 2 * 3];

	determine_bounding_box(bnd,PixelIdxListx, PixelIdxListy, PixelIdxListz);


	//initialise the histogram of the fragment

	  for(int l = 0; l < max_label; l++)
	  {

	      int n_cc = PixelIdxListz[l].size();

	      if(n_cc < base_thrs)
	      {
	    	  for(int i = bnd[2 * l]; i < bnd[2 * l + 1]; i++)
	    	  {



	    		  for(int j = bnd[2 * l + max_label * 2]; j < bnd[2 * l + 1 + max_label * 2] ; j ++)
	    		  {



	    			  for(int k = bnd[2 * l + max_label * 4]; k < bnd[2 * l + 1 + max_label * 4]; k ++)
	    			  {

	    				  int g3d[4];

	    				  int v3[3];

	    				  //uchar* rp = label_img.ptr<uchar>(i);

	    				  //int offsetj = j * label_img.channels();

	    				  v3[0] = i;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  int offsetz = k * sz[1] * sz[0];

	    				  int offsety = j * sz[0];




	    				  int p1 = label_img.at<int>(v3);

	    				  p1 = (p1 == (l + 1)) ? 1: 0;

	    				  // gradient in the Z direction

	    				  v3[2] = k + 1;

	    				  int p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[2] = p1 - p2 + 1;

	    				  // gradient in the Y direction

	    				  v3[0] = i;

	    				  v3[1] = j + 1;

	    				  v3[2] = k;

	    				  p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[1] = p1 - p2 + 1;

	    				  // gradient in the X direction

	    				  v3[0] = i + 1;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  p2 = label_img.at<int>(v3);

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[0] = p1 - p2 + 1;

	    				  g3d[3] = l;

	    				  int h_bin = hist1.at<int>(g3d);

	    				  h_bin ++;

	    				  hist1.at<int>(g3d) = h_bin;

	    				  //hist[g3d[0]][g3d[1]][g3d[2]][l] ++;


	    			  }


	    		  }

	    	  }

	      }

	  }

	  delete [] bnd;

	  // reset the count for null as zero

	  for(int k = 0; k < max_label; k++)
	  {
		  //hist[1][1][1][k] = 0;

		  int g3d[4];

		  g3d[0] = 1;

		  g3d[1] = 1;

		  g3d[2] = 1;

		  g3d[3] = k;

		  hist1.at<int>(g3d) = 0;

	  }


	  return 0;


}

*/



// obtain the local patch
int local_ftrs(Mat image,Mat & centre_cc1,Mat & cub1,vector<vector<int> >  PixelIdxListx,vector<vector<int> >  PixelIdxListy,vector<vector<int> >  PixelIdxListz)
{
	int max_label = PixelIdxListx.size();

	int cb_sz[3];

	for(int i = 0; i < 3; i++)
	{
		int cd = (cub1.size[i] - 1) / 2;

		cb_sz[i] = cd ;

	}


	cout << "start calculating the weight centre" << endl;


// calculate the weight centre

	  for(int i = 0; i < max_label; i++)
	  {
		  	  int n_cc = PixelIdxListx[i].size();


			  float cc;

			  cc = float(accumulate(PixelIdxListx[i].begin(),PixelIdxListx[i].end(),0));

			  cc = cc / float(n_cc);

			  centre_cc1.at<float>(i,0) = cc;



			  cc = float(accumulate(PixelIdxListy[i].begin(),PixelIdxListy[i].end(),0));

			  cc = cc / float(n_cc);

			  centre_cc1.at<float>(i,1) = cc;



			  cc = float(accumulate(PixelIdxListz[i].begin(),PixelIdxListz[i].end(),0));

			  cc = cc / float(n_cc);

			  centre_cc1.at<float>(i,2) = cc;


	  }

	cout << "finish calculating the weight centre" << endl;


	  for(int l = 0; l < max_label; l++)
	  {

         // cout << " Collect the local features for CC" << l << endl;

		  int cx ;

		  cx = int(centre_cc1.at<float>(l,0));

		  int cy ;

		  cy = int(centre_cc1.at<float>(l,1));

		  int cz;

		  cz = int(centre_cc1.at<float>(l,2));

		  // keep the cubic from breaching the boundary

		  cx = max(cx,cb_sz[0] + 1);

		  cx = min(cx,image.size[0] - cb_sz[0] - 1);



		  cy = max(cy,cb_sz[1]);

		  cy = min(cy,image.size[1] - cb_sz[1] - 1);


		  cz = max(cz,cb_sz[2]);

		  cz = min(cz,image.size[2] - cb_sz[2] - 1);

		//  cout << "cx is " << cx << endl;

		 // cout << "cy is " << cy << endl;

		 // cout << "cz is " << cz << endl;



		  for(int i = 0; i < 2 * cb_sz[0] + 1; i++ )
		  {
			  //uchar* rp = image.ptr<uchar>(cx + i - cb_sz[0]);

			  for(int j = 0; j < 2 * cb_sz[1] + 1; j ++)
			  {
				//  int offsetj = (cy + j - cb_sz[1]) * image.channels();

				  for(int k = 0; k < 2 * cb_sz[2] + 1; k ++ )
				  {
					//  cub[i][j][k][l] = rp[cz - cb_sz[2] + k + offsetj];

					  int vc[4];

					  vc[0] = i;

					  vc[1] = j;

					  vc[2] = k;

					  vc[3] = l;

					  int v3[3];

					  v3[0] = cx + i - cb_sz[0];

					  v3[1] = cy + j - cb_sz[1];

					  v3[2] = cz + k - cb_sz[2];

                  /*

					  if(l > 900)
					  {



                        //cout << " i j k  " << i << " " << j << " " << k << " " << endl;

                        cout << " vc is " << vc[0] << " " << vc[1] << " " << vc[2]  << " " << vc[3]  << endl;


                        cout << " cub1 size is " << cub1.size[0] << " " << cub1.size[1] << " " << cub1.size[2] << " " << cub1.size[3] << endl;

                        cout << " image size is " << image.size[0] << " " << image.size[1] << " " << image.size[2] << endl;

                        cout << " v3 is " << v3[0] << " " << v3[1] << " " << v3[2]  << endl;

                        cub1.at<uchar>(vc) = image.at<uchar>(v3);

                      }

                      */

				  }

			  }

		  }


	  }

	  return 1;

}

int link_nearest(int *min_dist1, vector <int> frag_list, vector <int> base_list, Mat &frag_p1, Mat &base_p1,vector<vector<int> >  PixelIdxListx,vector<vector<int> >  PixelIdxListy,vector<vector<int> >  PixelIdxListz)
{

	int max_label = PixelIdxListx.size();

	int n_frag;

	int n_base;

	n_frag = frag_list.size();

	n_base = base_list.size();


	// initialise min_dist1

	for(int i = 0; i < max_label; i++)
		min_dist1[i] = 0;

	  for(int i = 0; i < n_frag; i ++)
	  {
		  int frag_i = frag_list.at(i);

		  int md = 1000000;

		  int xd;

		  int yd;

		  int zd;

		  int tmp_d;

		  int fp[3];

		  int bp[3];

		  int j;

		  int k;

		  int l;


		  for(j = 0; j < n_base; j ++)
		  {

			  int base_i = base_list.at(j);

			  int n_p1 = PixelIdxListx[frag_i].size();

			  for(k = 0; k < n_p1; k ++)
			  {

				  int n_p2 = PixelIdxListx[base_i].size();

				  for(l = 0; l < n_p2; l++)
				  {

					  xd = PixelIdxListx[frag_i][k] - PixelIdxListx[base_i][l];

					  yd = PixelIdxListy[frag_i][k] - PixelIdxListy[base_i][l];

					  zd = PixelIdxListz[frag_i][k] - PixelIdxListz[base_i][l];

					  tmp_d = sqrt(xd * xd + yd * yd + zd * zd);

					  if(tmp_d < md)
					  {
						  md = tmp_d;

						  fp[0] = PixelIdxListx[frag_i][k];

						  bp[0] = PixelIdxListx[base_i][l];


						  fp[1] = PixelIdxListy[frag_i][k];

						  bp[1] = PixelIdxListy[base_i][l];


						  fp[2] = PixelIdxListz[frag_i][k];

						  bp[2] = PixelIdxListz[base_i][l];

					  }

				  }

			  }

		  }

		  min_dist1[frag_i] = md;

		  frag_p1.at<int>(0,frag_i) = fp[0];

		  frag_p1.at<int>(1,frag_i) = fp[1];

		  frag_p1.at<int>(2,frag_i) = fp[2];


		  base_p1.at<int>(0,frag_i) = bp[0];

		  base_p1.at<int>(1,frag_i) = bp[1];

		  base_p1.at<int>(2,frag_i) = bp[2];

	  }


    return 1;

}


int link_nearest1(int *min_dist1, vector <int> frag_list, vector <int> base_list, Mat &frag_p1, Mat &base_p1,vector<vector<int> >  PixelIdxListx,vector<vector<int> >  PixelIdxListy,vector<vector<int> >  PixelIdxListz)
{

	int max_label = PixelIdxListx.size();

	int n_frag;

	int n_base;

	n_frag = frag_list.size();

	n_base = base_list.size();


	// initialise min_dist1

	for(int i = 0; i < max_label; i++)
		min_dist1[i] = 0;

	  for(int i = 0; i < n_frag; i ++)
	  {
		  int frag_i = frag_list.at(i);

		  int md = 1000000;

		  int xd;

		  int yd;

		  int zd;

		  int tmp_d;

		  int fp[3];

		  int bp[3];

		  int j;

		  int k;

		  int l;


		  for(j = 0; j < n_base; j ++)
		  {

			  int base_i = base_list.at(j);

			  int n_p1 = PixelIdxListx[frag_i].size();

			  for(k = 0; k < n_p1; k ++)
			  {

				  int n_p2 = PixelIdxListx[base_i].size();

				  for(l = 0; l < n_p2; l++)
				  {

					  xd = PixelIdxListx[frag_i][k] - PixelIdxListx[base_i][l];

					  yd = PixelIdxListy[frag_i][k] - PixelIdxListy[base_i][l];

					  zd = PixelIdxListz[frag_i][k] - PixelIdxListz[base_i][l];

					  tmp_d = sqrt(xd * xd + yd * yd + zd * zd);

					  if(tmp_d < md)
					  {
						  md = tmp_d;

						  fp[0] = PixelIdxListx[frag_i][k];

						  bp[0] = PixelIdxListx[base_i][l];


						  fp[1] = PixelIdxListy[frag_i][k];

						  bp[1] = PixelIdxListy[base_i][l];


						  fp[2] = PixelIdxListz[frag_i][k];

						  bp[2] = PixelIdxListz[base_i][l];

					  }

				  }

			  }

		  }

		  min_dist1[frag_i] = md;

		  frag_p1.at<int>(0,frag_i) = fp[0];

		  frag_p1.at<int>(1,frag_i) = fp[1];

		  frag_p1.at<int>(2,frag_i) = fp[2];


		  base_p1.at<int>(0,frag_i) = bp[0];

		  base_p1.at<int>(1,frag_i) = bp[1];

		  base_p1.at<int>(2,frag_i) = bp[2];

	  }


    return 1;

}


int curvature_link(double *curvature_l, int *bnd, vector <int> frag_list,Mat frag_p, Mat base_p)
{

     int	max_label = frag_p.cols;

	  double ori_f[3];

	  double ori_l[3];

	  for(int i = 0; i < max_label; i ++)
		  curvature_l[i] = 0;

	  int n_frag = frag_list.size();


	  for(int i = 0; i < n_frag; i++ )
	  {

		  int frag_i = frag_list.at(i);

		  ori_f[0] = double(bnd[frag_i * 2 + 1] - bnd[frag_i * 2]);

		  ori_f[1] = double(bnd[frag_i * 2 + 1 + 2 * max_label] - bnd[frag_i * 2 + 2 * max_label]);

		  ori_f[2] = double(bnd[frag_i * 2 + 1 + 4 * max_label] - bnd[frag_i * 2 + 4 * max_label]);

		  double norm_ori = ori_f[0] * ori_f[0] + ori_f[1] * ori_f[1] + ori_f[2] * ori_f[2];

		  norm_ori = sqrt(norm_ori);

		  for(int j = 0; j < 3; j ++)
			  ori_f[j] = ori_f[j] / norm_ori;


		  ori_l[0] = double(frag_p.at<int>(0,frag_i) - base_p.at<int>(0,frag_i));

		  ori_l[1] = double(frag_p.at<int>(1,frag_i) - base_p.at<int>(1,frag_i));

		  ori_l[2] = double(frag_p.at<int>(2,frag_i) - base_p.at<int>(2,frag_i));

		  norm_ori = ori_l[0] * ori_l[0] + ori_l[1] * ori_l[1] + ori_l[2] * ori_l[2];

		  norm_ori = sqrt(norm_ori);

		  for(int j = 0; j < 3; j ++)
			  ori_l[j] = ori_l[j] / norm_ori;




		  for(int j = 0; j < 3; j ++)
		  {

			//  cout << ori_f[j] << " " << ori_l[j] << endl;

			  curvature_l[frag_i] = abs(curvature_l[frag_i] + ori_f[j] * ori_l[j]);

		//	  cout << frag_i << " " << curvature_l[frag_i] << endl;

		  }

	  }

	  //for(int i = 0; i < max_label; i++)
		//  cout << curvature_l[i] << endl;

	//delete [] [] [] ori_f,ori_l;

	return 1;
}







int link_ftrs( int *min_dist1, double * curvature_l, int base_thrs, int *bnd, Mat & frag_p1,  Mat & base_p1, vector<vector<int> >  PixelIdxListx,vector<vector<int> >  PixelIdxListy,vector<vector<int> >  PixelIdxListz)
{

	int max_label = PixelIdxListx.size();

	vector <int>frag_list;

	vector <int>base_list;


	for(int i = 0; i < max_label; i ++)
	{

		int n_cc = PixelIdxListx[i].size();

		if(n_cc < base_thrs)
			frag_list.push_back(i);
		else
			base_list.push_back(i);

            int f_b = 0;

            if(n_cc < base_thrs)
               f_b = 1;

               cout << f_b << endl;



	}

	cout << "max label is " << max_label << endl;


//	  int *min_dist1 = new int[max_label];



	  link_nearest(min_dist1, frag_list, base_list, frag_p1,base_p1, PixelIdxListx,PixelIdxListy, PixelIdxListz);


	  // calculate the orientation of the link


	  curvature_link(curvature_l, bnd, frag_list, frag_p1, base_p1);


	  return 0;

}


int link_ftrs1( int *min_dist1, double * curvature_l, int * base_pl, int *bnd, Mat & frag_p1,  Mat & base_p1, vector<vector<int> >  PixelIdxListx,vector<vector<int> >  PixelIdxListy,vector<vector<int> >  PixelIdxListz)
{

	int max_label = PixelIdxListx.size();

	vector <int>frag_list;

	vector <int>base_list;


	for(int i = 0; i < max_label; i ++)
	{

		int n_cc = base_pl[i];

		if(n_cc ==  0)
			frag_list.push_back(i);
		else
			base_list.push_back(i);

            int f_b = 0;

            //if(n_cc < base_thrs)
              // f_b = 1;

               //cout << f_b << endl;

	}

	cout << "max label is " << max_label << endl;


//	  int *min_dist1 = new int[max_label];



	  link_nearest(min_dist1, frag_list, base_list, frag_p1,base_p1, PixelIdxListx,PixelIdxListy, PixelIdxListz);


	  // calculate the orientation of the link


	  curvature_link(curvature_l, bnd, frag_list, frag_p1, base_p1);


	  return 0;

}





int fill_features(Mat & feature_cc,vector<vector<int> > PixelIdxListx, int * bnd,int *min_dist1, double *curvature_l, Mat hist1, Mat cub1)
{
	int max_label = PixelIdxListx.size();

	  for(int k = 0; k < max_label; k++ )
	  {
		 int cub_sz[3];

		 cub_sz[0] = bnd[2 * k + 1] - bnd[2 * k];

		 cub_sz[1] = bnd[2 * k + 1 + 2 * max_label] - bnd[2 * k + 2 * max_label];

		 cub_sz[2] = bnd[2 * k + 1 + 4 * max_label] - bnd[2 * k + 4 * max_label];

		 int *max_axe_p = max_element(cub_sz,cub_sz + 3);

		 int max_axe = *max_axe_p;

		 int *min_axe_p = min_element(cub_sz,cub_sz + 3);

		 int min_axe = *min_axe_p;

		 int max_axe_idx = int(max_axe_p - cub_sz);

		 int min_axe_idx = int(min_axe_p - cub_sz);

		 cub_sz[max_axe_idx] = 0;

		 cub_sz[min_axe_idx] = 0;

		 int mid_axe = *max_element(cub_sz,cub_sz + 3);

		 int nPixel = PixelIdxListx[k].size();

		 feature_cc.at<float>(k,0) = float(nPixel);

		 feature_cc.at<float>(k,1) = float(max_axe);

		 feature_cc.at<float>(k,2) = float(mid_axe);

		 feature_cc.at<float>(k,3) = float(min_axe);

		 int min_d = min_dist1[k];

		 feature_cc.at<float>(k,4) = float(min_d);

		 double cur_l = curvature_l[k];

		 feature_cc.at<float>(k,5) = float(cur_l);

		 int fi = 0;

		 fi = 0;

		 for(int l = 0; l < 3; l++)
		 {
			 for(int j = 0; j < 3; j++)
			 {
				 for(int i = 0; i < 3; i++)
				 {
					 int v;

					 //v = hist.at<int>(i,j,l,k);


				//	 v = hist[i][j][l][k];

					 int vc[4];

					 vc[0] = i;

					 vc[1] = j;

					 vc[2] = l;

					 vc[3] = k;

					 v = hist1.at<int>(vc);

					 feature_cc.at<float>(k,6 + fi) = float(v);

					 fi++;
				 }

			 }
		 }

		// int cb_sz[3];

		// for(int i = 0; i < 3; i++)
		//	 cb_sz[i] = cub1.size[i];

		 for(int l = 0; l < cub1.size[2]; l++)
		 {
			 for(int j = 0; j <  cub1.size[1]; j++)
			 {
				 for(int i = 0; i < cub1.size[0]; i++)
				 {
					 float v;

					// v = float(cub[i][j][l][k]);

					 int * vc = new int[4];

					 vc[0] = i;

					 vc[1] = j;

					 vc[2] = l;

					 vc[3] = k;


					 v = cub1.at<float>(vc);

					 feature_cc.at<float>(k,6 + fi) = v;

					 fi++;
				 }

			 }
		 }


	  }


	return 1;
}


int rt_test(Mat feature_cc, double * resp_tst)
{
	  Ptr<RTrees> rtrees = StatModel::load<RTrees>("rf.xml");

	  for(int i = 0; i < feature_cc.rows; i++)
	  {
		  Mat testSample;

		  testSample = feature_cc.row(i);

		  float resp_elmt = rtrees->predict(testSample);

		  resp_tst[i] = double(resp_elmt);

	  }

	  return 1;


}

int recover_component(cv::Mat image, cv::Mat label_img, int * fila_frag, std::vector<std::vector<trip_idx> > &Pathxyz,
		std::vector<std::vector<int> > PixelIdxListx,std::vector<std::vector<int> > PixelIdxListy,
		std::vector<std::vector<int> > PixelIdxListz)
{

	int n_label = PixelIdxListx.size();

	int img_sz[3];

	for(int i = 0; i < 3; i++)
		img_sz[i] = image.size[i];


	int * dist_frag2base = new int[n_label];

	int base_thrs = 1000;

	vector <int>frag_list;

	vector <int>base_list;


	for(int i = 0; i < n_label; i ++)
	{

		//int n_cc = PixelIdxListx[i].size();

        if(fila_frag[i] == 2)
            base_list.push_back(i);
            else{
            if(fila_frag[i] == 1)
                frag_list.push_back(i);

            }

	}




	cout << "There are " << base_list.size() << " Base fragments" << endl;


	cout << "There are " << frag_list.size() << " frag fragments" << endl;


	cout << "There are " << n_label << "  fragments" << endl;


    int frag_list_size = frag_list.size();

    if(frag_list_size < 1)
        return 0;




	Mat frag_p1 = Mat(3,n_label,CV_32S,Scalar::all(0));

	Mat base_p1 = Mat(3,n_label,CV_32S,Scalar::all(0));


	cout << "Ready to link the components" << endl;


	link_nearest(dist_frag2base, frag_list, base_list, frag_p1,base_p1, PixelIdxListx,PixelIdxListy, PixelIdxListz);

    cout << "Complete linking the components" << endl;


	int dist_thrs = 50;




	for(int l = 0; l < frag_list_size; l++)
	{
		int ls = frag_list[l];

		int dist_frag2base_curr = dist_frag2base[ls];

		if(dist_frag2base_curr < dist_thrs)
		{
			int fp[3];

			int bp[3];

			for(int i = 0; i < 3; i++ )
			{
				fp[i] = frag_p1.at<int>(i,ls);

				bp[i] = base_p1.at<int>(i,ls);

			}

			// determine the searching bounding box, search the space which
			// is within [+- 5 5 2 closed point pair]


			int path_space[3][2];

			int path_space_range[3] = {5,5,2};

			int sub_img_sz[3];



			for(int i = 0; i < 3; i++)
			{
				int tmp_p;

				tmp_p =  min(fp[i],bp[i]) - path_space_range[i];

				path_space[i][0] = max(tmp_p,0);

				tmp_p =  max(fp[i],bp[i]) + path_space_range[i];

				path_space[i][1] = min(tmp_p,img_sz[i]);

				sub_img_sz[i] = path_space[i][1] - path_space[i][0] + 1;

			}

			int sub_img_pg = sub_img_sz[0] * sub_img_sz[1] * sub_img_sz[2];

			Mat sub_img(3,sub_img_sz,CV_8UC1,Scalar::all(0));

			Mat sub_label(3,sub_img_sz,CV_32S,Scalar::all(0));


			for(int i = 0; i < sub_img_sz[0]; i ++)
			{
				for(int j = 0; j < sub_img_sz[1]; j ++)
				{
					for(int k = 0; k < sub_img_sz[2]; k ++)
					{

						int v3[3];

						int v3s[3];

						v3[0] = i + path_space[0][0];

						v3[1] = j + path_space[1][0];

						v3[2] = k + path_space[2][0];

						v3s[0] = i;

						v3s[1] = j;

						v3s[2] = k;

						sub_img.at<uchar>(v3s) = image.at<uchar>(v3);

						sub_label.at<int>(v3s) = label_img.at<int>(v3);




					}

				}

			}


			// construct the affinity relation for individual node of the searching space

			// list the known fragment in the searching space

			set<int> f_ps(sub_label.begin<int>(),sub_label.end<int>());

		//	int iter = f_ps.begin;

			std::set<int>::iterator iter;

			vector<int> f_ps_list;

			int nf_psi = 0;

			nf_psi = f_ps.size();

			for(iter = f_ps.begin(); iter != f_ps.end(); ++iter)
			{
				int vl = *iter;

				if(vl > 0)
					f_ps_list.push_back(vl);

			}

			// assign each pixel in the space a value similar to the setting of the Matlab version

			int v_id = f_ps_list.size();

			int *label_img_x = new int[sub_img_pg];

			int *label_img_y = new int[sub_img_pg];

			int *label_img_z = new int[sub_img_pg];

			// initialise the label image x y z

			for(int i = 0; i < sub_img_pg; i ++)
			{
				label_img_x[i] = 0;

				label_img_y[i] = 0;

				label_img_z[i] = 0;

			}

			// now the label_img starts from 0;

			for(int i = 0; i < sub_img_sz[0]; i ++)
			{
				for(int j = 0; j < sub_img_sz[1]; j ++)
				{
					for(int k = 0; k < sub_img_sz[2]; k ++)
					{

						int v3s[3];

						v3s[0] = i;

						v3s[1] = j;

						v3s[2] = k;

						int vl = sub_label.at<int>(v3s);

						if(vl == 0)
						{
							sub_label.at<int>(v3s) = v_id;

							label_img_x[v_id] = i + path_space[0][0];

							label_img_y[v_id] = j + path_space[1][0];

							label_img_z[v_id] = k + path_space[2][0];

							v_id ++;

						}
						else
						{
							int n_fps = f_ps_list.size();

							for(int m = 0; m < n_fps; m++)
							{
								if(vl == f_ps_list.at(m))
								{
									sub_label.at<int>(v3s) = m;

									break;

								}

							}

						}

					}




				}

			}

			//  assign the weight and connection for individual pair
			// of pixels

			int n_idx = v_id;

			adjacency_list_t adj_list(n_idx);


			for(int i = 0; i < sub_img_sz[0]; i ++)
			{
				for(int j = 0; j < sub_img_sz[1]; j ++)
				{
					for(int k = 0; k < sub_img_sz[2]; k ++)
					{

						int v3s[3];

						v3s[0] = i;

						v3s[1] = j;

						v3s[2] = k;

						//int vl = sub_label.at<int>(v3s);

					    // build the edge on all six connections

						int v3t[3];




						v3t[0] = i + 1;

						v3t[1] = j;

						v3t[2] = k;

						assign_adj_neigh(sub_img, sub_label, v3s, v3t, adj_list);

						// x -

						v3t[0] = i - 1;

						v3t[1] = j;

						v3t[2] = k;

						assign_adj_neigh(sub_img, sub_label, v3s, v3t, adj_list);


						// y+

						v3t[0] = i;

						v3t[1] = j + 1;

						v3t[2] = k;

						assign_adj_neigh(sub_img, sub_label, v3s, v3t, adj_list);


						// y-

						v3t[0] = i;

						v3t[1] = j - 1;

						v3t[2] = k;

						assign_adj_neigh(sub_img, sub_label, v3s, v3t, adj_list);


						// z +

						v3t[0] = i;

						v3t[1] = j;

						v3t[2] = k + 1;

						assign_adj_neigh(sub_img, sub_label, v3s, v3t, adj_list);


						// z -

						v3t[0] = i;

						v3t[1] = j;

						v3t[2] = k - 1;

						assign_adj_neigh(sub_img, sub_label, v3s, v3t, adj_list);


					}

				}

			}

			cout << "Ready to find the shortest distance" << endl;


			int lt = label_img.at<int>(bp);

			vector<int>::iterator ls_sub_p;

			ls_sub_p = find(f_ps_list.begin(),f_ps_list.end(),(ls + 1));

			vector<int>::iterator lt_sub_p;

			lt_sub_p = find(f_ps_list.begin(),f_ps_list.end(),lt);

			int ls_sub = *ls_sub_p;

			int lt_sub = *lt_sub_p;

		    vector<weight_t> min_distance;

		    vector<vertex_t> previous;

		    cout << "Run the DijkstraComputePaths " << endl;


		    DijkstraComputePaths(ls_sub, lt_sub, adj_list, min_distance, previous);

		    list<vertex_t> path = DijkstraGetShortestPathTo(lt_sub, previous);


		    // collect the path index


		    cout << "start the path " << endl;


		    for(std::list<int>::iterator list_iter = path.begin();
		        list_iter != path.end(); list_iter++)
		    {
		    	int l_sub = *list_iter;

		    	int x = label_img_x[l_sub];

		    	int y = label_img_y[l_sub];

		    	int z = label_img_z[l_sub];

		    	int v3[3];

		    	v3[0] = x - path_space[0][0];

		    	v3[1] = y - path_space[1][0];

		    	v3[2] = z - path_space[2][0];


		    	//cout << "sub_label is " << sub_label.at<int>(v3) << endl;

		    	//cout << "x is " << x << endl;

		    	//cout << "y is " << y << endl;

		    	//cout << "z is " << z << endl;





		    	//cout << "l_sub is " << l_sub << endl;

                //cout << "label is " << label_img.at<int>(v3) << endl;


		    	if(l_sub > (f_ps_list.size() - 1) )
                    Pathxyz[ls].push_back(trip_idx(x,y,z));


		    }

		//    cout << "complete the path " << endl;



		    delete [] label_img_x;

		    delete [] label_img_y;

		    delete [] label_img_z;


		}

	}
	delete [] dist_frag2base;

	return 1;

}

int is_breach(int v3[], int im_sz[])
{
	int isb = 0;

	for(int i = 0; i < 3; i++)
	{
		if(v3[i] < 0)
			isb = 1;

		if(v3[i] > im_sz[i] - 1)
			isb = 1;
	}

	return isb;

}

int assign_adj_neigh(Mat sub_img,Mat sub_label, int v3s[], int v3t[], adjacency_list_t &adj_list)
{
	int img_sz[3];

	for(int i = 0; i < 3; i++)
		img_sz[i] = sub_img.size[i];

	if(!is_breach(v3t,img_sz))
	{
		int lt,ls;

		ls = sub_label.at<int>(v3s);

		lt = sub_label.at<int>(v3t);

		if(abs(ls - lt))
		{
			double wt,ws;

			ws = (double)sub_img.at<uchar>(v3s);

			wt = (double)sub_img.at<uchar>(v3t);

			double wgt = 500 - (ws + wt);

			adj_list[ls].push_back(neighbor(lt, wgt));

		}


	}


	return 1;

}

void DijkstraComputePaths(vertex_t source,
						  vertex_t target,
                          const adjacency_list_t &adjacency_list,
                          std::vector<weight_t> &min_distance,
                          std::vector<vertex_t> &previous)
{
    int n = adjacency_list.size();
    min_distance.clear();
    min_distance.resize(n, max_weight);
    min_distance[source] = 0;
    previous.clear();
    previous.resize(n, -1);
    std::set<std::pair<weight_t, vertex_t> > vertex_queue;
    vertex_queue.insert(std::make_pair(min_distance[source], source));

    int is_target = 0;

    while (!vertex_queue.empty())
    {
        weight_t dist = vertex_queue.begin()->first;
        vertex_t u = vertex_queue.begin()->second;
        vertex_queue.erase(vertex_queue.begin());

        // Visit each edge exiting u
	const std::vector<neighbor> &neighbors = adjacency_list[u];
        for (std::vector<neighbor>::const_iterator neighbor_iter = neighbors.begin();
             neighbor_iter != neighbors.end();
             neighbor_iter++)
        {
            vertex_t v = neighbor_iter->target;
            weight_t weight = neighbor_iter->weight;
            weight_t distance_through_u = dist + weight;
	    if (distance_through_u < min_distance[v]) {
	        vertex_queue.erase(std::make_pair(min_distance[v], v));

	        min_distance[v] = distance_through_u;
	        previous[v] = u;
	        vertex_queue.insert(std::make_pair(min_distance[v], v));

	        if(v == target)
	        	is_target  = 1;

	    }

        }

        if(is_target)
        	break;

    }
}


std::list<vertex_t> DijkstraGetShortestPathTo(
    vertex_t vertex, const std::vector<vertex_t> &previous)
{
    std::list<vertex_t> path;
    for ( ; vertex != -1; vertex = previous[vertex])
        path.push_front(vertex);
    return path;
}


int ReadNumbers( const string & s, vector <double> & v ) {
    istringstream is( s );
    double n;
    while( is >> n ) {
        v.push_back( n );
    }
    return v.size();
}


void import_matrix_from_txt_file(const char* filename_X, vector <double>& v, int& rows, int& cols)
{

    ifstream file_X;
    string line;

    file_X.open(filename_X);
    if (file_X.is_open())
    {
        int i=0;
        getline(file_X, line);


        cols =ReadNumbers( line, v );
        //cout << "cols:" << cols << endl;


        for ( i=1;i<32767;i++){
            if ( getline(file_X, line) == 0 ) break;
            ReadNumbers( line, v );

        }

        rows=i;
        //cout << "rows :" << rows << endl;
        if(rows >32766) cout<< "N must be smaller than MAX_INT";

      //  cout << v.size() <<endl;

        file_X.close();
    }
    else{
        cout << "file open failed";
    }

   // cout << "v:" << endl;

}




int LCM_classify(Mat feature_cc, double * resp_tst, char * dataset)
{


	int n_samp = feature_cc.rows;

	vector<int> inter_m;


	// travese through the LCM model to classify the labels

	// starts from the root node

	vector <double> v;

	int rows=0;

	int cols=0;

	// commence by reading the root node

   // char dataset[] = "OPFFM";

    char fn[300];

    sprintf(fn,"/home/gulin/workspace/simple_RF/LCM_trn_data/%sLCM_trn_root.txt",dataset);



    import_matrix_from_txt_file(fn,v,rows,cols);

    int root_node = (int)v.at(0);

    v.clear();

    cout << "The root node is " << root_node << endl;


    // initialise the inter_m with the root node

    for(int i = 0; i < n_samp;i++)
    	inter_m.push_back(root_node);



    list<int> nlist;

    nlist.push_back(root_node);

    int n_stack = nlist.size();




    while(n_stack > 0)
    {

    	int node = nlist.front();

    	nlist.pop_front();

    	cout << "Start processing the node " << node << endl;


    	vector <double> child_l;

    	sprintf(fn,"/home/gulin/workspace/simple_RF/LCM_trn_data/%sLCM_trn_child_%d.txt",dataset,node);

        import_matrix_from_txt_file(fn,child_l,rows,cols);

    	char rf_name[300];

        sprintf(rf_name,"/home/gulin/workspace/simple_RF/LCM_model/%sLCM_classifier_%d.xml",dataset,node);

        Ptr<RTrees> rtrees;

        rtrees = StatModel::load<RTrees>(rf_name);


        if(child_l[0] > 0)
        {
        	int n_child = child_l.size();


        	for(int i = 0; i < n_samp; i++)
        	{

        		if(inter_m[i] == node)
        		{

        			float resp_elmt = rtrees->predict(feature_cc.row(i));

        			int child_assign = (int) resp_elmt;

        			inter_m[i] = child_l[child_assign - 1];

        		}

        	}

        	for(int i = 0; i < n_child; i++)
        		nlist.push_back(child_l[i]);

        }else
        {
        	for(int i = 0; i < n_samp; i++)
        	{

        		if(inter_m[i] == node)
        		{

        			float resp_elmt = rtrees->predict(feature_cc.row(i));

        			resp_tst[i] = (double) resp_elmt;

        		}

        	}

        }

        v.clear();

        n_stack = nlist.size();



    }

    cout << "complete classifying" << endl;


	return 1;


}

int file_column_count ( string filename )

//****************************************************************************80
//
//  Purpose:
//
//    FILE_COLUMN_COUNT counts the columns in the first line of a file.
//
//  Discussion:
//
//    The file is assumed to be a simple text file.
//
//    Most lines of the file are presumed to consist of COLUMN_NUM words,
//    separated by spaces.  There may also be some blank lines, and some
//    comment lines, which have a "#" in column 1.
//
//    The routine tries to find the first non-comment non-blank line and
//    counts the number of words in that line.
//
//    If all lines are blanks or comments, it goes back and tries to analyze
//    a comment line.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string FILENAME, the name of the file.
//
//    Output, int FILE_COLUMN_COUNT, the number of columns assumed
//    to be in the file.
//
{
  int column_num;
  ifstream input;
  bool got_one;
  string text;
//
//  Open the file.
//
  input.open ( filename.c_str ( ) );

  if ( !input )
  {
    column_num = -1;
    cerr << "\n";
    cerr << "FILE_COLUMN_COUNT - Fatal error!\n";
    cerr << "  Could not open the file:\n";
    cerr << "  \"" << filename << "\"\n";
    exit ( 1 );
  }
//
//  Read one line, but skip blank lines and comment lines.
//
  got_one = false;

  for ( ; ; )
  {
    getline ( input, text );

    if ( input.eof ( ) )
    {
      break;
    }

    if ( s_len_trim ( text ) <= 0 )
    {
      continue;
    }

    if ( text[0] == '#' )
    {
      continue;
    }
    got_one = true;
    break;
  }

  if ( !got_one )
  {
    input.close ( );

    input.open ( filename.c_str ( ) );

    for ( ; ; )
    {
      input >> text;

      if ( input.eof ( ) )
      {
        break;
      }

      if ( s_len_trim ( text ) == 0 )
      {
        continue;
      }
      got_one = true;
      break;
    }
  }

  input.close ( );

  if ( !got_one )
  {
    cerr << "\n";
    cerr << "FILE_COLUMN_COUNT - Warning!\n";
    cerr << "  The file does not seem to contain any data.\n";
    return -1;
  }

  column_num = s_word_count ( text );

  return column_num;
}
//****************************************************************************80

int file_row_count ( string input_filename )

//****************************************************************************80
//
//  Purpose:
//
//    FILE_ROW_COUNT counts the number of row records in a file.
//
//  Discussion:
//
//    It does not count lines that are blank, or that begin with a
//    comment symbol '#'.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 February 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string INPUT_FILENAME, the name of the input file.
//
//    Output, int FILE_ROW_COUNT, the number of rows found.
//
{
  //int bad_num;
  int comment_num;
  ifstream input;
//  int i;
  string line;
  int record_num;
  int row_num;

  row_num = 0;
  comment_num = 0;
  record_num = 0;
  //bad_num = 0;

  input.open ( input_filename.c_str ( ) );

  if ( !input )
  {
    cerr << "\n";
    cerr << "FILE_ROW_COUNT - Fatal error!\n";
    cerr << "  Could not open the input file: \"" << input_filename << "\"\n";
    exit ( 1 );
  }

  for ( ; ; )
  {
    getline ( input, line );

    if ( input.eof ( ) )
    {
      break;
    }

    record_num = record_num + 1;

    if ( line[0] == '#' )
    {
      comment_num = comment_num + 1;
      continue;
    }

    if ( s_len_trim ( line ) == 0 )
    {
      comment_num = comment_num + 1;
      continue;
    }

    row_num = row_num + 1;

  }

  input.close ( );

  return row_num;
}
//****************************************************************************80

int i4_min ( int i1, int i2 )

//****************************************************************************80
//
//  Purpose:
//
//    I4_MIN returns the minimum of two I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    13 October 1998
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int I1, I2, two integers to be compared.
//
//    Output, int I4_MIN, the smaller of I1 and I2.
//
{
  int value;

  if ( i1 < i2 )
  {
    value = i1;
  }
  else
  {
    value = i2;
  }
  return value;
}
//****************************************************************************80

int i4block_components ( int l, int m, int n, int *a, int *c )

//****************************************************************************80
//
//  Purpose:
//
//    I4BLOCK_COMPONENTS assigns contiguous nonzero pixels to a common component.
//
//  Discussion:
//
//    On input, the A array contains values of 0 or 1.
//
//    The 0 pixels are to be ignored.  The 1 pixels are to be grouped
//    into connected components.
//
//    The pixel A(I,J,K) is "connected" to the pixels:
//
//      A(I-1,J,  K  ),  A(I+1,J,  K  ),
//      A(I,  J-1,K  ),  A(I,  J+1,K  ),
//      A(I,  J,  K-1),  A(I,  J,  K+1),
//
//    so most pixels have 6 neighbors.
//
//    On output, COMPONENT_NUM reports the number of components of nonzero
//    data, and the array C contains the component assignment for
//    each nonzero pixel, and is 0 for zero pixels.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 February 2012
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int L, M, N, the order of the array.
//
//    Input, int A[L*M*N], the pixel array.
//
//    Output, int C[L*M*N], the component array.
//
//    Output, int I4BLOCK_COMPONENTS, the number of components
//    of nonzero data.
//
{
  int b;
  int c1;
 // int c2;
  int component;
  int component_num;
  int i;
  int j;
  int k;
  int north;
  int *p;
  int *q;
  int up;
  int west;
//
//  Initialization.
//
  for ( k = 0; k < n; k++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( i = 0; i < l; i++ )
      {
        c[i+j*l+k*l*m] = 0;
      }
    }
  }
  component_num = 0;
//
//  P is simply used to store the component labels.  The dimension used
//  here is, of course, usually an absurd overestimate.
//
  p = new int[l*m*n+1];
  for ( i = 0; i <= l * m * n; i++ )
  {
    p[i] = i;
  }
//
//  "Read" the array one pixel at a time.  If a (nonzero) pixel has a north or
//  west neighbor with a label, the current pixel inherits it.
//  In case the labels disagree, we need to adjust the P array so we can
//  later deal with the fact that the two labels need to be merged.
//
  for ( i = 0; i < l; i++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( k = 0; k < n; k++ )
      {
        if ( i == 0 )
        {
          north = 0;
        }
        else
        {
          north = c[i-1+j*l+k*l*m];
        }

        if ( j == 0 )
        {
          west = 0;
        }
        else
        {
          west = c[i+(j-1)*l+k*l*m];
        }

        if ( k == 0 )
        {
          up = 0;
        }
        else
        {
          up = c[i+j*l+(k-1)*l*m];
        }

        if ( a[i+j*l+k*l*m] != 0 )
        {
//
//  New component?
//
          if ( north == 0 && west == 0 && up == 0 )
          {
            component_num = component_num + 1;
            c[i+j*l+k*l*m] = component_num;
          }
//
//  One predecessor is labeled.
//
          else if ( north != 0 && west == 0 && up == 0 )
          {
            c[i+j*l+k*l*m] = north;
          }
          else if ( north == 0 && west != 0 && up == 0 )
          {
            c[i+j*l+k*l*m] = west;
          }
          else if ( north == 0 && west == 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = up;
          }
//
//  Two predecessors are labeled.
//
          else if ( north == 0 && west != 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( west, up );
            c1 = i4_min ( p[west], p[up] );
            p[west] = c1;
            p[up] = c1;
          }
          else if ( north != 0 && west == 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( north, up );
            c1 = i4_min ( p[north], p[up] );
            p[north] = c1;
            p[up] = c1;
          }
          else if ( north != 0 && west != 0 && up == 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( north, west );
            c1 = i4_min ( p[north], p[west] );
            p[north] = c1;
            p[west] = c1;
          }
//
//  Three predecessors are labeled.
//
          else if ( north != 0 && west != 0 && up != 0 )
          {
            c[i+j*l+k*l*m] = i4_min ( north, i4_min ( west, up ) );
            c1 = i4_min ( p[north], i4_min ( p[west], p[up] ) );
            p[north] = c1;
            p[west] = c1;
            p[up] = c1;
          }
        }
      }
    }
  }
//
//  When a component has multiple labels, have the higher labels
//  point to the lowest one.
//
  for ( component = component_num; 1 <= component; component-- )
  {
    b = component;
    while ( p[b] != b )
    {
      b = p[b];
    }
    p[component] = b;
  }
//
//  Locate the minimum label for each component.
//  Assign these mininum labels new consecutive indices.
//
  q = new int[component_num+1];

  for ( j = 0; j <= component_num; j++ )
  {
    q[j] = 0;
  }

  i = 0;
  for ( component = 1; component <= component_num; component++ )
  {
    if ( p[component] == component )
    {
      i = i + 1;
      q[component] = i;
    }
  }

  component_num = i;
//
//  Replace the labels by consecutive labels.
//
  for ( i = 0; i < l; i++ )
  {
    for ( j = 0; j < m; j++ )
    {
      for ( k = 0; k < n; k++ )
      {
        c[i+j*l+k*l*m] = q [ p [ c[i+j*l+k*l*m] ] ];
      }
    }
  }

  delete [] p;
  delete [] q;

  return component_num;
}
//****************************************************************************80

int i4mat_components ( int m, int n, int a[], int c[] )

//****************************************************************************80
//
//  Purpose:
//
//    I4MAT_COMPONENTS assigns contiguous nonzero pixels to a common component.
//
//  Discussion:
//
//    On input, the A array contains values of 0 or 1.
//
//    The 0 pixels are to be ignored.  The 1 pixels are to be grouped
//    into connected components.
//
//    The pixel A(I,J) is "connected" to the pixels A(I-1,J), A(I+1,J),
//    A(I,J-1) and A(I,J+1), so most pixels have 4 neighbors.
//
//    (Another choice would be to assume that a pixel was connected
//    to the other 8 pixels in the 3x3 block containing it.)
//
//    On output, COMPONENT_NUM reports the number of components of nonzero
//    data, and the array C contains the component assignment for
//    each nonzero pixel, and is 0 for zero pixels.
//
//  Picture:
//
//    Input A:
//
//      0  2  0  0 17  0  3
//      0  0  3  0  1  0  4
//      1  0  4  8  8  0  7
//      3  0  6 45  0  0  0
//      3 17  0  5  9  2  5
//
//    Output:
//
//      COMPONENT_NUM = 4
//
//      C:
//
//      0  1  0  0  2  0  3
//      0  0  2  0  2  0  3
//      4  0  2  2  2  0  3
//      4  0  2  2  0  0  0
//      4  4  0  2  2  2  2
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 March 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, N, the order of the array.
//
//    Input, int A[M*N], the pixel array.
//
//    Output, int C[M*N], the component array.
//
//    Output, int I4MAT_COMPONENTS, the number of components
//    of nonzero data.
//
{
  int b;
  int component;
  int component_num;
  int i;
  int j;
  int north;
  int *p;
  int *q;
  int west;
//
//  Initialization.
//
  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      c[i+j*m] = 0;
    }
  }
  component_num = 0;
//
//  P is simply used to store the component labels.  The dimension used
//  here is, of course, usually an absurd overestimate.
//
  p = new int[m*n+1];

  for ( i = 0; i <= m * n; i++ )
  {
    p[i] = i;
  }
//
//  "Read" the array one pixel at a time.  If a (nonzero) pixel has a north or
//  west neighbor with a label, the current pixel inherits it.
//  In case the labels disagree, we need to adjust the P array so we can
//  later deal with the fact that the two labels need to be merged.
//
  for ( i = 0; i < m; i++ )
  {
    for ( j = 0; j < n; j++ )
    {
      if ( i == 0 )
      {
        north = 0;
      }
      else
      {
        north = c[i-1+j*m];
      }

      if ( j == 0 )
      {
        west = 0;
      }
      else
      {
        west = c[i+(j-1)*m];
      }
      if ( a[i+j*m] != 0 )
      {
        if ( north == 0 )
        {
          if ( west == 0 )
          {
            component_num = component_num + 1;
            c[i+j*m] = component_num;
          }
          else
          {
            c[i+j*m] = west;
          }
        }
        else if ( north != 0 )
        {
          if ( west == 0 || west == north )
          {
            c[i+j*m] = north;
          }
          else
          {
            c[i+j*m] = i4_min ( north, west );
            if ( north < west )
            {
              p[west] = north;
            }
            else
            {
              p[north] = west;
            }
          }
        }
      }
    }
  }
//
//  When a component has multiple labels, have the higher labels
//  point to the lowest one.
//
  for ( component = component_num; 1 <= component; component-- )
  {
    b = component;
    while ( p[b] != b )
    {
      b = p[b];
    }
    p[component] = b;
  }
//
//  Locate the minimum label for each component.
//  Assign these mininum labels new consecutive indices.
//
  q = new int[component_num+1];

  for ( j = 0; j <= component_num; j++ )
  {
    q[j] = 0;
  }

  i = 0;
  for ( component = 1; component <= component_num; component++ )
  {
    if ( p[component] == component )
    {
      i = i + 1;
      q[component] = i;
    }
  }

  component_num = i;
//
//  Replace the labels by consecutive labels.
//
  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      c[i+j*m] = q [ p [ c[i+j*m] ] ];
    }
  }

  delete [] p;
  delete [] q;

  return component_num;
}
//****************************************************************************80

int *i4mat_data_read ( string input_filename, int m, int n )

//****************************************************************************80
//
//  Purpose:
//
//    I4MAT_DATA_READ reads data from an I4MAT file.
//
//  Discussion:
//
//    An I4MAT is an array of I4's.
//
//    The file is assumed to contain one record per line.
//
//    Records beginning with '#' are comments, and are ignored.
//    Blank lines are also ignored.
//
//    Each line that is not ignored is assumed to contain exactly (or at least)
//    M real numbers, representing the coordinates of a point.
//
//    There are assumed to be exactly (or at least) N such records.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 February 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string INPUT_FILENAME, the name of the input file.
//
//    Input, int M, the number of spatial dimensions.
//
//    Input, int N, the number of points.  The program
//    will stop reading data once N values have been read.
//
//    Output, int I4MAT_DATA_READ[M*N], the data.
//
{
  bool error;
  ifstream input;
  int i;
  int j;
  string line;
  int *table;
  int *x;

  input.open ( input_filename.c_str ( ) );

  if ( !input )
  {
    cerr << "\n";
    cerr << "I4MAT_DATA_READ - Fatal error!\n";
    cerr << "  Could not open the input file: \"" << input_filename << "\"\n";
    exit ( 1 );
  }

  table = new int[m*n];

  x = new int[m];

  j = 0;

  while ( j < n )
  {
    getline ( input, line );

    if ( input.eof ( ) )
    {
      break;
    }

    if ( line[0] == '#' || s_len_trim ( line ) == 0 )
    {
      continue;
    }

    error = s_to_i4vec ( line, m, x );

    if ( error )
    {
      continue;
    }

    for ( i = 0; i < m; i++ )
    {
      table[i+j*m] = x[i];
    }
    j = j + 1;

  }

  input.close ( );

  delete [] x;

  return table;
}
//****************************************************************************80

void i4mat_header_read ( string input_filename, int *m, int *n )

//****************************************************************************80
//
//  Purpose:
//
//    I4MAT_HEADER_READ reads the header from an I4MAT file.
//
//  Discussion:
//
//    An I4MAT is an array of I4's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    23 February 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string INPUT_FILENAME, the name of the input file.
//
//    Output, int *M, the number of spatial dimensions.
//
//    Output, int *N, the number of points
//
{
  *m = file_column_count ( input_filename );

  if ( *m <= 0 )
  {
    cerr << "\n";
    cerr << "I4MAT_HEADER_READ - Fatal error!\n";
    cerr << "  FILE_COLUMN_COUNT failed.\n";
    exit ( 1 );
  }

  *n = file_row_count ( input_filename );

  if ( *n <= 0 )
  {
    cerr << "\n";
    cerr << "I4MAT_HEADER_READ - Fatal error!\n";
    cerr << "  FILE_ROW_COUNT failed.\n";
    exit ( 1 );
  }

  return;
}
//****************************************************************************80

int i4vec_components ( int n, int a[], int c[] )

//****************************************************************************80
//
//  Purpose:
//
//    I4VEC_COMPONENTS assigns contiguous nonzero pixels to a common component.
//
//  Discussion:
//
//    This calculation is trivial compared to the 2D problem, and is included
//    primarily for comparison.
//
//    On input, the A array contains values of 0 or 1.
//
//    The 0 pixels are to be ignored.  The 1 pixels are to be grouped
//    into connected components.
//
//    The pixel A(I) is "connected" to the pixels A(I-1) and A(I+1).
//
//    On output, COMPONENT_NUM reports the number of components of nonzero
//    data, and the array C contains the component assignment for
//    each nonzero pixel, and is 0 for zero pixels.
//
//  Picture:
//
//    Input A:
//
//      0 0 1 2 4 0 0 4 0 0 0 8 9 9 1 2 3 0 0 5 0 1 6 0 0 0 4 0
//
//    Output:
//
//      COMPONENT_NUM = 6
//
//      C:
//
//      0 0 1 1 1 0 0 2 0 0 0 3 3 3 3 3 3 0 0 4 0 5 5 0 0 0 6 0
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 March 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the order of the vector.
//
//    Input, int A(N), the pixel array.
//
//    Output, int C[N], the component array.
//
//    Output, int I4VEC_COMPONENTS, the number of components
//    of nonzero data.
//
{
  int component_num;
  int j;
  int west;
//
//  Initialization.
//
  for ( j = 0; j < n; j++ )
  {
    c[j] = 0;
  }
  component_num = 0;
//
//  "Read" the array one pixel at a time.  If a (nonzero) pixel has a west
//  neighbor with a label, the current pixel inherits it.  Otherwise, we have
//  begun a new component.
//
  west = 0;

  for ( j = 0; j < n; j++ )
  {
    if ( a[j] != 0 )
    {
      if ( west == 0 )
      {
        component_num = component_num + 1;
      }
      c[j] = component_num;
    }
    west = c[j];
  }

  return component_num;
}
//****************************************************************************80

int s_len_trim ( string s )

//****************************************************************************80
//
//  Purpose:
//
//    S_LEN_TRIM returns the length of a string to the last nonblank.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, a string.
//
//    Output, int S_LEN_TRIM, the length of the string to the last nonblank.
//    If S_LEN_TRIM is 0, then the string is entirely blank.
//
{
  int n;

  n = s.length ( );

  while ( 0 < n )
  {
    if ( s[n-1] != ' ' )
    {
      return n;
    }
    n = n - 1;
  }

  return n;
}
//****************************************************************************80

int s_to_i4 ( string s, int *last, bool *error )

//****************************************************************************80
//
//  Purpose:
//
//    S_TO_I4 reads an I4 from a string.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, a string to be examined.
//
//    Output, int *LAST, the last character of S used to make IVAL.
//
//    Output, bool *ERROR is TRUE if an error occurred.
//
//    Output, int *S_TO_I4, the integer value read from the string.
//    If the string is blank, then IVAL will be returned 0.
//
{
  char c;
  int i;
  int isgn;
  int istate;
  int ival;

  *error = false;
  istate = 0;
  isgn = 1;
  i = 0;
  ival = 0;

  for ( ; ; )
  {
    c = s[i];
    i = i + 1;
//
//  Haven't read anything.
//
    if ( istate == 0 )
    {
      if ( c == ' ' )
      {
      }
      else if ( c == '-' )
      {
        istate = 1;
        isgn = -1;
      }
      else if ( c == '+' )
      {
        istate = 1;
        isgn = + 1;
      }
      else if ( '0' <= c && c <= '9' )
      {
        istate = 2;
        ival = c - '0';
      }
      else
      {
        *error = true;
        return ival;
      }
    }
//
//  Have read the sign, expecting digits.
//
    else if ( istate == 1 )
    {
      if ( c == ' ' )
      {
      }
      else if ( '0' <= c && c <= '9' )
      {
        istate = 2;
        ival = c - '0';
      }
      else
      {
        *error = true;
        return ival;
      }
    }
//
//  Have read at least one digit, expecting more.
//
    else if ( istate == 2 )
    {
      if ( '0' <= c && c <= '9' )
      {
        ival = 10 * (ival) + c - '0';
      }
      else
      {
        ival = isgn * ival;
        *last = i - 1;
        return ival;
      }

    }
  }
//
//  If we read all the characters in the string, see if we're OK.
//
  if ( istate == 2 )
  {
    ival = isgn * ival;
    *last = s_len_trim ( s );
  }
  else
  {
    *error = true;
    *last = 0;
  }

  return ival;
}
//****************************************************************************80

bool s_to_i4vec ( string s, int n, int ivec[] )

//****************************************************************************80
//
//  Purpose:
//
//    S_TO_I4VEC reads an I4VEC from a string.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, the string to be read.
//
//    Input, int N, the number of values expected.
//
//    Output, int IVEC[N], the values read from the string.
//
//    Output, bool S_TO_I4VEC, is TRUE if an error occurred.
//
{
  int begin;
  bool error;
  int i;
  int lchar;
  int length;

  begin = 0;
  length = s.length ( );
  error = 0;

  for ( i = 0; i < n; i++ )
  {
    ivec[i] = s_to_i4 ( s.substr(begin,length), &lchar, &error );

    if ( error )
    {
      return error;
    }
    begin = begin + lchar;
    length = length - lchar;
  }

  return error;
}
//****************************************************************************80

int s_word_count ( string s )

//****************************************************************************80
//
//  Purpose:
//
//    S_WORD_COUNT counts the number of "words" in a string.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    05 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, string S, the string to be examined.
//
//    Output, int S_WORD_COUNT, the number of "words" in the string.
//    Words are presumed to be separated by one or more blanks.
//
{
  bool blank;
  int char_count;
  int i;
  int word_count;

  word_count = 0;
  blank = true;

  char_count = s.length ( );

  for ( i = 0; i < char_count; i++ )
  {
    if ( isspace ( s[i] ) )
    {
      blank = true;
    }
    else if ( blank )
    {
      word_count = word_count + 1;
      blank = false;
    }
  }

  return word_count;
}
//****************************************************************************80

void timestamp ( )

//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    31 May 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    08 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
{

/*
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct std::tm *tm_ptr;
  //size_t len;
  std::time_t now;

  now = std::time ( NULL );
  tm_ptr = std::localtime ( &now );

  //len = std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );

  std::cout << time_buffer << "\n";

  return;
# undef TIME_SIZE

*/

return;
}


QString getAppPath()
{
	QString v3dAppPath("~/Work/v3d_external/bin");
	QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
	if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
		testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	// In a Mac app bundle, plugins directory could be either
	//  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
	//  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
	if (testPluginsDir.dirName() == "MacOS") {
		QDir testUpperPluginsDir = testPluginsDir;
		testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
		testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
	}
#endif

    //testPluginsDir.cdUp();
	v3dAppPath = testPluginsDir.absolutePath();
	return v3dAppPath;
}

string file_type1(string para)
{
	int pos = para.find_last_of(".");
	if(pos == string::npos) return string("");
	else return para.substr(pos, para.size() - pos);
}

int multiscaleEhance(V3DPluginCallback2 & callback, char *infile, Mat &conf_img)
{


     QString plugin_name = "plugins/multiscaleEnhancement/libmultiscaleEnhancement.so";  //for Linux

     QString full_plugin_name = getAppPath() + "/" + plugin_name;

     QString func_name = "adaptive_auto";

     srand(time(NULL));

     QString tmpfolder =  getAppPath() +("/") + ("tmp_cache");

     system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));

      if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return 0;

    }

    char * outfile = new char [300];

    sprintf(outfile,"tmp_cache/tmp_msenhance.v3draw");

    cout << "input files    " << infile << endl;

    cout << "output files   " << outfile << endl;

    // QString input_image_name = tmpfolder + ;

    V3DPluginArgItem arg;

    V3DPluginArgItem arg1;

    V3DPluginArgList input;

    V3DPluginArgList output;

    V3DPluginArgList input1;


    std::vector<char*> infiles;

    infiles.push_back(infile);

    arg.p =  &infiles;

    input << arg;


    std::vector<char*> outfiles;

    outfiles.push_back(outfile);

    arg1.p =  & outfiles;

    output<< arg1;

     if (! callback.callPluginFunc(full_plugin_name, func_name, input, output) )
	{

        v3d_msg("Fail to call the multiscale enhancement");

		return 0;

    }else
    {

      cout << " Successfully call the multiscale enhancement" << endl;

    }


    loadMat(conf_img, outfile);

    cout << "Complete loading the image" << endl;

    return 1;

   // Multiscale enhancement


}

/*

int multiscaleEhance1(V3DPluginCallback2 & callback, char *infile, uchar * &conf_img1)
{


     QString plugin_name = "plugins/multiscaleEnhancement/libmultiscaleEnhancement.so";  //for Linux

     QString full_plugin_name = getAppPath() + "/" + plugin_name;

     QString func_name = "adaptive_auto";

     srand(time(NULL));

     QString tmpfolder =  getAppPath() +("/") + ("tmp_cache");

     system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));

      if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return 0;

    }

    char * outfile = new char [300];

    sprintf(outfile,"tmp_cache/tmp_msenhance.v3draw");

    cout << "input files    " << infile << endl;

    cout << "output files   " << outfile << endl;

    // QString input_image_name = tmpfolder + ;

    V3DPluginArgItem arg;

    V3DPluginArgItem arg1;

    V3DPluginArgList input;

    V3DPluginArgList output;

    V3DPluginArgList input1;


    std::vector<char*> infiles;

    infiles.push_back(infile);

    arg.p =  &infiles;

    input << arg;


    std::vector<char*> outfiles;

    outfiles.push_back(outfile);

    arg1.p =  & outfiles;

    output<< arg1;

     if (! callback.callPluginFunc(full_plugin_name, func_name, input, output) )
	{

        v3d_msg("Fail to call the multiscale enhancement");

		return 0;

    }else
    {

      cout << " Successfully call the multiscale enhancement" << endl;

    }

    //loadMat(conf_img, outfile);


    V3DLONG * in_sz = NULL;

    int datatype;

    loadImage(outfile, conf_img1, in_sz, datatype);

    return 1;

}

*/


int loadMat(Mat & image, char * fileName_string)
{

    unsigned char * inimg = NULL;

    V3DLONG * in_sz = NULL;

    int datatype;

    if (!loadImage(fileName_string, inimg, in_sz, datatype))
    {

        cout << "Could not open the vaa3d raw file" << endl;

        return -1;

    }

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = (int)in_sz[i];

   image = Mat(3,sz,CV_8UC1,Scalar::all(0));

   cout << "Ready to write the Opencv file" << endl;


    for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
    {

        //cout << "sz " << in_sz[2] << endl;

    //    cout << "iz is " << iz << endl;

        V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
        for(V3DLONG iy = 0; iy <  in_sz[1]; iy++)
        {

           // cout << "sz 2 " << in_sz[1] << endl;

          //  cout << "iy is " << iy << endl;


            V3DLONG offsetj = iy*in_sz[0];
            for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
            {

                unsigned char PixelValue = inimg[offsetk + offsetj + ix];

                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                //cout << "ix is " << ix << endl;

                //cout << "iy is " << iy << endl;

                //cout << "iz is " << iz << endl;

                image.at<uchar>(v3) = PixelValue;

            }
        }

    }

    delete [] inimg;




    delete [] in_sz;



    return 1;

}


int fastMarch(char * fileName_string, Mat & image)
{

    unsigned char * inimg = NULL;

    V3DLONG * in_sz = NULL;

    int datatype;

    if (!loadImage(fileName_string, inimg, in_sz, datatype))
    {

        cout << "Could not open the vaa3d raw file" << endl;

        return -1;

    }

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = (int)in_sz[i];

    float * cf_img = NULL;

    fastmarching_dt_XY(inimg, cf_img, in_sz[0], in_sz[1], in_sz[2],2, 10);

    float * max_value_p;

    int img_pg = sz[0] * sz[1] * sz[2];

    max_value_p = max_element(cf_img,cf_img + img_pg);

    float max_value = * max_value_p;

    cout << "max value of the fastMarching is " << (double)max_value << endl;

   image = Mat(3,sz,CV_8UC1,Scalar::all(0));

    for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
    {

        V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
        for(V3DLONG iy = 0; iy <  in_sz[1]; iy++)
        {
            V3DLONG offsetj = iy*in_sz[0];
            for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
            {

                float PixelValue = cf_img[offsetk + offsetj + ix];

                PixelValue = PixelValue / max_value * 254;

                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                image.at<uchar>(v3) = (uchar)PixelValue;

            }
        }

    }

    delete [] inimg;

    delete [] in_sz;

    delete [] cf_img;

    return 1;

}


int bwdist(Mat image,float * &dist_img,uchar lb1, float md)
{

    V3DLONG sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = (V3DLONG)image.size[i];

    int img_page = image.size[0] * image.size[1] * image.size[2];

    cout << "img_page is " << img_page << endl;

    uchar * seg_img1 = new uchar[img_page];

    int seg_sm = 0;

    for(V3DLONG iz = 0; iz < sz[2]; iz++)
    {

        V3DLONG offsetk = iz*sz[1]*sz[0];
        for(V3DLONG iy = 0; iy <  sz[1]; iy++)
        {
            V3DLONG offsetj = iy* sz[0];
            for(V3DLONG ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                unsigned char PixelValue = image.at<uchar>(v3);

                if(PixelValue > lb1)
                {

                    seg_img1[offsetk + offsetj + ix] = 0;

                    seg_sm ++;

                }else{

                    seg_img1[offsetk + offsetj + ix] = 1;

                }

            }
        }

    }

    cout << "Sum of the foreground is " << seg_sm << endl;

   fastmarching_dt1(seg_img1,dist_img, (int)sz[0], (int)sz[1], (int)sz[2], md);

   delete [] seg_img1;

}

float bwdist2(Mat image,float * &dist_img, int source, int target_tip[], float md)
{

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = image.size[i];

    int img_page = sz[0] * sz[1] * sz[2];

    uchar * seg_img1 = new uchar[img_page];

    uchar * seg_img2 = new uchar[img_page];

 //   cout << "img_page is " << img_page << endl;

 //   cout << "img_page is " << sz[0] * sz[1] * sz[2] << endl;

    int seg_sm = 0;




    for(int iz = 0; iz < sz[2]; iz++)
    {

        int offsetk = iz*sz[1]*sz[0];
        for(int iy = 0; iy <  sz[1]; iy++)
        {
            int offsetj = iy* sz[0];
            for(int ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                int LabelValue = image.at<int>(v3);

             //   uchar PixelValue = image.at<uchar>(v3);

                if(LabelValue == source)
                {

                    seg_img1[offsetk + offsetj + ix] = 0;

                    seg_img2[offsetk + offsetj + ix] = 0;

                    seg_sm ++;

                }else{

                    seg_img1[offsetk + offsetj + ix] = 1;

                    if(LabelValue > 254)
                        LabelValue = 254;

                    if(LabelValue > 0)
                        seg_img2[offsetk + offsetj + ix] = (uchar)LabelValue;
                        else
                        seg_img2[offsetk + offsetj + ix] = 0;

                }

            }
        }

    }

   cout << "Sum of the foreground is " << seg_sm << endl;

 //  float * dist_img;

//   int tip_seed[3];

   float min_dist;

   cout << "Start the cc fastmarching" << endl;


 //  cout << (int)seg_img1[0] << endl;

 //  cout << (int)seg_img2[0] << endl;

   //cout << "The tolsz is " << sz[0] * sz[1] * sz[2] << endl;

   //cout << seg_img1[sz[0] * sz[1] * sz[2] - 1] << endl;

  // cout << seg_img2[sz[0] * sz[1] * sz[2] - 1] << endl;

   uchar target;

   min_dist = fastmarching_cc(seg_img1,seg_img2,dist_img,sz[0],sz[1],sz[2],target_tip,target,md,3, 0);

   cout << "target is " << (int)target << endl;

   cout << "target is " << image.at<int>(target_tip) << endl;

   cout << "min_dist is " << min_dist << endl;

   ////cin.get();


   delete [] seg_img1;

   delete [] seg_img2;

   return min_dist;

}

float bwdist_img(Mat image, Mat label_image, float * &dist_img, int source, int target, int target_tip[], float md)
{

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = image.size[i];

    int img_page = sz[0] * sz[1] * sz[2];

    uchar * seg_img1 = new uchar[img_page];

    uchar * seg_img2 = new uchar[img_page];

 //   cout << "img_page is " << img_page << endl;

 //   cout << "img_page is " << sz[0] * sz[1] * sz[2] << endl;

    int seg_sm = 0;

    int seg_sm1 = 0;

    for(int iz = 0; iz < sz[2]; iz++)
    {

        int offsetk = iz*sz[1]*sz[0];
        for(int iy = 0; iy <  sz[1]; iy++)
        {
            int offsetj = iy* sz[0];
            for(int ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                int LabelValue = label_image.at<int>(v3);

                uchar PixelValue = image.at<uchar>(v3);

             //   uchar PixelValue = image.at<uchar>(v3);

                if(LabelValue == source)
                {

                    seg_img1[offsetk + offsetj + ix] = 0;

                    seg_img2[offsetk + offsetj + ix] = 0;

                    seg_sm ++;

                }else{

                    seg_img1[offsetk + offsetj + ix] = max(255 - PixelValue,2);

                    if(LabelValue > 254)
                        LabelValue = 254;

                    if(LabelValue == target)
                    {
                        seg_img2[offsetk + offsetj + ix] = (uchar)LabelValue;

                        seg_sm1 ++;


                     }else
                        seg_img2[offsetk + offsetj + ix] = 0;

                }

            }
        }

    }

   cout << "Sum of the foreground is " << seg_sm << endl;

   cout << "Sum of the other component is " << seg_sm1 << endl;

 //  float * dist_img;

//   int tip_seed[3];

   float min_dist;

   cout << "Start the cc fastmarching" << endl;


 //  cout << (int)seg_img1[0] << endl;

 //  cout << (int)seg_img2[0] << endl;

   //cout << "The tolsz is " << sz[0] * sz[1] * sz[2] << endl;

   //cout << seg_img1[sz[0] * sz[1] * sz[2] - 1] << endl;

  // cout << seg_img2[sz[0] * sz[1] * sz[2] - 1] << endl;

   uchar target1;


   min_dist = fastmarching_cc2(seg_img1,seg_img2,dist_img,sz[0],sz[1],sz[2],target_tip,target1,md,3, 0);

   cout << "target is " << (int)target1 << endl;

   cout << "target is " << label_image.at<int>(target_tip) << endl;


   cout << "source is " << source << endl;



   cout << "The minimal distance is " << min_dist << endl;

   if(debug_on1 > 0)
    cin.get();



  // //cin.get();


   delete [] seg_img1;

   delete [] seg_img2;

   return min_dist;

}

float bwdist_img1(Mat image, Mat label_image, float * &dist_img, float * dist_img1, int source, int target, int target_tip[], float md)
{

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = image.size[i];

    int img_page = sz[0] * sz[1] * sz[2];

    uchar * seg_img1 = new uchar[img_page];

    uchar * seg_img2 = new uchar[img_page];

 //   cout << "img_page is " << img_page << endl;

 //   cout << "img_page is " << sz[0] * sz[1] * sz[2] << endl;

    int seg_sm = 0;

    int seg_sm1 = 0;

    for(int iz = 0; iz < sz[2]; iz++)
    {

        int offsetk = iz*sz[1]*sz[0];
        for(int iy = 0; iy <  sz[1]; iy++)
        {
            int offsetj = iy* sz[0];
            for(int ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = ix;

                v3[1] = iy;

                v3[2] = iz;

                int LabelValue = label_image.at<int>(v3);



             //   uchar PixelValue = image.at<uchar>(v3);

                if(LabelValue == source)
                {

                    seg_img1[offsetk + offsetj + ix] = 0;

                    seg_img2[offsetk + offsetj + ix] = 0;

                    seg_sm ++;

                }else{

                    uchar PixelValue = image.at<uchar>(v3);

                    seg_img1[offsetk + offsetj + ix] = max(255 - PixelValue,2);

                 //   if((dist_img1[offsetk + offsetj + ix] < 15) & (PixelValue > 2))
                   //     cout << "The seg_img is " << (int)seg_img1[offsetk + offsetj + ix] << endl;



                    if(LabelValue > 254)
                        LabelValue = 254;

                    if(LabelValue == target)
                    {

                        seg_img2[offsetk + offsetj + ix] = (uchar)LabelValue;


                        seg_sm1 ++;


                     }else
                        seg_img2[offsetk + offsetj + ix] = 0;

                }

            }
        }

    }



   cout << "Sum of the foreground is " << seg_sm << endl;

   cout << "Sum of the other component is " << seg_sm1 << endl;

   cin.get();

 //  float * dist_img;

//   int tip_seed[3];

   float min_dist;

   cout << "Start the cc fastmarching" << endl;


 //  cout << (int)seg_img1[0] << endl;

 //  cout << (int)seg_img2[0] << endl;

   //cout << "The tolsz is " << sz[0] * sz[1] * sz[2] << endl;

   //cout << seg_img1[sz[0] * sz[1] * sz[2] - 1] << endl;

  // cout << seg_img2[sz[0] * sz[1] * sz[2] - 1] << endl;

   uchar target1;


   min_dist = fastmarching_cc(seg_img1,seg_img2,dist_img,sz[0],sz[1],sz[2],target_tip,target1,md,3, 0);

   int offsetk = target_tip[2]*sz[1]*sz[0];

   int offsetj = target_tip[1]* sz[0];

   cout << "The distance of tip is " << (double)dist_img1[offsetk + offsetj + target_tip[0]] << endl;


   cout << "target is " << (int)target1 << endl;

   cout << "target is " << label_image.at<int>(target_tip) << endl;


   cout << "source is " << source << endl;



   cout << "The minimal distance is " << min_dist << endl;

   if(debug_on1 > 0)
    cin.get();



  // //cin.get();


   delete [] seg_img1;

   delete [] seg_img2;

   return min_dist;

}



int bwdist1(Mat image,float * &dist_img, int source, float md)
{

    int sz[3];

    for(int i = 0; i < 3; i++)
        sz[i] = image.size[i];

    int img_page = image.size[0] * image.size[1] * image.size[2];

    uchar * seg_img1 = new uchar[img_page];

    int seg_sm = 0;

    for(V3DLONG iz = 0; iz < sz[2]; iz++)
    {

        V3DLONG offsetk = iz*sz[1]*sz[0];
        for(V3DLONG iy = 0; iy <  sz[1]; iy++)
        {
            V3DLONG offsetj = iy* sz[0];
            for(V3DLONG ix = 0; ix < sz[0]; ix++)
            {


                int v3[3];

                v3[0] = (int)ix;

                v3[1] = (int)iy;

                v3[2] = (int)iz;

                int PixelValue = image.at<int>(v3);

                if(PixelValue == source)
                {

                    seg_img1[offsetk + offsetj + ix] = 0;

                    seg_sm ++;

                }else{

                    seg_img1[offsetk + offsetj + ix] = 1;

                }

            }
        }

    }

    cout << "Sum of the foreground is " << seg_sm << endl;

   fastmarching_dt1(seg_img1,dist_img, (int)sz[0], (int)sz[1], (int)sz[2], md);

   delete [] seg_img1;

}




#define INTENSITY_DISTANCE_METHOD 0
#define __USE_APP_METHOD__



//bool topo_segs2swc(vector<HierarchySegment*> & topo_segs, vector<MyMarker*> & outmarkers, int swc_type = 1);

// 1. will change the type of each segment
// swc_type : 0 for length heatmap, 1 for level heatmap
bool topo_segs2swc1(vector<HierarchySegment*> & topo_segs, vector<MyMarker*> & outmarkers, int swc_type = 1)
{
	if(topo_segs.empty()) return false;

	double min_dst = topo_segs[0]->length, max_dst = min_dst;
	double min_level = topo_segs[0]->level, max_level = min_level;
	for(int i = 0; i < topo_segs.size(); i++)
	{
		double dst = topo_segs[i]->length;
		min_dst = MIN(min_dst, dst);
		max_dst = MAX(max_dst, dst);

		int level = topo_segs[i]->level;
		min_level = MIN(min_level, level);
		max_level = MAX(max_level, level);
	}
	max_level = MIN(max_level, 20);                         // todo1

	cout<<"min_dst = "<<min_dst<<endl;
	cout<<"max_dst = "<<max_dst<<endl;
	cout<<"min_level = "<<min_level<<endl;
	cout<<"max_level = "<<max_level<<endl;


	max_dst -= min_dst; if(max_dst == 0.0) max_dst = 0.0000001;
	max_level -= min_level; if(max_level == 0) max_level = 1.0;
	for(int i = 0; i < topo_segs.size(); i++)
	{
		double dst = topo_segs[i]->length;
		int level = MIN(topo_segs[i]->level, max_level);    // todo1
		int color_id = (swc_type == 0) ? (dst - min_dst) / max_dst * 254.0 + 20.5 : (level - min_level)/max_level * 254.0 + 20.5;
		vector<MyMarker*> tmp_markers;
		topo_segs[i]->get_markers(tmp_markers);
		for(int j = 0; j < tmp_markers.size(); j++)
		{
			tmp_markers[j]->type = color_id;
		}
		outmarkers.insert(outmarkers.end(), tmp_markers.begin(), tmp_markers.end());
	}
	return true;
}

// There is no overlap between HierarchySegment
template<class T> bool swc2topo_segs(vector<MyMarker*> & inswc, vector<HierarchySegment*> & topo_segs, int length_method = INTENSITY_DISTANCE_METHOD, T * inimg1d = 0, long sz0 = 0, long sz1 = 0, long sz2 = 0)
{
	if(length_method == INTENSITY_DISTANCE_METHOD && (inimg1d == 0 || sz0 == 0 || sz1 == 0 || sz2 == 0))
	{
		cerr<<"need image input for INTENSITY_DISTANCE_METHOD "<<endl;
		return false;
	}
	// 1. calc distance for every nodes
	int tol_num = inswc.size();
	map<MyMarker*, int> swc_map; for(int i = 0; i < tol_num; i++) swc_map[inswc[i]] = i;

	vector<MyMarker*> leaf_markers;
	//GET_LEAF_MARKERS(leaf_markers, inswc);
	vector<int> childs_num(tol_num);
	{
		for(int i = 0; i < tol_num; i++) childs_num[i]=0;
		for(int m1 = 0; m1 < tol_num; m1++)
		{
			if(!inswc[m1]->parent) continue;
			int m2 = swc_map[inswc[m1]->parent];
			childs_num[m2]++;
		}
		for(int i = 0; i < tol_num; i++) if(childs_num[i] == 0) leaf_markers.push_back(inswc[i]);
	}
	int leaf_num = leaf_markers.size();

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;

	vector<double> topo_dists(tol_num, 0.0);  // furthest leaf distance for each tree node
	vector<MyMarker*> topo_leafs(tol_num, (MyMarker*)0);

	for(int i = 0; i < leaf_num; i++)
	{
		MyMarker * leaf_marker = leaf_markers[i];
		MyMarker * child_node = leaf_markers[i];
		MyMarker * parent_node = child_node->parent;
		int cid = swc_map[child_node];
		topo_leafs[cid] = leaf_marker;
		topo_dists[cid] = (length_method == INTENSITY_DISTANCE_METHOD) ? inimg1d[leaf_marker->ind(sz0, sz01)]/255.0 : 0;
		while(parent_node)
		{
			int pid = swc_map[parent_node];
			double tmp_dst = (length_method == INTENSITY_DISTANCE_METHOD) ? (inimg1d[parent_node->ind(sz0, sz01)]/255.0 + topo_dists[cid]) : (dist(*child_node, *parent_node) + topo_dists[cid]);
			if(tmp_dst >= topo_dists[pid])   // >= instead of >
			{
				topo_dists[pid] = tmp_dst;
				topo_leafs[pid] = topo_leafs[cid];
			}
			else break;
			child_node = parent_node;
			cid = pid;
			parent_node = parent_node->parent;
		}
	}
	// 2. create Hierarchy Segments
	topo_segs.resize(leaf_num);
	map<MyMarker *, int> leaf_ind_map;
	for(int i = 0; i < leaf_num; i++)
	{
		topo_segs[i] = new HierarchySegment();
		leaf_ind_map[leaf_markers[i]] = i;
	}

	for(int i = 0; i < leaf_num; i++)
	{
		MyMarker * leaf_marker = leaf_markers[i];
		MyMarker * root_marker = leaf_marker;
		MyMarker * root_parent = root_marker->parent;
		int level = 1;
		while(root_parent && topo_leafs[swc_map[root_parent]] == leaf_marker)
		{
			if(childs_num[swc_map[root_marker]] >= 2) level++;
			root_marker = root_parent;
			root_parent = root_marker->parent;
		}

		double dst = topo_dists[swc_map[root_marker]];

		HierarchySegment * topo_seg = topo_segs[i];
		*topo_seg = HierarchySegment(leaf_marker, root_marker, dst, level);

		if(root_parent == 0) topo_seg->parent = 0;
		else
		{
			MyMarker * leaf_marker2 = topo_leafs[swc_map[root_parent]];
			int leaf_ind2 = leaf_ind_map[leaf_marker2];
			topo_seg->parent = topo_segs[leaf_ind2];
		}
	}

	swc_map.clear();
	leaf_markers.clear();
	leaf_ind_map.clear();
	topo_dists.clear();
	topo_leafs.clear();
	return true;
}





template<class T> bool hierarchy_prune(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc, T * inimg1d, long sz0, long sz1, long sz2, double length_thresh = 10.0)
{
	vector<HierarchySegment*> topo_segs;
	swc2topo_segs(inswc, topo_segs, INTENSITY_DISTANCE_METHOD, inimg1d, sz0, sz1, sz2);
	vector<HierarchySegment*> filter_segs;
//	if(length_thresh <= 0.0)
//	{
//		vector<short int> values;
//		for(int i = 0; i < topo_segs.size(); i++)
//		{
//			values.push_back(topo_segs[i]->length * 1000 + 0.5);
//		}
//		cout<<"segment num = "<<values.size()<<endl;
//		length_thresh = otsu_threshold(values) / 1000.0;
//		cout<<"otsu length = "<<length_thresh<<endl;
//	}
	for(int i = 0; i < topo_segs.size(); i++)
	{
		if(topo_segs[i]->length >= length_thresh) filter_segs.push_back(topo_segs[i]);
		//if(topo_segs[i]->length * topo_segs[i]->level >= length_thresh) filter_segs.push_back(topo_segs[i]);
	}
	topo_segs2swc1(filter_segs, outswc, 0);
	return true;
}

// hierarchy coverage pruning
template<class T> bool happ(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc, T * inimg1d, long sz0, long sz1, long sz2, double bkg_thresh = 10.0, double length_thresh = 2.0, double SR_ratio = 1.0/9.0, bool is_leaf_prune = true, bool is_smooth = true)
{
	double T_max = (1ll << sizeof(T));

	V3DLONG sz01 = sz0 * sz1;
	V3DLONG tol_sz = sz01 * sz2;

	map<MyMarker*, int> child_num;
	getLeaf_markers(inswc, child_num);

	vector<HierarchySegment*> topo_segs;
	cout<<"Construct hierarchical segments"<<endl;
	swc2topo_segs(inswc, topo_segs, INTENSITY_DISTANCE_METHOD, inimg1d, sz0, sz1, sz2);
	vector<HierarchySegment*> filter_segs;
	for(int i = 0; i < topo_segs.size(); i++)
	{
		if(topo_segs[i]->length >= length_thresh) filter_segs.push_back(topo_segs[i]);
	}
	cout<<"pruned by length_thresh (segment number) : "<<topo_segs.size() <<" - "<<topo_segs.size() - filter_segs.size()<<" = "<<filter_segs.size()<<endl;
	multimap<double, HierarchySegment*> seg_dist_map;
	for(int i = 0; i < filter_segs.size(); i++)
	{
		double dst = filter_segs[i]->length;
		seg_dist_map.insert(pair<double, HierarchySegment*> (dst, filter_segs[i]));
	}

	if(1) // dark nodes pruning
	{
		int dark_num_pruned = 1;
		int iteration = 1;
		vector<bool> is_pruneable(filter_segs.size(), 0);
		cout<<"===== Perform dark node pruning ====="<<endl;
		while(dark_num_pruned > 0)
		{
			dark_num_pruned = 0;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				if(iteration > 1 && !is_pruneable[i]) continue;
				HierarchySegment * seg = filter_segs[i];
				MyMarker * leaf_marker = seg->leaf_marker;
				MyMarker * root_marker = seg->root_marker;
				if(leaf_marker == root_marker) continue;
				if(inimg1d[leaf_marker->ind(sz0, sz01)] <= bkg_thresh)
				{
					seg->leaf_marker = leaf_marker->parent;
					dark_num_pruned ++;
					is_pruneable[i] = true;
				}
				else is_pruneable[i] = false;
			}
			cout<<"\t iteration ["<<iteration++<<"] "<<dark_num_pruned<<" dark node pruned"<<endl;
		}
	}

	if(1) // dark segment pruning
	{
		set<int> delete_index_set;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			if(leaf_marker == root_marker) {delete_index_set.insert(i); continue;}
			MyMarker * p = leaf_marker;
			double sum_int = 0.0, tol_num = 0.0, dark_num = 0.0;
			while(true)
			{
				double intensity = inimg1d[p->ind(sz0, sz01)];
				sum_int += intensity;
				tol_num++;
				if(intensity <= bkg_thresh) dark_num++;
				if(p == root_marker) break;
				p = p->parent;
			}
            if(sum_int/tol_num <= bkg_thresh ||
               dark_num/tol_num >= 0.2)
                delete_index_set.insert(i);
		}
		vector<HierarchySegment*> tmp_segs;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			if(delete_index_set.find(i) == delete_index_set.end()) tmp_segs.push_back(seg);
		}
		cout<<"\t"<<delete_index_set.size()<<" dark segments are deleted"<<endl;
		filter_segs = tmp_segs;
	}

	// calculate radius for every node
	{
		cout<<"Calculating radius for every node"<<endl;
		V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			MyMarker * p = leaf_marker;
			while(true)
			{
                double real_thres = 40; if (real_thres<bkg_thresh) real_thres = bkg_thresh; //by PHC 20121012

				p->radius = markerRadiusXY(inimg1d, in_sz, *p, real_thres);
				if(p == root_marker) break;
				p = p->parent;
			}
		}
	}

#ifdef __USE_APP_METHOD__
	if(1) // hierarchy coverage order pruning
#else
	if(1) // hierarchy coverage order pruning
#endif
	{
		cout<<"Perform hierarchical pruning"<<endl;
		T * tmpimg1d = new T[tol_sz]; memcpy(tmpimg1d, inimg1d, tol_sz * sizeof(T));
		V3DLONG tmp_sz[4] = {sz0, sz1, sz2, 1};

		multimap<double, HierarchySegment*>::reverse_iterator it = seg_dist_map.rbegin();
		//MyMarker * soma = (*it).second->root_marker;  // 2012/07 Hang, no need to consider soma
		//cout<<"soma ("<<soma->x<<","<<soma->y<<","<<soma->z<<") radius = "<<soma->radius<<" value = "<<(int)inimg1d[soma->ind(sz0, sz01)]<<endl;
		filter_segs.clear();
		set<HierarchySegment*> visited_segs;
		double tol_sum_sig = 0.0, tol_sum_rdc = 0.0;
		while(it != seg_dist_map.rend())
		{
			HierarchySegment * seg = it->second;
			if(seg->parent && visited_segs.find(seg->parent) == visited_segs.end()){it++; continue;}

			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			double SR_RATIO = SR_ratio;     // the soma area will use different SR_ratio
			//if(dist(*soma, *root_marker) <= soma->radius) SR_RATIO = 1.0;

			double sum_sig = 0;
			double sum_rdc = 0;

			if(1)
			{
				MyMarker * p = leaf_marker;
				while(true)
				{
					if(tmpimg1d[p->ind(sz0, sz01)] == 0){sum_rdc += inimg1d[p->ind(sz0, sz01)];}
					else
					{
						if(0) sum_sig += inimg1d[p->ind(sz0, sz01)]; // simple stragety
						if(1)// if sphere overlap
						{
							int r = p->radius;
							V3DLONG x1 = p->x + 0.5;
							V3DLONG y1 = p->y + 0.5;
							V3DLONG z1 = p->z + 0.5;
							double sum_sphere_size = 0.0;
							double sum_delete_size = 0.0;
							for(V3DLONG kk = -r; kk <= r; kk++)
							{
								V3DLONG z2 = z1 + kk;
								if(z2 < 0 || z2 >= sz2) continue;
								for(V3DLONG jj = -r; jj <= r; jj++)
								{
									V3DLONG y2 = y1 + jj;
									if(y2 < 0 || y2 >= sz1) continue;
									for(V3DLONG ii = -r; ii <= r; ii++)
									{
										V3DLONG x2 = x1 + ii;
										if(x2 < 0 || x2 >= sz0) continue;
										if(kk*kk + jj*jj + ii*ii > r*r) continue;
										V3DLONG ind2 = z2 * sz01 + y2 * sz0 + x2;
										sum_sphere_size++;
										if(tmpimg1d[ind2] != inimg1d[ind2]){sum_delete_size ++;}
									}
								}
							}
							// the intersection between two sphere with equal size and distance = R is 5/16 (0.3125)
							// sum_delete_size/sum_sphere_size should be < 5/16 for outsize points
							if(sum_sphere_size > 0 && sum_delete_size/sum_sphere_size > 0.1)
							{
								sum_rdc += inimg1d[p->ind(sz0, sz01)];
							}
							else sum_sig += inimg1d[p->ind(sz0, sz01)];
						}
					}
					if(p == root_marker) break;
					p = p->parent;
				}
			}

			//double sum_sig = total_sum_int - sum_rdc;
			if(!seg->parent || sum_rdc == 0.0 || (sum_sig/sum_rdc >= SR_RATIO && sum_sig >= 1.0 * T_max))
			{
				tol_sum_sig += sum_sig;
				tol_sum_rdc += sum_rdc;

				vector<MyMarker*> seg_markers;
				MyMarker * p = leaf_marker;
				while(true){if(tmpimg1d[p->ind(sz0, sz01)] != 0) seg_markers.push_back(p); if(p == root_marker) break; p = p->parent;}
				//reverse(seg_markers.begin(), seg_markers.end()); // need to reverse if resampling

				for(int m = 0; m < seg_markers.size(); m++)
				{
					p = seg_markers[m];

					int r = p->radius;
					if(r > 0)// && tmpimg1d[p->ind(sz0, sz01)] != 0)
					{
						double rr = r * r;
						V3DLONG x = p->x + 0.5;
						V3DLONG y = p->y + 0.5;
						V3DLONG z = p->z + 0.5;
						for(V3DLONG kk = -r; kk <= r; kk++)
						{
							V3DLONG z2 = z + kk;
							if(z2 < 0 || z2 >= sz2) continue;
							for(V3DLONG jj = -r; jj <= r; jj++)
							{
								V3DLONG y2 = y + jj;
								if(y2 < 0 || y2 >= sz1) continue;
								for(V3DLONG ii = -r; ii <= r; ii++)
								{
									V3DLONG x2 = x + ii;
									if(x2 < 0 || x2 >= sz0) continue;
									double dst = ii*ii + jj*jj + kk*kk;
									if(dst > rr) continue;
									V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
									tmpimg1d[ind] = 0;
								}
							}
						}
					}
				}

				filter_segs.push_back(seg);
				visited_segs.insert(seg);     // used to delete children when parent node is delete
			}
			it++;
		}
		cout<<"prune by coverage (segment number) : "<<seg_dist_map.size() << " - "<< filter_segs.size() <<" = "<<seg_dist_map.size() - filter_segs.size()<<endl;
		cout<<"R/S ratio = "<<tol_sum_rdc/tol_sum_sig<<" ("<<tol_sum_rdc<<"/"<<tol_sum_sig<<")"<<endl;
		if(1) // evaluation
		{
			double tree_sig = 0.0; for(int m = 0; m < inswc.size(); m++) tree_sig += inimg1d[inswc[m]->ind(sz0, sz01)];
			double covered_sig = 0.0; for(int ind = 0; ind < tol_sz; ind++) if(tmpimg1d[ind] == 0) covered_sig += inimg1d[ind];
			cout<<"S/T ratio = "<<covered_sig/tree_sig<<" ("<<covered_sig<<"/"<<tree_sig<<")"<<endl;
		}
		//saveImage("test.tif", tmpimg1d, tmp_sz, V3D_UINT8);
		if(tmpimg1d){delete [] tmpimg1d; tmpimg1d = 0; }
	}

	if(0) // resampling markers or internal node pruning //this part of code has bug: many fragmentations. noted by PHC, 20120628
	{
		cout<<"resampling markers"<<endl;
		vector<MyMarker*> tmp_markers;
		topo_segs2swc1(filter_segs, tmp_markers, 0); // no resampling
		child_num.clear();
		getLeaf_markers(tmp_markers, child_num);

		// calculate sampling markers
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			vector<MyMarker*> seg_markers;
			MyMarker * p = leaf_marker;
			while(true){seg_markers.push_back(p); if(p == root_marker) break; p = p->parent;}
			//reverse(seg_markers.begin(), seg_markers.end()); // need to reverse if resampling //commened by PHC, 130520 to build on Ubuntu. This should make no difference as the outside code is if (0)
			vector<MyMarker*> sampling_markers; // store resampling markers
			p = root_marker; sampling_markers.push_back(p);
			for(int m = 0; m < seg_markers.size(); m++)
			{
				MyMarker * marker = seg_markers[m];
				if(child_num[marker] > 1 || dist(*marker, *p) >= p->radius)// + marker->radius)
				{
					sampling_markers.push_back(marker);
					p = marker;
				}
			}
			if((*sampling_markers.rbegin()) != leaf_marker) sampling_markers.push_back(leaf_marker);
			for(int m = 1; m < sampling_markers.size(); m++) sampling_markers[m]->parent = sampling_markers[m-1];
		}
	}

#ifdef __USE_APP_METHOD__
	if(1)//is_leaf_prune)  // leaf nodes pruning
#else
	if(0)
#endif
	{
		cout<<"Perform leaf node pruning"<<endl;

		map<MyMarker*,int> tmp_child_num;
		if(1) // get child_num of each node
		{
			vector<MyMarker*> current_markers;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				HierarchySegment * seg = filter_segs[i];
				seg->get_markers(current_markers);
			}
			for(int m = 0; m < current_markers.size(); m++) tmp_child_num[current_markers[m]] = 0;
			for(int m = 0; m < current_markers.size(); m++)
			{
				MyMarker * par_marker = current_markers[m]->parent;
				if(par_marker) tmp_child_num[par_marker]++;
			}

		}
		int leaf_num_pruned = 1;
		int iteration = 1;
		vector<bool> is_pruneable(filter_segs.size(), 0);
		while(leaf_num_pruned > 0)
		{
			leaf_num_pruned = 0;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				if(iteration > 1 && !is_pruneable[i]) continue;
				HierarchySegment * seg = filter_segs[i];
				MyMarker * leaf_marker = seg->leaf_marker;
				MyMarker * root_marker = seg->root_marker;

				if(tmp_child_num[leaf_marker] >= 1) continue;

				assert(leaf_marker);
				MyMarker * par_marker = leaf_marker->parent;
				if(!par_marker)
				{
					is_pruneable[i] = 0;
					continue;
				}
				int r1 = leaf_marker->radius;
				int r2 =  par_marker->radius;
				double r1_r1 = r1 * r1;
				double r2_r2 = r2 * r2;
				V3DLONG x1 = leaf_marker->x + 0.5;
				V3DLONG y1 = leaf_marker->y + 0.5;
				V3DLONG z1 = leaf_marker->z + 0.5;
				V3DLONG x2 =  par_marker->x + 0.5;
				V3DLONG y2 =  par_marker->y + 0.5;
				V3DLONG z2 =  par_marker->z + 0.5;

				double sum_leaf_int  = 0.0;
				double sum_over_int = 0.0;
				for(V3DLONG kk = -r1; kk <= r1; kk++)
				{
					V3DLONG zz = z1 + kk;
					if(zz < 0 || zz >= sz2) continue;
					for(V3DLONG jj = -r1; jj <= r1; jj++)
					{
						V3DLONG yy = y1 + jj;
						if(yy < 0 || yy >= sz1) continue;
						for(V3DLONG ii = -r1; ii <= r1; ii++)
						{
							V3DLONG xx = x1 + ii;
							if(xx < 0 || xx >= sz0) continue;
							double dst = kk * kk + jj * jj + ii * ii;
							if(dst > r1_r1) continue;
							V3DLONG ind = zz * sz01 + yy * sz0 + xx;
							sum_leaf_int += inimg1d[ind];
							if((z2 - zz) * (z2 - zz) + (y2 - yy) * (y2 - yy) + (x2 - xx) * (x2 - xx) <= r2 * r2)
							{
								sum_over_int += inimg1d[ind];
							}
						}
					}
				}
				if(sum_leaf_int > 0 && sum_over_int/sum_leaf_int >= 0.9)
				{
					leaf_num_pruned ++;
					tmp_child_num[par_marker]--;
					assert(tmp_child_num[leaf_marker] == 0);
					if(leaf_marker != root_marker)
					{
						seg->leaf_marker = par_marker;
						is_pruneable[i] = true;
					}
					else
					{
						seg->leaf_marker = NULL;
						seg->root_marker = NULL;
						is_pruneable[i] = false;
					}
				}
				else is_pruneable[i] = false;
			}
			cout<<"\t iteration ["<<iteration++<<"] "<<leaf_num_pruned<<" leaf node pruned"<<endl;
		}
		// filter out segments with single marker
		vector<HierarchySegment*> tmp_segs;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			if(leaf_marker && root_marker) tmp_segs.push_back(seg);
		}
		cout<<"\t"<<filter_segs.size() - tmp_segs.size()<<" hierarchical segments are pruned in leaf node pruning"<<endl;
		filter_segs.clear(); filter_segs = tmp_segs;
	}

#ifdef __USE_APP_METHOD__
	if(1) // joint leaf node pruning
#else
	if(0) // joint leaf node pruning
#endif
	{
		cout<<"Perform joint leaf node pruning"<<endl;
		cout<<"\tcompute mask area"<<endl;
		unsigned short * mask = new unsigned short[tol_sz];
		memset(mask, 0, sizeof(unsigned short) * tol_sz);
		for(int s = 0; s < filter_segs.size(); s++)
		{
			HierarchySegment * seg = filter_segs[s];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			MyMarker * p = leaf_marker;
			while(true)
			{
				int r = p->radius;
				if(r > 0)
				{
					double rr = r * r;
					V3DLONG x = p->x + 0.5;
					V3DLONG y = p->y + 0.5;
					V3DLONG z = p->z + 0.5;
					for(V3DLONG kk = -r; kk <= r; kk++)
					{
						V3DLONG z2 = z + kk;
						if(z2 < 0 || z2 >= sz2) continue;
						for(V3DLONG jj = -r; jj <= r; jj++)
						{
							V3DLONG y2 = y + jj;
							if(y2 < 0 || y2 >= sz1) continue;
							for(V3DLONG ii = -r; ii <= r; ii++)
							{
								V3DLONG x2 = x + ii;
								if(x2 < 0 || x2 >= sz0) continue;
								double dst = ii*ii + jj*jj + kk*kk;
								if(dst > rr) continue;
								V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
								mask[ind]++;
							}
						}
					}
				}
				if(p == root_marker) break;
				p = p->parent;
			}
		}
		cout<<"\tget post_segs"<<endl;
		vector<HierarchySegment*> post_segs;
		if(0) // get post order of filter_segs
		{
			multimap<double, HierarchySegment*> tmp_seg_map;
			for(int s = 0; s < filter_segs.size(); s++)
			{
				double dst = filter_segs[s]->length;
				tmp_seg_map.insert(pair<double, HierarchySegment*>(dst, filter_segs[s]));
			}
			multimap<double, HierarchySegment*>::iterator it = tmp_seg_map.begin();
			while(it != tmp_seg_map.end())
			{
				post_segs.push_back(it->second);
				it++;
			}
		}
		else post_segs = filter_segs; // random order

		map<MyMarker*,int> tmp_child_num;
		if(1) // get child_num of each node
		{
			vector<MyMarker*> current_markers;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				HierarchySegment * seg = filter_segs[i];
				seg->get_markers(current_markers);
			}
			for(int m = 0; m < current_markers.size(); m++) tmp_child_num[current_markers[m]] = 0;
			for(int m = 0; m < current_markers.size(); m++)
			{
				MyMarker * par_marker = current_markers[m]->parent;
				if(par_marker) tmp_child_num[par_marker]++;
			}

		}
		if(1) // start prune leaf nodes
		{
			cout<<"\tleaf node pruning"<<endl;
			int leaf_num_pruned = 1;
			int iteration = 1;
			vector<bool> is_pruneable(post_segs.size(), 0);
			while(leaf_num_pruned > 0)
			{
				leaf_num_pruned = 0;
				for(int i = 0; i < post_segs.size(); i++)
				{
					if(iteration > 1 && !is_pruneable[i]) continue;
					HierarchySegment * seg = post_segs[i];
					MyMarker * leaf_marker = seg->leaf_marker;
					MyMarker * root_marker = seg->root_marker;
					int r = leaf_marker->radius;
					if(r <= 0 )
					{
						is_pruneable[i] = 0;
						continue;
					}
					double rr = r * r;
					V3DLONG x = leaf_marker->x + 0.5;
					V3DLONG y = leaf_marker->y + 0.5;
					V3DLONG z = leaf_marker->z + 0.5;

					double covered_sig = 0; double total_sig = 0.0;
					for(V3DLONG kk = -r; kk <= r; kk++)
					{
						V3DLONG z2 = z + kk;
						if(z2 < 0 || z2 >= sz2) continue;
						for(V3DLONG jj = -r; jj <= r; jj++)
						{
							V3DLONG y2 = y + jj;
							if(y2 < 0 || y2 >= sz1) continue;
							for(V3DLONG ii = -r; ii <= r; ii++)
							{
								V3DLONG x2 = x + ii;
								if(x2 < 0 || x2 >= sz0) continue;
								double dst = ii*ii + jj*jj + kk*kk;
								if(dst > rr) continue;
								V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
								if(mask[ind] > 1) covered_sig += inimg1d[ind];
								total_sig += inimg1d[ind];
							}
						}
					}
					if(covered_sig / total_sig >= 0.9) // 90% joint cover, prune it
					{
						if(tmp_child_num[leaf_marker] == 0) // real leaf node
						{
							leaf_num_pruned++;
							MyMarker * par_marker = leaf_marker->parent;
							if(par_marker) tmp_child_num[par_marker]--;
							if(leaf_marker != root_marker)
							{
								seg->leaf_marker = par_marker;
								is_pruneable[i] = 1; // *** able to prune continuous
							}
							else // if(leaf_marker == root_marker) // unable to prune
							{
								seg->leaf_marker = NULL;
								seg->root_marker = NULL;
								is_pruneable[i] = 0; // *** no marker left, unable to prune again
							}
							// unmask leaf_marker area
							{
								for(V3DLONG kk = -r; kk <= r; kk++)
								{
									V3DLONG z2 = z + kk;
									if(z2 < 0 || z2 >= sz2) continue;
									for(V3DLONG jj = -r; jj <= r; jj++)
									{
										V3DLONG y2 = y + jj;
										if(y2 < 0 || y2 >= sz1) continue;
										for(V3DLONG ii = -r; ii <= r; ii++)
										{
											V3DLONG x2 = x + ii;
											if(x2 < 0 || x2 >= sz0) continue;
											double dst = ii*ii + jj*jj + kk*kk;
											if(dst > rr) continue;
											V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
											if(mask[ind] > 1) mask[ind]--;
										}
									}
								}
							}
						}
						else is_pruneable[i] = 1; // keep it until it is leaf node
					}
					else is_pruneable[i] = 0;
				}
				cout<<"\t iteration ["<<iteration++<<"] "<<leaf_num_pruned<<" leaf node pruned"<<endl;
			}
			// filter out segments with single marker
			vector<HierarchySegment*> tmp_segs;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				HierarchySegment * seg = filter_segs[i];
				MyMarker * leaf_marker = seg->leaf_marker;
				MyMarker * root_marker = seg->root_marker;
				if(leaf_marker && root_marker) tmp_segs.push_back(seg); // filter out empty segments
			}
			cout<<"\t"<<filter_segs.size() - tmp_segs.size()<<" hierarchical segments are pruned in joint leaf node pruning"<<endl;
			filter_segs.clear(); filter_segs = tmp_segs;
		}

		if(mask){delete [] mask; mask = 0;}
	}

	if(is_smooth) // smooth curve
	{
		cout<<"Smooth the final curve"<<endl;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			vector<MyMarker*> seg_markers;
			MyMarker * p = leaf_marker;
			while(p != root_marker)
			{
				seg_markers.push_back(p);
				p = p->parent;
			}
			seg_markers.push_back(root_marker);
			smooth_curve_and_radius(seg_markers, 5);
		}
	}
	outswc.clear();
	cout<<filter_segs.size()<<" segments left"<<endl;
	topo_segs2swc1(filter_segs, outswc, 0); // no resampling

	// release hierarchical segments
	for(int i = 0; i < topo_segs.size(); i++) delete topo_segs[i];
	return true;
}



