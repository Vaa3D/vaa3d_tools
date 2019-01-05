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


	/***************** Object-based Operations *****************/
	static boost::container::flat_map<string, double> objMorphFeature(const connectedComponent& inputSample);
	static connectedComponent getObjSurface(const connectedComponent& inputObj);
	/***********************************************************/


	/**************** Morphological Feature Extraction *****************/
	static double irregularity_voxBased(const connectedComponent& inputSample);
	/*******************************************************************/
};



#endif