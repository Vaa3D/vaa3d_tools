#include "swc_convert.h"

vector<MyMarker*> swc_convert(NeuronTree & nt)
{
	if(nt.listNeuron.empty()) return vector<MyMarker*>();

	long num = nt.listNeuron.size();
	vector<MyMarker*> outswc(num, 0);
	for(long i = 0; i < num; i++)
	{
		const NeuronSWC & ns = nt.listNeuron.at(i);
		MyMarker * marker = new MyMarker(ns.x, ns.y, ns.z);
		marker->radius = ns.r;
		outswc[i] = marker;
	}
	for(long i = 0; i < num; i++)
	{
		MyMarker * cur_marker = outswc[i];
		const NeuronSWC & ns = nt.listNeuron.at(i);
		if(ns.pn == -1) cur_marker->parent = 0;
		else
		{
			long pid = nt.hashNeuron.value(ns.pn);
			if(pid < num) cur_marker->parent = outswc[pid];
			else cerr<<"pid >= num error"<<endl;
		}
	}
	return outswc;
}

NeuronTree swc_convert(vector<MyMarker*> & inswc)
{
	if(inswc.empty()) return NeuronTree();
	map<MyMarker*, long> swc_map;
	long num = inswc.size();
	NeuronTree nt;
	for(long i = 0; i < num; i++)
	{
		MyMarker * marker = inswc[i];
		swc_map[marker] = i+1;
	}
	for(long i = 0; i < num; i++)
	{
		MyMarker * marker = inswc[i];
		long pid = -1;
		if(marker->parent) pid = swc_map[marker->parent];

		NeuronSWC ns;
		ns.x = marker->x;
		ns.y = marker->y;
		ns.z = marker->z;
		ns.r = marker->radius;
		ns.n = swc_map[marker];
		ns.pn = pid;
		nt.listNeuron.push_back(ns);
	}
	for(long i = 0; i < num; i++)
	{
		nt.hashNeuron[i] = i+1;
	}
	return nt;
}
