#include <v3d_utils.h>

//
//statistics of count of labeling
class STCL
{
public:
	STCL(){}
	~STCL(){}
	
public:
	int count;
	int label;
};

#define NO_OBJECT 0


static int find( int set[], int x )
{
    int r = x;
    while ( set[r] != r )
        r = set[r];
    return r;
}

void v3d_utils::rescaleForDisplay(double* input, double* output, V3DLONG numVoxels, ImagePixelType dataType)
{
	int maxValType;
	switch (dataType)
	{
		case V3D_UINT8:
    		maxValType = 255;
			break;
    	case V3D_UINT16:
			maxValType = 65535;
			break;
    	case V3D_FLOAT32:
    		maxValType = 3.402823466E+38;
			break;
    	case V3D_UNKNOWN:
    	default:
			maxValType = 255;
	}
	
	double max = 0;
	double min = 0;
	for (int i = 0; i < numVoxels; i++)
	{
 		max = (max < input[i]) ? input[i] : max;	
 		min = (min > input[i]) ? input[i] : min;	
	}
	for (int i = 0; i < numVoxels; i++) {
		output[i] = round(maxValType * (input[i] - min) / (max - min));
	}
}


unsigned char* v3d_utils::doubleArrayToCharArray(double* data1dD, V3DLONG numVoxels, ImagePixelType dataType)
{
  	unsigned char* dataOut1d;
    
	// output the processed image
    switch(dataType)
    {
    	case V3D_UINT8:
    	{
			dataOut1d = new unsigned char[numVoxels];
			uint8* tmp1 = (uint8*) dataOut1d;
			for(V3DLONG i = 0; i < numVoxels; i++)
			{
				tmp1[i] = (uint8) round(data1dD[i]);
			}
    	}
			break;
    	case V3D_UINT16:
    	{
			dataOut1d = new unsigned char[2*numVoxels];
			uint16* tmp2 = (uint16*) dataOut1d;
			for(V3DLONG i = 0; i < numVoxels; i++)
			{
				tmp2[i] = (uint16) round(data1dD[i]);
			}
    	}
			break;
    	case V3D_FLOAT32:
    	{
			dataOut1d = new unsigned char[4*numVoxels];
			float32* tmp3 = (float32*) dataOut1d;
			for(V3DLONG i = 0; i < numVoxels; i++)
			{
				tmp3[i] = (float32) data1dD[i];
			}
    	}
			break;
    	case V3D_UNKNOWN:
    	default:
			return NULL;
    }
    
    return dataOut1d;
}
double* v3d_utils::channelToDoubleArray(Image4DSimple* inputImage, int channel)
{	
    unsigned char* data1d = inputImage->getRawData();//get raw data
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = inputImage->getTotalUnitNumberPerChannel();
	//get dims
    V3DLONG szx = inputImage->getXDim();
    V3DLONG szy = inputImage->getYDim();
    V3DLONG szz = inputImage->getZDim();
    V3DLONG sc = inputImage->getCDim();
    V3DLONG N = szx * szy * szz;
	
	//convert first channel to double values
    double* data1dD = new double[N]; //(double*)malloc(sizeof(double)*N*M*P);
	int offset = (channel-1)*N;
	switch(inputImage->getDatatype())
    {
    	case V3D_UINT8:
    	{
			uint8* tmp = (uint8*) data1d;
			for(V3DLONG i = 0; i < pagesz; i++)
			{
				data1dD[i] = tmp[i+offset];
			}
    	}
			break;
    	case V3D_UINT16:
    	{
			uint16* tmp1 = (uint16*) data1d;
			for(V3DLONG i = 0; i < pagesz; i++)
			{
				data1dD[i] = tmp1[i+offset];
			}
    	}
			break;
    	case V3D_FLOAT32:
    	{
			float32* tmp2 = (float32*) data1d;
			for(V3DLONG i = 0; i < pagesz; i++)
			{
				data1dD[i] = tmp2[i+offset];
			}
    	}
			break;
    	case V3D_UNKNOWN:
    	default:
			free(data1dD);
			return NULL;
    }
    return data1dD;
}

LandmarkList v3d_utils::getConnectedComponents2D(double* data1dD, V3DLONG szx, V3DLONG szy, V3DLONG szz, double threshold)
{
	V3DLONG N = szx * szy * szz;
	unsigned char *bw = new unsigned char [N];
	unsigned int *L = new unsigned int [N];
	
	for(V3DLONG i=0; i<N; i++)
	{
		bw[i] = (data1dD[i]>threshold)?1:0;
		L[i] = 0;
	}
	
	V3DLONG offset_y = szx, offset_z = szx*szy;
//	V3DLONG neighborhood_13[13] = {-1, -offset_y, -offset_z,
//		-offset_y-1, -offset_y-offset_z, 
//		offset_y-1, offset_y-offset_z,
//		offset_z-1, -offset_z-1,
//		-1-offset_y-offset_z, -1+offset_y-offset_z,
//		1-offset_y-offset_z, 1+offset_y-offset_z}; 

	V3DLONG neighborhood[4] = {-1, -offset_y, -offset_y-1, offset_y-1}; 
	
	// other variables
	int *lset = new int [N];   // label table/tree
	int ntable;                // number of elements in the component table/tree
	
	ntable = 0;
	lset[0] = 0;
	
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				if(i==0 || i==szx-1 || j==0 || j==szy-1 )
					continue;
				
				// find connected components
				if(bw[idx]) // if there is an object 
				{
					int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13;
					
					n1  = find(lset, (int)L[idx + neighborhood[0] ]);
					n2  = find(lset, (int)L[idx + neighborhood[1] ]);
					n3  = find(lset, (int)L[idx + neighborhood[2] ]);
					n4  = find(lset, (int)L[idx + neighborhood[3] ]);
//					n5  = find(lset, (int)L[idx + neighborhood_13[4] ]);
//					n6  = find(lset, (int)L[idx + neighborhood_13[5] ]);
//					n7  = find(lset, (int)L[idx + neighborhood_13[6] ]);
//					n8  = find(lset, (int)L[idx + neighborhood_13[7] ]);
//					n9  = find(lset, (int)L[idx + neighborhood_13[8] ]);
//					n10 = find(lset, (int)L[idx + neighborhood_13[9] ]);
//					n11 = find(lset, (int)L[idx + neighborhood_13[10] ]);
//					n12 = find(lset, (int)L[idx + neighborhood_13[11] ]);
//					n13 = find(lset, (int)L[idx + neighborhood_13[12] ]);
					
					if(n1 || n2 || n3 || n4 )
					{
						int tlabel;
						
						if(n1) tlabel = n1;
						else if(n2) tlabel = n2;
						else if(n3) tlabel = n3;
						else if(n4) tlabel = n4;
//						else if(n5) tlabel = n5;
//						else if(n6) tlabel = n6;
//						else if(n7) tlabel = n7;
//						else if(n8) tlabel = n8;
//						else if(n9) tlabel = n9;
//						else if(n10) tlabel = n10;
//						else if(n11) tlabel = n11;
//						else if(n12) tlabel = n12;
//						else if(n13) tlabel = n13;
						
						L[idx] = tlabel;
						
						if(n1 && n1 != tlabel) lset[n1] = tlabel;
						if(n2 && n2 != tlabel) lset[n2] = tlabel;
						if(n3 && n3 != tlabel) lset[n3] = tlabel;
						if(n4 && n4 != tlabel) lset[n4] = tlabel;
//						if(n5 && n5 != tlabel) lset[n5] = tlabel;
//						if(n6 && n6 != tlabel) lset[n6] = tlabel;
//						if(n7 && n7 != tlabel) lset[n7] = tlabel;
//						if(n8 && n8 != tlabel) lset[n8] = tlabel;
//						if(n9 && n9 != tlabel) lset[n9] = tlabel;
//						if(n10 && n10 != tlabel) lset[n10] = tlabel;
//						if(n11 && n11 != tlabel) lset[n11] = tlabel;
//						if(n12 && n12 != tlabel) lset[n12] = tlabel;
//						if(n13 && n13 != tlabel) lset[n13] = tlabel;
					}
					else
					{
						ntable++;
						L[idx] = lset[ntable] = ntable;
					}
				}
				else
				{
					L[idx] = 0; //NO_OBJECT;
				}
			}
		}
	}
	
	// consolidate component table
	for(V3DLONG i = 0; i <= ntable; i++)
		lset[i] = find( lset, i );
	
	// run image through the look-up table
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
	
	// count up the objects in the image
	for( int i = 0; i <= ntable; i++ )
		lset[i] = 0;
	
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				lset[ (int)L[idx] ]++;
			}
		}
	}
	
	// number the objects from 1 through n objects
	int nobj = 0;
	lset[0] = 0;
	for( int i = 1; i <= ntable; i++ )
		if ( lset[i] > 0 )
			lset[i] = ++nobj;
	
	printf("############## how many objects found ...%d\n",nobj);
	
	// run through the look-up table again
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
	
	// visualize L 
	int max_L=0;
	for(long i=0; i<N; i++)
	{
		if(max_L<L[i]) max_L = L[i];
	}
	
	if(max_L)
	{
		if(max_L<256)
		{
			for(long i=0; i<N; i++)
			{
				//data1dD[i] = L[i];  //change to bw
				bw[i] = L[i];
			}
			
		}
		else
		{
			for(long i=0; i<N; i++)
			{
				bw[i] = 255*(L[i])/max_L;
				//data1dD[i] = 255*(L[i])/max_L;  //change to bw
			}
		}
		
	}
	
	// find the first N biggest regions 
	std::vector<STCL> labelList;
	// histogram of L
	int *a = new int [nobj+1];
	for(long i=0;  i<=nobj; i++)
	{
		a[i] = 0;
	}
	
	for(long i=0; i<N; i++)
	{
		a[ L[i] ] ++;
	}
	//
	//int np = fmin(5, nobj);
	int np = nobj;
	for(int i=1;  i<=nobj; i++) // 0 is background
	{
		
		STCL s;
		
		s.count = a[i];
		s.label = i;
		
		//
		if(labelList.size()<1)
			labelList.push_back(s);
		else
		{
			for(unsigned int it=labelList.size(); it!=0; it--)
			{
				if(s.count<=labelList.at(it-1).count)
				{
					labelList.insert(labelList.begin() + it, 1, s);
					
					if(labelList.size()>np) // pick 5 points
						labelList.erase(labelList.end());
					
					break;
				}
				else
					continue;
				
			}
			
			//
			if(s.count>labelList.at(0).count && labelList.size()<np) // pick 5 points
				labelList.insert(labelList.begin(), s);
		}
		
		
	}
	
	
	LandmarkList cmList;
	
	bool flag_l = new bool [np];
	
	for(int i_n = 0; i_n<np; i_n++)
	{
		float scx=0,scy=0,scz=0,si=0;
		
		int label=labelList.at(i_n).label;
		
		for(long k = 0; k < szz; k++) 
		{				
			long idxk = k*offset_z;
			for(long j = 0;  j < szy; j++) 
			{
				long idxj = idxk + j*offset_y;
				
				for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
				{
					
					//
					if(L[idx]==label)
					{
						float cv = data1dD[ idx ];
						
						scz += k*cv;
						scy += j*cv;
						scx += i*cv;
						si += cv;
					}
					
					
				}
			}
		}
		
		//
		if (si>0)
		{
			long ncx = scx/si + 0.5 +1; 
			long ncy = scy/si + 0.5 +1; 
			long ncz = scz/si + 0.5 +1;
			
			// qDebug() << "position ..." << ncx << ncy << ncz;
			
			LocationSimple pp(ncx, ncy, ncz);
			//pp.color = random_rgba8();
			//pp.radius = 10;
			//pp.shape = pxCube;
			cmList.push_back(pp);
			
		}
		
	}
	return cmList;
}


LandmarkList v3d_utils::getConnectedComponents(double* data1dD, V3DLONG szx, V3DLONG szy, V3DLONG szz, double threshold)
{
	V3DLONG N = szx * szy * szz;
	unsigned char *bw = new unsigned char [N];
	unsigned int *L = new unsigned int [N];
	
	for(V3DLONG i=0; i<N; i++)
	{
		bw[i] = (data1dD[i]>threshold)?1:0;
		L[i] = 0;
	}
	
	V3DLONG offset_y = szx, offset_z = szx*szy;
	V3DLONG neighborhood_13[13] = {-1, -offset_y, -offset_z,
		-offset_y-1, -offset_y-offset_z, 
		offset_y-1, offset_y-offset_z,
		offset_z-1, -offset_z-1,
		-1-offset_y-offset_z, -1+offset_y-offset_z,
		1-offset_y-offset_z, 1+offset_y-offset_z}; 
	
	// other variables
	int *lset = new int [N];   // label table/tree
	int ntable;                // number of elements in the component table/tree
	
	ntable = 0;
	lset[0] = 0;
	
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				if(i==0 || i==szx-1 || j==0 || j==szy-1 || k==0 || k==szz-1)
					continue;
				
				// find connected components
				if(bw[idx]) // if there is an object 
				{
					int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13;
					
					n1  = find(lset, (int)L[idx + neighborhood_13[0] ]);
					n2  = find(lset, (int)L[idx + neighborhood_13[1] ]);
					n3  = find(lset, (int)L[idx + neighborhood_13[2] ]);
					n4  = find(lset, (int)L[idx + neighborhood_13[3] ]);
					n5  = find(lset, (int)L[idx + neighborhood_13[4] ]);
					n6  = find(lset, (int)L[idx + neighborhood_13[5] ]);
					n7  = find(lset, (int)L[idx + neighborhood_13[6] ]);
					n8  = find(lset, (int)L[idx + neighborhood_13[7] ]);
					n9  = find(lset, (int)L[idx + neighborhood_13[8] ]);
					n10 = find(lset, (int)L[idx + neighborhood_13[9] ]);
					n11 = find(lset, (int)L[idx + neighborhood_13[10] ]);
					n12 = find(lset, (int)L[idx + neighborhood_13[11] ]);
					n13 = find(lset, (int)L[idx + neighborhood_13[12] ]);
					
					if(n1 || n2 || n3 || n4 || n5 || n6 || n7 || n8 || n9 || n10 || n11 || n12 || n13)
					{
						int tlabel;
						
						if(n1) tlabel = n1;
						else if(n2) tlabel = n2;
						else if(n3) tlabel = n3;
						else if(n4) tlabel = n4;
						else if(n5) tlabel = n5;
						else if(n6) tlabel = n6;
						else if(n7) tlabel = n7;
						else if(n8) tlabel = n8;
						else if(n9) tlabel = n9;
						else if(n10) tlabel = n10;
						else if(n11) tlabel = n11;
						else if(n12) tlabel = n12;
						else if(n13) tlabel = n13;
						
						L[idx] = tlabel;
						
						if(n1 && n1 != tlabel) lset[n1] = tlabel;
						if(n2 && n2 != tlabel) lset[n2] = tlabel;
						if(n3 && n3 != tlabel) lset[n3] = tlabel;
						if(n4 && n4 != tlabel) lset[n4] = tlabel;
						if(n5 && n5 != tlabel) lset[n5] = tlabel;
						if(n6 && n6 != tlabel) lset[n6] = tlabel;
						if(n7 && n7 != tlabel) lset[n7] = tlabel;
						if(n8 && n8 != tlabel) lset[n8] = tlabel;
						if(n9 && n9 != tlabel) lset[n9] = tlabel;
						if(n10 && n10 != tlabel) lset[n10] = tlabel;
						if(n11 && n11 != tlabel) lset[n11] = tlabel;
						if(n12 && n12 != tlabel) lset[n12] = tlabel;
						if(n13 && n13 != tlabel) lset[n13] = tlabel;
					}
					else
					{
						ntable++;
						L[idx] = lset[ntable] = ntable;
					}
				}
				else
				{
					L[idx] = 0; //NO_OBJECT;
				}
			}
		}
	}
	
	// consolidate component table
	for(V3DLONG i = 0; i <= ntable; i++)
		lset[i] = find( lset, i );
	
	// run image through the look-up table
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
	
	// count up the objects in the image
	for( int i = 0; i <= ntable; i++ )
		lset[i] = 0;
	
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				lset[ (int)L[idx] ]++;
			}
		}
	}
	
	// number the objects from 1 through n objects
	int nobj = 0;
	lset[0] = 0;
	for( int i = 1; i <= ntable; i++ )
		if ( lset[i] > 0 )
			lset[i] = ++nobj;
	
	printf("############## how many objects found ...%d\n",nobj);
	
	// run through the look-up table again
	for(long k = 0; k < szz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < szy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
	
	// visualize L 
	int max_L=0;
	for(long i=0; i<N; i++)
	{
		if(max_L<L[i]) max_L = L[i];
	}
	
	if(max_L)
	{
		if(max_L<256)
		{
			for(long i=0; i<N; i++)
			{
				//data1dD[i] = L[i];  //change to bw
				bw[i] = L[i];
			}
			
		}
		else
		{
			for(long i=0; i<N; i++)
			{
				bw[i] = 255*(L[i])/max_L;
				//data1dD[i] = 255*(L[i])/max_L;  //change to bw
			}
		}
		
	}
	
	// find the first N biggest regions 
	std::vector<STCL> labelList;
	// histogram of L
	int *a = new int [nobj+1];
	for(long i=0;  i<=nobj; i++)
	{
		a[i] = 0;
	}
	
	for(long i=0; i<N; i++)
	{
		a[ L[i] ] ++;
	}
	//
	//int np = fmin(5, nobj);
	int np = nobj;
	for(int i=1;  i<=nobj; i++) // 0 is background
	{
		
		STCL s;
		
		s.count = a[i];
		s.label = i;
		
		//
		if(labelList.size()<1)
			labelList.push_back(s);
		else
		{
			for(unsigned int it=labelList.size(); it!=0; it--)
			{
				if(s.count<=labelList.at(it-1).count)
				{
					labelList.insert(labelList.begin() + it, 1, s);
					
					if(labelList.size()>np) // pick 5 points
						labelList.erase(labelList.end());
					
					break;
				}
				else
					continue;
				
			}
			
			//
			if(s.count>labelList.at(0).count && labelList.size()<np) // pick 5 points
				labelList.insert(labelList.begin(), s);
		}
		
		
	}
	
	
	LandmarkList cmList;
	
	bool flag_l = new bool [np];
	
	for(int i_n = 0; i_n<np; i_n++)
	{
		float scx=0,scy=0,scz=0,si=0;
		
		int label=labelList.at(i_n).label;
		
		for(long k = 0; k < szz; k++) 
		{				
			long idxk = k*offset_z;
			for(long j = 0;  j < szy; j++) 
			{
				long idxj = idxk + j*offset_y;
				
				for(long i = 0, idx = idxj; i < szx;  i++, idx++) 
				{
					
					//
					if(L[idx]==label)
					{
						float cv = data1dD[ idx ];
						
						scz += k*cv;
						scy += j*cv;
						scx += i*cv;
						si += cv;
					}
					
					
				}
			}
		}
		
		//
		if (si>0)
		{
			long ncx = scx/si + 0.5 +1; 
			long ncy = scy/si + 0.5 +1; 
			long ncz = scz/si + 0.5 +1;
			
			// qDebug() << "position ..." << ncx << ncy << ncz;
			
			LocationSimple pp(ncx, ncy, ncz);
			//pp.color = random_rgba8();
			//pp.radius = 10;
			//pp.shape = pxCube;
			cmList.push_back(pp);
			
		}
		
	}
	return cmList;
}

