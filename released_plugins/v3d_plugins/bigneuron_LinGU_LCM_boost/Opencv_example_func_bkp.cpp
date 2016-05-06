/* Opencv_example_func.cpp
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */

#include "Opencv_example_func.h"



using namespace std;
using namespace cv;
using namespace cv::ml;


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

	int img_sz[3];

	for(int i = 0; i < 3; i++)
        img_sz[i] = (int)in_sz[i];

    Mat label_img = Mat(3,img_sz,CV_32S,Scalar::all(0));


    // process the labelling image

    cout << "Start processing  the image" << endl;

    LCM_boost(data1d, in_sz,label_img);

    cout << "Complete processing  the image" << endl;


    Mat show_img;

    label_img.convertTo(show_img,CV_8UC1);

    // start visualising the image

    v3dhandle newwin = callback.newImageWindow();

    p4DImage->setData(show_img.data, in_sz[0], in_sz[1], in_sz[2], 1, p4DImage->getDatatype());

    callback.setImage(newwin, p4DImage);

    callback.setImageName(newwin, QObject::tr("Boosted Image"));

    callback.updateImageWindow(newwin);


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

bool Opencv_example(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	unsigned char * inimg1d = 0;
	V3DLONG * in_sz = 0;

	if(input.empty()) return false;


    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);

        if (inlist->size() != 1)
        {
                cout<<"You must specify 1 input file!"<<endl;
                return -1;
        }

    char * infile = inlist->at(0);


    Mat image;

    loadMat(image, infile);

    Mat conf_img;

    // now uses the multiscale enhancement

    multiscaleEhance(callback, infile, conf_img);

    double sum_value = (double)std::accumulate(conf_img.begin<uchar>(), conf_img.end<uchar>(), 0.0);

    cout << "sum value is " << sum_value << endl;

    Mat seg_img;

    LCM_boost(conf_img,seg_img);


    /*


    Mat label_img = Mat(3,img_sz,CV_32S,Scalar::all(0));






    // output the result into the harddisk


    vector<char*>* outlist = (vector<char*>*)(output.at(0).p);

        if (outlist->size() != 1)
        {
                cout<<"You must specify 1 output file!"<<endl;
                return -1;
        }

          char * outfile = outlist->at(0);
        cout<<"output file: "<<outfile<<endl;

        V3DLONG sz[4];

        for(int i =0; i < 4; i++)
            sz[i] = in_sz[i];

    Mat show_img;

    label_img.convertTo(show_img,CV_8UC1);

    saveImage(outfile, show_img.data, sz, datatype);

    /*

	if(in_str.size() < 2)
	{
        cerr<<"Error: no enough input."<<endl;

        return false;

	}

	*/





	//string inimg_file, inswc_file;


    //int n_char = in_str.size();

    //std::cout << n_char << endl;


	//string ftp1 = file_type(in_str[0]);

	//std::cout << ftp1 << endl;

    //printf(in_str);

	/*

	string ftp2 = file_type(in_str[1]);
	if(ftp1 == ".raw" || ftp1 == ".tif" || ftp1 == ".tiff" || ftp1 == ".lsm") inimg_file = in_str[0];
	else {if(ftp1 == ".swc") inswc_file = in_str[0];}
	if(ftp2 == ".raw" || ftp2 == ".tif" || ftp2 == ".tiff" || ftp2 == ".lsm") inimg_file = in_str[1];
	else {if(ftp2 == ".swc") inswc_file = in_str[1];}
	if(inimg_file == "" || inswc_file == "") {cerr<<"Error : either image file or swc file is missing."<<endl; return false;}
	int datatype;
	loadImage((char*) inimg_file.c_str(), inimg1d, in_sz, datatype);
	nt = readSWC_file((char*)inswc_file.c_str());



	//V3DLONG in_sz_input[4];

	//for(int id = 0; id < 4; id++)
      //  in_sz_input[id] = in_sz[id];

	LCM_boost(inimg1d,in_sz);


	//READ_IMAGE_AND_SWC_FROM_INPUT(inimg1d, in_sz, nt);

	cout<<"in_sz[0] = "<<in_sz[0]<<endl;
	cout<<"in_sz[1] = "<<in_sz[1]<<endl;
	cout<<"in_sz[2] = "<<in_sz[2]<<endl;
	cout<<"in_sz[3] = "<<in_sz[3]<<endl;
	cout<<"nt.listNeuron.size() = "<<nt.listNeuron.size()<<endl;

	*/
	return true;
}

int LCM_boost(Mat image, Mat & seg_img)
{
       // at first load the image into opencv

    int img_sz[3];

    img_sz[0] = (int)image.size[0];

    img_sz[1] = (int)image.size[1];

    img_sz[2] = (int)image.size[2];

    seg_img = Mat(3,img_sz,CV_8UC1,Scalar::all(0));


    // in the current version, estimate the lower bound of the base method by thresholding

    int img_page = img_sz[0] * img_sz[1] * img_sz[2];

    double sum_value = (double)std::accumulate(imagel.begin<uchar>(), imagel.end<uchar>(), 0.0);

    double mean_value = sum_value / (double) img_page;

    cout << "The mean value is " << mean_value << endl;

    for(int i = 0; i < img_sz[0]; i++)
    {
        for(int j = 0; j < img_sz[1]; j++)
        {
            for(int k = 0; k < img_sz[2]; k++)
            {
                int v3[3];

                v3[0] = i;

                v3[1] = j;

                v3[2] = k;

                double PixelValue = (double)image.at<uchar>(v3);

                seg_img.at<uchar>(v3) = PixelValue > mean_value ? 254 : 0;

            }

        }

    }

      int max_label;

      Mat label_img;

      cout << "Ready to do the bwconnmp on the images" << endl;

	  max_label = bwconnmp_img(seg_img,label_img);

	  cout << "Complete the bwconnmp on the images" << endl;



	  // find how many labels are there in the image

	  vector<vector<int> > PixelIdxListx(max_label);

	  vector<vector<int> > PixelIdxListy(max_label);

	  vector<vector<int> > PixelIdxListz(max_label);


	  //v3d_msg(QString("The maximal label is %1").arg(max_label));


	  bwconnmp(label_img,PixelIdxListx,PixelIdxListy,PixelIdxListz);


	  cout << "start the determine_bounding_box on the images" << endl;



     cout << max_label << endl;
	  // find the bounding box for individual component

	  int *bnd = new int[max_label * 2 * 3];

	  determine_bounding_box(bnd,PixelIdxListx, PixelIdxListy, PixelIdxListz);


      cout << "Complete the determine_bounding_box on the images" << endl;



	  int base_thrs = 1000;

	  int hist_sz[4];

	  hist_sz[0] = 3;

	  hist_sz[1] = 3;

	  hist_sz[2] = 3;

	  hist_sz[3] = max_label;


	  Mat hist1 = Mat(4,hist_sz,CV_32S, Scalar::all(0));

	  cout << "Start HOG" << endl;

	  collect_hog(label_img,hist1,base_thrs,PixelIdxListx, PixelIdxListy, PixelIdxListz);

	  cout << "Finish HOG" << endl;


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

	  local_ftrs(image,centre_cc1, cub1,PixelIdxListx,PixelIdxListy,PixelIdxListz);

        cout << "Finish Local Features" << endl;

	  // calculate the minimal distance to the main base and their features

	  vector <int>frag_list;

	  vector <int>base_list;


	  Mat frag_p1 = Mat(3,max_label,CV_32S,Scalar::all(0));

	  Mat base_p1 = Mat(3,max_label,CV_32S,Scalar::all(0));


	  double *curvature_l = new double[max_label];

	  int *min_dist1 = new int[max_label];

	  cout << "start pre-linking the components" << endl;


	  link_ftrs(min_dist1,curvature_l, base_thrs, bnd, frag_p1,base_p1, PixelIdxListx, PixelIdxListy,PixelIdxListz);

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

	  LCM_classify(feature_cc, resp_tst);

	  cout << "Complete classifying" << endl;

	  //rt_test(feature_cc, resp_tst);

	  int * fila_frag = new int[n_samp];

	  for(int i = 0; i < feature_cc.rows; i++)
		  fila_frag[i] = resp_tst[i] > 0 ? 1 : 0;

	  for(int i = 0; i < feature_cc.rows; i++)
	  {
		  cout << fila_frag[i] << endl;

		  cout << resp_tst[i] << endl;

	  }




	  vector<vector<trip_idx> > Pathxyz(max_label);

	  for(int i = 0; i < feature_cc.rows; i++)
	  {
		  if(feature_cc.at<float>(i,0) >150)
		  fila_frag[i] = 1;

		  cout << i << endl;

		  cout << feature_cc.at<float>(i,0) << endl;

		  cout << fila_frag[i] << endl;

	  }

	  // attempt to connect the individual components

    cout << "start recovering the path" << endl;

	  recover_component(image,label_img,fila_frag,Pathxyz,PixelIdxListx,PixelIdxListy,PixelIdxListz);

        cout << "complete recovering the path" << endl;

	  // add the recovered path into the label image.


	  for(int i = 0; i < max_label; i++)
	  {

		  if(fila_frag[i])
		  {
			  int n_p = Pathxyz[i].size();

			  for(int j = 0; j < n_p; j ++)
			  {
				  int v3[3];

				  v3[0] = Pathxyz[i][j].x;

				  v3[1] = Pathxyz[i][j].y;

				  v3[2] = Pathxyz[i][j].z;

				  label_img.at<int>(v3) = i + max_label + 1;

			  }


		  }

	  }

        // visualise the recovered image

        //v3dhandle newwin = callback.newImageWindow();

        //p4DImage->setData(nm, sz[0], sz[1], sz[2], 1, p4DImage->getDatatype());//setData() will free the original memory automatically







	  delete [] resp_tst;

	  delete [] bnd;

	  delete [] curvature_l;

	  delete [] min_dist1;

	  delete [] fila_frag;

	  return 1;

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

    /*

	for(int k = 0; k < input_img.size[2]; k ++)
	{
		//uchar* rp = label_img.ptr<uchar>(i);

		for(int j = 0; j < input_img.size[1] ; j ++)
		{
			//int offsetj = j * label_img.size[2];

			for(int i = 0; i < input_img.size[0]; i ++)
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

	*/

	int component_num;

	int * c_label = new int[im_sz[0] * im_sz[1] * im_sz[2]];

	component_num = i4block_components(im_sz[0], im_sz[1], im_sz[2], img, c_label);

	delete [] img;

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

          cout << " Collect the local features for CC" << l << endl;

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

	}


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

int recover_component(cv::Mat image, cv::Mat label_img, int * fila_frag, std::vector<std::vector<trip_idx> > Pathxyz,
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

		int n_cc = PixelIdxListx[i].size();

		if(n_cc > base_thrs)
			base_list.push_back(i);
		else
		{
			if(fila_frag[i] >0)
				frag_list.push_back(i);

		}

	}


	Mat frag_p1 = Mat(3,n_label,CV_32S,Scalar::all(0));

	Mat base_p1 = Mat(3,n_label,CV_32S,Scalar::all(0));



	link_nearest(dist_frag2base, frag_list, base_list, frag_p1,base_p1, PixelIdxListx,PixelIdxListy, PixelIdxListz);


	int dist_thrs = 50;

	int frag_list_size = frag_list.size();

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

							label_img_x[v_id] = i;

							label_img_y[v_id] = j;

							label_img_z[v_id] = k;

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


			int lt = label_img.at<int>(bp);

			vector<int>::iterator ls_sub_p;

			ls_sub_p = find(f_ps_list.begin(),f_ps_list.end(),ls);

			vector<int>::iterator lt_sub_p;

			lt_sub_p = find(f_ps_list.begin(),f_ps_list.end(),lt);

			int ls_sub = *ls_sub_p;

			int lt_sub = *lt_sub_p;

		    vector<weight_t> min_distance;

		    vector<vertex_t> previous;

		    DijkstraComputePaths(ls_sub, lt_sub, adj_list, min_distance, previous);

		    list<vertex_t> path = DijkstraGetShortestPathTo(lt_sub, previous);




		    // collect the path index


		    for(std::list<int>::iterator list_iter = path.begin();
		        list_iter != path.end(); list_iter++)
		    {
		    	int l_sub = *list_iter;

		    	int x = label_img_x[l_sub];

		    	int y = label_img_y[l_sub];

		    	int z = label_img_z[l_sub];

		    	Pathxyz[ls].push_back(trip_idx(x,y,z));


		    }

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

			double wgt = ws + wt + 1;

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




int LCM_classify(Mat feature_cc, double * resp_tst)
{


	int n_samp = feature_cc.rows;

	vector<int> inter_m;


	// travese through the LCM model to classify the labels

	// starts from the root node

	vector <double> v;

	int rows=0;

	int cols=0;

	// commence by reading the root node

    import_matrix_from_txt_file("/home/gulin/workspace/BoostedNeuron/LCM_trn_data/LCM_trn_root.txt",v,rows,cols);

    int root_node = (int)v.at(0);

    v.clear();

    // initialise the inter_m with the root node

    for(int i = 0; i < n_samp;i++)
    	inter_m.push_back(root_node);



    list<int> nlist;

    nlist.push_back(root_node);

    int n_stack = nlist.size();

    //label =

//    vector<double> tmp_label();


    while(n_stack > 0)
    {

    	int node = nlist.front();

    	nlist.pop_front();


    	char fn[300];

    	vector <double> child_l;

    	sprintf(fn,"/home/gulin/workspace/BoostedNeuron/LCM_trn_data/LCM_trn_child_%d.txt",node);

        import_matrix_from_txt_file(fn,child_l,rows,cols);

    	char rf_name[300];

        sprintf(rf_name,"/home/gulin/workspace/BoostedNeuron/LCM_model/LCM_classifier_%d.xml",node);

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

        			//cout << " i is " << i << "  " << resp_tst[i] << endl;

        		}

        	}

        }

        v.clear();

        n_stack = nlist.size();



    }

    cout << "complete classifying" << endl;


	return 1;

    /*

    for(int i = 0; i < n_samp; i++)
    {

    	cout << "sample " << i << "  is " << endl;

    	cout << inter_m[i] << endl;

    	cout << resp_tst[i] << endl;


    }

    */


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

    return 1;

   // Multiscale enhancement


}

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

    return 1;

}
