#ifndef DISTANCETRANSTWO
#define DISTANCETRANSTWO


class DistanceTransTwo
{

public:

	//DistanceTransTwo(bool flag = true);
	DistanceTransTwo(int x, int y, bool flag = true);
	~DistanceTransTwo();
	
	// distanceTrans Algorithm
	// parameter : input output x y
	void euclideanTrans( unsigned char*, unsigned char*, int, int );
	void euclideanTrans( unsigned char*, double*, int, int );
	void chamferTrans( unsigned char*, unsigned char*, int, int );
	void chessboardTrans( unsigned char*, unsigned char*, int, int );

	// parameter : iterator
	void getEuclideanMask( int );
	int getMaskMin();

private:

	int mi_x;
	int mi_y;
	int mai_mask[9];
	int mai_maskcacul[9];
	int* mp_input;
	//int* mp_output;
	double* mp_output;
	unsigned char* mp_output_uc;
	unsigned char* mp_input_uc;
	bool mb_flag;
	bool uc_flag;
};

#endif