/* mst_prim.h

   ver: 0.2

   adapted from PHC, June, 2002
   updated by Hanchuan Peng, Nov, 19, 2004. Add an optional root node
   
   changed by Yinan Wan Feb, 9, 2012. Change to maximum Spanning Tree

  */
#ifndef __MST_PRIM_H_
#define __MST_PRIM_H_


//global variables

const int ColorWhite = 0;
const int ColorGrey = 1;
const int ColorBlack = 2;
const int ColorRed = 3;

class PrimMSTClass //the Prim's MST algorithm
{
public:
  V3DLONG nnode;

  double * adjMatrix1d,  ** adjMatrix2d;

  int * nodeColor; //decide if a node has been visited or not
  V3DLONG * nodeParent; 

  void dosearch(V3DLONG r);//r -- root node
  int allocatememory(V3DLONG nodenum);
  void delocatememory();

  int b_disp;

  PrimMSTClass() {
    nnode = 0;
    adjMatrix1d = 0;
    adjMatrix2d = 0;
    nodeColor = 0;
    nodeParent = 0;
    b_disp = 0;
  }
  ~PrimMSTClass() {
    delocatememory();
    nnode = 0;
  }
};

void PrimMSTClass::dosearch(V3DLONG r) //r -- root node
{
	if (nnode<=0 || !adjMatrix1d || !adjMatrix2d) {
		printf("The input data has not been set yet!\n");
		return;
	}

	//make r a reasonable index

	r = (r<0)?0:r;
	r = (r>nnode)?nnode-1:r;

	V3DLONG i,j;
	V3DLONG nleftnode;

	// initialization

	for (i=0;i<nnode;i++) {
		nodeColor[i] = ColorWhite;
		nodeParent[i] = -1;
	}

	nodeParent[r] = -1;
	nodeColor[r] = ColorBlack;

	// begin BFS loop

	nleftnode = nnode-1;
	V3DLONG par,child;
	while (nleftnode>0) {
		par = -1;
		double max = 0;
		V3DLONG i,j;
		for (i=0;i<nnode;i++) {
			if (nodeColor[i]==ColorBlack) {
				for (j=0;j<nnode;j++) {
					if (nodeColor[j]==ColorWhite && adjMatrix2d[i][j]>0) {
						if (max<adjMatrix2d[i][j]){
							max = adjMatrix2d[i][j];
							par = i;
							child = j;
						}
					}
				}
			}
		}

		if (par==-1) {
			printf("input graph is not connected, please check!\n");
			return;
		}

		nodeParent[child] = par;

		nodeColor[child] = ColorBlack;
		nleftnode--;
	}

	return;
}//%================ end of MST_dosearch()=================

int PrimMSTClass::allocatememory(V3DLONG nodenum) 
{
	if (nodenum>0) {
		nnode = nodenum;
	}

	int b_memory = 1;
	if (nnode>0) {
		delocatememory();
		try{
			nodeColor = new int[nnode];
			adjMatrix2d = new double*[nnode];
			nodeParent = new V3DLONG[nnode];
		}
		catch(...){
			b_memory = 0;
		}
	}
	if (!b_memory) {
		delocatememory();
		return 0; //fail
	}
	else
		return 1; //success
}
void PrimMSTClass::delocatememory() 
{
	if (nodeColor) {delete [] nodeColor; nodeColor = 0;}
	if (nodeParent) {delete [] nodeParent; nodeParent = 0;}
	if (adjMatrix2d) {delete [] adjMatrix2d; adjMatrix2d = 0;}
}

//main program

void mst_prim(double* adj_matrix, V3DLONG n_node, V3DLONG* plist, V3DLONG rootnode)
{
	//copy data

	PrimMSTClass * pMST = new PrimMSTClass;
	if (!pMST)
	{fprintf(stderr,"Fail to allocate memory for MSTClass().\n");}
	pMST->nnode = n_node;
	pMST->allocatememory(pMST->nnode);

	pMST->adjMatrix1d = adj_matrix;
	for (V3DLONG i=0;i<n_node;i++)
		pMST->adjMatrix2d[i] = adj_matrix+i*n_node;

	//begin computation
	pMST->dosearch(rootnode); //set root as the first node

	//create the Matlab structure array

	for (V3DLONG i=0;i<pMST->nnode;i++) {
		plist[i] = pMST->nodeParent[i];
	}

	//free memory and return

	if (pMST) {delete pMST;}
	return;
}
#endif
