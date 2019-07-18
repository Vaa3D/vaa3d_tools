#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
#include <sstream>
#include "omp.h"
#include "ctime"
#include <queue>
#include <iomanip>
#include<vector>
#define PI 3.1415926

using namespace std;
int dx[26] = { 1,1,1,-1,-1,-1,0,0,1,1,1,0,0,0,-1,-1,-1,1,1,1,0,0,0,-1,-1,-1};
int dy[26] = { 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
int dz[26] = { 1,0,-1,1,0,-1,1,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1 };

//struct tag{
//	int index;
//	unsigned int add[258][3];
//};
void getIndex(int i,int row,int col,int height,int array[]) {
	array[0] = i % row;
	array[1] = i /row%col;
	array[2] = i / row / col;
}

int getNum(int x, int y, int z,int row,int col,int height) {
	int a;
	a = z*row*col+y*row+x;
	return a;
}
int *  ImageFilter(char * array, int sigma, int row, int col, int height) {//LoG filter
	cout << "step 1" << endl;
	int w = sigma * 3 * 2 + 1;
	double  *h ;
	double  r ;
	double  *present;
	int *present1;
	try{
		h=(double*)malloc(sizeof(double) * w * w * w);
		present= (double*)malloc(sizeof(double) * row * col * height);
		present1 = (int*)malloc(sizeof(int) * row * col * height);
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		return NULL;
	}
	double sum = 0.0;
	int count = 0;
	int c = (w + 1) / 2;
	for(int i=0;i<w;i++){
		for(int j=0;j<w;j++){
			for (int k = 0; k < w; k++) {
				r = pow(double(i - c), 2) + pow(double(j - c), 2) + pow(double(k - c), 2);
				h[count] = (3 - r / (pow(double(sigma), 2)))*exp(-r / (2 * pow(double(sigma), 2)));
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
	clock_t start,end;
	start=clock();
	//#pragma omp parallel num_threads(4)
	//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
	omp_set_num_threads(8);
	//#pragma omp parallel for
	int test[3];
	for (int i = 0; i < col*row*height; i++) {
		//cout << i << endl;
		//cout << i << endl;
		//cout << i << " " << omp_get_thread_num() << endl;
		getIndex(i,row,col,height,test);
		int x = test[0];
		int y = test[1];
		int z = test[2];
		int b = w / 2;
		int start = 0;
		if ((x - b) < 0 || (y - b) < 0 || (z - b) < 0 || (x + b) >=row || (y + b) >=col || (z + b) >=height)
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
						aa += (double)array[getNum(m, n, p,row,col,height)] * h[start];
						start++;
					}
					present[i] = aa;
		}		
	}
	cout << "time: " << (clock()-start)/CLOCKS_PER_SEC <<endl; 
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
	delete present;
	delete h;
	return present1;
}



unsigned char * ImageThresh(int* array, int row, int col, int height) {//otsu thresholding
	int pixelSum = row * col*height;
	unsigned char *present;
	try{
		present = (unsigned char *)malloc(sizeof(unsigned char) * row * col * height);
	}catch(...){
		cout << __FUNCTION__ << ": " << "error mallco" << endl; 
		return NULL;
	}
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
	//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
	cout<<"threshold"<<threshold<<endl;
	for (int i = 0; i < pixelSum; i++) {
		if (array[i] < threshold) {
			present[i] = 0;
		}
		else
			present[i] = 1;
	}

	return present;
}


void bfs( unsigned char * array,int row, int col, int height,int x,int y,int z,int count,int *vis) {
	vis[getNum(x,y,z,row,col,height)]= count;
	queue <int>q;
	q.push(getNum(x, y,z,row,col,height)); 
	int test[3];
	while(!q.empty()){
		int front=q.front();
		getIndex(front,row,col,height,test);
		q.pop();
		for (int i = 0; i < 26; i++) { // 枚举26个方向
			int nr = test[0]+ dx[i];
			int nc = test[1]+ dy[i];
			int rh =test[2]+ dz[i];
			if (nr >= 0 && nr < row && nc >= 0 && nc < col && rh >= 0 && rh < height && !vis[getNum(nr,nc,rh,row,col,height)] && array[getNum(nr, nc, rh,row,col,height)] == 1) { // 判断是否出界，是否访问过，是否为1
				vis[getNum(nr,nc,rh,row,col,height)]=count;
				q.push(getNum(nr,nc,rh,row,col,height));	
			}
		}
	}
}


unsigned char * ImageVolEliminate(unsigned char * array, int row, int col, int height, int threshold,int * vis) {
	cout<<"vol 1"<<endl;
	unsigned char *present;
	try{
		present = (unsigned char*)malloc(sizeof(unsigned char) * row * col * height);
	
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		return NULL;
	}
	cout<<"step 5.1"<<endl;
	int flag = 0;
	for (int i = 0; i < row; i++)
		for (int j = 0; j < col; j++)
			for (int k = 0; k < height; k++) {
				if (array[getNum(i, j, k,row,col,height)] == 1 && !vis[getNum(i,j,k,row,col,height)]) {
					flag++;
					bfs(array, row,  col, height, i, j, k ,flag,vis);
				}
			}
			cout<<"step 9"<<endl;
			cout<<"flag"<<flag<<endl;
			int *Count;
			try{
				Count = (int*)malloc(sizeof(int) * (flag+1));
			}catch(...){
				cout << __FUNCTION__ << " error malloc" << endl;
				return NULL;
			}
			for (int m = 0; m <= flag; m++)
				Count[m] = 0;

			cout<<"step 10"<<endl;
			//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
			//omp_set_num_threads(12);
			//#pragma omp parallel for
			int test[3];
			int test1[3];
			for(int i=0;i<row*col*height;i++ ){
				getIndex(i,row,col,height,test);
				Count[vis[getNum(test[0],test[1],test[2],row,col,height)]]++;
			}
			//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
			for(int i=0;i<row*col*height;i++){
				getIndex(i,row,col,height,test1);
				if(Count[vis[getNum(test[0],test[1],test[2],row,col,height)]] < threshold){
					present[i] = 0;
				}
				else{
					present[i] = array[i];			
				}		
			}

			delete Count;
			return present;

}
unsigned char * ImageFill(unsigned char * array, int row, int col, int height) {
	unsigned char *present ;
	try{
		present = (unsigned char*)malloc(sizeof(unsigned char) * row * col * height);
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		return NULL;
	}
	int  num[3];
	cout<<"step 7"<<endl;
	int test[3];
	//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
	cout<<"start print fill"<<endl;
	for (int i = 0; i < row * col * height; i++) {
		int count = 0;
		if (array[i] == 0) {
			getIndex(i,row, col, height,test);				
			for (int t = 0; t < 26; ++t) {
				int nr = test[0] + dx[t];
				int nc = test[1] + dy[t];
				int rh = test[2] + dz[t];
				if (nr >= 0 && nr < row && nc >= 0 && nc < col&&rh >= 0 && rh < height &&array[getNum(nr, nc, rh,row,col,height)] == 1)
					count++;
			}
		}
		if (count == 26) present[i] = 1;
		else present[i] = array[i];	
	}

	return present;

}

unsigned char * ImageBin(char *array, int Threshold, int row, int col,int height,int *vis)
{
	int *present;
	try{
		present = (int*)malloc(sizeof(int) * row * col * height);
	}catch(...){
		cout << __FUNCTION__<< " error malloc" << endl;
		return NULL;
	}
	int *present1;
	int *present2;
	unsigned char *imgBin;
	unsigned char *imgBin1;
	for (int i = 0; i < row * col * height; i++)
	{
		if ((int)array[i] > 255)
			present[i] = 255;
		else if ((int)array[i] < 0)
			present[i] = 0;
		else present[i] = (int)array[i];                                    

	}
	present1 = ImageFilter(array, 3, row, col, height);
	present2 = ImageFilter(array, 4, row, col, height);
	if(present1==NULL||present2==NULL){
		return NULL;
	}

	for (int i = 0; i < row * col * height; i++) {
		present[i] = present1[i] + present2[i];

	}

	delete present1;
	delete present2;
	imgBin = ImageThresh(present, row, col, height);
	delete present;
	if(imgBin==NULL){
		return NULL;
	}
	cout<<"start imgVol"<<endl;
	imgBin1 = ImageVolEliminate(imgBin, row, col, height, Threshold,vis);
	if(imgBin1==NULL){
	return NULL;
	}
	cout<<"start imgFill"<<endl;
	imgBin = ImageFill(imgBin1, row, col, height);
	if(imgBin==NULL){
	return NULL;
	}
	delete imgBin1;
	cout<<"present end"<<endl;
	for(int i = 0; i < row * col * height; i++){
		if(imgBin[i]==1)
         imgBin[i]=255;
	}
	return imgBin;

}
//float getDistance(int x1, int y1, int z1, int x2, int y2, int z2) {
//	int x =(x1 - x2)*(x1 - x2);
//	int y =(y1 - y2)*(y1 - y2);
//	int z = (z1 - z2)*(z1 - z2);
//	float distance = sqrt(float(x + y + z));
//	return distance;
//}
//int * bwdist(int *array, int row, int col, int height) {
//		float *present ;
//		int *imgDist;
//		try{
//			present = (float*)malloc(sizeof(float) * row * col * height);
//			imgDist=(int*)malloc(sizeof(int) * row * col * height);
//		}catch(...){
//			cout << __FUNCTION__ << " error malloc" << endl;
//			return NULL;
//		}
//		int num;
//		for (int i = 0; i < row * col * height; i++){
//			if(array[i]==1){
//				num=i;
//				break;
//			}
//		}
//		int index[3];
//		int test[3];
//		int test2[3];
//		float max ;
//	     getIndex(num,row,col,height,index);
//		//freopen("D:/vaa3d_tools/hackathon/ChenMY/somaDetection/log.txt","w",stdout);
//		for (int i = 0; i < row * col * height; i++) {
//		if (array[i] == 1)
//			present[i] = 0.0;
//		else {
//			
//			getIndex(i, row, col, height,test);
//			max=getDistance(index[0], index[1], index[2], test[0], test[1], test[2]);
//			for (int j = 0; j < row * col * height; j++) {
//				if (array[j] == 1) {
//					if (j == i)
//						continue;
//					 getIndex(j, row, col, height,test2);
//					float s = getDistance(test[0], test[1], test[2], test2[0], test2[1], test2[2]);
//					if (s < max)
//						max = s;
//				}
//
//			}
//			present[i] = max;
//		
//		}
//		//cout<<present[i]<<endl;
//		}
//			
//		for (int i = 0; i < row * col * height; i++) {
//			if(present[i]>255)
//				imgDist[i] = 255;
//				else if (present[i] < 0)
//				imgDist[i] = 0;
//				else imgDist[i] = (int)present[i];			
//		}
//		return imgDist;
//
//	}     


unsigned char *jitter(unsigned char * array, int row, int col,int height) {//局部极大值
	unsigned char *regionMax;
	try{
		regionMax= (unsigned char*)malloc(sizeof(unsigned char)*row*col*height);
	}
	catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		return NULL;
	}
	int test[3];
	for (int j = 0; j < row*col*height; j++) {
		getIndex(j, row, col,height,test);
		unsigned char max = array[j];
		int count = 0;
		for (int i = 0; i < 26; ++i) { // 枚举26个方向
			int nr = test[0] + dx[i];
			int nc = test[1] + dy[i];
			int rh = test[2] + dz[i];
			if (nr >= 0 && nr < row && nc >= 0 && nc < col&&rh >= 0 && rh < height) {
				if (array[getNum(nr, nc,rh,row,col,height)] > max)
					max = array[getNum(nr, nc,rh,row,col,height)];
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


unsigned char * SomaLocate(unsigned char * DTarray, int row, int col,int height, int jitterHeight) {
	unsigned char *imgDist;
	unsigned char *DTSample;
	try{
		imgDist = (unsigned char*)malloc(sizeof(unsigned char) * row * col * height);
		DTSample=(unsigned char*)malloc(sizeof(unsigned char) * row * col * height);
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		return NULL;
	}
	for(int i=0;i<row*col*height; i++){
		DTSample[i]=DTarray[i];
	}
	imgDist = jitter(DTSample, row, col,height);
	if(imgDist==NULL){
		return NULL;
	}
	for(int j=0;j<jitterHeight;j++){
		for(int i=0;i<row*col*height; i++){
			if(imgDist[i]==1){
				if(DTSample[i]>1)
					DTSample[i]--;	   
			}
		}
		imgDist=jitter(DTSample, row, col,height);
	}
     delete DTSample;
	return imgDist;
}



int getMax(int X[]){
	int max=X[0];
	for(int i=0;i<258;i++){
		if(max<X[i])
			max=X[i];
	}
	return max;
}

int getMin(int X[]){
	int min=X[0];
	for(int i=0;i<258;i++){
		if(min>X[i])
			min=X[i];
	}
	return min;
}

double getA(double arc[][4],int n){//计算|A|
	if(n==1){
		return arc[0][0];
	}
	double ans=0;
	double temp[4][4]={0.0};
	for (int i=0;i<n;i++){
		for(int j=0;j<n-1;j++){
			for(int k=0;k<n-1;k++){
				temp[j][k]=arc[j+1][(k>=i)?k+1:k];
			}
		}
		double t=getA(temp,n-1);
		if(i%2==0){
			ans+=arc[0][i]*t;
		}
		else{
			ans-=arc[0][i]*t;
		}
	}
	return ans;
}


void getAstart(double arc[4][4],int n,double res[4][4])//求伴随矩阵
{
	if(n==1){
		res[0][0]=1;
		return;
	}
	double temp[4][4];
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			for(int k=0;k<n-1;k++){
				for(int t=0;t<n-1;t++){
					temp[k][t]=arc[k>=i?k+1:k][t>=j?t+1:t];
				}
			}
			res[j][i]=getA(temp,n-1);
			if((i+j)%2==1){
				res[j][i]=-res[j][i];
			}
		}

	}

}

bool GetMatrixInverse(double src[4][4],int n,double des[4][4]){
	double flag=getA(src,n); 
	double t[4][4];
	if(flag==0){
		//cout<<"原矩阵行列式为0，无法求逆"<<endl;
		return false;
	}
	else{
		getAstart(src,n,t);
		for(int i=0;i<n;i++){
			for(int j=0;j<n;j++){
				des[i][j]=t[i][j]/flag;
			}
		}
	}
	return true;
}


double getA1(double arc[][6],int n){//计算|A|
	if(n==1){
		return arc[0][0];
	}
	double ans=0;
	double temp[6][6]={0.0};
	for (int i=0;i<n;i++){
		for(int j=0;j<n-1;j++){
			for(int k=0;k<n-1;k++){
				temp[j][k]=arc[j+1][(k>=i)?k+1:k];
			}
		}
		double t=getA1(temp,n-1);
		if(i%2==0){
			ans+=arc[0][i]*t;
		}
		else{
			ans-=arc[0][i]*t;
		}
	}
	return ans;
}


void getAstart1(double arc[6][6],int n,double res[6][6])//求伴随矩阵
{
	if(n==1){
		res[0][0]=1;
		return;
	}
	double temp[6][6];
	for(int i=0;i<n;i++){
		for(int j=0;j<n;j++){
			for(int k=0;k<n-1;k++){
				for(int t=0;t<n-1;t++){
					temp[k][t]=arc[k>=i?k+1:k][t>=j?t+1:t];
				}
			}
			res[j][i]=getA1(temp,n-1);
			if((i+j)%2==1){
				res[j][i]=-res[j][i];
			}
		}
	}
}

bool GetMatrixInverse1(double src[6][6],int n,double des[6][6]){
	double flag=getA1(src,n); 
	double t[6][6];
	if(flag==0){
		//cout<<"原矩阵行列式为0，无法求逆"<<endl;
		return false;
	}
	else{
		getAstart1(src,n,t);
		for(int i=0;i<n;i++){
			for(int j=0;j<n;j++){
				des[i][j]=t[i][j]/flag;
			}
		}
	}
	return true;
}

bool Jacobi(double a[][6], double p[][6], int n,double eps,int T)//求特征矩阵与特征向量
{
	int i, j, k;
	double max = a[0][1];
	int row = 0;
	int col = 0;
	double Ptemp[6][6];
	int ite_num = 0;
	for (i = 0; i < n; i++)
		p[i][i] = 1;
	while (1)
	{
		max = fabs(a[0][1]);
		row = 0;
		col = 1;
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)
				if (i != j && fabs(a[i][j])>max)
				{
					max = fabs(a[i][j]);
					row = i;
					col = j;
				}
				if (max < eps)
				{
					/*cout << "max:" << max << endl;
					cout << "t:" << ite_num << endl;*/
					return true;
				}

				if (ite_num>T)
				{
					/*cout << "max:" << max << endl;
					cout << "t:" << ite_num << endl;*/
					return false;
				}

				double theta;
				if (a[row][row] == a[col][col])
					theta = PI / 4;
				else
					theta = 0.5*atan(2 * a[row][col] / (a[row][row] - a[col][col]));
				double aii = a[row][row];
				double ajj = a[col][col];
				double aij = a[row][col];
				double sin_theta = sin(theta);
				double cos_theta = cos(theta);
				double sin_2theta = sin(2 * theta);
				double cos_2theta = cos(2 * theta);
				a[row][row] = aii*cos_theta*cos_theta + ajj*sin_theta*sin_theta + aij*sin_2theta;
				a[col][col] = aii*sin_theta*sin_theta + ajj*cos_theta*cos_theta - aij*sin_2theta;
				a[row][col] = 0.5*(ajj - aii)*sin_2theta + aij*cos_2theta;
				a[col][row] = a[row][col];
				for (k = 0; k < n; k++)
				{
					if (k != row && k != col)
					{
						double arowk = a[row][k];
						double acolk = a[col][k];
						a[row][k] = arowk * cos_theta + acolk * sin_theta;
						a[k][row] = a[row][k];
						a[col][k] = acolk * cos_theta - arowk * sin_theta;
						a[k][col] = a[col][k];
					}
				}
				if (ite_num == 0)
				{
					p[row][row] = cos_theta;
					p[row][col] = -sin_theta;
					p[col][row] = sin_theta;
					p[col][col] = cos_theta;
				}
				else
				{
					for (k = 0; k < n; k++)
					{
						double pki = p[k][row];
						double pkj = p[k][col];
						p[k][row] = pki*cos_theta + pkj*sin_theta;
						p[k][col] = pkj*cos_theta - pki*sin_theta;
					}
				}
				ite_num++;
	}    
}


bool Ellipsed(int X[],int Y[],int Z[] ,double alpha[] ){
	int mx=getMin(X);
	int my=getMin(Y);
	int mz=getMin(Z);
	int sx=(getMax(X)-mx)/2;
	int sy=(getMax(Y)-my)/2;
	int sz=(getMax(Z)-mz)/2;
	if(sx==0||sy==0||sz==0)
		return false;
	for(int i=0;i<258;i++){
		X[i]=(X[i]-mx)/sx;
		Y[i]=(Y[i]-my)/sy;
		Z[i]=(Z[i]-mz)/sz;
	}
	int D1[258][6];
	int D2[258][4];
	for(int i=0;i<258;i++){
		D1[i][0]=X[i]*X[i];
		D1[i][1]=Y[i]*Y[i];
		D1[i][2]=Z[i]*Z[i];
		D1[i][3]=X[i]*Y[i];
		D1[i][4]=X[i]*Z[i];
		D1[i][5]=Y[i]*Z[i];
		D2[i][0]=X[i];
		D2[i][1]=Y[i];
		D2[i][2]=Z[i];
		D2[i][3]=1;

	}
	int C[6][6]={{-1,1,1,0,0,0},{1,-1,1,0,0,0},{1,1,-1,0,0,0},{0,0,0,-1,0,0},{0,0,0,0,-1,0},{0,0,0,0,0,-1}};
	int S1[6][6];
	for(int i=0;i<6;i++){
		for(int j=0;j<6;j++){
			S1[i][j]=0;
			for(int k=0;k<258;k++){
				S1[i][j]+=D1[k][i]*D1[k][j];
			}
		}
	}
	int S2[6][4];
	for(int i=0;i<6;i++){
		for(int j=0;j<4;j++){
			S2[i][j]=0;
			for(int k=0;k<258;k++){
				S2[i][j]+=D1[k][i]*D2[k][j];
			}
		}
	}
	int  S3[4][4];
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			S3[i][j]=0;
			for(int k=0;k<258;k++){
				S3[i][j]+=D2[k][i]*D2[k][j];
			}
		}
	}
	double SS3[4][4];
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			SS3[i][j]=double(S3[i][j]);
		}
	}
	double RS3[4][4];
	bool isverse;
	isverse=GetMatrixInverse(SS3,4,RS3);
	if(isverse==false)
		return false;
	double T[4][6];
	for(int i=0;i<4;i++){
		for(int j=0;j<6;j++){
			T[i][j]=0;
			for(int k=0;k<4;k++){
				T[i][j]+=(-RS3[i][k])*S2[j][k];
			}
		}
	}
	double M[6][6];
	for(int i=0;i<6;i++){
		for(int j=0;j<6;j++){
			M[i][j]=0;
			for(int k=0;k<4;k++){
				M[i][j]+=S2[i][k]*T[k][j];
			}
		}
	}
	for(int i=0;i<6;i++){
		for(int j=0;j<6;j++){
			M[i][j]+=S1[i][j];
		}
	}
	double DC[6][6];
	for(int i=0;i<6;i++){
		for(int j=0;j<6;j++){
			DC[i][j]=double(C[i][j]);
		}
	}
	double ReC[6][6];
	isverse=GetMatrixInverse1(DC,6,ReC);
	if(isverse==false)
		return false;
	double Mm[6][6];
	for(int i=0;i<6;i++){
		for(int j=0;j<6;j++){
			Mm[i][j]=0;
			for(int k=0;k<6;k++){
				Mm[i][j]+=ReC[i][k]*M[k][j];
			}
		}
	}

	double evec[6][6]={0};
	Jacobi(M, evec,6,1e-10,1000);
	double a1[6][1];
	for(int m=0;m<6;m++)
	{
		for(int i=0;i<6;i++)
			a1[i][0]=evec[i][m];
		double total[1][6];
		for(int i=0;i<1;i++){
			for(int j=0;j<6;j++){
				total[i][j]=0;
				for(int k=0;k<6;k++){
					total[i][j]+=a1[k][i]*C[k][j];
				}
			}
		}
		double sum=0;
		for(int s=0;s<6;s++){
			sum+=a1[s][0]*total[0][s];
		}
		if(sum>0)
			break;

	}
	double aa[4][1];
	for(int i=0;i<4;i++){
		for(int j=0;j<1;j++){
			aa[i][j]=0;
			for(int k=0;k<6;k++){
				aa[i][j]+=T[i][k]*a1[k][j];
			}
		}
	}

	alpha[0]=a1[0][0]/(sx*sx);
	alpha[1]=a1[1][0]/(sy*sy);
	alpha[2]=a1[2][0]/(sz*sz);
	alpha[3]=a1[3][0]/(sx*sy);
	alpha[4]=a1[4][0]/(sx*sz);
	alpha[5]=a1[5][0]/(sy*sz);
	alpha[6]= -2*mx*a1[0][0]/(sx*sx)-my*a1[3][0]/(sx*sy)-mz*a1[4][0]/(sx*sz)+aa[0][0]/sx;
	alpha[7]= -2*my*a1[1][0]/(sy*sy)-mx*a1[3][0]/(sx*sy)-mz*a1[5][0]/(sy*sz)+aa[1][0]/sy;
	alpha[8]= -2*mz*a1[2][0]/(sz*sz)-mx*a1[4][0]/(sx*sz)-my*a1[5][0]/(sy*sz)+aa[2][0]/sz;
	alpha[9]=mx*mx*a1[0][0]/(sx*sx) + my*my*a1[1][0]/(sy*sy) + mz*mz*a1[2][0]/(sz*sz) + mx*my*a1[3][0]/(sx*sy) + mx*mz*a1[4][0]/(sx*sz) + my*mz*a1[5][0]/(sy*sz) - mx*aa[0][0]/sx - my*aa[1][0]/sy - mz*aa[2][0]/sz + aa[3][0];
	return true;
} 
//bool SomaJudge(char * origin,int *DTarray,double alpha[],int row,int col,int height,double rate,int light){
//	int test[3];
//	double sum=0;;
//	int num=0;
//	int  num1=0;
//	float hh=0.0;
//	for(int m=0;m<row*col*height;m++){
//		getIndex(m,row,col,height,test);
//		sum=alpha[0]*test[0]*test[0]+alpha[1]*test[1]*test[1]+alpha[2]*test[2]*test[2]+alpha[3]*test[0]*test[1]+alpha[4]*test[0]*test[2]+alpha[5]*test[1]*test[2]+alpha[6]*test[0]+alpha[7]*test[1]+alpha[8]*test[2]+alpha[9];
//		if(sum<0){
//			num++;
//			if(DTarray[m]>=light){
//				num1++;
//			}
//		}
//	}
//	//cout<<"num1    "<<num1<<endl;
//	hh=(float)num1/num;
//   // cout<<"rate  "<<hh<<endl;
//	if(hh>rate)
//		return true;
//	else return false;
//	
//
//}

bool SomaJudge(char * origin,unsigned char *DTarray,int i,double alpha[],int row,int col,int height,int resultFilteringThreshold,char * flagtest,int somacount[]){

 int test[3];
 int count=0;
 double sum=0.0;
 getIndex(i,row,col,height,test);
 vector <int> flag;
 if((test[0]-25)>=0&&(test[0]+25)<row&&(test[1]-25)>=0&&(test[1]+25)<col&&(test[2]-25)>=0&&(test[2]+25)<height){
	  for(int a=test[0]-25;a<(test[0]+25);a++)
	  {
		  for(int b=test[1]-25;b<(test[1]+25);b++){
			  for(int c=test[2]-25;c<(test[2]+25);c++)
			  {  
				 sum=alpha[0]*a*a+alpha[1]*b*b+alpha[2]*c*c+alpha[3]*a*b+alpha[4]*a*c+alpha[5]*b*c+alpha[6]*a+alpha[7]*b+alpha[8]*c+alpha[9];
				 if (sum<0){
					 count++;
					 flag.push_back(getNum(a,b,c,row,col,height));
					 }
			  }
		  }
	  }
  }
  if(count>=resultFilteringThreshold){

	  for(int s=0;s<flag.size();s++)
		   {
			   somacount[0]=count;
	  flagtest[flag[s]]=1;}
	  return true;
            
  }
	  
  else return false;
  
}
 unsigned char * RayBurstSampling(char * origin,unsigned char * DTarray,unsigned char * RMarray,int row,int col,int height,int resultFilteringThreshold){
	
	//struct tag  ray;
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
	ifile.close();
	int test[3];
	int flag1;
	int flag2;
	int flag3;
	int flag;
	int x1;
	int y1;
	int z1;
	int x_front;
	int y_front;
	int z_front;
	float var_x;
	float var_y;
	float var_z;
	float x;
	float y;
	float z;
	int X[258];
	int Y[258];
	int Z[258];
    double alpha[10];
	bool ellipse;
	bool soma;
	unsigned char *final;
	int count=0;
	char  *flagtest;
	int somacount[1];
	somacount[0]=0;
	ofstream fout("F:/data.txt");
	try{
		final = (unsigned char*)malloc(sizeof(unsigned char) * row * col * height);
		flagtest=(char*)malloc(sizeof(char) * row * col * height);
	}catch(...){
		cout << __FUNCTION__ << ": " << "error mallco" << endl; 
		return NULL;
	}
	for(int s=0;s< row * col * height;s++)
		flagtest[s]=0;
	for(int i=0;i<row*col*height;i++)
	{   final[i]=0;
		if(RMarray[i]==1){
			count++;
			getIndex(i,row,col,height,test);
			for(int j=0;j<258;j++){
				flag1=1;
				flag2=1;
				flag3=1;
				if(array[j][0]<0)
					flag1=-1;
				if(array[j][1]<0)
					flag2=-1;
				if(array[j][2]<0)
					flag3=-1;
				flag=0;
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
					var_x=flag1;
					x=test[0]+var_x;
					y=test[1]+var_x/array[j][0]*array[j][1];
					z=test[2]+var_x/array[j][0]*array[j][2];
					x1=int(x);
					y1=int(y);
					z1=int(z);
					x_front=test[0];
					y_front=test[1];
					z_front=test[2];
					while(DTarray[getNum(x1,y1,z1,row,col,height)]<DTarray[getNum(x_front,y_front,z_front,row,col,height)]&&DTarray[getNum(x1,y1,z1,row,col,height)]!=0&&x1>0&&x1<(row-1)&&y1>0&&y1<(col-1)&&z1>0&&z1<(height-1)&&x_front>=0&&x_front<row&&y_front>=0&&y_front<col&&z_front>=0&&z_front<height){
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
					X[j]=x_front;
					Y[j]=y_front;
					Z[j]=z_front;
				}
				if(flag==1){
					var_y=flag2;
					x=test[0]+var_y/array[j][1]*array[j][0];
					y=test[1]+ var_y;
					z=test[2]+var_y/array[j][1]*array[j][2];
					x1=int(x);
					y1=int(y);
					z1=int(z);
					x_front=test[0];
					y_front=test[1];
					z_front=test[2];
					while(DTarray[getNum(x1,y1,z1,row,col,height)]<DTarray[getNum(x_front,y_front,z_front,row,col,height)]&&DTarray[getNum(x1,y1,z1,row,col,height)]!=0&&x1>0&&x1<(row-1)&&y1>0&&y1<(col-1)&&z1>0&&z1<(height-1)&&x_front>=0&&x_front<row&&y_front>=0&&y_front<col&&z_front>=0&&z_front<height){
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
					X[j]=x_front;
					Y[j]=y_front;
					Z[j]=z_front;
				}
				if(flag==2){
					var_z=flag3;
					x=test[0]+var_z/array[j][2]*array[j][0];
					y=test[1]+ var_z/array[j][2]*array[j][1];
					z=test[2]+var_z;
					x1=int(x);
					y1=int(y);
					z1=int(z);
					x_front=test[0];
					y_front=test[1];
					z_front=test[2];
					while(DTarray[getNum(x1,y1,z1,row,col,height)]<DTarray[getNum(x_front,y_front,z_front,row,col,height)]&&DTarray[getNum(x1,y1,z1,row,col,height)]!=0&&x1>0&&x1<(row-1)&&y1>0&&y1<(col-1)&&z1>0&&z1<(height-1)&&x_front>=0&&x_front<row&&y_front>=0&&y_front<col&&z_front>=0&&z_front<height)
						{
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
					X[j]=x_front;
					Y[j]=y_front;
					Z[j]=z_front;
				}
			}
			ellipse=Ellipsed(X,Y,Z,alpha);         
			if(ellipse==true){
				soma=SomaJudge(origin, DTarray,i,alpha,row, col, height,resultFilteringThreshold,flagtest,somacount);
				if(soma==true){
					//cout<<"start write"<<endl;
					if(flagtest[i]==0)
					fout<<test[0]<<","<<test[1]<<","<<test[2]<<","<<somacount[0]<<endl;				
				}
				
			}
		}

		}
	/*cout<<"elll-final"<<endl;
	int h=0;
	for(int a=0;a<row*col*height;a++){
		if(flagtest[a]==1){
			h++;
			final[a]=origin[a];}
			}
		cout<<"h   "<<h<<endl;*/
		fout.close();
		return final;

	}




