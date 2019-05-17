//#define INF 1E20
//#define fabs(x) ((x<0)?-x:x)
//#define fitRadius fitRadiusPercent
//#define CHECK_DATA_GD_TRACING(AA) \
//    if (!p4d || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 ) \
//    { \
//        v3d_msg("Invalid image and sz parameters.\n", 0); \
//        throw("Invalid image and sz parameters.\n"); \
//        return AA; \
//    }

//typedef double     Weight;
//typedef V3DLONG    Node;
//typedef std::pair<Node, Node> Edge;


//#include <boost/config.hpp>
//#include <boost/math/special_functions/fpclassify.hpp>
//using namespace boost;
//static
//struct edge_table_item {
//    V3DLONG i0,j0,k0,  i1,j1,k1;  double dist;
//} edge_table[13] = {           // in one cube
//    {0,0,0, 1,0,0, 1},         // 3 dist=1
//    {0,0,0, 0,1,0, 1},
//    {0,0,0, 0,0,1, 1},
//    {0,0,0, 1,1,0, sqrt(2.0)}, // 6 dist=sqrt(2)
//    {0,0,0, 0,1,1, sqrt(2.0)},
//    {0,0,0, 1,0,1, sqrt(2.0)},
//    {1,0,0, 0,1,0, sqrt(2.0)},
//    {0,1,0, 0,0,1, sqrt(2.0)},
//    {0,0,1, 1,0,0, sqrt(2.0)},
//    {0,0,0, 1,1,1, sqrt(3.0)}, // 4 dist=sqrt(3)
//    {1,0,0, 0,1,1, sqrt(3.0)},
//    {0,1,0, 1,0,1, sqrt(3.0)},
//    {0,0,1, 1,1,0, sqrt(3.0)},
//};


//struct ParaShortestPath
//{
//    int node_step; //should be >=1 && odd.
//    int outsample_step;
//    int smooth_winsize;
//    int edge_select;  //0 -- only use length 1 edge(optimal for small step), 1 -- plus diagonal edge
//    int background_select; //0 -- no background, 1 -- compute background threshold
//    double imgTH;
//    double visible_thresh;

//    bool b_use_favorite_direction; //add by PHC 170606
//    double favorite_direction[3];

//    int downsample_method; //0 -- use average, 1 -- use max //added by Zhi 20170925

//    ParaShortestPath()
//    {
//        node_step = 3; //should be >=1
//        outsample_step = 2;
//        smooth_winsize = 5;
//        edge_select = 0;  //0 -- bgl_shortest_path(), 1 -- phc_shortest_path()
//        background_select = 1;
//        imgTH = 0; //do not tracing image background
//        visible_thresh = 30;

//        b_use_favorite_direction = false;
//        favorite_direction[0] = favorite_direction[1] = favorite_direction[2] = 0;

//        downsample_method = 0;
//    }
//};

//struct V_NeuronSWC_coord    //for sort
//{
//    union {
//    double data[3];
//    struct { double x,y,z; };
//    };
//    bool equal(const V_NeuronSWC_coord & other) const {if (x==other.x && y==other.y && z==other.z) return true; else return false;}
//    bool equal(double x1, double y1, double z1) const {if (x==x1 && y==y1 && z==z1) return true; else return false;}
//    bool nonequal(const V_NeuronSWC_coord &other) const {return !equal(other);}
//    bool nonequal(double x1, double y1, double z1) const {return !equal(x1, y1, z1);}
//    void set(double x1, double y1, double z1) {x=x1; y=y1; z=z1;}
//};

//struct V_BranchUnit // for ranking segment hierarchy, used in loop detection
//{
//    V_BranchUnit() { isBranch = false; x = 0; y = 0; z = 0; paID = 0; hierarchy = 0; }
//    bool isBranch;
//    float x, y, z;
//    int ID, paID;
//    int hierarchy;
//    vector<int> childIDs;

//    int segLoc, segPaLoc;
//    vector<int> childSegLocs;
//};

//struct V_NeuronSWC_unit
//{
//    union {
//        double data[10];
//        struct {
//            double n, type, x, y, z, r, parent,
//            nchild,
//            seg_id, nodeinseg_id,
//            level;
//        };
//    };
//        V_NeuronSWC_unit() {for (V3DLONG i=0;i<V3DLONG(sizeof(data)/sizeof(double));i++) data[i]=0; r=0.5;}
//    operator V_NeuronSWC_coord() {V_NeuronSWC_coord c; c.x=x; c.y=y; c.z=z; return c;}
//    V_NeuronSWC_coord get_coord() {V_NeuronSWC_coord c; c.x=x; c.y=y; c.z=z; return c;}
//    void set(double x1, double y1, double z1, double r1, double p1, double t1) {x=x1; y=y1; z=z1; r=r1;parent=p1;type=t1;}
//    void set(double x1, double y1, double z1, double r1, double p1) {x=x1; y=y1;z=z1;r=r1;parent=p1;}
//    void set(double x1, double y1, double z1, double r1) {x=x1; y=y1;z=z1;r=r1;}
//    void set(double x1, double y1, double z1) {x=x1; y=y1;z=z1;}
//    void setType(double t) {type = t;}
//};

//struct V_SWCNodes
//{
//    vector <V3DLONG> nid_array; //the array of node id (node id is the 1st column value)
//    vector <V_NeuronSWC_coord> ncoord_array; //following the same order of nid_array
//    map <V3DLONG, V3DLONG> nid_ipos_lut; //a look up table to return the index of an nid, i.e. nid_ipos_lut[nid_array.at(i)] equals i;
//};

//struct V_NeuronSWC
//{
//    vector <V_NeuronSWC_unit> row;
//    bool b_linegraph;
//    string name;
//    string comment;
//    string file;
//    unsigned char color_uc[4];
//    bool b_jointed;
//    bool to_be_deleted;   // @ADDED by Alessandro on 2015-05-08. Needed to support late delete of multiple neuron segments.
//    bool to_be_broken;
//    bool on; //Added by Y. Wang on 2016-05-25. For the segment-wise display of a SWC.

//    bool check_data_consistency() {/* to verify if unique node id have unique coord, and if parent are in the nid, except -1*/ return true;}

//    V_NeuronSWC(string new_name="unset", bool is_linegraph=false)
//    {
//        name=new_name; b_linegraph=is_linegraph;  *(int*)color_uc=0; b_jointed=false;
//        to_be_deleted = false;
//        to_be_broken = false;
//        on = true;
//    }

//    V_BranchUnit branchingProfile;

//    void printInfo();

//    V3DLONG nrows() {return row.size();}

//    V_SWCNodes unique_nodes_info(); //determine the unique nodes

//    vector<V3DLONG> unique_nid(); //unique node id (id is the first column value in SWC)
//    V3DLONG n_unique_nid(); //number of unique node ids
//    vector<V_NeuronSWC_coord> unique_ncoord(); //unique node coordinates (coordinates are the 3rd to 5th column)
//    V3DLONG n_unique_ncoord(); //number of unique node coords

//    V3DLONG maxnoden() //091029 change maxnoden from >=-1 to >=0 for base_n in set_simple_path...
//    {
//                V3DLONG maxn=0;	for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).n > maxn) maxn = row.at(i).n;		return maxn;
//    }
//    V3DLONG getIndexofParent(V3DLONG j)
//    {
//        V3DLONG res=-1; V3DLONG parent = row.at(j).parent;
//                for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).n==parent)	{res=i; break;}
//        return res;
//    }
//    vector<V3DLONG> getIndexofParent_nodeid(V3DLONG nid) //return the array of of node "nid"'s parents' nid
//    {
//        vector<V3DLONG> res;
//                for (V3DLONG i=0;i<(V3DLONG)row.size();i++)
//        {
//            if (row.at(i).n==nid)
//            {
//                V3DLONG curparent = row.at(i).parent;
//                bool b_exist=false;
//                                for (V3DLONG j=0;j<(V3DLONG)res.size();j++)
//                    if (res.at(j)==curparent) {	b_exist=true; break;}
//                if (!b_exist)
//                    res.push_back(curparent);
//            }
//        }
//        return res;
//    }

//    void append(V_NeuronSWC_unit & new_node) {row.push_back(new_node);}
//    void clear() {row.clear();}
//    vector <V_NeuronSWC> decompose();
//    bool reverse();

//    bool isLineGraph() {return b_linegraph;} //just return the "claimed" property is a line graph
//    //check if a 3D location is contained in the swc
//    V3DLONG getFirstIndexof3DPos(double x,double y,double z) //return -1 is no included, othwise return the first detected index
//    {
//        V3DLONG res=-1;
//                for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).data[2]==x && row.at(i).data[3]==y && row.at(i).data[4]==z)	{res=i; break;}
//        return res;
//    }
//    V3DLONG getFirstIndexof3DPos(const V_NeuronSWC_unit & subject_node) {return getFirstIndexof3DPos(subject_node.data[2], subject_node.data[3], subject_node.data[4]);}
//    V3DLONG getFirstIndexof3DPos(const V_NeuronSWC_unit * subject_node) {return getFirstIndexof3DPos(subject_node->data[2], subject_node->data[3], subject_node->data[4]);}

//    vector<V3DLONG> getAllIndexof3DPos(double x,double y,double z, V3DLONG noninclude_ind) //return all indexes except the one indicated as noninclude_ind
//    {
//        vector<V3DLONG> res;
//                for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).data[2]==x && row.at(i).data[3]==y && row.at(i).data[4]==z)	{ if (i!=noninclude_ind) res.push_back(i); }
//        return res;
//    }
//    vector <V3DLONG> getAllIndexof3DPos(const V_NeuronSWC_unit & subject_node, V3DLONG noninclude_ind) {return getAllIndexof3DPos(subject_node.data[2], subject_node.data[3], subject_node.data[4], noninclude_ind);}
//    vector <V3DLONG> getAllIndexof3DPos(const V_NeuronSWC_unit * subject_node, V3DLONG noninclude_ind) {return getAllIndexof3DPos(subject_node->data[2], subject_node->data[3], subject_node->data[4], noninclude_ind);}
//};

//struct V_NeuronSWC_list
//{
//    vector <V_NeuronSWC> seg; //since each seg could be a complete neuron or multiple paths, thus I call it "seg", but not "path"
//    V3DLONG last_seg_num; //?? for what purpose? seems only used once in v3d_core.cpp. Questioned by Hanchuan, 20100210
//    string name;
//    string comment;
//    string file;
//    unsigned char color_uc[4];
//    bool b_traced;

//    V_NeuronSWC_list() {last_seg_num=-1; *(int*)color_uc=0; b_traced=true;}

//    V3DLONG nsegs() {return seg.size();}
//        V3DLONG nrows() {V3DLONG n=0; for (V3DLONG i=0;i<(V3DLONG)seg.size();i++) n+=seg.at(i).nrows(); return n;}
//    V3DLONG maxnoden()
//    {
//                V3DLONG maxn=0;	for (V3DLONG i=0;i<(V3DLONG)seg.size();i++) if (seg.at(i).maxnoden() > maxn) maxn = seg.at(i).maxnoden();	return maxn;
//    }
//    bool isJointed() {return nsegs()==1 && seg.at(0).b_jointed;}

//    void append(V_NeuronSWC & new_seg) {seg.push_back(new_seg); last_seg_num=seg.size();}
//        void append(vector <V_NeuronSWC> & new_segs) {for (int k=0; k<(int)new_segs.size(); k++) seg.push_back(new_segs.at(k)); last_seg_num=seg.size();}
//    void clear() {last_seg_num=seg.size(); seg.clear();}
//    void merge();
//    void decompose();
//    bool reverse();
//    bool split(V3DLONG seg_id, V3DLONG nodeinseg_id);
//    bool deleteSeg(V3DLONG seg_id);

//    // @ADDED by Alessandro on 2015-05-08. Needed to support late delete of multiple neuron segments.
//    void                                            // no value returned
//        deleteMultiSeg(                             // by default, deletes neuron segments having 'to_be_deleted' field set to 'true'
//            std::vector <V3DLONG> *seg_ids = 0);    // if provided, deletes the corresponding neuron segments.
//};
//NeuronTree v3dneuron_GD_tracing(unsigned char ****p4d, V3DLONG sz[4], LocationSimple & p0, vector<LocationSimple> & pp,
//                                CurveTracePara & trace_para, double trace_z_thickness);
 
//NeuronTree LinkOnePoint(unsigned char ****p4d, V3DLONG sz[4], LocationSimple & p0, LocationSimple & p1,
//                        CurveTracePara & trace_para, double trace_z_thickness);

//double getBlockAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
//                            V3DLONG x0, V3DLONG y0, V3DLONG z0,
//                            int xstep, int ystep, int zstep)
//{
//    if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
//        x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
//        return 0;

//    double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
//    V3DLONG xs=x0-xsteph, xe=x0+xsteph,
//        ys=y0-ysteph, ye=y0+ysteph,
//        zs=z0-zsteph, ze=z0+zsteph;

//    if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
//    if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
//    if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;

//    V3DLONG i,j,k,n;
//    double v=0;
//    n=0;
//    for (k=zs;k<=ze; k++)
//        for (j=ys;j<=ye; j++)
//            for (i=xs;i<=xe; i++)
//            {
//                v += double(img3d[k][j][i]);
//                n++;
//            }
//    return (n==0)?0:v/n;
//}

//double getBlockMaxValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
//                            V3DLONG x0, V3DLONG y0, V3DLONG z0,
//                            int xstep, int ystep, int zstep)
//{
//    if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
//        x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
//        return 0;

//    double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
//    V3DLONG xs=x0-xsteph, xe=x0+xsteph,
//        ys=y0-ysteph, ye=y0+ysteph,
//        zs=z0-zsteph, ze=z0+zsteph;

//    if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
//    if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
//    if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;

//    V3DLONG i,j,k,n;
//    double v=0;
//    double mv=0;
//    for (k=zs;k<=ze; k++)
//        for (j=ys;j<=ye; j++)
//            for (i=xs;i<=xe; i++)
//            {
//                v = double(img3d[k][j][i]);
//                if (v>mv) mv = v;
//            }
//    return mv;
//}

//double metric_func(double v, double max_v=255)
//{
//    double tmpv = 1-v/max_v;
//    return	exp((tmpv*tmpv)*10); //float min-step:1e-6, min:1e-37, max:1e38
//}

//double edge_weight_func(int it, double va, double vb, double max_v=255)
//{
//    double dist = edge_table[it].dist;
//    double m_ab = (metric_func(va, max_v) + metric_func(vb, max_v))*0.5;
//    //printf("[%5.3f] ", m_ab);
//                // (metric_func((va + vb, max_v)*0.5);

//    const double min_weight_step = 1e-5;   //090915 more precise //float min-step:1e-6
//    return (dist * m_ab) *min_weight_step;
//}





//char* bgl_shortest_path(Edge *edge_array, V3DLONG n_edges, Weight *weights, V3DLONG n_nodes, //input graph
//        Node start_nodeind, //input source
//        Node *plist) //output path
//{
//    char* s_error=0;

//    //typedef adjacency_list < vecS, vecS, directedS, no_property, property < edge_weight_t, Weight > > graph_t;
//    typedef adjacency_list < vecS, vecS, undirectedS, no_property, property < edge_weight_t, Weight > > graph_t;
//    typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
//    typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

//    printf("n_nodes=%ld start_node=%ld \n", n_nodes, start_nodeind);

//    //graph_t g(edge_array, edge_array + num_edges, weights, num_nodes);
//    graph_t g(n_nodes);

//    //printf("num_vertices(g)=%ld  num_edges(g)=%ld \n", num_vertices(g), num_edges(g));

//    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
//    for (V3DLONG j=0; j<n_edges; ++j)
//    {
//        edge_descriptor e; bool inserted;
//        boost::tuples::tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);		weightmap[e] = weights[j]; // Need to specify namespace for Visual Studio to avoid confusion with std library. MK 2017 June
//    }

//    printf("num_vertices(g)=%ld  num_edges(g)=%ld \n", num_vertices(g), num_edges(g));
//    if (n_nodes != num_vertices(g))
//    {
//        printf(s_error="ERROR to create graph: n_nodes != num_vertices(g) \n");
//        return s_error;
//    }
////	//for debugging purpose
////	{
////		property_map<graph_t, edge_weight_t>::type   weightmap = get(edge_weight, g);
////	    std::cout << " vertices(g) = ";
////		vertex_iterator vi, vend;
////	    for (tie(vi, vend) = vertices(g); vi != vend; ++vi)
////			;//std::cout << index[*vi] <<  " ";
////	    std::cout <<"total "<<i<< std::endl;
////	    std::cout << " edges(g) = ";
////	    edge_iterator ei, ei_end;
////	    for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
////			;//std::cout << "(" << index[source(*ei, g)] << "," << index[target(*ei, g)] << ":" << weightmap[*ei] <<") ";
////	    std::cout <<"total "<<i<< std::endl;
////	}

//    vertex_descriptor s = vertex(start_nodeind, g);
//    std::vector<vertex_descriptor> p(num_vertices(g));
//    std::vector<Weight> d(num_vertices(g));
//    dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));


////	std::cout<<"===================================================================\n";
////	std::cout << "distances and parents:" << std::endl;
//    graph_traits < graph_t >::vertex_iterator vi, vend;
//    V3DLONG i=0;
//    for (boost::tuples::tie(vi, vend) = vertices(g), i=0;
//         // Need to specify namespace for Visual Studio to avoid confusion with std library. MK 2017 June
//        i<n_nodes && vi != vend;
//        ++vi, i++)
//    {
//        //std::cout << "distance(" << (*vi) << ") = " << d[*vi] << ", ";
//        //std::cout << "parent(" << (*vi) << ") = " << (p[*vi]) << std::endl;
//        plist[*vi] = p[*vi];
//    }

//    return s_error;
//}

//void rearrange_and_remove_labeled_deletion_nodes_mmUnit(vector< vector<V_NeuronSWC_unit> >& mmUnit) //by PHC, 2011-01-15
//{
//    printf("....... rearranging index number \n");
//    std::map<double,V3DLONG> index_map;

//    V3DLONG i,j,k;

//    for (k=0; k<mmUnit.size(); k++)
//    {
//        printf("....... removing the [%ld] segment children that have been labeled to delete (i.e. nchild < 0).\n", k);
//        vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(k);
//        index_map.clear();

//        for (j=0; j<mUnit.size(); j++)
//        {
//            if (mUnit[j].nchild >=0)
//            {
//                double ndx   = mUnit[j].n;
//                V3DLONG new_ndx = index_map.size()+1;
//                index_map[ndx] = new_ndx;
//            }
//        }
//        for (j=0; j<mUnit.size(); j++)
//        {
//            if (mUnit[j].nchild >=0)
//            {
//                double ndx    = mUnit[j].n;
//                double parent = mUnit[j].parent;
//                mUnit[j].n = index_map[ndx];
//                if (parent>=0)	mUnit[j].parent = index_map[parent];
//            }
//        }

//        vector<V_NeuronSWC_unit> mUnit_new;
//        V3DLONG root_id=-1;
//        for (j=0; j<mUnit.size(); j++)
//        {
//            if (mUnit[j].nchild >= 0)
//            {
//                mUnit_new.push_back(mUnit[j]);
//            }

//            if (mUnit[j].parent<0)
//            {
//                if (root_id!=-1)
//                    printf("== [segment %ld] ================== detect a non-unique root!\n", k);
//                root_id = V3DLONG(mUnit[j].n);
//                printf("== [segment %ld] ================== nchild of root [%ld, id=%ld] = %ld\n", k, j, V3DLONG(mUnit[j].n), V3DLONG(mUnit[j].nchild));
//            }

//        }

//        mmUnit[k] = mUnit_new;
//    }
//}

//char* find_shortest_path_graphimg(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, //image
//        float zthickness, // z-thickness for weighted edge
//        //const V3DLONG box[6],  //bounding box
//        V3DLONG bx0, V3DLONG by0, V3DLONG bz0, V3DLONG bx1, V3DLONG by1, V3DLONG bz1, //bounding box (ROI)
//                            float x0, float y0, float z0,       // start node
//                            int n_end_nodes,                    // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
//                            float x1[], float y1[], float z1[],    // all end nodes
//                            vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
//                            const ParaShortestPath & para)
//{
//    //printf("start of find_shortest_path_graphimg \n");
//    bool b_error = false;
//    char* s_error = 0;
//    const float dd = 0.5;

//    printf("sizeof(Weight) = %d, sizeof(Node) = %d \n", sizeof(Weight), sizeof(Node));
//    printf("bounding (%ld %ld %ld)--(%ld %ld %ld) in image (%ld x %ld x %ld)\n", bx0,by0,bz0, bx1,by1,bz1, dim0,dim1,dim2);
//    if (!img3d || dim0<=0 || dim1<=0 || dim2<=0)
//    {
//        printf(s_error="Error happens: no image data!\n");
//        return s_error;
//    }
//    if ((bx0<0-dd || bx0>=dim0-dd || by0<0-dd || by0>=dim1-dd || bz0<0-dd || bz0>=dim2-dd)
//       || (bx1<0-dd || bx1>=dim0-dd || by1<0-dd || by1>=dim1-dd || bz1<0-dd || bz1>=dim2-dd))
//    {
//        printf(s_error="Error happens: bounding box out of image bound!\n");
//        printf("inside z1=%ld\n", bz1);;
//        return s_error;
//    }

//    //now set parameters
//    int min_step       = para.node_step; //should be >=1
//    int smooth_winsize = para.smooth_winsize;
//    int edge_select    = para.edge_select;  //0 -- only use length 1 edge(optimal for small step), 1 -- plus diagonal edge
//    double imgTH = para.imgTH; //anything <= imgTH will NOT be traced!

//    int dowsample_method = para.downsample_method; //0 for average, 1 for max

//    if (min_step<1)       min_step =1;
//    if (smooth_winsize<1) smooth_winsize =1;

//    //bounding box volume
//    V3DLONG xmin = bx0, xmax = bx1,
//         ymin = by0, ymax = by1,
//         zmin = bz0, zmax = bz1;

//    V3DLONG nx=((xmax-xmin)/min_step)+1, 	xstep=min_step,
//         ny=((ymax-ymin)/min_step)+1, 	ystep=min_step,
//         nz=((zmax-zmin)/min_step)+1, 	zstep=min_step;

//    V3DLONG num_edge_table = (edge_select==0)? 3:13; // exclude/include diagonal-edge

//    printf("valid bounding (%ld %ld %ld)--(%ld %ld %ld) ......  ", xmin,ymin,zmin, xmax,ymax,zmax);
//    printf("%ld x %ld x %ld nodes, step = %d, connect = %d \n", nx, ny, nz, min_step, num_edge_table*2);

//    V3DLONG num_nodes = nx*ny*nz;
//    V3DLONG i,j,k,n,m;

//    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    #define NODE_FROM_XYZ(x,y,z) 	(V3DLONG((z+.5)-zmin)/zstep*ny*nx + V3DLONG((y+.5)-ymin)/ystep*nx + V3DLONG((x+.5)-xmin)/xstep)
//    #define NODE_TO_XYZ(j, x,y,z) \
//    { \
//        z = (j)/(nx*ny); 		y = ((j)-V3DLONG(z)*nx*ny)/nx; 	x = ((j)-V3DLONG(z)*nx*ny-V3DLONG(y)*nx); \
//        x = xmin+(x)*xstep; 	y = ymin+(y)*ystep; 			z = zmin+(z)*zstep; \
//    }
//    #define NODE_FROM_IJK(i,j,k) 	((k)*ny*nx+(j)*nx+(i))
//    #define X_I(i)				 	(xmin+(i)*xstep)
//    #define Y_I(i)				 	(ymin+(i)*ystep)
//    #define Z_I(i)				 	(zmin+(i)*zstep)
//    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//    //out of bound handler
//    #define NODE_XYZ_OUT_OF_BOUND(x0,y0,z0)		(x0<xmin-dd || x0>xmax+dd || y0<ymin-dd || y0>ymax+dd || z0<zmin-dd || z0>zmax+dd)
//    #define NODE_INDEX_OUT_OF_BOUND(ind) 		(ind<0 || ind>=num_nodes)

//    V3DLONG start_nodeind, *end_nodeind = 0;
//    if (n_end_nodes>0) //101210 PHC
//        end_nodeind = new V3DLONG [n_end_nodes]; //100520, PHC
//    else
//        printf("**************** n_end_nodes is 0, and thus do not need to allocate memory. *********************\n");


//    if (NODE_XYZ_OUT_OF_BOUND(x0,y0,z0))
//    {
//        printf(s_error="Error happens: start_node out of bound! \n");
//        return s_error;
//    }
//    start_nodeind = NODE_FROM_XYZ(x0,y0,z0);
//    if (NODE_INDEX_OUT_OF_BOUND(start_nodeind))
//    {
//        printf(s_error="Error happens: start_node index out of range! \n");
//        if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
//        return s_error;
//    }

//    V3DLONG n_end_outbound = 0;
//    for (i=0; i<n_end_nodes; i++)
//    {
//        if (NODE_XYZ_OUT_OF_BOUND(x1[i],y1[i],z1[i]))
//        {
//            end_nodeind[i] = -1;
//            printf("Warning: end_node[%d] out of bound! \n", i);
//            n_end_outbound ++;
//            continue; //ignore this end_node out of ROI
//        }
//        end_nodeind[i]   = NODE_FROM_XYZ(x1[i],y1[i],z1[i]);
//        if (NODE_INDEX_OUT_OF_BOUND(end_nodeind[i]))
//        {
//            end_nodeind[i] = -1;
//            printf("Warning: end_node[%d] index out of range! \n", i);
//            n_end_outbound ++;
//            continue; //ignore this end_node out of ROI
//        }
//    }

//    if (n_end_nodes>0 //for 1-to-N, not 1-to-image
//        && n_end_outbound>=n_end_nodes)
//    {
//        printf(s_error="Error happens: all end_nodes out of bound! At least one end_node must be in bound.\n");
//        if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
//        return s_error;
//    }


//#define _creating_graph_

//    ///////////////////////////////////////////////////////////////////////////////////////////////////////
//    //switch back to new[] from std::vector for *** glibc detected *** ??? on Linux
//    std::vector<Node> 	plist(num_nodes);		for (i=0;i<num_nodes;i++) plist[i]=i;
//    std::vector<Edge> 	edge_array;				edge_array.clear();
//    std::vector<Weight>	weights;				weights.clear();
//    ///////////////////////////////////////////////////////////////////////////////////////////////////////

//#define _setting_weight_of_edges_
//    printf("setting weight of edges ......  ");

//    // z-thickness weighted edge
//    for (V3DLONG it=0; it<num_edge_table; it++)
//    {
//        double di = (edge_table[it].i0 - edge_table[it].i1);
//        double dj = (edge_table[it].j0 - edge_table[it].j1);
//        double dk = (edge_table[it].k0 - edge_table[it].k1) * zthickness;
//        edge_table[it].dist = sqrt(di*di + dj*dj + dk*dk);
//    }

//    double va, vb;
//    double maxw=0, minw=1e+6; //for debug info
//    n=0; m=0;
//    for (k=0;k<nz;k++)
//    {
//        for (j=0;j<ny;j++)
//        {
//            for (i=0;i<nx;i++)
//            {
//                for (int it=0; it<num_edge_table; it++)
//                {
//                    // take an edge
//                    V3DLONG ii = i+ edge_table[it].i0;
//                    V3DLONG jj = j+ edge_table[it].j0;
//                    V3DLONG kk = k+ edge_table[it].k0;
//                    V3DLONG ii1 = i+ edge_table[it].i1;
//                    V3DLONG jj1 = j+ edge_table[it].j1;
//                    V3DLONG kk1 = k+ edge_table[it].k1;

//                    if (ii>=nx || jj>=ny || kk>=nz || ii1>=nx || jj1>=ny || kk1>=nz) continue;//for boundary condition

//                    V3DLONG node_a = NODE_FROM_IJK(ii,jj,kk);
//                    V3DLONG node_b = NODE_FROM_IJK(ii1,jj1,kk1);

//                    m++;

//                    //=========================================================================================
//                    // edge link
//                    if(dowsample_method == 0)
//                    {
//                        va = getBlockAveValue(img3d, dim0, dim1, dim2, X_I(ii),Y_I(jj),Z_I(kk),
//                                xstep, ystep, (zstep/zthickness)); //zthickness
//                        vb = getBlockAveValue(img3d, dim0, dim1, dim2, X_I(ii1),Y_I(jj1),Z_I(kk1),
//                                xstep, ystep, (zstep/zthickness)); //zthickness
//                    }else if(dowsample_method == 1)
//                    {
//                        va = getBlockMaxValue(img3d, dim0, dim1, dim2, X_I(ii),Y_I(jj),Z_I(kk),
//                                xstep, ystep, (zstep/zthickness)); //zthickness
//                        vb = getBlockMaxValue(img3d, dim0, dim1, dim2, X_I(ii1),Y_I(jj1),Z_I(kk1),
//                                xstep, ystep, (zstep/zthickness)); //zthickness
//                    }

//                    if (va<=imgTH || vb<=imgTH)
//                        continue; //skip background node link

//                    Edge e = Edge(node_a, node_b);
//                    edge_array.push_back( e );

//                    Weight w =	edge_weight_func(it, va,vb, 255);

//                    //now try to use favorite direction if literally specified. by PHC 20170606
//                    if (para.b_use_favorite_direction)
//                    {
//                        double x_mid = (double(X_I(ii)) + X_I(ii1))/2.0;
//                        double y_mid = (double(Y_I(jj)) + Y_I(jj1))/2.0;
//                        double z_mid = (double(Z_I(kk)) + Z_I(kk1))/2.0;

//                        double d_startpt_2_mid[3]; //the direction vector from the starting pt to the mid-pt of the two current ends of an edge
//                        d_startpt_2_mid[0] = x_mid - x0;
//                        d_startpt_2_mid[1] = y_mid - y0;
//                        d_startpt_2_mid[2] = z_mid - z0;

//                        double cangle = cosangle_two_vectors(para.favorite_direction, d_startpt_2_mid);

//                        if (cangle != -2) //-2 is a special return value indicating the status of cosangle_two_vectors()
//                        {
//                            double w_direction = 1 - (cangle+1.0)/2.0; //add 1.0 to force 0 degree angle to be max, 180 degree angle to be minimal; divide 2.0 to make sure w will not be too big
//                            if (w_direction>0.35) w += 100;
//                            //printf("wd=%5.3f x_mid=%5.3f y_mid=%5.3f z_mid=%5.3f x0=%5.3f y0=%5.3f z0=%5.3f vpq=%5.3f vpp=%5.3f vqq=%5.3f \n", w_direction, x_mid, y_mid, z_mid, x0, y0, z0, vpq, vpp, vqq);
//                        }
//                        else
//                        {
//                            //do nothing here.
//                        }
//                        //also in the furture we may not need to divide sqrt(vqq) because it is constant, and this should also avoid the extra-exception when vqq=0
//                    }

//                    weights.push_back( w );
//                    //=========================================================================================

//                    n++; // that is the correct position of n++

//                    if (w>maxw) maxw=w;	if (w<minw) minw=w;
//                }
//            }
//        }
//    }
//    printf(" minw=%g maxw=%g ", minw,maxw);
//    printf(" graph defined! \n");

//    if (n != edge_array.size())
//    {
//        printf(s_error="The number of edges is not consistent \n");
//        if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
//        return s_error;
//    }
//    V3DLONG num_edges = n; // back to undirectEdge for less memory consumption

//    //printf("image average =%g, std =%g, max =%g.  select %ld out of %ld links \n", imgAve, imgStd, imgMax, n, m);
//    printf("select %ld out of %ld links \n", n, m);
//    printf("total %ld nodes, total %ld edges \n", num_nodes, num_edges);
//    printf("start from #%ld to ", start_nodeind);
//    for(V3DLONG i=0; i<n_end_nodes; i++) printf("#%ld ", end_nodeind[i]); printf("\n");
//    printf("---------------------------------------------------------------\n");



//#define _do_shortest_path_algorithm_
//    //========================================================================================================

//    int code_select = 0; // BGL has the best speed and correctness
//    switch(code_select)
//    {
//        case 0:
//            printf("bgl_shortest_path() \n");
//            s_error = bgl_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes, start_nodeind, &plist[0]);
//            break;
////        case 1:
////            printf("phc_shortest_path() \n");
////            s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
////            break;
////        case 2:
////            printf("mst_shortest_path() \n");
////            s_error = mst_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
////            break;
//    }
//    if (s_error)
//    {
//        if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
//        return s_error;
//    }
//    //=========================================================================================================
//    //for (i=0;i<num_nodes;i++)	std::cout<<"p("<<i<<")="<<plist[i]<<";   ";  std::cout<<std::endl;


//    // output node coordinates of the shortest path
//    mmUnit.clear();
//    V3DLONG nexist = 0;

//    V_NeuronSWC_unit cc;
//    vector<V_NeuronSWC_unit> mUnit;

//    if (n_end_nodes==0) // trace from start-->each possible node
//    {
//#define _output_shortest_path_TREE_
//        printf("trace from start-->each possible node \n");
//        mUnit.clear();

//        std::map<double,V3DLONG> index_map;	index_map.clear();
//        // set nchild=0
//        for (j=0; j<num_nodes; j++)
//        {
//            if (j==start_nodeind)
//            {
//                cc.x = x0;
//                cc.y = y0;
//                cc.z = z0;
//                cc.n = 1+j;
//                cc.parent = -1;
//                cc.nchild = 0; // although root isn't leaf, but the different should be told by checking their parent nodes instead of number of children. changed from 1 to 0, by PHC, 20110908. I think root can also be a leaf,
//                mUnit.push_back(cc);
//                index_map[cc.n] = mUnit.size()-1; //fix the missing line bug by PHC, 2010-12-30
//                printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
//            }
//            else if ( (k=plist[j]) != j ) // has parent
//                if (k>=0 && k<num_nodes)  // is valid
//            {
//                NODE_TO_XYZ(j, cc.x, cc.y, cc.z);
//                cc.n = 1+j;
//                cc.parent = 1+k; //k=plist[j]
//                cc.nchild = 0;
//                mUnit.push_back(cc);
//                //printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);

//                index_map[cc.n] = mUnit.size()-1;
//            }
//        }

//        printf("counting parent->nchild \n");
//        // count parent->nchild
//        for (j=0; j<mUnit.size(); j++)
//        {
//            double parent = mUnit[j].parent;
//            V3DLONG i = index_map[parent]; // this is very fast

//            mUnit[i].nchild++;
//        }

//        double myTH = imgTH; if (myTH<para.visible_thresh) myTH=para.visible_thresh;
//        if (1)
//        {
//            printf("labeling to remove bark leaf child \n");
//            printf("before dark-pruning there are %ld nodes in total, from %ld nodes in the initial graph. \n", mUnit.size(), num_nodes);
//            //remove leaf node (nchild==0)

//            for (k=0; ; k++)
//            {
//                V3DLONG nprune=0;
//                for (j=0; j<mUnit.size(); j++)
//                {
//                    if (mUnit[j].nchild ==0)
//                    {
//                        double parent = mUnit[j].parent;
//                        V3DLONG i = index_map[parent];

//                        int min_cut_level = 10/min_step;	//maybe can change to 1 ? 2011-01-13. by PHC
//                        if (min_cut_level<1) min_cut_level=1;
//                        //double va = getBlockAveValue(img3d, dim0, dim1, dim2, mUnit.at(i).x,mUnit.at(i).y,mUnit.at(i).z,
//                        //		min_cut_level, min_cut_level, min_cut_level);
//                        double va = getBlockAveValue(img3d, dim0, dim1, dim2, mUnit.at(j).x,mUnit.at(j).y,mUnit.at(j).z,
//                                                     min_cut_level, min_cut_level, min_cut_level);

//                        //if (k<min_cut_level || va <= imgAve+imgStd*min_cut_level)

//                        if (va <= myTH  //dark pruning
//                            //|| (k<5
//                                           //&& mUnit[i].nchild >=2
//                            //			   )
//                            ) //this gives a symmetric pruning, seems better than (k<5) criterion which leads to an asymmetric prunning
//                        {
//                            mUnit[i].nchild--;
//                            //label to remove
//                            mUnit[j].nchild = -1;	//mUnit[j].parent = mUnit[j].n; //seems no need for this
//                            nprune++;
//                        }

//    // no need to do at this moment. by PHC, 101226
//    //					else //091108. update this node's coordinate using its CoM
//    //					{
//    //						if (1) //091120. temporary block the code for the demo
//    //						{
//    //							float curx= mUnit.at(i).x, cury = mUnit.at(i).y, curz = mUnit.at(i).z;
//    //							fitPosition(img3d, dim0, dim1, dim2, imgTH, 3*min_cut_level, curx, cury, curz);
//    //							mUnit.at(i).x = curx, mUnit.at(i).y = cury, mUnit.at(i).z = curz;
//    //						}
//    //					}
//                    }
//                }

//                if (0)
//                for (j=mUnit.size()-1; j>=0; j--)
//                {
//                    if (mUnit[j].nchild ==0)
//                    {
//                        double parent = mUnit[j].parent;
//                        V3DLONG i = index_map[parent];

//                        int min_cut_level = 10/min_step;	if (min_cut_level<1) min_cut_level=1;
//                        double va = getBlockAveValue(img3d, dim0, dim1, dim2, mUnit.at(i).x,mUnit.at(i).y,mUnit.at(i).z,
//                                                     min_cut_level, min_cut_level, min_cut_level);

//                        if (va <= myTH || (k<5
//                                           //&& mUnit[i].nchild >=2
//                                           )) //this gives a symmetric pruning, seems better than (k<5) criterion which leads to an asymmetric prunning
//                        {
//                            mUnit[i].nchild--;
//                            mUnit[j].nchild = -1; 							//label to remove
//                        }
//                    }
//                }

//                printf("dark prune loop %ld. remove %ld nodes.\n", k, nprune);
//                if (nprune==0)
//                    break;
//            }
//        }

//        //remove those nchild<0 and rearraneg indexes

//        mmUnit.push_back(mUnit);
//        rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);

//        printf("done with the SP step. \n");
//    }

//    else
//    for (V3DLONG npath=0; npath<n_end_nodes; npath++) // n path of back tracing end-->start
//    {
//#define _output_shortest_path_N_
//        printf("the #%d path of back tracing end-->start \n", npath+1);
//        mUnit.clear();

//        j = end_nodeind[npath]; //search from the last one
//        cc.x = x1[npath];
//        cc.y = y1[npath];
//        cc.z = z1[npath];
//        cc.n = nexist +1+mUnit.size();
//        cc.parent = cc.n +1;
//        printf("[end: x y z] %ld: %g %g %g ", j, cc.x, cc.y, cc.z);
//        if (j<0 || j>=num_nodes) // for the end_node out of ROI
//        {
//            printf(" end_node is out of ROI, ignored.\n");
//            continue;
//        }
//        printf("\n");

//        mUnit.push_back(cc);

//        for (k=0;k<n;k++) //at most n edge links
//        {
//            V3DLONG jj = j;	j = plist[j];

//            if (j==jj)
//            {
//                mUnit.clear();
//                printf(s_error="Error happens: this path is broken because a node has a self-link!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
//                break;
//            }
//            else if (j>=num_nodes)
//            {
//                mUnit.clear();
//                printf(s_error="Error happens: this node's parent has an index out of range!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
//                break;
//            }
//            else if (j<0) // should not be reached, because stop back trace at his child node
//            {
//                mUnit.clear();
//                printf(s_error="find the negative node, which should indicate the root has been over-reached."); printf(" [j->p(j)] %ld->%ld \n", jj, j);
//                break;
//            }

//            if (j!=start_nodeind)
//            {
//                NODE_TO_XYZ(j, cc.x, cc.y, cc.z);
//                cc.n = nexist +1+mUnit.size();
//                cc.parent = cc.n +1;
//                mUnit.push_back(cc);
//                //printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
//            }
//            else //j==start_nodeind
//            {
//                cc.x = x0;
//                cc.y = y0;
//                cc.z = z0;
//                cc.n = nexist +1+mUnit.size();
//                cc.parent = -1;
//                mUnit.push_back(cc);
//                printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);

//                break; //STOP back tracing
//            }
//        }
//        nexist += mUnit.size();

//        if (mUnit.size()>=2)	mmUnit.push_back(mUnit);
//    }

////	//also can do smoothing outside in proj_trace_smooth_dwonsample_last_traced_neuron
////	printf("smooth_curve + downsample_curve \n");
////	smooth_curve(mCoord, smooth_winsize);
////	mCoord = downsample_curve(mCoord, outsample_step);

//    if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC

//    if (mmUnit.size())	return 0;
//    return s_error;
//}


//V_NeuronSWC_list trace_one_pt_to_N_points_shortestdist(unsigned char ****p4d, V3DLONG sz[4],
//                                                             LocationSimple & p0, vector<LocationSimple> & pp,
//                                                             CurveTracePara & trace_para, double trace_z_thickness,
//                                                             vector< vector<V_NeuronSWC_unit> > & mmUnit)
//{
//    V_NeuronSWC_list tracedNeuron;
////    CHECK_DATA_GD_TRACING(tracedNeuron);

//    ParaShortestPath sp_para;
//    sp_para.edge_select       = trace_para.sp_graph_connect;
//    sp_para.background_select = trace_para.sp_graph_background;
//    sp_para.node_step      = trace_para.sp_graph_resolution_step;
//    sp_para.outsample_step = trace_para.sp_downsample_step;
//    sp_para.smooth_winsize = trace_para.sp_smoothing_win_sz;
//    sp_para.imgTH = trace_para.imgTH;
//    sp_para.visible_thresh = trace_para.visible_thresh;
//    sp_para.downsample_method = trace_para.sp_downsample_method;

//    //170606
//    sp_para.b_use_favorite_direction = trace_para.b_use_favorite_direction;
//    for (int myii=0; myii<3; myii++)
//        sp_para.favorite_direction[myii] = trace_para.favorite_direction[myii];

//    //

//    mmUnit.clear();
//    int chano = trace_para.channo;
//    if (chano>=sz[3]) chano=sz[3]-1; if (chano<0) chano=0;
//    int n_end_nodes = pp.size();
//    vector<float> px, py, pz;
//    px.clear(), py.clear(), pz.clear();
//    for (int i=0;i<pp.size();i++) {px.push_back(pp[i].x), py.push_back(pp[i].y), pz.push_back(pp[i].z);}

//    BoundingBox trace_bounding_box;
//    printf("find_shortest_path_graphimg >>> ");
//    if (trace_bounding_box == NULL_BoundingBox)
//    {
//        trace_bounding_box.x0 = trace_bounding_box.y0 = trace_bounding_box.z0 = 0;
//        trace_bounding_box.x1 = sz[0]-1;
//        trace_bounding_box.y1 = sz[1]-1;
//        trace_bounding_box.z1 = sz[2]-1;
//        printf("set z1=%ld\n", V3DLONG(trace_bounding_box.z1));
//    }
//    printf("z1=%ld\n", V3DLONG(trace_bounding_box.z1));

//    float *pxp = 0, *pyp=0, *pzp=0;
//    if (n_end_nodes>0)
//    {
//        pxp = &(px[0]);
//        pyp = &(py[0]);
//        pzp = &(pz[0]);
//    }

//    char* s_error = 0;
//    if (sz[3]==1)
//    {
//        if(1)
//        {
//            s_error = find_shortest_path_graphimg(p4d[chano], sz[0], sz[1], sz[2],
//                                              trace_z_thickness,
//                                              trace_bounding_box.x0, trace_bounding_box.y0, trace_bounding_box.z0,
//                                              trace_bounding_box.x1, trace_bounding_box.y1, trace_bounding_box.z1,
//                                              p0.x, p0.y, p0.z,
//                                              n_end_nodes,
//                                              pxp, pyp, pzp, //fix this bug 100624
//                                              mmUnit,
//                                              sp_para);

//        }

//    }

//    printf("find_shortest_path_graphimg <<< ");
//    if (s_error)
//    {
//        v3d_msg(s_error,0);
//      //  throw (const char*)s_error;
//        return tracedNeuron;
//    }

////    int nSegsTrace = mergeback_mmunits_to_neuron_path(n_end_nodes, mmUnit, tracedNeuron);

//    //return traced res
//    return tracedNeuron;
//}

//V_NeuronSWC merge_V_NeuronSWC_list(V_NeuronSWC_list & in_swc_list)
//{
//    V_NeuronSWC out_swc;  out_swc.clear();
//    V_NeuronSWC_unit v;
//    V3DLONG n=0, i,j,k;
//    V3DLONG nsegs = in_swc_list.seg.size();
//    for (k=0;k<nsegs;k++)
//    {
//        if(in_swc_list.seg.at(k).to_be_deleted)
//            continue;

//        if(!in_swc_list.seg.at(k).on)
//            continue;

//        vector <V_NeuronSWC_unit> &row = (in_swc_list.seg.at(k).row);
//        if (row.size()<=0) continue;

//        //first find the min index number, then all index will be automatically adjusted
//        V3DLONG min_ind = row[0].n;
//        for (j=1;j<row.size();j++)
//        {
//            if (row[j].n < min_ind)  min_ind = row[j].n;
//            if (min_ind<0) v3d_msg("Found illeagal neuron node index which is less than 0 in merge_V_NeuronSWC_list()!");
//        }
//        //qDebug()<<min_ind;

//        // segment id & color type
//        int seg_id = k;

//        //now merge
//        V3DLONG n0=n;
//        for (j=0;j<row.size();j++)
//        {
//            v.seg_id = seg_id;
//            v.nodeinseg_id = j;
//            v.level = row[j].level;
//            v.n = (n0+1) + row[j].n-min_ind;
//            for (i=1;i<=5;i++)	v.data[i] = row[j].data[i];
//            v.parent = (row[j].parent<0)? -1 : ((n0+1) + row[j].parent-min_ind); //change row[j].parent<=0 to row[j].parent<0, PHC 091123.

//            //qDebug()<<row[j].n<<"->"<<v.n<<" "<<row[j].parent<<"->"<<v.parent<<" "<<n;

//            out_swc.row.push_back(v);
//            n++;
//        }
//    }
//    out_swc.color_uc[0] = in_swc_list.color_uc[0];
//    out_swc.color_uc[1] = in_swc_list.color_uc[1];
//    out_swc.color_uc[2] = in_swc_list.color_uc[2];
//    out_swc.color_uc[3] = in_swc_list.color_uc[3];

//    out_swc.name = "merged";
////	for (i=0;i<out_swc.nrows();i++)
////		qDebug()<<out_swc.row.at(i).data[2]<<" "<<out_swc.row.at(i).data[3]<<" "<<out_swc.row.at(i).data[4]<<" "<<out_swc.row.at(i).data[6];

//    return out_swc;
//}

//NeuronTree convertNeuronTreeFormat(V_NeuronSWC_list & tracedNeuron)
//{
//    NeuronTree SS;

//    //first conversion

//    V_NeuronSWC seg = merge_V_NeuronSWC_list(tracedNeuron);
//    seg.name = tracedNeuron.name;
//    seg.file = tracedNeuron.file;

//    //second conversion

//    QList <NeuronSWC> listNeuron;
//    QHash <int, int>  hashNeuron;
//    listNeuron.clear();
//    hashNeuron.clear();

//    {
//        int count = 0;
//        for (int k=0;k<seg.row.size();k++)
//        {
//            count++;
//            NeuronSWC S;

//            S.n 	= seg.row.at(k).data[0];
//            if (S.type<=0) S.type 	= 2; //seg.row.at(k).data[1];
//            S.x 	= seg.row.at(k).data[2];
//            S.y 	= seg.row.at(k).data[3];
//            S.z 	= seg.row.at(k).data[4];
//            S.r 	= seg.row.at(k).data[5];
//            S.pn 	= seg.row.at(k).data[6];

//            //for hit & editing
//            S.seg_id       = seg.row.at(k).seg_id;
//            S.nodeinseg_id = seg.row.at(k).nodeinseg_id;

//            //qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);

//            //if (! listNeuron.contains(S)) // 081024
//            {
//                listNeuron.append(S);
//                hashNeuron.insert(S.n, listNeuron.size()-1);
//            }
//        }
//        printf("---------------------read %d lines, %d remained lines", count, listNeuron.size());

//        SS.n = -1;
//        SS.color = XYZW(seg.color_uc[0],seg.color_uc[1],seg.color_uc[2],seg.color_uc[3]);
//        SS.on = true;
//        SS.listNeuron = listNeuron;
//        SS.hashNeuron = hashNeuron;

//        SS.name = seg.name.c_str();
//        SS.file = seg.file.c_str();
//    }

//    return SS;
//}


//bool dt_v3dneuron(unsigned char *p1d, V3DLONG sz[3], float imgTH, bool b_to_background, bool b_3d_dt, float *p1dt) //imgTH specify the foreground
//{
//    v3d_msg("dt_v3dneuron(). \n", 0);
//    if (!p1d || !sz || sz[0]<1 || sz[1]<1 || sz[2]<1 || !p1dt)
//    {
//        v3d_msg("Invalid parameters.\n",0);
//        return false;
//    }

//    V3DLONG len = sz[0]*sz[1]*sz[2];
//    V3DLONG *label = 0;
//    try {
//        label = new V3DLONG [len];
//    }
//    catch (...)
//    {
//        v3d_msg("Fail to allocate memory in dt_v3dneuron().\n", 0);
//        return false;
//    }

//    float *tmpdatap = p1dt, *endp=p1dt+len;
//    unsigned char *indatap = p1d;
//    for (tmpdatap=p1dt; tmpdatap<endp; ++tmpdatap)
//    {
//        if (b_to_background) //distance to background
//            *tmpdatap = (*indatap++ > imgTH) ? INF : 0;
//        else //distance to foreground
//            *tmpdatap = (*indatap++ > imgTH) ? 0 : INF;
//    }

//    if (b_3d_dt)
//    {
//        printf("3d =====================================\n");
//        dt3d(p1dt, label, sz);
//    }
//    else //2d dt
//    {
//        printf("2d=============================\n");
//        for (V3DLONG k=0; k<sz[2]; k++)
//            dt2d(p1dt + k*sz[0]*sz[1], label + k*sz[0]*sz[1], sz);
//    }

//    indatap = p1d;
//    for (tmpdatap=p1dt; tmpdatap<endp; ++tmpdatap)
//    {
//        *tmpdatap = sqrt(double(*indatap++));
//    }

//    if (label) {delete []label; label=0;}
//    return true;
//}

//double cosAngleBetween(VPoint *a, VPoint *b)
//{
//    if (!(a->isComparable(b)))
//        return 0;

//    double sa = a->abs();
//    double sb = b->abs();
//    if (sa==0 || sb==0) return 0;

//    double s=0;
//    for (V3DLONG i=0;i<a->n;i++) s+=	a->v[i] * b->v[i];
//    return (s/(sa*sb));
//}


//double fitRadiusPercent(unsigned char ****img4d, V3DLONG sz[4], double imgTH, double bound_r,
//                        float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly)
//{
//    if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitRadiusPercent() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset

//    V3DLONG dim0 = sz[0], dim1 = sz[1], dim2 = sz[2];
//    double max_r = dim0/2;
//    if (max_r > dim1/2) max_r = dim1/2;
//    if (!b_est_in_xyplaneonly)
//    {
//        if (max_r > (dim2*zthickness)/2) max_r = (dim2*zthickness)/2;
//    }
//    //max_r = bound_r; //unused as of now (comment added by PHC, 2010-Dec-21)

//    VPoint v0(sz[3],0), v1(sz[3],0);
//    V3DLONG c;
//    for (c=0;c<sz[3];c++)
//        v0.v[c] = img4d[c][(V3DLONG)(z+0.5)][(V3DLONG)(y+0.5)][(V3DLONG)(x+0.5)];

//    double total_num, background_num;
//    double ir;
//    for (ir=1; ir<=max_r; ir++)
//    {
//        total_num = background_num = 0;

//        double dz, dy, dx;
//        double zlower = -ir/zthickness, zupper = +ir/zthickness;
//        if (b_est_in_xyplaneonly)
//            zlower = zupper = 0;
//        for (dz= zlower; dz <= zupper; ++dz)
//            for (dy= -ir; dy <= +ir; ++dy)
//                for (dx= -ir; dx <= +ir; ++dx)
//                {
//                    total_num++;

//                    double r = sqrt(dx*dx + dy*dy + dz*dz);
//                    if (r>ir-1 && r<=ir)
//                    {
//                        V3DLONG i = x+dx;	if (i<0 || i>=dim0) goto end;
//                        V3DLONG j = y+dy;	if (j<0 || j>=dim1) goto end;
//                        V3DLONG k = z+dz;	if (k<0 || k>=dim2) goto end;

//                        for (c=0;c<sz[3];c++)
//                            v1.v[c] = img4d[c][k][j][i];

//                        if (v1.abs() <= imgTH ||
//                            cosAngleBetween(&v0, &v1) < 0.866
//                            )
//                        {
//                            background_num++;

//                            if ((background_num/total_num) > 0.001)	goto end; //change 0.01 to 0.001 on 100104
//                        }
//                    }
//                }
//    }
//end:
//    return ir;
//}

//bool fit_radius_and_position(unsigned char ****img4d, V3DLONG sz[4],
//                             vector <V_NeuronSWC_unit> & mCoord, bool b_move_position, float zthickness, bool b_est_in_xyplaneonly, double vis_threshold)
////template <class T>
////bool fit_radius_and_position(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
////							vector <T> & mCoord, bool b_move_position)
//{
//    if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fit_radius_and_position() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset

//    if (mCoord.size()<2)
//        return false;

//    double AR = 0;
//    for (int i=0; i<mCoord.size()-1; i++)
//    {
//        float x = mCoord[i].x;
//        float y = mCoord[i].y;
//        float z = mCoord[i].z;
//        float x1 = mCoord[i+1].x;
//        float y1 = mCoord[i+1].y;
//        float z1 = mCoord[i+1].z;
//        AR += sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1) + (z-z1)*(z-z1));
//    }
//    AR /= mCoord.size()-1; // average distance between nodes

//    V3DLONG i;
//    V3DLONG *channelsToUse= new V3DLONG[sz[3]]; for (i=0;i<sz[3];i++) channelsToUse[i]=i;

//    int method=0;
//    double imgAve = getImage4DAveValue(img4d, sz, channelsToUse, sz[3], method);
//    double imgStd = getImage4DStdValue(img4d, sz, channelsToUse, sz[3]);
//    double imgTH = imgAve + imgStd; //change to VISIBLE_THRESHOLD 2011-01-21 but the result is not good
//    if (imgTH<vis_threshold) imgTH = vis_threshold; //added by PHC 20121016. consistent with APP2 improvement

//    for (i=0; i<mCoord.size(); i++)
//    {
//        float x = mCoord[i].x;
//        float y = mCoord[i].y;
//        float z = mCoord[i].z;

//        double r=0;
//        if (i==0 || i==mCoord.size()-1) // don't move start && end point
//        {
//            r = fitRadius(img4d, sz, imgTH, AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
//        }
//        else
//        {
//            if (! b_move_position)
//            {
//                r = fitRadius(img4d, sz, imgTH, AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
//            }
////			else
////			{
////				float axdir[3];
////				DIFF(axdir[0], mCoord, i, x, 5);
////				DIFF(axdir[1], mCoord, i, y, 5);
////				DIFF(axdir[2], mCoord, i, z, 5);
////
////				r = AR;
////				for (int j=0; j<ITER_POSITION; j++)
////				{
////					fitPosition(img4d, sz,  0,   r*2, x, y, z,  axdir, zthickness);
////					r = fitRadius(img4d, sz, imgTH,  AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
////				}
////			}
//        }

//        mCoord[i].r = r;
//        mCoord[i].x = x;
//        mCoord[i].y = y;
//        mCoord[i].z = z;
//    }

//    if (channelsToUse) {delete []channelsToUse; channelsToUse=0;}

//    return true;
//}

//bool proj_trace_compute_radius_of_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], vector< vector<V_NeuronSWC_unit> >& mmUnit, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness, bool b_smooth)
//{
//    v3d_msg("proj_trace_compute_radius_of_last_traced_neuron(). \n", 0);
//    CHECK_DATA_GD_TRACING(false);

//    int chano = trace_para.channo;
//    int smoothing_win_sz = trace_para.sp_smoothing_win_sz;

//    for(int iseg=0; iseg<mmUnit.size(); iseg++)
//    {
//        if (iseg <seg_begin || iseg >seg_end) continue;
//        printf("#seg=%d(%d) ", iseg, mmUnit[iseg].size());
//        std::vector<V_NeuronSWC_unit> & mUnit = mmUnit[iseg]; // do in place
//        {
//            fit_radius_and_position(p4d[chano], sz[0], sz[1], sz[2],
//                                    mUnit,
//                                    false,       // do not move points here
//                                    myzthickness,
//                                    trace_para.b_3dcurve_width_from_xyonly);

//            if (b_smooth)
//                smooth_radius(mUnit, smoothing_win_sz, false);
//        }
//    }
//    printf("\n");

//    return true;
//}


//NeuronTree v3dneuron_GD_tracing(unsigned char ****p4d, V3DLONG sz[4],
//                                LocationSimple & p0, vector<LocationSimple> & pp,
//                                CurveTracePara & trace_para, double trace_z_thickness)
//{
//    NeuronTree nt;
//    if (!p4d || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || trace_para.channo<0 || trace_para.channo>=sz[3])
//    {
//        fprintf(stderr, "Invalid image or sz for v3dneuron_GD_tracing().\n");
//        return nt;
//    }

//    V_NeuronSWC_list tracedNeuron;
//    vector< vector<V_NeuronSWC_unit> > mmUnit;

//    //
//    tracedNeuron = trace_one_pt_to_N_points_shortestdist(p4d, sz, p0, pp, trace_para, trace_z_thickness, mmUnit);
//    //

//    if (pp.size()>0) //trace to some selected markers
//    {
////        if (trace_para.b_deformcurve==false && tracedNeuron.nsegs()>=1)	{proj_trace_smooth_downsample_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1);v3d_msg("trace_para.b_deformcurve==false && tracedNeuron.nsegs()>=1",0);}
////        if (trace_para.b_estRadii==true) {proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, true);v3d_msg("trace_para.b_estRadii==true");}
////        if (trace_para.b_postMergeClosebyBranches && tracedNeuron.nsegs()>=2) {proj_trace_mergeAllClosebyNeuronNodes(tracedNeuron); v3d_msg("trace_para.b_postMergeClosebyBranches && tracedNeuron.nsegs()>=2",0);}
//    }
//    else //if (sz[3]==1) //trace to the entire image
//    {
//        if ( trace_para.b_post_trimming )
//        {
//            //note the dark-pruning is done in GD tracing

//            //dt

//            bool b_to_background = true;
//            bool b_3d_dt = true;
//            float *dd = new float [sz[0]*sz[1]*sz[2]];
//            //fastmarching_dt(p4d[trace_para.channo][0][0], dd, sz[0], sz[1], sz[2], 2, 30);
//            if(!dd) {cerr<<"dd error"<<endl; exit(0);}
//            dt_v3dneuron(p4d[trace_para.channo][0][0], sz, trace_para.imgTH, b_to_background, b_3d_dt, dd);

//            //



//                // estimate radius for filtering
//            trace_para.b_3dcurve_width_from_xyonly = true;

//            //covered-leaf pruning

//            V3DLONG ninitotalpruned = mmUnit[0].size();
//            V3DLONG ntotalpruned = 0;
//            if (1)
//            {
//                for (int tmp=1; ; tmp++)
//                {
//                    V3DLONG nnodes = mmUnit[0].size();
//                    printf("\n***** iteration [%d] ...... neuron node # = %ld ...... \n", tmp, mmUnit[0].size());
//                    proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, mmUnit, trace_para, 0, mmUnit.size()-1, trace_z_thickness, false);
//                    condense_branches(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness); //when disable bad for Jefferies data. need to find a better way as only deform leaf node is not optimal. 2011-01-13
//                    V3DLONG npruned = 0;
//                    npruned += pruning_covered_leaf_single_cover(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                    npruned += pruning_covered_leaf_multi_covers(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                    printf("totally pruned [%ld, or %5.4f%%] nodes in this [%d] iteration. total pruned so far [%ld, or %5.4f%%]\n",
//                           npruned, double(npruned)/nnodes*100.0, tmp, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);

//                    ntotalpruned += npruned;

//                    if (npruned<=0 || double(npruned)/nnodes<0.001)
//                        break;
//                }
//            }

//            //remove other closeby branches (may not have been detected due to imperfect reconstruction node radius estimation)

//            if (0) //disabled on 110801
//            {
//                ntotalpruned += pruning_covered_leaf_closebyfake_branches(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness, trace_para.visible_thresh);
//            }

//            //inter-node pruning

//            if (1)
//            {
//                V3DLONG nnodes = mmUnit[0].size();
//                V3DLONG ninternodepruned = pruning_internodes(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                ntotalpruned += ninternodepruned;
//                printf("totally pruned [%ld, or %5.4f%%] nodes in the inter-node iteration. total pruned so far [%ld, or %5.4f%%]\n",
//                       ninternodepruned, double(ninternodepruned)/nnodes*100.0, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);
//            }


//            //branch-node pruning. to be added 110801
//            if (1)
//            {
//                V3DLONG nnodes = mmUnit[0].size();
//                V3DLONG nbranchnodepruned = pruning_branch_nodes(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                ntotalpruned += nbranchnodepruned;
//                printf("totally pruned [%ld, or %5.4f%%] nodes in the branch-node iteration. total pruned so far [%ld, or %5.4f%%]\n",
//                       nbranchnodepruned, double(nbranchnodepruned)/nnodes*100.0, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);

//            }

//            //redo leaf and branch node pruning
//            if (1)
//            {
//                printf("mmUnit size=%d",mmUnit.size());
//                if (mmUnit.size()>0)
//                    proj_trace_smooth_traced_neuron(p4d, sz, mmUnit, trace_para, 0, mmUnit.size()-1);
//                proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, false);

//                printf("\n***** REDO leaf pruning neuron node # = %ld ...... \n", mmUnit[0].size());
//                for (int tmp=1; ; tmp++)
//                {
//                    V3DLONG nnodes = mmUnit[0].size();
//                    printf("\n***** iteration [%d] ...... neuron node # = %ld ...... \n", tmp, mmUnit[0].size());
//                    proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, mmUnit, trace_para, 0, mmUnit.size()-1, trace_z_thickness, false);
//                    condense_branches(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness); //when disable bad for Jefferies data. need to find a better way as only deform leaf node is not optimal. 2011-01-13
//                    V3DLONG npruned = 0;
//                    npruned += pruning_covered_leaf_single_cover(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                    npruned += pruning_covered_leaf_multi_covers(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                    printf("totally pruned [%ld, or %5.4f%%] nodes in this [%d] iteration. total pruned so far [%ld, or %5.4f%%]\n",
//                           npruned, double(npruned)/nnodes*100.0, tmp, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);

//                    ntotalpruned += npruned;

//                    if (npruned<=0 || double(npruned)/nnodes<0.001)
//                        break;
//                }

//                if (1)
//                {
//                    V3DLONG nnodes = mmUnit[0].size();
//                    V3DLONG nbranchnodepruned = pruning_branch_nodes(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                    ntotalpruned += nbranchnodepruned;
//                    printf("totally pruned [%ld, or %5.4f%%] nodes in the branch-node iteration. total pruned so far [%ld, or %5.4f%%]\n",
//                           nbranchnodepruned, double(nbranchnodepruned)/nnodes*100.0, ntotalpruned, double(ntotalpruned)/ninitotalpruned*100.0);

//                }

//            }

//            //detect the artificial long, straight & dark branches
//            if (trace_para.b_pruneArtifactBranches)
//            {
//                V3DLONG nArtificialNodes = pruning_artifacial_branches(mmUnit, p4d[trace_para.channo], dd, sz, trace_z_thickness, trace_para.visible_thresh);
//                while (1)
//                {
//                    V3DLONG nFinalSingleCoverPruned = pruning_covered_leaf_single_cover(mmUnit, p4d[trace_para.channo],  dd, sz, trace_z_thickness);
//                    if (nFinalSingleCoverPruned<=0)
//                        break;
//                }
//            }

//        }

//        //

//        int n_end_nodes = 0;//for (V3DLONG ii=0;ii<mmUnit[0].size();ii++)	{if (mmUnit[0][ii].nchild==0) n_end_nodes++;}
//        V_NeuronSWC_list tn_new;
//        int nSegsTrace = mergeback_mmunits_to_neuron_path(n_end_nodes, mmUnit, tn_new);
//        tracedNeuron = tn_new; //use tn_new as the merge function will merge the old and new
//        printf("nSegsTrace=%d, n_end_nodes=%ld\n", nSegsTrace, n_end_nodes);

//        proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, false);


//    }

//    nt = convertNeuronTreeFormat(tracedNeuron);
//    return nt;
//}

//NeuronTree LinkOnePoint(unsigned char ****p4d, V3DLONG sz[4],
//LocationSimple & p0, LocationSimple & p1,
//CurveTracePara & trace_para, double trace_z_thickness)
//{
//    NeuronTree nt;
//    if (!p4d || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || trace_para.channo<0 || trace_para.channo>=sz[3])
//    {
//        fprintf(stderr, "Invalid image or sz for v3dneuron_GD_tracing().\n");
//        return nt;
//    }

//    V_NeuronSWC_list tracedNeuron;
//    vector< vector<V_NeuronSWC_unit> > mmUnit;

//    vector<LocationSimple>pp;
//    pp.push_back(p1);
//    //
//    tracedNeuron = trace_one_pt_to_N_points_shortestdist(p4d, sz, p0, pp, trace_para, trace_z_thickness, mmUnit);
//    //

//    if (0) //trace to some selected markers
//    {
////        if (trace_para.b_deformcurve==false && tracedNeuron.nsegs()>=1)	{proj_trace_smooth_downsample_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1);v3d_msg("trace_para.b_deformcurve==false && tracedNeuron.nsegs()>=1",0);}
////        if (trace_para.b_estRadii==true) {proj_trace_compute_radius_of_last_traced_neuron(p4d, sz, tracedNeuron, trace_para, 0, tracedNeuron.nsegs()-1, trace_z_thickness, true);v3d_msg("trace_para.b_estRadii==true");}
////        if (trace_para.b_postMergeClosebyBranches && tracedNeuron.nsegs()>=2) {proj_trace_mergeAllClosebyNeuronNodes(tracedNeuron); v3d_msg("trace_para.b_postMergeClosebyBranches && tracedNeuron.nsegs()>=2",0);}
//    }

//    nt = convertNeuronTreeFormat(tracedNeuron);
//    return nt;
//}
