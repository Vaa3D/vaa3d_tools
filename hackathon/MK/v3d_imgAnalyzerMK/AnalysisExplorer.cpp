#include <iostream>
#include <string>
#include <cmath>

#include "AnalysisExplorer.h"
#include "ImgProcessor.h"

using namespace std;

void AnalysisExplorer::localCrossCorr(unsigned char testing1D[], unsigned char ref1D[], long int dims[2], int range)
{
	long int testingXlb, testingXhb, testingYlb, testingYhb;
	long int refXlb, refXhb, refYlb, refYhb;
	for (int j = -range; j <= range; ++j)
	{
		for (int i = -range; i <= range; ++i)
		{
			long int croppedSize = (dims[0] - abs(i)) * (dims[1] - abs(j));
			unsigned char* testing1DCropped = new unsigned char[croppedSize];
			unsigned char* ref1DCropped = new unsigned char[croppedSize];
			
			if (i < 0)
			{
				testingXlb = 0 - i + 1; testingXhb = dims[0];
				refXlb = 1; refXhb = dims[0] + i;
			}
			else if (i > 0)
			{
				testingXlb = 1; testingXhb = dims[0] - i;
				refXlb = i + 1; refXhb = dims[0];
			}
			else if (i == 0)
			{
				testingXlb = 1; testingXhb = dims[0];
				refXlb = 1; refXhb = dims[0];
			}

			if (j < 0)
			{
				testingYlb = 0 - j + 1; testingYhb = dims[1];
				refYlb = 1; refYhb = dims[1] + j;
			}
			else if (j > 0)
			{
				testingYlb = 1; testingYhb = dims[1] - j;
				refYlb = j + 1; refYhb = dims[1];
			}
			else if (j == 0)
			{
				testingYlb = 1; testingYhb = dims[1];
				refYlb = 1; refYhb = dims[1];
			}

			ImgProcessor::cropImg2D(testing1D, testing1DCropped, testingXlb, testingXhb, testingYlb, testingYhb, dims[0], dims[1]);
			ImgProcessor::cropImg2D(ref1D, ref1DCropped, refXlb, refXhb, refYlb, refYhb, dims[0], dims[1]);

			double crossCorrK = 0;
			double selfCorrK = 0;
			double crossCorrSum = 0;
			double selfCorrSum = 0;
			for (size_t k = 0; k < croppedSize; ++k)
			{
				crossCorrK = (testing1DCropped[k] / 255) * (ref1DCropped[k] / 255);
				selfCorrK = (ref1DCropped[k] / 255) * (ref1DCropped[k] / 255);
				crossCorrSum = crossCorrSum + crossCorrK;
				selfCorrSum = selfCorrSum + selfCorrK;
			}
			double crossCorrCoeff = crossCorrSum / selfCorrSum;

			this->corrScores.push_back(crossCorrCoeff);

			if (testing1DCropped) { delete[] testing1DCropped; testing1DCropped = 0; }
			if (ref1DCropped) { delete[] ref1DCropped; ref1DCropped = 0; }
		}
	}
}