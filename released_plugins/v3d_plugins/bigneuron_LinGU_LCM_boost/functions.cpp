/*
 * functions.cpp
 *
 *  Created on: May 18, 2015
 *      Author: gulin
 */

//#include "BoostedNeuron.h"

#include "functions.h"

using namespace cv;

using namespace std;

using namespace cv::ml;






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

				int lc = int(input_img.at<uchar>(v3));

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

				label_img.at<uchar>(v3) = (uchar)c_label[i+j*im_sz[0]+k*im_sz[0]*im_sz[1]];


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

				  int lc = int(label_img.at<uchar>(v3));

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


        bnd[i * 2] = *min_element(PixelIdxListx[i].begin(),PixelIdxListx[i].end());

        bnd[i * 2 + 1] = *max_element(PixelIdxListx[i].begin(),PixelIdxListx[i].end());


        bnd[i * 2 + max_label * 2] = *min_element(PixelIdxListy[i].begin(),PixelIdxListy[i].end());

        bnd[i * 2 + 1 + max_label * 2] = *max_element(PixelIdxListy[i].begin(),PixelIdxListy[i].end());


        bnd[i * 2 + max_label * 4] = *min_element(PixelIdxListz[i].begin(),PixelIdxListz[i].end());

        bnd[i * 2 + 1 + max_label * 4] = *max_element(PixelIdxListz[i].begin(),PixelIdxListz[i].end());


	  }

	  return 0;



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

	    				  int p1 = int(label_img.at<uchar>(v3));

	    				  p1 = (p1 == (l + 1)) ? 1: 0;

	    				  // gradient in the Z direction

	    				  v3[2] = k + 1;

	    				  int p2 = int(label_img.at<uchar>(v3));

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[2] = p1 - p2 + 1;

	    				  // gradient in the Y direction

	    				  v3[0] = i;

	    				  v3[1] = j + 1;

	    				  v3[2] = k;

	    				  p2 = int(label_img.at<uchar>(v3));

	    				  p2 = (p2 == (l + 1)) ? 1: 0;

	    				  g3d[1] = p1 - p2 + 1;

	    				  // gradient in the X direction

	    				  v3[0] = i + 1;

	    				  v3[1] = j;

	    				  v3[2] = k;

	    				  p2 = int(label_img.at<uchar>(v3));

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

	  for(int l = 0; l < max_label; l++)
	  {
		  int cx ;

		  cx = int(centre_cc1.at<float>(l,0));

		  int cy ;

		  cy = int(centre_cc1.at<float>(l,1));

		  int cz;

		  cz = int(centre_cc1.at<float>(l,2));

		  // keep the cubic from breaching the boundary

		  cx = max(cx,cb_sz[0]);

		  cx = min(cx,image.size[0] - cb_sz[0]);



		  cy = max(cy,cb_sz[1]);

		  cy = min(cy,image.size[1] - cb_sz[1]);


		  cz = max(cz,cb_sz[2]);

		  cz = min(cz,image.size[2] - cb_sz[2]);


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

					  cub1.at<uchar>(vc) = image.at<uchar>(v3);

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

						sub_label.at<int>(v3s) = (int)label_img.at<uchar>(v3);




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


			int lt = label_img.at<uchar>(bp);

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



		 //   adjacency_list_t().swap(adj_list);


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

    import_matrix_from_txt_file("LCM_trn_data/LCM_trn_root.txt",v,rows,cols);

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


    	char fn[50];

    	vector <double> child_l;

    	sprintf(fn,"LCM_trn_data/LCM_trn_child_%d.txt",node);

        import_matrix_from_txt_file(fn,child_l,rows,cols);

    	char rf_name[50];

        sprintf(rf_name,"LCM_model/LCM_classifier_%d.xml",node);

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

    for(int i = 0; i < n_samp; i++)
    {

    	cout << "sample " << i << "  is " << endl;

    	cout << inter_m[i] << endl;

    	cout << resp_tst[i] << endl;


    }


	return 1;
}



