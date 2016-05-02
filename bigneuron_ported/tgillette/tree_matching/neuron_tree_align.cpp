#include "neuron_tree_align.h"


/**
 * Tree vectors are expected to be ordered such that a branch always comes after all of its children, with the root at the back
 * Weights are positive, neuron_tree_align maximizing the total score
 **/
template<class T> double neuron_tree_align(vector<T*> &tree1, vector<T*> &tree2, vector<double> & w, vector<pair<int, int> > & result)
{
    // Indices of branches, sets giving branch merges with a child, sets 1ll<<child_num1 and 1ll<<child_num2 giving max score for pair
    // weights gives scores for a branch pair and each branch's possible merge with one of its children
    map<SetIndexPair, double> weights;
    // results contains a vector of branch index pairs giving all aligned branch pairs
	map<SetIndexPair, vector<pair<int, int> > >  results;

	int nrows = tree1.size();
	int ncols = tree2.size();

//    printf("nrows %i, ncols %i\n",nrows,ncols);
    
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

//            printf("Node1 childlist size %i, node2 childlist size %i, weight %f\n",child_num1,child_num2, w[ind1 * ncols + ind2]);
            
			SetIndexPair sip;
			sip.ind1 = ind1;
			sip.ind2 = ind2;

			// Compare all the subset align
            // Each set gives a combination of the branch of ind1 with one of its children, position 0 giving the branch alone without its children
			for(long set1 = 0; set1 < (1ll<<child_num1); set1++)
			{
				sip.set1 = set1;
				vector<int> set1_bits; // all bits index of set1 with value 1
                // Determines which children are available for matching for the current set
                // set = 1 -> child 0; set = 2 -> child 1; set = 3 -> children 0 and 1
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

                    // When set1 == 0 but set2 > 0 max_sip1 and max_sip2 will be default initialized: ind1=0, ind2=0, set1=0, set2=0
                    //      weights will have a value of 0 (zero-initialized), results will have empty vector of pair<int,int>
                    // When set1 > 0 and set2 == 0, _sip1 gets set, but nothing else happens in terms of max_val, max_sip1, and max_sip2 - same as above
                    //      But then why run this if either set1 or set2 are 0? Seems like wasted operations.
                    
                    // Picking best assignment of children via looking at all possible assignments
					// first item
					for(long i = 0; i < set1_bits.size(); i++)
					{
                        // The bit gives which child to take;
                        // Set=0: no children, Set=1: child 0, Set=2: child 1, Set=3 (if >1 children): children 0 and 1
						long b1 = set1_bits[i];
						SetIndexPair _sip1; // Match
						_sip1.ind1 = ind1;
                        // For set in 1,2 returns 0, if set=3 gets the inverse set number associated with the bit (0:1->2, 1:2->1)
                        // Ex: Set=1: 01 & ~01 = 01&10 = 0;
                        // Ex: Set=3, b1 = 0: 11 & ~01 = 11 & 10 = 2 (b1 assoc set=1, get set=2)
                        // Ex: Set=3, b1 = 1: 11 & ~10 = 11 & 01 = 1 (b1 assoc set=2, get set=1)
						_sip1.set1 = set1 & (~(1ll<<b1));
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
                            // weights[_sip1] is 0 if set is 1 or 2;
                            // If set is 3, weights[_sip1] gives weight of opposite child pair match, weights[_sip2] of previous set iteration
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
			/* calcute wTT - Here is where joining of parent and child segments is made possible */
			{
                // For storing the optimal match case with set values above those from previous section
				sip.set1 = 1ll << child_num1;
				sip.set2 = 1ll << child_num2;
				double max_val = -1.0;
				SetIndexPair max_sip;
				// wTT rule 3: initialize max_sip with the last/optimal set from previous child matching section
				max_sip.ind1 = ind1;
				max_sip.ind2 = ind2;
				max_sip.set1 = (1ll << child_num1) - 1;
				max_sip.set2 = (1ll << child_num2) - 1;
				max_val = weights[max_sip];
				// wTT rule 1 - See if best match of node1's children with node 2 is better than child matchings
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
				// wTT rule 2 - See if best match of node2's children with node 1 is better than prev matchings
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

                // Weight of segment 1 and segment 2 + weight of child connections
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

bool neuron_mapping_dynamic(vector<MyMarker*> &inswc1, vector<MyMarker*> & inswc2, vector<map<MyMarker*, MyMarker*> > & result_map) //result_map[i] is the mapping result of the ith segment
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
