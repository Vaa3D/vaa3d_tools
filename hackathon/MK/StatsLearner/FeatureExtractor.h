#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <iostream>
#include <vector>
#include <string>

#include <boost/container/flat_map.hpp>

#include "ImgAnalyzer.h"

using namespace std;

class FeatureExtractor
{
public:
	/***************** Constructors and Basic Data/Function Members *****************/
	enum featureType { morphology, texture };
	/********************************************************************************/


	/***************** Object-based Feature Extraction *****************/
	static boost::container::flat_map<string, double> objMorphFeature(const connectedComponent& inputSample);
	static connectedComponent getSurfaceObj(const connectedComponent& inputObj);
	/*******************************************************************/
};



#endif