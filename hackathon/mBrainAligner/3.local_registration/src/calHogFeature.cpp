#include <stdio.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <string>
#include "calHogFeature.h"

using namespace std;

struct CellParam
{
	vector<float> bin;
};

struct BlockParam
{
	vector<float> bin;
};

bool CellFeature(float ***grad_value, float ***thetaImage, int z, int y, int x, int cell_size, int bin_num, vector<float>&cell_bin_feature)
{
	int bin_size = 180 / bin_num;
	cell_bin_feature.clear();
	cell_bin_feature.resize(bin_num, 0);
	int binindex = 0;
	int theta = 0;
	float gw = 0;
	float ww = 0, wn = 0, wp = 0;
	for (int i = z; i < z + cell_size; i++){
		for (int j = y; j < y + cell_size; j++){
			for (int k = x; k < x + cell_size; k++){
				theta = (int)thetaImage[i][j][k];
				//cout << "theta:" << theta << endl;
				// 计算权重梯度，一次双线性插值
				ww = theta % bin_size;   //theta % 20
				//cout << "ww:"<< ww << endl;
				if (ww >= bin_size / 2) {
					wn = ww - bin_size / 2;
					wp = (bin_size - wn) / bin_size;
				}
				else {
					wn = bin_size / 2 - ww;
					wp = (bin_size - wn) / bin_size;
				}
				binindex = theta / bin_size;
				if (binindex >= 9) {
					binindex = 8;
				}
				//cout << wp << endl;
				/*权重梯度值累加,把0-180分为9个区间，每个区间bin=20，当某个角度值落在某个区间时，又把每个区间分成两部分，
				当这个值落在区间中心后半部分不包括中心点时，会把这个点的梯度值分成两部分放到当前区间和后一个区间，
				这个权重大小跟这个点到当前区间中心点的距离有关，越近，在当前区间的幅值比重越大，*/
				gw = grad_value[i][j][k];
				//cout << gw << endl;
				if (ww >= bin_size / 2)
				{
					cell_bin_feature[binindex] += (wp*gw);
					if (isnan(cell_bin_feature[binindex]))
					{
						printf("ERROR: Nan value appears!");
						return false;
					}
					if (binindex < 8) {
						cell_bin_feature[binindex + 1] += ((1.0 - wp)*gw);
						if (isnan(cell_bin_feature[binindex + 1]))
						{
							printf("ERROR: Nan value appears!");
							return false;
						}
					}
					if (binindex == 8)
					{
						cell_bin_feature[0] += ((1.0 - wp)*gw);
						if (isnan(cell_bin_feature[0]))
						{
							printf("ERROR: Nan value appears!");
							return false;
						}
					}
				}
				else
				{
					cell_bin_feature[binindex] += (wp*gw);
					if (isnan(cell_bin_feature[binindex]))
					{
						printf("ERROR: Nan value appears!");
						return false;
					}
					if (binindex > 0) {
						cell_bin_feature[binindex - 1] += ((1.0 - wp)*gw);
						if (isnan(cell_bin_feature[binindex - 1]))
						{
							printf("ERROR: Nan value appears!");
							return false;
						}
					}
					if (binindex == 0)
					{
						cell_bin_feature[8] += ((1.0 - wp)*gw);
						if (isnan(cell_bin_feature[8]))
						{
							printf("ERROR: Nan value appears!");
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}

float sumOfSquares(float accumulator, float currentValue)
{
	return accumulator + currentValue * currentValue;
}

bool nomolize(vector<float> &data)
{
	float dnormV;
	float normV = accumulate(data.begin(), data.end(), 0.0, sumOfSquares);    //sumOfSquares()自定义类型的数据处理,求平方和
	dnormV = 1 / sqrtf(normV);
	for (int m = 0; m < data.size(); m++)
	{
		data[m] = data[m] * dnormV;
		if (isnan(data[m]))
		{
			printf("ERROR: Nan value appears!");
			return false;
			break;
		}
	}
	return true;
}


bool BlockFeature(int i, int j, int k, int block_size, CellParam ***cells, vector<float>&block_bin_feature)
{
	block_bin_feature.clear();
	for (int z = i; z < i + block_size; z++)
		for (int y = j; y < j + block_size; y++)
			for (int x = k; x < k + block_size; x++)
				block_bin_feature.insert(block_bin_feature.end(), cells[z][y][x].bin.begin(), cells[z][y][x].bin.end());
	return true;
}

bool calHogFeature(float *patch, int patch_r, int cell_size, int bin_num, int block_size, vector<float>&HogFeature)
{
	HogFeature.clear();
	int cell_w = patch_r / cell_size;
	int block_w = cell_w - block_size + 1;
	CellParam ***cells;
	BlockParam ***blocks;
	cells = new CellParam**[cell_w];
	for (int z = 0; z < cell_w; z++)
	{
		cells[z] = new CellParam*[cell_w];
		for (int y = 0; y < cell_w; y++)
		{
			cells[z][y] = new CellParam[cell_w];
		}
	}
	blocks = new BlockParam**[block_w];
	for (int z = 0; z < block_w; z++)
	{
		blocks[z] = new BlockParam*[block_w];
		for (int y = 0; y < block_w; y++)
		{
			blocks[z][y] = new BlockParam[block_w];
		}
	}
	//printf("%d\n", cell_w);
	float ***patch_image;
	float ***grad_x, ***grad_y, ***grad_z;
	float ***grad_value, ***thetaImage;
	patch_image = new float**[patch_r];
	for (int z = 0; z < patch_r; z++)
	{
		patch_image[z] = new float*[patch_r];
		for (int y = 0; y < patch_r; y++)
		{
			patch_image[z][y] = new float[patch_r];
		}
	}
	grad_x = new float**[patch_r];
	for (int z = 0; z < patch_r; z++)
	{
		grad_x[z] = new float*[patch_r];
		for (int y = 0; y < patch_r; y++)
		{
			grad_x[z][y] = new float[patch_r];
		}
	}
	grad_y = new float**[patch_r];
	for (int z = 0; z < patch_r; z++)
	{
		grad_y[z] = new float*[patch_r];
		for (int y = 0; y < patch_r; y++)
		{
			grad_y[z][y] = new float[patch_r];
		}
	}
	grad_z = new float**[patch_r];
	for (int z = 0; z < patch_r; z++)
	{
		grad_z[z] = new float*[patch_r];
		for (int y = 0; y < patch_r; y++)
		{
			grad_z[z][y] = new float[patch_r];
		}
	}
	for (int z = 0; z < patch_r; z++)
		for (int y = 0; y < patch_r; y++)
			for (int x = 0; x < patch_r; x++)
				patch_image[z][y][x] = patch[z*patch_r*patch_r + y*patch_r + x];
	//-----------计算每个像素的梯度幅值和方向---------------------
	for (int i = 0; i < patch_r; i++)
		for (int j = 0; j < patch_r; j++)
			for (int k = 0; k < patch_r; k++)
			{
				int i1 = i - 1 < 0 ? i : i - 1;
				int i2 = i + 1 >= patch_r ? patch_r - 1 : i + 1;
				int j1 = j - 1 < 0 ? j : j - 1;
				int j2 = j + 1 >= patch_r ? patch_r - 1 : j + 1;
				int k1 = k - 1 < 0 ? k : k - 1;
				int k2 = k + 1 >= patch_r ? patch_r - 1 : k + 1;
				grad_z[i][j][k] = (patch_image[i2][j][k] - patch_image[i1][j][k]) / 2;
				grad_x[i][j][k] = (patch_image[i][j][k2] - patch_image[i][j][k1]) / 2;
				grad_y[i][j][k] = (patch_image[i][j2][k] - patch_image[i][j1][k]) / 2;
			}
	//printf("x0=%f,x=%f\n", grad_x[0][0][0], grad_x[(patch_r - 1) / 2][(patch_r - 1) / 2][(patch_r - 1) / 2]);
	//printf("y0=%f,y=%f\n", grad_y[0][0][0], grad_y[(patch_r - 1) / 2][(patch_r - 1) / 2][(patch_r - 1) / 2]);
	//printf("z0=%f,z=%f\n", grad_z[0][0][0], grad_z[(patch_r - 1) / 2][(patch_r - 1) / 2][(patch_r - 1) / 2]);
	//计算梯度幅值
	{
		grad_value = new float**[patch_r];
		for (int z = 0; z < patch_r; z++)
		{
			grad_value[z] = new float*[patch_r];
			for (int y = 0; y < patch_r; y++)
			{
				grad_value[z][y] = new float[patch_r];
			}
		}
		for (int z = 0; z < patch_r; z++)
			for (int y = 0; y < patch_r; y++)
				for (int x = 0; x < patch_r; x++)
				{
					grad_value[z][y][x] = sqrt((float)(grad_x[z][y][x] * grad_x[z][y][x] + grad_y[z][y][x] * grad_y[z][y][x] + grad_z[z][y][x] * grad_z[z][y][x]));
					//cout << grad_value[z][y][x] << endl;
				}
	}

	//计算角度
	{
		thetaImage = new float**[patch_r];
		for (int z = 0; z < patch_r; z++)
		{
			thetaImage[z] = new float*[patch_r];
			for (int y = 0; y < patch_r; y++)
			{
				thetaImage[z][y] = new float[patch_r];
			}
		}
		//把角度范围变换到【0,180】
		float theta = 90.0f;
		for (int z = 0; z < patch_r; z++)
			for (int y = 0; y < patch_r; y++)
				for (int x = 0; x < patch_r; x++)
				{
					if (grad_x[z][y][x] == 0.0f && grad_y[z][y][x] == 0.0f)
					{
						if (grad_z[z][y][x] > 0)
							theta = 90.0f;
						else if (grad_z[z][y][x] < 0)
							theta = -90.0;
						else
							theta = 0.0f;
					}
					else if (grad_z[z][y][x] == 0.0f)
					{
						if (grad_x[z][y][x] > 0 && grad_y[z][y][x] > 0)
							theta = 0.0f;
						else
							theta = 180.0f;
					}
					else
					{
						theta = atan(grad_z[z][y][x] / sqrt((float)(grad_x[z][y][x] * grad_x[z][y][x] + grad_y[z][y][x] * grad_y[z][y][x]))) * 180 / 3.14;
					}
					if (theta < 0)
						theta += 180.0f;
					thetaImage[z][y][x] = theta;
					//cout << thetaImage[z][y][x] << endl;
				}
	}
	for (int z = 0; z < patch_r; z++)
	{
		for (int y = 0; y < patch_r; y++)
		{
			delete[] patch_image[z][y];
		}
		delete[] patch_image[z];
	}
	delete[] patch_image; patch_image = 0;

	for (int z = 0; z < patch_r; z++)
	{
		for (int y = 0; y < patch_r; y++)
		{
			delete[] grad_x[z][y];
		}
		delete[] grad_x[z];
	}
	delete[] grad_x; grad_x = 0;

	for (int z = 0; z < patch_r; z++)
	{
		for (int y = 0; y < patch_r; y++)
		{
			delete[] grad_y[z][y];
		}
		delete[] grad_y[z];
	}
	delete[] grad_y; grad_y = 0;

	for (int z = 0; z < patch_r; z++)
	{
		for (int y = 0; y < patch_r; y++)
		{
			delete[] grad_z[z][y];
		}
		delete[] grad_z[z];
	}
	delete[] grad_z; grad_z = 0;
	//划分cell，然后计算每个cell的特征

	for (int i = 0; i < patch_r; i += cell_size)
		for (int j = 0; j < patch_r; j += cell_size)
			for (int k = 0; k < patch_r; k += cell_size)
			{
				if (i + cell_size > patch_r || j + cell_size > patch_r || k + cell_size > patch_r)
					continue;
				int hid = i / cell_size;
				int wid = j / cell_size;
				int cid = k / cell_size;
				CellFeature(grad_value, thetaImage, i, j, k, cell_size, bin_num, cells[hid][wid][cid].bin);
				//for (int r = 0; r < cells[hid][wid][cid].bin.size(); r++)
				//	printf("%f\n", cells[hid][wid][cid].bin[r]);
			}
	for (int z = 0; z < patch_r; z++)
	{
		for (int y = 0; y < patch_r; y++)
		{
			delete[] grad_value[z][y];
		}
		delete[] grad_value[z];
	}
	delete[] grad_value; grad_value = 0;

	for (int z = 0; z < patch_r; z++)
	{
		for (int y = 0; y < patch_r; y++)
		{
			delete[] thetaImage[z][y];
		}
		delete[] thetaImage[z];
	}
	delete[] thetaImage; thetaImage = 0;

	//计算每个block的特征
	for (int i = 0; i < block_w; i++)
	{
		for (int j = 0; j < block_w; j++)
		{
			for (int k = 0; k < block_w; k++)
			{
				BlockFeature(i, j, k, block_size, cells, blocks[i][j][k].bin);

				//归一化
				bool is = false;
				for (int r = 0; r < blocks[i][j][k].bin.size(); r++)
				{
					if (blocks[i][j][k].bin[r] != 0.0)
					{
						is = true;
						break;
					}
				}
				if (is)
				{
					if (!nomolize(blocks[i][j][k].bin))
						return false;
				}

				//cout << blocks[i][j][k].bin.size() << endl;
				HogFeature.insert(HogFeature.end(), blocks[i][j][k].bin.begin(), blocks[i][j][k].bin.end());

			}
		}
	}
	//out.close();
	for (int z = 0; z < cell_w; z++)
	{
		for (int y = 0; y < cell_w; y++)
		{
			delete[] cells[z][y];
		}
		delete[] cells[z];
	}
	delete[] cells;

	for (int z = 0; z < block_w; z++)
	{
		for (int y = 0; y < block_w; y++)
		{
			delete[] blocks[z][y];
		}
		delete[] blocks[z];
	}
	delete[] blocks;
	return true;
}