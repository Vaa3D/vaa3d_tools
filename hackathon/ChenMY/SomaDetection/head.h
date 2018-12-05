#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
using namespace std;
int dx[26] = { 1,1,1,-1,-1,-1,0,0,1,1,1,0,0,0,-1,-1,-1,1,1,1,0,0,0,-1,-1,-1};
int dy[26] = { 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
int dz[26] = { 1,0,-1,1,0,-1,1,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1 };
int vis[256][256][32];
int * getIndex(int i,int row,int col,int height) {
	int* presult = new int[3];
	presult[0] = i % row;
	presult[1] = i /row%col;
	presult[2] = i / row / col;
	return presult;
}

int getNum(int x, int y, int z) {

	int a;
	a = x * 512 * 32 + y * 32 + z;
	return a;
}
int *  ImageFilter(double * array, int sigma, int row, int rol, int height) {
	int w = sigma * 3 * 2 + 1;
	double  *h = (double*)malloc(sizeof(double) * w * w * w);
	double  *r = (double*)malloc(sizeof(double) * w * w * w);
	double  *present= (double*)malloc(sizeof(double) * row * rol * height);
	int *present1 = (int*)malloc(sizeof(int) * row * rol * height);
	double sum = 0.0;
	int count = 0;
	int c = (w + 1) / 2;
	
	for(int i=0;i<w;i++)
		for(int j=0;j<w;j++)
			for (int k = 0; k < w; k++) {
				r[count] = pow((i - c), 2) + pow((j - c), 2) + pow((k - c), 2);
				h[count] = (3 - r[count] / (pow(sigma, 2)))*exp(-r[count] / (2 * pow(sigma, 2)));
				
				sum = sum + fabs(h[count]);
				
				count++;
			
			}
	
	for (int i = 0; i < w*w*w; i++)
	{
		h[i] =(fabs(h[i])) / sum;
		
	}
	for (int i = 0; i < row*rol*height; i++) {
		int *test = getIndex(i,512,512,32);
		int x = test[0];
		int y = test[1];
		int z = test[2];
		int b = w / 2;
		int start = 0;
		if ((x - b) < 0 || (y - b) < 0 || (z - b) < 0 || (x + b) > 512 || (y + b) > 512 || (z + b) > 32)
			present[i] = array[i];
		else {
			double aa = 0.0;
			for(int m=x-b;m<x+b;m++)
				for(int n=y-b;n<y+b;n++)
					for (int p = z - b; p < z + b; p++) {
						aa += array[getNum(m, n, p)] * h[start];
						start++;
					}
			present[i] = aa;

		}
		
			
	}

	for (int i = 0; i < row * rol * height; i++)
	{
		if (present[i] > 255)
			present1[i] = 255;
		else if (present[i] < 0)
			present1[i] = 0;
		else present1[i] = (int)present[i];

	}

	return present1;
}

int * ImageThresh(int* array, int row, int col, int height) {
	int pixelSum = row * col*height;
	int *present = (int*)malloc(sizeof(int) * row * col * height);
	int pixelCount[256];
	int threshold = 0;
	float ip1, ip2, is1, is2, w0, w1, mean1, mean2,mean, deltaTmp, deltaMax;
	for (int i = 0; i < 256; i++) {
		pixelCount[i] = 0;
	}
	for (int i = 0; i < pixelSum; i++)
		pixelCount[array[i]]++;
	deltaMax = 0;
	for (int i = 0; i < 256; i++) {
		ip1 = ip2 = is1 = is2 = w0 = w1 = mean1 = mean2 = deltaTmp = mean = 0;
		for (int j = 0; j < 256; j++) {
			if (j <= i) {
				ip1 += pixelCount[j] * j;
				is1 += pixelCount[j];
			}
			else {
				ip2 += pixelCount[j] * j;
				is2 += pixelCount[j];
			}
		}
		mean1 = ip1 / is1;
		mean2 = ip2 / is2;
		w0 = (float)is1 / (float)pixelSum;
		w1 = 1 - w0;
		mean = mean1 * w0 + mean2 * w1;
		deltaTmp = w0 * w1*pow((mean1, mean2), 2);
		if (deltaTmp > deltaMax) {
			deltaMax = deltaTmp;
			threshold = i;
		}

	}
	for (int i = 0; i < pixelSum; i++) {
		if (array[i] < threshold) {
			present[i] = 0;
		}
		else
			present[i] = array[i];
	}
	
	return present;
}
void dfs( int * array,int row, int col, int height,int x,int y,int z,int count) {
	vis[x][y][z] = count;
	for (int i = 0; i < 26; ++i) { // 枚举26个方向
		int nr = x+ dx[i];
		int nc = y + dy[i];
		int rh = z + dz[i];
		if (nr >= 0 && nr < row && nc >= 0 && nc < col&&rh >= 0 && rh < height && !vis[nr][nc][rh] && array[getNum(nr, nc, rh)] == 1) { // 判断是否出界，是否访问过，是否为1
			dfs(array, row,  col, height, nr, nc, rh ,count);
		}
	}

}



int * ImageVolEliminate(int * array, int row, int col, int height, int threshold) {
	int *present = (int*)malloc(sizeof(int) * row * col * height);
	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
			for (int k = 0; k < height; k++)
				vis[i][j][k] = 0;
	int flag = 0;
	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
			for (int k = 0; k < height; k++) {
				if (array[getNum(i, j, k)] == 1 && !vis[i][j][k]) {
					flag++;
					dfs(array, row, col, height, i, j, k, flag);
				}
			}

	int*Count = (int*)malloc(sizeof(int) * flag);
	for (int m = 1; m <= flag; m++)
		Count[m] = 0;
	int count = 0;
	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
			for (int k = 0; k < height; k++) {
				for (int m = 1; m <= flag; m++)
					if (vis[i][j][k] == m)
						Count[m]++;
			}
	for (int i = 1; i <= flag; i++) {
		if (Count[i] < threshold) {
			for (int p = 0; p < row; p++)
				for (int j = 0; j < col; j++)
					for (int k = 0; k < height; k++) {
						if (vis[p][j][k] == i)
							present[getNum(p, j, k)] = 0;
						else present[getNum(p, j, k)] = array[getNum(p, j, k)];
					}
		}

	}
	return present;

}
int * ImageFill(int * array, int row, int col, int height) {
	int *present = (int*)malloc(sizeof(int) * row * col * height);
	int  num[3];
	for (int i = 0; i < row * col * height; i++) {
		int count = 0;
		if (array[i] == 0) {
			int *test = getIndex(i,row, col, height);
		
			for (int t = 0; t < 26; ++t) {
				int nr = test[0] + dx[t];
				int nc = test[1] + dy[t];
				int rh = test[2] + dz[t];
				if (nr >= 0 && nr < row && nc >= 0 && nc < col&&rh >= 0 && rh < height &&array[getNum(nr, nc, rh)] == 1)
					count++;
			}
			
		}
		if (count == 26) present[i] = 1;
		else present[i] = array[i];

	}
	return present;

}

void ImageBin(double *array, int Threshold, int row, int col,int height)
{
	int *present = (int*)malloc(sizeof(int) * row * col * height);
	int *present1= (int*)malloc(sizeof(int) * row * col * height);
	int *present2 = (int*)malloc(sizeof(int) * row * col * height);
	int *present3 = (int*)malloc(sizeof(int) * row * col * height);
	int *imgBin = (int*)malloc(sizeof(int) * row * col * height);
	int *imgBin1 = (int*)malloc(sizeof(int) * row * col * height);
	for (int i = 0; i < row * col * height; i++)
	{
		if (present[i] > 255)
			present[i] = 255;
		else if (present[i] < 0)
			present[i] = 0;
		else present[i] = (int)present[i];

	}
	present1 = ImageFilter(array, 3, 512, 512, 32);
	present2 = ImageFilter(array, 4, 512, 512, 32);
	for (int i = 0; i < row * col * height; i++) {
		present3[i] = present[i] - present1[i] - present2[i];		
		present[i] = present[i] - present3[i];
	}
	imgBin = ImageThresh(present, row, col, height);
	for (int i = 0; i < row * col * height; i++) {
		if (imgBin > 0)
			imgBin[i] = 1;
	}

	imgBin1 = ImageVolEliminate(imgBin, row, col, height, Threshold);
	imgBin = ImageFill(imgBin1, row, col, height);

}

