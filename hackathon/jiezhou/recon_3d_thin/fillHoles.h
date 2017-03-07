#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThinningImageFilter3D.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <queue>
#include <ctime>

void fillHoles(itk::Image<signed short, (unsigned)3> * image) {

	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	int width = size[0];
	int height = size[1];
	int depth = size[2];

	ImageType::IndexType pixelIndex;


	int xe = size[0];
	int ye = size[1];
	int ze = size[2];
	int x, y;
	bool b;

	//pixel 2-d array
	int ** pixel = new int*[xe];
	for (int i = 0; i < xe; i++){
		pixel[i] = new int[ye];
	}
	//int[][] pixel = new int[xe][ye];

	for (int z = 0; z < ze; z++){

		//get original
		for (y = 0; y < ye; y++) {
			for (x = 0; x < xe; x++){
				pixelIndex[0] = x;
				pixelIndex[1] = y;
				pixelIndex[2] = z;
				pixel[x][y] = image->GetPixel(pixelIndex);
				//pixel[x][y] = ip.getPixel(x, y);
			}
		}

		//label background borders
		for (y = 0; y < ye; y++){
			pixelIndex[0] = 0;
			pixelIndex[1] = y;
			pixelIndex[2] = z;
			if (image->GetPixel(pixelIndex) == 0){
				image->SetPixel(pixelIndex, 127);
				//ip.putPixel(0, y, 127);
			}
			pixelIndex[0] = xe -1;
			pixelIndex[1] = y;
			pixelIndex[2] = z;
			if (image->GetPixel(pixelIndex) == 0){
				image->SetPixel(pixelIndex, 127);
				//ip.putPixel(xe - 1, y, 127);
			}
		}

		for (x = 0; x < xe; x++){
			pixelIndex[0] = x;
			pixelIndex[1] = 0;
			pixelIndex[2] = z;
			if (image->GetPixel(pixelIndex) == 0){
				image->SetPixel(pixelIndex, 127);
				//ip.putPixel(x, 0, 127);
			}
			pixelIndex[0] = x;
			pixelIndex[1] = ye - 1;
			pixelIndex[2] = z;
			if (image->GetPixel(pixelIndex) == 0){
				image->SetPixel(pixelIndex, 127);
				//ip.putPixel(x, ye - 1, 127);
			}
		}
		
		//flood background from borders
		//the background of 8-connected particles is 4-connected
		b = true;
		while (b){
			b = false;
			for (y = 1; y < ye - 1; y++) {
				for (x = 1; x < xe - 1; x++) {
					pixelIndex[0] = x;
					pixelIndex[1] = y;
					pixelIndex[2] = z;
					if (image->GetPixel(pixelIndex) == 0){
						pixelIndex[0] = x;
						pixelIndex[1] = y - 1;
						pixelIndex[2] = z;
						if (image->GetPixel(pixelIndex) == 127) {
							pixelIndex[0] = x;
							pixelIndex[1] = y;
							pixelIndex[2] = z;
							image->SetPixel(pixelIndex, 127);
							//ip.putPixel(x, y, 127);
							b = true;
						}
						pixelIndex[0] = x - 1;
						pixelIndex[1] = y;
						pixelIndex[2] = z;
						if (image->GetPixel(pixelIndex) == 127) {
							pixelIndex[0] = x;
							pixelIndex[1] = y;
							pixelIndex[2] = z;
							image->SetPixel(pixelIndex, 127);
							//ip.putPixel(x, y, 127);
							b = true;
						}
					}
				}
			}
			for (y = ye - 2; y >= 1; y--) {
				for (x = xe - 2; x >= 1; x--) {
					pixelIndex[0] = x;
					pixelIndex[1] = y;
					pixelIndex[2] = z;
					if (image->GetPixel(pixelIndex) == 0){
						pixelIndex[0] = x + 1;
						pixelIndex[1] = y;
						pixelIndex[2] = z;
						if (image->GetPixel(pixelIndex) == 127) {
							pixelIndex[0] = x;
							pixelIndex[1] = y;
							pixelIndex[2] = z;
							image->SetPixel(pixelIndex, 127);
							//ip.putPixel(x, y, 127);
							b = true;
						}
						pixelIndex[0] = x;
						pixelIndex[1] = y + 1;
						pixelIndex[2] = z;
						if (image->GetPixel(pixelIndex) == 127) {
							pixelIndex[0] = x;
							pixelIndex[1] = y;
							pixelIndex[2] = z;
							image->SetPixel(pixelIndex, 127);
							//ip.putPixel(x, y, 127);
							b = true;
						}
					}
				}
			}
		}//idempotent

		for (y = 0; y < ye; y++) {
			for (x = 0; x < xe; x++){
				pixelIndex[0] = x;
				pixelIndex[1] = y;
				pixelIndex[2] = z;
				if (image->GetPixel(pixelIndex) == 0){
					pixelIndex[0] = x;
					pixelIndex[1] = y;
					pixelIndex[2] = z;
					image->SetPixel(pixelIndex, 255);
					//ip.putPixel(x, y, 255);
				}
				else{
					pixelIndex[0] = x;
					pixelIndex[1] = y;
					pixelIndex[2] = z;
					image->SetPixel(pixelIndex, pixel[x][y]);
					//ip.putPixel(x, y, pixel[x][y]);
				}
			}
		}

		//return to original state
		//if (!backgroundIsZero)
		//	ip.invert();
	}
}
