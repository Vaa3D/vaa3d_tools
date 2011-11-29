/*y_ytree.h
 *created by Yang Yu
 *20090803
 */

// YTree codes
//
//e{0,1,2,3} i{1,2,3}
//
//class 1:
//		     |
//	  e0     |
//           |
//           *  soma
//
//class 2:
//        \     /
//	  e1   \   /  e1 
//          \ /
//		     +
//	  i1     |
//		     |
//           *  soma
//
//class 3:
//           \     /
//	     e2   \   /  e2 
//             \ /
//        \     +
//	  e1   \   /  i2 
//          \ /
//		     +
//	  i1     |
//		     |
//           *  soma
//
//class 4:
//
//                \     /
//	          e3   \   /  e3 
//                  \ /
//           \       +
//            \     /
//	      e2   \   /  i3 
//              \ /
//       \       +
//		  \     /
//	  e1   \   /  i2 
//          \ /
//		     +
//	  i1     |
//		     |
//           *  soma

#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"

struct YTreeNode 
{
	float x,y,z,r; //location and radius
	int nid; //self id same with swc
	int pid; //parent id in swc
	
	const char* ytbcode; //code for branch
	float dc; //distance of this node to cell
	
};

class YTree 
{
public:
	YTree(){}
	~YTree(){}
	
	bool buildYT(NeuronTree neurons)
	{
		NeuronSWC *pNeuron=0;
		QList <YTreeNode> pYTBlist;
		pYTBlist.clear();
		
		long NBIN = neurons.listNeuron.size();
		long *flag = new long [NBIN];
		if(!flag)
		{
			printf("Error for allocate memory!");
			return false;
		}
		else
		{
			for(long i=0; i<NBIN; i++)
				flag[i]=0;
		}
//		long *count = new long [NBIN];
//		if(!count)
//		{
//			printf("Error for allocate memory!");
//			return false;
//		}
//		else
//		{
//			for(long i=0; i<NBIN; i++)
//				count[i]=0;
//		}
		
		for (long ii=0; ii<NBIN; ii++)
		{
			pNeuron = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			
			//root
			if(pNeuron->pn == -1)
			{
				cell.x = pNeuron->x;
				cell.y = pNeuron->y;
				cell.z = pNeuron->z;
				cell.r = pNeuron->r;
				
				cell.nid = pNeuron->n;
				cell.pid = -1;
				cell.ytbcode = "soma";
				cell.dc = 0;
				
				//pYTBlist.append(cell);
			}
			
			//finding branch point
			if(pNeuron->pn != -1)
				flag[pNeuron->pn-1]++;
		}
		
		NeuronSWC *preNeuron = (NeuronSWC *)(&(neurons.listNeuron.at(0)));
		YTreeNode *pYTNpre = &cell;
		YTreeNode *pYTNode;
		
		long number = 1; //coding NO.
		
		//coding
		for (long ii=0; ii<NBIN; ii++)
		{
			NeuronSWC *curNeuron = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			
			//printf("%ld \n", flag[curNeuron->n-1]);
			
			if(curNeuron->pn != -1)
			{
				//printf("flag[curNeuron->n-1] %d \n", flag[curNeuron->n-1]);
				
				if(flag[curNeuron->n-1]==0 || flag[curNeuron->n-1]>=2)
				{
					//printf("begin\n");
					
					while(flag[curNeuron->pn-1]==1)
					{
						pYTNode = new YTreeNode;
						pYTNode->x = curNeuron->x;
						pYTNode->y = curNeuron->y;
						pYTNode->z = curNeuron->z;
						pYTNode->r = curNeuron->r;
						
						pYTNode->nid = curNeuron->n;
						pYTNode->pid = curNeuron->pn;
						
//						if(flag[preNeuron->pn-1]<2)
//							pYTNode->ytbcode = pYTNpre->ytbcode;
//						else
//							pYTNode->ytbcode = number;
						
						//compute the distance to the cell
						pYTNode->dc = 0;
						NeuronSWC *pCurNode=curNeuron;
						while(pCurNode->pn!=-1)
						{
							//printf("test no. %d %d \n", pCurNode->pn-1, flag[pCurNode->pn-1]);
							NeuronSWC *preNode = (NeuronSWC *)(&(neurons.listNeuron.at(pCurNode->pn-1)));
							
							float dist = sqrt((pCurNode->x - preNode->x)*(pCurNode->x - preNode->x) + (pCurNode->y - preNode->y)*(pCurNode->y - preNode->y) +(pCurNode->z - preNode->z)*(pCurNode->z - preNode->z));
							pYTNode->dc = pYTNpre->dc + dist;
							
							pCurNode = preNode;	
						}

						pYTBlist.append(*pYTNode);
						pYTNpre = pYTNode;
						curNeuron = (NeuronSWC *)(&(neurons.listNeuron.at(curNeuron->pn-1)));
						
						//printf("%d %d %f \n", curNeuron->n-1, curNeuron->pn-1, pYTNode->dc);
						//printf("flag[curNeuron->n-1] %d \n", flag[curNeuron->n-1]);
					}
					
					//re-compute the branch point
					if(flag[curNeuron->pn-1]>=2 || curNeuron->pn == -1)
					{
						pYTNode = new YTreeNode;
						pYTNode->x = curNeuron->x;
						pYTNode->y = curNeuron->y;
						pYTNode->z = curNeuron->z;
						pYTNode->r = curNeuron->r;
						
						pYTNode->nid = curNeuron->n;
						pYTNode->pid = curNeuron->pn;
						
//						if(flag[preNeuron->pn-1]<2)
//							pYTNode->ytbcode = pYTNpre->ytbcode;
//						else
//							pYTNode->ytbcode = number;
						
						//compute the distance to the cell
						pYTNode->dc = 0;
						NeuronSWC *pCurNode=curNeuron;
						while(pCurNode->pn!=-1)
						{
							//printf("test no. %d %d \n", pCurNode->pn-1, flag[pCurNode->pn-1]);
							NeuronSWC *preNode = (NeuronSWC *)(&(neurons.listNeuron.at(pCurNode->pn-1)));
							
							float dist = sqrt((pCurNode->x - preNode->x)*(pCurNode->x - preNode->x) + (pCurNode->y - preNode->y)*(pCurNode->y - preNode->y) +(pCurNode->z - preNode->z)*(pCurNode->z - preNode->z));
							pYTNode->dc = pYTNpre->dc + dist;
							
							pCurNode = preNode;	
						}
						
						//add branch point
						pYTBlist.append(*pYTNode);
					}

					//add branch
					pYTTlist.append(pYTBlist);
					pYTBlist.clear();
					//printf("done for one branch\n");
				}
				
			}
			
			preNeuron = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
		}		
		
		//de-alloc
		if(flag) {delete []flag; flag=0;} 
//		if(count) {delete []count; count=0;} 
		
		//end
		return true;
	}

	
public:
	QList < QList<YTreeNode> > pYTTlist;
	YTreeNode cell; //root
	
	
};

