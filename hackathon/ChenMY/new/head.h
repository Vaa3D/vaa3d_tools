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

using namespace std;
int dx[26] = { 1,1,1,-1,-1,-1,0,0,1,1,1,0,0,0,-1,-1,-1,1,1,1,0,0,0,-1,-1,-1};
int dy[26] = { 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
int dz[26] = { 1,0,-1,1,0,-1,1,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0,-1 };

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
			if (nr >= 0 && nr < row && nc >= 0 && nc < col && rh >= 0 && rh < height && !vis[getNum(nr,nc,rh,row,col,height)] && array[getNum(nr, nc, rh,row,col,height)] == 255) { // 判断是否出界，是否访问过，是否为1
				vis[getNum(nr,nc,rh,row,col,height)]=count;
				q.push(getNum(nr,nc,rh,row,col,height));	
			}
		}
	}
}




