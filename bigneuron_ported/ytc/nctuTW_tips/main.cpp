#include <fstream>
#include <iostream>
#include <cstring>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string>
//#include <tuple>
#include "Image3D.h"

#include "IPT.h"
#include "Geo.h"
#include "Dijkstra.h"
#include "Geo.h"
#include "Image3D.h"
#include "DistanceTransTwo.h"
#include "Point3D.h"
#include "LineSeg3D.h"
#include "findingtips.h"

std::string g_sAppDir = "";
std::string g_sInputRawFileName = "input_data.raw";
std::string g_sImageInfoName = "imginfo";
std::string g_sOutSwcFileName = "binarized_Reconstruction.swc";

std::string g_sInputRawFilePath;
std::string g_sImageInfoPath;
std::string g_sOutSwcFilePath;

int width = -1, height = -1, zSize = -1;

double g_rThreshold = 0.9;
int g_nSomaX = -1, g_nSomaY = -1, g_nSomaZ = -1;

unsigned char *imgBuf_raw;
unsigned char *tmp_binarized;
unsigned char *tmp_ForTracing;
unsigned char *Repaired;

//// Smoothing
std::vector< std::vector<Point> > AllSegment;
std::vector< Point > BranchPoint;

const int DIM=3050;
int M[DIM][DIM];
float epsilon= 1.73205081;
//int OneLine (int i, int j, const Point P[], float epsilon);
void Evaluate (int i, int j, int NumberOfPoints, float epsilon, const int M[][DIM], const Point P[], int & MinCost, int & r);
void RestorePath(int i, int j, const int M[][DIM], const Point P[], Point Path[], int & Index);
std::vector< std::vector< Point > > AllBranch;

//// Repairing
//typedef struct _FrontierPoint
class FrontierPoint
{
public:
	FrontierPoint( VectorCordinate v, int idx ):point( v ), CCIdx( idx ){};
	VectorCordinate point;
	int CCIdx;
};// FrontierPoint;

class BridgeEdge
{
public:
	BridgeEdge(int p1, int p2, float w):Idx1(p1), Idx2(p2), weight(w){};
	
	bool operator < ( const BridgeEdge edge)
	{
		if ( weight < edge.weight )
			return true;
		else 
			return false;
	}

	int Idx1, Idx2;
	float weight;
};

//// Tracing 
//typedef struct  _EndPoint3D
class SkeleEndNode3D
{
public:
	SkeleEndNode3D():pathLength(.0),visited(false){} 
	MyGridGraph_3D::_GridNode_3D node;
	float pathLength;
	bool visited;
};//SkeleEndNode3D;

//// PreprocessforTracing
void ImagePreprocessforTracing(char *outFilnename = NULL, const float BIN_THRESHOLD = 0.8, const int CC_2D_THRESHOLD = 9, const int CC_3D_THRESHOLD = 30)
{
	const int Area = width * height;
	const int Vol = Area * zSize;

    unsigned char *imgBuf = new unsigned char [Vol];
    memcpy(imgBuf,imgBuf_raw,Vol);

	//std::cout<<"Image loaded\n";
	//統計灰階的分布 找出binarization的cut point
	int *Slot = new int [ 256 ];
	for(int i=0; i<256; ++i)
		Slot[i] = 0;
	
	int sampleCnt = 0;
	for(int i=0; i<Vol; ++i)
	{
		if( imgBuf[i] > 0 )
		{
			++Slot[ imgBuf[i] ];
			++sampleCnt;
		}
	}

	int GREY_THRESHOLD = 0;
	float accumCnt = 0;
	for(int i=255; i>1; --i)
	{
		accumCnt += Slot[i];
		if( accumCnt / sampleCnt < BIN_THRESHOLD && (accumCnt + Slot[i - 1]) / sampleCnt >= BIN_THRESHOLD ) 
		{
			GREY_THRESHOLD = i-1;
			break;
		}
	}
    //std::cout<<"Grey threshold: "<<GREY_THRESHOLD<<'\n';
	//binarization
	Binarize3D_sliceWise<unsigned char>( imgBuf, width, height, zSize, GREY_THRESHOLD );

	//report binarized result

    tmp_binarized = new unsigned char [Vol];
    memcpy(tmp_binarized,imgBuf,Vol);

	//std::cout<<"Binarization\n";


	//2D CCs
	std::vector< std::vector< int > >CCs;
	for(int z=0; z<zSize; ++z)
	{
		FindCC2Ds( imgBuf + z * Area, width, height, CC_2D_THRESHOLD, CCs );
		
		memset( imgBuf + z * Area, 0, Area );
		for(int i=0; i<CCs.size(); ++i)
			for(int j=0; j<CCs[i].size(); ++j)
				imgBuf[ z * Area + CCs[i][j] ] = 255;

		for(int i=0; i<CCs.size(); ++i)
			CCs[i].clear();
		CCs.clear();
	}

	//std::cout<<"CCs 2D\n";
	//3D CCs
	FindCCs( imgBuf, width, height, zSize, CC_3D_THRESHOLD, CCs );
	memset( imgBuf, 0, Vol );
	for(int i=0; i<CCs.size(); ++i)
		for(int j=0; j<CCs[i].size(); ++j)
			imgBuf[ CCs[i][j] ] = 255;

	//std::cout<<"CCs 3D\n";
	//2D binary dilation
	for(int z=0; z<zSize; ++z)
		BinaryDilation2D( imgBuf + z * Area, width, height );

    //std::cout<<"dlation\n";
    //2D skeletonization
    unsigned char *outBuf = new unsigned char [ Vol ];
    memset(outBuf, 0, Vol );
    for(int z=0; z<zSize; ++z)
        thinning_1frT( imgBuf + z * Area, outBuf + z * Area, width, height );

    //std::cout<<"Thinnig\n";
    //???X binary
    //std::string outName = g_sAppDir + "tmp_ForTracing.bin";
    //std::string outName("tmp_ForTracing.bin");
    //if( outFilnename != NULL )
    //	outName = std::string( outFilnename );

    //std::ofstream outFile( outName.c_str(), std::ios::binary );
    //outFile.write( (char*) outBuf, Vol );

    tmp_ForTracing = new unsigned char [Vol];
    memcpy(tmp_ForTracing,outBuf,Vol);
}

//// Soma_detection
VectorCordinate SomaDetection( unsigned char *imgBuf, unsigned char *binaryBuf, const int width, const int height, const int zSize, float R = 4.0 )
{
	float maxDensity = .0;
	
	int imgV = width * height * zSize;
	int Area = width * height;

	bool *possiblePos = new bool [ imgV ];

	std::list<VectorCordinate> maxLocation;
	for(int i=0; i<imgV; i++)
	{
		if( binaryBuf[i] > 0 )
		{
			possiblePos[i] = true;
			maxLocation.push_back( VectorCordinate( i% width, (i%Area)/width, i / Area ) );
		}
		else
			possiblePos[i]= false;
	}

	
	//while( maxLocation.size() > 1 && R < 10.0 )
	VectorCordinate maxPos;
	while( R < 10 && maxLocation.size() > 1 )
	{
		for(int i=0; i<imgV; i++)
		{
			//if( binaryBuf[i] > 0 && possiblePos[i] )
			if( binaryBuf[i] > 0 )
			{
				int x = i % width;
				int y = (i % Area ) / width;
				int z = i / Area;

				float imgMass = .0;

				for(int mx = -1 * R; mx <= R; mx++)
				{
					if( mx + x < 0 || mx + x >= width ) 
						continue;

					for( int my = -1 * R; my <= R; my++ )
					{
						if( my + y < 0 || my + y >= height )
							continue;

						for(int mz = -1 * ceil(R / 3.0); mz <= ceil(R / 3.0); mz++ )
						{
							if( mz + z < 0 || mz + z >= zSize )
								continue;

							int pos = (mz + z) * Area + (my + y) * width + (mx + x);
							if( binaryBuf[ pos ] > 0 )
								imgMass += imgBuf[ pos ];
						}
					}
				}

				imgMass /= ( R * R * R * 8.0 );

				if( imgMass >= maxDensity )
				{
					maxPos = VectorCordinate( i % width, (i % Area) / width, i / Area );
					if( imgMass > maxDensity )
					{
						maxLocation.clear();					
						maxDensity = imgMass;
					}
					maxLocation.push_back( VectorCordinate( i % width, (i % Area) / width, i / Area ) );
					
					//std::cout<<maxDensity<<' '<<maxLocation.back()<<'\n';
				}
			}
		}

		for(int i=0; i<imgV; i++)
			possiblePos[i] = false;

		std::list< VectorCordinate >::iterator l_Itr = maxLocation.begin();
		for(; l_Itr != maxLocation.end(); l_Itr++)
		{
			int pos = l_Itr->z * Area + l_Itr->y * width + l_Itr->x;
			possiblePos[ pos ] = true;
		}

		R++;

		//std::cout<<R<<' '<<maxDensity<<' '<<maxPos<<'\n';
	}

	//std::cout<<maxLocation.size()<<'\n';
	//std::cout<<maxLocation.front()<<'\n';
	
	return maxLocation.front();

	delete [] possiblePos;
}

//// ImageRepair
bool BridgeEdgeComp(const BridgeEdge lhs, const BridgeEdge rhs)
{
	if( lhs.weight < rhs.weight )
		return true;
	else
		return false;
}

void Endpoint2D(unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector<VectorCordinate >& endP )
{
	int Area = width * height;
	for(int z=0; z<zSize; z++)
	{
		unsigned char* slice = new unsigned char [ Area ];
		memcpy( slice, inBuf + z * Area, Area );

		for(int i=0; i<Area; i++)
		{
			if( slice[i] > 0 )
			{
				int Cnt=0;

				for(int x=-1; x<=1; x++)
				{
					for(int y=-1; y<=1; y++)
					{
						int pos = i + y * width + x;
						if( pos >= 0 && pos < Area && slice[pos] > 0)
							Cnt++;
					}
				}

				if( Cnt <= 2 )
					endP.push_back( VectorCordinate( i % width, i / width, z ) );
			}

		}

		delete [] slice;
	}
}

void Skeletonization( unsigned char* inBuf, unsigned char* skeleBuf, const int width, const int height, const int zSize )
{
	int area = width * height;

	for(int i=0; i<zSize; i++)
	{
		unsigned char* slice = new unsigned char [area];
		unsigned char* sliceSkele = new unsigned char [ area ];

		memcpy( slice, inBuf + i * area, area);

		thinning_1frT( slice, sliceSkele, width, height);

		memcpy( skeleBuf + i * area, sliceSkele, area );

		delete [] slice;
		delete [] sliceSkele;
	}

}

void ImageRepair(unsigned char* imgBuffer, const int width, const int height, const int zSize, const int CC_THRESHOLD = 50, const unsigned char GREY_THRESHOLD = 0)
{
	int imgAera = width * height;
	int imgVolume = imgAera * zSize;

	for(int i=0; i<imgVolume;i++)
	{
		if( imgBuffer[i] > GREY_THRESHOLD )
			imgBuffer[i] = 255;
	}

	std::vector< std::vector<int> >CCs;

	FindCCs(imgBuffer, width, height, zSize, CC_THRESHOLD, CCs);

	for(int i=0; i<imgVolume; i++)
		imgBuffer[i] = 0;

	//std::cout<<"CCs : "<<CCs.size()<<'\n';

	for(int i=0; i<CCs.size(); i++)
		for(int j=0; j<CCs[i].size(); j++)
			imgBuffer[ CCs[i][j] ] = 255;
		
	unsigned char* skeleBuf = new unsigned char [ imgVolume ];
	Skeletonization( imgBuffer, skeleBuf, width, height, zSize );

	std::vector< VectorCordinate > endP;
	Endpoint2D(skeleBuf, width, height, zSize, endP );
	//std::cout<<"endpoint size: "<<endP.size()<<'\n';

		
	std::vector< FrontierPoint > Frontiers;
	
	for(int i=0; i<endP.size(); i++)
	{
		bool HIT_FLAG = false;
		for(int j=0; j<CCs.size() && !HIT_FLAG; j++)
		{
			for(int k=0; k<CCs[j].size() && !HIT_FLAG; k++)
			{
				int pos = endP[i].z * imgAera + endP[i].y * width + endP[i].x;
				if( pos == CCs[j][k] )
				{
					Frontiers.push_back( FrontierPoint(endP[i], j ) );
					HIT_FLAG = true;
				}
			}
		}
	}

	endP.clear();
	std::vector< VectorCordinate >().swap( endP );
	
	std::vector< BridgeEdge > EdgeSet;
	for(int i=0; i<Frontiers.size()-1; i++)
	{
		for(int j=i+1; j<Frontiers.size(); j++)
		{
			if(Frontiers[i].CCIdx != Frontiers[j].CCIdx )
			{
				float weight = L2_Norm( Frontiers[i].point, Frontiers[j].point );
				EdgeSet.push_back( BridgeEdge( i, j, weight ) );
			}
		}
	}

	std::sort( EdgeSet.begin(), EdgeSet.end(), BridgeEdgeComp );
	//std::sort( EdgeSet.begin(), EdgeSet.end() );
	//Greedily merge
	

	int* CCsParent = new int [CCs.size()];
	for(int i=0; i<CCs.size(); i++)
		CCsParent[i] = i;


	bool ALL_VISIT_FLAG = true;
	if( CCs.size() > 1 )
		ALL_VISIT_FLAG = false;

	int VisitCnt = 1;
	
	while( VisitCnt < CCs.size() )
	{
		for(int i=0; i<EdgeSet.size(); i++)
		{
			if( CCsParent[ Frontiers[ EdgeSet[i].Idx1 ].CCIdx ] != CCsParent[ Frontiers[ EdgeSet[i].Idx2 ].CCIdx ] )
			{
				VisitCnt++;
				
				int tmpIdx = CCsParent[ Frontiers[ EdgeSet[i].Idx2 ].CCIdx ];
				
				//std::cout<<VisitCnt<<' '<<CCsParent[ Frontiers[ EdgeSet[i].Idx1 ].CCIdx ]<<' '<<CCsParent[ Frontiers[ EdgeSet[i].Idx2 ].CCIdx ]<<'\n';
				
				VectorCordinate sPoint = Frontiers[ EdgeSet[i].Idx1 ].point;
				VectorCordinate ePoint = Frontiers[ EdgeSet[i].Idx2 ].point;

				VectorCordinate rangeVec = ePoint - sPoint;

				VectorCordinate stepVec = rangeVec;
				stepVec /= EdgeSet[i].weight;

				//std::cout<<sPoint<<'\n'<<ePoint<<'\n'<<rangeVec<<"\n";
				
				int range = ceil( EdgeSet[i].weight / 2.0 );
				for(int t=0; t<=ceil(EdgeSet[i].weight); t++)
				{
					int OriginPos = ( ceil(sPoint.z + stepVec.z * t) ) * imgAera + ( ceil(sPoint.y + stepVec.y * t) ) * width + ( ceil(sPoint.x + stepVec.x * t) );

					for(int z=-1; z<=1; z++)
					{
						for(int y=-1; y<=1; y++)
						{
							for(int x=-1; x<=1; x++)
							{
								int pos = OriginPos + z * imgAera + y * width + x;
								if( pos >=0 && pos < imgVolume)
									imgBuffer[pos] = 255;
							}
						}
					}
				}

				for(int j=0; j<CCs.size(); j++)
					if( CCsParent[j] == tmpIdx )
						CCsParent[j] = CCsParent[ Frontiers[ EdgeSet[i].Idx1 ].CCIdx ];

				//for(int j=0; j<CCs.size(); j++)
			//		std::cout<<CCsParent[j]<<' ';
			//	std::cout<<"\n\n";

				break;
			}
		}
	}

	for(int i=0; i<CCs.size(); i++)
	{
		CCs[i].clear();
		std::vector<int>().swap(CCs[i]);
	}
	CCs.clear();
	std::vector< std::vector<int> >().swap( CCs );

	Frontiers.clear();
	std::vector< FrontierPoint >().swap( Frontiers );

	EdgeSet.clear();
	std::vector< BridgeEdge >().swap( EdgeSet );

	delete [] skeleBuf;
	delete [] CCsParent;
}

//// Tracing 
bool SkeleEndNode_Comp(const SkeleEndNode3D& lhs,const SkeleEndNode3D& rhs)
{
	return lhs.pathLength < rhs.pathLength;
}

void DT_Tracing_Ver2(unsigned char* inBuf, unsigned char* skeleBuf, unsigned char* repairedBuf, unsigned char* CostDT, 
					 const int width, const int height, const int zSize, 
					 const int SrcX, const int SrcY, const int SrcZ, 
					 const double avgR, 
					 char* filename = NULL,
					 const double R = 1.0, const double Ita = 10.0,  const double REMOVE_RATIO = 0.2,
					 const double CONNECT_R_3D = 1.0, const double CONNECT_R_2D = 0.75
					 )
{

	const int area  = width * height;
	const int Volume = area * zSize;
	//unsigned char* skeleBuf = new unsigned char [ area * zSize ];

	time_t start, end;
		
	bool*** skeleMap = new bool** [ zSize ];
	for( int z=0; z<zSize; z++)
	{
		skeleMap[z] = new bool* [ height ];
		for(int y=0; y<height; y++)
			skeleMap[z][y] = new bool [ width ];
	}

	for( int z=0; z < zSize; z++)
	{
		for(int y=0; y < height; y++)
		{
			for(int x=0; x < width; x++)
			{
				int pos = z * area + y * width + x ;
				if( skeleBuf[ pos ] > 0 ) 
				{
					skeleMap[z][y][x] = true;
				}
				else
					skeleMap[z][y][x] = false;
			}
		}
	}

	//delete [] skeleBuf;

	time( &start );
	//MyGridGraph_3D::GridGraph_3 graph3D(inBuf, width, height, zSize);
	MyGridGraph_3D::GridGraph_3 graph3D(repairedBuf, width, height, zSize);
	time( &end );
//	std::cout<<"time for graph construction: "<<difftime( end, start )<<'\n';
	//std::cout<<"Node number: "<<graph3D.GetSize()<<'\n';

	std::vector<VectorCordinate> endP3D;
	Endpoint3D( skeleBuf, width, height, zSize, endP3D );

	//All_Path		//用來存放每個點是被誰relaxed
	int* All_Path = new int [ graph3D.GetSize() ];
	
	double tmpMinDis = 100000.0;
	int minSrcIndex = 0;
	VectorCordinate minSkeleV;

	for( int z=0; z < zSize; z++)
	{
		for(int y=0; y < height; y++)
		{
			for(int x=0; x < width; x++)
			{
				int pos = z * area + y * width + x ;
				if( skeleBuf[ pos ] > 0 ) 
				{
					double tmpDis = sqrt( double( ( SrcX - x ) * ( SrcX - x ) + ( SrcY - y ) * ( SrcY - y ) + ( SrcZ - z ) * ( SrcZ - z ) ) );
					if( tmpDis < tmpMinDis )
					{
						minSkeleV.x = x;
						minSkeleV.y = y;
						minSkeleV.z = z;

						tmpMinDis = tmpDis;
					}
				}
			}
		}
	} 
	
	
	int SrcIdx = graph3D.GetNodeIndex( minSkeleV.x, minSkeleV.y, minSkeleV.z );
	MyGridGraph_3D::GridNode_3D SrcNode( minSkeleV.x, minSkeleV.y, minSkeleV.z );
	

	//std::cout<<"Source node index : "<<SrcIdx<<'\n';
	if( SrcIdx < 0 ) return ;
	
	//改變cost table，強化branch point這個feature
	//收集 2D branch points 如果在 [-4, 4] X [-4, 4] X [-1, 1]內則取最中間的點為代表
	std::list< VectorCordinate > Bps2D;
	BranchPoint2D_Iter2D( skeleBuf, width, height, zSize, Bps2D); //Bp 2D 是以z 遞增排序
	

	std::list<VectorCordinate>::iterator bp_l_Itr = Bps2D.begin();
	
	const unsigned char Award = Ita;
	for( ; bp_l_Itr != Bps2D.end(); ++bp_l_Itr)
	{
		int x = bp_l_Itr->x;
		int y = bp_l_Itr->y; 
		int z = bp_l_Itr->z;

		int pos = z * area + y * width + x;

		for(int mz =-1; mz<=1; ++mz)
		{
			for(int my=-2; my<=2; ++my)
			{
				for(int mx=-2; mx<=2; ++mx)
				{
					if( x + mx >= 0 && x + mx < width && 
						y + my >= 0 && y + my < height &&
						z + mz >= 0 && z + mz < zSize )
					{
						int neighbor = pos + mz * area + my * width + mx;
						double dis = sqrt( (double) mx * mx + my * my + mz * mz );
						
						if( CostDT[ neighbor ] > 0 )
							//CostDT[ neighbor ] += ceil( 4.0 * exp( -1 * dis * dis / 4.0 ) ) ;
							CostDT[ neighbor ] += floor( Award - dis );
					}
				}
			}
		}
	}
	delete [] skeleBuf;
//	std::cout<<"Branch point enhanced\n";


	time( &start );
	//Single_Dijkstra_GridGraph_3D(SrcNode.x, SrcNode.y, SrcNode.z, graph3D, All_Path, skeleMap, inBuf, width, height, zSize);
	if( CostDT != NULL )
		//Single_Dijkstra_GridGraph_3D_Ver2(SrcNode.x, SrcNode.y, SrcNode.z, graph3D, All_Path, skeleMap, CostDT, inBuf, width, height, zSize);
		Single_Dijkstra_GridGraph_3D_Ver2(SrcNode.x, SrcNode.y, SrcNode.z, graph3D, All_Path, skeleMap, CostDT, repairedBuf, width, height, zSize);
	else
		Single_Dijkstra_GridGraph_3D(SrcNode.x, SrcNode.y, SrcNode.z, graph3D, All_Path, skeleMap, inBuf, width, height, zSize);
	time( &end );
//	std::cout<<"Time for Dijkstra: "<<difftime(end, start)<<'\n';

	time(&start);
	std::vector< SkeleEndNode3D > EndNode;
	for(int i=0; i<endP3D.size(); i++)
	{
		SkeleEndNode3D tmpNode;
		tmpNode.node.x = endP3D[i].x;
		tmpNode.node.y = endP3D[i].y;
		tmpNode.node.z = endP3D[i].z;

		tmpNode.node.index = graph3D.GetNodeIndex(endP3D[i].x, endP3D[i].y, endP3D[i].z); 

		if( All_Path[ tmpNode.node.index ] >= 0 )
			EndNode.push_back( tmpNode );
	}

	time(&end);
//	std::cout<<"Time for construct EndNode: "<<difftime(end, start)<<'\n';
	//std::cout<<"end point size: "<<EndNode.size()<<'\n';
	

	bool* CoveredRange = new bool [ area * zSize ];	//標示dilated過後一條path佔的範圍
	for(int i=0; i<area * zSize; i++)
		CoveredRange[i] = false;

	std::vector< std::vector< VectorCordinate > > AllPaths; //存放各個path上所有的point
	VectorCordinate SrcNodePos( SrcNode.x, SrcNode.y, SrcNode.z );  //把neuron root 加入 BranchPoint

	for(int i=0; i<EndNode.size(); i++)		//計算所有的shortest path 的長度
	{
		double pLength = .0;
		MyGridGraph_3D::GridNode_3D endNode = EndNode[i].node;

		if( All_Path[ EndNode[i].node.index ] >= 0)
		{
			while( endNode.x != SrcNode.x || endNode.y != SrcNode.y || endNode.z != SrcNode.z )
			{
				GridNode_3D prevNode = graph3D.GetNode( All_Path[ endNode.index ] );

				pLength += sqrt( (double) (prevNode.x - endNode.x) *  (prevNode.x - endNode.x) +
								(prevNode.y - endNode.y) *  (prevNode.y - endNode.y) +
								(prevNode.z - endNode.z) *  (prevNode.z - endNode.z) );
				endNode = prevNode;
			}
		}
		EndNode[i].pathLength = pLength;
	}//計算所有的shortest path 的長度

	std::sort( EndNode.begin(), EndNode.end(), SkeleEndNode_Comp );
	std::reverse( EndNode.begin(), EndNode.end() );
	//將所有的最短路徑由長到短排序

	while( EndNode.back().pathLength <= REMOVE_RATIO * EndNode[0].pathLength )	//把長度不到max-min path  REMOVE_RATIO 的 path 的endNode拿掉
		EndNode.pop_back();
//	std::cout<<"test";

	int MaxNodeIdx = 0;
	int PathIdx = -1;

	float Max_min_pathLfngth = .0;

	bool ALL_VISIT_FLAG;
	if( EndNode.empty() ) 
		ALL_VISIT_FLAG = true;
	else
		ALL_VISIT_FLAG = false;

	std::vector<SkeleEndNode3D> SurviveNode;	//真正用來建立路徑的EndNode
	
	int graphSize = graph3D.GetSize();
	bool *inPath = new bool [ graphSize ];
	bool *prevInPath = new bool [ graphSize ];
	memset( inPath, 0, sizeof( bool ) * graphSize );

	std::list< MyGridGraph_3D::GridNode_3D > BpList;

	time( &start );

	bool InitialFlag = true;
	while( !ALL_VISIT_FLAG )
	{
		//std::cout<<"End node num: "<<endNodeNum<<'\n';

		++PathIdx;
		GridNode_3D pathNode;
		ALL_VISIT_FLAG = true;
		for(int i=0; i<EndNode.size(); i++)
		{
			if( !EndNode[i].visited )
			{
				EndNode[i].visited = true;
				pathNode = EndNode[i].node;
				SurviveNode.push_back( EndNode[i] );
				MaxNodeIdx = EndNode[i].node.index;
				ALL_VISIT_FLAG = false;
				break;
			}
		}
		if( ALL_VISIT_FLAG )
			break;

		if( PathIdx == 0)
			Max_min_pathLfngth = EndNode[0].pathLength;


		//Track back的時候當path走到已經存在的bp附近就把路徑導引過去
		//初始化
		bool FIND_BP_FLAG = false;
		std::vector< VectorCordinate > testPath;
		if( InitialFlag )
		{
			while( pathNode.x != SrcNode.x || pathNode.y != SrcNode.y || pathNode.z != SrcNode.z )  //畫出現存的最長的最短路徑 
			{
				VectorCordinate tmpPNode( pathNode.x, pathNode.y, pathNode.z );
				testPath.push_back( tmpPNode );
				pathNode = graph3D.GetNode( All_Path[ pathNode.index ] );

			}
			InitialFlag = false;

			//std::cout<<"Initialization done!\n";
		}
		else
		{
			//導引過去
			bool MergedFlag = false;
			while( pathNode.x != SrcNode.x || pathNode.y != SrcNode.y || pathNode.z != SrcNode.z ) 
			{
				VectorCordinate tmpPNode( pathNode.x, pathNode.y, pathNode.z );

				if( MergedFlag )
				{
					testPath.push_back( tmpPNode );
					pathNode = graph3D.GetNode( All_Path[ pathNode.index ] );
					continue;
				}

				//
				//先判斷周圍有沒有已經存在的branch point
				std::list<MyGridGraph_3D::GridNode_3D>::iterator bp_l_Itr = BpList.begin();
				double mindis = 100000.0;
				int closestIdx;
				VectorCordinate closestNode;
				MyGridGraph_3D::GridNode_3D closestGraphNode;

				int discreteRador3D_X = 4, discreteRador3D_Y = 4, discreteRador3D_Z = 1; 
				if( CONNECT_R_3D > 1.5 )
				{
					discreteRador3D_X = CONNECT_R_3D / 0.33;
					discreteRador3D_Y = CONNECT_R_3D / 0.33;
					discreteRador3D_X = CONNECT_R_3D / 1.0;
				}

				for( ;bp_l_Itr != BpList.end(); ++bp_l_Itr)
				{
					if( abs( bp_l_Itr->x - pathNode.x ) > discreteRador3D_X ||  
						abs( bp_l_Itr->y - pathNode.y ) > discreteRador3D_Y || 
						abs( bp_l_Itr->z - pathNode.z ) > discreteRador3D_Z )
						continue;

					FIND_BP_FLAG = true;
					VectorCordinate diffV( tmpPNode.x - bp_l_Itr->x, tmpPNode.y - bp_l_Itr->y, tmpPNode.z - bp_l_Itr->z );

					double dis = diffV.x * diffV.x + diffV.y * diffV.y + diffV.z * diffV.z;
					if( dis < mindis )
					{
						closestIdx = bp_l_Itr->index;
						closestGraphNode = *bp_l_Itr;
						closestNode = VectorCordinate( bp_l_Itr->x, bp_l_Itr->y, bp_l_Itr->z ); 
						mindis = dis;
					}
				}	

				//if( FIND_BP_FLAG )
				//	std::cout<<"Find BP\n";
				
				//如果已經真的變成branch point的點找不到從 Bps2D找
				if( !FIND_BP_FLAG )
				{
					bool ADD_BP_FLAG = false;
					std::list<VectorCordinate>::iterator bp2D_l_Itr = Bps2D.begin();

					int discreteRador2D_X = 3, discreteRador2D_Y = 3, discreteRador3D_Z = 1; 
					if( CONNECT_R_2D > 1.0 )
					{
						discreteRador2D_X = CONNECT_R_2D / 0.33;
						discreteRador2D_Y = CONNECT_R_2D / 0.33;
					}

					for(; bp2D_l_Itr != Bps2D.end(); ++bp2D_l_Itr)
					{
						if( abs( bp2D_l_Itr->x - pathNode.x ) > 3 ||  abs( bp2D_l_Itr->y - pathNode.y ) > 3 || abs( bp2D_l_Itr->z - pathNode.z ) > 1 )
							continue;

						ADD_BP_FLAG = true;
						VectorCordinate diffV( tmpPNode.x - bp2D_l_Itr->x, tmpPNode.y - bp2D_l_Itr->y, tmpPNode.z - bp2D_l_Itr->z );

						double dis = diffV.x * diffV.x + diffV.y * diffV.y + diffV.z * diffV.z;
						if( dis < mindis )
						{
							closestIdx = graph3D.GetNodeIndex( bp2D_l_Itr->x, bp2D_l_Itr->y, bp2D_l_Itr->z );
							closestGraphNode = graph3D.GetNode( closestIdx );
							closestNode = *bp2D_l_Itr;
							mindis = dis;
						}
					}

					if( ADD_BP_FLAG )
					{
						FIND_BP_FLAG = true;
						BpList.push_back( closestGraphNode );
						//std::cout<<"Find new BP\n";
					}
				}

				//找到了就導引過去
				if( FIND_BP_FLAG )
				{
					VectorCordinate Vec = closestNode - tmpPNode;
					double L = L2_Norm( Vec );
					if( L > 0 )
					{
						Vec /= L;

						VectorCordinate preNode = closestNode;
						preNode.x = (int) preNode.x;
						preNode.y = (int) preNode.y;
						preNode.z = (int) preNode.z;
						for(int s=0; s<L; ++s)
						{
							VectorCordinate nxtNode = tmpPNode + Vec * s;
							VectorCordinate discreteNode = nxtNode;
							discreteNode.x = (int) discreteNode.x;
							discreteNode.y = (int) discreteNode.y;
							discreteNode.z = (int) discreteNode.z;
							if( !(discreteNode == preNode ) )
							{
								testPath.push_back( preNode );
								preNode = discreteNode;
							}
						}
					}

					if( testPath.empty() || !(closestNode == testPath.back() ) )
						testPath.push_back( closestNode );

					MergedFlag = true;
					pathNode = graph3D.GetNode( All_Path[ closestIdx ] );
				}
				else
				{
					testPath.push_back( tmpPNode );
					pathNode = graph3D.GetNode( All_Path[ pathNode.index ] );
				}
			}
		}

		VectorCordinate source( SrcNode.x, SrcNode.y, SrcNode.z );
		testPath.push_back( source );
		
		AllPaths.push_back( testPath ); //把現在track的路徑加到 AllPaths
		
		if( InitialFlag )
			InitialFlag = false;

		

		int zR = 2, xR=4, yR =4;
		if( avgR > 4 )
		{
			zR = avgR / 2;
			xR = avgR;
			yR = avgR;
		}

		pathNode = graph3D.GetNode( MaxNodeIdx );
		for(int i=0; i<testPath.size(); i++)
		{
			for(int mz=-zR; mz<=zR; mz++)  
			{
				if( testPath[i].z + mz < 0 || testPath[i].z + mz >= zSize )
					continue;
				for(int my=-yR; my<=yR; my++)
				{
					if( testPath[i].y + my < 0 || testPath[i].y + my >= height )
						continue;
					for(int mx=-xR; mx<=xR; mx++)
					{
						if( testPath[i].x + mx < 0 || testPath[i].x + mx >= width)
							continue;

						int pos = (testPath[i].z + mz) * area + (testPath[i].y + my) * width + (testPath[i].x + mx);  
						if( !CoveredRange[ pos ] )
							CoveredRange[ pos ] = true;
					}
				}
			}
		
		}//畫出自己的branch 勢力範圍


		for(int j=0; j<EndNode.size(); j++)	//更新end point set
			if( !EndNode[j].visited && CoveredRange[ (int)(EndNode[j].node.z * area + EndNode[j].node.y * width + EndNode[j].node.x) ] )
				EndNode[j].visited = true;

	}

	time(&end);
//	std::cout<<"Time for path construction: "<<difftime( end, start )<<'\n';

	delete [] inPath;
	delete [] prevInPath;
	// 把AllPath輸出到text  file, AllPath[i].front() 存的是SrcNode, AllPath[i].back()存的是tip
	//std::ofstream outTxt("trace_out.txt");
	std::ofstream outTxt(filename);
	//outTxt<<"@.IPT@.BCF\n"<<3<<' '<<0<<'\n';	//Header for IPToolBox snake loader
    outTxt <<g_nSomaX<<","<<g_nSomaY<<","<<g_nSomaZ<<std::endl;
	for(int i=0; i<AllPaths.size(); i++)
	{
		//outTxt<<"P "<<i<<" n "<<AllPaths[i].size()<<'\n';
        outTxt<<AllPaths[i][0].x<<','<<AllPaths[i][0].y<<','<<AllPaths[i][0].z<<'\n';
		/*
        for(int j=1; j<AllPaths[i].size(); j++)
			outTxt<<AllPaths[i][j]<<'\n';
         */
	}
	outTxt.close();

	delete [] CoveredRange;
	EndNode.clear();	// release the memory held by EndNode vector;
	std::vector< SkeleEndNode3D >().swap( EndNode );

	Bps2D.clear();

	for( int z=0; z<zSize; z++)
	{
		for(int y=0; y<height; y++)
			delete [] skeleMap[z][y];
		delete [] skeleMap[z];
	}
	delete[] skeleMap;
	
	return;
}

//// PolygonalApproximation
void AllBranchLoader( const char* filepath )
{
	std::ifstream infile( filepath );
	if( !infile.is_open() )
	{
		std::cout<<"fail to open the file: "<<filepath<<'\n';
		return;
	}
	
	char buf[256];
	infile.getline(buf, 256);
	for(int i=0; i<256; i++)
		buf[i] = '\0';
	infile.getline(buf, 256);

	char tmp;
	infile>>tmp;
	while( tmp != 'e' )
	{
		char tmpChar;
		int idx, size;
		infile>>idx>>tmpChar>>size;

		std::vector<Point> tmpBranch;
		for(int i=0; i<size; i++)
		{
			int x,y,z;
			infile>>x>>y>>z;
			tmpBranch.push_back( Point(x,y,z) );
		}

		AllBranch.push_back( tmpBranch );
		infile>>tmp;
	}

}

void BranchPointCollector( std::vector< std::vector< Point > >& AllBranch, std::vector< Point >& pointSet )
{
	if( AllBranch.empty() ) 
	{
		std::cout<<"The input is empty\n";
		return;
	}
	
	for(int i=1; i<AllBranch.size(); i++)
	{
		bool HIT_FLAG = false;
		for(int m=0; m<AllBranch.size(); m++)
		{
			if( m != i )
			{
				for(int j=AllBranch[i].size()-1, n=AllBranch[m].size()-1; j >0 && n >0; j--, n--)
				{
					if( AllBranch[i][j] == AllBranch[m][n] && AllBranch[i][j-1] != AllBranch[m][n-1] )
					{
						bool visited = false;
						for(int k=0; k<pointSet.size(); k++)
						{
							if( AllBranch[i][j-1] == pointSet[k] )
							{
								visited = true;
								break;
							}
						}
						if( !visited )
							pointSet.push_back( AllBranch[i][j-1] );

						//HIT_FLAG = true;
					}
				}
			}
		}
	}

	return;
}

void dp3D(const char* outPath)
{
	std::ofstream outFile(outPath);
	
	outFile<<"@.IPT@.BCF\n"<<3<<' '<<0<<'\n';	//header for IPToolBox snake; 
	int MinCost,r;
	int Start = 22;

	time_t start,end;
	time(&start);

//	std::cout<<AllSegment.size()<<'\n';
	

	for(int i=0; i<AllSegment.size(); i++)
	{
		Point lastPoint;

		int NumberOfPoints = AllSegment[i].size();

		Point P[DIM];
		Point Q[DIM];
		Point Path[DIM];
		LineSeg L[DIM];

		outFile<<"P "<<i<<" n ";
		for(int j=0; j<NumberOfPoints; j++)
			Q[j] = AllSegment[i][j];
		
		for(int i=0; i<NumberOfPoints; i++)
			P[i] = Q[i];

		for (int i=0; i<NumberOfPoints-1; i++) 
			L[i].P1P2(P[i],P[(i+1)]);
		
		for(int i=0; i<NumberOfPoints; i++) 
		{
			for(int j=0; j<NumberOfPoints; j++) 
				//M[i][j] = 0;
				M[i][j] = 100000;
		}
		
		for(int i=0; i<NumberOfPoints; i++) 
			M[i][i] = 1;

		for(int k=1;k<NumberOfPoints;k++) 
		{
			//printf ("k = %d of TotalNumber %d\n",k,NumberOfPoints);
			for(int i=0; i<NumberOfPoints-k; i++)
			{
				Evaluate(i,(i+k),NumberOfPoints,epsilon,M,P,MinCost,r);
				M[i][i+k]=MinCost;
				M[i+k][i]=r;
			}
		}

		int Index = 0;
		Path[Index] = P[0];
		
		RestorePath(0,NumberOfPoints-1,M,P,Path,Index);
		outFile<<(++Index)<<'\n';
		Path[Index-1] = Q[NumberOfPoints - 1];
		//fprintf (fp1,"%d 2\n",Index);
		
		for (int i=0; i<Index; i++)
		{

			outFile<<Path[i].X()<<'\t'<<Path[i].Y()<<'\t'<<Path[i].Z()<<'\n';
			//fprintf (fp1,"%d %f %f \n",i,Path[i].X(),Path[i].Y());

			int x,y,z;
			x = Path[i].X(); y = Path[i].Y(); z = Path[i].Z();

//			outBuf[  z * header.width * header.height + y * header.width + x ] = 255;
		}
	}
	outFile<<"e\n";

	time(&end);
//	std::cout<<outPath<<'\n'<<"time for polygonal path approximation: "<<difftime(end , start)<<'\n';

}

void dp3D(std::vector< std::vector< Point > >& AllSeg, std::vector< Point >& refinedBranch )
{

	int MinCost,r;
	int Start = 22;

	time_t start,end;
	time(&start);

	//std::cout<<AllSeg.size()<<'\n';

	for(int i=0; i<AllSeg.size(); i++)
	{
		Point lastPoint;

		int NumberOfPoints = AllSeg[i].size();

		Point P[DIM];
		Point Q[DIM];
		Point Path[DIM];
		LineSeg L[DIM];

		//outFile<<"P "<<i<<" n ";
		for(int j=0; j<NumberOfPoints; j++)
			Q[j] = AllSegment[i][j];
		
		for(int j=0; j<NumberOfPoints; j++)
			P[j] = Q[j];

		for (int j=0; j<NumberOfPoints-1; j++) 
			L[j].P1P2(P[j],P[(j+1)]);
		
		for(int j=0; j<NumberOfPoints; j++) 
		{
			for(int k=0; k<NumberOfPoints; k++) 
				//M[j][k] = 0;
				M[j][k] = 1000000;
		}
		
		for(int j=0; j<NumberOfPoints; j++) 
			M[j][j] = 1;

		for(int j=1;j<NumberOfPoints;j++) 
		{
			//printf ("k = %d of TotalNumber %d\n",k,NumberOfPoints);
			for(int k=0; k<NumberOfPoints-j; k++)
			{
				Evaluate(k,(k+j),NumberOfPoints,epsilon,M,P,MinCost,r);
				M[k][k+j]=MinCost;
				M[k+j][k]=r;
			}
		}

		int Index = 0;
		Path[Index] = P[0];
		
		RestorePath(0,NumberOfPoints-1,M,P,Path,Index);
		++Index;
//		outFile<<(++Index)<<'\n';
		Path[Index-1] = Q[NumberOfPoints - 1];
		//fprintf (fp1,"%d 2\n",Index);
		
		if( i == 0)
				refinedBranch.push_back( Path[0] );
		for (int j=1; j<Index; j++)
		{
			refinedBranch.push_back( Path[j] );

			//outFile<<Path[i].X()<<'\t'<<Path[i].Y()<<'\t'<<Path[i].Z()<<'\n';
			//fprintf (fp1,"%d %f %f \n",i,Path[i].X(),Path[i].Y());

			int x,y,z;
			x = Path[j].X(); y = Path[j].Y(); z = Path[j].Z();

//			outBuf[  z * header.width * header.height + y * header.width + x ] = 255;
		}
	}
//	outFile<<"e\n";

	time(&end);

}

void Evaluate (int i, int j, int NumberOfPoints, float epsilon, const int M[][DIM], const Point P[], int & MinCost, int & r)
{
  unsigned int k,CostK;
  unsigned int a_line;
  double distance;

  MinCost = j-i+1;
  r = i;

  LineSeg l(P[i],P[j]);
  a_line = 1;
  for (int k=i+1;k<j;k++) {
    distance = l.Distance(P[k]);
    //  printf ("i %d k %d j %d %f \n",i,k,j,distance);
    if (distance > double(epsilon)){
      a_line = 0;
      break;
    }
  }
  if (a_line) {r=i; MinCost = 1;}
  else {
    for (k = i+1; k<j;k++) {
      CostK=M[i][k] + M[k][j];
      if (MinCost > CostK){
	MinCost = CostK;
	r = k;
      }
    }
  }
}

void RestorePath(int i, int j, const int M[][DIM], const Point P[],Point Path[],int & Index)
{
  int r;
  if (M[i][j]>1) {
    //printf ("In the recursion %d \n",Index);
    r = M[j][i];
    RestorePath(i,r,M,P,Path,Index);
    RestorePath(r,j,M,P,Path,Index);
  }
  else {
    //P[M[j][i]].Print();
    //printf ("\n");
    Path[Index++] = P[M[j][i]];
  }

}


///////////////////////////////////////////////   start 5 main 
//order :  PreprocessforTracing Soma_detection ImageRepair Tracing PolygonalApproximation
void do_PreprocessforTracing()
{	
	//**** need to write x y z dimension to imginfo file ****//
    //char filename[80]="input.raw";	//input ****raw name****
    ImagePreprocessforTracing(NULL,g_rThreshold);		//output tmp_binarized  tmp_ForTracing.bin

}

void do_Soma_detection()
{
    if(g_nSomaX!=-1)
        return ;

	int imgV = width * height * zSize;
	unsigned char *imgBuf = new unsigned char [ imgV ];
    memcpy(imgBuf, imgBuf_raw, imgV);

	unsigned char *imgBin = new unsigned char [ imgV ];
    memcpy(imgBin, tmp_binarized, imgV);

	VectorCordinate somaPos = SomaDetection( imgBuf, imgBin, width, height, zSize );

    g_nSomaX = somaPos.x;
    g_nSomaY = somaPos.y;
    g_nSomaZ = somaPos.z;

}

void do_ImageRepair()
{
	int imgAera = width * height;
	int imgVolume = imgAera * zSize;
	
	unsigned char* imgBuffer = new unsigned char [ imgVolume ];
    memcpy( imgBuffer, tmp_ForTracing, imgVolume);

	ImageRepair( imgBuffer, width, height, zSize, 2, 0 );

    Repaired = new unsigned char [ imgVolume ];
    memcpy(Repaired, imgBuffer, imgVolume);
    delete [] imgBuffer;
}

void do_EndPoints3D()
{
	time_t start, end, total_start, total_end;
	time( &total_start );

	//const int Vol =  header.zSize * header.height * header.width;
	const int Vol = width * height * zSize;

	unsigned char* inBuf = new unsigned char [ Vol ];
    memcpy(inBuf, tmp_binarized, Vol);


	unsigned char* skeleBuf = new unsigned char [ Vol ];
	
	//Skeletonization( inBuf, skeleBuf, header.width, header.height, header.zSize );
	
	unsigned char* repairedBuf = new unsigned char [ Vol ];
    memcpy(repairedBuf, Repaired, Vol);


	Skeletonization( repairedBuf, skeleBuf, width, height, zSize );

	int area = width * height;
	unsigned char* CostDT = new unsigned char [ Vol ];
	memset( CostDT, 0, Vol );

	unsigned char* slice = new unsigned char [ area ];

	for(int z=0; z<zSize; z++)
	{
		memcpy(slice, inBuf + z * area, area);
		unsigned char* output = new unsigned char [ area ];
	
		bool SHRINK_FLAG = true;

		memset( output, 0, area );
		//DistanceTransTwo DT2(header.width, header.height, false);
		DistanceTransTwo DT2(width, height, false);
		//DT2.euclideanTrans(slice, output, header.width, header.height);
		DT2.euclideanTrans(slice, output, width, height);
		memcpy(CostDT + z*area, output, area );
		delete [] output;
	}
	delete [] slice;
//	std::cout<<"DT complete\n";

	double sampleCnt = .0;
	double Sum = .0;
	for(int i=0; i<Vol; ++i)
	{
		if( CostDT[i] > 0 )
		{
			Sum += CostDT[i];
			++sampleCnt;
		}
	}

	double avgR = Sum / sampleCnt;
	//std::cout<<"avgR: "<<avgR<<'\n';

	char* filename;

	filename = new char [256];
    strcpy(filename,"EndPoints3D.marker");
	//strcat(/,"_Reconstruction");

//	std::cout<<filename<<'\n';

	int SrcX, SrcY, SrcZ;
    SrcX = g_nSomaX;
    SrcY = g_nSomaY;
    SrcZ = g_nSomaZ;
	//==========================================//
	//	Parameters								//
	//											//
	//	const double R = 1.0,					//
	//	const double Ita = 10.0					//
	//	const double REMOVE_RATIO = 0.2,		//
	//	const double 2D_CONNECT_RATIO = 0.75,	//
	//	const double 3D_CONNECT_RATIO = 1.0 )	//
	//											//
	//==========================================//

	DT_Tracing_Ver2( inBuf, skeleBuf, repairedBuf, CostDT, width, height, zSize, SrcX, SrcY, SrcZ, avgR, filename);
	
	time(&total_end);
//	std::cout<<"total time: "<<difftime(total_end, total_start)<<'\n';


	delete [] filename;

	if( CostDT )
		delete [] CostDT;
	if( repairedBuf )
		delete [] repairedBuf;
	if( inBuf )
		delete [] inBuf;
	
}

void do_PolygonalApproximation()
{
	std::string filepath, outpath;
	float EPSILON = sqrt(2.0);
	
	filepath = "tmp_binarized_Reconstruction";
	
	AllBranchLoader( filepath.c_str() );
	
	BranchPointCollector(AllBranch, BranchPoint );

//	std::cout<<AllBranch.size()<<'\n';
//	std::cout<<BranchPoint.size()<<'\n';


	outpath = std::string("tmp_binarized_Reconstruction");
	outpath.append( "_poly" );
//	std::cout<<outpath<<'\n';

	std::ofstream outGeoPoly( outpath.c_str() );
	
	outGeoPoly<<"@.IPT@.BCF\n"<<3<<' '<<0<<'\n';

	clock_t start_clock = clock();
	for(int i=0; i<AllBranch.size(); i++)
	{
		std::vector< Point > refinedBranch;
		std::vector< Point > tmpSeg;
		tmpSeg.push_back( AllBranch[i][0] );

		for(int j=1; j<AllBranch[i].size(); j++)
		{
			tmpSeg.push_back( AllBranch[i][j] );
			//tmpSeg.push_back( Point( AllBranch[i][j].X(), AllBranch[i][j].Y(), AllBranch[i][j].Z() )  );

			for(int k=0; k<BranchPoint.size(); k++)
			{
				if( BranchPoint[k] == AllBranch[i][j] )
				{
					AllSegment.push_back( tmpSeg );
					tmpSeg.clear();
					std::vector< Point >().swap( tmpSeg );
					tmpSeg.push_back( AllBranch[i][j] );
					break;
				}
			}

		}
		
		AllSegment.push_back( tmpSeg );
		tmpSeg.clear();
		std::vector< Point >().swap( tmpSeg );

		dp3D( AllSegment, refinedBranch );
		outGeoPoly<<"P "<<i<<" n "<<refinedBranch.size()<<'\n';

		for(int j=0; j<refinedBranch.size(); j++)
			outGeoPoly<<refinedBranch[j].X()<<'\t'<<refinedBranch[j].Y()<<'\t'<<refinedBranch[j].Z()<<'\n';

		for(int j=0; j<AllSegment.size(); j++)
		{
			AllSegment[j].clear();
			std::vector< Point >().swap( AllSegment[j] );
		}
		AllSegment.clear();
		std::vector< std::vector< Point > >().swap( AllSegment );

		refinedBranch.clear();
	}
	outGeoPoly<<"e\n";
	clock_t end_clock = clock();
	
//	std::cout<< (double) difftime(end_clock, start_clock) / (double) CLOCKS_PER_SEC <<'\n';

}


void convert_to_SWC(char* filename)
{
	char* out_filename;
	out_filename = new char [256];
	strcpy(out_filename,filename);
	strcat(out_filename,"_nctuTW.swc"); 
    std::ofstream fout_SWC(out_filename);			//output file name
    //std::ofstream fout_SWC(g_sOutSwcFilePath.data());			//output file name
    std::ifstream fin_poly;
    std::string sTmp ="tmp_binarized_Reconstruction_poly";
    fin_poly.open(sTmp.data());

    std::vector <std::vector<std::vector<int> > > data;
    std::string temp_string;
    std::vector<int> V(3,0);
    int temp_int, line_number, vertices_number;

    std::vector<std::vector<int> > temp_vector;

    fin_poly>>temp_string;				//@.IPT@.BCF
    fin_poly>>temp_int>>temp_int;		//3 0
    while(fin_poly>>temp_string)		//P
    {
        temp_vector.clear();
        if(strcmp(temp_string.c_str(),"e")==0) break;				//eof
        fin_poly>>temp_int>>temp_string>>vertices_number;			// 0 N number
        for(int i=0;i<vertices_number;i++)
        {
            fin_poly>>V[0]>>V[1]>>V[2];
            temp_vector.push_back(V);
        }
        line_number++;
        data.push_back(temp_vector);

    }
//    std::cout<<"total lines are "<<data.size()<<std::endl;					//end of load poly file

    std::vector <std::vector<int> > Vertice_data;		//x y z parent type
    std::vector <std::vector<int> > Vertice_data_help;	//x y z			for check repeat
    std::vector<int> V2(4,0);                           //x y z parent type
    int parents=-1;
    int k=0;
    bool find=false;
    int max_length=data[0].size();						//from soma to longest tip
   // std::cout<<"wilson_test"<<max_length<<std::endl;
    for(int i=0; i<data.size(); i++ )
        for(int j=data[i].size()-1 ;j>=0; j--)
        {
            V=data[i][j];
            for(k=0;k<Vertice_data_help.size();k++)
            {
                if(V==Vertice_data_help[k]) 			//find repeat
                {
                    parents=k+1;
                    find=true;
                    break;
                }
            }
            if(k==Vertice_data_help.size())  {			//not find, new vertices
                V2[0]=V[0]; V2[1]=V[1]; V2[2]=V[2];
                if(find)
                    V2[3]=parents++;
                else
                    V2[3]=Vertice_data_help.size();
                    
                find=false;
                if(k==0) V2[3]--;					//for first point
                Vertice_data_help.push_back(V);
                Vertice_data.push_back(V2);

            }
        }

        fout_SWC<<"#nctu.TW Neuron Tracing Output"<<std::endl;
        fout_SWC<<"# total lines are "<<data.size()<<std::endl;		
        //fout_SWC<<"# Created by Wilson HE on 2015"<<std::endl;

	////	 
	
	delete [] out_filename;

	//////////////
        int count=1;
        fout_SWC<<count++<<" 1 "<<Vertice_data[0][0]<<" "<<Vertice_data[0][1]<<" "<<Vertice_data[0][2]<<" 3 "<<Vertice_data[0][3]<<std::endl;
        for(int i=1;i<Vertice_data.size();i++)
            fout_SWC<<count++<<" 2 "<<Vertice_data[i][0]<<" "<<Vertice_data[i][1]<<" "<<Vertice_data[i][2]<<" 1 "<<Vertice_data[i][3]<<std::endl;

}
bool load_v3d(char* filename)		//return true if read finish
{	
	// v3draw format
	// string (raw_image_stack_by_hpeng),  endian flag (1 byte), datatype (2 bytes), sz (4*4 bytes)
	
	std::string formatkey = "raw_image_stack_by_hpeng";
	int lenkey = formatkey.length();
	char* format_read = new char[lenkey+1];
	
	std::ifstream inImg( filename, std::ios::binary );
    inImg.read((char*) format_read, lenkey );			// read string
    format_read[lenkey]='\0';
	if(strcmp(formatkey.c_str(),format_read)!=0)  {  std::cout<<"file header error (string)!!!"<<std::endl;  return false; }
	delete [] format_read;
	
	format_read = new char[1];
	inImg.read((char*) format_read, 1 ); 				//read endian flag
	if(format_read[0]!='B' && format_read[0]!='L') {  std::cout<<"file header error (edian flag)!!!"<<std::endl;  return false; }
	bool endian_big= (format_read[0]=='B');
	delete format_read;
	
	format_read = new char[2];						   //read datatype
	inImg.read((char*) format_read, 2 ); 				
	int chartoint(char*, bool, int);
	int data_type=chartoint(format_read,endian_big, 2);
	if(data_type!=1 && data_type!=2 && data_type!=4 )  {  std::cout<<"file header error (datatype)!!!"<<std::endl;  return false; }
	delete [] format_read;
	
	format_read = new char[4];
	inImg.read((char*) format_read, 4 ); 				//read width height zslice channel
	int width=chartoint(format_read,endian_big,4);
	
	inImg.read((char*) format_read, 4 ); 			
	int height=chartoint(format_read,endian_big,4);
	
    inImg.read((char*) format_read, 4 ); 			
	int zslice=chartoint(format_read,endian_big,4);
	
    inImg.read((char*) format_read, 4 ); 			
	int channel=chartoint(format_read,endian_big,4);
	delete [] format_read;
	
	if(data_type!=1 ) 	{ std::cout<<"I only can do 1 data_type"<<std::endl ; return false;}
	if(channel!=1)	{ std::cout<<"I only can do 1 channel"<<std::endl ; return false;}
	
	int Vol = width*height*zslice;
	char* data_read = new char[Vol];
	inImg.read((char*) data_read, Vol); 
	std::ofstream outFile_raw("input.raw", std::ios::binary );
	outFile_raw.write( (char*) data_read, Vol );

	std::ofstream outFile("imginfo");
	outFile<<width<<" "<<height<<" "<<zslice<<"\n";

	return true;
	
}
int chartoint(char* base, bool endian_big,int length)			// char [2] -> short
{
    short retVal = 0;
    if (!endian_big)
        for (int i=length-1;i>=0;i--) {
            retVal = (retVal<<8) + (base[i] & 0xff);
        }
    else
        for (int i=0;i<length;i++) {
            retVal = (retVal<<8) + (base[i] & 0xff);
        }
       
    return retVal;
}

int findingtipsmain()
{
	//char* filename="00001.FruMARCM-M002262_seg001.lsm.tif.c3.v3draw.uint8.v3draw";
	
	clock_t start_clock = clock();

    std::cout<<"do_PreprocessforTracing"<<std::endl;
	do_PreprocessforTracing();
    std::cout<<"do_Soma_detection"<<std::endl;
	do_Soma_detection();
    std::cout<<"do_ImageRepair"<<std::endl;
	do_ImageRepair();
    std::cout<<"do_EndPoint3D"<<std::endl;
	do_EndPoints3D();

	clock_t end_clock = clock();
	std::cout<< (double) difftime(end_clock, start_clock) / (double) CLOCKS_PER_SEC <<'\n';
}
