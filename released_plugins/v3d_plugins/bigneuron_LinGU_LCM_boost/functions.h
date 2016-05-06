/*
 * functions.h
 *
 *  Created on: May 18, 2015
 *      Author: gulin
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_



typedef int vertex_t;
typedef double weight_t;

const weight_t max_weight = std::numeric_limits<double>::infinity();


struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
};



struct trip_idx
{
    int x;
    int y;
    int z;
    trip_idx(int arg_x, int arg_y, int arg_z)
        : x(arg_x), y(arg_y), z(arg_z) { }
};


typedef std::vector<std::vector<neighbor> > adjacency_list_t;


void DijkstraComputePaths(vertex_t source,
						  vertex_t target,
                          const adjacency_list_t &adjacency_list,
                          std::vector<weight_t> &min_distance,
                          std::vector<vertex_t> &previous);


std::list<vertex_t> DijkstraGetShortestPathTo(
    vertex_t vertex, const std::vector<vertex_t> &previous);


int bwconnmp_img(cv::Mat input_img, cv::Mat &label_img);

int count_labels_img(cv::Mat label_img);

int bwconnmp(cv::Mat label_img,std::vector<std::vector<int> > &PixelIdxListx,std::vector<std::vector<int> > &PixelIdxListy,std::vector<std::vector<int> > &PixelIdxListz);

int determine_bounding_box(int *bnd,std::vector<std::vector<int> > PixelIdxListx,std::vector<std::vector<int> > PixelIdxListy,std::vector<std::vector<int> > PixelIdxListz);

int collect_hog(cv::Mat label_img,cv::Mat & hist1,int base_thrs,std::vector<std::vector<int> > PixelIdxListx,
		std::vector<std::vector<int> > PixelIdxListy,std::vector<std::vector<int> > PixelIdxListz);


int local_ftrs(cv::Mat image,cv::Mat & centre_cc1,cv::Mat & cub1,std::vector<std::vector<int> >  PixelIdxListx,
		std::vector<std::vector<int> >  PixelIdxListy,std::vector<std::vector<int> >  PixelIdxListz);


int link_nearest(int *min_dist1, std::vector <int> frag_list, std::vector <int> base_list, cv::Mat &frag_p1,
		cv::Mat &base_p1, std::vector<std::vector<int> >  PixelIdxListx,std::vector<std::vector<int> >  PixelIdxListy,
		std::vector<std::vector<int> >  PixelIdxListz);

int curvature_link(double *curvature_l, int *bnd, std::vector <int> frag_list,cv::Mat frag_p, cv::Mat base_p);

int link_ftrs( int *min_dist1, double * curvature_l, int base_thrs, int *bnd, cv::Mat & frag_p1,
		cv::Mat & base_p1, std::vector<std::vector<int> >  PixelIdxListx,std::vector<std::vector<int> >  PixelIdxListy,
		std::vector<std::vector<int> >  PixelIdxListz);

int fill_features(cv::Mat & feature_cc,std::vector<std::vector<int> > PixelIdxListx, int * bnd,int *min_dist1,
		double *curvature_l, cv::Mat hist1, cv::Mat cub1);

int rt_test(cv::Mat feature_cc, double * resp_tst);

int LCM_classify(cv::Mat feature_cc, double * resp_tst);


int recover_component(cv::Mat image,cv::Mat label_img, int * fila_frag, std::vector<std::vector<trip_idx> > Pathxyz,
		std::vector<std::vector<int> > PixelIdxListx,std::vector<std::vector<int> > PixelIdxListy,
		std::vector<std::vector<int> > PixelIdxListz);

int is_breach(int v3[], int im_sz[]);

/*
struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
};
*/

int assign_adj_neigh(cv::Mat sub_img,cv::Mat sub_label, int v3s[], int v3t[],std::vector<std::vector<neighbor> > &adj_list);


int ReadNumbers( const std::string & s, std::vector <double> & v );

void import_matrix_from_txt_file(const char* filename_X, std::vector <double>& v, int& rows, int& cols);


#endif /* FUNCTIONS_H_ */
