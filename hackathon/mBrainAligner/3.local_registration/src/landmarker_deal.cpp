

#include "landmarker_deal.h"

bool auto_warp_marker_sp(float & lam, vector<point3D64F> &ccf_all_marker, vector<point3D64F> &fmost_all_marker, 
	vector<point3D64F>&sub_marker, bool outline)
{
	Matrix x4x4_affine, xnx4_c, xnxn_K;
	if (!q_TPS_cd(ccf_all_marker, fmost_all_marker, lam, x4x4_affine, xnx4_c, xnxn_K))
	{
		printf("ERROR: q_TPS_cd() return false.\n");
	}

	for (long long l = 0; l < sub_marker.size(); l++)
	{
		point3D64F pt_sub, pt_sub2tar;
		pt_sub.x = sub_marker[l].x;
		pt_sub.y = sub_marker[l].y;
		pt_sub.z = sub_marker[l].z;
		if (!q_compute_ptwarped_from_stps_3D(pt_sub, ccf_all_marker, x4x4_affine, xnx4_c, pt_sub2tar))
		{
			printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
			return false;
		}
		if (outline)
		{
			if (sub_marker[l].outline == 1)
			{
				sub_marker[l].x = pt_sub2tar.x;
				sub_marker[l].y = pt_sub2tar.y;
				sub_marker[l].z = pt_sub2tar.z;
			}
		}
		else
		{
			if (sub_marker[l].outline == 0)
			{
				sub_marker[l].x = pt_sub2tar.x;
				sub_marker[l].y = pt_sub2tar.y;
				sub_marker[l].z = pt_sub2tar.z;
			}
		}
	}
	return true;
}

bool auto_warp_marker(float & lam, vector<point3D64F> &ccf_all_marker, vector<point3D64F> &fmost_all_marker, vector<point3D64F>&sub_marker)
{
	Matrix x4x4_affine, xnx4_c, xnxn_K;
	if (!q_TPS_cd(ccf_all_marker, fmost_all_marker, lam, x4x4_affine, xnx4_c, xnxn_K))
	{
		printf("ERROR: q_TPS_cd() return false.\n");
	}

	for (long long l = 0; l < sub_marker.size(); l++)
	{
		point3D64F pt_sub, pt_sub2tar;
		pt_sub.x = sub_marker[l].x;
		pt_sub.y = sub_marker[l].y;
		pt_sub.z = sub_marker[l].z;
		if (!q_compute_ptwarped_from_stps_3D(pt_sub, ccf_all_marker, x4x4_affine, xnx4_c, pt_sub2tar))
		{
			printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
			return false;
		}
		sub_marker[l].x = pt_sub2tar.x;
		sub_marker[l].y = pt_sub2tar.y;
		sub_marker[l].z = pt_sub2tar.z;

	}
	return true;
}

bool auto_warp_average_marker(vector<double> label, vector<point3D64F> &ccf_all_marker, vector<point3D64F> &fmost_all_marker, vector<point3D64F>&sub_marker)
{
	Matrix x4x4_affine, xnx4_c, xnxn_K;
	if (!q_TPS_cd(ccf_all_marker, fmost_all_marker, 0, x4x4_affine, xnx4_c, xnxn_K))
	{
		printf("ERROR: q_TPS_cd() return false.\n");
	}

	for (long long l = 0; l < sub_marker.size(); l++)
	{
		point3D64F pt_sub, pt_sub2tar;
		pt_sub.x = sub_marker[l].x;
		pt_sub.y = sub_marker[l].y;
		pt_sub.z = sub_marker[l].z;
		if (!q_compute_ptwarped_from_stps_3D(pt_sub, ccf_all_marker, x4x4_affine, xnx4_c, pt_sub2tar))
		{
			printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
			return false;
		}
		bool update_marker = false;
		for (int i = 0; i < label.size(); i++)
		{
			if (sub_marker[l].label == label[i])
			{
				update_marker = true;
				break;
			}

		}
		if (update_marker)
		{
			sub_marker[l].x = pt_sub2tar.x;
			sub_marker[l].y = pt_sub2tar.y;
			sub_marker[l].z = pt_sub2tar.z;
		}
	}
	return true;
}

bool landmark_region(vector<point3D64F>& vec_corners, vector<point3D64F>& fine_sub_corner, vector<point3D64F> & aver_corner, float **** & p_img_label, vector<int> &label, long long *sz_img_tar)
{
	label.clear();
	vector<point3D64F> vec_corners_range, fine_sub_corner_range, aver_sub_range;
//#pragma omp parallel for
	for (int i = 0; i < vec_corners.size(); i++)
	{
		int label_c = p_img_label[0][int(vec_corners[i].z)][int(vec_corners[i].y)][int(vec_corners[i].x)];
		if (label_c == 0)
		{
			double dis = 10000;
			int search = 5;
			while (label_c == 0)
			{
				for (int z = -search; z < search; z++)
					for (int y = -search; y < search; y++)
						for (int x = -search; x < search; x++)
						{
							int xx = int(vec_corners[i].x) + x;
							int yy = int(vec_corners[i].y) + y;
							int zz = int(vec_corners[i].z) + z;
							if (zz < 0 || yy < 0 || xx < 0 || zz >= sz_img_tar[2] || yy >= sz_img_tar[1] || xx >= sz_img_tar[0] )
								continue;
							double dis1 = sqrt(z*z + y*y + x*x);
							int label_s=p_img_label[0][zz][yy][xx];
							if (dis1 < dis && label_s != 0)
							{
								dis = dis1;
								label_c = label_s;
							}
						}
				search = search + 5;
			}
			
		}
		
		vec_corners[i].label = label_c;
		fine_sub_corner[i].label = label_c;
		aver_corner[i].label = label_c;
		vector<int>::iterator ret;
		ret = std::find(label.begin(), label.end(), label_c);
		if (ret == label.end())
			label.push_back(label_c);
	}
	return true;
}

bool find_nearst_landmarks(vector< vector<neighborinfo> > &vec2d_neighborinfo, vector<point3D64F>& vec_corners, int &nneighbors)

{
	float xx, yy, zz, dis;
	for (int i = 0; i < vec_corners.size(); i++)
	{
		vector<neighborinfo> vec_neighborinfo;

		for (int j = 0; j < vec_corners.size(); j++)
		{
			if (vec_corners[i].label != vec_corners[j].label)
				continue;
			xx = vec_corners[i].x - vec_corners[j].x;
			yy = vec_corners[i].y - vec_corners[j].y;
			zz = vec_corners[i].z - vec_corners[j].z;
			dis = sqrt(xx*xx + yy*yy + zz*zz);
			vec_neighborinfo.push_back(neighborinfo(j, dis, vec_corners[j].label));

		}
		sort(vec_neighborinfo.begin(), vec_neighborinfo.end(), compare_neighborinfo);
		vector<neighborinfo> vec_tmp(nneighbors, neighborinfo());
		for (int j = 0; j < nneighbors; j++)
		{
			vec_tmp[j].ind = vec_neighborinfo[j].ind;
			vec_tmp[j].dis = vec_neighborinfo[j].dis;
			vec_tmp[j].ind_reg = vec_neighborinfo[j].ind_reg;
		}
		vec2d_neighborinfo.push_back(vec_tmp);
	}
	return true;
}

bool update_average_landmarker(vector<point3D64F> vec_corners, vector<point3D64F> fine_sub_corner, vector<point3D64F> & aver_corner)
{
	vector<point3D64F> average_raw = aver_corner, vec_corners_tar, vec_corners_sub;
	vec_corners_tar.clear();
	vec_corners_sub.clear();
	for (int i = 0; i < vec_corners.size(); i++)
	{
		if (vec_corners[i].outline == 1)
		{
			vec_corners_tar.push_back(vec_corners[i]);
			vec_corners_sub.push_back(fine_sub_corner[i]);
			aver_corner[i] = vec_corners[i];
		}
	}
	bool outline = true;
	float at_lam = 0.2;

	auto_warp_marker_sp(at_lam, vec_corners_tar, vec_corners_sub, aver_corner, outline);

	vec_corners_tar.clear();
	vec_corners_sub.clear();
	for (int i = 0; i < vec_corners.size(); i++)
	{
		if (vec_corners[i].outline == 1)
		{
			vec_corners_tar.push_back(average_raw[i]);
			vec_corners_sub.push_back(aver_corner[i]);
		}
	}
	outline = false;
	at_lam = 0.2;
	auto_warp_marker_sp(at_lam, vec_corners_tar, vec_corners_sub, aver_corner, outline);
}

bool load_fine_marker(QString fine_filename, vector<point3D64F> &vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> &aver_corner, Parameter &input_Parameter)
{
	QString tar_file, fine_sub_file, average_file;

	QDir dir(fine_filename);
	if (!dir.exists()) {
		printf("ERROR: fine_filename path is not exist!!!!\n");
		return -1;
	}

	//取到所有的文件和文件名，但是去掉.和..的文件夹（这是QT默认有的）
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

	//文件夹优先
	dir.setSorting(QDir::DirsFirst);

	//转化成一个list
	QFileInfoList list_image = dir.entryInfoList();
	if (list_image.size()< 1 || list_image.size()>3) {
		printf("ERROR: The number of subfiles in the fine file is not correct (only three), please check!!!!!!\n");
		return -1;
	}

	int J = 0;

	do{
		QFileInfo fileInfo_img = list_image.at(J);

		qDebug() << fileInfo_img.filePath() << ":" << fileInfo_img.fileName();
		QString file_norm_path = fileInfo_img.filePath();
		if (fileInfo_img.fileName().section("_", 1,1).section(".", 0, 0) == "tar")
		{
			tar_file = fileInfo_img.filePath();
			input_Parameter.star_iter = fileInfo_img.fileName().section("_", 0, 0).toInt();
			printf("star iteration number:%d \n", input_Parameter.star_iter);
		}
		else if (fileInfo_img.fileName().section("_", 1,1).section(".", 0, 0) == "sub")
		{
			fine_sub_file = fileInfo_img.filePath();
		}
		else if (fileInfo_img.fileName().section("_", 1,1).section(".", 0, 0) == "average")
		{
			average_file = fileInfo_img.filePath();
		}
		J++;
	} while (J < list_image.size());

	if (tar_file.isNull() || fine_sub_file.isNull() || average_file.isNull())
	{
		printf("ERROR: The name of subfiles in the fine file is not correct, please check!!!!!!\n");
		return -1;
	}
	QList<ImageMarker> marker_tar, marker_sub, average_marker;
	marker_tar = readMarker_file(tar_file);
	marker_sub = readMarker_file(fine_sub_file);
	average_marker = readMarker_file(average_file);

	if (marker_tar.size() != marker_sub.size() || marker_sub.size() != average_marker.size())
	{
		printf("ERROR: The number of loading landmarks is not equal, please check!!!!!!\n");
		return -1;
	}
	point3D64F re_tmp;
	for (long long i = 0; i < marker_tar.size(); i++)
	{
		re_tmp.x = marker_tar[i].x; re_tmp.y = marker_tar[i].y; re_tmp.z = marker_tar[i].z; vec_corners.push_back(re_tmp);
		re_tmp.x = marker_sub[i].x; re_tmp.y = marker_sub[i].y; re_tmp.z = marker_sub[i].z; fine_sub_corner.push_back(re_tmp);
		re_tmp.x = average_marker[i].x; re_tmp.y = average_marker[i].y; re_tmp.z = average_marker[i].z; aver_corner.push_back(re_tmp);
	}
	return true;
}


bool cmp(int x, int y) ///cmp函数传参的类型不是vector<int>型，是vector中元素类型,即int型
{
	return x<y;
}

bool compare_neighborinfo(const neighborinfo& first, const neighborinfo& second)
{
	return (first.dis < second.dis);
}

bool compare_outline(const point3D64F& first, const point3D64F& second)
{
	return (first.outline > second.outline);
}

bool compare_label(const point3D64F& first, const point3D64F& second)
{
	return (first.label < second.label);
}
