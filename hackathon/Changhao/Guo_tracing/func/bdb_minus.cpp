/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */
/************
                                            ********* LICENSE NOTICE ************
This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 
You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.
1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.
2. You agree to appropriately cite this work in your related studies and publications.
Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )
Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )
3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.
*************/
//BDB-minus algorithm for detection of backbone of a curved image object, such as the body of a C. elegans worm.
//
// Copyright by Hanchuan Peng
//
// This is a re-written version of the Matlab codes.
//
// Last update: 2008-03-05
// Last update: 2008-03-25: add a Qt Global include  to handle the random() and rand() for OS X and Windows
// Last update: 2008-04-02: add the resampling cutplane function
#include "bdb_minus.h"
#include "../basic_c_fun/volimg_proc.h"
//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#ifdef _WIN32
#define fabs(x) ((x<0)?-x:x)
#endif

//#include <QtGlobal>
int Coord2D::dim=2;
int Coord3D::dim=3;
template <class T> list <Coord2D> * main_simple_BDB_minus(Image2DSimple<T> * inimg, const BDB_Minus_ConfigParameter & mypara)
{
	list <Coord2D> * mCoord_new=0;
	//find the mean and std of the entire image
	double mean_val, std_val;
	vol2d_mean_and_std(inimg, mean_val, std_val);
	double th_foreground = mean_val + mypara.Kfactor*std_val;
	//now compute
	V3DLONG i,j;
	vector <LocationSimple> xypos;
	LocationSimple cur_pos;
	T ** inimg_data2d = inimg->getData2dHandle();
	for (j=0;j<inimg->sz1();j++)
	{
		for (i=0;i<inimg->sz0();i++)
		{
			if (inimg_data2d[j][i]>th_foreground)
			{
				cur_pos.x = i;
				cur_pos.y = j;
				cur_pos.z = 0;
				xypos.push_back(cur_pos);
			}
		}
	}
	V3DLONG foreground_pixelnum = xypos.size();
	//
	vector <V3DLONG> turn_pos;
	turn_pos.push_back(-1);
	double TH=0.1;
	double KK=40;
    while (turn_pos.size()>0)
	{
		//randomly initilize the control points
		vector <LocationSimple> mCoord_out;
		for (i=0;i<KK;i++)
		{
#ifdef Q_OS_DARWIN
			mCoord_out.push_back(xypos.at(random()%foreground_pixelnum));
#else
#ifdef Q_OS_WIN32
			mCoord_out.push_back(xypos.at((rand()*rand())%foreground_pixelnum)); //080325, for compatability to Windows
#endif
#endif
		}
		//now generate an MST and use only its diameter
		vector<PtIndexAndParents> parentList;
		vector <LocationSimple> mCoord_diameter;
		find_mst_diameter(mCoord_out, mCoord_diameter, parentList); //note that the function find_mst_diameter() will update (usually shorten) mCoord_out, so that it
		//contains only the coordinates of the diameter
		//now use the BDB_minus method to update the coordinates of the control points
		point_bmeans(xypos, inimg_data2d, &mCoord_diameter, TH, mypara);
		//now remove sharp turns
		//[turn_pos, tt]=detect_sharp_turn(mCoord_new);
		//if (pmCoord_diameter)
		//{delete pmCoord_diameter; pmCoord_diameter=0;}
	}
	/*
	 while (~isempty(turn_pos)),
	 p=randperm(length(ind));
	 TH = 0.1;
	 KK=40;
	 mCoord_out = [xx(p(1:KK)) yy(p(1:KK))];
	 save ini_mCoord.mat mCoord_out
	 m = mst_points(mCoord_out); %% find the MST
	 myord = img_mst_diameter(m);
	 mCoord_out = mCoord_out(myord, :);
	 mCoord_new = point_bmeans([xx yy], double(inimg(ind))/255, mCoord_out, TH, mypara.f_image, mypara.f_smooth, mypara.f_length, inimg);
	 %mCoord_new = point_bmeans([xx yy], double(inimg(ind))/255, mCoord_out, TH, 1, 1, 0.5, inimg);
	 //Now remove the sharp turns
	 [turn_pos, tt]=detect_sharp_turn(mCoord_new);
	 % % if (~isempty(turn_pos)),
	 % %     if (turn_pos<length(mCoord_out)/2),
	 % %         mCoord_out = mCoord_out(turn_pos:end,:);
	 % %     else,
	 % %         mCoord_out = mCoord_out(1:turn_pos,:);
	 % %     end;
	 % % end;
	 end;
	 */
	return mCoord_new;
}
bool find_mst_diameter(const vector <LocationSimple> & ptCoord, vector<LocationSimple> & diameterCoord, vector<PtIndexAndParents> & parentList)
{
	V3DLONG npt = ptCoord.size();
	if (npt<=0)
		return false;
	//first compute the distance matrix
	Image2DSimple<MYFLOAT> *dist = new Image2DSimple<MYFLOAT>(npt, npt);
	if (!dist)
		return false;
	MYFLOAT ** dist2d = dist->getData2dHandle();
	MYFLOAT max_val;
	V3DLONG i,j;
	max_val = -1;
	for(j=0;j<npt;j++)
	{
		LocationSimple c_j = ptCoord.at(j);
		for(i=j;i<npt;i++)
		{
			if (i==j)
			{
				dist2d[i][j] = dist2d[j][i] = 0.0;
				continue;
			}
			LocationSimple c_i = ptCoord.at(i);
			double tmpx = c_j.x-c_i.x; tmpx *= tmpx;
			double tmpy = c_j.y-c_i.y; tmpy *= tmpy;
			double tmpz = c_j.z-c_i.z; tmpz *= tmpz;
			dist2d[i][j] = dist2d[j][i] = sqrt(tmpx+tmpy+tmpz);
			if (max_val<dist2d[i][j])
				max_val = dist2d[i][j];
		}
	}
	if (max_val>0)
	{
		for (j=0;j<npt;j++)
			for (i=j+1;i<npt;i++)
				dist2d[i][j] = dist2d[j][i] = dist2d[j][i]/max_val;
	}
	//then compute the MST
	V3DLONG *mst_ParentList = mst_parents_list(dist);
	parentList.erase(parentList.begin(),parentList.end());
	for (j=0;j<npt;j++)
	{
		if (mst_ParentList[j]!=-1)
		{
			parentList.push_back(PtIndexAndParents(j, mst_ParentList[j]));
		}
	}
	Image2DSimple<MYFLOAT> * m = mst_fullgraph(mst_ParentList, npt);
	V3DLONG length_diameter;
	V3DLONG *myord = img_mst_diameter(m, length_diameter);
	diameterCoord.erase(diameterCoord.begin(), diameterCoord.end());
	//vector<LocationSimple> *outCoord=0;
	if (myord && length_diameter>0)
	{
		//outCoord = new vector<LocationSimple>;
		for (i=0;i<length_diameter;i++)
			diameterCoord.push_back(ptCoord.at(myord[i]));
	}
	//free all memory allocated
	if (myord) {delete myord; myord=0;}
	if (m) {delete m; m=0;}
	if (mst_ParentList) {delete []mst_ParentList; mst_ParentList=0;}
	if (dist) {delete dist; dist=0;}
	return true;
}
template <class T> bool point_bmeans_2d(const vector <Coord2D> & xypos, T ** inimg_data2d, vector <Coord2D> * mCoord_out, double TH, const BDB_Minus_ConfigParameter & mypara)
{
	double gamma=0.5;
	double beta=1;
	double alpha=1;
	//double TH = 0.1; //the threshold to judge if the algorithm converges
	V3DLONG N = xypos.size(); // number of pixels/points
	int M = mCoord_out->size(); // number of control points / centers of k_means
	Image2DSimple <float> * dd = new Image2DSimple <float> (N, M);
	if (!dd || !dd->valid())
	{
		printf("Fail to allocate memory in point_bmeans_2d().\n");
		return false;
	}
	float ** d2d = dd->getData2dHandle();
	int * II = new int [N];
	if (!II)
	{
		printf("Fail to allocate memory in point_bmeans_2d().\n");
		if (dd) delete dd;
		return false;
	}
	V3DLONG i,j;
	Coord2D F_1_term, F_2_term, M_term;
	vector <Coord2D> mCoord_out_new = *mCoord_out;
	int MAXLOOP=100;
	for (int nloop=0; nloop<MAXLOOP; nloop++)
	{
		for (i=0; i<N; i++)
		{
			Coord2D cur_pixel = xypos.at(i);
			int ind_mm; double tmpv;
			for (j=0; j<M; j++)
			{
				double tmpx = cur_pixel.x - mCoord_out->at(j).x;
				double tmpy = cur_pixel.y - mCoord_out->at(j).y;
				d2d[j][i] = sqrt(tmpx*tmpx + tmpy*tmpy);
				if (j==0)
				{
					ind_mm = 0; tmpv = d2d[j][i];
				}
				else
				{
					if (tmpv>d2d[j][i])
					{
						ind_mm = j;
						tmpv = d2d[j][i];
					}
				}
			}
			II[i] = ind_mm;
		}
		for (j=0; j<M; j++)
		{
			//image force
			int b_use_M_term=0;
			bool b_find=false;
			M_term.x = M_term.y = 0;
			double sum_x=0, sum_y=0, sum_px=0, sum_py=0;
			for (i=0;i<N;i++)
			{
				if (II[i]==j)
				{
					if (!b_find) {b_find=true; b_use_M_term=1;}
					int cx = xypos.at(i).x;
					int cy = xypos.at(i).y;
					sum_x += inimg_data2d[cy][cx];
					sum_y += inimg_data2d[cy][cx];
					sum_px += inimg_data2d[cy][cx] * cx;
					sum_py += inimg_data2d[cy][cx] * cy;
				}
			}
			if (b_use_M_term)
			{
				M_term.x = sum_px/sum_x;
				M_term.y = sum_py/sum_y;
			}
			//model force
			if (j<=1)
			{
				if (j==0)
				{
					F_1_term = mCoord_out->at(j+1);
					F_2_term.x = 2*mCoord_out->at(j+1).x - mCoord_out->at(j+2).x;
					F_2_term.y = 2*mCoord_out->at(j+1).y - mCoord_out->at(j+2).y;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
				}
				else //j==1
				{
					F_1_term.x = mCoord_out->at(j-1).x + mCoord_out->at(j+1).x;
					F_1_term.y = mCoord_out->at(j-1).y + mCoord_out->at(j+1).y;
					F_2_term.x = 2*mCoord_out->at(j-1).x + 4*mCoord_out->at(j+1).x - mCoord_out->at(j+2).x;
					F_2_term.y = 2*mCoord_out->at(j-1).y + 4*mCoord_out->at(j+1).y - mCoord_out->at(j+2).y;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
				}
			}
			else if (j>=M-2)
			{
				if (j==M-2)
				{
					F_1_term.x = mCoord_out->at(j-1).x + mCoord_out->at(j+1).x;
					F_1_term.y = mCoord_out->at(j-1).y + mCoord_out->at(j+1).y;
					F_2_term.x = -mCoord_out->at(j-2).x + 4*mCoord_out->at(j-1).x + 2*mCoord_out->at(j+1).x;
					F_2_term.y = -mCoord_out->at(j-2).y + 4*mCoord_out->at(j-1).y + 2*mCoord_out->at(j+1).y;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
				}
				else //j==M-1
				{
					F_1_term = mCoord_out->at(j-1);
					F_2_term.x = -mCoord_out->at(j-2).x + 2*mCoord_out->at(j-1).x;
					F_2_term.y = -mCoord_out->at(j-2).y + 2*mCoord_out->at(j-1).y;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
				}
			}
			else
			{
				F_1_term.x = mCoord_out->at(j-1).x + mCoord_out->at(j+1).x;
				F_1_term.y = mCoord_out->at(j-1).y + mCoord_out->at(j+1).y;
				F_2_term.x = -0.5*mCoord_out->at(j-2).x + 1.5*mCoord_out->at(j-1).x + 1.5*mCoord_out->at(j+1).x - 0.5*mCoord_out->at(j+2).x;
				F_2_term.y = -0.5*mCoord_out->at(j-2).y + 1.5*mCoord_out->at(j-1).y + 1.5*mCoord_out->at(j+1).y - 0.5*mCoord_out->at(j+2).y;
				mCoord_out_new.at(j).x = (beta*F_1_term.x + gamma*F_2_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_out_new.at(j).y = (beta*F_1_term.y + gamma*F_2_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + 2*gamma + b_use_M_term*alpha);
			}
		}
		double score = 0.0;
		for (j=0; j<M; j++)
		{
			score += fabs(mCoord_out_new.at(j).x - mCoord_out->at(j).x) + fabs(mCoord_out_new.at(j).y - mCoord_out->at(j).y);
		}
		printf("score[%d]=%5.3f.\r", nloop, score);
		if (score<TH)
			break;
	}
	*mCoord_out = mCoord_out_new;
	//free space
	if (II) {delete []II;II=0;}
	if (dd) {delete dd; dd=0;}
	return true;
}
bool point_bdb_minus_2d_localwinmass(unsigned char ** inimg_data2d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, vector <Coord2D> & mCoord_out, const BDB_Minus_ConfigParameter & mypara)
{
	double alpha=mypara.f_image;
	double beta=mypara.f_length;
	double gamma=mypara.f_smooth;
	int radius=mypara.radius;
	double TH = mypara.TH; //0.1; //the threshold to judge if the algorithm converges
	int M = mCoord_out.size(); // number of control points / centers of k_means
	V3DLONG i,j;
	Coord2D F_1_term, F_2_term, M_term;
	vector <Coord2D> mCoord_out_new = mCoord_out;
	//  for (int nloop=0; nloop<mypara.nloops; nloop++)
	for (int nloop=0; nloop<10; nloop++)
	{
		for (j=0; j<M; j++)
		{
			//image force
			int b_use_M_term=1;
			bool b_find=false;
			M_term.x = M_term.y = 0;
			double sum_x=0, sum_y=0, sum_px=0, sum_py=0;
			int x0 = mCoord_out.at(j).x - radius; x0 = (x0<0)?0:x0;
			int x1 = mCoord_out.at(j).x + radius; x1 = (x1>=inimg_sz0)?(inimg_sz0-1):x1;
			int y0 = mCoord_out.at(j).y - radius; y0 = (y0<0)?0:y0;
			int y1 = mCoord_out.at(j).y + radius; y1 = (y1>=inimg_sz1)?(inimg_sz1-1):y1;
			int ix, iy;
			for (iy=y0;iy<=y1;iy++)
			{
				for (ix=x0;ix<x1;ix++)
				{
					register unsigned char tmpval = inimg_data2d[iy][ix];
					if (tmpval)
					{
						sum_x += tmpval;
						sum_y += tmpval;
						sum_px += double(tmpval) * ix;
						sum_py += double(tmpval) * iy;
					}
				}
			}
			if (b_use_M_term)
			{
				if (sum_x && sum_y)
				{
					M_term.x = sum_px/sum_x;
					M_term.y = sum_py/sum_y;
				}
				else
				{
					M_term.x = 0;
					M_term.y = 0;
				}
			}
			//model force
			if (j<=1)
			{
				if (j==0)
				{
					F_1_term = mCoord_out.at(j+1);
					F_2_term.x = 2*mCoord_out.at(j+1).x - mCoord_out.at(j+2).x;
					F_2_term.y = 2*mCoord_out.at(j+1).y - mCoord_out.at(j+2).y;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
				}
				else //j==1
				{
					F_1_term.x = mCoord_out.at(j-1).x + mCoord_out.at(j+1).x;
					F_1_term.y = mCoord_out.at(j-1).y + mCoord_out.at(j+1).y;
					F_2_term.x = 2*mCoord_out.at(j-1).x + 4*mCoord_out.at(j+1).x - mCoord_out.at(j+2).x;
					F_2_term.y = 2*mCoord_out.at(j-1).y + 4*mCoord_out.at(j+1).y - mCoord_out.at(j+2).y;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
				}
			}
			else if (j>=M-2)
			{
				if (j==M-2)
				{
					F_1_term.x = mCoord_out.at(j-1).x + mCoord_out.at(j+1).x;
					F_1_term.y = mCoord_out.at(j-1).y + mCoord_out.at(j+1).y;
					F_2_term.x = -mCoord_out.at(j-2).x + 4*mCoord_out.at(j-1).x + 2*mCoord_out.at(j+1).x;
					F_2_term.y = -mCoord_out.at(j-2).y + 4*mCoord_out.at(j-1).y + 2*mCoord_out.at(j+1).y;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
				}
				else //j==M-1
				{
					F_1_term = mCoord_out.at(j-1);
					F_2_term.x = -mCoord_out.at(j-2).x + 2*mCoord_out.at(j-1).x;
					F_2_term.y = -mCoord_out.at(j-2).y + 2*mCoord_out.at(j-1).y;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
				}
			}
			else
			{
				F_1_term.x = mCoord_out.at(j-1).x + mCoord_out.at(j+1).x;
				F_1_term.y = mCoord_out.at(j-1).y + mCoord_out.at(j+1).y;
				F_2_term.x = -0.5*mCoord_out.at(j-2).x + 1.5*mCoord_out.at(j-1).x + 1.5*mCoord_out.at(j+1).x - 0.5*mCoord_out.at(j+2).x;
				F_2_term.y = -0.5*mCoord_out.at(j-2).y + 1.5*mCoord_out.at(j-1).y + 1.5*mCoord_out.at(j+1).y - 0.5*mCoord_out.at(j+2).y;
				mCoord_out_new.at(j).x = (beta*F_1_term.x + gamma*F_2_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_out_new.at(j).y = (beta*F_1_term.y + gamma*F_2_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + 2*gamma + b_use_M_term*alpha);
			}
			//		printf("[%5.3f, %5.3f], %d\n", mCoord_out_new.at(j).x, mCoord_out_new.at(j).y, j);
			//		if (j==11)
			//		{
			//			double ppp=1;
			//			ppp=2;
			//		}
		}
		double score = 0.0;
		for (j=0; j<M; j++)
		{
			score += fabs(mCoord_out_new.at(j).x - mCoord_out.at(j).x) + fabs(mCoord_out_new.at(j).y - mCoord_out.at(j).y);
		}
		printf("score[%d]=%5.3f.\n", nloop, score);
		mCoord_out = mCoord_out_new;
		if (score<TH)
			break;
	}
	//free space
	return true;
}
//the following special function is for segmentation of fly brain
bool point_bdb_minus_2d_localwinmass_bl(unsigned char ** inimg_data2d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, vector<Coord2D> & mCoord_in, vector<Coord2D> &mCoord_out, const BDB_Minus_ConfigParameter & mypara)
{
	double alpha=mypara.f_image;
	double beta=mypara.f_length;
	double gamma=mypara.f_smooth;
	int radius_x=mypara.radius_x;
	int radius_y=mypara.radius_y;
	double TH = mypara.TH; //0.1; //the threshold to judge if the algorithm converges
	//mCoord_out=mCoord_in;
	int M = mCoord_in.size(); // number of control points / centers of k_means
	//cout << "M" << M << endl;
	V3DLONG j;
	Coord2D F_1_term, F_2_term, M_term;
	vector<Coord2D> mCoord_in_new = mCoord_in;
	vector<Coord2D> mCoord_in_old;
	for (int nloop=0; nloop<mypara.nloops; nloop++)
	{
		mCoord_in_old = mCoord_in;
		//for (j=0; j<M; j++)
		//cout << mCoord_in_old[j].x << endl;
		for (j=0; j<M; j++)
		{
			//image force
			int b_use_M_term=1;
			M_term.x = M_term.y = 0;
			V3DLONG x0 = V3DLONG(mCoord_in.at(j).x - radius_x + 0.5);
			x0 = (x0<0)?0:x0;
			V3DLONG x1 = V3DLONG(mCoord_in.at(j).x + radius_x + 0.5);
			x1 = (x1>=(inimg_sz0-1))?(inimg_sz0-1):x1;
			V3DLONG y0 = V3DLONG(mCoord_in.at(j).y - radius_y + 0.5);
			y0 = (y0<0)?0:y0;
			V3DLONG y1 = V3DLONG(mCoord_in.at(j).y + radius_y + 0.5);
			y1 = (y1>=(inimg_sz1-1))?(inimg_sz1-1):y1;
			//	cout << "x0 "<< x0 << ' ' << x1 << " y0 " << ' ' << y0 <<  ' ' << y1 << endl;
			V3DLONG ix, iy;
			double tmpval;
			double sum_xy=0, sum_px=0, sum_py=0;
			for (iy=y0;iy<=y1;iy++)
			{
				for (ix=x0;ix<=x1;ix++)
				{
					if ((tmpval = inimg_data2d[iy][ix])>0)
					{
						tmpval = tmpval*tmpval/255.0;
						sum_xy += tmpval;
						sum_px += tmpval*ix;
						sum_py += tmpval*iy;
					}
				}
			}
			//	cout << "x0=" << x0 << " y0=" << y0 << ' ' << tmpval << endl;
			if (b_use_M_term)
			{
				if (sum_xy>0)
				{
					M_term.x = (sum_px/sum_xy);// + mCoord_in.at(j).x - 1;
					M_term.y = (sum_py/sum_xy);// + mCoord_in.at(j).y - 1;
				}
				else
				{
					printf("warning: sum_xy = 0! check your program to make sure it is correct.\n");
					M_term.x = mCoord_in.at(j).x;
					M_term.y = mCoord_in.at(j).y;
				}
			}
			//	cout << "j=" << j << " M_term " << M_term.x << ' ' << M_term.y << endl;
			//	cout << "mCoord_in.at(j) " << mCoord_in.at(j).x << ' ' << mCoord_in.at(j).y << endl;
			F_1_term.x = F_1_term.y = 0;
			F_2_term.x = F_2_term.y = 0;
			//model force
			if (j<=1)
			{
				if (j==0)
				{
					F_1_term = mCoord_in.at(j+1);
					F_2_term.x = 2*mCoord_in.at(j+1).x - mCoord_in.at(j+2).x;
					F_2_term.y = 2*mCoord_in.at(j+1).y - mCoord_in.at(j+2).y;
					mCoord_in_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_in_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
				}
				else //j==1
				{
					F_1_term.x = mCoord_in.at(j-1).x + mCoord_in.at(j+1).x;
					F_1_term.y = mCoord_in.at(j-1).y + mCoord_in.at(j+1).y;
					F_2_term.x = 2*mCoord_in.at(j-1).x + 4*mCoord_in.at(j+1).x - mCoord_in.at(j+2).x;
					F_2_term.y = 2*mCoord_in.at(j-1).y + 4*mCoord_in.at(j+1).y - mCoord_in.at(j+2).y;
					mCoord_in_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_in_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
				}
			}
			else if (j>=M-2)
			{
				if (j==M-2)
				{
					F_1_term.x = mCoord_in.at(j-1).x + mCoord_in.at(j+1).x;
					F_1_term.y = mCoord_in.at(j-1).y + mCoord_in.at(j+1).y;
					F_2_term.x = -mCoord_in.at(j-2).x + 4*mCoord_in.at(j-1).x + 2*mCoord_in.at(j+1).x;
					F_2_term.y = -mCoord_in.at(j-2).y + 4*mCoord_in.at(j-1).y + 2*mCoord_in.at(j+1).y;
					mCoord_in_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_in_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
				}
				else //j==M-1
				{
					F_1_term = mCoord_in.at(j-1);
					F_2_term.x = -mCoord_in.at(j-2).x + 2*mCoord_in.at(j-1).x;
					F_2_term.y = -mCoord_in.at(j-2).y + 2*mCoord_in.at(j-1).y;
					mCoord_in_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_in_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
				}
			}
			else
			{
				F_1_term.x = mCoord_in.at(j-1).x + mCoord_in.at(j+1).x;
				F_1_term.y = mCoord_in.at(j-1).y + mCoord_in.at(j+1).y;
				F_2_term.x = -0.5*mCoord_in.at(j-2).x + 1.5*mCoord_in.at(j-1).x + 1.5*mCoord_in.at(j+1).x - 0.5*mCoord_in.at(j+2).x;
				F_2_term.y = -0.5*mCoord_in.at(j-2).y + 1.5*mCoord_in.at(j-1).y + 1.5*mCoord_in.at(j+1).y - 0.5*mCoord_in.at(j+2).y;
				mCoord_in_new.at(j).x = (beta*F_1_term.x + gamma*F_2_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_in_new.at(j).y = (beta*F_1_term.y + gamma*F_2_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + 2*gamma + b_use_M_term*alpha);
			}
		}
		double lastscore;
		double score = 0.0;
		for (j=0; j<M; j++)
			score += fabs(mCoord_in_new.at(j).x - mCoord_in.at(j).x) + fabs(mCoord_in_new.at(j).y - mCoord_in.at(j).y);
		//cout << "score[" << nloop << "] = " << score << endl;
		if (score<TH || isnan(score))
			break;
		if (nloop > 0)
		{
			if (fabs(lastscore-score) < 0.00001)
				break;
			else
			{
				mCoord_in = mCoord_in_new;
				mCoord_in[0].x = mCoord_in_old[0].x;
				mCoord_in[M-1].x = mCoord_in_old[M-1].x;
				mCoord_in[0].y = mCoord_in_old[0].y;
				mCoord_in[M-1].y = mCoord_in_old[M-1].y;
				for (j=0; j<M; j++)
					mCoord_in[j].y = mCoord_in_old[j].y;
			}
		}
		else
		{
			//update the coordinates of the control points
			mCoord_in = mCoord_in_new;
			//do not change the start and end points, just useful for this application
			mCoord_in[0].x = mCoord_in_old[0].x;
			mCoord_in[M-1].x = mCoord_in_old[M-1].x;
			mCoord_in[0].y = mCoord_in_old[0].y;
			mCoord_in[M-1].y = mCoord_in_old[M-1].y;
			// do not change the y location. Just useful for this application
			for (j=0; j<M; j++)
				mCoord_in[j].y = mCoord_in_old[j].y;
		}
		lastscore = score;
	}
	//update the coordinates of the control points
	mCoord_out = mCoord_in_new;
	//do not change the start and end points, just useful for this application
	mCoord_out[0].x = mCoord_in_old[0].x;
	mCoord_out[M-1].x = mCoord_in_old[M-1].x;
	mCoord_out[0].y = mCoord_in_old[0].y;
	mCoord_out[M-1].y = mCoord_in_old[M-1].y;
	// do not change the y location. Just useful for this application
	for (j=0; j<M; j++)
		mCoord_out[j].y = mCoord_in_old[j].y;
	//free space and return
	return true;
}
template <class T> bool point_bmeans_3d(const vector <LocationSimple> & xypos, T *** inimg_data3d, vector <LocationSimple> * mCoord_out, double TH, const BDB_Minus_ConfigParameter & mypara)
{
	double gamma=0.5;
	double beta=1;
	double alpha=1;
	//double TH = 0.1; //the threshold to judge if the algorithm converges
	V3DLONG N = xypos.size(); // number of pixels/points
	int M = mCoord_out->size(); // number of control points / centers of k_means
	Image2DSimple <float> * dd = new Image2DSimple <float> (N, M);
	if (!dd || !dd->valid())
	{
		printf("Fail to allocate memory in point_bmeans_3d().\n");
		return false;
	}
	float ** d2d = dd->getData2dHandle();
	int * II = new int [N];
	if (!II)
	{
		printf("Fail to allocate memory in point_bmeans_3d().\n");
		if (dd) delete dd;
		return false;
	}
	V3DLONG i,j;
	LocationSimple F_1_term, F_2_term, M_term;
	vector <LocationSimple> mCoord_out_new = *mCoord_out;
	int MAXLOOP=100;
	for (int nloop=0; nloop<MAXLOOP; nloop++)
	{
		for (i=0; i<N; i++)
		{
			LocationSimple cur_pixel = xypos.at(i);
			int ind_mm; double tmpv;
			for (j=0; j<M; j++)
			{
				double tmpx = cur_pixel.x - mCoord_out->at(j).x;
				double tmpy = cur_pixel.y - mCoord_out->at(j).y;
				double tmpz = cur_pixel.z - mCoord_out->at(j).z;
				d2d[j][i] = sqrt(tmpx*tmpx + tmpy*tmpy + tmpz*tmpz);
				if (j==0)
				{
					ind_mm = 0; tmpv = d2d[j][i];
				}
				else
				{
					if (tmpv>d2d[j][i])
					{
						ind_mm = j;
						tmpv = d2d[j][i];
					}
				}
			}
			II[i] = ind_mm;
		}
		for (j=0; j<M; j++)
		{
			//image force
			int b_use_M_term=0;
			bool b_find=false;
			M_term.x = M_term.y = M_term.z = 0;
			double sum_x=0, sum_y=0, sum_z=0, sum_px=0, sum_py=0, sum_pz=0;
			for (i=0;i<N;i++)
			{
				if (II[i]==j)
				{
					if (!b_find) {b_find=true; b_use_M_term=1;}
					int cx = xypos.at(i).x;
					int cy = xypos.at(i).y;
					int cz = xypos.at(i).z;
					sum_x += inimg_data3d[cz][cy][cx];
					sum_y += inimg_data3d[cz][cy][cx];
					sum_z += inimg_data3d[cz][cy][cx];
					sum_px += inimg_data3d[cz][cy][cx] * cx;
					sum_py += inimg_data3d[cz][cy][cx] * cy;
					sum_pz += inimg_data3d[cz][cy][cx] * cz;
				}
			}
			if (b_use_M_term)
			{
				M_term.x = sum_px/sum_x;
				M_term.y = sum_py/sum_y;
				M_term.z = sum_pz/sum_z;
			}
			//model force
			if (j<=1)
			{
				if (j==0)
				{
					F_1_term = mCoord_out->at(j+1);
					F_2_term.x = 2*mCoord_out->at(j+1).x - mCoord_out->at(j+2).x;
					F_2_term.y = 2*mCoord_out->at(j+1).y - mCoord_out->at(j+2).y;
					F_2_term.z = 2*mCoord_out->at(j+1).z - mCoord_out->at(j+2).z;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (b_use_M_term*alpha*M_term.z) / (b_use_M_term*alpha);
				}
				else //j==1
				{
					F_1_term.x = mCoord_out->at(j-1).x + mCoord_out->at(j+1).x;
					F_1_term.y = mCoord_out->at(j-1).y + mCoord_out->at(j+1).y;
					F_2_term.x = 2*mCoord_out->at(j-1).x + 4*mCoord_out->at(j+1).x - mCoord_out->at(j+2).x;
					F_2_term.y = 2*mCoord_out->at(j-1).y + 4*mCoord_out->at(j+1).y - mCoord_out->at(j+2).y;
					F_2_term.z = 2*mCoord_out->at(j-1).z + 4*mCoord_out->at(j+1).z - mCoord_out->at(j+2).z;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (beta*F_1_term.z + b_use_M_term*alpha*M_term.z) / (2*beta + b_use_M_term*alpha);
				}
			}
			else if (j>=M-2)
			{
				if (j==M-2)
				{
					F_1_term.x = mCoord_out->at(j-1).x + mCoord_out->at(j+1).x;
					F_1_term.y = mCoord_out->at(j-1).y + mCoord_out->at(j+1).y;
					F_1_term.z = mCoord_out->at(j-1).z + mCoord_out->at(j+1).z;
					F_2_term.x = -mCoord_out->at(j-2).x + 4*mCoord_out->at(j-1).x + 2*mCoord_out->at(j+1).x;
					F_2_term.y = -mCoord_out->at(j-2).y + 4*mCoord_out->at(j-1).y + 2*mCoord_out->at(j+1).y;
					F_2_term.z = -mCoord_out->at(j-2).z + 4*mCoord_out->at(j-1).z + 2*mCoord_out->at(j+1).z;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (beta*F_1_term.z + b_use_M_term*alpha*M_term.z) / (2*beta + b_use_M_term*alpha);
				}
				else //j==M-1
				{
					F_1_term = mCoord_out->at(j-1);
					F_2_term.x = -mCoord_out->at(j-2).x + 2*mCoord_out->at(j-1).x;
					F_2_term.y = -mCoord_out->at(j-2).y + 2*mCoord_out->at(j-1).y;
					F_2_term.z = -mCoord_out->at(j-2).z + 2*mCoord_out->at(j-1).z;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (b_use_M_term*alpha*M_term.z) / (b_use_M_term*alpha);
				}
			}
			else
			{
				F_1_term.x = mCoord_out->at(j-1).x + mCoord_out->at(j+1).x;
				F_1_term.y = mCoord_out->at(j-1).y + mCoord_out->at(j+1).y;
				F_1_term.z = mCoord_out->at(j-1).z + mCoord_out->at(j+1).z;
				F_2_term.x = -0.5*mCoord_out->at(j-2).x + 1.5*mCoord_out->at(j-1).x + 1.5*mCoord_out->at(j+1).x - 0.5*mCoord_out->at(j+2).x;
				F_2_term.y = -0.5*mCoord_out->at(j-2).y + 1.5*mCoord_out->at(j-1).y + 1.5*mCoord_out->at(j+1).y - 0.5*mCoord_out->at(j+2).y;
				F_2_term.z = -0.5*mCoord_out->at(j-2).z + 1.5*mCoord_out->at(j-1).z + 1.5*mCoord_out->at(j+1).z - 0.5*mCoord_out->at(j+2).z;
				mCoord_out_new.at(j).x = (beta*F_1_term.x + gamma*F_2_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_out_new.at(j).y = (beta*F_1_term.y + gamma*F_2_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_out_new.at(j).z = (beta*F_1_term.z + gamma*F_2_term.z + b_use_M_term*alpha*M_term.z) / (2*beta + 2*gamma + b_use_M_term*alpha);
			}
		}
		double score = 0.0;
		for (j=0; j<M; j++)
		{
			score += fabs(mCoord_out_new.at(j).x - mCoord_out->at(j).x) + fabs(mCoord_out_new.at(j).y - mCoord_out->at(j).y) + fabs(mCoord_out_new.at(j).z - mCoord_out->at(j).z);
		}
		printf("score[%d]=%5.3f.\n", nloop, score);
		if (score<TH)
			break;
	}
	*mCoord_out = mCoord_out_new;
	//free space
	if (II) {delete []II;II=0;}
	if (dd) {delete dd; dd=0;}
	return true;
}
bool straight_nearestfill(UINT8_TYPE * invol1d, V3DLONG *insz, int szlen,
                          double *bposx, double *bposy, double *alpha, V3DLONG cutPlaneNum,
						  V3DLONG OutWid, //a good default value could be 160
						  UINT8_TYPE * & outvol1d, V3DLONG *&outdims)
{
#define PI 3.141592635
	//check if the parameters make sense
	if (!invol1d || !insz || (szlen!=4 && szlen!=3 && szlen!=2))
	{
		printf("The input parameters are invalid in straight_nearestfill(). Do nothing. \n");
		return false;
	}
	V3DLONG nx = insz[0];
	V3DLONG ny = insz[1];
	V3DLONG nz = (szlen>2) ? insz[2] : 1;
	V3DLONG nc = (szlen>3) ? insz[3] : 1;
	if (nx<=1 || ny<=1 || nz<=0 || nc<=0)
	{
		printf("The SIZE information of input image is not correct. Do nothing. \n");
		return false;
	}
	if (outvol1d)
	{
		printf("The output image DATA pointer is not empty (may contain data) or not initialized as NULL. Do nothing. \n");
		return false;
	}
	if (outdims)
	{
		printf("The output image SIZE pointer is not empty (may contain data) or not initialized as NULL. Do nothing. \n");
		return false;
	}
	if (!bposx || !bposy || !alpha || cutPlaneNum<=0 || OutWid<=0)
	{
		printf("The size of cutting planes' parameters seem incorrect. Do nothing. \n");
		return false;
	}
	//continue to check if parameters are correct
	V3DLONG i,j,k,c;
	//printf("%i %i %i\n", nx, ny, nz);
	for (i=0; i<cutPlaneNum; i++)
	{
//		printf("i=%ld, [%5.3f,%5.3f]<--[0, %ld].\n", bposx[i], bposy[i], nx-1);
//		printf("090722-QL:i=%ld, [%5.3f,%5.3f]<--[0, %ld].\n", i,bposx[i], bposy[i], nx-1);//090722: QL
		if (bposx[i]<0 || bposx[i]>=nx-1)
		{//080912: abondon debug quit
			if (bposx[i]<0) bposx[i]=0;
			if (bposx[i]>=nx-1) bposx[i] = nx-1;
			//	  printf("Find illegal bposx that are out of range [%5.3f]<--[0, %ld].\n", bposx[i], nx-1);
			//	  return false;
		}
		if (bposy[i]<0 || bposy[i]>=ny-1)
		{//080912: abondon debug quit
			if (bposy[i]<0) bposy[i]=0;
			if (bposy[i]>=ny-1) bposy[i] = ny-1;
			//	  printf("Find illegal bposy that are out of range [%5.3f]<--[0, %ld].\n", bposy[i], ny-1);
			//	  return false;
		}
		if (alpha[i]<-PI || alpha[i]>PI)
		{
			printf("Find illegal alpha (cutplane's tangent direction) that are out of range [%7.6f]<--[%7.6f, %7.6f].\n", alpha[i], -PI, PI);
			return false;
		}
	}
	//==================== set up the input and output volume matrix =====================
	outdims = new V3DLONG [4];
	if (!outdims)
	{
		printf("Fail to allocate memory for the SIZE array of the straightened object. Do nothing.\n");
		return false;
	}
	//  outdims[0] = OutWid; //080407
	//  outdims[1] = cutPlaneNum;
	outdims[1] = OutWid;
	outdims[0] = cutPlaneNum;
	outdims[2] = nz;
	outdims[3] = nc;
	outvol1d = new _ELEMENT_GRAPH_UBYTE [V3DLONG(OutWid)*cutPlaneNum*nz*nc];
	if (!outvol1d)
	{
		printf("Fail to allocate memory for the straightened object. Do nothing.\n");
		if (outdims) {delete []outdims; outdims=0;}
		return false;
	}
	_ELEMENT_GRAPH_UBYTE **** outvol4d = 0, **** invol4d = 0;
	if (!new4dpointer(outvol4d, outdims[0], outdims[1], outdims[2], outdims[3], outvol1d) ||
		!new4dpointer(invol4d, nx, ny, nz, nc, invol1d))
	{
		printf("Fail to allocate memory for the straightened object. Do nothing.\n");
		if (outvol1d) {delete []outvol1d; outvol1d=0;}
		if (outdims) {delete []outdims; outdims=0;}
		if (outvol4d) {delete4dpointer(outvol4d, outdims[0], outdims[1], outdims[2], outdims[3]);}
		if (invol4d) {delete4dpointer(invol4d, nx, ny, nz, nc);}
		return false;
	}
	int ptspace = 1; // default value
	//============ generate nearest interpolation ===================
	double base0 = 0;
	V3DLONG Krad = 0;
	if ((V3DLONG)(OutWid/2)*2==OutWid)
	{
		Krad = (OutWid-1)/2;
		base0 = 0;
	}
	else
	{
		Krad = OutWid/2;
		base0 = ptspace/2;
	}
	for (j=0;j<cutPlaneNum; j++)
	{
		double curalpha = alpha[j];
		double ptminx = bposx[j] - cos(curalpha)*(base0+Krad*ptspace);
		double ptminy = bposy[j] - sin(curalpha)*(base0+Krad*ptspace);
		for (k=0; k<OutWid; k++)
		{
			double curpx = ptminx + cos(curalpha)*(k*ptspace);
			double curpy = ptminy + sin(curalpha)*(k*ptspace);
			if (curpx<0 || curpx>nx-1 || curpy<0 || curpy>ny-1)
			{
				for (i=0;i<nz; i++)
				{
					for (c=0;c<nc; c++)
					{
						//		    outvol4d[c][i][j][k] = (_ELEMENT_GRAPH_UBYTE)(0); //out of image and set as default
						outvol4d[c][i][k][j] = (_ELEMENT_GRAPH_UBYTE)(0); //out of image and set as default
					}
				}
				continue;
			}
            
#ifdef _WIN32
			V3DLONG cpx0 = (V3DLONG)(curpx), cpx1 = (V3DLONG)(ceil(curpx));
			V3DLONG cpy0 = (V3DLONG)(curpy), cpy1 = (V3DLONG)(ceil(curpy));
#else
			V3DLONG cpx0 = (V3DLONG)(floor(curpx)), cpx1 = (V3DLONG)(ceil(curpx));
			V3DLONG cpy0 = (V3DLONG)(floor(curpy)), cpy1 = (V3DLONG)(ceil(curpy));
#endif
            double w0x0y = (cpx1-curpx)*(cpy1-curpy);
			double w0x1y = (cpx1-curpx)*(curpy-cpy0);
			double w1x0y = (curpx-cpx0)*(cpy1-curpy);
			double w1x1y = (curpx-cpx0)*(curpy-cpy0);
			for (i=0;i<nz; i++)
			{
				for (c=0;c<nc;c++)
				{
					//		  outvol4d[c][i][j][k] = (_ELEMENT_GRAPH_UBYTE)(w0x0y * double(invol4d[c][i][cpy0][cpx0]) + w0x1y * double(invol4d[c][i][cpy1][cpx0]) +
					outvol4d[c][i][k][j] = (_ELEMENT_GRAPH_UBYTE)(w0x0y * double(invol4d[c][i][cpy0][cpx0]) + w0x1y * double(invol4d[c][i][cpy1][cpx0]) +
																  w1x0y * double(invol4d[c][i][cpy0][cpx1]) + w1x1y * double(invol4d[c][i][cpy1][cpx1]));
				}
			}
		}
	}
	// ====free memory=============
	if (outvol4d) {delete4dpointer(outvol4d, outdims[0], outdims[1], outdims[2], outdims[3]);}
	if (invol4d) {delete4dpointer(invol4d, nx, ny, nz, nc);}
	return true;
}
bool point_bdb_minus_3d_localwinmass(unsigned char *** inimg_data3d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, V3DLONG inimg_sz2, vector <Coord3D> & mCoord_out, const BDB_Minus_ConfigParameter & mypara)
{
	double alpha=mypara.f_image;
	double beta=mypara.f_length;
	double gamma=mypara.f_smooth;
	int radius=mypara.radius;
	double TH = mypara.TH; //0.1; //the threshold to judge if the algorithm converges
	int M = mCoord_out.size(); // number of control points / centers of k_means
	if (M<=2) return true; //in this case no adjusting is needed. by PHC, 090119. also prevent a memory crash
	V3DLONG i,j;
	Coord3D F_1_term, F_2_term, M_term;
	vector <Coord3D> mCoord_out_new = mCoord_out;
	vector <Coord3D> mCoord_out_old;
	double lastscore;
	for (int nloop=0; nloop<mypara.nloops; nloop++)
	{
		mCoord_out_old = mCoord_out;
		for (j=0; j<M; j++)
		{
			//image force
			int b_use_M_term=1;
			bool b_find=false;
			M_term.x = M_term.y = M_term.z = 0;
			double sum_x=0, sum_y=0, sum_z=0, sum_px=0, sum_py=0, sum_pz=0;
			int xc = mCoord_out.at(j).x;
			int yc = mCoord_out.at(j).y;
			int zc = mCoord_out.at(j).z;
			int x0 = xc - radius; x0 = (x0<0)?0:x0;
			int x1 = xc + radius; x1 = (x1>inimg_sz0-1)?(inimg_sz0-1):x1;
			int y0 = yc - radius; y0 = (y0<0)?0:y0;
			int y1 = yc + radius; y1 = (y1>inimg_sz1-1)?(inimg_sz1-1):y1;
			int z0 = zc - radius; z0 = (z0<0)?0:z0;
			int z1 = zc + radius; z1 = (z1>inimg_sz2-1)?(inimg_sz2-1):z1;
			int ix, iy, iz;
			//use a sphere region, as this is easiest to compute the unbiased center of mass
			double dx,dy,dz, r2=double(radius)*radius;
			for (iz=z0;iz<=z1;iz++)
			{
				dz = fabs(iz-zc); dz*=dz;
				for (iy=y0;iy<=y1;iy++)
				{
					dy = fabs(iy-yc); dy*=dy;
					if (dy+dz>r2) continue;
					dy += dz;
					for (ix=x0;ix<x1;ix++)
					{
						dx = fabs(ix-xc); dx*=dx;
						if (dx+dy>r2) continue;
						register unsigned char tmpval = inimg_data3d[iz][iy][ix];
						if (tmpval)
						{
							sum_x += tmpval;
							sum_y += tmpval;
							sum_z += tmpval;
							sum_px += double(tmpval) * ix;
							sum_py += double(tmpval) * iy;
							sum_pz += double(tmpval) * iz;
						}
					}
				}
			}
			if (b_use_M_term)
			{
				if (sum_x && sum_y && sum_z)
				{
					M_term.x = sum_px/sum_x;
					M_term.y = sum_py/sum_y;
					M_term.z = sum_pz/sum_z;
				}
				else
				{
					M_term.x = xc;
					M_term.y = yc;
					M_term.z = zc;
				}
			}
			//printf("M_term : [%5.3f, %5.3f, %5.3f], %d\n", M_term.x, M_term.y, M_term.z, j);
			//model force
			if (j<=1)
			{
				if (j==0)
				{
//					F_1_term = mCoord_out.at(j+1);
//
//					F_2_term.x = 2*mCoord_out.at(j+1).x - mCoord_out.at(j+2).x;
//					F_2_term.y = 2*mCoord_out.at(j+1).y - mCoord_out.at(j+2).y;
//					F_2_term.z = 2*mCoord_out.at(j+1).z - mCoord_out.at(j+2).z;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (b_use_M_term*alpha*M_term.z) / (b_use_M_term*alpha);
				}
				else //j==1
				{
					F_1_term.x = mCoord_out.at(j-1).x + mCoord_out.at(j+1).x;
					F_1_term.y = mCoord_out.at(j-1).y + mCoord_out.at(j+1).y;
					F_1_term.z = mCoord_out.at(j-1).z + mCoord_out.at(j+1).z;
//					F_2_term.x = 2*mCoord_out.at(j-1).x + 4*mCoord_out.at(j+1).x - mCoord_out.at(j+2).x;
//					F_2_term.y = 2*mCoord_out.at(j-1).y + 4*mCoord_out.at(j+1).y - mCoord_out.at(j+2).y;
//					F_2_term.z = 2*mCoord_out.at(j-1).z + 4*mCoord_out.at(j+1).z - mCoord_out.at(j+2).z;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (beta*F_1_term.z + b_use_M_term*alpha*M_term.z) / (2*beta + b_use_M_term*alpha);
				}
			}
			else if (j>=M-2)
			{
				if (j==M-2)
				{
					F_1_term.x = mCoord_out.at(j-1).x + mCoord_out.at(j+1).x;
					F_1_term.y = mCoord_out.at(j-1).y + mCoord_out.at(j+1).y;
					F_1_term.z = mCoord_out.at(j-1).z + mCoord_out.at(j+1).z;
//					F_2_term.x = -mCoord_out.at(j-2).x + 4*mCoord_out.at(j-1).x + 2*mCoord_out.at(j+1).x;
//					F_2_term.y = -mCoord_out.at(j-2).y + 4*mCoord_out.at(j-1).y + 2*mCoord_out.at(j+1).y;
//					F_2_term.z = -mCoord_out.at(j-2).z + 4*mCoord_out.at(j-1).z + 2*mCoord_out.at(j+1).z;
					mCoord_out_new.at(j).x = (beta*F_1_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (beta*F_1_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (beta*F_1_term.z + b_use_M_term*alpha*M_term.z) / (2*beta + b_use_M_term*alpha);
				}
				else //j==M-1
				{
//					F_1_term = mCoord_out.at(j-1);
//
//					F_2_term.x = -mCoord_out.at(j-2).x + 2*mCoord_out.at(j-1).x;
//					F_2_term.y = -mCoord_out.at(j-2).y + 2*mCoord_out.at(j-1).y;
//					F_2_term.z = -mCoord_out.at(j-2).z + 2*mCoord_out.at(j-1).z;
					mCoord_out_new.at(j).x = (b_use_M_term*alpha*M_term.x) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).y = (b_use_M_term*alpha*M_term.y) / (b_use_M_term*alpha);
					mCoord_out_new.at(j).z = (b_use_M_term*alpha*M_term.z) / (b_use_M_term*alpha);
				}
			}
			else
			{
				F_1_term.x = mCoord_out.at(j-1).x + mCoord_out.at(j+1).x;
				F_1_term.y = mCoord_out.at(j-1).y + mCoord_out.at(j+1).y;
				F_1_term.z = mCoord_out.at(j-1).z + mCoord_out.at(j+1).z;
				F_2_term.x = -0.5*mCoord_out.at(j-2).x + 1.5*mCoord_out.at(j-1).x + 1.5*mCoord_out.at(j+1).x - 0.5*mCoord_out.at(j+2).x;
				F_2_term.y = -0.5*mCoord_out.at(j-2).y + 1.5*mCoord_out.at(j-1).y + 1.5*mCoord_out.at(j+1).y - 0.5*mCoord_out.at(j+2).y;
				F_2_term.z = -0.5*mCoord_out.at(j-2).z + 1.5*mCoord_out.at(j-1).z + 1.5*mCoord_out.at(j+1).z - 0.5*mCoord_out.at(j+2).z;
				mCoord_out_new.at(j).x = (beta*F_1_term.x + gamma*F_2_term.x + b_use_M_term*alpha*M_term.x) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_out_new.at(j).y = (beta*F_1_term.y + gamma*F_2_term.y + b_use_M_term*alpha*M_term.y) / (2*beta + 2*gamma + b_use_M_term*alpha);
				mCoord_out_new.at(j).z = (beta*F_1_term.z + gamma*F_2_term.z + b_use_M_term*alpha*M_term.z) / (2*beta + 2*gamma + b_use_M_term*alpha);
			}
			//printf("[%5.3f, %5.3f, %5.3f], %d\n", mCoord_out_new.at(j).x, mCoord_out_new.at(j).y, mCoord_out_new.at(j).z, j);
		}
		double score = 0.0;
		for (j=0; j<M; j++)
			score += fabs(mCoord_out_new.at(j).x - mCoord_out.at(j).x) + fabs(mCoord_out_new.at(j).y - mCoord_out.at(j).y) + fabs(mCoord_out_new.at(j).z - mCoord_out.at(j).z);
		//cout << "score[" << nloop << "] = " << score << endl;
		if (score<TH || isnan(score))
			break;
		if (nloop > 0)
		{
			if (fabs(lastscore-score) < 0.00001)
				break;
		}
		//now reset the two ends' coordinates
		mCoord_out = mCoord_out_new;
		mCoord_out[0].x = mCoord_out_old[0].x;
		mCoord_out[M-1].x = mCoord_out_old[M-1].x;
		mCoord_out[0].y = mCoord_out_old[0].y;
		mCoord_out[M-1].y = mCoord_out_old[M-1].y;
		mCoord_out[0].z = mCoord_out_old[0].z;
		mCoord_out[M-1].z = mCoord_out_old[M-1].z;
		lastscore = score;
	}
		//update the coordinates of the control points, without changing the start and end points, just useful for this application
	mCoord_out = mCoord_out_new;
	mCoord_out[0].x = mCoord_out_old[0].x;
	mCoord_out[M-1].x = mCoord_out_old[M-1].x;
	mCoord_out[0].y = mCoord_out_old[0].y;
	mCoord_out[M-1].y = mCoord_out_old[M-1].y;
	mCoord_out[0].z = mCoord_out_old[0].z;
	mCoord_out[M-1].z = mCoord_out_old[M-1].z;
//		// do not change the y location. Just useful for this application
//
//		for (j=0; j<M; j++)
//			mCoord_out[j].y = mCoord_in_old[j].y;
	//free space
	return true;
}
bool path_statistics_3d(Basic_Path_Statistics & bps, unsigned char *** inimg_data3d, V3DLONG inimg_sz0, V3DLONG inimg_sz1, V3DLONG inimg_sz2, vector <Coord3D> & mCoord)
{
	if (!inimg_data3d || inimg_sz0<=0 || inimg_sz1<=0 || inimg_sz2<=0) return false;
	V3DLONG n_segs = mCoord.size()-1;
	double * seg_profile = 0;
	try {seg_profile = new double [n_segs];}
	catch (...) {return false;}
	for (V3DLONG k=0;k<n_segs;k++)
	{
		seg_profile[k] = twopoints_lineprofile_3dimg_sum(inimg_data3d, inimg_sz0, inimg_sz1, inimg_sz2, mCoord.at(k).x, mCoord.at(k).y, mCoord.at(k).z, mCoord.at(k+1).x, mCoord.at(k+1).y, mCoord.at(k+1).z);
	}
	moment(seg_profile, n_segs, bps.ave, bps.adev, bps.sdev, bps.var, bps.skew, bps.curt);
	if (seg_profile) {delete []seg_profile;}
	return true;
}
