#ifndef __NEURON_TREE_ALIGN_H__
#define __NEURON_TREE_ALIGN_H__
#include "swc_utils.h"
#include "seg_weight.h"
#include <algorithm>

// make sure tree1 and tree2 are bottom up order
struct SetIndexPair
{
	short int ind1;
	short int ind2;
	short int set1; // 0,1,2,3
	short int set2; // 0,1,2,3
	bool operator<(const SetIndexPair & others) const
	{ 
		if (ind1<others.ind1) return true;
		if (ind1>others.ind1) return false;
		if (ind2<others.ind2) return true;
		if (ind2>others.ind2) return false;
		if (set1<others.set1) return true;
		if (set1>others.set1) return false;
		if (set2<others.set2) return true;
		if (set2>others.set2) return false;
		return false;
	}
};


template<class T> double neuron_tree_align(vector<T*> &tree1, vector<T*> &tree2, vector<double> & w, vector<pair<int, int> > & result)
{
	map<SetIndexPair, double> weights;
	map<SetIndexPair, vector<pair<int, int> > >  results;

	int nrows = tree1.size();
	int ncols = tree2.size();

	map<T*, long> ind_map1, ind_map2;
	for(long ind1 = 0; ind1 < tree1.size(); ind1++) ind_map1[tree1[ind1]] = ind1;
	for(long ind2 = 0; ind2 < tree2.size(); ind2++) ind_map2[tree2[ind2]] = ind2;

	for(long ind1 = 0; ind1 < tree1.size(); ind1++)
	{
		T * node1 = tree1[ind1];
		int child_num1 = node1->child_list.size();
		for(long ind2 = 0; ind2 < tree2.size(); ind2++)
		{
			T * node2 = tree2[ind2];
			int child_num2 = node2->child_list.size();

			SetIndexPair sip;
			sip.ind1 = ind1;
			sip.ind2 = ind2;

			// compare all the subset align
			for(long set1 = 0; set1 < (1ll<<child_num1); set1++)
			{
				sip.set1 = set1;
				vector<int> set1_bits; // all bits index of set1 with value 1
				for(int b = 0; b < child_num1; b++) if(set1 & (1ll << b)) set1_bits.push_back(b);

				for(long set2 = 0; set2 < (1ll<<child_num2); set2++)
				{
					sip.set2 = set2;
					vector<int> set2_bits; // all bits index of set2 with value 1
					for(int b = 0; b < child_num2; b++) if(set2 & (1ll << b)) set2_bits.push_back(b);

					if(set1 == 0 && set2 == 0) 
					{
						weights[sip] = 0.0;
						results[sip] = vector<pair<int, int> >();
						continue; 
					}

					double max_val = 0.0;
					SetIndexPair max_sip1, max_sip2;

					// first item
					for(long i = 0; i < set1_bits.size(); i++)
					{
						long b1 = set1_bits[i];
						SetIndexPair _sip1;
						_sip1.ind1 = ind1;
						_sip1.set1 = set1 & (~(1ll<<b1));            // clear bit b1
						_sip1.ind2 = ind2;

						for(long j = 0; j < set2_bits.size(); j++)
						{
							long b2 = set2_bits[j];
							_sip1.set2 = set2 & (~(1ll<<b2));        // clear bit j

							SetIndexPair _sip2;                   // A - {a_i}, B - {b_j}
							_sip2.ind1 = ind_map1[node1->child_list[b1]];                   // the index of  C(a_i)
							_sip2.ind2 = ind_map2[node2->child_list[b2]];                   // the index of  C(a_i)
							_sip2.set1 = 1ll << tree1[_sip2.ind1]->child_list.size();
							_sip2.set2 = 1ll << tree2[_sip2.ind2]->child_list.size();
							double val = weights[_sip1] + weights[_sip2];
							if(val > max_val)
							{
								max_val = val;
								max_sip1 = _sip1;
								max_sip2 = _sip2;
							}
						}
					}

					weights[sip] = max_val;
					results[sip].insert(results[sip].begin(), results[max_sip1].begin(), results[max_sip1].end());
					results[sip].insert(results[sip].begin(), results[max_sip2].begin(), results[max_sip2].end());
				}
			}
			// calcute wTT
			{
				sip.set1 = 1ll << child_num1;
				sip.set2 = 1ll << child_num2;
				double max_val = -1.0;
				SetIndexPair max_sip;
				// wTT rule 3
				max_sip.ind1 = ind1;
				max_sip.ind2 = ind2;
				max_sip.set1 = (1ll << child_num1) - 1;
				max_sip.set2 = (1ll << child_num2) - 1;
				max_val = weights[max_sip];
				// wTT rule 1
				for(int ai = 0; ai < child_num1; ai++)
				{
					SetIndexPair _sip;
					_sip.ind1 = ind_map1[node1->child_list[ai]];
					_sip.set1 = 1ll << tree1[_sip.ind1]->child_list.size();
					_sip.ind2 = ind2;
					_sip.set2 = 1ll << tree2[_sip.ind2]->child_list.size();
					double val = weights[_sip];
					if(val > max_val)
					{
						max_val = val;
						max_sip = _sip;
					}
				}
				// wTT rule 2
				for(int bj = 0; bj < child_num2; bj++)
				{
					SetIndexPair _sip;
					_sip.ind1 = ind1;
					_sip.set1 = 1ll << tree1[_sip.ind1]->child_list.size();
					_sip.ind2 = ind_map2[node2->child_list[bj]];
					_sip.set2 = 1ll << tree2[_sip.ind2]->child_list.size();
					double val = weights[_sip];
					if(val > max_val)
					{
						max_val = val;
						max_sip = _sip;
					}
				}

				weights[sip] = w[ind1 * ncols + ind2] + max_val;
				results[sip] = results[max_sip];
				results[sip].push_back(pair<long, long>(ind1, ind2));
			}
		}
	}
	SetIndexPair root_sip;
	root_sip.ind1 = tree1.size() - 1;
	root_sip.ind2 = tree2.size() - 1;
	root_sip.set1 = 1ll << tree1[root_sip.ind1]->child_list.size();
	root_sip.set2 = 1ll << tree2[root_sip.ind2]->child_list.size();
	result = results[root_sip];
	return weights[root_sip];
}

void merge_multi_match(vector<pair<int, int> > & result, vector<vector<int> > & pairs_merged1, vector<vector<int> > & pairs_merged2)
{
	map<int, int> cluster1, cluster2; //map from value to cluster value
	vector<int> class_nums;
	int cluster_num = 0;
	for (int i=0;i<result.size();i++)
	{
		cluster1[result[i].first] = -1;
		cluster2[result[i].second] = -1;
	}

	for (int i=0;i<result.size();i++)
	{
		int s1 = result[i].first;
		int s2 = result[i].second;
		if (cluster1[s1]<0 && cluster2[s2]<0)
		{
			cluster1[s1] = cluster_num;
			cluster2[s2] = cluster_num;
			class_nums.push_back(cluster_num);
			cluster_num++;
		}
		else if (cluster1[s1]<0 && cluster2[s2]>=0)
			cluster1[s1] = cluster2[s2];
		else if (cluster1[s1]>=0 && cluster2[s2]<0)
			cluster2[s2] = cluster1[s1];
		else
		{
			if (cluster1[s1] != cluster2[s2])
			{
				int cl1 = cluster1[s1];
				int cl2 = cluster2[s2];
				class_nums.erase(remove(class_nums.begin(), class_nums.end(), cl1), class_nums.end());
				class_nums.erase(remove(class_nums.begin(), class_nums.end(), cl2), class_nums.end());
				for (map<int, int>::iterator it=cluster1.begin(); it!=cluster1.end(); it++)
				{
					if ((*it).second==cl1)
						cluster1[(*it).first] = cluster_num;
				}
				for (map<int, int>::iterator it=cluster2.begin(); it!=cluster2.end(); it++)
				{
					if ((*it).second==cl2)
						cluster2[(*it).first] = cluster_num;
				}
				class_nums.push_back(cluster_num);
				cluster_num++;
			}
}
	}
	for (int i=0;i<class_nums.size();i++)
	{
		int cur_class = class_nums[i];
		vector<int> c1, c2;
		for (map<int, int>::iterator it=cluster1.begin(); it!=cluster1.end(); it++)
			if ((*it).second==cur_class)
				c1.push_back((*it).first);
		for (map<int, int>::iterator it=cluster2.begin(); it!=cluster2.end(); it++)
			if ((*it).second==cur_class)
				c2.push_back((*it).first);
		pairs_merged1.push_back(c1);
		pairs_merged2.push_back(c2);
	}

}

bool neuron_mapping_dynamic(vector<MyMarker*> &inswc1, vector<MyMarker*> & inswc2, 
		vector<map<MyMarker*, MyMarker*> > & result_map) //result_map[i] is the mapping result of the ith segment
{
	vector<NeuronSegment*> final_tree1, final_tree2;
	double max_score = -1;
	vector<pair<int, int> > result;
	
	//find longest path in the two trees
	vector<MyMarker*> ends1, ends2;
	get_far_ends(inswc1, ends1);
	get_far_ends(inswc2, ends2);
	
//	for (int iti=0; iti<2;iti++)
//		for (int itj=0;itj<2;itj++)
//		{
			vector<NeuronSegment*> tree1, tree2;
//			reroot(inswc1, ends1[iti]);
//			reroot(inswc2, ends2[itj]);
			swc_to_segments(inswc1, tree1); // convert to post order
			swc_to_segments(inswc2, tree2); // convert to post order
			cout<<"convert to post order done"<<endl;
			cout<<"tree 1 seg num: "<<tree1.size()<<endl;
			cout<<"tree 2 seg num: "<<tree2.size()<<endl;

			int nrows = tree1.size();
			int ncols = tree2.size();
			vector<double> weights(nrows * ncols, 0.0);

			double max_weight = 0;
			for(int i = 0; i < nrows; i++)
			{
				for(int j = 0; j < ncols; j++)
				{
					//vector<MyMarker*> seg1 = tree1[i]->markers;
					//vector<MyMarker*> seg2 = tree2[i]->markers;
					weights[i*ncols + j] = seg_dist(tree1[i]->markers, tree2[j]->markers);
					if (weights[i*ncols + j] >  max_weight)
						max_weight = weights[i*ncols + j];
				}
			}
			for (int i=0;i<nrows*ncols; i++)
				weights[i] = max_weight - weights[i];
			cout<<"weight calculation done"<<endl;
			vector<pair<int, int> > cur_result;

			double score = neuron_tree_align(tree1, tree2, weights, cur_result);
			if (score >= max_score)
			{
				max_score = score;
				result = cur_result;
				for (int i=0;i<final_tree1.size();i++)
					if (final_tree1[i]) { delete(final_tree1[i]); final_tree1[i] = NULL;}
				for (int i=0;i<final_tree2.size();i++)
					if (final_tree2[i]) { delete(final_tree2[i]); final_tree2[i] = NULL;}
				final_tree1 = tree1;
				final_tree2 = tree2;
			}
			else
			{
				for (int i=0;i<tree1.size();i++)
					if (tree1[i]) { delete(tree1[i]); tree1[i] = NULL;}
				for (int i=0;i<tree2.size();i++)
					if (tree2[i]) { delete(tree2[i]); tree2[i] = NULL;}
			}
			cout<<"tree align done"<<endl;
			cout<<"score="<<score<<endl;
			cout<<"result size = "<<cur_result.size()<<endl;
//		}

	//merge multiple matches
	vector<pair<vector<MyMarker*>, vector<MyMarker*> > > pairs_merged;
	vector<vector<int> > seg_clusters1, seg_clusters2;
	merge_multi_match(result, seg_clusters1, seg_clusters2);
	cout<<"cluster num: "<<seg_clusters1.size()<<endl;
	for (int i=0;i<seg_clusters1.size();i++)
	{
		vector<MyMarker*> new_seg1, new_seg2;
		vector<MyMarker*> tmp_seg;
		//the clustered segments should be in increasing order, so just concatinate them
		for (int j=0;j<seg_clusters1[i].size();j++)
		{
			tmp_seg.clear();
			tmp_seg = final_tree1[seg_clusters1[i][j]]->markers;
			for (int k=0;k<tmp_seg.size();k++)
				new_seg1.push_back(tmp_seg[k]);
		}
		for (int j=0;j<seg_clusters2[i].size();j++)
		{
			tmp_seg.clear();
			tmp_seg = final_tree2[seg_clusters2[i][j]]->markers;
			for (int k=0;k<tmp_seg.size();k++)
				new_seg2.push_back(tmp_seg[k]);
		}
		pairs_merged.push_back(pair<vector<MyMarker*>, vector<MyMarker*> >(new_seg1, new_seg2));
	}
	cout<<"merge multi match done"<<endl;


	//affine curve alignment

	for (int i=0;i<pairs_merged.size();i++)
	{
		vector<pair<int, int> > seg_res;
		vector<MyMarker*> seg1 = pairs_merged[i].first;
		vector<MyMarker*> seg2 = pairs_merged[i].second;
		seg_weight(seg1, seg2, seg_res);
		map<MyMarker*, MyMarker*> map_segment;
		//seg_dist_lpsolve((final_tree1[result[i].first])->markers, (final_tree2[result[i].second])->markers, seg_res);
		for (int j=0;j<seg_res.size();j++)
		{
			map_segment[seg1[seg_res[j].first]] = seg2[seg_res[j].second];
		}
		result_map.push_back(map_segment);
	}
	cout<<"save to file done"<<endl;

	for (int i=0;i<final_tree1.size();i++)
		if (final_tree1[i]) { delete(final_tree1[i]); final_tree1[i] = NULL;}
	for (int i=0;i<final_tree2.size();i++)
		if (final_tree2[i]) { delete(final_tree2[i]); final_tree2[i] = NULL;}
	return true;
}

void convert_matchmap_2swc(vector<map<MyMarker*, MyMarker*> > & inmap, vector<MyMarker*> & outswc)
{
	int color_interval = floor(256.0 / inmap.size());
	int color = 20;
	for (int i=0;i<inmap.size();i++)
	{
		for (map<MyMarker*, MyMarker*>::iterator it=inmap[i].begin(); it!=inmap[i].end(); it++)
		{
			MyMarker * p_src = new MyMarker(*(*it).first);
			MyMarker * p_tgt = new MyMarker(*(*it).second);
			p_src->type = color;
			p_tgt->type = color;
			p_src->parent = NULL;
			p_tgt->parent = p_src;
			p_src->radius = 1.0;
			p_tgt->radius = 1.0;
			outswc.push_back(p_src);
			outswc.push_back(p_tgt);
		}
		color += color_interval;
	}
}

#endif
