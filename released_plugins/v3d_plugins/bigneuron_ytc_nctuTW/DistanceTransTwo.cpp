#include "DistanceTransTwo.h"
#include <cmath>
#include <iostream>
DistanceTransTwo::DistanceTransTwo( int x, int y, bool flag)
{
	mi_x = x;
	mi_y = y;
	
	uc_flag = flag;

	if( !uc_flag )
	{
		mp_input_uc = 0;
		mp_input = new int [ x * y ];
	}
	else
	{
		mp_input = 0;
		mp_input_uc = new unsigned char [ x * y ];
	}
	mp_output = 0;
	mp_output_uc = 0;
	mb_flag = false;
}

DistanceTransTwo::~DistanceTransTwo()
{
	if( mp_input ) delete [] mp_input;
	else delete [] mp_input_uc;
	if( mp_output ) delete [] mp_output;
	else delete [] mp_output_uc;
}



void DistanceTransTwo::getEuclideanMask(int iterator)
{
	mai_mask[0] = mai_mask[2] = mai_mask[6] = mai_mask[8] = -(4*iterator - 2);
	mai_mask[1] = mai_mask[3] = mai_mask[5] = mai_mask[7] = -(2*iterator - 1);
	mai_mask[4] = 0;
}

int DistanceTransTwo::getMaskMin()
{
	int min;
	min = mai_maskcacul[0];
	for(int i = 1; i < 9; i++)
	{
		if(min > mai_maskcacul[i])
		{
			min = mai_maskcacul[i];
		}
	}

	if(min >= 65025)
		min = 65025;

	return min;
}

void DistanceTransTwo::euclideanTrans(unsigned char * input, unsigned char* output, int xSize, int ySize)
{
	//mp_input = new int[ xSize * ySize ];
	//mp_output = new int[ xSize * ySize ];
	if( !uc_flag ) 
		mp_output = new double[ xSize * ySize ];
	else 
		mp_output_uc = new unsigned char [ xSize * ySize ];

	for( int i = 0; i < xSize * ySize; i++ )
	{
		if(input[i] != 0)
		{
			if( !uc_flag ) mp_input[i] = mp_output[i] = 50000;
			else mp_input_uc[i] = mp_output_uc[i] = 255;
		}
		else
		{
			if(!uc_flag)
				mp_input[i] = mp_output[i] = 0;//(int)input[i];
			else
			{
				//std::cout<<i<<' ';
				mp_input_uc[i] = 0;
				mp_output_uc[i] = 0;
			}
		}
	}

	int index;
	int iter = 1;
	mb_flag = false;
	while( !mb_flag )
	{

		mb_flag = true;
		getEuclideanMask( iter );

		for(int i = 1; i < ySize-1; i++)
		{
			for(int j = 1; j < xSize-1; j++)
			{
				index = i*xSize + j;

				if( (!uc_flag && mp_output[index] != 0 ) || (uc_flag && mp_output_uc[index] != 0 ))
				{
					if( !uc_flag )
					{
						mai_maskcacul[0] = mp_output[index - xSize - 1] - mai_mask[0];
						mai_maskcacul[1] = mp_output[index - xSize    ] - mai_mask[1];
						mai_maskcacul[2] = mp_output[index - xSize + 1] - mai_mask[2];
						mai_maskcacul[3] = mp_output[index - 1		  ]	- mai_mask[3];
						mai_maskcacul[4] = mp_output[index			  ]	- mai_mask[4];
						mai_maskcacul[5] = mp_output[index + 1		  ]	- mai_mask[5];
						mai_maskcacul[6] = mp_output[index + xSize - 1] - mai_mask[6];
						mai_maskcacul[7] = mp_output[index + xSize	  ] - mai_mask[7];
						mai_maskcacul[8] = mp_output[index + xSize + 1] - mai_mask[8];
					}
					else
					{
						mai_maskcacul[0] = mp_output_uc[index - xSize - 1] - mai_mask[0];
						mai_maskcacul[1] = mp_output_uc[index - xSize    ] - mai_mask[1];
						mai_maskcacul[2] = mp_output_uc[index - xSize + 1] - mai_mask[2];
						mai_maskcacul[3] = mp_output_uc[index - 1		  ]	- mai_mask[3];
						mai_maskcacul[4] = mp_output_uc[index			  ]	- mai_mask[4];
						mai_maskcacul[5] = mp_output_uc[index + 1		  ]	- mai_mask[5];
						mai_maskcacul[6] = mp_output_uc[index + xSize - 1] - mai_mask[6];
						mai_maskcacul[7] = mp_output_uc[index + xSize	  ] - mai_mask[7];
						mai_maskcacul[8] = mp_output_uc[index + xSize + 1] - mai_mask[8];
					}
					if( ( !uc_flag && mp_input[index] != getMaskMin() ) || (uc_flag && mp_input_uc[index] != getMaskMin() ) )
					{
						mb_flag = false;
						if(!uc_flag) mp_input[index] = getMaskMin();
						else mp_input_uc[index] = getMaskMin();
					}

				}

			} // end for xSize

		} // end for ySize


		if( !mb_flag )
		{
			if( !uc_flag )
			{
				for( int k = 0; k < xSize*ySize; k++ )
				{
					mp_output[k] = mp_input[k];
				}
			}
			else
			{
				for( int k = 0; k < xSize*ySize; k++ )
				{
					mp_output_uc[k] = mp_input_uc[k];
				}
			}
		} // end if

		iter++;

	} // end while

	for( int l = 0; l < xSize*ySize; l++ )
	{
		int a = output[l];
		if( !uc_flag )
		{
			//int b = mp_output[l];
			output[l] = (int) sqrt(mp_output[l]);
		}
		else
		{
			
			output[l] = (unsigned char) sqrt( (double)mp_output_uc[l] );

		}
	}

//	delete [] mp_input;
//	delete [] mp_output;

}


void DistanceTransTwo::euclideanTrans(unsigned char * input, double* output, int xSize, int ySize)
{
	mp_input = new int[ xSize * ySize ];
	//mp_output = new int[ xSize * ySize ];
	mp_output = new double[ xSize * ySize ];

	for( int i = 0; i < xSize * ySize; i++ )
	{
		if(input[i] != 0)
		{
			mp_input[i] = mp_output[i] = 50000;
		}
		else
		{
			mp_input[i] = mp_output[i] = 0;//(int)input[i];
		}
	}

	int index;
	int iter = 1;
	mb_flag = false;
	while( !mb_flag )
	{

		mb_flag = true;
		getEuclideanMask( iter );

		for(int i = 1; i < ySize-1; i++)
		{
			for(int j = 1; j < xSize-1; j++)
			{
				index = i*xSize + j;

				if(mp_output[index] != 0)
				{
					mai_maskcacul[0] = mp_output[index - xSize - 1] - mai_mask[0];
					mai_maskcacul[1] = mp_output[index - xSize    ] - mai_mask[1];
					mai_maskcacul[2] = mp_output[index - xSize + 1] - mai_mask[2];
					mai_maskcacul[3] = mp_output[index - 1		  ]	- mai_mask[3];
					mai_maskcacul[4] = mp_output[index			  ]	- mai_mask[4];
					mai_maskcacul[5] = mp_output[index + 1		  ]	- mai_mask[5];
					mai_maskcacul[6] = mp_output[index + xSize - 1] - mai_mask[6];
					mai_maskcacul[7] = mp_output[index + xSize	  ] - mai_mask[7];
					mai_maskcacul[8] = mp_output[index + xSize + 1] - mai_mask[8];

					if( mp_input[index] != getMaskMin() )
					{
						mb_flag = false;
						mp_input[index] = getMaskMin();
					}

				}

			} // end for xSize

		} // end for ySize

		if( !mb_flag )
		{
			for( int k = 0; k < xSize*ySize; k++ )
			{
				mp_output[k] = mp_input[k];
			}
		} // end if

		iter++;


	} // end while

	for( int l = 0; l < xSize*ySize; l++ )
	{
		int a = output[l];
		int b = mp_output[l];
		//output[l] = (int)System::Math::Sqrt(mp_output[l]);
		output[l] = sqrt( mp_output[l] );
	}

//	delete [] mp_input;
//	delete [] mp_output;

}