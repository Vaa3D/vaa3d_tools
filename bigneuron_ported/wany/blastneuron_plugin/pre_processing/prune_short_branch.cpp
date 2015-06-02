#include "prune_short_branch.h"
#include <math.h>
#define DIST(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))


bool prune_branch(NeuronTree nt, NeuronTree & result)
{
	double thres = 0.05;

	V3DLONG siz = nt.listNeuron.size();
	vector<V3DLONG> branches(siz,0); //number of branches on the pnt: 0-tip, 1-internal, >=2-branch
	for (V3DLONG i=0;i<siz;i++)
	{
		if (nt.listNeuron[i].pn<0) continue;
		V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
		branches[pid]++;
	}
	
	double diameter = calculate_diameter(nt, branches);
	printf("diameter=%.3f\n",diameter);
	

	//calculate the shortest edge starting from each tip point
	vector<bool> to_prune(siz, false);
	for (V3DLONG i=0;i<siz;i++)
	{
		if (branches[i]!=0) continue;
		//only consider tip points
		vector<V3DLONG> segment;
		double edge_length = 0;
		V3DLONG cur = i;
		V3DLONG pid;
		do
		{
			NeuronSWC s = nt.listNeuron[cur];
			segment.push_back(cur);
			pid = nt.hashNeuron.value(s.pn);
			edge_length += DIST(s, nt.listNeuron[pid]);
			cur = pid;
		}
		while (branches[pid]==1 && pid>0);
		if (pid<0)
		{
			printf("The input tree has only 1 root point. Please check.\n");
			return false;
		}
		if (edge_length < diameter * thres)
		{
			for (int j=0;j<segment.size();j++)
				to_prune[segment[j]] = true;
		}
	}



	//prune branches
	result.listNeuron.clear();
	result.hashNeuron.clear();
	for (V3DLONG i=0;i<siz;i++)
	{
		if (!to_prune[i])
		{
			NeuronSWC s = nt.listNeuron[i];
			result.listNeuron.append(s);
			result.hashNeuron.insert(nt.listNeuron[i].n, result.listNeuron.size()-1);
		}
	}

	return true;
}

//the diameter of a tree is defined as the longest path between every pair of leafs in a rootless tree
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches)
{
	V3DLONG siz = nt.listNeuron.size();
	vector<vector<double> > longest_path(siz, vector<double>(2,0));//the 1st and 2nd longest path to each node in a rooted tree
	vector<V3DLONG> chd(siz, -1);//immediate child of the current longest path
	for (V3DLONG i=0;i<siz;i++)
	{
		if (branches[i]!=0) continue;
		V3DLONG cur = i;
		V3DLONG pid;
		do
		{
			NeuronSWC s = nt.listNeuron[cur];
			pid = nt.hashNeuron.value(s.pn);
			double dist = DIST(s, nt.listNeuron[pid]) + longest_path[cur][0];
			if (dist>longest_path[pid][0])
			{
				chd[pid] = cur;
				longest_path[pid][0] = dist;
			}
			else if (dist>longest_path[pid][1] && chd[pid]!=cur)
				longest_path[pid][1] = dist;
			cur = pid;
		}
		while (branches[cur]!=0 && pid>0);
	}

	double diam = -1;
	for (V3DLONG i=0;i<siz;i++)
	{
		if (longest_path[i][0] + longest_path[i][1]>diam)
			diam = longest_path[i][0]+longest_path[i][1];
	}
	return diam;
}


bool export_listNeuron_2swc(QList<NeuronSWC> & list, const char* filename)
{
	FILE * fp;
	fp = fopen(filename,"w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",filename);
		return false;
	}
	fprintf(fp,"##n,type,x,y,z,radius,parent\n");
	for (int i=0;i<list.size();i++)
	{
		NeuronSWC curr = list.at(i);
		fprintf(fp,"%d %d %.2f %.2f %.2f %.2f %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn);
	}
	fclose(fp);
	return true;
}
