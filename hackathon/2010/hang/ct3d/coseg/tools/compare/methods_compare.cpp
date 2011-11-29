//
//=======================================================================
// Copyright 2011 Institute PICB.
// Authors: Hang Xiao
// Data : March 20, 2011
//=======================================================================
//


#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <cassert>

#include "../../myalgorithms.h"      // SetStartTime PrintElapsedTime
#include "../../component_tree.h"
#include "lp_lib.h"
#include "../../CT3D/bipartite.h"

using namespace std;

bool methods_compare(ComponentTree* tree1, ComponentTree* tree2);

int main(int argc, char* argv[])
{
	time_t start, end;
	time(&start);
	ComponentTree* tree1 = new ComponentTree();
	ComponentTree* tree2 = new ComponentTree();
	tree1->load(argv[1]);
	tree2->load(argv[2]);
	methods_compare(tree1,tree2);
	time(&end);
	cout<<"time elapsed (s) : "<<difftime(end, start)<<endl;

}

int max_item(vector<float> items)
{
    if(items.empty()) return -1;
    int numItem = items.size();
    float max_value = 0.0;
    int  max_label = 0;
    for(int i = 0; i < numItem; i++)
    {
        if(i == 0)
        {
            max_value = items[0];
            max_label = 0;
        }
        else
        {
            if(items[i] > max_value)
            {
                max_value = items[i];
                max_label = i;
            }
        }
    }
    return max_label;
}

bool methods_compare(ComponentTree* tree1, ComponentTree* tree2)
{
	vector<int> method0_ids1;
	vector<int> method0_ids2;
	vector<int> method1_ids1;
	vector<int> method1_ids2;
	float sum_weights1 = 0.0;
	float sum_weights2 = 0.0;
	if(1) // ILP programming
	{
		if(tree1->width() != tree2->width() || tree1->height() != tree2->height() || tree1->depth() != tree2->depth())
		{
			cerr<<"The two trees with different size. Unalbe to align."<<endl;
			return false;
		}

		// 1. get weights
		vector<float> weights;
		SetStartTime();
		tree1->setWeightMatrix(tree2, weights);
		PrintElapsedTime();
		SetStartTime();
		int numVars1 = (int)tree1->nodeNum();
		int numVars2 = (int)tree2->nodeNum();
		assert((int)weights.size() == numVars1 * numVars2);
		// Forbid the assignment of root node
		weights[numVars1*numVars2 - 1] = 0.0;

		vector<vector<int> > paths1 = tree1->getPaths();
		vector<vector<int> > paths2 = tree2->getPaths();

		// 2. linear model
		lprec *lp;
		int Ncol, *colno=NULL, k;       
		REAL * row = NULL;
		int i=0,j=0;
		Ncol = numVars1 * numVars2; 
		lp = make_lp(0,Ncol);           
		if(lp == NULL) return false;    

		colno = (int *) malloc(Ncol * sizeof(*colno));
		row = (REAL *) malloc(Ncol * sizeof(*row));
		if((colno == NULL) || (row == NULL)) return false;
		for(i=0;i< Ncol;i++) row[i]=1.0; // assign all the content of row as 1

		set_add_rowmode(lp,TRUE);
		set_binary(lp,Ncol,TRUE);

		// add paths1 constraints
		// one path one constraint

		ComponentTree::Paths::iterator it=paths1.begin();
		while(it != paths1.end())
		{
			k=0;
			ComponentTree::Path::iterator itr = (*it).begin();
			while(itr != (*it).end())
			{
				i = (*itr);
				for(j=0;j<numVars2;j++)colno[k++] = i*numVars2+j+1;
				itr++;
			}
			if(!add_constraintex(lp, k, row, colno, LE, 1))
				return false;
			it++;
		}
		// add paths2 constraints
		it=paths2.begin();
		while (it != paths2.end())
		{
			ComponentTree::Path::iterator itr = (*it).begin();
			k=0;
			while(itr != (*it).end())
			{
				j = (*itr);
				for(i=0;i<numVars1;i++)colno[k++] = i*numVars2+j+1;
				itr++;
			}
			if(!add_constraintex(lp, k, row, colno, LE, 1))
				return false;
			it++;
		}
		set_add_rowmode(lp,FALSE);	

		// add the object
		k=0;
		for(i=0;i< numVars1; i++)
			for(j=0; j< numVars2; j++)
			{
				colno[k] = i*numVars2+j+1; //todo: why i*numVar2 + j + 1
				row[k++] = weights[i * numVars2 + j];
			}
		if(!set_obj_fnex(lp, k, row, colno))return false;
		set_maxim(lp);
		set_verbose(lp,IMPORTANT);

		// 3. solve the linear problem
		if(::solve(lp) != OPTIMAL)
		{
			cout<<"Not optimized results"<<endl;
			return false;
		}
		// 4. save results to row
		get_variables(lp,row);
		
		float sum_weights = 0.0;
		for(i = 0; i < numVars1; i++)
		{
			for(int j = 0; j < numVars2; j++)
			{
				if(fabs(row[i * numVars2 + j] - 1.0) < 0.1)
				{
					cout<<"("<<i<<","<<j<<")"<<weights[i*numVars2 + j]<<" ";
					sum_weights += weights[i*numVars2 + j];
					method0_ids1.push_back(i);
					method0_ids2.push_back(j);
				}
			}
		}
		cout<<endl;
		assert(method0_ids1.size() == method0_ids2.size());
		cout<<"match num : "<<method0_ids1.size()<<"  sum of weight : "<<sum_weights<<endl;
		sum_weights1 = sum_weights;

		if(colno != NULL) free(colno);
		if(row != NULL) free(row);
		if(lp != NULL) delete_lp(lp);
		PrintElapsedTime();
	}
	if(true)  // three point condition
	{
		if(tree1->width() != tree2->width() || tree1->height() != tree2->height() || tree1->depth() != tree2->depth())
		{
			cerr<<"The two trees with different size. Unalbe to align."<<endl;
			return false;
		}
		// 1. get weights
		vector<float> weights;
		SetStartTime();
		tree1->setWeightMatrix(tree2, weights);
		PrintElapsedTime();
		SetStartTime();
		//assert((int)weights.size() == numVars1 * numVars2);

		vector<ComponentTree::Node*> nodes1 = tree1->root()->getPostOrderNodes();
		vector<ComponentTree::Node*> nodes2 = tree2->root()->getPostOrderNodes();

		int numVars1 = (int)tree1->nodeNum();
		int numVars2 = (int)tree2->nodeNum();

		vector<float> wvt(numVars1*numVars2, 0.0);  // weight of specific vertex -> subtree
		vector<vector<int> > rev_wvt(numVars1*numVars2, vector<int>());
		vector<float> wtv(numVars1*numVars2, 0.0); // weight of subtree -> specific vertex
		vector<vector<int> > rev_wtv(numVars1*numVars2, vector<int>());
		vector<float> wtt(numVars1*numVars2, 0.0); // weight of tree to tree
		vector<vector<int> > rev_wtt(numVars1*numVars2, vector<int>());

		//calc wvt, wvt[v][u] = max{wvv[v][u] , wvt[v][c1], wvt[v][c2], ...}
		for(int i = 0; i < numVars1; i++)
		{
			ComponentTree::Node* node1 = nodes1[i];
			int label1 = node1->getLabel();
			assert(i == label1);
			for(int j = 0; j < numVars2; j++)
			{
				ComponentTree::Node* node2 = nodes2[j];
				int label2 = node2->getLabel();
				assert(j == label2);
				vector<ComponentTree::Node*> & childs = node2->getChilds(); 
				float max_weight = weights[label1 * numVars2 + label2];
				int max_label = label2;
				vector<ComponentTree::Node*>::iterator it = childs.begin();
				while(it != childs.end())
				{
					int child_label = (*it)->getLabel();
					float child_weight = wvt[label1 * numVars2 + child_label];
					if(child_weight > max_weight)
					{
						max_weight = child_weight;
						max_label = child_label;
					}
					it++;
				}
				wvt[label1*numVars2 + label2] = max_weight;
				if(max_weight > weights[label1 * numVars2 + label2])
				{
					rev_wvt[label1*numVars2 + label2] = rev_wvt[label1*numVars2 + max_label];
					//cout<<label1<<"->"<<rev_wvt[label1*numVars2 + max_label][1]<<"\t";
				}
				else 
				{
					vector<int> match;
					match.push_back(label1);
					match.push_back(label2);
					rev_wvt[label1*numVars2 + label2] = match;
					//cout<<label1<<"->"<<label2<<"\t";
				}
			}
			//cout<<endl;
		}

		//calc wtv, wtv[v][u] = max{wvv[v][u] , wtv[c1][u], wtv[c2][u], ...}
		for(int j = 0; j < numVars2; j++)
		{
			ComponentTree::Node* node2 = nodes2[j];
			int label2 = node2->getLabel();
			for(int i = 0; i < numVars1; i++)
			{
				ComponentTree::Node* node1 = nodes1[i];
				int label1 = node1->getLabel();
				vector<ComponentTree::Node*> & childs = node1->getChilds(); 
				float max_weight = weights[label1 * numVars2 + label2];
				int max_label = label1;
				vector<ComponentTree::Node*>::iterator it = childs.begin();
				while(it != childs.end())
				{
					int child_label = (*it)->getLabel();
					float child_weight = wtv[child_label * numVars2 + label2];
					if(child_weight > max_weight)
					{
						max_weight = child_weight;
						max_label = child_label;
					}
					it++;
				}
				wtv[label1*numVars2 + label2] = max_weight;
				if(max_weight > weights[label1 * numVars2 + label2]) rev_wtv[label1*numVars2 + label2] = rev_wtv[max_label*numVars2 + label2];
				else 
				{
					vector<int> match;
					match.push_back(label1);
					match.push_back(label2);
					rev_wtv[label1*numVars2 + label2] = match;
				}
			}
		}
		// calc wtt
		// wtt[v][u] = max{wvt[v][u], wtv[v][u], bipartite maching between children of v and children of u}
		for(int i = 0; i < numVars1; i++)
		{
			ComponentTree::Node* node1 = nodes1[i];
			int label1 = node1->getLabel();
			vector<ComponentTree::Node*>& childs1 = node1->getChilds();
			int numChilds1 = childs1.size();
			for(int j = 0; j < numVars2; j++)
			{
				ComponentTree::Node* node2 = nodes2[j];
				int label2 = node2->getLabel();
				vector<ComponentTree::Node*>& childs2 = node2->getChilds();
				int numChilds2 = childs2.size(); 
				if(numChilds1 == 0)  // leaf node
				{
					wtt[label1 * numVars2 + label2] = wvt[label1 * numVars2 + label2];
					rev_wtt[label1 * numVars2 + label2] = rev_wvt[label1 * numVars2 + label2];
				}
				else if(numChilds2 == 0)
				{
					wtt[label1 * numVars2 + label2] = wtv[label1 * numVars2 + label2];
					rev_wtt[label1 * numVars2 + label2] = rev_wtv[label1 * numVars2 + label2];
				}
				else
				{
					vector<float> three_weights(3, 0.0);
					vector<vector<int> > three_matches(3, vector<int>());
					three_weights[0] = wvt[label1*numVars2 + label2];
					three_matches[0] = rev_wvt[label1*numVars2 + label2];

					three_weights[1] = wtv[label1*numVars2 + label2];
					three_matches[1] = rev_wtv[label1*numVars2 + label2];
					vector<float> children_weights(numChilds1 * numChilds2, 0.0);
					for(int ii = 0; ii < numChilds1; ii++)
					{
						ComponentTree::Node* child1 = childs1[ii];
						for(int jj = 0; jj < numChilds2;jj++)
						{
							ComponentTree::Node* child2 = childs2[jj];
							children_weights[ii * numChilds2 + jj] = wtt[child1->getLabel() * numVars2 + child2->getLabel()];
						}
					}
					vector<int> ids1, ids2;
					vector<int> match;
					three_weights[2] = bipartite_matching(children_weights, numChilds1, numChilds2, ids1, ids2);
					assert(ids1.size() == ids2.size());
					for(int k = 0; k < (int)ids1.size(); k++)
					{
						int id1 = childs1[ids1[k]]->getLabel();
						int id2 = childs2[ids2[k]]->getLabel();
						vector<int>& child_match = rev_wtt[id1 * numVars2 + id2];
						match.insert(match.end(), child_match.begin(), child_match.end());
					}
					three_matches[2] = match;
					int which_item = max_item(three_weights);
					wtt[label1*numVars2 + label2] = three_weights[which_item];
					rev_wtt[label1*numVars2 + label2] = three_matches[which_item];
				}
			}
		}
	
		vector<int>& match = rev_wtt.back();
		//cout<<"match num : "<<match.size()<<endl;
		assert(match.size() % 2 == 0);
		int match_num = match.size() / 2;
		for(int k = 0; k < match_num; k++)
		{
			int i = match[2*k];
			int j = match[2*k + 1];
			cout<<"("<<i<<","<<j<<")"<<weights[i * numVars2 + j]<<" ";
			method1_ids1.push_back(i);
			method1_ids2.push_back(j);
		}
		cout<<endl;
		assert(method1_ids1.size() == method1_ids2.size());
		cout<<"match num : "<<method1_ids1.size()<<"  sum of weight : "<<wtt.back()<<endl;	
		PrintElapsedTime();
		sum_weights2 = wtt.back();
		float percent = sum_weights2 / sum_weights1;
		cout<<"weight percentage : "<<percent<<endl;
	}

	cout<<endl<<"============================ overlap ============================"<<endl;
	if(true)
	{
		//================= calculate the overlap ======================//
		vector<float> weights;
		tree1->setWeightMatrix(tree1, weights);
		int numVars1 = tree1->nodeNum();
		int numVars2 = tree1->nodeNum();

		vector<float> weights1;  // method0_ids1 <==> method1_ids1
		int numIds1 = method0_ids1.size();
		int numIds2 = method1_ids1.size();
		weights1.resize(numIds1 * numIds2);
		vector<int> ids1;  // store the matching results
		vector<int> ids2;  // same as above
		//int non_zero_num = 0;
		for(int i = 0; i < numIds1; i++)
		{
			int label1 = method0_ids1[i];
			for(int j = 0; j < numIds2; j++)
			{
				int label2 = method1_ids1[j];
				weights1[i * numIds2 + j ] = weights[label1 * numVars2 + label2];
			}
		}
		float total_weight = bipartite_matching(weights1, numIds1, numIds2, ids1, ids2);
		cout<<"overlap of tree1's two output : "<<endl;
		//float mean_weight = total_weight / ids1.size();
		for(int i = 0; i < ids1.size(); i++)
		{
			int label1 = method0_ids1[ids1[i]];
			int label2 = method1_ids1[ids2[i]];
			cout<<"("<<label1<<","<<label2<<")"<<weights[label1 * numVars2 + label2]<<" ";
			//if(weights[label1 * numVars2 + label2] > 0.0f) non_zero_num++;
		}
		cout<<endl;
		assert(ids1.size() == ids2.size());
		float mean_weight = total_weight / ids1.size();
		//float mean_weight2 = total_weight / non_zero_num;
		cout<<"match num : "<<ids1.size()<<"  mean weight : "<<mean_weight<<endl;
		//cout<<"match num (without zero): "<<non_zero_num<<"  mean weight : "<<mean_weight2<<endl;
	}
	if(true)
	{
		//================= calculate the overlap ======================//
		vector<float> weights;
		tree2->setWeightMatrix(tree2, weights);
		int numVars1 = tree2->nodeNum();
		int numVars2 = tree2->nodeNum();

		vector<float> weights1;  // method0_ids1 <==> method1_ids1
		int numIds1 = method0_ids2.size();
		int numIds2 = method1_ids2.size();
		weights1.resize(numIds1 * numIds2);
		vector<int> ids1;  // store the matching results
		vector<int> ids2;  // same as above
		//int non_zero_num = 0;
		for(int i = 0; i < numIds1; i++)
		{
			int label1 = method0_ids1[i];
			for(int j = 0; j < numIds2; j++)
			{
				int label2 = method1_ids1[j];
				weights1[i * numIds2 + j ] = weights[label1 * numVars2 + label2];
			}
		}
		float total_weight = bipartite_matching(weights1, numIds1, numIds2, ids1, ids2);
		cout<<"overlap of tree2's two output : "<<endl;
		for(int i = 0; i < ids1.size(); i++)
		{
			int label1 = method0_ids1[ids1[i]];
			int label2 = method1_ids1[ids2[i]];
			//if(weights[label1 * numVars2 + label2 ] > 0.0f) non_zero_num++;
			cout<<"("<<label1<<","<<label2<<")"<<weights[label1 * numVars2 + label2]<<" ";
		}
		cout<<endl;
		assert(ids1.size() == ids2.size());
		float mean_weight = total_weight / ids1.size();
		//float mean_weight2 = total_weight / non_zero_num;
		cout<<"match num : "<<ids1.size()<<"  mean weight : "<<mean_weight<<endl;
		//cout<<"match num (without zero): "<<non_zero_num<<"  mean weight : "<<mean_weight2<<endl;
	}
	return true;
}
