#include "Graph_AdjacencyList3D.h"
#include <fstream>

using namespace MyGridGraph_3D;

void GridGraph_3::Delete()
{
	for( int i=0; i<AdjList.size(); i++)
		AdjList[i].clear();
	AdjList.clear();
}

void GridGraph_3::MakeGraph( unsigned char* const inImage, double* const CostBuf, const int width, const int height, const int zSize )
{
	int area = width * height;
	int imgVolume = width * height * zSize;

	size = 0;
	int index = 0;
	for(int z=0; z<zSize; z++)
	{
		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width; x++)
			{
				if( inImage[ z * area + y * width + x] > 0)
				{
					std::list< GridNode_3D > tmpList;
					GridNode_3D tmpNode(x, y, z, index, CostBuf[z * area + y * width + x] );
					tmpList.push_front(tmpNode);
					AdjList.push_back(tmpList);
					index++;
					size++;
				}
			}
		}
	}

	bool* visited = new bool [ imgVolume ];
	for(int i=0; i<imgVolume; i++ )
		visited[i] = false;

	for(int i=0; i<AdjList.size(); i++)
	{
		GridNode_3D currentNode = AdjList[i].front();
		
		visited[ currentNode.z * area + currentNode.y * width + currentNode.x ] = true;

		int MaxDegree = 0;
		
		for(int mx=-1; mx<=1; mx++)
		{
			for(int my=-1; my<=1; my++)
			{
				for(int mz=-1; mz<=1; mz++)
				{
					int pos = (currentNode.z + mz) * area + (currentNode.y + my) * width + (currentNode.x + mx);
					if( pos >=0 && pos < imgVolume && inImage[pos] > 0)
						MaxDegree++;
				}

			}
		}

		MaxDegree -= ( AdjList[i].size() );

		for(int j=i+1; j<AdjList.size() && MaxDegree > 0; j++)
		{
			GridNode_3D tmpNode = AdjList[j].front();
			
			int pos = tmpNode.z * area + tmpNode.y * width + tmpNode.x;

			if( !visited[pos] && abs( currentNode.x - tmpNode.x ) <= 1 && abs( currentNode.y - tmpNode.y ) <= 1 && abs( currentNode.z - tmpNode.z ) <= 1 )
			{
				AdjList[i].push_back( tmpNode );
				AdjList[j].push_back( currentNode );
				MaxDegree--;
			}
		}

		
	}

	delete [] visited;
}

void GridGraph_3::MakeGraph( unsigned char* const inImage, const int width, const int height, const int zSize )
{
	int area = width * height;
	int imgVolume = width * height * zSize;

	size = 0;
	int index = 0;
	for(int z=0; z<zSize; z++)
	{
		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width; x++)
			{
				if( inImage[ z * area + y * width + x] > 0)
				{
					std::list< GridNode_3D > tmpList;
					GridNode_3D tmpNode(x, y, z, index, 0);
					tmpList.push_front(tmpNode);
					AdjList.push_back(tmpList);
					index++;
					size++;
				}
			}
		}
	}

	bool* visited = new bool [ imgVolume ];
	for(int i=0; i<imgVolume; i++ )
		visited[i] = false;

	for(int i=0; i<AdjList.size(); i++)
	{
		GridNode_3D currentNode = AdjList[i].front();
		
		visited[ currentNode.z * area + currentNode.y * width + currentNode.x ] = true;

		int MaxDegree = 0;
		
		for(int mx=-1; mx<=1; mx++)
		{
			for(int my=-1; my<=1; my++)
			{
				for(int mz=-1; mz<=1; mz++)
				{
					int pos = (currentNode.z + mz) * area + (currentNode.y + my) * width + (currentNode.x + mx);
					if( pos >=0 && pos < imgVolume && inImage[pos] > 0)
						MaxDegree++;
				}

			}
		}

		MaxDegree -= ( AdjList[i].size() );

		for(int j=i+1; j<AdjList.size() && MaxDegree > 0; j++)
		{
			GridNode_3D tmpNode = AdjList[j].front();
			
			int pos = tmpNode.z * area + tmpNode.y * width + tmpNode.x;

			if( !visited[pos] && abs( currentNode.x - tmpNode.x ) <= 1 && abs( currentNode.y - tmpNode.y ) <= 1 && abs( currentNode.z - tmpNode.z ) <= 1 )
			{
				AdjList[i].push_back( tmpNode );
				AdjList[j].push_back( currentNode );
				MaxDegree--;
			}
		}

		
	}

	delete [] visited;
}

void GridGraph_3::MakeGraph( std::ifstream& inG )
{
	char tmp;
	int n;
	inG >> tmp >> n;
	
	size = n;

	for( int i=0; i<size; i++ )
	{
		int idx,x,y,z;
		inG>>idx>>x>>y>>z;
		std::list< GridNode_3D > tmpList;
		GridNode_3D tmpNode(x, y, z, idx, 0);
		tmpList.push_front( tmpNode );
		AdjList.push_back( tmpList );
	}
	
	for(int i=0; i<size; i++)
	{
		int idx;
		inG>>tmp>>n;
		for(int j=0; j<n; j++)
		{
			inG >> idx;
			AdjList[j].push_back( AdjList[ idx ].front() );
		}
	}

}
int GridGraph_3::GetNodeIndex(const int x, const int y, const int z) const 
{
	for(int i=0; i<AdjList.size(); i++)
	{
		if( AdjList[i].front().x == x && AdjList[i].front().y == y && AdjList[i].front().z == z )
		{
			return i;
		}
	}

	//std::cout<<"The node does not exist\n";
	return -1;
}

GridNode_3D GridGraph_3::GetNode(int index) const
{
	if( index < 0 || index > size )
	{
		std::cout<<index<<"Out of range in GetNode()\n";
		exit(1);
	}
	else
		return AdjList[index].front();
}

void GridGraph_3::OutputGraph( std::ofstream& outG)
{
//	outG.write( (char*) &( AdjList.size() ), 4 );
	outG<<"n "<<AdjList.size()<<'\n';

	for(int i=0; i<AdjList.size(); i++ )
		outG<<AdjList[i].front()<<'\n';

	for(int i=0; i<AdjList.size(); i++)
	{
		std::list< GridNode_3D >::iterator l_Itr = AdjList[i].begin();
		outG<<"l "<<AdjList[i].size()-1<<' ';
		++l_Itr;
		for(l_Itr; l_Itr != AdjList[i].end(); l_Itr++)
			outG<<l_Itr->index<<' ';

		outG<<'\n';
	}
}
