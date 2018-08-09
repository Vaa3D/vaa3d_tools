/*
 *  TIP_DETECTION.cpp
 *
 *  Created by Yang, Jinzhu, on 12/15/10.
 *  revised by Hanchuan Peng, 2010-Dec-20 
 *
 */

#include "tip_detection.h"
#include "v3d_message.h"
#include <math.h>
#include <deque>
#include <algorithm>
#include <functional>

#define BACKGROUND 0 // image background
#define DISOFENDS 1 // distance of two ends
#define NONVISITED -1 //for the float image

void ImageTip::Set_DFS_Seed(const DPoint_t & seed)
{
	m_sptSeed = seed;
}

bool ImageTip::Initialize(unsigned char* data, V3DLONG sz[3]) //do padding
{
	try
	{
		if (!data || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0)
		{
			v3d_msg("Invalid data in SetImageInfo()", 0);
			return false;
		}
	}
	catch (...)
	{
		v3d_msg("Invalid data in SetImageInfo()", 0);
		return false;
	}

	m_ppsOriData1D = data;
	m_szx = sz[0];
	m_szy = sz[1];
	m_szz = sz[2];

    // add margins to x,y,z for 26 neighborhood searching
	realszx = m_szx + 2;
	realszy = m_szy + 2;
	realszz = m_szz + 2;
	totallen = realszx*realszy*realszz;
	
	V3DLONG i,j,k;
	V3DLONG pagesz = realszx*realszy;
	
	//allocate memory and set default
	m_ppsImgData = new unsigned char [totallen];
	memset(m_ppsImgData, BACKGROUND, totallen);
	
	//padding with 1 (margin)
	for (k=0; k<m_szz; k++)
		for (j=0; j<m_szy; j++)
			for (i=0; i<m_szx; i++)
			{
				m_ppsImgData[(k+1)*pagesz + (j+1)*realszx + (i+1)] = m_ppsOriData1D[(k)*m_szy*m_szx + (j)*m_szx + (i)];
			}
	
	//find all boundary/surface points
	for (V3DLONG k=1; k<realszz-1; k++)
		for (V3DLONG j=1; j<realszy-1; j++)
			for (V3DLONG i=1; i<realszx-1; i++)
			{
				if (m_ppsImgData[k*pagesz + j*realszx + i] != BACKGROUND)
				{
					for (int koff=-1; koff<=1; koff++)
						for (int joff=-1; joff<=1; joff++)
							for (int ioff=-1; ioff<=1; ioff++)
								if (m_ppsImgData[(k+koff)*pagesz + (j+joff)*realszx + (i+ioff)] == BACKGROUND)
								{
									DPoint_t t;
									t.m_x = i; t.m_y = j; t.m_z = k; t.m_l = -1;
									m_vdfbptSurface.push_back(t);
									goto Label_BoundaryPt_Found;
								}
				}
				
			Label_BoundaryPt_Found:
				continue;
			}
	
	//
	return true;
}


void ImageTip::Compute_DFS()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	V3DLONG count = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG k = 0;
	V3DLONG l = 0;
	deque<DPoint_t> dfs;
	
	V3DLONG temp_l = 0;
	V3DLONG index_nei = 0;
	
    DPoint_t point;
	point.m_x = m_sptSeed.m_x + 1;
	point.m_y = m_sptSeed.m_y + 1;
	point.m_z = m_sptSeed.m_z + 1;
	point.m_l = 0;  
	dfs.push_back(point);
	
	try  //m_piDFS indicates the distance values
	{
		m_piDFS = new float [totallen]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.", 0);
		if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
		return;
	}
	memset(m_piDFS, NONVISITED, totallen * sizeof(float));
	
	V3DLONG m_pagesz = realszx*realszy; 
	
	m_piDFS[point.m_z * m_pagesz + point.m_y * realszx + point.m_x] = 0; //the distance to the starting pt is 0 (of course!)
	
	while(!dfs.empty())
	{	
        point = dfs.front();
		dfs.pop_front();
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		l = point.m_l;

		// 26
		count = 0; //to indicate the direction type
		for(int m = 0; m < 3; m++)
		{
			for(int n = 0; n < 9; n++)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				++count;
				
				index_nei = z * m_pagesz + y * realszx + x;
				
				if (m_ppsImgData[index_nei] != BACKGROUND)
				{
					switch(count)
					{
						// 6 faces
						case 5:
						case 11:
						case 13:
						case 15:
						case 17:
						case 23:
							temp_l = l + 1;
							if(m_piDFS[index_nei] != NONVISITED)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									//printf("temp=%d m_pidfs=%d\n",temp_l,m_piDFS[index_nei]);
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else //set initial value
							{
								//	printf("tem=%d",temp_l);
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							
						// 12 edges
						case 2:
						case 4:
						case 6:
						case 8:
						case 10:
						case 12:
						case 16:
						case 18:
						case 20:
						case 22:
						case 24:
						case 26:				
							temp_l = l + sqrt(2.0);
							if(m_piDFS[index_nei] != NONVISITED)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else //set initial value
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							
						// 8 vertexes
						case 1:
						case 3:
						case 7:
						case 9:
						case 19:
						case 21:
						case 25:
						case 27:
							temp_l = l + sqrt(3.0);
							if(m_piDFS[index_nei] != NONVISITED)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else  //set initial value
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
						default:
							break;
					}
				}
			}
		}
	}
	
	return;
}

vector<DPoint_t> ImageTip::Search_End_Points()
{
	V3DLONG i, j, k, l;
	V3DLONG index, index_nei;
	
//	DPoint_t last;
//	vector<V3DLONG> len;// 
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG pagesz = realszx*realszy;
	
	for(V3DLONG num = 0; num < m_vdfbptSurface.size(); ++num)
	{
		DPoint_t t = m_vdfbptSurface.at(num);
		k = t.m_z;
		j = t.m_y;
		i = t.m_x;
		
		index = k*pagesz + j*realszx + i;
		l = m_piDFS[index];		
		
		// 26
		bool endp = true;
		for (int m = 0; m < 3; ++m)
		{
			for (int n = 0; n < 9; ++n)
			{
				V3DLONG z = k + nDz[m];
				V3DLONG y = j + nDy[n];
				V3DLONG x = i + nDx[n];

				index_nei = z*pagesz + y*realszx + x;

				if (m_ppsImgData[index_nei] != BACKGROUND)
				{
					if (index != index_nei)
					{
						if (l <= m_piDFS[index_nei]) //this case indicates the current pt must NOT be a local maxima 
						{
							endp = false;
							goto Label_JudgeTipPoint;
						}
					}
				}
			}
		}
		
	Label_JudgeTipPoint:
		// 
		if (endp)
		{
			DPoint_t point_l;	
			point_l.m_x = k;
			point_l.m_y = j;
			point_l.m_z = i;	
			point_l.m_l = l;
			m_vdfsptEndPoint.push_back(point_l);	
		}
	}
	
	return m_vdfsptEndPoint;
}

void ImageTip::Clear()
{
	if (m_piDFS) { delete []m_piDFS; m_piDFS = 0;}
	if (m_ppsImgData) { delete []m_ppsImgData; m_ppsImgData = 0;}
	
	m_vdfsptEndPoint.clear();
	m_vdfbptSurface.clear();
}

vector<DPoint_t> tip_detection(unsigned char *p, V3DLONG sz[3])
{
	vector<DPoint_t> vv;
	
	ImageTip mytip;
	if(!mytip.Initialize(p, sz))
		return vv;

	bool b_find=false;
	for (V3DLONG n=0; n<mytip.totallen; n++)
		if (mytip.m_ppsImgData[n])
		{
			DPoint_t h;
			h.m_z = n/(mytip.realszx*mytip.realszy);
			h.m_y = (n-h.m_z*mytip.realszx*mytip.realszy)/mytip.realszx;
			h.m_x = (n-h.m_z*mytip.realszx*mytip.realszy - h.m_y*mytip.realszx);
			mytip.Set_DFS_Seed(h);
			b_find = true;
			break;
		}
	
	if (b_find==false)
	{
		v3d_msg("You image contains only background!! Do nothing.", 0);
		return vv;
	}
		
	mytip.Compute_DFS();

	return mytip.Search_End_Points();	
}

