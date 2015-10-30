/* mst_dij.h
   maximum path tree by Dijkstra's algorithm
   12-02-14 by Yinan Wan
  */
#ifndef __MST_DIJ_H__
#define __MST_DIJ_H__
bool mst_dij(double* adj_matrix, V3DLONG n_node, V3DLONG* plist, V3DLONG rootnode)
{//output plist: the list of parent nodes
	//initialize
	double** adjMatrix2d;
	double * longest_path;
	bool * visited;
	try {
		adjMatrix2d = new double* [n_node];
		longest_path = new double[n_node];
		visited = new bool[n_node];
	}
	catch (...) 
	{
		fprintf(stderr, "Fail to allocate memory.\n");
		if (adjMatrix2d) {delete []adjMatrix2d; adjMatrix2d=NULL;}
		if (visited) {delete []visited; visited=NULL;}
		if (longest_path) {delete []longest_path; longest_path=NULL;}
	}
	for (V3DLONG i=0;i<n_node;i++)
	{
		adjMatrix2d[i] = adj_matrix + i*n_node;
		longest_path[i] = 0;
		visited[i] = false;
		plist[i] = -1;
	}

	
	//begin computation
	V3DLONG cur = rootnode;
	while (cur>=0)
	{
		for (V3DLONG i=0;i<n_node;i++)
		{
			if (adjMatrix2d[cur][i] && !visited[i] && cur!=i) //refresh longest path for cur node's unvisited neighbor
			{
				if (longest_path[cur]+adjMatrix2d[cur][i] >= longest_path[i])
				{
					longest_path[i] = longest_path[cur]+adjMatrix2d[cur][i];
					plist[i] = cur;
				}
			}
		}
		visited[cur] = true;
		//set the unvisited node marked witht the longest tentative distance as the "current node"
		double max_tentDis = 0;
		cur = -1;
		for (V3DLONG i=0;i<n_node;i++)
		{
			if (!visited[i] && longest_path[i]>=max_tentDis)
			{
				cur = i;
				max_tentDis = longest_path[i];
			}
		}
				
	}

	for (V3DLONG i=0;i<n_node;i++)
	{
		if (i==rootnode) continue;
		if (plist[i]<0)
		{
			fprintf(stderr,"The graph is not connected.\n");
            fprintf(stderr,"%d  parentid is %d \n", i,plist[i]);

            return false;

		}
	}

	//free memory and return
	if (adjMatrix2d) {delete []adjMatrix2d; adjMatrix2d=NULL;}
	if (visited) {delete []visited; visited=NULL;}
	if (longest_path) {delete []longest_path; longest_path=NULL;}
	return true;
}
#endif
