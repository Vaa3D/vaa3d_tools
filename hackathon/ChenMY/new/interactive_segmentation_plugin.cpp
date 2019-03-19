 #include "v3d_message.h"
#include <vector>
#include <map>
#include <iostream>
#include <QString>
#include <math.h>
#include "head.h"

#include<fstream>
//#include <vnl_vector.h>
//#include <vnl_sparse_matrix.h>
//#include <vnl/algo/vnl_sparse_lu.h>
#include <time.h>
#include "interactive_segmentation_plugin.h"
#include "NeuronEnhancementFilter.h"
#include "NeuronSegmentation.h"
using namespace std;

Q_EXPORT_PLUGIN2(image_segmentation,image_segmentation);
 
QStringList image_segmentation::menulist() const
{
	return QStringList() 
		<<tr("interactive_segmentation")
		<<tr("about");
}

QStringList image_segmentation::funclist() const
{
    return QStringList()
        <<tr("interactive_segmentation")
		<<tr("about");
}


void image_segmentation::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("interactive_segmentation"))
	{
        image_IO(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will show mean distance"));
	}
}

bool image_segmentation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("interactive_segmentation"))

    {
		cout<<"000000000000000000000000000000000"<<endl;
        image_IO(input,output, callback,true);

    }
	
	else return false;

	return true;
}

template <class T1, class T2> bool assign_val(T1 *dst, T2 *src, V3DLONG total_num)
{
	if (!dst || !src || total_num<=0) return false;
	for (V3DLONG i=0; i<total_num; i++){
		//if(src[i]>15)
		//	dst[i] = 0;
		//else
			dst[i] = src[i];
	} 
		
	return true;
}

template <class T1, class T2> bool assign_val_condition(T1 *dst, T2 *src, V3DLONG total_num, T2 condition_val, T1 condition_yes_val, T1 condition_no_val)
{
	if (!dst || !src || total_num<=0) return false;
	for (V3DLONG i=0; i<total_num; i++) 
		dst[i] = (src[i]==condition_val) ? condition_yes_val : condition_no_val;
	return true;
}

template <class T1, class T2> bool do_seg(T1 * inputData,
                               T2 * outputData,
                               V3DLONG sx, V3DLONG sy, V3DLONG sz, 
                               int & iVesCnt, bool b_binarization=true)
{
    bool res=false;
    if (!inputData || !outputData || sx<=0 || sy<=0 || sz<=0)
        return res;
    
	short *internal_input = 0;
	V3DLONG channel_sz = sx*sy*sz;
    
	try
	{
		internal_input = new short[channel_sz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in do_seg().");
        res = false;
        goto Label_exit_do_seg;
	}
	
    assign_val(internal_input, inputData, channel_sz);
    
    if(!do_seg(internal_input, sx, sy, sz, iVesCnt, b_binarization))
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        res = false;
        goto Label_exit_do_seg;
    }	
	
	printf("Line/Vessle count = [%ld]\n", iVesCnt);
	
    if (b_binarization)
        assign_val_condition(outputData, internal_input, channel_sz, (short int)(-1), (T2)0, (T2)255);
    else
    {
        for (V3DLONG i=0;i<channel_sz;i++)
            outputData[i] = (T2)(internal_input[i]);
    }
    
    res = true; 
    
Label_exit_do_seg:
    if (internal_input) {delete []internal_input; internal_input=0;}
    return res;
}


template <class T1, class T2> bool do_seg2(T1 * inputData,
                               T2 * outputData,
                               V3DLONG sx, V3DLONG sy, V3DLONG sz, 
                               int & iVesCnt, bool b_binarization=true)
{
    bool res=false;
    if (!inputData || !outputData || sx<=0 || sy<=0 || sz<=0)
        return res;
    
	short *internal_input = 0;
	V3DLONG channel_sz = sx*sy*sz;
    
	try
	{
		internal_input = new short[channel_sz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in do_seg().");
        res = false;
        goto Label_exit_do_seg;
	}
	
    assign_val(internal_input, inputData, channel_sz);
    
    if(!do_seg2(internal_input, sx, sy, sz, iVesCnt, b_binarization))
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        res = false;
        goto Label_exit_do_seg;
    }	
	
	printf("Line/Vessle count = [%ld]\n", iVesCnt);
	
    if (b_binarization)
        assign_val_condition(outputData, internal_input, channel_sz, (short int)(-1), (T2)0, (T2)255);
    else
    {
        for (V3DLONG i=0;i<channel_sz;i++)
            outputData[i] = (T2)(internal_input[i]);
    }
    
    res = true; 
    
Label_exit_do_seg:
    if (internal_input) {delete []internal_input; internal_input=0;}
    return res;
}


//template<class T>
//vnl_vector<T> my_get_row(vnl_sparse_matrix<T> S,unsigned int row_index)
//{
//  vnl_vector<T> v( S.cols(), 0 );
//
//  for (unsigned int j = 0; j != S.cols(); ++j) {    // For each element in row, unefficient way to read all values of a sparse matrix
//    v[j] = S(row_index,j);
//  }
//  return v;
//}




/*--------------------------------------------
  elim element at position  pos
  --------------------------------------------*/
//vnl_vector<double> elim_ele(vnl_vector<double> b, int pos)
//{
//  int sz = b.size();
//  vnl_vector<double> b_elim(sz-1);
//  int it_in,it_out;
//  for(it_in=0, it_out=0; it_in!=sz; ++it_in, ++it_out)
//  { 
//    if (it_in==pos) {
//      ++it_in;
//    }
//    //else {
//      b_elim(it_out)=b(it_in);
//    //}
//  }
//  return b_elim;
//}
//
///*--------------------------------------------
//  insert element at position  pos
//  --------------------------------------------*/
//vnl_vector<double> insert_ele(vnl_vector<double> b_elim, int pos, double value)
//{
//  int sz = b_elim.size();
//  vnl_vector<double> b(sz+1);
//  int it_in,it_out;
//  for(it_in=0, it_out=0; it_in!=sz; ++it_in, ++it_out)
//  { 
//    if (it_in==pos) {
//      b(it_out) = value;
//      ++it_out;
//    }
//    b(it_out)=b_elim(it_in);
//  }
//  return b;
//
//}
//
//
///*--------------------------------------------
//  elim element at position  pos
//  --------------------------------------------*/
//vnl_sparse_matrix<double> elim_ele(vnl_sparse_matrix<double> A, int pos)
//{
//  int M = A.rows();
//  int N = A.columns();
//  vnl_sparse_matrix<double> A_elim(M-1,N-1);
//  int it_in_x,it_out_x,it_in_y,it_out_y;
//
//  for(it_in_y=0, it_out_y=0; it_in_y!=N; ++it_in_y, ++it_out_y) {
//    if(it_in_y==pos) {
//      ++it_in_y;
//    }
//    for(it_in_x=0, it_out_x=0; it_in_x!=M; ++it_in_x, ++it_out_x) {
//      if (it_in_x==pos) {
//        ++it_in_x;
//      }
//      A_elim(it_out_x,it_out_y)=A(it_in_x,it_in_y);
//    }
//  }
//  return A_elim;
//}





int cal_weight(int a,int b){
	
	return abs(a*a - b*b);
}

double edge_weight( int g1, int g2 )
{
  double beta = 90;
  double dg = ((double) g1)/256 - ((double) g2)/256;
  double weight;
  weight =  exp(-beta * dg * dg);
  
  return std::max(weight,1e-5);
}


void ind2sub(int i,long *index, int (&array)[3])//将一维转化为三维
{
  // Warning: Untested!!!
  //int	*ind;
  int x ,y, z;
  z = i/(index[0]*index[1]);
  y = (i%(index[0]*index[1]))/index[0];
  x = (i%(index[0]*index[1]))%index[0];
  array[0] = x,array[1] = y,array[2] = z;
  //return ind;
}


void image_IO(V3DPluginCallback2 &callback, QWidget *parent){
	v3dhandle curwin = callback.currentImageWindow();
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	LandmarkList landmarks = callback.getLandmark(curwin);
	cout<<"marker size = "<<landmarks.size()<<endl;
	if (!subject)
	{
		QMessageBox::information(0, "", QObject::tr("No image is open."));
		return;
	}
	//if(landmarks.size()<2){
	//	QMessageBox::information(0, "", QObject::tr("Need at least 2 markers."));
	//	return;
	//}
    unsigned char * inimg1d = subject->getRawData(); 
    V3DLONG in_sz[4]; 
    in_sz[0] = subject->getXDim(); in_sz[1] = subject->getYDim(); in_sz[2] = subject->getZDim(); in_sz[3] = subject->getCDim();
    int datatype = subject->getDatatype();
	
    V3DLONG channel_sz = in_sz[0]*in_sz[1]*in_sz[2];//总像素
    V3DLONG outsz[4];

	//新增2      
	int* vis=0;
	try{
		vis=new int[channel_sz];
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;

	}

	//copy
	int* vist=0;
	try{
		vist=new int[channel_sz];
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;

	}

	int dx[6] = { 0,0,1,-1,0,0};
	int dy[6] = { 0,0,0,0,1,-1};
	int dz[6] = { 1,-1,0,0,0,0};

	/*vector<int>ld(landmarks.size());
	int indRHS[2];
	for(int i=0;i<landmarks.size();i++){
		
		int x = (int)landmarks.at(i).x;
		int y = (int)landmarks.at(i).y;
		int z = (int)landmarks.at(i).z;
		indRHS[i] = z*in_sz[0]*in_sz[1]+y*in_sz[0]+x;
		ld[i] = (int)inimg1d[z*in_sz[0]*in_sz[1]+y*in_sz[0]+x];
		cout<<"marker value="<<(int)inimg1d[z*in_sz[0]*in_sz[1]+y*in_sz[0]+x]<<endl;
		cout<<"marker x="<<(int)landmarks.at(i).x<<endl;
		cout<<"marker y="<<(int)landmarks.at(i).y<<endl;
		cout<<"marker z="<<(int)landmarks.at(i).z<<endl;
	}
	*/
	//int index[3] = {2,2,2};
    //segmentation
    //int *sub =  ind2sub(5,index);
	//cout<<"x = "<<sub[0]<<endl;cout<<"y = "<<sub[1]<<endl;cout<<"z = "<<sub[2]<<endl;
    int iVesCnt = 1, i;
    bool res,res2;
	unsigned char * m_OutImgData = 0;
	unsigned char * m_OutImgData2 = 0;
    unsigned char * m_OutImgData3 = 0;
	try
	{
		m_OutImgData = new  unsigned char[channel_sz]; 
		m_OutImgData2 = new unsigned char[channel_sz];
		m_OutImgData3 = new unsigned char[channel_sz];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
	}
	
	bool b_binarization = true;
	//改动
    //switch (datatype)
    //{
    //    case 1:
    //        res = do_seg(inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
    //        break;
    //    case 2:
    //        res = do_seg((short int *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
    //        break;
    //    case 4:
    //        res = do_seg((float *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
    //        break;
    //    default:
    //        break;
    //}

    //
    //if(!res)
    //{
    //    cerr<<"The segmentation of foreground fails."<<endl;
    //    if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
    //    return;    
    //}

	switch (datatype)
    {
        case 1:
            res = do_seg2(inimg1d, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 2:
            res = do_seg2((short int *)inimg1d, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 4:
            res = do_seg2((float *)inimg1d, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        default:
            break;
    }
	if(!res)
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        if (m_OutImgData2) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
    }
	////////////////////////////////////////
	/*
	vector<int>ld(landmarks.size());
	for(int i=0;i<landmarks.size();i++){
		
		int x = (int)landmarks.at(i).x;
		int y = (int)landmarks.at(i).y;
		int z = (int)landmarks.at(i).z;
		ld[i] = (int)inimg1d[z*in_sz[0]*in_sz[1]+y*in_sz[0]+x];
		cout<<"marker value="<<(int)inimg1d[z*in_sz[0]*in_sz[1]+y*in_sz[0]+x]<<endl;
		cout<<"marker x="<<(int)landmarks.at(i).x<<endl;
		cout<<"marker y="<<(int)landmarks.at(i).y<<endl;
		cout<<"marker z="<<(int)landmarks.at(i).z<<endl;
	}
	
	//for
	//double ave = */
	long size[3] = {in_sz[0],in_sz[1],in_sz[2]};
	int location[3]={0,0,0};
	for(int i=0;i<channel_sz;i++){
		ind2sub(i,size,location);
		//m_OutImgData2[i] = (int)m_OutImgData[i]+(int)inimg1d[i];
		if(5<location[1]&&location[1]<in_sz[1]-5){
			if(((int)m_OutImgData[i] >= (int)m_OutImgData2[i]))
				m_OutImgData3[i] = m_OutImgData[i];
			else
				m_OutImgData3[i] = m_OutImgData2[i];
		}
		else
			m_OutImgData3[i] = 0;
	}


	for(int i=0;i<channel_sz;i++){
		if(((int)inimg1d[i]>(int)m_OutImgData3[i])&&((int)inimg1d[i]>150))
			m_OutImgData3[i] = 255;
		if((int)inimg1d[i]<100)
			m_OutImgData2[i] = 0;
		
	}

	//新增1
	//for(int i=0;i<channel_sz;i++){
	//	if(((int)inimg1d[i]>(int)m_OutImgData2[i])&&((int)inimg1d[i]>200))
	//		m_OutImgData2[i] = 255;

	//}


	//新增3
	int flag = 0;
	for (int i = 0; i < in_sz[0]; i++)
		for (int j = 0; j < in_sz[1]; j++)
			for (int k = 0; k < in_sz[2]; k++) {
				if (m_OutImgData2[getNum(i, j, k,in_sz[0],in_sz[1],in_sz[2])] == 255 && !vis[getNum(i,j,k,in_sz[0],in_sz[1],in_sz[2])]) {
					flag++;
					bfs(m_OutImgData2,in_sz[0],in_sz[1],in_sz[2], i, j, k ,flag,vis);
					
				}
			}

			int *Count;
			cout<<"flag:  "<<flag<<endl;
			try{
				Count = (int*)malloc(sizeof(int) * (flag+1));
			}catch(...){
				cout << __FUNCTION__ << " error malloc" << endl;
				return ;
			}
			for (int m = 0; m <= flag; m++)
				Count[m] = 0;
			for(int i=0;i<channel_sz;i++ ){
				Count[vis[i]]++;
			}
		
			for(int i=0;i<channel_sz;i++){
			   
				if(Count[vis[i]] <300){
					m_OutImgData2[i] = 0;
				}

			}
			cout<<"endl"<<endl;
			for(int i=0;i<(flag+1);i++)
			{
				cout<<Count[i]<<endl;
			}
			//copy
			int flags = 0;
			for (int i = 0; i < in_sz[0]; i++)
				for (int j = 0; j < in_sz[1]; j++)
					for (int k = 0; k < in_sz[2]; k++) {
						if (m_OutImgData2[getNum(i, j, k,in_sz[0],in_sz[1],in_sz[2])] == 255 && !vist[getNum(i,j,k,in_sz[0],in_sz[1],in_sz[2])]) {
							flags++;
							bfs(m_OutImgData2,in_sz[0],in_sz[1],in_sz[2], i, j, k ,flags,vist);

						}
					}

					int *Counts;
					cout<<"flag:  "<<flags<<endl;
					try{
						Counts = (int*)malloc(sizeof(int) * (flags+1));
					}catch(...){
						cout << __FUNCTION__ << " error malloc" << endl;
						return ;
					}
					for (int m = 0; m <= flags; m++)
						Counts[m] = 0;
					for(int i=0;i<channel_sz;i++ ){
						Counts[vist[i]]++;
					}
					cout<<"the secend"<<endl;
					/*	for(int i=0;i<channel_sz;i++){
					if(vist[i]>0)
					cout<<vist[i]<<endl;


					}*/
					cout<<"endlllll"<<endl;
					for(int i=0;i<(flags+1);i++)
					{
						cout<<Counts[i]<<endl;
					}
					long ax;
					long ay;
					long az;
					int array[3];
					int sum=0;
					ofstream fout("F:/data1.txt");
					//freopen("F:/log.txt","w",stdout);
					for(int i=0;i<(flags+1);i++){
						ax=0;
						ay=0;
						az=0;
						for(int j=0;j<channel_sz;j++){
						    //cout<<"step1:"<<endl;
							if(m_OutImgData2[j]==255&&vist[j]==i){
								getIndex(j,in_sz[0],in_sz[1],in_sz[2],array);
								ax=ax+array[0];
								ay=ay+array[1];
								az=az+array[2];
								sum=Counts[vist[j]];
								  //cout<<"step2:"<<endl;
							}
						}
						 if(sum!=0&&ax!=0){
						ax=ax/sum;
						ay=ay/sum;
						az=az/sum;
                  fout<<ax<<" "<<ay<<" "<<az<<" ";	
						 }
					}
			

			//新增4
	/*		char  *flagtest;
			try{
				
				flagtest=(char*)malloc(sizeof(char) *channel_sz);
			}catch(...){
				cout << __FUNCTION__ << ": " << "error mallco" << endl; 
				return ;
			}
			for(int s=0;s< channel_sz;s++)
				flagtest[s]=0;
			bool soma;
			cout<<"soma位置"<<endl;
			for(int i=0;i< channel_sz;i++){
			soma=SomaJudge(m_OutImgData2,i,in_sz[0],in_sz[1],in_sz[2],4200,flagtest);
			if(soma==true){
				if(flagtest[i]==1)
					cout<<test[0]<<","<<test[1]<<","<<test[2]<<","<<endl;				
			}
			}*/
	//vnl_sparse_matrix<double> vnlMatrix(channel_sz,channel_sz);
	//long size[3] = {in_sz[0],in_sz[1],in_sz[2]};
	
	/*for(int i=0;i<channel_sz;i++){
		int *location = ind2sub(i,size);
		for(int j = 0;j<6;j++){
			//int rx = location[0]+dx[j],ry = location[1]+dy[j],rz =location[2]+dz[j];
			//if(rx >= 0 && rx < in_sz[0] && ry >= 0 && ry < in_sz[1]&& rz >= 0 && rz < in_sz[2]){
				vnlMatrix(i,j) = 1;
			//}
		}
		if(i == channel_sz-1){
			cout<<"x:"<<location[0]<<endl;
			cout<<"y:"<<location[1]<<endl;
			cout<<"z:"<<location[2]<<endl;
		}
		//free(location);
		//location = NULL;
	}
	int *location = NULL;
	for(int i=0;i<channel_sz;i++)
	{	
		location = ind2sub(i,size);
		if(i == channel_sz-1){
			cout<<"channel size = "<<channel_sz<<endl;
			cout<<"x = "<<location[0]<<endl;cout<<"y = "<<location[1]<<endl;cout<<"z = "<<location[2]<<endl;
		}

		for(int j = 0;j<6;j++){
			int rx = location[0]+dx[j],ry = location[1]+dy[j],rz =location[2]+dz[j];
			if(rx >= 0 && rx < in_sz[0] && ry >= 0 && ry < in_sz[1] && rz >= 0 && rz < in_sz[2]){
				if(i == channel_sz-1){
				cout<<"start last one"<<endl;
				}
				//vnlMatrix(i,rx+ry*in_sz[0]+rz*in_sz[0]*in_sz[1]) = -cal_weight(m_OutImgData2[i],m_OutImgData2[rx+ry*in_sz[0]+rz*in_sz[0]*in_sz[1]]);
				vnlMatrix(i,rx+ry*in_sz[0]+rz*in_sz[0]*in_sz[1]) = -cal_weight(255,128);
				if(i == channel_sz-1){
				cout<<"done"<<endl;
				}
			}
		}
	}
	//int *location = NULL;
	double start,end,cost; 
	start=clock(); 
	int location[3]={0,0,0};
	for(int i=0;i<channel_sz;i++){
		ind2sub(i,size,location);
		for(int j = 0;j<6;j++){
			int rx = location[0]+dx[j],ry = location[1]+dy[j],rz =location[2]+dz[j];
			if(rx >= 0 && rx < in_sz[0] && ry >= 0 && ry < in_sz[1]&& rz >= 0 && rz < in_sz[2]){
				vnlMatrix(i,rx+ry*in_sz[0]+rz*in_sz[0]*in_sz[1]) = -edge_weight((int)m_OutImgData2[i],(int)m_OutImgData2[rx+ry*in_sz[0]+rz*in_sz[0]*in_sz[1]]);
				vnlMatrix(i,i) += edge_weight((int)m_OutImgData2[i],(int)m_OutImgData2[rx+ry*in_sz[0]+rz*in_sz[0]*in_sz[1]]);
			}
		}
		//delete(location);
	}
	
	vnl_vector<double> solution(channel_sz);
	vnl_vector<double> b(channel_sz,0);
	b[indRHS[0]] = 1;*/
	//cout << "cal b start" << std::endl;
	//b = my_get_row <double> (vnlMatrix, indRHS[0]);
	//cout << "cal b end" << std::endl;
	//vnl_vector<double> solution_elim(channel_sz-1);
    //vnl_vector<double> b_elim(channel_sz-1,0); 
    //vnl_sparse_matrix<double> vnlMatrix_elim(channel_sz-1,channel_sz-1);
	/*
	//#ifdef DEBUG
		std::cout << "Eliminate element in Vector..." << std::endl;
	//#endif
		 b_elim = elim_ele(b,indRHS[0]);
	//#ifdef DEBUG
		std::cout << "Eliminate element in Matrix..." << std::endl;
	//#endif
	vnlMatrix_elim = elim_ele(vnlMatrix,indRHS[0]);


	//#ifdef DEBUG
		std::cout << "Solving large linear system..." << std::endl;
	//#endif

  //vnl_sparse_lu linear_solver(vnlMatrix, vnl_sparse_lu::estimate_condition);
  //linear_solver.solve( -b , &solution);

  //solution = insert_ele(solution_elim,indRHS[0],1);
  end=clock();
  cost = end-start;
  cout<<"cost = "<<cost/1000<<"s"<<endl;

  // export
  //vnl_matrix<double> solM(x,y);
  //solM = reshape(solution, x, y);
*/


   //改动
	//Image4DSimple p4DImage,p4DImage2;
	//p4DImage.setData((unsigned char*)m_OutImgData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
     Image4DSimple  p4DImage2;
	p4DImage2.setData((unsigned char*)m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
	//v3dhandle newwin,newwin2;
	//newwin = callback.newImageWindow();
	//callback.setImage(newwin, &p4DImage);		
	//callback.setImageName(newwin, QString("Segmentation1"));
	//callback.updateImageWindow(newwin);
	v3dhandle  newwin2;
	newwin2 = callback.newImageWindow();
	callback.setImage(newwin2, &p4DImage2);		
	callback.setImageName(newwin2, QString("Segmentation2"));
	callback.updateImageWindow(newwin2);
	//Clear();
    fout.close();
	string path="F:/data1.txt";
	ifstream fin(path);
	int *marker;
	try{
		marker=(int *)malloc (sizeof(int)*flags*3);
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		return;
	}
	int line=0;
	int a,b,c;
	while (fin >> a >> b >> c) {
		marker[3*line] = a;
		marker[3*line+1] = b;
		marker[3*line+2] = c;
		line++;
	}
	fin.close();
	ofstream fmarker("F://ouf.marker");
	for(int i=0;i<line;i++)
		fmarker<<marker[3*i]<<","<<marker[3*i+1]<<","<<marker[3*i+2]<<","<<3<<","<<0<<","<<0<<","<<0<<","<<125<<","<<0<<","<<0<<endl;
	fmarker.close();
		return; 
}


void image_IO(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback &callback,bool b_binarization)
{
    if (input.size()<=0 && output.size()<=0)
        return;
	char * infile = (*(vector<char*> *)(input.at(0).p)).at(0);
    
	cout<<"infile : "<<infile<<endl;

	string str = infile;
	str = str +"_seg.v3draw";
	char * outfile = (char *)str.data();

	


    unsigned char * inimg1d = 0; V3DLONG in_sz[4]; int datatype;
    simple_loadimage_wrapper(callback,infile, inimg1d, in_sz, datatype);
    
	V3DLONG channel_sz = in_sz[0]*in_sz[1]*in_sz[2];
    V3DLONG outsz[4];
    
    //segmentation
    
    int iVesCnt = 1, i;
    bool res;
	
	unsigned char * m_OutImgData = 0;
	unsigned char * m_OutImgData2 = 0;
    unsigned char * m_OutImgData3 = 0;
	try
	{
		m_OutImgData = new  unsigned char[channel_sz]; 
		m_OutImgData2 = new unsigned char[channel_sz];
		m_OutImgData3 = new unsigned char[channel_sz];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Neuron_segment_entry_func().");
        if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
	}
	
    switch (datatype)
    {
        case 1:
            res = do_seg(inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 2:
            res = do_seg((short int *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 4:
            res = do_seg((float *)inimg1d, m_OutImgData, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        default:
            break;
    }
    
    if(!res)
    {
        cerr<<"The enhancement/segmentation of foreground fails."<<endl;
        goto Label_exit_Neuron_segment_entry_func;
    }	
	switch (datatype)
    {
        case 1:
            res = do_seg2(inimg1d, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 2:
            res = do_seg2((short int *)inimg1d, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        case 4:
            res = do_seg2((float *)inimg1d, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
            break;
        default:
            break;
    }
	if(!res)
    {
        cerr<<"The segmentation of foreground fails."<<endl;
        if (m_OutImgData2) {delete []m_OutImgData; m_OutImgData=0;}
        return;    
    }

	long size[3] = {in_sz[0],in_sz[1],in_sz[2]};
	int location[3]={0,0,0};
	for(int i=0;i<channel_sz;i++){
		ind2sub(i,size,location);
		//m_OutImgData2[i] = (int)m_OutImgData[i]+(int)inimg1d[i];
		if(5<location[1]&&location[1]<in_sz[1]-5){
			if(((int)m_OutImgData[i] >= (int)m_OutImgData2[i]))
				m_OutImgData3[i] = m_OutImgData[i];
			else
				m_OutImgData3[i] = m_OutImgData2[i];
		}
		else
			m_OutImgData3[i] = 0;
	}


	for(int i=0;i<channel_sz;i++){
		if(((int)inimg1d[i]>(int)m_OutImgData3[i])&&((int)inimg1d[i]>150))
			m_OutImgData3[i] = 255;
		
	}
	
	
    //save mask file
    for (i=0;i<3;i++) outsz[i]=in_sz[i]; outsz[3]=1;	
    simple_saveimage_wrapper(callback,outfile, m_OutImgData3, outsz, 1);
    
    //clear memory
Label_exit_Neuron_segment_entry_func:
    if (m_OutImgData3) {delete []m_OutImgData3; m_OutImgData3=0;}
	if (m_OutImgData2) {delete []m_OutImgData2; m_OutImgData2=0;}
	if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=0;}
    if (inimg1d) {delete []inimg1d; inimg1d=0;}
}


bool do_seg(short *pData, V3DLONG sx, V3DLONG sy, V3DLONG sz, int & iVesCnt, bool b_binarization=true)
{
    if (!pData || sx<=0 || sy<=0 || sz<=0)
        return false;
    
	int iSigma[1] = {1};
	
	//vesselness
	float fA = 0.5;
	float fB = 0.5;
    float fC = 1.0*10*5;
	
	//volume filter
	bool bVolFilter = true;
    
	NeuronSeg m_CerebroSeg;
	m_CerebroSeg.Neuron_Segmentation(pData, iSigma, 
									  sy, sx, sz,
									  fA, fB, fC,  //parameters for the line-likelihood
									  1,NULL,
									  80, //ignore connected components that size smaller than this threshold 
                                      iVesCnt,//220 ,3 30,1.300.3//40.3／／20.3 50 .3//20 .3
									  b_binarization, 
									  bVolFilter);	
    return true;
}
bool do_seg2(short *pData, V3DLONG sx, V3DLONG sy, V3DLONG sz, int & iVesCnt, bool b_binarization=true)
{
    if (!pData || sx<=0 || sy<=0 || sz<=0)
        return false;
    
	int iSigma[1] = {1};
	
	//vesselness
	float fA = 0.5;
	float fB = 0.5;
    float fC = 1.0*10*5;
	
	//volume filter
	bool bVolFilter = true;
    
	NeuronSeg m_CerebroSeg;
	m_CerebroSeg.Neuron_Segmentation2(pData, iSigma, 
									  sy, sx, sz,
									  fA, fB, fC,  //parameters for the line-likelihood
									  1,NULL,
									  80, //ignore connected components that size smaller than this threshold 
                                      iVesCnt,//220 ,3 30,1.300.3//40.3／／20.3 50 .3//20 .3
									  b_binarization, 
									  bVolFilter);	
    return true;
}