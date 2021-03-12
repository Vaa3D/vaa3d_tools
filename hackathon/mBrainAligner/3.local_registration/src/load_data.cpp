
#include "load_data.h"
#include"preprocessing.h"
#include"landmarker_deal.h"

bool load_density_map(QString qs_filename_img_sub_seg,  map <int, float *> & density_map_sub)
{
	
	int array[10] =  { 62, 75, 80, 100, 145, 159, 168, 237, 249 };
	
	vector<int> label_map;
	for (int i = 0; i < 9; i++)
	{
		QString map_file_path = qs_filename_img_sub_seg  + QString::number(i + 1) + ".v3draw";
		unsigned char *p_img_map = 0;
		long long *sz_img_map = 0;
		int datatype_map = 0;
		if (!map_file_path.isNull())
		{
			if (!loadImage((char *)qPrintable(map_file_path), p_img_map, sz_img_map, datatype_map))
			{
				printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(map_file_path));
				return false;
			}
			printf("\t>>read image file [%s] complete.\n", qPrintable(map_file_path));
			printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_map[0], sz_img_map[1], sz_img_map[2], sz_img_map[3]);
			printf("\t\tdatatype: %d\n", datatype_map);
		}
		float *map = (float *)p_img_map;
		density_map_sub.insert(pair<int, float*>(array[i], map));
		
	}
}

bool loadImageData(Parameter input_Parameter,QString data_file, QString qs_filename_img_sub, unsigned char *&p_img_sub, float *&p_img32f_tar, float *&p_img32f_sub_bk,
	float *& p_img32_sub_label,long long *&sz_img)
{
	QString qs_filename_img_tar, qs_filename_img_label;
	
	unsigned char *p_img_tar = 0,  *p_img_sub_label = 0 ;
	long long *sz_img_tar = 0, *sz_img_sub = 0, *sz_img_label = 0;
	int datatype_tar, datatype_sub, datatype_label;

	//-----------------------------------------------------------------------------------------
	printf("1-1. load 3D raw image. \n");

	if (!qs_filename_img_sub.isNull())
	{
		if (!loadImage((char *)qPrintable(qs_filename_img_sub), p_img_sub, sz_img_sub, datatype_sub))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
			return false;
		}
		printf("\t>>read sub image file [%s] complete.\n", qPrintable(qs_filename_img_sub));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]);
		printf("\t\tdatatype: %d\n", datatype_sub);
		if (datatype_sub != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
	}

	if (input_Parameter.Select_modal < 2)
	{
		qs_filename_img_tar = data_file + "/average_template_25_u8_xpad.v3draw";
		qs_filename_img_label = data_file + "/region_attention.v3draw";
		
		if (!qs_filename_img_label.isNull())
		{
			if (!loadImage((char *)qPrintable(qs_filename_img_label), p_img_sub_label, sz_img_label, datatype_label))
			{
				printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_label));
				return false;
			}
			printf("\t>>read label image file [%s] complete.\n", qPrintable(qs_filename_img_label));
			printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_label[0], sz_img_label[1], sz_img_label[2], sz_img_label[3]);
			printf("\t\tdatatype: %d\n", datatype_label);
		}
		long long l_npixels = sz_img_label[0] * sz_img_label[1] * sz_img_label[2] * sz_img_label[3];
		p_img32_sub_label = new(std::nothrow) float[l_npixels]();
		for (int i = 0; i < l_npixels; i++)
		{
			p_img32_sub_label[i] = p_img_sub_label[i];
		}	
	}
	else
	{
		qs_filename_img_tar = data_file + "/Target_image.v3draw";
	}

	if (!qs_filename_img_tar.isNull())
	{
		if (!loadImage((char *)qPrintable(qs_filename_img_tar), p_img_tar, sz_img_tar, datatype_tar))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
			return false;
		}
		printf("\t>>read tar image file [%s] complete.\n", qPrintable(qs_filename_img_tar));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]);
		printf("\t\tdatatype: %d\n", datatype_tar);
		if (datatype_tar != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
	}

	
	if (sz_img_tar[0] != sz_img_sub[0] || sz_img_tar[1] != sz_img_sub[1] || sz_img_tar[2] != sz_img_sub[2] || sz_img_tar[3] != 1 || sz_img_sub[3] != 1)
	{
		printf("ERROR: Input images have different size or channel > 1.\n");
		return false;
	}
	else
	{
		sz_img = sz_img_tar;
	}

	long long l_npixels = sz_img_tar[0] * sz_img_tar[1] * sz_img_tar[2];

	//-----------------------------------------------------------------------------------------
	printf("1-2. Convert image datatype from uint8 to 32f and rescale to [0~255]. \n");
	float *p_img32f_tar255 = 0, *p_img32f_sub255 = 0;

	if (!Convert_image255(p_img32f_tar255, p_img32f_sub255,  p_img_sub, p_img_tar,  l_npixels))
	{
		printf("Error Convert_image255() is wrong!");
		return false;
	}

	//-----------------------------------------------------------------------------------------
	printf("1-3-1. Calculate the gradient sub_images. \n");

	float *grand_sub = 0;

	if (!Calculate_gradient_img(grand_sub, sz_img_sub, p_img32f_sub255, p_img_sub))
	{
		printf("Error sub Calculate_gradient_img() is wrong!");
		return false;
	}

	printf("1-3-2. Calculate the gradient tar_images. \n");

	float  *grand_tar = 0;
	if (!Calculate_gradient_img(grand_tar, sz_img_sub, p_img32f_tar255, p_img_tar))
	{
		printf("Error tar Calculate_gradient_img() is wrong!");
		return false;
	}

	//-----------------------------------------------------------------------------------------
	printf("1-4. Convert image datatype from uint8 to 32f and scale to [0~1]. \n");

	if (!Convert_image_datatype(p_img32f_tar,  p_img32f_sub_bk, grand_tar, grand_sub, p_img_sub, sz_img_tar))
	{
		printf("Error tar Convert_image_datatype() is wrong!");
		return false;
	}

	if (p_img_tar) 		{ delete[]p_img_tar;		p_img_tar = 0; }
	if (p_img32f_tar255) 		{ delete[]p_img32f_tar255;		p_img32f_tar255 = 0; }
	if (p_img32f_sub255) 		{ delete[]p_img32f_sub255;		p_img32f_sub255 = 0; }
	if (p_img_sub_label) 		{ delete[]p_img_sub_label;		p_img_sub_label = 0; }

	return true;
}

bool LoadLandmarksData(vector<point3D64F> &vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> &aver_corner, vector<int> &label, QString data_file,
	QString fine_filename, long long *sz_img, Parameter &input_Parameter, float  **** p_img_label_4d,
	QString qs_filename_img_sub_seg, map <int, float *> & density_map_sub,float*& fmost_label_edge , float  ****&fmost_label_edge_4d)
{
	

	QString qs_filename_landmark_tar = data_file + "/landmarker_tar.marker";
	if (fine_filename != NULL)
	{
		printf("2-1. Load finetune landmarks. \n");
		load_fine_marker(fine_filename, vec_corners, fine_sub_corner, aver_corner, input_Parameter);
		
	}
	else
	{
		printf("2-1. Load landmarks of target image. \n");
		QList<ImageMarker> ql_marker_tar;
		{
			ql_marker_tar = readMarker_file(qs_filename_landmark_tar);
			point3D64F tmp;
			for (long long i = 0; i < ql_marker_tar.size(); i++)
			{
				tmp.x = ql_marker_tar[i].x; tmp.y = ql_marker_tar[i].y; tmp.z = ql_marker_tar[i].z;
				vec_corners.push_back(tmp);
			}
		}
		fine_sub_corner = vec_corners;
	}


	// Determine whether the landmarks belongs to the contour, so that the internal landmarks and the external landmarks can be registered separately.
	if (input_Parameter.Select_modal < 2)
	{
		QString ccf_edge_file = data_file + "/edge.v3draw";
		unsigned char   *ccf_edge = 0;
		long long *sz_img_ccf_edge = 0;
		int datatype_ccf_edge;

		if (!ccf_edge_file.isNull())
		{
			if (!loadImage((char *)qPrintable(ccf_edge_file), ccf_edge, sz_img_ccf_edge, datatype_ccf_edge))
			{
				printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(ccf_edge_file));
				return false;
			}
			printf("\t>>read ccf edge image file [%s] complete.\n", qPrintable(ccf_edge_file));
			printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_ccf_edge[0], sz_img_ccf_edge[1], sz_img_ccf_edge[2], sz_img_ccf_edge[3]);
			printf("\t\tdatatype: %d\n", datatype_ccf_edge);
		}
		for (int i = 0; i < vec_corners.size(); i++)
		{
			int index = vec_corners[i].z*sz_img_ccf_edge[0] * sz_img_ccf_edge[1] + vec_corners[i].y*sz_img_ccf_edge[0] + vec_corners[i].x;
			if (ccf_edge[index] != 0)
			{
				vec_corners[i].outline = 1, fine_sub_corner[i].outline = 1;
			}
			else
			{
				vec_corners[i].outline = 0, fine_sub_corner[i].outline = 0;
			}
		}

		if (ccf_edge) 		{ delete[]ccf_edge;		ccf_edge = 0; }

		//landmark region
		aver_corner = vec_corners;
		if (!landmark_region(vec_corners, fine_sub_corner, aver_corner, p_img_label_4d, label, sz_img))
		{
			printf("ERROR:landmark_region()!!!! \n");
			return true;
		}
		
		if (input_Parameter.Select_modal == 0)
		{
		/*	printf("Registration modal: fMOST to CCF \n");*/

			if (fine_filename.isNull())
			{
				// initail sample landmark
				printf("2-2 load initail landmarks of subject image \n");
				/// read fmost average brain marker file and update.
				vector<point3D64F> average_tar, average_sub;
				QList<ImageMarker> average_tar_file = readMarker_file(data_file + "/average_tar.marker");
				QList<ImageMarker> average_sub_file = readMarker_file(data_file + "/average_sub.marker");

				for (long long i = 0; i < average_tar_file.size(); i++)
				{
					point3D64F i_tmp;
					i_tmp.x = average_tar_file[i].x; i_tmp.y = average_tar_file[i].y; i_tmp.z = average_tar_file[i].z; i_tmp.label = average_tar_file[i].color.b; average_tar.push_back(i_tmp);
					i_tmp.x = average_sub_file[i].x; i_tmp.y = average_sub_file[i].y; i_tmp.z = average_sub_file[i].z; i_tmp.label = average_tar_file[i].color.b; average_sub.push_back(i_tmp);
				}

				vector<vector<double>> label_sp;
				{
					vector<double> label_s;
					label_s.push_back(75);
					label_s.push_back(80);
					label_sp.push_back(label_s);
					label_s.clear();
					label_s.push_back(237);
					label_sp.push_back(label_s);
					label_s.clear();
					int array[10] = { 62, 100, 145, 159, 168, 249 };
					for (int i = 0; i < 6; i++)
					{
						label_s.push_back(array[i]);
					}
					label_sp.push_back(label_s);
				}

				aver_corner = vec_corners;
				for (int i = 0; i < label_sp.size(); i++)
				{
					vector<double> label_av = label_sp[i];
					vector<point3D64F> vec_corners_region, fine_sub_region;
					for (int j = 0; j < average_tar.size(); j++)
					{
						bool update_marker = false;
						for (int k = 0; k < label_av.size(); k++)
						{
							if (average_tar[j].label == label_av[k])
							{
								update_marker = true;
								break;
							}
						}
						if (update_marker)
						{
							vec_corners_region.push_back(average_tar[j]);
							fine_sub_region.push_back(average_sub[j]);
						}
					
					}
					auto_warp_average_marker(label_av, vec_corners_region, fine_sub_region, aver_corner);
				}

				fine_sub_corner = aver_corner;
				/// update fine_sub_corner based on fmost segmentation result
								
				update_sub_corner(qs_filename_img_sub_seg, fmost_label_edge, fmost_label_edge_4d,  fine_sub_corner);
			
			}
			else
			{
				update_average_landmarker(vec_corners, fine_sub_corner, aver_corner);
			}

			// load density map 
			printf("2-3 load ten subject density map \n");
			if (!load_density_map(qs_filename_img_sub_seg, density_map_sub))
			{
				printf("ERROE:load_density_map()!!!! \n");
				return true;
			}
		
		}
		else
		{
			/*printf("Registration modal: other modal mouse image to CCF");*/
			aver_corner = vec_corners;
		}

		sort(label.begin(), label.end(), cmp);
		sort(vec_corners.begin(), vec_corners.end(), compare_label);
		sort(fine_sub_corner.begin(), fine_sub_corner.end(), compare_label);
		sort(aver_corner.begin(), aver_corner.end(), compare_label);
	}
	else if (input_Parameter.Select_modal == 2)
	{
		/*printf("Registration modal: Zebra fish registration");*/
		aver_corner = vec_corners;
	}
	
	//====================================================================================================================
	//save landmarks

	{
		QList<ImageMarker> ql_marker_tar, ql_marker_sub, ql_marker_aver;

		for (long long i = 0; i < vec_corners.size(); i++)
		{
			ImageMarker tmp;
			tmp.x = vec_corners[i].x;	tmp.y = vec_corners[i].y;	tmp.z = vec_corners[i].z; tmp.radius = 5, tmp.shape = 1; ql_marker_tar.push_back(tmp);
			tmp.x = fine_sub_corner[i].x;	tmp.y = fine_sub_corner[i].y;	tmp.z = fine_sub_corner[i].z; tmp.radius = 5, tmp.shape = 1;	ql_marker_sub.push_back(tmp);
			tmp.x = aver_corner[i].x;	tmp.y = aver_corner[i].y;	tmp.z = aver_corner[i].z; tmp.radius = 5, tmp.shape = 1;	ql_marker_aver.push_back(tmp);
		}
		char filename[2000];
		sprintf(filename, "%s/tar.marker", qPrintable(input_Parameter.save_path));	writeMarker_file(filename, ql_marker_tar);
		sprintf(filename, "%s/sub.marker", qPrintable(input_Parameter.save_path));	writeMarker_file(filename, ql_marker_sub);
		sprintf(filename, "%s/average.marker", qPrintable(input_Parameter.save_path));	writeMarker_file(filename, ql_marker_aver);
	}
}

bool outline_detec(float *& p_img_input, long long *& sz_img_input, float *& img_edge)
{
	long l_npixels = sz_img_input[0] * sz_img_input[1] * sz_img_input[2] * sz_img_input[3];
	for (long i = 0; i < l_npixels; i++)
		img_edge[i] = 0.0;
	for (int z = 0; z < sz_img_input[2]; z++)
		for (int y = 0; y < sz_img_input[1]; y++)
			for (int x = 0; x < sz_img_input[0]; x++)
			{
				float value = p_img_input[z * sz_img_input[1] * sz_img_input[0] + y * sz_img_input[0] + x];
				int cx, cy, cz;

				bool bb = false;
				for (cz = z - 2; cz < z + 2; ++cz)
				{
					for (cy = y - 2; cy < y + 2; ++cy)
					{
						for (cx = x - 2; cx < x + 2; ++cx)
						{
							if (cx < 0 || cx > sz_img_input[0] - 1 || cy < 0 || cy > sz_img_input[1] - 1 || cz < 0 || cz > sz_img_input[2] - 1)
								continue;
							//cout << p_img_input[cz * sz_img_input[1] * sz_img_input[0] + cy * sz_img_input[0] + cx] << endl;
							if (value != (float)p_img_input[cz * sz_img_input[1] * sz_img_input[0] + cy * sz_img_input[0] + cx])
							{
								img_edge[z * sz_img_input[1] * sz_img_input[0] + y * sz_img_input[0] + x] = 255.0;
								bb = true; break;
							}
						}
						if (bb) break;
					}
					if (bb) break;
				}
			}
	return true;
}

bool update_sub_corner(QString qs_filename_img_sub_seg, float * &fmost_label, float  **** &fmost_label_4d,  vector<point3D64F> & fine_sub_corner)
{
	vector<point3D64F> fine_sub_corner_raw = fine_sub_corner;
	QString fmost_label_file = qs_filename_img_sub_seg +  "seg.v3draw";
	unsigned char * fmost_label_char = 0;
	long long * label_size = 0;
	int datatype_sub = 0;

	if (!fmost_label_file.isNull())
	{
		if (!loadImage((char *)qPrintable(fmost_label_file), fmost_label_char, label_size, datatype_sub))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(fmost_label_file));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(fmost_label_file));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", label_size[0], label_size[1], label_size[2], label_size[3]);
		printf("\t\tdatatype: %d\n", datatype_sub);

	}
	fmost_label = (float *)fmost_label_char;

	long long l_npixels = label_size[0]*label_size[1]*label_size[2]*label_size[3];

	float * fmost_label_edge = 0;

	fmost_label_edge = new(std::nothrow) float[l_npixels]();

	outline_detec(fmost_label, label_size, fmost_label_edge);
	char filename[2000];
	
	int array[10] = {0, 62, 75, 80, 100, 145, 159, 168, 237, 249 };
	/*int array[1] = {100 };*/

	for (int i = 0; i < l_npixels; i++)
	{
		if (fmost_label_edge[i] != 0)
		{
			fmost_label_edge[i] = array[int(fmost_label[i])];
		}
		if (fmost_label[i] != 0)
		{
			fmost_label[i] = array[int(fmost_label[i])];
		}
			
	}
	float ****fmost_label_edge_4d = 0;
	new4dpointer(fmost_label_edge_4d, label_size[0], label_size[1], label_size[2], label_size[3], fmost_label_edge);
	new4dpointer(fmost_label_4d, label_size[0], label_size[1], label_size[2], label_size[3], fmost_label);

	int se_radius = 10;
#pragma omp parallel for
	for (int i = 0; i < fine_sub_corner.size(); i++)
	{
		if (fine_sub_corner[i].outline == 0)
			continue;
		float dis = 1000;
		int iter = 0;
		while (dis == 1000)
		{
			iter = iter + 1;
			
			/*cout << iter << endl;*/
			float X = 0, Y = 0, Z = 0;
			for (int zz = -se_radius; zz <= se_radius; zz++)
				for (int yy = -se_radius; yy <= se_radius; yy++)
					for (int xx = -se_radius; xx <= se_radius; xx++)
					{
						int x = fine_sub_corner[i].x + xx;
						int y = fine_sub_corner[i].y + yy;
						int z = fine_sub_corner[i].z + zz;
						float s_dis = pow(xx*xx + yy*yy + zz*zz, 0.5);
						if (x<0 || y<0 || z<0 || x >= label_size[0] || y >= label_size[1] || z >= label_size[2])
							continue;

						if (s_dis < dis && int(fmost_label_edge_4d[0][z][y][x]) == fine_sub_corner[i].label)
						{
							dis = s_dis;
							X = x;
							Y = y;
							Z = z;
						}
					}
			if (dis != 1000)
			{
				fine_sub_corner[i].x = X;
				fine_sub_corner[i].y = Y;
				fine_sub_corner[i].z = Z;
			}
			else
			{
				if (fine_sub_corner[i].outline == 1)
					se_radius = se_radius + 20;
				else
					dis = dis - 10;
			}

		}
	
	}

	vector<point3D64F> vec_corners_ouline, fine_sub_outline;
	
	for (int i = 0; i < fine_sub_corner_raw.size(); i++)
	{
		if (fine_sub_corner_raw[i].outline == 1)
		{
			vec_corners_ouline.push_back(fine_sub_corner_raw[i]);
			fine_sub_outline.push_back(fine_sub_corner[i]);
		}
	}

	bool outline = false;
	float at_lam = 0.2;
	auto_warp_marker_sp(at_lam, vec_corners_ouline, fine_sub_outline, fine_sub_corner, outline);

#pragma omp parallel for
	for (int i = 0; i < fine_sub_corner.size(); i++)
	{
		if (fine_sub_corner[i].outline == 1)
			continue;
		se_radius = 10;
		float dis = 1000;
		int iter = 0;
		while (dis == 1000)
		{
			iter = iter + 1;
			
			/*cout << iter << endl;*/
			float X = 0, Y = 0, Z = 0;
			for (int zz = -se_radius; zz <= se_radius; zz++)
				for (int yy = -se_radius; yy <= se_radius; yy++)
					for (int xx = -se_radius; xx <= se_radius; xx++)
					{
						int x = fine_sub_corner[i].x + xx;
						int y = fine_sub_corner[i].y + yy;
						int z = fine_sub_corner[i].z + zz;
						float s_dis = pow(xx*xx + yy*yy + zz*zz, 0.5);
						if (x<0 || y<0 || z<0 || x >= label_size[0] || y >= label_size[1] || z >= label_size[2])
							continue;

						if (s_dis < dis && int(fmost_label_edge_4d[0][z][y][x]) == fine_sub_corner[i].label)
						{
							dis = s_dis;
							X = x;
							Y = y;
							Z = z;
						}
					}
			if (dis != 1000)
			{
				fine_sub_corner[i].x = X;
				fine_sub_corner[i].y = Y;
				fine_sub_corner[i].z = Z;
			}
			else
			{
				if (fine_sub_corner[i].outline == 1)
					se_radius = se_radius + 20;
				else
					dis = dis - 10;
			}

		}

	}

	vector<point3D64F> warp_corner = fine_sub_corner_raw;
	at_lam =500;
	auto_warp_marker(at_lam, fine_sub_corner_raw, fine_sub_corner, warp_corner);
	fine_sub_corner = warp_corner;
	if (fmost_label_edge) 			{ delete[]fmost_label_edge;		fmost_label_edge = 0; }
}
	

