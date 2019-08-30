#include "v3d_message.h"
#include <vector>
#include <map>
#include <iostream>
#include <QString>
#include <math.h>
#include "head.h"
#include<fstream>
#include<string>
#include <time.h>
#include "interactive_segmentation_plugin.h"
#include "NeuronEnhancementFilter.h"
#include "NeuronSegmentation.h"
#include "basic_4dimage.h"
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
		/*time_t start,end;
		start=time(NULL);*/
		image_IO(input,output, callback,true);
		/*end=time(NULL);
		cout<<"time use"<<end-start<<endl;*/

	}
	else if (func_name == tr("about"))
	{
		cout<<"about"<<endl;
	}

	else return false;

}

template <class T1, class T2> bool assign_val(T1 *dst, T2 *src, V3DLONG total_num)
{  
	if (!dst || !src || total_num<=0) return false;
	for (V3DLONG i=0; i<total_num; i++){
		if((int)src[i]>30)
		dst[i] = src[i];
		else dst[i] = 0;
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
	if (internal_input) {delete [] internal_input; internal_input=NULL;}
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
	if (internal_input) {delete []internal_input; internal_input=NULL;}
	return res;
}
template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev)
{
	if (!data || n<=0)
		return false;

	int j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
		//printf("len must be at least 2 in mean_and_std\n");
		ave = data[0];
		sdev = (T2)0;
		return true; //do nothing
	}

	s=0.0;
	for (j=0;j<n;j++) s += data[j];
	double ave_double=(T2)(s/n); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(T2)(sqrt(var));
	ave=(T2)ave_double; //use ave_double for the best accuracy

	return true;
}

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
		if (vis) {delete[] vis;	vis = NULL;}
	}
	//copy
	int* vist=0;
	try{
		vist=new int[channel_sz];
	}catch(...){
		cout << __FUNCTION__ << " error malloc" << endl;
		if (vist) {delete[] vist;	vist = NULL;}

	}

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
		if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=NULL;}
		if (m_OutImgData2) {delete []m_OutImgData2; m_OutImgData2=NULL;}
		if (m_OutImgData3) {delete []m_OutImgData3; m_OutImgData3=NULL;}
		return;    
	}

	bool b_binarization = true;

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
		if (m_OutImgData2) {delete []m_OutImgData; m_OutImgData=NULL;}
		return;    
	}

	for(int i=0;i<channel_sz;i++){

		if((int)inimg1d[i]<50)
			m_OutImgData2[i] = 0;

	}

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
				free(Count);
				Count=NULL;
				return ;
			}
			for (int m = 0; m <= flag; m++)
				Count[m] = 0;
			for(int i=0;i<channel_sz;i++ ){
				Count[vis[i]]++;
			}
		
			for(int i=0;i<channel_sz;i++){
				if(Count[vis[i]] <200){
					m_OutImgData2[i] = 0;
				}
			}

			
			cout<<"endl"<<endl;
			/*for(int i=0;i<(flag+1);i++)
			{
			cout<<Count[i]<<endl;
			}*/
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
					if(flags>20){
						if(Count!=NULL){free(Count);Count=NULL;}
						if(vis!=NULL){delete []vis;vis=NULL;}
						if(vist!=NULL){delete []vist;vist=NULL;}
						Image4DSimple  p4DImage2;
						p4DImage2.setData((unsigned char*)m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
						v3dhandle  newwin2;
						newwin2 = callback.newImageWindow();
						callback.setImage(newwin2, &p4DImage2);		
						callback.setImageName(newwin2, QString("Segmentation2"));
						callback.updateImageWindow(newwin2);
						ofstream fmarker(m_InputFileName.toStdString()+".marker");
						fmarker.close();
						return; 
					}
					else{
						if(Count!=NULL){free(Count);Count=NULL;}
						if(vis!=NULL){delete []vis;vis=NULL;}

						try{
							Counts = (int*)malloc(sizeof(int) * (flags+1));
						}catch(...){
							cout << __FUNCTION__ << " error malloc" << endl;
							free(Counts);
							Counts=NULL;
							return ;
						}
						for (int m = 0; m <= flags; m++)
							Counts[m] = 0;
						for(int i=0;i<channel_sz;i++ ){
							Counts[vist[i]]++;
						}
						cout<<"the secend"<<endl;
						cout<<"endlllll"<<endl;
						/*for(int i=0;i<(flags+1);i++)
						{
							cout<<Counts[i]<<endl;
						}*/
						long ax;
						long ay;
						long az;
						int array[3];
						int sum=0;

						ofstream fout(m_InputFileName.toStdString()+".txt");
						for(int i=0;i<(flags+1);i++){
							ax=0;
							ay=0;
							az=0;
							for(int j=0;j<channel_sz;j++){
								if(m_OutImgData2[j]==255&&vist[j]==i){
									getIndex(j,in_sz[0],in_sz[1],in_sz[2],array);
									ax=ax+array[0];
									ay=ay+array[1];
									az=az+array[2];
									sum=Counts[vist[j]];
								}
							}
							if(sum!=0&&ax!=0){
								ax=ax/sum;
								ay=ay/sum;
								az=az/sum;
								fout<<ax<<" "<<ay<<" "<<az<<" ";	
							}
						}

						if(Counts!=NULL){free(Counts);Counts=NULL;}
						if(vist!=NULL){delete []vist;vist=NULL;}
						Image4DSimple  p4DImage2;
						p4DImage2.setData((unsigned char*)m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
						v3dhandle  newwin2;
						newwin2 = callback.newImageWindow();
						callback.setImage(newwin2, &p4DImage2);		
						callback.setImageName(newwin2, QString("Segmentation2"));
						callback.updateImageWindow(newwin2);
						fout.close();
						string path=m_InputFileName.toStdString()+".txt";
						ifstream fin(path);
						int *marker;
						try{
							marker=(int *)malloc (sizeof(int)*flags*3);
						}catch(...){
							cout << __FUNCTION__ << " error malloc" << endl;
							free(marker);
							marker=NULL;
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
						ofstream fmarker(m_InputFileName.toStdString()+".marker");
						for(int i=0;i<line;i++)
							fmarker<<marker[3*i]<<","<<marker[3*i+1]<<","<<marker[3*i+2]<<","<<3<<","<<0<<","<<0<<","<<0<<","<<125<<","<<0<<","<<0<<endl;
						fmarker.close();
						if(marker!=NULL){
							free(marker);
							marker=NULL;
						}
						return; 
					}
}

void image_IO(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback,bool b_binarization){
	vector<char*> infiles,inparas;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	QString m_InputfolderName = infiles.at(0);
	QString savePath = infiles.at(1);
	QString logPath = infiles.at(2);
	QString length1 = inparas.at(0);
	QString length2 = inparas.at(1);
	QString thread = inparas.at(2);
	int intensity=length1.toInt();
	int totalcount=length2.toInt();
	int threadno=thread.toInt();

	cout<<"intensity： "<<intensity<<" totalcount: "<<totalcount<<endl;
	V3DLONG *in_zz;
	if(!callback.getDimTeraFly(m_InputfolderName.toStdString(), in_zz))
	{
		cout<<"the terafly is wrong"<<endl;

	}
	int xs=in_zz[0];
	int ys=in_zz[1];
	int zs=in_zz[2];
	int Cx=xs/512;
	int Cy=ys/512;
	int Cz=zs/512;
	cout<<xs<<"  "<<ys<<" "<<zs<<endl;
	time_t starttime,endtime;
	int *xyzlist=(int*)malloc(sizeof(int)*Cx*Cy*Cz*3);
	int xyzcount=0;
	for(int i=0;i<Cz;i++){
		for(int j=0;j<Cy;j++){
			for(int k=0;k<Cx;k++){
				int px=0,py=0,pz=0;
				px=256+512*k;
				py=256+512*j;
				pz=256+512*i;
				xyzlist[xyzcount]=px;
				xyzlist[xyzcount+1]=py;
				xyzlist[xyzcount+2]=pz;
				xyzcount=xyzcount+3;

			}
		}
	}

	starttime=time(NULL);
	int order=0;
	omp_set_num_threads(threadno);
#pragma omp parallel for
	for(int i=0;i<Cx*Cy*Cz;i++)
	{	
		int px=0,py=0,pz=0;
		px=xyzlist[3*i];
		py=xyzlist[3*i+1];
		pz=xyzlist[3*i+2];
		ofstream log(logPath.toStdString(),ios::app);
		log<<"order "<<order++<<endl;
		log.close(); 
		unsigned char * cropped_image = 0;
		int state=1;
		try{
			cropped_image = callback.getSubVolumeTeraFly(m_InputfolderName.toStdString(),
				(px-256), (px+256),
				(py-256), (py+256),
				(pz-256), (pz+256)); 
		}
		catch(...){
			//v3d_msg("the image is broke");
			state=0;
		}
		if(cropped_image==NULL)
			state=0;
		if(state==0){
			continue;
		}
		else{
			V3DLONG in_sz[4]; int datatype;
			in_sz[0]=512;
			in_sz[1]=512;
			in_sz[2]=512;
			in_sz[3]=in_zz[3];
			datatype=1;
			V3DLONG channel_sz = in_sz[0]*in_sz[1]*in_sz[2];
			int iVesCnt = 1;
			bool res;
			unsigned char * m_OutImgData = 0;
			unsigned char * m_OutImgData2 = 0;
			try
			{
				m_OutImgData = new  unsigned char[channel_sz]; 
				m_OutImgData2 = new unsigned char[channel_sz];

			}
			catch (...)
			{
				printf("Fail to allocate memory in Neuron_segment_entry_func().\n");
				if (m_OutImgData) {delete []m_OutImgData; m_OutImgData=NULL;}
				if (m_OutImgData2) {delete []m_OutImgData2; m_OutImgData2=NULL;}

			}
			switch (datatype)
			{
			case 1:
				{
					res = do_seg2(cropped_image, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
					break;}
			case 2:
				{res = do_seg2((short int *)cropped_image, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);        
				break;}
			case 4:
				{res = do_seg2((float *)cropped_image, m_OutImgData2, in_sz[0], in_sz[1], in_sz[2], iVesCnt, b_binarization);
				break;}
			default:
				break;
			}
			if(!res)
			{
				printf("The segmentation of foreground fails.\n");
				if (m_OutImgData2) {delete []m_OutImgData; m_OutImgData=NULL;}

			}
			for(int i=0;i<channel_sz;i++){
				if((int)cropped_image[i]<intensity)
					m_OutImgData2[i] = 0;
			}
			if(cropped_image!=NULL){delete []cropped_image;cropped_image=NULL;}
			int* vis=0;
			try{
				vis=new int[channel_sz];
			}catch(...){
				cout << __FUNCTION__ << " error malloc" << endl;
				if(vis) {delete [] vis;vis=NULL;}
			}
			int* vist=0;
			try{
				vist=new int[channel_sz];
			}catch(...){
				cout << __FUNCTION__ << " error malloc" << endl;
				if(vist) {delete [] vist;vist=NULL;}
			}
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
					printf("flag:%d\n",flag);
					try{
						Count = (int*)malloc(sizeof(int) * (flag+1));
					}catch(...){
						cout << __FUNCTION__ << " error malloc" << endl;
						free(Count);

					}
					for (int m = 0; m <= flag; m++)
						Count[m] = 0;
					for(int i=0;i<channel_sz;i++ ){
						Count[vis[i]]++;
					}

					for(int i=0;i<channel_sz;i++){
						if(Count[vis[i]] <totalcount){
							m_OutImgData2[i] = 0;
						}
					}
					if(vis!=NULL){delete []vis;vis=NULL;}
					for(int i=0;i<(flag+1);i++)
					{
						printf("%d\n",Count[i]);
					}
					int flags = 0;
					for (int i = 0; i < in_sz[0]; i++)
						for (int j = 0; j < in_sz[1]; j++)
							for (int k = 0; k < in_sz[2]; k++) {
								if (m_OutImgData2[getNum(i, j, k,in_sz[0],in_sz[1],in_sz[2])] == 255 && !vist[getNum(i,j,k,in_sz[0],in_sz[1],in_sz[2])]) {
									flags++;
									bfs(m_OutImgData2,in_sz[0],in_sz[1],in_sz[2], i, j, k ,flags,vist);
								}
							}
							printf("flags: %d\n",flags);
							if(flags>20){
								if(vist!=NULL){delete []vist;vist=NULL;}
								if(m_OutImgData!=NULL){delete []m_OutImgData;m_OutImgData=NULL;}
								if(m_OutImgData2!=NULL){delete []m_OutImgData2;m_OutImgData2=NULL;}
								if(Count!=NULL){free(Count);Count=NULL;}

							}
							else{
								if(Count!=NULL){free(Count);Count=NULL;}	
								int *Counts;
								try{
									Counts = (int*)malloc(sizeof(int) * (flags+1));
								}catch(...){
									cout << __FUNCTION__ << " error malloc" << endl;
									free(Counts);
									Counts=NULL;

								}
								for (int m = 0; m <= flags; m++)
									Counts[m] = 0;
								for(int i=0;i<channel_sz;i++ ){
									Counts[vist[i]]++;
								}

								for(int i=0;i<(flags+1);i++)
								{
									printf("%d\n",Counts[i]);
								}
								long ax;
								long ay;
								long az;
								int array[3];
								int sum=0;

								ofstream fout(savePath.toStdString(),ios::app);
								for(int i=0;i<(flags+1);i++){
									ax=0;
									ay=0;
									az=0;
									for(int j=0;j<channel_sz;j++){
										if(m_OutImgData2[j]==255&&vist[j]==i){
											getIndex(j,in_sz[0],in_sz[1],in_sz[2],array);
											ax=ax+array[0];
											ay=ay+array[1];
											az=az+array[2];
											sum=Counts[vist[j]];
										}
									}
									if(sum!=0&&ax!=0){
										ax=ax/sum;
										ay=ay/sum;
										az=az/sum;
										fout<<px<<","<<py<<","<<pz<<","<<ax+px-256<<","<<ay+py-256<<","<<az+pz-256<<","<<sum<<endl;	
									}

								}
								fout.close();
								if(vist!=NULL){delete []vist;vist=NULL;}
								if(m_OutImgData!=NULL){delete [] m_OutImgData;m_OutImgData=NULL;}
								if(m_OutImgData2!=NULL){delete []m_OutImgData2;m_OutImgData2=NULL;}
								if(Counts!=NULL){free(Counts);Counts=NULL;}

							}

		}


	}	
	endtime=time(NULL);
	ofstream log(logPath.toStdString(),ios::app);
	log<<"totaltime: "<<endtime-starttime<<endl;
	log.close(); 
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
