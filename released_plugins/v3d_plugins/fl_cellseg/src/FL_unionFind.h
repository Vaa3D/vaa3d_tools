// define union find class
// F. Long
// 20080508

#ifndef __FL_UNION_FIND__
#define __FL_UNION_FIND__

#define LENGTH 100
#define EXTENDLENGTHFACTOR 2 

#include "FL_defType.h"

class UnionFind 
{
private:

    V3DLONG *its_idd;
    V3DLONG *its_sz;
    V3DLONG its_num_sets;
    V3DLONG its_allocated_length;
    V3DLONG its_num_nodes;	
    V3DLONG its_finalized;
	
public:

	// constructor
	UnionFind(V3DLONG len)
	{
		if (len <= 0)
		{
			len = LENGTH;
		}

		its_idd = new V3DLONG [len]; 
		its_sz = new V3DLONG [len];
		its_allocated_length = len;
		its_num_nodes = 0;
		its_num_sets = 0;
		its_finalized = false;
	}
		
	// new a node in the tree
	void unionFindNewNode()
	{
		if (its_num_nodes >= its_allocated_length)
		{
			V3DLONG allocated_length_new = its_allocated_length * EXTENDLENGTHFACTOR;
						
			V3DLONG *idd1 = new V3DLONG [allocated_length_new]; 
			V3DLONG *sz1 = new V3DLONG [allocated_length_new];
				
			V3DLONG i;
			
			for (i=0;i< its_allocated_length; i++)
			{
				idd1[i] = its_idd[i];
				sz1[i] = its_sz[i];
			}			
			
			V3DLONG *iddtmp, *sztmp;
			iddtmp = its_idd; 
			sztmp = its_sz;

			its_idd = idd1; // now both its_idd and idd1 point to the same address
			its_sz = sz1; // now both its_sz and sz1 point to the same address
			
			if (iddtmp) {delete []iddtmp; iddtmp = 0;}
			if (sztmp) {delete [] sztmp; sztmp = 0;}
						
			its_allocated_length = allocated_length_new;
			
		}

		its_num_nodes++;
		its_idd[its_num_nodes - 1] = its_num_nodes - 1;
		its_sz[its_num_nodes - 1] = 1;
	}
	
	V3DLONG unionFindSearch(V3DLONG p)
	{
		V3DLONG i;
		V3DLONG t;
		V3DLONG *idd1 = its_idd;
		
		for (i = p; i != idd1[i]; i = idd1[i])
		{
			t = i;
			i = idd1[idd1[t]];
			idd1[t] = i;
		}

		return i;
	}
	
	void unionFindUnion(V3DLONG p, V3DLONG q)
	{
		if (its_sz[p] < its_sz[q])
		{
			its_idd[p] = q;
			its_sz[q] += its_sz[p];
		}
		else
		{
			its_idd[q] = p;
			its_sz[p] += its_sz[q];
		}
	}

	
	void unionFindMergePair(V3DLONG p, V3DLONG q)
	{
		V3DLONG i;
		V3DLONG j;

		i = unionFindSearch(p);
		j = unionFindSearch(q);
		if (i != j)
		{
			unionFindUnion(i, j);
		}
	}
	
	// renumber 
	V3DLONG unionFindEnumerate(V3DLONG first)
	{
		V3DLONG k;
		V3DLONG counter = first;

		its_finalized = true;

		for (k = 0; k < its_num_nodes; k++)
		{
			if (its_idd[k] == k)
			{
				its_sz[k] = counter++;
			}
		}

		its_num_sets = counter - first;

		return its_num_sets;
	}
	
	V3DLONG unionFindGetSet(V3DLONG p)
	{
		V3DLONG k;
//		V3DLONG *idd = idd;

		k = unionFindSearch(p);
		return its_sz[k];
	}
	
	~UnionFind()
	{
		if (its_idd) { delete [] its_idd; its_idd = 0;}
		if (its_sz)  { delete [] its_sz; its_sz = 0;}
	}	
};

#endif 

