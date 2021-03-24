#include <iostream>
#include "edge_detection.h"
#include "basic_surf_objs.h"

using namespace std;
using namespace cv;

#define max(a,b) ((a)>(b)?(a):(b))


bool edgeContourExtract(unsigned char* img_tar, unsigned char* img_sub, long long* sz_tar, long long* sz_sub,
	vector<Coord3D_PCM>& tar_points, vector<Coord3D_PCM>& sub_points)
{
	if (img_tar == 0 || img_sub == 0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if (sz_tar[0] <= 0 || sz_tar[1] <= 0 || sz_tar[2] <= 0 || sz_tar[3] <= 0 ||
		sz_sub[0] <= 0 || sz_sub[1] <= 0 || sz_sub[2] <= 0 || sz_sub[3] <= 0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	vector<vector<Coord3D_PCM>> points_t(sz_tar[0]); 
	for (int x = 0; x < sz_tar[0]; x++)
	{
		vector<vector<Point>> contours;
		Mat img2_yz = Mat(sz_tar[1], sz_tar[2], CV_8UC1);
		for (int y = 0; y < sz_tar[1]; y++)
			for (int z = 0; z < sz_tar[2]; z++)
			{
				img2_yz.at<uchar>(y, z) = (int)img_tar[z*sz_tar[1] * sz_tar[0] + y*sz_tar[0] + x];
			}
		findContours(img2_yz, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		Mat linePic = Mat::zeros(img2_yz.rows, img2_yz.cols, CV_8UC1);
		for (int index = 0; index < contours.size(); index++)
		{
			vector<Point> contour;
			contour = contours[index];
			for (int i = 0; i < contour.size(); i++)
			{
				Coord3D_PCM point(x, contour[i].y, contour[i].x);
				points_t[x].push_back(point);
				linePic.at<uchar>(contour[i].y, contour[i].x) = 255;
			}
		}
	}

	int rang_tar,rang_sub, x_space, y_space, z_space;
	rang_tar = ceil(max(max(sz_tar[0], sz_tar[1]), sz_tar[2]) / 25);
	x_space = sz_tar[0] / rang_tar;
	y_space = sz_tar[1] / rang_tar;
	z_space = sz_tar[2] / rang_tar;
	for (int i = 0; i < y_space; i++)
	{
		for (int j = 0; j < z_space; j++)
		{
			for (int k = 0; k < x_space; k++)
			{
				vector<Coord3D_PCM> points;
				int cc;
				if (k != (x_space - 1))
				{
					cc = rang_tar * (k + 1);
				}
				else
					cc = sz_tar[0];
				for (int ind = rang_tar * k; ind < cc; ind++)
				{
					for (int a = 0; a < points_t[ind].size(); a++)
					{
						if (i != (y_space - 1) && j != (z_space - 1))
						{
							if (points_t[ind][a].y >= rang_tar*i && points_t[ind][a].y < rang_tar*(i + 1) &&
								points_t[ind][a].z >= rang_tar*j && points_t[ind][a].z < rang_tar*(j + 1))
								points.push_back(points_t[ind][a]);
						}
						else if (i != (y_space - 1) && j == (z_space - 1))
						{
							if (points_t[ind][a].y >= rang_tar*i && points_t[ind][a].y < rang_tar*(i + 1) &&
								points_t[ind][a].z >= rang_tar*j && points_t[ind][a].z < sz_tar[2])
								points.push_back(points_t[ind][a]);
						}
						else if (i == (y_space - 1) && j != (z_space - 1))
						{
							if (points_t[ind][a].y >= rang_tar*i && points_t[ind][a].y < sz_tar[1] &&
								points_t[ind][a].z >= rang_tar*j && points_t[ind][a].z < rang_tar*(j + 1))
								points.push_back(points_t[ind][a]);
						}
						else if (i == (y_space - 1) && j == (z_space - 1))
						{
							if (points_t[ind][a].y >= rang_tar*i && points_t[ind][a].y < sz_tar[1] &&
								points_t[ind][a].z >= rang_tar*j && points_t[ind][a].z < sz_tar[2])
								points.push_back(points_t[ind][a]);
						}
					}
				}
				int min_len = INT_MAX;
				int xx = rang_tar * k + (cc - rang_tar * k) / 2;
				int yy, zz;
				if (i != (y_space - 1) && j != (z_space - 1))
				{
					yy = rang_tar * i + (rang_tar * (i + 1) - rang_tar * i) / 2;
					zz = rang_tar * j + (rang_tar * (j + 1) - rang_tar * j) / 2;
				}
				else if (i != (y_space - 1) && j == (z_space - 1))
				{
					yy = rang_tar * i + (rang_tar * (i + 1) - rang_tar * i) / 2;
					zz = rang_tar * j + (sz_tar[2] - 1 - rang_tar * j) / 2;
				}
				else if (i == (y_space - 1) && j != (z_space - 1))
				{
					yy = rang_tar * i + (sz_tar[1] - 1 - rang_tar * i) / 2;
					zz = rang_tar * j + (rang_tar * (j + 1) - rang_tar * j) / 2;
				}
				else if (i == (y_space - 1) && j == (z_space - 1))
				{
					yy = rang_tar * i + (sz_tar[1] - 1 - rang_tar * i) / 2;
					zz = rang_tar * j + (sz_tar[2] - 1 - rang_tar * j) / 2;
				}
				Coord3D_PCM point;
				for (int b = 0; b < points.size(); b++)
				{
					int len = pow(pow(points[b].x - xx, 2) + pow(points[b].y - yy, 2) + pow(points[b].z - zz, 2), 0.5);
					if (len < min_len)
					{
						point = points[b];
						min_len = len;
					}
				}
				if (point.x != 0 || point.y != 0 || point.z != 0)
					tar_points.push_back(point);
			}
		}
	}
	unsigned int direction_size = sz_sub[2];
	vector<vector<Coord3D_PCM>> points_s(direction_size);
	for (int z = 0; z < sz_sub[2]; z++)
	{
		vector<vector<Point>> contours;
		Mat img2_xy = Mat(sz_sub[1], sz_sub[0], CV_8UC1);
		for (int y = 0; y < sz_sub[1]; y++)
			for (int x = 0; x < sz_sub[0]; x++)
			{
				img2_xy.at<uchar>(y, x) = (int)img_sub[z*sz_sub[1] * sz_sub[0] + y*sz_sub[0] + x];
			}
		Mat imageOtsu;
		medianBlur(img2_xy, img2_xy, 11);
		threshold(img2_xy, imageOtsu, 20, 255, CV_THRESH_OTSU);
		findContours(imageOtsu, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		Mat linePic = Mat::zeros(img2_xy.rows, img2_xy.cols, CV_8UC1);
		for (int index = 0; index < contours.size(); index++)
		{
			vector<Point> contour;
			contour = contours[index];
			//if (contourArea(contours[index]) > sz_sub[0] * (sz_sub[1] / 2))
			//	continue;
			//if (contourArea(contours[index]) < 2000)
			//	continue;
			for (int i = 0; i < contour.size(); i++)
			{
				Coord3D_PCM point(contour[i].x, contour[i].y, z);
				points_s[z].push_back(point);
				linePic.at<uchar>(contour[i].y, contour[i].x) = 255;
			}
		}
	}
	rang_sub = ceil(max(max(sz_sub[0], sz_sub[1]), sz_sub[2]) / 25);
	x_space = sz_sub[0] / rang_sub;
	y_space = sz_sub[1] / rang_sub;
	z_space = sz_sub[2] / rang_sub;
	for (int i = 0; i < x_space; i++)
	{
		for (int j = 0; j < y_space; j++)
		{
			for (int k = 0; k < z_space; k++)
			{
				vector<Coord3D_PCM> points;
				int cc;
				if (k != (x_space - 1))
				{
					cc = rang_sub * (k + 1);
				}
				else
					cc = sz_sub[2];
				for (int ind = rang_sub * k; ind < cc; ind++)
				{
					for (int a = 0; a < points_s[ind].size(); a++)
					{
						if (i != (x_space - 1) && j != (y_space - 1))
						{
							if (points_s[ind][a].x >= rang_sub*i && points_s[ind][a].x < rang_sub*(i + 1) &&
								points_s[ind][a].y >= rang_sub*j && points_s[ind][a].y < rang_sub*(j + 1))
								points.push_back(points_s[ind][a]);
						}
						else if (i != (x_space - 1) && j == (y_space - 1))
						{
							if (points_s[ind][a].x >= rang_sub*i && points_s[ind][a].x < rang_sub*(i + 1) &&
								points_s[ind][a].y >= rang_sub*j && points_s[ind][a].y < sz_sub[1])
								points.push_back(points_s[ind][a]);
						}
						else if (i == (x_space - 1) && j != (y_space - 1))
						{
							if (points_s[ind][a].x >= rang_sub*i && points_s[ind][a].x < sz_sub[0] &&
								points_s[ind][a].y >= rang_sub*j && points_s[ind][a].y < rang_sub*(j + 1))
								points.push_back(points_s[ind][a]);
						}
						else if (i == (x_space - 1) && j == (y_space - 1))
						{
							if (points_s[ind][a].x >= rang_sub*i && points_s[ind][a].x < sz_sub[0] &&
								points_s[ind][a].y >= rang_sub*j && points_s[ind][a].y < sz_sub[1])
								points.push_back(points_s[ind][a]);
						}
					}
				}
				int min_len = INT_MAX;
				int xx = rang_sub * k + (cc - rang_sub * k) / 2;
				int yy, zz;
				if (i != (x_space - 1) && j != (y_space - 1))
				{
					yy = rang_sub * i + (rang_sub * (i + 1) - rang_sub * i) / 2;
					zz = rang_sub * j + (rang_sub * (j + 1) - rang_sub * j) / 2;
				}
				else if (i != (x_space - 1) && j == (y_space - 1))
				{
					yy = rang_sub * i + (rang_sub * (i + 1) - rang_sub * i) / 2;
					zz = rang_sub * j + (sz_sub[1] - 1 - rang_sub * j) / 2;
				}
				else if (i == (x_space - 1) && j != (y_space - 1))
				{
					yy = rang_sub * i + (sz_sub[0] - 1 - rang_sub * i) / 2;
					zz = rang_sub * j + (rang_sub * (j + 1) - rang_sub * j) / 2;
				}
				else if (i == (x_space - 1) && j == (y_space - 1))
				{
					yy = rang_sub * i + (sz_sub[0] - 1 - rang_sub * i) / 2;
					zz = rang_sub * j + (sz_sub[1] - 1 - rang_sub * j) / 2;
				}
				Coord3D_PCM point;
				for (int b = 0; b < points.size(); b++)
				{
					int len = pow(pow(points[b].x - xx, 2) + pow(points[b].y - yy, 2) + pow(points[b].z - zz, 2), 0.5);
					if (len < min_len)
					{
						point = points[b];
						min_len = len;
					}
				}
				if (point.x != 0 || point.y != 0 || point.z != 0)
					sub_points.push_back(point);
			}
		}
	}
	return true;
}
