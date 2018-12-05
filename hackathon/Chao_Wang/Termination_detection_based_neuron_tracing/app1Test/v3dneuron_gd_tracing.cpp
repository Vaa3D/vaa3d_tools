//v3dneuron_tracing_shortestpath.cpp
// by Hanchuan Peng
// 2008-2010
// separated from main V3D program on May 24, 2010 for command line program. by Hanchuan Peng

#include "v3dneuron_gd_tracing.h"
#include "tip_detection.h"
#include "FL_bwdist.h"

#include <deque>

#include "stackutil.h"

//#define __USE_HIERARCHICAL_PRUNING__
#ifdef __USE_HIERARCHICAL_PRUNING__
#include "hp.h"
#endif

#define CHECK_DATA_GD_TRACING(AA) \
	if (!p4d || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 ) \
	{ \
		v3d_msg("Invalid image and sz parameters.\n", 0); \
		throw("Invalid image and sz parameters.\n"); \
		return AA; \
	} 

#define _CHECK_PRUNING_PARAMETERS_() \
    V3DLONG npruned = 0; \
    if (mmUnit.size()<=0 || mmUnit[0].size()<=0 || !imap || !dmap || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0) \
    { \
        v3d_msg("Wrong parameters in a pruning function.\n", 0); \
        return npruned; \
    } 

#define TRACED_NAME "APP1_Tracing"
#define TRACED_FILE "v3d_traced_neuron"



NeuronTree v3dneuron_GD_tracing(unsigned char ****p4d, V3DLONG sz[4], 
								LocationSimple & p0, vector<LocationSimple> & pp, 
								CurveTracePara & trace_para, double trace_z_thickness) 
{
	NeuronTree nt;
	if (!p4d || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || trace_para.channo<0 || trace_para.channo>=sz[3])
	{
		fprintf(stderr, "Invalid image or sz for v3dneuron_GD_tracing().\n");
		return nt;
	}
	
	V_NeuronSWC_list tracedNeuron;
	vector< vector<V_NeuronSWC_unit> > mmUnit;
	
	//
	tracedNeuron = trace_one_pt_to_N_points_shortestdist(p4d, sz, p0, pp, trace_para, trace_z_thickness, mmUnit);
	//
	
	if (pp.size()>0) //trace to some selected markers
	{
		if (trace_para.b_deformcurve==false && tracedNeuron.nsegs()>=1)	proj_trace_smooth_downsample_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1);
		if (trace_para.b_estRadii==true) proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, true);
		if (trace_para.b_postMergeClosebyBranches && tracedNeuron.nsegs()>=2) proj_trace_mergeAllClosebyNeuronNodes(tracedNeuron); 
	}
	else //if (sz[3]==1) //trace to the entire image
	{		
		if (0 && pp.size()<=0) //disable the immatured tip-detection
		{
			//detect the tip automatically
			vector<LocationSimple> mypp;
			vector<DPoint_t> tt = tip_detection(p4d[trace_para.channo][0][0], sz);
			if (tt.size()>0)
			{
				for (V3DLONG i=0;i<tt.size();i++)
				{
					LocationSimple s; s.x = tt.at(i).m_x; s.y = tt.at(i).m_y; s.z = tt.at(i).m_z; mypp.push_back(s);
				}
			}
		}
		
		
		if ( trace_para.b_post_trimming )
		{
			//note the dark-pruning is done in GD tracing
			
			//dt
			
			bool b_to_background = true;
			bool b_3d_dt = true; 
			float *dd = new float [sz[0]*sz[1]*sz[2]];
			//fastmarching_dt(p4d[trace_para.channo][0][0], dd, sz[0], sz[1], sz[2], 2, 30);
			if(!dd) {cerr<<"dd error"<<endl; exit(0);}
			dt_v3dneuron(p4d[trace_para.channo][0][0], sz, trace_para.imgTH, b_to_background, b_3d_dt, dd);

			//
			
	//		vector<LocationSimple> ppnew;
	//		ppnew =  find_valid_tip_via_ignore_short_branches(mmUnit, p4d[trace_para.channo][0][0], dd, sz);
	//		FILE *f=fopen("tmp.marker", "wt");
	//		fprintf(f, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");
	//		for (V3DLONG i=0;i<ppnew.size();i++)
	//			fprintf(f, "%5.3f, %5.3f, %5.3f, 1, 0, , , 0, 255, 0\n", ppnew.at(i).x+1, ppnew.at(i).y+1, ppnew.at(i).z+1); 
	//		fclose(f);
				
				// estimate radius for filtering
			trace_para.b_3dcurve_width_from_xyonly = true;
			
			//covered-leaf pruning
			
			V3DLONG ninitotalpruned = mmUnit[0].size();
			V3DLONG ntotalpruned = 0;
			if (1)
			{
				for (int tmp=1; ; tmp++)
				{
					V3DLONG nnodes = mmUnit[0].size();
					printf("\n***** iteration [%d] ...... neuron node # = %ld ...... \n", tmp, mmUnit[0].size());
					proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, mmUnit, trace_para, 0, mmUnit.size()-1, trace_z_thickness, false);
					condense_branches(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness); //when disable bad for Jefferies data. need to find a better way as only deform leaf node is not optimal. 2011-01-13
					V3DLONG npruned = 0;
					npruned += pruning_covered_leaf_single_cover(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
					npruned += pruning_covered_leaf_multi_covers(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
					printf("totally pruned [%ld, or %5.4f%%] nodes in this [%d] iteration. total pruned so far [%ld, or %5.4f%%]\n", 
						   npruned, double(npruned)/nnodes*100.0, tmp, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);
					
					ntotalpruned += npruned;
					
					if (npruned<=0 || double(npruned)/nnodes<0.001)
						break;
				}
			}
			
			//remove other closeby branches (may not have been detected due to imperfect reconstruction node radius estimation)
			
			if (0) //disabled on 110801
			{
				ntotalpruned += pruning_covered_leaf_closebyfake_branches(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness, trace_para.visible_thresh);
			}
			
			//inter-node pruning
			
			if (1)
			{
				V3DLONG nnodes = mmUnit[0].size();
				V3DLONG ninternodepruned = pruning_internodes(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
				ntotalpruned += ninternodepruned;
				printf("totally pruned [%ld, or %5.4f%%] nodes in the inter-node iteration. total pruned so far [%ld, or %5.4f%%]\n", 
					   ninternodepruned, double(ninternodepruned)/nnodes*100.0, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);
			}
			

			//branch-node pruning. to be added 110801
			if (1)
			{
				V3DLONG nnodes = mmUnit[0].size();
				V3DLONG nbranchnodepruned = pruning_branch_nodes(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
				ntotalpruned += nbranchnodepruned;
				printf("totally pruned [%ld, or %5.4f%%] nodes in the branch-node iteration. total pruned so far [%ld, or %5.4f%%]\n", 
					   nbranchnodepruned, double(nbranchnodepruned)/nnodes*100.0, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);
				
			}

			//redo leaf and branch node pruning
			if (1)
			{
				printf("mmUnit size=%d",mmUnit.size());
				if (mmUnit.size()>0) 
					proj_trace_smooth_traced_neuron(p4d, sz, mmUnit, trace_para, 0, mmUnit.size()-1);
				proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, false);

				printf("\n***** REDO leaf pruning neuron node # = %ld ...... \n", mmUnit[0].size());
				for (int tmp=1; ; tmp++)
				{
					V3DLONG nnodes = mmUnit[0].size();
					printf("\n***** iteration [%d] ...... neuron node # = %ld ...... \n", tmp, mmUnit[0].size());
					proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, mmUnit, trace_para, 0, mmUnit.size()-1, trace_z_thickness, false);
					condense_branches(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness); //when disable bad for Jefferies data. need to find a better way as only deform leaf node is not optimal. 2011-01-13
					V3DLONG npruned = 0;
					npruned += pruning_covered_leaf_single_cover(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
					npruned += pruning_covered_leaf_multi_covers(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
					printf("totally pruned [%ld, or %5.4f%%] nodes in this [%d] iteration. total pruned so far [%ld, or %5.4f%%]\n", 
						   npruned, double(npruned)/nnodes*100.0, tmp, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);
					
					ntotalpruned += npruned;
					
					if (npruned<=0 || double(npruned)/nnodes<0.001)
						break;
				}

				if (1)
				{
					V3DLONG nnodes = mmUnit[0].size();
					V3DLONG nbranchnodepruned = pruning_branch_nodes(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
					ntotalpruned += nbranchnodepruned;
					printf("totally pruned [%ld, or %5.4f%%] nodes in the branch-node iteration. total pruned so far [%ld, or %5.4f%%]\n", 
						   nbranchnodepruned, double(nbranchnodepruned)/nnodes*100.0, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);
					
				}
                
			}
			
            //detect the artificial long, straight & dark branches 
            if (trace_para.b_pruneArtifactBranches)
            {
                V3DLONG nArtificialNodes = pruning_artifacial_branches(mmUnit, p4d[trace_para.channo], dd, sz, trace_z_thickness, trace_para.visible_thresh);
				while (1)
                {
                    V3DLONG nFinalSingleCoverPruned = pruning_covered_leaf_single_cover(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
                    if (nFinalSingleCoverPruned<=0)
                        break;
                }
            }
            
		}
        
        //
		
		int n_end_nodes = 0;//for (V3DLONG ii=0;ii<mmUnit[0].size();ii++)	{if (mmUnit[0][ii].nchild==0) n_end_nodes++;}
		V_NeuronSWC_list tn_new;
		int nSegsTrace = mergeback_mmunits_to_neuron_path(n_end_nodes, mmUnit, tn_new);
		tracedNeuron = tn_new; //use tn_new as the merge function will merge the old and new
		printf("nSegsTrace=%d, n_end_nodes=%ld\n", nSegsTrace, n_end_nodes);
		
		proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, false);

		if (0 && trace_para.b_post_trimming)
		{
			filterNodesByFindingBestCovers(p4d, sz, 0, sz[0]-1, 0, sz[1]-1, 0, sz[2]-1, 
										   tracedNeuron, trace_para, trace_z_thickness);
			//
			//			//filter out the thin axons which should be over-traced of weak signals
			//			filterThinSegmentsInImage(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness);
			//
			//			//redo it for the filtered image 
			//			
			//			
			//			tracedNeuron = trace_one_pt_to_N_points_shortestdist(p4d, sz, p0, pp, trace_para, trace_z_thickness);
			//			proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness);
		}
		
//		if (dd) {delete []dd; dd=0;}
	}

	nt = convertNeuronTreeFormat(tracedNeuron);
	return nt;
}


V_NeuronSWC_list trace_one_pt_to_N_points_shortestdist(unsigned char ****p4d, V3DLONG sz[4], 
															 LocationSimple & p0, vector<LocationSimple> & pp, 
															 CurveTracePara & trace_para, double trace_z_thickness,
															 vector< vector<V_NeuronSWC_unit> > & mmUnit) 
{
	V_NeuronSWC_list tracedNeuron;
    CHECK_DATA_GD_TRACING(tracedNeuron);

	ParaShortestPath sp_para;
	sp_para.edge_select       = trace_para.sp_graph_connect; 
	sp_para.background_select = trace_para.sp_graph_background; 
	sp_para.node_step      = trace_para.sp_graph_resolution_step; 
	sp_para.outsample_step = trace_para.sp_downsample_step;
	sp_para.smooth_winsize = trace_para.sp_smoothing_win_sz;
	sp_para.imgTH = trace_para.imgTH;
    sp_para.visible_thresh = trace_para.visible_thresh;
    
    mmUnit.clear();
	int chano = trace_para.channo;
	if (chano>=sz[3]) chano=sz[3]-1; if (chano<0) chano=0; 
	int n_end_nodes = pp.size();
	vector<float> px, py, pz;
	px.clear(), py.clear(), pz.clear();
	for (int i=0;i<pp.size();i++) {px.push_back(pp[i].x), py.push_back(pp[i].y), pz.push_back(pp[i].z);}

	BoundingBox trace_bounding_box;
	printf("find_shortest_path_graphimg >>> ");
	if (trace_bounding_box == NULL_BoundingBox)
	{
		trace_bounding_box.x0 = trace_bounding_box.y0 = trace_bounding_box.z0 = 0;
		trace_bounding_box.x1 = sz[0]-1;
		trace_bounding_box.y1 = sz[1]-1;
		trace_bounding_box.z1 = sz[2]-1;
		printf("set z1=%ld\n", V3DLONG(trace_bounding_box.z1));
	}
	printf("z1=%ld\n", V3DLONG(trace_bounding_box.z1));
	
	float *pxp = 0, *pyp=0, *pzp=0;
	if (n_end_nodes>0)
	{
		pxp = &(px[0]);
		pyp = &(py[0]);
		pzp = &(pz[0]);
	}
	
	char* s_error = 0;
	if (sz[3]==1)
	{
		if(1)
		{
			s_error = find_shortest_path_graphimg(p4d[chano], sz[0], sz[1], sz[2],
											  trace_z_thickness,
											  trace_bounding_box.x0, trace_bounding_box.y0, trace_bounding_box.z0,
											  trace_bounding_box.x1, trace_bounding_box.y1, trace_bounding_box.z1,
											  p0.x, p0.y, p0.z,
											  n_end_nodes,
											  pxp, pyp, pzp, //fix this bug 100624
											  mmUnit,
											  sp_para);

		}
		else
		{
#ifdef __USE_HIERARCHICAL_PRUNING__
            
			s_error = find_shortest_path_graphimg_FM(p4d[chano], sz[0], sz[1], sz[2],
											  trace_z_thickness,
											  trace_bounding_box.x0, trace_bounding_box.y0, trace_bounding_box.z0,
											  trace_bounding_box.x1, trace_bounding_box.y1, trace_bounding_box.z1,
											  p0.x, p0.y, p0.z,
											  n_end_nodes,
											  pxp, pyp, pzp, //fix this bug 100624
											  mmUnit,
											  sp_para);
#endif
		}
	}
	else
	{
		V3DLONG *chans = new V3DLONG [sz[3]];
		for(V3DLONG tmpi=0; tmpi<sz[3]; tmpi++) chans[tmpi]=tmpi; //use all channels (since the data of channels_to_use have actually been copied here)
		s_error = find_shortest_path_graphimg(p4d, sz, chans, sz[3],
											  trace_z_thickness,
											  trace_bounding_box.x0, trace_bounding_box.y0, trace_bounding_box.z0,
											  trace_bounding_box.x1, trace_bounding_box.y1, trace_bounding_box.z1,
											  p0.x, p0.y, p0.z,
											  n_end_nodes,
											  pxp, pyp, pzp, //fix this bug 100624
											  mmUnit,
											  sp_para);
	}
	printf("find_shortest_path_graphimg <<< ");
	if (s_error)
	{
        v3d_msg(s_error,0);
      //  throw (const char*)s_error;
		return tracedNeuron;
	}
	
	int nSegsTrace = mergeback_mmunits_to_neuron_path(n_end_nodes, mmUnit, tracedNeuron);
	
	//return traced res
	return tracedNeuron;
}

int mergeback_mmunits_to_neuron_path(int n_end_nodes, vector< vector<V_NeuronSWC_unit> > & mmUnit, V_NeuronSWC_list & tNeuron)
{
	if (mmUnit.size()<=0) 
		return mmUnit.size();
	
	//merge traced path /////////////////////////////////////////////////////////
	if (n_end_nodes >=2)
	{
		merge_back_traced_paths(mmUnit); // start --> n end
	}
	
	//put into tNeuron /////////////////////////////////////////////////////////
	if (n_end_nodes<=0) // entire image, direct copy
	{
		V3DLONG nexist = tNeuron.maxnoden();
		
		V_NeuronSWC cur_seg;	cur_seg.clear();
		vector<V_NeuronSWC_unit> & mUnit = mmUnit[0];
		
		for (V3DLONG i=0;i<mUnit.size();i++)
		{
			if (mUnit[i].nchild<0) continue; 
			
			V_NeuronSWC_unit node = mUnit[i];
			//node.r = 0.5;
			node.n += nexist;
			if (node.parent >=1)  node.parent += nexist;
				else node.parent = -1;
					cur_seg.append(node);
		}
		
		QString tmpss;  tmpss.setNum(tNeuron.nsegs()+1);
		cur_seg.name = qPrintable(tmpss);
		cur_seg.b_linegraph=false; //don't forget to do this
		tNeuron.append(cur_seg);
		tNeuron.name = TRACED_NAME;
		tNeuron.file = TRACED_FILE;
	}
	else
	{
		for (V3DLONG ii=0;ii<mmUnit.size();ii++)
		{
			V3DLONG nexist = tNeuron.maxnoden();
			
			V_NeuronSWC cur_seg;	cur_seg.clear();
			vector<V_NeuronSWC_unit> & mUnit = mmUnit[ii];
			
			for (V3DLONG i=0;i<mUnit.size();i++)
			{
				if (mUnit[i].nchild<0) continue; 
				
				V_NeuronSWC_unit v;
				set_simple_path_unit(v, nexist, mUnit, i, (n_end_nodes==1)); // link_order determined by 1/N path
				
				cur_seg.append(v);
				//qDebug("%d ", cur_seg.nnodes());
			}
			
			QString tmpss;  tmpss.setNum(tNeuron.nsegs()+1);
			cur_seg.name = qPrintable(tmpss);
			cur_seg.b_linegraph=true; //don't forget to do this
			tNeuron.append(cur_seg);
			tNeuron.name = TRACED_NAME;
			tNeuron.file = TRACED_FILE;
		}
	}
	
	return mmUnit.size();
}



NeuronTree convertNeuronTreeFormat(V_NeuronSWC_list & tracedNeuron)
{
	NeuronTree SS;
	
	//first conversion
	
	V_NeuronSWC seg = merge_V_NeuronSWC_list(tracedNeuron);
	seg.name = tracedNeuron.name;
	seg.file = tracedNeuron.file;
	
	//second conversion
	
	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
	
	{
		int count = 0;
		for (int k=0;k<seg.row.size();k++)
		{
			count++;
			NeuronSWC S;
			
			S.n 	= seg.row.at(k).data[0];
			if (S.type<=0) S.type 	= 2; //seg.row.at(k).data[1];
			S.x 	= seg.row.at(k).data[2];
			S.y 	= seg.row.at(k).data[3];
			S.z 	= seg.row.at(k).data[4];
			S.r 	= seg.row.at(k).data[5];
			S.pn 	= seg.row.at(k).data[6];
			
			//for hit & editing
			S.seg_id       = seg.row.at(k).seg_id;
			S.nodeinseg_id = seg.row.at(k).nodeinseg_id;
			
			//qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);
			
			//if (! listNeuron.contains(S)) // 081024
			{
				listNeuron.append(S);
				hashNeuron.insert(S.n, listNeuron.size()-1);
			}
		}
		printf("---------------------read %d lines, %d remained lines", count, listNeuron.size());
		
		SS.n = -1;
		SS.color = XYZW(seg.color_uc[0],seg.color_uc[1],seg.color_uc[2],seg.color_uc[3]);
		SS.on = true;
		SS.listNeuron = listNeuron;
		SS.hashNeuron = hashNeuron;
		
		SS.name = seg.name.c_str();
		SS.file = seg.file.c_str();
	}
	
	return SS;
}

bool proj_trace_smooth_downsample_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, int seg_begin, int seg_end)
{
	v3d_msg("proj_trace_smooth_downsample_last_traced_neuron(). \n",0);
	CHECK_DATA_GD_TRACING(false);
	
	V3DLONG nexist = 0; // re-create index number
	
	// (VneuronSWC_list tracedNeuron).(V_neuronSWC seg[]).(V_nueronSWC_unit row[])
	for(V3DLONG iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) continue; //091023
		
		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
		printf("#seg=%d(%d)", iseg, cur_seg.row.size());
		
		vector<V_NeuronSWC_unit> & mUnit = cur_seg.row; // do in place
		{
			//------------------------------------------------------------
			vector<V_NeuronSWC_unit> mUnit_prior = mUnit; // a copy as prior
			
			//smooth_curve(mCoord, trace_para.sp_smoothing_win_sz);
			mUnit = downsample_curve(mUnit, trace_para.sp_downsample_step);
			
			//------------------------------------------------------------
			BDB_Minus_Prior_Parameter bdbp_para;
			bdbp_para.nloops   =trace_para.nloops;
			bdbp_para.f_smooth =trace_para.internal_force2_weight;
			bdbp_para.f_length =trace_para.internal_force_weight;
			bdbp_para.f_prior  = 0.2;
			int chano = trace_para.channo;
			
			point_bdb_minus_3d_localwinmass_prior(p4d[chano], sz[0], sz[1], sz[2],
												  mUnit, bdbp_para, true, // 090618: add constraint to fix 2 ends
												  mUnit_prior);
			//-------------------------------------------------------------
			
		}
		printf(">>%d(%d) ", iseg, mUnit.size());
		
		reset_simple_path_index (nexist, mUnit);
		nexist += mUnit.size();
	}
	printf("\n");
	
	return true;
}

bool proj_trace_smooth_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], vector< vector<V_NeuronSWC_unit> > & mmUnit, CurveTracePara & trace_para, int seg_begin, int seg_end)
{	
	v3d_msg("proj_trace_smooth_traced_neuron(). \n",0);
	CHECK_DATA_GD_TRACING(false);
	
	V3DLONG nexist = 0; // re-create index number
	
	for(V3DLONG iseg=0; iseg<mmUnit.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) continue; //091023
		
		vector<V_NeuronSWC_unit> & mUnit = mmUnit[iseg]; // do in place
		printf("#seg=%d(%d)", iseg, mUnit.size());
		
		{
			//------------------------------------------------------------
			vector<V_NeuronSWC_unit> mUnit_prior = mUnit; // a copy as prior
			
			//smooth_curve(mCoord, trace_para.sp_smoothing_win_sz);
			//mUnit = downsample_curve(mUnit, trace_para.sp_downsample_step);
			
			//------------------------------------------------------------
			BDB_Minus_Prior_Parameter bdbp_para;
			bdbp_para.nloops   =trace_para.nloops;
			bdbp_para.f_smooth =trace_para.internal_force2_weight;
			bdbp_para.f_length =trace_para.internal_force_weight;
			bdbp_para.f_prior  = 0.2;
			int chano = trace_para.channo;
			
			point_bdb_minus_3d_localwinmass_prior_withGraphOrder(p4d[chano], sz[0], sz[1], sz[2],
												  mUnit, bdbp_para, true, 
												  mUnit_prior);
			//-------------------------------------------------------------
			

		}
		printf(">>%d(%d) ", iseg, mUnit.size());
		
		//reset_simple_path_index (nexist, mUnit); //because this is not a simple structure, thus should not reannage the order
		//nexist += mUnit.size();

	}
	printf("\n");
	
	return true;
}


bool proj_trace_compute_radius_of_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness, bool b_smooth)
{
	v3d_msg("proj_trace_compute_radius_of_last_traced_neuron(). \n", 0);
	CHECK_DATA_GD_TRACING(false);
	
	//int chano = trace_para.channo; //20110917. the multichannel and single channel tracing can be distinguished based sz[3].  
	int smoothing_win_sz = trace_para.sp_smoothing_win_sz;
	
	for(int iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) continue; 
		
		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
		printf("#seg=%d(%d) ", iseg, cur_seg.row.size());
		
		std::vector<V_NeuronSWC_unit> & mUnit = cur_seg.row; // do in place
		{
			if (sz[3]==1)
			{
				fit_radius_and_position(p4d[0], sz[0], sz[1], sz[2],
										mUnit,
										false,       // do not move points here
										myzthickness, 
										trace_para.b_3dcurve_width_from_xyonly); 
			}
			else 
			{
				fit_radius_and_position(p4d, sz,
										mUnit,
										false,       // do not move points here
										myzthickness, 
										trace_para.b_3dcurve_width_from_xyonly); 
			}

			if (b_smooth)
				smooth_radius(mUnit, smoothing_win_sz, false); 
		}
	}
	printf("\n");
	
	return true;
}

bool proj_trace_compute_radius_of_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], vector< vector<V_NeuronSWC_unit> >& mmUnit, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness, bool b_smooth)
{
	v3d_msg("proj_trace_compute_radius_of_last_traced_neuron(). \n", 0);
	CHECK_DATA_GD_TRACING(false);
	
	int chano = trace_para.channo;
	int smoothing_win_sz = trace_para.sp_smoothing_win_sz;
	
	for(int iseg=0; iseg<mmUnit.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) continue; 
		printf("#seg=%d(%d) ", iseg, mmUnit[iseg].size());
		std::vector<V_NeuronSWC_unit> & mUnit = mmUnit[iseg]; // do in place
		{
			fit_radius_and_position(p4d[chano], sz[0], sz[1], sz[2],
									mUnit,
									false,       // do not move points here
									myzthickness, 
									trace_para.b_3dcurve_width_from_xyonly); 
			
			if (b_smooth)
				smooth_radius(mUnit, smoothing_win_sz, false); 
		}
	}
	printf("\n");
	
	return true;
}


bool proj_trace_mergeAllClosebyNeuronNodes(V_NeuronSWC_list & tracedNeuron)
{
	v3d_msg("proj_trace_mergeAllClosebyNeuronNodes(). \n", 0);
	//this function will merge closeby nodes in different segments.
	bool res=true;
	
	V3DLONG seg_id, cur_sid;
	V3DLONG i,j;
	
	int NSegs = tracedNeuron.seg.size();
	if (NSegs==1) //do nothing when there is only one segment
	{
		v3d_msg("Only one segment. Do nothing.",0);
		return false;
	}
	
	//if there are multiple segs, then ensure each one is a line graph
	for (seg_id=0; seg_id<NSegs;seg_id++) 
	{
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		if (!subject_swc.isLineGraph())
		{
			v3d_msg(QString("The %1 neuron segment is not a simple line graph. You should first select Edit to turn it into simpler pieces.").arg(seg_id));
			return false;
		}
	}
	
	//first	determine the termini nodes which should be preserved
	vector <V_NeuronSWC_unit> terminiNodePool;
	vector <V_NeuronSWC_unit> realRootNodePool;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		//for the first node of the segment
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		int slength = subject_swc.nrows();
		
		bool b_termini; 
		for (cur_sid=0, b_termini=true;cur_sid<NSegs;cur_sid++)
		{
			if (cur_sid==seg_id)
				continue;
			
			V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
			int tlength = target_swc.nrows();
			
			for (i=0;i<target_swc.row.size();i++)
			{
				if (subject_swc.row.at(0).get_coord() == target_swc.row.at(i).get_coord())
				{	
					b_termini=false;
					break;
				}
			}
			if (!b_termini)
				break;
		}
		
		if (b_termini)
		{
			terminiNodePool.push_back(subject_swc.row.at(0));
			if (subject_swc.row.at(0).parent<0) //then it is a root, and does not overlap with others; this should be real root in the whole swc file
				realRootNodePool.push_back(subject_swc.row.at(0));
		}
		
		if (slength>1) //for the last node of the segment
		{
			for (cur_sid=0, b_termini=true;cur_sid<NSegs;cur_sid++)
			{
				if (cur_sid==seg_id)
					continue;
				
				V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
				int tlength = target_swc.nrows();
				
				for (i=0;i<target_swc.row.size();i++)
				{
					if (subject_swc.row.at(slength-1).get_coord() == target_swc.row.at(i).get_coord())
					{	
						b_termini=false;
						break;
					}
				}
				if (!b_termini)
					break;
			}
			
			if (b_termini)
			{
				terminiNodePool.push_back(subject_swc.row.at(slength-1));
				if (subject_swc.row.at(slength-1).parent<0) //then it is a root, and does not overlap with others; this should be real root in the whole swc file
					realRootNodePool.push_back(subject_swc.row.at(slength-1));
			}
		}
	}
	
	
	//then find and update the location/radius that correspond to the merged nodes 
	
#define V3DNEURON_MERGE_TO_BIGSPHERE 1
	
	V_NeuronSWC_list tracedNeuronNew = tracedNeuron;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		V_NeuronSWC & subject_swc_new = tracedNeuronNew.seg.at(seg_id);
		V3DLONG slength = subject_swc.nrows();
		
		for (j=0; j<slength; j++)
		{
			double scx = subject_swc.row.at(j).x;
			double scy = subject_swc.row.at(j).y;
			double scz = subject_swc.row.at(j).z;
			double scr2 = subject_swc.row.at(j).r; scr2 *= scr2; //squared radius
			
			V3DLONG ind_best_merge_seg=-1, ind_best_merge_node=-1;	double r_best_merge=-1, dist_best; //set as -1 for initialization
			for (cur_sid=seg_id+1;cur_sid<NSegs;cur_sid++)
			{
				if (cur_sid==seg_id) //only find in a different seg
					continue;
				
				V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
				//map <V3DLONG, V3DLONG> target_index_map = unique_V_NeuronSWC_nodeindex(target_swc);
				V3DLONG tlength = target_swc.nrows();
				
				for (i=0;i<tlength;i++)
				{
					double tcr2 = target_swc.row.at(i).r; tcr2 *= tcr2; //squared radius
					
					double tt = (scr2>tcr2)?scr2:tcr2; 
					tt=tt; //4; //devide by 2*2=4 so that the transition will be smoother
					
					double dtcx = target_swc.row.at(i).x - scx; dtcx *= dtcx;
					if (dtcx>tt)
						continue; 
					
					double dtcy = target_swc.row.at(i).y - scy; dtcy *= dtcy;
					if (dtcy>tt || dtcy+dtcx>tt)
						continue;
					
					double dtcz = target_swc.row.at(i).z - scz; dtcz *= dtcz;
					if (dtcz>tt || dtcz+dtcy+dtcx>tt)
						continue;
					
					if (tcr2>scr2 && tcr2>r_best_merge)
					{
						ind_best_merge_seg = cur_sid;
						ind_best_merge_node = i;
						r_best_merge = tcr2;
						dist_best = dtcz+dtcy+dtcx;
					}
				}
			}
			
			if (ind_best_merge_seg>=0) // && ind_best_merge_node>=0 && r_best_merge>=0) //only one judgment is enough
			{
				V_NeuronSWC & target_swc = tracedNeuron.seg.at(ind_best_merge_seg);
				
				if (V3DNEURON_MERGE_TO_BIGSPHERE) //I set it to 0, thus don't use this method
				{
					subject_swc_new.row.at(j).x = target_swc.row.at(ind_best_merge_node).x;
					subject_swc_new.row.at(j).y = target_swc.row.at(ind_best_merge_node).y;
					subject_swc_new.row.at(j).z = target_swc.row.at(ind_best_merge_node).z;
					subject_swc_new.row.at(j).r = target_swc.row.at(ind_best_merge_node).r;
				}
				else //then merge the location proportionally
				{
					V_NeuronSWC & target_swc_new = tracedNeuronNew.seg.at(ind_best_merge_seg);
					//
					double rs = subject_swc.row.at(j).r, rt = target_swc.row.at(ind_best_merge_node).r;
					double newx = (subject_swc.row.at(j).x*rs + target_swc.row.at(ind_best_merge_node).x*rt)/(rs+rt);
					double newy = (subject_swc.row.at(j).y*rs + target_swc.row.at(ind_best_merge_node).y*rt)/(rs+rt);
					double newz = (subject_swc.row.at(j).z*rs + target_swc.row.at(ind_best_merge_node).z*rt)/(rs+rt);
					double newr = (rs + rt + sqrt(dist_best))/2.0;
					
					//still just update one
					subject_swc_new.row.at(j).x = target_swc_new.row.at(ind_best_merge_node).x = newx;
					subject_swc_new.row.at(j).y = target_swc_new.row.at(ind_best_merge_node).y = newy;
					subject_swc_new.row.at(j).z = target_swc_new.row.at(ind_best_merge_node).z = newz;
					subject_swc_new.row.at(j).r = target_swc_new.row.at(ind_best_merge_node).r = newr;
				}
			}
		}
	}
	
	//then determine the pool of all nodes
	vector <V_NeuronSWC_unit> finalNodePool;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuronNew.seg.at(seg_id);
		V3DLONG slength = subject_swc.nrows();
		for (j=0; j<slength; j++)
		{
			bool b_exist=false;
			for (i=0;i<finalNodePool.size();i++)
			{
				if (subject_swc.row.at(j).get_coord() == finalNodePool.at(i).get_coord())
				{
					b_exist=true;
					break;
				}
			}
			if (!b_exist) 
				finalNodePool.push_back(subject_swc.row.at(j)); //thus finalNodePool contains unique coordinates. Note the respective radius has been updated to the largest already.
		}
	}
	
	//now determine the edge graph
	std::vector<Edge> 	edge_array;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuronNew.seg.at(seg_id);
		map <V3DLONG, V3DLONG> subject_index_map = unique_V_NeuronSWC_nodeindex(subject_swc);
		V3DLONG slength = subject_swc.nrows();
		
		printf("seg=%ld\n", seg_id);
		for (j=0; j<slength; j++)
		{
			printf("j=%ld key=%ld mapped row=%ld node=%ld x=%5.3f y=%5.3f z=%5.3f parent=%ld\n", j, V3DLONG(subject_swc.row.at(j).n), subject_index_map[V3DLONG(subject_swc.row.at(j).n)], V3DLONG(subject_swc.row.at(j).n), subject_swc.row.at(j).x, subject_swc.row.at(j).y, subject_swc.row.at(j).z, V3DLONG(subject_swc.row.at(j).parent));
		}
		printf("\n\n");
		
		for (j=0; j<slength; j++)
		{
			V3DLONG c = j;
			V3DLONG p =  V3DLONG(subject_swc.row.at(j).parent);
			if (p<0) continue;
			else p = subject_index_map[p];
			
			//printf("c0=%ld p0=%ld c=%ld p=%ld\n", c0, p0, c, p);
			if (p<0 || p>=slength) {v3d_msg(QString("detect a strange parent! row(%1) node=%2 parent=%3").arg(j).arg(subject_swc.row.at(j).n).arg(p)); continue;}
			
			V3DLONG ipos=-1;	for (i=0;i<finalNodePool.size();i++)	{if (subject_swc.row.at(c).get_coord() == finalNodePool.at(i).get_coord())	{ipos=i;break;	}}
			V3DLONG ippos=-1;	for (i=0;i<finalNodePool.size();i++)	{if (subject_swc.row.at(p).get_coord() == finalNodePool.at(i).get_coord())	{ippos=i;break;	}}
			if (ipos>=0 && ippos>=0)
			{
				edge_array.push_back(Edge(ippos, ipos)); //for simplicity here I only keep a one-directional edge
			}
			else
			{
				v3d_msg("Error: this message should never appear. You have a neuron node whose coordinate cannot be detected!");
				return false;
			}
		}
	}
	
	
	//find the indexes of the termini nodes and root nodes in the finalNodePool (which contains the complete node info)
	V3DLONG *ind_end_nodes = new V3DLONG [terminiNodePool.size()], ind_startnode;
	if (realRootNodePool.size()<=0)
		ind_startnode=0;
	else
	{
		double cur_dist = distL2square(realRootNodePool.at(0).get_coord(), finalNodePool.at(0).get_coord()); //note that if realRootNodePool has size >=1, then finalNodePool should at least that big; then finalNodePool(0) should be safe
		for (j=0;j<finalNodePool.size();j++)
		{
			double tmp = distL2square(realRootNodePool.at(0).get_coord(), finalNodePool.at(j).get_coord());
			if (tmp<=cur_dist)
			{
				cur_dist = tmp;
				ind_startnode = j;
				if (cur_dist==0)
				{
					break;
				}
			}
		}
	}
	for (i=0;i<terminiNodePool.size();i++)
	{
		double cur_dist = distL2square(terminiNodePool.at(i).get_coord(), finalNodePool.at(0).get_coord()); //note that if realRootNodePool has size >=1, then finalNodePool should at least that big; then finalNodePool(0) should be safe
		for (j=0;j<finalNodePool.size();j++)
		{
			double tmp = distL2square(terminiNodePool.at(i).get_coord(), finalNodePool.at(j).get_coord());
			if (tmp<=cur_dist)
			{
				cur_dist = tmp;
				ind_end_nodes[i] = j;
				if (cur_dist == 0)
				{
					break;
				}
			}
		}
	}
	
	//now recompute the neuron structure based on merged node locations
	V3DLONG ntotalnodes = finalNodePool.size();
	double *xa = new double [ntotalnodes];
	double *ya = new double [ntotalnodes];
	double *za = new double [ntotalnodes];
	double *va = new double [ntotalnodes];
	for (i=0;i<ntotalnodes;i++)
	{
		xa[i] = finalNodePool.at(i).x;
		ya[i] = finalNodePool.at(i).y;
		za[i] = finalNodePool.at(i).z;
		va[i] = 255;
	}
	float zthickness = 1.0;
	vector< vector<V_NeuronSWC_unit> > mmUnit;
	ParaShortestPath para;
	V_NeuronSWC_list curTraceNeuron;
	curTraceNeuron.name = tracedNeuron.name;
	curTraceNeuron.file = tracedNeuron.file;
	
	char *err_msg = find_shortest_path_graphpointset(ntotalnodes,
													 xa, ya, za, va, //the coordinates and values of all nodes
													 zthickness, // z-thickness for weighted edge
													 edge_array,	
													 ind_startnode,        // start node's index 
													 terminiNodePool.size(),          // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
													 ind_end_nodes,      // all end nodes' indexes
													 mmUnit, // change from Coord3D for shortest path tree
													 para);
	
	int nsegsnew = mergeback_mmunits_to_neuron_path(terminiNodePool.size(), mmUnit, curTraceNeuron);
	
	for (i=0;i<curTraceNeuron.seg.size();i++) //now use the already estimated radius information
	{
		for (j=0;j<curTraceNeuron.seg.at(i).row.size();j++)
		{
			bool b_find=false;
			for (V3DLONG ipos=0;ipos<finalNodePool.size();ipos++)	
			{
				if (curTraceNeuron.seg.at(i).row.at(j).get_coord() == finalNodePool.at(ipos).get_coord())	
				{
					curTraceNeuron.seg.at(i).row.at(j).r = finalNodePool.at(ipos).r;
					b_find=true;
					break;
				}
			}
			if (!b_find)
			{
				v3d_msg(QString("The %1 seg %2 node has an unmatched node.\n").arg(i).arg(j), 0); 
			}
		}
	}
	
	//tracedNeuron = curTraceNeuron; //overwrite the neuron tracing result
	tracedNeuron.seg = curTraceNeuron.seg; //overwrite the neuron tracing result w/o producing an additional neuron. why? 100415
	
	//free space
	if (xa) {delete []xa; xa=0;}
	if (ya) {delete []ya; ya=0;}
	if (za) {delete []za; za=0;}
	if (va) {delete []va; va=0;}
	if (ind_end_nodes) {delete []ind_end_nodes; ind_end_nodes=0;}
	
	v3d_msg("merge all successfully", 0);
	return res;
}

bool filterThinSegmentsInImage(unsigned char ****p4d, V3DLONG sz[4], V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness)
{
	v3d_msg("filterThinSegmentsInImage(). \n", 0);
	CHECK_DATA_GD_TRACING(false);
	
	V3DLONG chano = trace_para.channo;
	V3DLONG sp_downsample_step = trace_para.sp_graph_resolution_step;
	unsigned char ***p3d = p4d[chano];
	
	for(V3DLONG iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) 
			continue; 
		
		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
		printf("#seg=%d(%d) ", iseg, cur_seg.row.size());
		
		std::vector<V_NeuronSWC_unit> & mUnit = cur_seg.row; // do in place
		
		for (V3DLONG iu = 0; iu<mUnit.size(); iu++)
		{
			V_NeuronSWC_unit & u = mUnit.at(iu);
			//p3d[V3DLONG(u.z)][V3DLONG(u.y)][V3DLONG(u.x)] = 0;
			
			if (u.r<=1 && u.parent>=0)
			//	p3d[V3DLONG(u.z)][V3DLONG(u.y)][V3DLONG(u.x)] = 0;
			setBlockAveValue(p3d, sz[0], sz[1], sz[2], u.x, u.y, u.z, sp_downsample_step, sp_downsample_step, sp_downsample_step/myzthickness, 0);
		}
	}

	printf("\n");
	
	return true;
}

bool filterNodesByFindingBestCovers(unsigned char ****p4d, V3DLONG sz[4], 
									V3DLONG bx0, V3DLONG bx1,
									V3DLONG by0, V3DLONG by1,
									V3DLONG bz0, V3DLONG bz1,
									V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, float myzthickness)
{
	v3d_msg("filterNodesByFindingBestCovers(). \n", 0);
	CHECK_DATA_GD_TRACING(false);

	//parameters
	int min_step       = trace_para.sp_graph_resolution_step; //should be >=1
	if (min_step<1)       min_step =1;
	
	//bounding box volume
	V3DLONG xmin = bx0, xmax = bx1,
		ymin = by0, ymax = by1,
		zmin = bz0, zmax = bz1;
		
	V3DLONG nx=((xmax-xmin)/min_step)+1, 	xstep=min_step,
		ny=((ymax-ymin)/min_step)+1, 	ystep=min_step,
		nz=((zmax-zmin)/min_step)+1, 	zstep=min_step;
	
	printf("valid bounding (%ld %ld %ld)--(%ld %ld %ld) ......  ", xmin,ymin,zmin, xmax,ymax,zmax);
	printf("%ld x %ld x %ld nodes, step = %d \n", nx, ny, nz, min_step);
	
	V3DLONG num_nodes = nx*ny*nz;
	V3DLONG i,j,k,n,m;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NODE_FROM_XYZ(x,y,z) 	(V3DLONG((z+.5)-zmin)/zstep*ny*nx + V3DLONG((y+.5)-ymin)/ystep*nx + V3DLONG((x+.5)-xmin)/xstep)
#define NODE_TO_XYZ(j, x,y,z) \
{ \
z = (j)/(nx*ny); 		y = ((j)-V3DLONG(z)*nx*ny)/nx; 	x = ((j)-V3DLONG(z)*nx*ny-V3DLONG(y)*nx); \
x = xmin+(x)*xstep; 	y = ymin+(y)*ystep; 			z = zmin+(z)*zstep; \
}
#define NODE_FROM_IJK(i,j,k) 	((k)*ny*nx+(j)*nx+(i))
#define X_I(i)				 	(xmin+(i)*xstep)
#define Y_I(i)				 	(ymin+(i)*ystep)
#define Z_I(i)				 	(zmin+(i)*zstep)
	
#define X_SI(i)				 	(V3DLONG(((i)-xmin)/xstep))
#define Y_SI(i)				 	(V3DLONG(((i)-ymin)/ystep))
#define Z_SI(i)				 	(V3DLONG(((i)-zmin)/zstep))
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SET_ALLELEMENTS_ZEROS(pp, pplen) \
{ \
for (V3DLONG ppi=0; ppi<pplen; ppi++) pp[ppi] = 0; \
}
#define PUT_IN_RANGE(x0, x, x1) {if (x<x0) x=x0; else if (x>x1) x=x1; }	
	
	V3DLONG chano = trace_para.channo;
	unsigned char ***p3d = p4d[chano];

	//create memory for flag and radius
	V3DLONG stacksz = nx*ny*nz;
	float * maxr_img_1d = 0, *** maxr_img_3d = 0; 
	V3DLONG * maxid_img_1d = 0, *** maxid_img_3d = 0; 
	try
	{
		maxr_img_1d = new float [stacksz]; SET_ALLELEMENTS_ZEROS(maxr_img_1d, stacksz);
		new3dpointer(maxr_img_3d, nx, ny, nz, maxr_img_1d);
		maxid_img_1d = new V3DLONG [stacksz]; SET_ALLELEMENTS_ZEROS(maxid_img_1d, stacksz);
		new3dpointer(maxid_img_3d, nx, ny, nz, maxid_img_1d);
	}
	catch(...)
	{
		v3d_msg("Fail to allocate memory in filterNodesByFindingBestCovers();\n", 0);
		goto Label_Exit_filterNodesByFindingBestCovers;
	}
	
	for(V3DLONG iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
	{
		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
		printf("#seg=%d(%d) ", iseg, cur_seg.row.size());
		
		std::vector<V_NeuronSWC_unit> & mUnit = cur_seg.row; // do in place
		
		//find the best covering radius & id
		for (V3DLONG iu = 0; iu<mUnit.size(); iu++)
		{
			V_NeuronSWC_unit & u = mUnit.at(iu);
			
			V3DLONG curnx = X_SI(V3DLONG(u.x));
			V3DLONG curny = Y_SI(V3DLONG(u.y));
			V3DLONG curnz = Z_SI(V3DLONG(u.z));
			
			V3DLONG i0 = curnx - u.r/xstep; PUT_IN_RANGE(0, i0, nx-1);
			V3DLONG	i1 = curnx + u.r/xstep; PUT_IN_RANGE(0, i1, nx-1);
			V3DLONG j0 = curny - u.r/ystep; PUT_IN_RANGE(0, j0, ny-1);
			V3DLONG	j1 = curny + u.r/ystep; PUT_IN_RANGE(0, j1, ny-1);
			V3DLONG k0 = curnz - u.r/zstep; PUT_IN_RANGE(0, k0, nz-1);
			V3DLONG	k1 = curnz + u.r/zstep; PUT_IN_RANGE(0, k1, nz-1);
			
			double d;
			for (V3DLONG k=k0;k<=k1;k++)
				for (V3DLONG j=j0;j<=j1;j++)
					for (V3DLONG i=i0;i<=i1;i++)
					{
						double d = sqrt(double(i-curnx)*(i-curnx) + double(j-curny)*(j-curny) + double(k-curnz)*(k-curnz));
						if (d > u.r/min_step)
							continue;
						
						if ( maxr_img_3d[k][j][i] < u.r)
						{
							maxr_img_3d[k][j][i] = u.r;
							maxid_img_3d[k][j][i] = u.n;
						}
					}
		}
		
		//remove the unneeded neuron node
		V3DLONG cnt=0, tcnt=0;
		for (V3DLONG iu = 0; iu<mUnit.size(); iu++)
		{
			V_NeuronSWC_unit & u = mUnit.at(iu);
			
			V3DLONG curnx = X_SI(V3DLONG(u.x));
			V3DLONG curny = Y_SI(V3DLONG(u.y));
			V3DLONG curnz = Z_SI(V3DLONG(u.z));

			if ( u.n != maxid_img_3d[curnz][curny][curnx] )
			{
				u.x = X_I(curnx);
				u.y = Y_I(curny);
				u.z = Z_I(curnz);
				u.parent = maxid_img_3d[curnz][curny][curnx];
				
				cnt++;
			}
			tcnt++;
		}
		printf("merge cnt=%ld total cnt=%ld\n", cnt, tcnt);
	}
	
	printf("\n");
	
Label_Exit_filterNodesByFindingBestCovers:
	if (maxr_img_3d) {delete3dpointer(maxr_img_3d, nx, ny, nz);};
	if (maxr_img_1d) {delete []maxr_img_1d; maxr_img_1d = 0;}
	if (maxid_img_3d) {delete3dpointer(maxid_img_3d, nx, ny, nz);};
	if (maxid_img_1d) {delete []maxid_img_1d; maxid_img_1d = 0;}
	
	return true;
}

bool est_pixel_radius(unsigned char *p1d, V3DLONG sz[3], float imgTH, float myzthickness, bool b_est_in_xyplaneonly, float *p1dr) //p1dr the detected radius
{
	v3d_msg("est_pixel_radius(). \n", 0);
	if (!p1d || !sz || sz[0]<1 || sz[1]<1 || sz[2]<1 || myzthickness<=0 || !p1dr)
	{
		v3d_msg("Invalid parameters.\n",0);
		return false;
	}
	
	unsigned char ***p3d = 0;
	float ***p3dr = 0;
	
	try{
		new3dpointer(p3d, sz[0], sz[1], sz[2], p1d);
		new3dpointer(p3dr, sz[0], sz[1], sz[2], p1dr);
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in est_pixel_radius().\n", 0);
		goto Label_Exit_est_pixel_radius;
	}
	
	//
	{
		double bound_r = 0; //this bound is not really being used, so set it arbitrarily as of now (2010-Dec-21)
		for (V3DLONG k=0;k<sz[2];k++)
			for (V3DLONG j=0;j<sz[1];j++)
				for (V3DLONG i=0;i<sz[0];i++)
				{
					p3dr[k][j][i] = (p3d[k][j][i] > imgTH) ? fitRadiusPercent(p3d, sz[0], sz[1], sz[2], imgTH, bound_r, i, j, k, myzthickness, b_est_in_xyplaneonly) : 0;
				}
	}
	//
	
Label_Exit_est_pixel_radius:
	if (p3d) {delete3dpointer(p3d, sz[0], sz[1], sz[2]);}
	if (p3dr) {delete3dpointer(p3dr, sz[0], sz[1], sz[2]);}

	return true;
}

bool dt_v3dneuron(unsigned char *p1d, V3DLONG sz[3], float imgTH, bool b_to_background, bool b_3d_dt, float *p1dt) //imgTH specify the foreground
{
	v3d_msg("dt_v3dneuron(). \n", 0);
	if (!p1d || !sz || sz[0]<1 || sz[1]<1 || sz[2]<1 || !p1dt)
	{
		v3d_msg("Invalid parameters.\n",0);
		return false;
	}
	
	V3DLONG len = sz[0]*sz[1]*sz[2];
	V3DLONG *label = 0;
	try {
		label = new V3DLONG [len];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in dt_v3dneuron().\n", 0);
		return false;
	}
	
	float *tmpdatap = p1dt, *endp=p1dt+len;
	unsigned char *indatap = p1d;
	for (tmpdatap=p1dt; tmpdatap<endp; ++tmpdatap)
	{
		if (b_to_background) //distance to background
			*tmpdatap = (*indatap++ > imgTH) ? INF : 0;  
		else //distance to foreground
			*tmpdatap = (*indatap++ > imgTH) ? 0 : INF;
	}
	
	if (b_3d_dt)
	{
		printf("3d =====================================\n");
		dt3d(p1dt, label, sz);
	}
	else //2d dt
	{
		printf("2d=============================\n");
		for (V3DLONG k=0; k<sz[2]; k++)
			dt2d(p1dt + k*sz[0]*sz[1], label + k*sz[0]*sz[1], sz);
	}
		
	indatap = p1d;
	for (tmpdatap=p1dt; tmpdatap<endp; ++tmpdatap)
	{
		*tmpdatap = sqrt(double(*indatap++));
	}

//	bool b_save_tmpres = false;
//	if (b_save_tmpres)
//	{
//		float *label_float = new float [len];
//		for (V3DLONG i=0;i<len; i++) label_float[i] = label[i];
//		if (b_3d_dt)
//		{
//			saveImage("tmp_dt_3d_dist.raw", (unsigned char *)p1dt, sz, 4);
//			saveImage("tmp_dt_3d_label.raw", (unsigned char *)label_float, sz, 4);
//		}
//		else
//		{
//			saveImage("tmp_dt_2d_dist.raw", (unsigned char *)p1dt, sz, 4);
//			saveImage("tmp_dt_2d_label.raw", (unsigned char *)label_float, sz, 4);
//		}
//		if (label_float) { delete []label_float; label_float=0;}
//	}
	
	if (label) {delete []label; label=0;}
	return true;
}


vector<LocationSimple> find_valid_tip_via_ignore_short_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char *imap,  float *dmap, V3DLONG sz[3])
{
	vector<LocationSimple> mytip;
	if (mmUnit.size()<=0 || mmUnit[0].size()<=0 || !dmap || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0)
	{
		v3d_msg("Wrong parameters in find_valid_tip_via_ignore_short_branches().\n", 0);
		return mytip;
	}
	
	V3DLONG i,j;
	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	
	vector<V3DLONG> leafArray, branchPointArray;
	V3DLONG tnodes = mUnit.size();
	for (i=0;i<tnodes; i++)
	{
		int nc = mUnit.at(i).nchild;
		if (nc==0) 
			leafArray.push_back(i);
		else if (nc>=2)
			branchPointArray.push_back(i);
	}
	
	printf("total leaf nodes = [%ld], branching nodes = [%ld]\n", leafArray.size(), branchPointArray.size());
	for (j=0;j<leafArray.size();j++)
	{
		V3DLONG ci = leafArray.at(j); 
		//printf("ini ci=%ld nchild=%ld cp=%ld\n", ci, V3DLONG(mUnit.at(ci).nchild), V3DLONG(mUnit.at(ci).parent));
		V3DLONG cnodes = 0;
		double tlen = 0, tsum = 0;
		while (mUnit.at(ci).nchild<2 && mUnit.at(ci).parent>=0 && cnodes<tnodes)
		{
			V_NeuronSWC_unit & curnode = mUnit.at(ci);
			V3DLONG cp = V3DLONG(mUnit.at(ci).parent);
			V_NeuronSWC_unit & curpnode = mUnit.at(cp);
			if (ci==cp) {cnodes=tnodes; break;} //why this happens?
			tlen += sqrt(distL2square(curnode, curpnode));
			tsum += imap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)];
			cnodes++;
			ci = cp;
		}
		
		if (cnodes<tnodes)
		{
			V_NeuronSWC_unit & curpnode = mUnit.at(ci);
			double curd = dmap[V3DLONG(curpnode.z)*sz[0]*sz[1] + V3DLONG(curpnode.y)*sz[0] + V3DLONG(curpnode.x)];
			if (tlen > 5 &&         // the path is "long" enough
				tlen > 2.0*curd &&  // the path is significantly long
				curd >=2 &&         // the path is long with repsect a significant branching point
				tsum/tlen > 20      // the path is bright
				) //then output it as a valid tip point
			{
				LocationSimple p;
				V_NeuronSWC_unit curnode = mUnit.at(leafArray.at(j));
				p.x = curnode.x; p.y = curnode.y; p.z = curnode.z; 
				mytip.push_back(p);
				printf("Just add another potential tip [cur # = %ld  cnodes=%ld tlen=%5.4f  r(bp)=%5.4f]\n", mytip.size(), cnodes, tlen, curd);
			}
		}
		
	}
	
	return mytip;
}


bool condense_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness)
{
#define MAXSR 10
#define transformDval2SR(x) ((MAXSR-x < 1) ? 1 : (MAXSR-x))
	//#define transformDval2SR(x) (10)
	//#define transformDval2SR(x) ((x >10 ) ? 10 : (x))

	if (mmUnit.size()<=0 || mmUnit[0].size()<=0 || !imap || !dmap || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0)
	{
		v3d_msg("Wrong parameters in condense_branches().\n", 0);
		return false;
	}
	
	V3DLONG i,j;
	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	V3DLONG tnodes = mUnit.size();
	
	//first propduce a LUT for swc access
	std::map<double,V3DLONG> index_map;	index_map.clear();
	vector <unsigned char> visited;
	for (j=0; j<tnodes; j++)
	{
		double ndx   = mUnit[j].n;
		V3DLONG new_ndx = index_map.size(); //map the neuron node'd id to row number
		index_map[ndx] = new_ndx;
		visited.push_back(0); //set as non-visited
	}
	
	//condensing the spatial locations of all reconstruction nodes
	for (i=0; i<tnodes; i++)
	{
		V3DLONG pi = mUnit[i].parent;
		if (pi<0) //no need to deformable the root 
			continue;
		V_NeuronSWC_unit & curpnode = mUnit[index_map[pi]]; 
		
		V_NeuronSWC_unit & curnode = mUnit[i]; 
		
		if (curnode.nchild!=0)
			continue;
		
		double cur_dval = dmap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)];
		double cur_searchr = transformDval2SR(cur_dval);

		int method_code=1;
		if (method_code==1)
		{
			int tmpcnt=0;
			while (1)
			{
				tmpcnt++;
				float x = curnode.x, y = curnode.y, z = curnode.z;
				//qDebug()<<cur_searchr << " ";
				fitPosition(imap, sz[0], sz[1], sz[2], 0, cur_searchr, x, y, z,  0, 1);
				if ((curnode.x - x)*(curnode.x - x) + (curnode.y - y)*(curnode.y - y) + (curnode.z - z)*(curnode.z - z) < 1.0)
				{
					//qDebug() << "tmpcnt=" << tmpcnt;
					break;
				}
				curnode.x = x, curnode.y = y, curnode.z = z;
				cur_dval = dmap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)];
				cur_searchr = transformDval2SR(cur_dval);
			}
		}		
		else  
		{
			float x = curnode.x, y = curnode.y, z = curnode.z;
			fitPosition(imap, sz[0], sz[1], sz[2], 0, cur_searchr, x, y, z,  0, 1);
			if ((curnode.x - x)*(curnode.x - x) + (curnode.y - y)*(curnode.y - y) + (curnode.z - z)*(curnode.z - z) > 1.0)
			{
				curnode.x = curpnode.x;
				curnode.y = curpnode.y;
				curnode.z = curpnode.z;
			}
		}

		if (i%1000==0)
			printf("%ld ", i);
	}
	printf("\n");
	
	return true;
	
	//rearrange index
	
	rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	
	printf("done with the condense_branches() step. \n");
	
	return true;
}



V3DLONG pruning_covered_leaf_single_cover(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness)
{
    v3d_msg("pruning_covered_leaf_single_cover()\n", 0);
    _CHECK_PRUNING_PARAMETERS_();
	
	V3DLONG i,j;
	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	V3DLONG tnodes = mUnit.size();

	//first propduce a LUT for swc access
	std::map<double,V3DLONG> index_map;	index_map.clear();
	vector <unsigned char> visited;
	V3DLONG root_id=-1;
	for (j=0; j<tnodes; j++)
	{
		double ndx   = mUnit[j].n;
		V3DLONG new_ndx = index_map.size(); //map the neuron node'd id to row number
		index_map[ndx] = new_ndx;
		visited.push_back(0); //set as non-visited
		
		if (mUnit[j].parent<0)
		{
			if (root_id!=-1)
				printf("==================== detect a non-unique root!\n");
			root_id = V3DLONG(mUnit[j].n);
			printf("==================== nchild of root [%ld, id=%ld] = %ld\n", j, V3DLONG(mUnit[j].n), V3DLONG(mUnit[j].nchild));
		}
	}

	V3DLONG nleafdelete;
	V3DLONG nloops = 0;
	while (1)
	{
		nleafdelete = 0;
		nloops++;
		
		for (i=0; i<tnodes; i++)
		{
			if (mUnit[i].nchild!=0) //do not start from a non-leaf node
				continue;
			
			V3DLONG pi = mUnit[i].parent;
			if (pi<0 || pi==mUnit[i].n) //isolated nodes should be removed
			{
				mUnit[i].nchild = -1; //mark to delete
				nleafdelete++;
				continue;
			}
			
			V3DLONG tmpcnt=0;
			j = i; 
			V_NeuronSWC_unit & curnode = mUnit[i]; 
			
			double cur_margin = curnode.r; 
			//if (cur_margin > dmap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)]) cur_margin = dmap[V3DLONG(curnode.z)*sz[0]*sz[1] + V3DLONG(curnode.y)*sz[0] + V3DLONG(curnode.x)];
			//cur_margin = 0;
			while (tmpcnt<10) //to avoid loops
			{
				tmpcnt++;
				
				pi = mUnit[j].parent;
				V3DLONG pi_rownum = index_map[pi];
				if (mUnit[j].n == pi) //note should use .n == .parent, but not j==.parent! Also this condition should never be met, anyway.
				{
					mUnit[j].nchild = -1;
					nleafdelete++;
					printf("**************** exit at loopy node. tmpcnt=%ld\n", tmpcnt); 
					break;
				}
				
				if (pi<0)
					break;
				
				V_NeuronSWC_unit & curpnode = mUnit[pi_rownum]; 
				double tmpd = sqrt(distL2square(curnode, curpnode));
				double pi_radius = curpnode.r; 
				//if (pi_radius < dmap[V3DLONG(curpnode.z)*sz[0]*sz[1] + V3DLONG(curpnode.y)*sz[0] + V3DLONG(curpnode.x)]) pi_radius = dmap[V3DLONG(curpnode.z)*sz[0]*sz[1] + V3DLONG(curpnode.y)*sz[0] + V3DLONG(curpnode.x)];
				if (tmpd + cur_margin <= pi_radius ||  //stop when it is out of the control-range (defined by radius)
					calculate_overlapping_ratio_n1(curnode, curpnode, imap,  sz, trace_z_thickness)>0.9)
				{
					//if (tmpcnt>1) printf("exit at decreasing r (j=%ld) and tmpcnt>=2. tmpcnt=%ld\n", j, tmpcnt);
					curnode.nchild = -1;
					mUnit[index_map[curnode.parent]].nchild--;
					nleafdelete++;
					
					if (V3DLONG(mUnit[index_map[curnode.parent]].n)==root_id)
						printf("root nchild after decreasing 1 = %ld cur node id =%ld, cur node parent=%ld\n", V3DLONG(mUnit[index_map[curnode.parent]].nchild), V3DLONG(curnode.n), V3DLONG(curnode.parent));
					break; 
				}

				j = pi_rownum; //continue
			}
		}
		
		npruned += nleafdelete;
		if (nleafdelete==0)
		{
			printf("No more leaf should be deleted. total loops = [%ld].\n", nloops);
			break;
		}
		else
		{
			printf("delete [%ld] leaf-nodes in this [%ld] run.\n", nleafdelete, nloops);
		}
	}
	
	//delete those deleted labels and rearrange index 
	
	rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	
	printf("done with the pruning_covered_leaf_single_cover() step. \n");
	
	return npruned;
}


V3DLONG pruning_covered_leaf_multi_covers(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness)
{
	v3d_msg("pruning_covered_leaf_multi_covers(). \n", 0);
    _CHECK_PRUNING_PARAMETERS_();
    
	V3DLONG nx=sz[0], ny=sz[1], nz=sz[2];
	
	V3DLONG num_nodes = nx*ny*nz;
	V3DLONG i,j,k,n,m;
	
	unsigned char ***p3d = imap;
	
	//create memory for flag and radius
	V3DLONG stacksz = nx*ny*nz;
	float * maxr_img_1d = 0, *** maxr_img_3d = 0; 
	V3DLONG * maxid_img_1d = 0, *** maxid_img_3d = 0; 
	unsigned char * b_pixelusedbynode_1d = 0, *** b_pixelusedbynode_3d = 0;
	try
	{
		maxr_img_1d = new float [stacksz]; SET_ALLELEMENTS_ZEROS(maxr_img_1d, stacksz);
		new3dpointer(maxr_img_3d, nx, ny, nz, maxr_img_1d);
		maxid_img_1d = new V3DLONG [stacksz]; SET_ALLELEMENTS_ZEROS(maxid_img_1d, stacksz);
		new3dpointer(maxid_img_3d, nx, ny, nz, maxid_img_1d);
		
		b_pixelusedbynode_1d = new unsigned char [stacksz]; SET_ALLELEMENTS_ZEROS(b_pixelusedbynode_1d, stacksz);
		new3dpointer(b_pixelusedbynode_3d, nx, ny, nz, b_pixelusedbynode_1d);
	}
	catch(...)
	{
		v3d_msg("Fail to allocate memory in pruning_covered_leaf_multi_covers();\n", 0);
		if (maxr_img_3d) {delete3dpointer(maxr_img_3d, nx, ny, nz);};
		if (maxr_img_1d) {delete []maxr_img_1d; maxr_img_1d = 0;}
		if (maxid_img_3d) {delete3dpointer(maxid_img_3d, nx, ny, nz);};
		if (maxid_img_1d) {delete []maxid_img_1d; maxid_img_1d = 0;}
		
		if (b_pixelusedbynode_3d) {delete3dpointer(b_pixelusedbynode_3d, nx, ny, nz);};
		if (b_pixelusedbynode_1d) {delete []b_pixelusedbynode_1d; b_pixelusedbynode_1d = 0;}
		return npruned;
	}
	
	//find the covering nodes and sort them from large to small

	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	V3DLONG tnodes = mUnit.size();
	std::map<double,V3DLONG> index_map;	index_map.clear();

	for(i=0; i<tnodes; i++)
	{
		V_NeuronSWC_unit & curnode = mUnit[i];
		b_pixelusedbynode_3d[V3DLONG(curnode.z)][V3DLONG(curnode.y)][V3DLONG(curnode.x)] = 1;

		double ndx   = mUnit[i].n;
		index_map[ndx] = i; //map the neuron node'd id to row number
	}
	
	std::deque< list<V3DLONG> > cover_list; //record for the pixel location the covering list
	std::map<V3DLONG,V3DLONG> index_map_pixelloc; //map the pixel location to the location in cover_list
	
	for(i=0; i<tnodes; i++)
	{
		V_NeuronSWC_unit & curnode = mUnit[i];
		double curnx = curnode.x, curny = curnode.y, curnz = curnode.z;
		
		V3DLONG i0 = curnx - curnode.r; PUT_IN_RANGE(0, i0, nx-1);
		V3DLONG	i1 = curnx + curnode.r; PUT_IN_RANGE(0, i1, nx-1);
		V3DLONG j0 = curny - curnode.r; PUT_IN_RANGE(0, j0, ny-1);
		V3DLONG	j1 = curny + curnode.r; PUT_IN_RANGE(0, j1, ny-1);
		V3DLONG k0 = curnz - curnode.r; PUT_IN_RANGE(0, k0, nz-1);
		V3DLONG	k1 = curnz + curnode.r; PUT_IN_RANGE(0, k1, nz-1);
			
		double d, r2 = curnode.r * curnode.r;
		for (V3DLONG k=k0;k<=k1;k++)
			for (V3DLONG j=j0;j<=j1;j++)
				for (V3DLONG ii=i0;ii<=i1;ii++)
				{
					if (b_pixelusedbynode_3d[k][j][ii]==0) //if this pixel location does not correspond to the center of a reconstruction node, then do nothing
						continue;
					
					double d = sqrt(double(ii-curnx)*(ii-curnx) + double(j-curny)*(j-curny) + double(k-curnz)*(k-curnz));
					if (d > r2)
						continue;
					
					if (b_pixelusedbynode_3d[k][j][ii]==1) //then the list has not been created yet, so to create it
					{
						list<V3DLONG> mylist_n;
						mylist_n.push_back(V3DLONG(curnode.n));  
						cover_list.push_back(mylist_n);
						index_map_pixelloc[k*nx*ny+j*nx+ii] = cover_list.size()-1;
						b_pixelusedbynode_3d[k][j][ii]=2; //2 means the list has been created so next time no need to create it again
					}
					else
					{
						list<V3DLONG> & mylist_n = cover_list[ index_map_pixelloc[k*nx*ny+j*nx+ii] ];
						list<V3DLONG>::iterator it = mylist_n.begin();
						for ( it=mylist_n.begin() ; it != mylist_n.end(); it++ )
							if (curnode.r > mUnit[index_map[*it]].r)
							{
								mylist_n.insert(it, V3DLONG(curnode.n));
								break;
							}
						if (it==mylist_n.end()) //if the radius is smaller than any existing one, then insert at the end
							mylist_n.insert(it, curnode.n);
						
						//for ( it=mylist_n.begin() ; it != mylist_n.end(); it++ ) printf("%5.3f ", mUnit[index_map[*it]].r); printf("\n");
					}
					
					if ( maxr_img_3d[k][j][ii] < curnode.r)
					{
						maxr_img_3d[k][j][ii] = curnode.r;
						maxid_img_3d[k][j][ii] = curnode.n;
					}
				}
		
	}
	
	//remove the unneeded neuron node
	
	V3DLONG nleafdelete;
	V3DLONG nloops = 0;
	while (1)
	{
		nleafdelete = 0;
		nloops++;
		
		for (i=0; i<tnodes; i++)
		{
			V_NeuronSWC_unit & curnode = mUnit[i]; 

			if (curnode.nchild!=0) //do not start from a non-leaf node or start from an already deleted node
				continue;
			
			V3DLONG pi = mUnit[i].parent;
			if (pi<0 || pi==mUnit[i].n) //isolated nodes should be removed
			{
				mUnit[i].nchild = -1; //mark to delete
				nleafdelete++;
				continue;
			}
			
			double cur_maxid = maxid_img_3d[V3DLONG(curnode.z)][V3DLONG(curnode.y)][V3DLONG(curnode.x)];
			if (cur_maxid==curnode.n) //keep itself
				continue; 
			
			V_NeuronSWC_unit & curmnode = mUnit[index_map[cur_maxid]]; 
			
			if (curmnode.nchild<0) //if the best convering node has been removed, then do nothing
				continue; 
			
			list<V3DLONG> & mylist_n = cover_list[ index_map_pixelloc[V3DLONG(curnode.z)*nx*ny+V3DLONG(curnode.y)*nx+V3DLONG(curnode.x)] ];
			
			double tmpd = sqrt(distL2square(curnode, curmnode));
			
			//printf("curnode (n=%ld) curmnode (n=%ld) index_map best = [%ld]\n", V3DLONG(curnode.n), V3DLONG(curmnode.n), V3DLONG(mUnit[index_map[*mylist_n.begin()]].n)); 
			if (tmpd + curnode.r <= curmnode.r   //stop when it is out of the control-range (defined by radius)
				//|| calculate_overlapping_ratio_n1(curnode, curmnode, imap,  sz, trace_z_thickness)>0.9 // stop if this node can be effectively covered by another single node
				|| calculate_overlapping_ratio_n1(curnode, mUnit, index_map, mylist_n, imap,  sz, trace_z_thickness)>0.9 // stop if this node can be covered by several other nodes
				)
			{
				curnode.nchild = -1;
				mUnit[index_map[curnode.parent]].nchild--;
				nleafdelete++;
			}
		}
		
		npruned += nleafdelete;
		if (nleafdelete==0)
		{
			printf("No more leaf should be deleted. total loops = [%ld].\n", nloops);
			break;
		}
		else
		{
			printf("delete [%ld] leaf-nodes in this [%ld] run.\n", nleafdelete, nloops);
		}
	}
	
	//rearrange index
	
	rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	
	printf("done with the pruning_covered_leaf_single_cover() step. \n");
	
	
Label_Exit_filterNodesByFindingBestCovers:
	if (maxr_img_3d) {delete3dpointer(maxr_img_3d, nx, ny, nz);};
	if (maxr_img_1d) {delete []maxr_img_1d; maxr_img_1d = 0;}
	if (maxid_img_3d) {delete3dpointer(maxid_img_3d, nx, ny, nz);};
	if (maxid_img_1d) {delete []maxid_img_1d; maxid_img_1d = 0;}
	
	if (b_pixelusedbynode_3d) {delete3dpointer(b_pixelusedbynode_3d, nx, ny, nz);};
	if (b_pixelusedbynode_1d) {delete []b_pixelusedbynode_1d; b_pixelusedbynode_1d = 0;}

	
	return npruned;
}


struct V3DLONGTRIPLE
{
	V3DLONG id;
	V3DLONG attr;
	V3DLONG ind;
};

bool cmp_V3DLONGTRIPLE(V3DLONGTRIPLE a, V3DLONGTRIPLE b)
{
	return (a.attr < b.attr) ? true : false;
}

V3DLONG pruning_covered_leaf_closebyfake_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness, double imgTH)
{
	v3d_msg("pruning_covered_leaf_closebyfake_branches(). \n", 0);
    _CHECK_PRUNING_PARAMETERS_();
	
	V3DLONG i,j,k,n,m;
	V3DLONG nx=sz[0], ny=sz[1], nz=sz[2];
	
	unsigned char ***p3d = imap;
	
	//create memory for flag and radius
	V3DLONG stacksz = nx*ny*nz;
	V3DLONG * ind_pixelusedbynode_1d = 0, *** ind_pixelusedbynode_3d = 0;
	try
	{
		ind_pixelusedbynode_1d = new V3DLONG [stacksz]; 
		new3dpointer(ind_pixelusedbynode_3d, nx, ny, nz, ind_pixelusedbynode_1d);
	}
	catch(...)
	{
		if (ind_pixelusedbynode_3d) {delete3dpointer(ind_pixelusedbynode_3d, nx, ny, nz);};
		if (ind_pixelusedbynode_1d) {delete []ind_pixelusedbynode_1d; ind_pixelusedbynode_1d = 0;}
		return npruned;
	}
	
	while (1)
	{
		V3DLONG npruned_branches = 0, npruned_curloop=0;
		
		//find the occupied indexes 
		
		vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
		V3DLONG tnodes = mUnit.size();
		std::map<double,V3DLONG> index_map;	index_map.clear();
		
		SET_ALLELEMENTS_ZEROS(ind_pixelusedbynode_1d, stacksz); //do NOT forget this!
		for(i=0; i<tnodes; i++)
		{
			V_NeuronSWC_unit & curnode = mUnit[i];
			ind_pixelusedbynode_3d[V3DLONG(curnode.z)][V3DLONG(curnode.y)][V3DLONG(curnode.x)] = curnode.n+1; //use +1 so that the indexing will have no problem for the 0th node
			
			double ndx   = mUnit[i].n;
			index_map[ndx] = i; //map the neuron node's id to row number
		}
		
		//find the leaf node array and sort them from based on their length
		
		vector < vector <V3DLONG> > LeafBranchPath;
		list <V3DLONGTRIPLE> LeafArray;
		for(i=0; i<tnodes; i++)
		{
			if (mUnit[i].nchild!=0)
				continue;
			
			V3DLONG cur_ind = i;
			vector <V3DLONG> branchPath; branchPath.clear();
			while (1)
			{
				V_NeuronSWC_unit & curnode = mUnit[cur_ind];
				branchPath.push_back(curnode.n);
				
				V3DLONG pi = curnode.parent;
				if (pi<0 || pi==curnode.n) //isolated nodes should be removed. Note that here I don't really consider pi<0 case as it should be removed in preceding steps
				{
					break;
				}
				V_NeuronSWC_unit & curpnode = mUnit[index_map[pi]];
				if (curpnode.nchild>=2 || curpnode.parent<0) //either a branching point or a root
				{
					branchPath.push_back(curpnode.n); //note that I also add the branching point/root in the path, thus later-on to remove the path, the last one MUST be skipped
					break;
				}
				
				cur_ind = index_map[pi];
			}
			
			LeafBranchPath.push_back(branchPath);

			V3DLONGTRIPLE vp;
			vp.id = i; vp.attr = branchPath.size(); vp.ind = LeafBranchPath.size()-1;
			LeafArray.push_back(vp);
		}
		
		LeafArray.sort(cmp_V3DLONGTRIPLE);
		{
			list<V3DLONGTRIPLE>::iterator it; V3DLONG li;
			for (it=LeafArray.begin(), li=0; it!=LeafArray.end(); it++, li++)
			{
				i = it->id;
				printf("li = %ld sz=%ld id=%ld ind=%ld leaf node id=%ld\n", li, it->attr, it->id, it->ind, V3DLONG(mUnit[it->id].n));
			}
		}
					   
		//remove the unneeded neuron node
		
		{
			npruned_branches = 0;
			list<V3DLONGTRIPLE>::iterator it; V3DLONG li;
			for (it=LeafArray.begin(), li=0; it!=LeafArray.end(); it++, li++)
			{
				i = it->id;
				
				V_NeuronSWC_unit & curnode = mUnit[i]; 
				vector <V3DLONG> & curnodepathindex = LeafBranchPath[it->ind];
				
				if (curnode.nchild!=0) //do not start from a non-leaf node or start from an already deleted node
				{
					printf("li=%ld [node id=%ld] is not a leaf node. sth wrong. Check!\n", li, V3DLONG(curnode.n));
					continue;
				}
				
				if (mUnit[index_map[curnodepathindex.back()]].nchild<=1) //that means the previous branching point has already no more than one branch left. thus should not delete any more
				{
					printf("li=%ld [node id=%ld] 's previous closest branching point is not long a branching point. Skip!\n", li, V3DLONG(curnode.n));
					continue;
				}
				
				bool b_deletecurbranch = false;
				if (curnodepathindex.size()<=2)
					b_deletecurbranch = true; //since if the leaf node is immediately connected to another branch, the optimal path must have been all foreground 
				else //if (b_deletecurbranch==false)
				{
					V3DLONG pi = mUnit[i].parent;
					if (pi<0 || pi==mUnit[i].n) //isolated nodes should be removed
					{
						mUnit[i].nchild = -1; //mark to delete
						printf("should NOT see this printed. sth wrong. Check!\n");
						continue;
					}

					double curnx = curnode.x, curny = curnode.y, curnz = curnode.z;
					
					V3DLONG i0 = curnx - curnode.r; PUT_IN_RANGE(0, i0, nx-1);
					V3DLONG	i1 = curnx + curnode.r; PUT_IN_RANGE(0, i1, nx-1);
					V3DLONG j0 = curny - curnode.r; PUT_IN_RANGE(0, j0, ny-1);
					V3DLONG	j1 = curny + curnode.r; PUT_IN_RANGE(0, j1, ny-1);
					V3DLONG k0 = curnz - curnode.r; PUT_IN_RANGE(0, k0, nz-1);
					V3DLONG	k1 = curnz + curnode.r; PUT_IN_RANGE(0, k1, nz-1);
					
					V_NeuronSWC_unit & cur_branchingpt = mUnit[index_map[curnodepathindex.back()]];
					double max_r = 50;
					//3.0 + sqrt((curnx-cur_branchingpt.x)*(curnx-cur_branchingpt.x) + (curny-cur_branchingpt.y)*(curny-cur_branchingpt.y) + (curnz-cur_branchingpt.z)*(curnz-cur_branchingpt.z));
					V3DLONG ci, cj, ck;
					for (double ir=1; ir<=max_r; ir++)
					{
						double zlower = -ir/trace_z_thickness, zupper = +ir/trace_z_thickness;
						for (double dz= zlower; dz <= zupper; ++dz)
							for (double dy= -ir; dy <= +ir; ++dy)
								for (double dx= -ir; dx <= +ir; ++dx)
								{
									double r = sqrt(dx*dx + dy*dy + dz*dz);
									if (r>ir-1 && r<=ir)
									{
										ci = curnx+dx;	if (ci<0 || ci>=sz[0]) continue;
										cj = curny+dy;	if (cj<0 || cj>=sz[1]) continue;
										ck = curnz+dz;	if (ck<0 || ck>=sz[2]) continue;
										
										if (ind_pixelusedbynode_3d[ck][cj][ci]>=1) //then this is a valid index number
										{
											V3DLONG actual_index = ind_pixelusedbynode_3d[ck][cj][ci] - 1; //-1 because originally + 1
											bool b_exist = false;
											for (V3DLONG tmpi=0;tmpi<curnodepathindex.size();tmpi++)
											{
												if (curnodepathindex[tmpi]==actual_index)
												{
													b_exist = true;
													break; //the currently encountered node is on the branch path, thus ignore it
												}
											}
											if (b_exist==false) //then this is the nearest reconstruction that is also not on the current branch (and also include the branching point itself)
											{
												//now check if along the straight line all image pixel are foreground. if yes, then this branch should be deleted
												double startx = curnx, starty = curny, startz = curnz; 
												double endx = ci, endy = cj, endz = ck; 
												
												double dd = ceil(sqrt((startx-endx)*(startx-endx)+(starty-endy)*(starty-endy)+(startz-endz)*(startz-endz)));
												double stepx = (endx-startx)/dd, stepy = (endy-starty)/dd, stepz = (endz-startz)/dd;
												
												vector <float> realdatavec, concavevec;
												bool b_allfg = true;
												
												for (V3DLONG tmpi=0; tmpi<dd; tmpi++)
												{
													if (p3d[V3DLONG(startz+stepz*tmpi+0.5)][V3DLONG(starty+stepy*tmpi+0.5)][V3DLONG(startx+stepx*tmpi+0.5)]<=imgTH)
													{
														b_allfg = false;
														break;
													}
													
													realdatavec.push_back(float(p3d[V3DLONG(startz+stepz*tmpi+0.5)][V3DLONG(starty+stepy*tmpi+0.5)][V3DLONG(startx+stepx*tmpi+0.5)]));
													concavevec.push_back(float(fabs((1-tmpi/double(dd)))));
												}
												if (b_allfg==true)
												{
													b_deletecurbranch = true;
//													double cc = corrcoef(realdatavec, concavevec);
//													printf("*** current cc=%5.3f datavec len=%d  imgTH=%5.3f\n", cc, realdatavec.size(), double(imgTH));
//													for (int myii=0;myii<realdatavec.size();myii++)
//														printf("%5.1f ", realdatavec[myii]);
//													printf("\n");
//													for (int myii=0;myii<realdatavec.size();myii++)
//														printf("%5.1f ", concavevec[myii]);
//													printf("\n");
//													if (cc>0.5) //relatively weak positive-correlation
//														b_deletecurbranch = false;
												}
												
												goto Label_pruning_covered_leaf_closebyfake_branches;
											}
										}
									}
								}
						}
						
					}
				
				//
			Label_pruning_covered_leaf_closebyfake_branches:
				if (b_deletecurbranch)
				{
					for (j=0; j<curnodepathindex.size()-1; j++) //use -1 as the last one is the branching point or root and should NOT be removed
						mUnit[index_map[curnodepathindex[j]]].nchild = -1;
					mUnit[index_map[curnodepathindex[curnodepathindex.size()-1]]].nchild--; //also decrease the branching point's nchild by 1
					
					npruned_curloop += curnodepathindex.size()-1;
					npruned_branches++;
				}
			}
			
			printf("delete [%ld] nodes in [%ld] branches.\n", npruned_curloop, npruned_branches);
		}
		
		//rearrange index
		
		rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
		
		npruned += npruned_curloop;
		//
		if (npruned_branches==0)
			break;
	}
	printf("**** totally delete [%ld] nodes closeby-branches pruning.\n", npruned);
	
	//free space
	if (ind_pixelusedbynode_3d) {delete3dpointer(ind_pixelusedbynode_3d, nx, ny, nz);};
	if (ind_pixelusedbynode_1d) {delete []ind_pixelusedbynode_1d; ind_pixelusedbynode_1d = 0;}
	
	return npruned;
}



V3DLONG pruning_branch_nodes(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness)
{
    v3d_msg("pruning branch nodes.\n",0);
    _CHECK_PRUNING_PARAMETERS_();
	
	V3DLONG i,j;
	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	V3DLONG trows = mUnit.size();
	
	//first propduce a LUT for swc access
	std::map<double,V3DLONG> index_map;	index_map.clear();
	vector <unsigned char> visited;
	V3DLONG root_id=-1;
	for (j=0; j<trows; j++)
	{
		double ndx   = mUnit[j].n;
		V3DLONG new_ndx = index_map.size(); //map the neuron node'd id to row number
		index_map[ndx] = new_ndx;
		visited.push_back(0); //set as non-visited
		
		if (mUnit[j].parent<0)
		{
			if (root_id!=-1)
				printf("==================== detect a non-unique root!\n");
			root_id = V3DLONG(mUnit[j].n);
			printf("==================== nchild of root [%ld, id=%ld] = %ld\n", j, V3DLONG(mUnit[j].n), V3DLONG(mUnit[j].nchild));
		}
	}
	
	//create a data structure to hold all nodes' children
	vector < vector < V3DLONG > > childrenList; childrenList.clear();
	for (j=0; j<trows; j++)
	{
		vector<V3DLONG> tmp; tmp.clear();
		childrenList.push_back(tmp);
	}
	
	for (j=0; j<trows; j++)
	{
		if (mUnit[j].parent>=0)
		{
			childrenList[index_map[mUnit[j].parent]].push_back(mUnit[j].n); 
		}
	}
	
	for (j=0; j<trows; j++)
	{
		if (mUnit[j].parent<0)
			for (int k=0;k<mUnit[j].nchild;k++)
				printf("the current root node [%ld] has a child c[%d]=%d ", mUnit[j].n, k, childrenList[index_map[mUnit[j].n]].at(k));
	}
	printf("\n\n\n");
	
	//now prune
	V3DLONG nbranchnodedelete;
	V3DLONG nloops = 0;
	while (nloops<1)
	{
		nbranchnodedelete = 0;
		nloops++;
		
		for (i=0; i<trows; i++)
		{
			V_NeuronSWC_unit & curnode = mUnit[i]; 
			
			if (curnode.nchild<2 || visited[index_map[curnode.n]]) //do not start from a non-branching-node
				continue;
			
			if (index_map[ mUnit[i].n ] != i)
			{
				printf("*** The LUT is corrupted! index_map[mUnit[i].n] = %ld, which != %ld \n", i, mUnit[index_map[i]].n, i);
				continue;
			}

			V3DLONG pi = mUnit[i].parent;
			if (pi>=0)
			if (mUnit[index_map[pi]].n != pi)
			{
				printf("*** The LUT is corrupted! mUnit[index_map[%ld]].n = %ld, which != %ld \n", pi, mUnit[index_map[pi]].n, pi);
				continue;
			}
			
			if (pi==curnode.n) //isolated nodes should be removed
			{
				curnode.nchild = -1; //mark to delete
				nbranchnodedelete++;
				printf("remove an insolated node [%ld]\n", curnode.n);
				continue;
			}
			
			if (pi>=0) 
			{
				if (mUnit[index_map[pi]].nchild<=0) //this node's parent node already becomes a node to be removed or a leaf node (without child)
				{
					printf("potential breaking point detected [i=%ld pi=%ld pi's nchild=%ld]. Skip it.\n", i, pi, V3DLONG(mUnit[index_map[pi]].nchild));
					continue;
				}
			}
			
			V_NeuronSWC_unit & curpinode = mUnit[index_map[pi]]; 

			double cur_margin = curnode.r; 
			for (V3DLONG k=0;k<curnode.nchild;k++)
			{
				//V_NeuronSWC_unit & curcnode = mUnit[index_map[childrenList[index_map[curnode.n]][k]]];
				V_NeuronSWC_unit & curcnode = mUnit[index_map[childrenList[i][k]]]; //should be the same with the above sentence, but save some operations
				
				if (curcnode.nchild==-1) //if a child node has been marked to be deleted, then no need to do it
					continue;
				
				if (curcnode.parent!=curnode.n)
				{
					printf("Very wrong detected! curnode.n=%ld  curcnode.n=%ld\n", curnode.n, curcnode.n);
					continue;
				}
				
				double tmpd = sqrt(distL2square(curnode, curcnode));
				double c_radius = curcnode.r; 
				double overlap_ratio=0;

				if (tmpd <= cur_margin + c_radius &&  //stop when it is out of the control-range (defined by radius)
					(overlap_ratio = calculate_overlapping_ratio_n1(curnode, curcnode, imap,  sz, trace_z_thickness))>0.9) //0.7 also work for Rubin1 data //when this threshold is lowered, the neuron will be broken, why?
				{
					printf("distance of two nodes=%5.3f curnode.r=%5.3f curcnode.r=%5.3f overlap ratio=%5.3f\n", tmpd, curnode.r, curcnode.r, overlap_ratio);
					
					if (1) //merge locations
					{
						curcnode.x = (curnode.x+curcnode.x)/2.0;
						curcnode.y = (curnode.y+curcnode.y)/2.0;
						curcnode.z = (curnode.z+curcnode.z)/2.0;
						curcnode.r = (curnode.r+curcnode.r + tmpd)/2.0;
					}
					
					if (1)
					{
						curcnode.parent = pi; //set the currently used/merged child node's parent to the current branch-node's parent
						
						if (pi<0) printf("***** set the current child's parent to <0, so it [current child node id=%ld] becomes a root!\n", curcnode.n);
						
						V3DLONG tmpk;
						
						for (tmpk=0; tmpk<curnode.nchild;tmpk++) //update the curcnode' children list
						{
							if (tmpk==k) continue;
							V_NeuronSWC_unit & anothercnode = mUnit[index_map[childrenList[index_map[curnode.n]][tmpk]]];
							
							if (anothercnode.nchild==-1) //if the other child node has been marked to be deleted, then no need to do it
								continue;
							
							anothercnode.parent = curcnode.n; //set all other children's parents to be the current used/merged child node
							visited[index_map[anothercnode.n]] = 1;
							
							//also update the children list
							childrenList[index_map[curcnode.n]].push_back(anothercnode.n);
							curcnode.nchild++;
						}
						
						if (pi>=0)
						{
							for (tmpk=0; tmpk<curpinode.nchild;tmpk++) //update the pi's children list
							{
								V_NeuronSWC_unit & anothercnode = mUnit[index_map[childrenList[index_map[curpinode.n]][tmpk]]];
								if (anothercnode.n == curnode.n)
								{
									childrenList[index_map[curpinode.n]][tmpk] = curcnode.n; //using the current child node to replace the current node in pi's children list
									break;
								}
							}
						}

						nbranchnodedelete++;
						
						curnode.nchild = -1; //set the curnode to be removed
						
						visited[index_map[curcnode.n]] = 1; //force not to touch the just-changed child-node again (to avoid the potential breaking connectedness of the graph)
						
						break; //do not allow multiple deletions
					}
				}
				
			}
			
		}
		
		npruned += nbranchnodedelete;
		if (nbranchnodedelete==0)
		{
			printf("No more branch node should be deleted. total loops = [%ld].\n", nloops);
			break;
		}
		else
		{
			printf("delete [%ld] branch-nodes in this [%ld] run.\n", nbranchnodedelete, nloops);
		}
	}
	
	//delete those deleted labels and rearrange index 
	
	rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	
	printf("done with the pruning_branch_nodes() step. \n");
	
	return npruned;
}




V3DLONG pruning_internodes(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness)
{
	v3d_msg("pruning_internodes(). \n", 0);
    _CHECK_PRUNING_PARAMETERS_();
	
	V3DLONG i,j;
	
	//build the index map 
	
	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	V3DLONG tnodes = mUnit.size();
	std::map<double,V3DLONG> index_map;	index_map.clear();
	
	for(i=0; i<tnodes; i++)
	{
		V_NeuronSWC_unit & curnode = mUnit[i];
		double ndx   = mUnit[i].n;
		index_map[ndx] = i; //map the neuron node'd id to row number
	}
	
	//find the starting points

	vector <V3DLONG> leaf_branching_set;
	for (i=0; i<tnodes; i++)
	{
		V_NeuronSWC_unit & curnode = mUnit[i]; 
		
		if ((curnode.nchild==0 || curnode.nchild>=2) && curnode.parent>=0) //do not start from a non-leaf or non-branching node or root 
			leaf_branching_set.push_back(i);
	}
		
	//actual pruning
	
	V3DLONG ncurdelete = 0; //# of nodes removed in the current loop
	for (j=0; j<leaf_branching_set.size(); j++)
	{
		i = leaf_branching_set[j];
		V_NeuronSWC_unit & curnode = mUnit[i]; 
		
		V3DLONG cur_row = i;
		while (1) //loop until a root or branching point has been reached
		{
			V_NeuronSWC_unit & curnode = mUnit[cur_row];

			V3DLONG pi = curnode.parent;
			if (pi<0 || pi==curnode.n) //isolated nodes should be removed
			{
				curnode.nchild = -1; //mark to delete
				ncurdelete++;
				break;
			}

			V3DLONG parent_row = index_map[pi];
			//printf("parent_node_row = %ld cur_row = %ld\n", parent_row, cur_row);
			if (parent_row == cur_row)
			{
				printf("strange, should not happen.\n");
				break;
			}

			V_NeuronSWC_unit & curpnode = mUnit[parent_row]; 

			if (curpnode.parent < 0) //root node reached
			{
				//printf("root reached.\n");
				break;
			}
			if (curpnode.nchild >= 2) //branching point reached
			{
				//printf("branching pt reached.\n");
				break;
			}
			
			double tmpd = sqrt(distL2square(curnode, curpnode));

			if (tmpd + curpnode.r <= curnode.r   //remove when it is out of the control-range (defined by radius)
				|| calculate_overlapping_ratio_n1(curpnode, curnode, imap,  sz, trace_z_thickness)>0.1  // stop if this node can be effectively covered, use 0.1 as inter-nodes can be well separated 
				)
			{
				curpnode.nchild = -1;
				curnode.parent = curpnode.parent;
				ncurdelete++;
				//cur_row remains the same
			}
			else //do not delete, then keep the current, and move to the next
			{
				cur_row = index_map[curpnode.n];
			}
		} 
			
	}
		
	printf("delete [%ld] inter-nodes.\n", ncurdelete);
	
	//rearrange index
	
	rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	
	printf("done with the pruning_internodes() step. \n");
	
	//
	return (npruned = ncurdelete);
}


V3DLONG pruning_artifacial_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness, double VISIBLE_THRESHOLD)
{
    v3d_msg("pruning_artifacial_branches()\n", 0);
    _CHECK_PRUNING_PARAMETERS_();

	double vt = VISIBLE_THRESHOLD; //50
	V3DLONG i,j;
	vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(0);
	V3DLONG tnodes = mUnit.size();
    
	//first propduce a LUT for swc access
	std::map<double,V3DLONG> index_map;	index_map.clear();
	vector <unsigned char> visited;
	V3DLONG root_id=-1;
	for (j=0; j<tnodes; j++)
	{
		double ndx   = mUnit[j].n;
		V3DLONG new_ndx = index_map.size(); //map the neuron node'd id to row number
		index_map[ndx] = new_ndx;
		visited.push_back(0); //set as non-visited
		
		if (mUnit[j].parent<0)
		{
			if (root_id!=-1)
				printf("==================== detect a non-unique root!\n");
			root_id = V3DLONG(mUnit[j].n);
			printf("==================== nchild of root [%ld, id=%ld] = %ld\n", j, V3DLONG(mUnit[j].n), V3DLONG(mUnit[j].nchild));
		}
	}
    
	V3DLONG nleafdelete;
	V3DLONG nloops = 0;
	while (1)
	{
		nleafdelete = 0;
		nloops++;
		
		for (i=0; i<tnodes; i++)
		{
			if (mUnit[i].nchild!=0) //do not start from a non-leaf node
				continue;
			
			V3DLONG pi = mUnit[i].parent;
			if (pi<0 || pi==mUnit[i].n) //isolated nodes should be removed
			{
				mUnit[i].nchild = -1; //mark to delete
				nleafdelete++;
				continue;
			}
			
			j = i; 
			
			double totalDarkEdgeLength = 0; 
			while (1) 
			{
                V_NeuronSWC_unit & curnode = mUnit[j]; 
				pi = curnode.parent;
				V3DLONG pi_rownum = index_map[pi];
				if (curnode.n == pi) //note should use .n == .parent, but not j==.parent! Also this condition should never be met, anyway.
				{
					curnode.nchild = -1;
					nleafdelete++;
					printf("**************** exit at loopy node. \n"); 
					break;
				}
				
				if (pi<0)
					break;
				
				V_NeuronSWC_unit & curpnode = mUnit[pi_rownum]; 
                
                double curEdgeMeanInt = computeEdgeMeanVal(curnode, curpnode, imap, sz);
                double curEdgeLength = distL2square(curnode, curpnode);
                
                if (curEdgeMeanInt < vt)
                    totalDarkEdgeLength += curEdgeLength;
                    
                if (curEdgeMeanInt >= vt) 
				{ 
                    if (totalDarkEdgeLength > 30)
                    {
                        V3DLONG k=i;
                        while (k!=index_map[curnode.n])
                        {
                            V_NeuronSWC_unit & mynode = mUnit[k];
                            mynode.nchild = -1;
                            V3DLONG mypi = index_map[mynode.parent];
                            mUnit[mypi].nchild--;
                            nleafdelete++;

                            if (V3DLONG(mUnit[mypi].n)==root_id)
                                printf("root nchild after decreasing 1 = %ld cur node id =%ld, cur node parent=%ld\n", V3DLONG(mUnit[index_map[curnode.parent]].nchild), V3DLONG(curnode.n), V3DLONG(curnode.parent));
                            
                            k = mypi;
                        }
                        break;
                    }
                    else
                        totalDarkEdgeLength = 0; //reset the search when a bright edge is detected
                }

                j = pi_rownum; //continue
			}
		}
		
		npruned += nleafdelete;
		if (nleafdelete==0)
		{
			printf("No more leaf should be deleted. total loops = [%ld].\n", nloops);
			break;
		}
		else
		{
			printf("delete [%ld] leaf-nodes in this [%ld] run.\n", nleafdelete, nloops);
		}
	}
	
	//delete those deleted labels and rearrange index 
	
	rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	
	printf("done with the pruning_artifacial_branches() step. \n");
	
	return npruned;
}


