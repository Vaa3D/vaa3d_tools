#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
#include <sstream>
#include "omp.h"
#include "ctime"
using namespace std;
int dx[26] = { 1,1,1,-1,-1,-1,0,0,1,1,1,0,0,0,-1,-1,-1,1,1,1,0,0,0,-1,-1,-1};
int dy[26] = { 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
int dz[26] = { 1,0,-1,1,0,-1,1,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1 };
int vis[512][512][256];
int * getIndex(int i,int row,int col,int height) {
	int* presult = new int[3];
	presult[0] = i % row;
	presult[1] = i /row%col;
	presult[2] = i / row / col;
	return presult;
}

int getNum(int x, int y, int z) {

	int a;
	a = z*512*512+y*512+x;
	return a;
}
int *  ImageFilter(char * array, int sigma, int row, int col, int height) {//LoG filter
	cout << "step 1" << endl;
	int w = sigma * 3 * 2 + 1;
	double  *h = (double*)malloc(sizeof(double) * w * w * w);
	double  *r = (double*)malloc(sizeof(double) * w * w * w);
	double  *present= (double*)malloc(sizeof(double) * row * col * height);
	int *present1 = (int*)malloc(sizeof(int) * row * col * height);
	double sum = 0.0;
	int count = 0;
	int c = (w + 1) / 2;
	
	for(int i=0;i<w;i++){
		for(int j=0;j<w;j++){
			for (int k = 0; k < w; k++) {
				r[count] = pow(double(i - c), 2) + pow(double(j - c), 2) + pow(double(k - c), 2);
				h[count] = (3 - r[count] / (pow(double(sigma), 2)))*exp(-r[count] / (2 * pow(double(sigma), 2)));
				sum = sum + fabs(h[count]);
				count++;
			
			}
		}
	}

	cout << "step 2" << endl;
	
	for (int i = 0; i < w*w*w; i++)
	{
		h[i] =(fabs(h[i])) / sum;
		
	}
	
	
	
	//cout << "8389120" << array[8389120] <<endl;
	clock_t start,end;
	start=clock();
	//#pragma omp parallel num_threads(4)
	//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
	omp_set_num_threads(12);
	#pragma omp parallel for
	for (int i = 0; i < col*row*height; i++) {
		//cout << i << endl;
		//cout << i << endl;
		//cout << i << " " << omp_get_thread_num() << endl;
		int *test = getIndex(i,row,col,height);
		int x = test[0];
		int y = test[1];
		int z = test[2];
		int b = w / 2;
		int start = 0;
		if ((x - b) < 0 || (y - b) < 0 || (z - b) < 0 || (x + b) >=512 || (y + b) >=512 || (z + b) >=256)
			present[i] = (double)array[i];
		else {
			double aa = 0.0;
			for(int m=x-b;m<=x+b;m++)
				for(int n=y-b;n<=y+b;n++)
					for (int p = z - b; p <= z + b; p++) {
						//cout << x << " " << y << " " << z << endl;
						//cout << m << " " << n << " " << p << endl;
						//cout << getNum(m, n, p) << endl;
						//cout << array[getNum(m, n, p)] << endl;
						aa += (double)array[getNum(m, n, p)] * h[start];
						start++;
					}
			present[i] = aa;

		}
		
	}
	cout << "time: " << (clock()-start)/CLOCKS_PER_SEC <<endl; 
	cout << "step 2.2" << endl;
	cout << "step 3" << endl;
	for (int i = 0; i < row * col * height; i++)
	{
		if (present[i] > 255)
			present1[i] = 255;
		else if (present[i] < 0)
			present1[i] = 0;
		else present1[i] = (int)present[i];

	}
	cout << "step 4" << endl;
	for(int i=0;i<10;i++){
		cout << present[i] << endl;
	}

	return present1;
}



int * ImageThresh(int* array, int row, int col, int height) {//otsu thresholding
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
	cout<<"step 7"<<endl;
	int flag = 0;
	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
			for (int k = 0; k < height; k++) {
				if (array[getNum(i, j, k)] == 1 && !vis[i][j][k]) {
					flag++;
					dfs(array, row, col, height, i, j, k, flag);
				}
			}
	cout<<"step 8"<<endl;

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

int * ImageBin(char *array, int Threshold, int row, int col,int height)
{
	int *present = (int*)malloc(sizeof(int) * row * col * height);
	int *present1= (int*)malloc(sizeof(int) * row * col * height);
	int *present2 = (int*)malloc(sizeof(int) * row * col * height);
	int *present3 = (int*)malloc(sizeof(int) * row * col * height);
	int *imgBin = (int*)malloc(sizeof(int) * row * col * height);
	int *imgBin1 = (int*)malloc(sizeof(int) * row * col * height);
	for (int i = 0; i < row * col * height; i++)
	{
		if ((int)array[i] > 255)
			present[i] = 255;
		else if ((int)array[i] < 0)
			present[i] = 0;
		else present[i] = (int)array[i];

	}
	present1 = ImageFilter(array, 1, row, col, height);
	present2 = ImageFilter(array, 1, row, col, height);
	for (int i = 0; i < row * col * height; i++) {
		present3[i] = present[i] - present1[i] - present2[i];		
		present[i] = present[i] - present3[i];
	}
	imgBin = ImageThresh(present, row, col, height);
	for (int i = 0; i < row * col * height; i++) {
		if (imgBin > 0)
			imgBin[i] = 1;
	}
	cout<<"step 5"<<endl;
	imgBin1 = ImageVolEliminate(imgBin, row, col, height, Threshold);
	cout<<"step 6"<<endl;
	//imgBin = ImageFill(imgBin1, row, col, height);
	return imgBin1;

}
float getDistance(int x1, int y1, int z1, int x2, int y2, int z2) {
	int x =(x1 - x2)*(x1 - x2);
	int y =(y1 - y2)*(y1 - y2);
	int z = (z1 - z2)*(z1 - z2);
	float distance = sqrt(float(x + y + z));
	return distance;
}
int * bwdist(int *array, int row, int col, int height) {
	float *present = (float*)malloc(sizeof(float) * row * col * height);
	int *imgDist=(int*)malloc(sizeof(int) * row * col * height);
	for (int i = 0; i < row * col * height; i++) {
		if (array[i] == 1)
			present[i] = 0.0;
		else {
			float max = 65536.0;
			int *test = getIndex(i, row, col, height);

			for (int j = 0; j < row * col * height; j++) {
				if (array[j] == 1) {

					int *test2 = getIndex(j, row, col, height);
					float s = getDistance(test[0], test[1], test[2], test2[0], test2[1], test2[2]);
					if (s < max)
						max = s;
					if (j == i)
						continue;

				}

			}
			present[i] = max;
		}
	}
	for (int i = 0; i < row * col * height; i++) {
		if(present[i]>255)
			imgDist[i] = 255;
			else if (present[i] < 0)
			imgDist[i] = 0;
			else imgDist[i] = (int)present[i];
	}
	return imgDist;

}     
int *jitter(int * array, int row, int col,int height) {//局部极大值
	
	int *regionMax = (int*)malloc(sizeof(int)*row*col*height);
	
	for (int j = 0; j < row*col*height; j++) {
		int *test = getIndex(j, row, col,height);
		int max = array[j];
		int count = 0;
		for (int i = 0; i < 26; ++i) { // 枚举26个方向
			int nr = test[0] + dx[i];
			int nc = test[1] + dy[i];
			int rh = test[2] + dz[i];
			if (nr >= 0 && nr < row && nc >= 0 && nc < col&&rh >= 0 && rh < height) {
				if (array[getNum(nr, nc,rh)] > max)
					max = array[getNum(nr, nc,rh)];
				count++;

			}

		}
		if (count == 26 && max == array[j] && max != 0)
			regionMax[j] = 1;
		else
			regionMax[j] = 0;
	}

	return regionMax;
}


int * SomaLocate(int * array, int row, int col,int height, int jitterHeight) {
	int *imgDist = (int*)malloc(sizeof(int)*row*col*height);
	imgDist = jitter(array, row, col,height);
	int *regionMax = (int*)malloc(sizeof(int)*row*col*height);
	int *regionMax2 = (int*)malloc(sizeof(int)*row*col*height);
	for (int m = 0; m < row*col*height; m++) {
		regionMax2[m] = array[m];
	}
	for (int i = 0; i < jitterHeight; i++) {
		for (int j = 0; j < row*col*height; j++)
		{
			if (imgDist[j] == 1)
			{
				regionMax[j] =( --regionMax2[j]);
			}
			else regionMax[j] = regionMax2[j];
		}

		imgDist = jitter(regionMax, row, col,height);
	}

return imgDist;

}

int **RayBurstSampling(int * DTarray,int * RMarray,int row,int col,int height){
	int count=0;
	for(int i=0;i<row*col*height;i++){
		if(RMarray[i]==1)
			count++;
	}
	int num=0;
	int ** ReArray=new int *[(count+1)*258];
	for(int m=0;m<(count+1)*258;m++){
					ReArray[m]=new int[4];
				}
	for(int i=0;i<row*col*height;i++)
	{
	if(RMarray[i]==1){
	ifstream ifile;
	ifile.open("C:/Users/Ashley Chen/Desktop/258.txt");
	float a;
	float b;
	float c;
	float array[258][3];
	int m = 0;
	while (ifile >> a >> b >> c) {
    array[m][0] = a;
    array[m][1] = b;
	array[m][2] = c;
	m++;
	}
	int *test=getIndex(i,row,col,height);
	for(int j=0;j<258;j++){
		int flag1=1;
		int flag2=1;

		int flag3=1;
		if(array[j][0]<0)
			flag1=-1;
		if(array[j][1]<0)
			flag2=-1;
		if(array[j][2]<0)
			flag3=-1;
		int flag=0;
		float max=fabs(array[j][0]);
		if(max<fabs(array[j][1]))
		{
			flag=1;
			max=fabs(array[j][1]);
		}
		if(max<fabs(array[j][2])){
			flag=2;
		}
		
		if(flag==0){
			float var_x=flag1*1;
			float x=test[0]+var_x;
			float y=test[1]+var_x/array[j][0]*array[j][1];
			float z=test[2]+var_x/array[j][0]*array[j][2];
			int x1=int(x);
			int y1=int(y);
			int z1=int(z);
			int x_front=test[0];
			int y_front=test[1];
			int z_front=test[2];
			while(DTarray[getNum(x1,y1,z1)]<DTarray[getNum(x_front,y_front,z_front)]&&DTarray[getNum(x1,y1,z1)]!=0){
				x_front=x1;
			    y_front=y1;
			    z_front=z1;
				x=x+var_x;
				y=y+var_x/array[j][0]*array[j][1];
				z=z+var_x/array[j][0]*array[j][2];
				x1=int(x);
				y1=int(y);
				z1=int(z);

			}

				ReArray[j+258*num][0]=i;
				ReArray[j+258*num][1]=x1;
				ReArray[j+258*num][2]=y1;
				ReArray[j+258*num][3]=z1;

		}



				if(flag==1){
			float var_y=flag2*1;
			float x=test[0]+var_y/array[j][1]*array[j][0];
			float y=test[1]+ var_y;
			float z=test[2]+var_y/array[j][1]*array[j][2];
			int x1=int(x);
			int y1=int(y);
			int z1=int(z);
			int x_front=test[0];
			int y_front=test[1];
			int z_front=test[2];
			while(DTarray[getNum(x1,y1,z1)]<DTarray[getNum(x_front,y_front,z_front)]&&DTarray[getNum(x1,y1,z1)]!=0){
				x_front=x1;
			    y_front=y1;
			    z_front=z1;
				x=x+var_y/array[j][1]*array[j][0];
				y=y+var_y;
				z=z+var_y/array[j][1]*array[j][2];
				x1=int(x);
				y1=int(y);
				z1=int(z);

			}

				ReArray[j+258*num][0]=i;
				ReArray[j+258*num][1]=x1;
				ReArray[j+258*num][2]=y1;
				ReArray[j+258*num][3]=z1;

		}

		 	if(flag==2){
			float var_z=flag3*1;
			float x=test[0]+var_z/array[j][2]*array[j][0];
			float y=test[1]+ var_z/array[j][2]*array[j][1];
			float z=test[2]+var_z;
			int x1=int(x);
			int y1=int(y);
			int z1=int(z);
			int x_front=test[0];
			int y_front=test[1];
			int z_front=test[2];
			while(DTarray[getNum(x1,y1,z1)]<DTarray[getNum(x_front,y_front,z_front)]&&DTarray[getNum(x1,y1,z1)]!=0){
				x_front=x1;
			    y_front=y1;
			    z_front=z1;
				x=x+var_z/array[j][2]*array[j][0];
				y=y+ var_z/array[j][2]*array[j][1];
				z=z+var_z;
				x1=int(x);
				y1=int(y);
				z1=int(z);

			}

				ReArray[j+258*num][0]=i;
				ReArray[j+258*num][1]=x1;
				ReArray[j+258*num][2]=y1;
				ReArray[j+258*num][3]=z1;
 }

}

}
			num++;
		}
		
	return ReArray;
}
	