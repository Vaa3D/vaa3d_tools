#ifndef __HEAP_SORT_H__
#define __HEAP_SORT_H__

#include <cassert>

struct HeapElem
{
	int    heap_id;
	long   img_ind;
	double value;
	HeapElem(long _ind, double _value)
	{
		heap_id = -1;
		img_ind = _ind;
		value = _value;
	}
};

struct HeapElemX : public HeapElem
{
	long prev_ind;  // previous img ind
	HeapElemX(long _ind, double _value) : HeapElem(_ind, _value){prev_ind = -1;}
};

template <class T>
class BasicHeap       // Basic Min heap
{
public:
	BasicHeap()
	{
		elems.reserve(10000);
	}
	T* delete_min()
	{
		if(elems.empty()) return 0;
		T* min_elem = elems[0];

		if(elems.size()==1) elems.clear();
		else
		{
			elems[0] = elems[elems.size() - 1];
			elems[0]->heap_id = 0;
			elems.erase(elems.begin() + elems.size() - 1);
			down_heap(0);
		}
		return min_elem;
	}
	void insert(T* t)
	{
		elems.push_back(t);
		t->heap_id = elems.size() - 1;
		up_heap(t->heap_id);
	}
	bool empty(){return elems.empty();}
	void adjust(int id, double new_value)
	{
		double old_value = elems[id]->value;
		elems[id]->value = new_value;
		if(new_value < old_value) up_heap(id);
		else if(new_value > old_value) down_heap(id);
	}
private:
	vector<T*> elems;
	bool swap_heap(int id1, int id2)
	{
		if(id1 < 0 || id1 >= elems.size() || id2 < 0 || id2 >= elems.size()) return false;
		if(id1 == id2) return false;
		int pid = id1 < id2 ? id1 : id2;
		int cid = id1 > id2 ? id1 : id2;
		assert(cid == 2*(pid+1) -1 || cid == 2*(pid+1));

		if(elems[pid]->value <= elems[cid]->value) return false;
		else
		{
			T * tmp = elems[pid];
			elems[pid] = elems[cid];
			elems[cid] = tmp;
			elems[pid]->heap_id = pid;
			elems[cid]->heap_id = cid;
			return true;
		}
	}
	void up_heap(int id)
	{
		int pid = (id+1)/2 - 1;
		if(swap_heap(id, pid)) up_heap(pid);
	}
	void down_heap(int id)
	{
		int cid1 = 2*(id+1) -1;
		int cid2 = 2*(id+1);
		if(cid1 >= elems.size()) return;
		else if(cid1 == elems.size() - 1)
		{
			swap_heap(id, cid1);
		}
		else if(cid1 < elems.size() - 1)
		{
			int cid = elems[cid1]->value < elems[cid2]->value ? cid1 : cid2;
			if(swap_heap(id, cid)) down_heap(cid);
		}
	}
};

#endif
