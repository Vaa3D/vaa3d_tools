#include <iostream>

#include "NeuronGeoGrapher.h"

using namespace std;

/* ======================================= Vector Geometry ======================================= */
vector<pair<float, float>> NeuronGeoGrapher::getProjectionVector(const vector<pair<float, float>>& axialVector, const vector<pair<float, float>>& projectingVector)
{
	vector<pair<float, float>> projectionVector;

	double axialVectorLength = sqrt(axialVector.begin()->second * axialVector.begin()->second + (axialVector.begin() + 1)->second * (axialVector.begin() + 1)->second + (axialVector.begin() + 2)->second * (axialVector.begin() + 2)->second);

	vector<float> projectingStartVec(3);
	projectingStartVec[0] = projectingVector.begin()->first - axialVector.begin()->first;
	projectingStartVec[1] = (projectingVector.begin() + 1)->first - (axialVector.begin() + 1)->first;
	projectingStartVec[2] = (projectingVector.begin() + 2)->first - (axialVector.begin() + 2)->first;
	vector<float> axialVector_vec(3);
	axialVector_vec[0] = axialVector.begin()->second;
	axialVector_vec[1] = (axialVector.begin() + 1)->second;
	axialVector_vec[2] = (axialVector.begin() + 2)->second;
	double projectingStartVecSine = NeuronGeoGrapher::getVectorSine(axialVector_vec, projectingStartVec);
	double projectingStartVecLength = sqrt(projectingStartVec.at(0) * projectingStartVec.at(0) + projectingStartVec.at(1) * projectingStartVec.at(1) + projectingStartVec.at(2) * projectingStartVec.at(2));
	double ProjectedStartLength = projectingStartVecLength * projectingStartVecSine;
	double projectedStartLengthRatio = ProjectedStartLength / axialVectorLength;
	vector<float> projectedStart(3);
	projectedStart[0] = axialVector.begin()->second * projectedStartLengthRatio + axialVector.begin()->first;
	projectedStart[1] = (axialVector.begin() + 1)->second * projectedStartLengthRatio + (axialVector.begin() + 1)->first;
	projectedStart[2] = (axialVector.begin() + 2)->second * projectedStartLengthRatio + (axialVector.begin() + 2)->first;

	vector<float> projectingVector_vec(3);
	projectingVector_vec[0] = projectingVector.begin()->second;
	projectingVector_vec[1] = (projectingVector.begin() + 1)->second;
	projectingVector_vec[2] = (projectingVector.begin() + 2)->second;
	double projectingVecSine = NeuronGeoGrapher::getVectorSine(axialVector_vec, projectingVector_vec);
	double projectingVecLength = sqrt(projectingVector.begin()->second * projectingVector.begin()->second + (projectingVector.begin() + 1)->second * (projectingVector.begin() + 1)->second + (projectingVector.begin()->second + 2) * (projectingVector.begin()->second + 2));
	double projectedVecLength = projectingVecLength * projectingVecSine;
	double projectedLengthRatio = projectedVecLength / projectingVecLength;
	vector<float> projectedVector_vec(3);
	projectedVector_vec[0] = axialVector.begin()->second * projectedLengthRatio;
	projectedVector_vec[1] = (axialVector.begin() + 1)->second * projectedLengthRatio;
	projectedVector_vec[2] = (axialVector.begin() + 2)->second * projectedLengthRatio;

	projectionVector.push_back(pair<float, float>(projectedStart[0], projectedVector_vec[0]));
	projectionVector.push_back(pair<float, float>(projectedStart[1], projectedVector_vec[1]));
	projectionVector.push_back(pair<float, float>(projectedStart[2], projectedVector_vec[2]));

	return projectionVector;
}
/* =================================== END of [Vector Geometry] =================================== */




/* ======================================= Segment Geometry ======================================= */
double NeuronGeoGrapher::segTurningAngle(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connPointUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = NeuronGeoGrapher::getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongHeadLoc, elongTailLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = NeuronGeoGrapher::getDispUnitVector(connPointLoc, elongHeadLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connHeadNode.x);
		connPointLoc.push_back(connHeadNode.y);
		connPointLoc.push_back(connHeadNode.z * zRATIO);
		connPointUnitVec = NeuronGeoGrapher::getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongTailLoc, elongHeadLoc);
		vector<float> connPointLoc;
		connPointLoc.push_back(connTailNode.x);
		connPointLoc.push_back(connTailNode.y);
		connPointLoc.push_back(connTailNode.z * zRATIO);
		connPointUnitVec = NeuronGeoGrapher::getDispUnitVector(connPointLoc, elongTailLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connPointUnitVec);
		return radAngle;
	}
}

segUnit NeuronGeoGrapher::segmentStraighten(const segUnit& inputSeg)
{
	segUnit outputSeg = inputSeg;

	if (inputSeg.nodes.size() <= 3) return outputSeg;

	cout << " seg ID: " << inputSeg.segID << "   number of nodes contained in this segment: " << inputSeg.nodes.size() << endl;

	double segDistSqr = ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) * ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) +
		((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) * ((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) +
		((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z) * ((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z);
	double segDist = sqrt(segDistSqr);
	cout << "segment displacement: " << segDist << endl << endl;

	for (QList<NeuronSWC>::const_iterator check = inputSeg.nodes.begin(); check != inputSeg.nodes.end(); ++check) cout << "[" << check->x << " " << check->y << " " << check->z << "] ";
	cout << endl;

	vector<profiledNode> pickedNode;
	double dot, sq1, sq2, dist, turnCost, radAngle, turnCostSum = 0, turnCostMean;
	double nodeDeviation, nodeHeadDistSqr, nodeHeadRadAngle, nodeToMainDist, nodeToMainDistSum = 0, nodeToMainDistMean;
	for (QList<NeuronSWC>::const_iterator it = inputSeg.nodes.begin() + 1; it != inputSeg.nodes.end() - 1; ++it)
	{
		dot = ((it - 1)->x - it->x) * ((it + 1)->x - it->x) + ((it - 1)->y - it->y) * ((it + 1)->y - it->y) + ((it - 1)->z - it->z) * ((it + 1)->z - it->z);
		sq1 = ((it - 1)->x - it->x) * ((it - 1)->x - it->x) + ((it - 1)->y - it->y) * ((it - 1)->y - it->y) + ((it - 1)->z - it->z) * ((it - 1)->z - it->z);
		sq2 = ((it + 1)->x - it->x) * ((it + 1)->x - it->x) + ((it + 1)->y - it->y) * ((it + 1)->y - it->y) + ((it + 1)->z - it->z) * ((it + 1)->z - it->z);
		if (isnan(acos(dot / sqrt(sq1 * sq2)))) return outputSeg;
		radAngle = acos(dot / sqrt(sq1 * sq2));

		nodeDeviation = (it->x - inputSeg.nodes.begin()->x) * ((inputSeg.nodes.end() - 1)->x - inputSeg.nodes.begin()->x) +
			(it->y - inputSeg.nodes.begin()->y) * ((inputSeg.nodes.end() - 1)->y - inputSeg.nodes.begin()->y) +
			(it->z - inputSeg.nodes.begin()->z) * ((inputSeg.nodes.end() - 1)->z - inputSeg.nodes.begin()->z);
		nodeHeadDistSqr = (it->x - inputSeg.nodes.begin()->x) * (it->x - inputSeg.nodes.begin()->x) +
			(it->y - inputSeg.nodes.begin()->y) * (it->y - inputSeg.nodes.begin()->y) +
			(it->z - inputSeg.nodes.begin()->z) * (it->z - inputSeg.nodes.begin()->z);
		nodeHeadRadAngle = PI - acos(nodeDeviation / sqrt(segDistSqr * nodeHeadDistSqr));
		nodeToMainDist = sqrt(nodeHeadDistSqr) * sin(nodeHeadRadAngle);
		nodeToMainDistSum = nodeToMainDistSum + nodeToMainDist;
		cout << "       d(node-main):" << nodeToMainDist << " radian/pi:" << (radAngle / PI) << " turning cost:" << (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI) << " " << it->x << " " << it->y << " " << it->z << endl;

		if ((radAngle / PI) < 0.6) // Detecting sharp turns and distance outliers => a) obviously errorneous depth situation
		{
			profiledNode sharp;
			sharp.x = it->x; sharp.y = it->y; sharp.z = it->z;
			cout << "this node is picked" << endl;

			sharp.segID = inputSeg.segID;
			sharp.distToMainRoute = nodeToMainDist;
			sharp.previousSqr = sq1; sharp.nextSqr = sq2; sharp.innerProduct = dot;
			sharp.radAngle = radAngle;
			sharp.index = int(it - inputSeg.nodes.begin());
			sharp.turnCost = (sqrt(sq1) + sqrt(sq2)) / (radAngle / PI);

			pickedNode.push_back(sharp);
			turnCostSum = turnCostSum + sharp.turnCost;
		}
	}
	if (pickedNode.empty()) return outputSeg;

	nodeToMainDistMean = nodeToMainDistSum / (inputSeg.nodes.size() - 2);
	turnCostMean = turnCostSum / pickedNode.size();

	cout << endl << endl << "  ==== start deleting nodes... " << endl;
	int delete_count = 0;
	vector<int> delLocs;
	for (vector<profiledNode>::iterator it = pickedNode.begin(); it != pickedNode.end(); ++it)
	{
		cout << "  Avg(d(node_main)):" << nodeToMainDistMean << " d(node-main):" << it->distToMainRoute << " Avg(turning cost):" << turnCostMean << " turning cost:" << it->turnCost;
		cout << " [" << it->x << " " << it->y << " " << it->z << "] " << endl;
		if (it->distToMainRoute >= nodeToMainDistMean || it->turnCost >= turnCostMean || it->distToMainRoute >= segDist)
		{
			outputSeg.nodes[it->index + 1].parent = -1;
			delLocs.push_back(it->index);
		}
	}
	sort(delLocs.rbegin(), delLocs.rend());
	for (vector<int>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputSeg.nodes.erase(outputSeg.nodes.begin() + ptrdiff_t(*it));
	delLocs.clear();

	cout << endl << "  ==== cheking angles... " << endl;
	int deleteCount2;
	do
	{
		deleteCount2 = 0;
		for (QList<NeuronSWC>::iterator it = outputSeg.nodes.begin() + 1; it != outputSeg.nodes.end() - 1; ++it)
		{
			double dot2 = (it->x - (it - 1)->x) * ((it + 1)->x - it->x) + (it->y - (it - 1)->y) * ((it + 1)->y - it->y) + (it->z - (it - 1)->z) * ((it + 1)->z - it->z);
			double sq1_2 = ((it - 1)->x - it->x) * ((it - 1)->x - it->x) + ((it - 1)->y - it->y) * ((it - 1)->y - it->y) + ((it - 1)->z - it->z) * ((it - 1)->z - it->z);
			double sq2_2 = ((it + 1)->x - it->x) * ((it + 1)->x - it->x) + ((it + 1)->y - it->y) * ((it + 1)->y - it->y) + ((it + 1)->z - it->z) * ((it + 1)->z - it->z);
			if (isnan(acos(dot2 / sqrt(sq1_2 * sq2_2)))) break;
			double radAngle_2 = acos(dot2 / sqrt(sq1_2 * sq2_2));
			cout << "2nd rad Angle:" << radAngle_2 << " [" << it->x << " " << it->y << " " << it->z << "]" << endl;

			if ((radAngle_2 / PI) * 180 > 75)
			{
				if (sqrt(sq1_2) > (1 / 10) * sqrt(sq2_2))
				{
					++deleteCount2;
					cout << "delete " << " [" << it->x << " " << it->y << " " << it->z << "] " << deleteCount2 << endl;
					if ((outputSeg.nodes.size() - deleteCount2) <= 2)
					{
						--deleteCount2;
						break;
					}

					(it + 1)->parent = -1;
					delLocs.push_back(int(it - outputSeg.nodes.begin()));
				}
			}
		}
		sort(delLocs.rbegin(), delLocs.rend());
		for (vector<int>::iterator it = delLocs.begin(); it != delLocs.end(); ++it) outputSeg.nodes.erase(outputSeg.nodes.begin() + ptrdiff_t(*it));
		delLocs.clear();
		cout << "deleted nodes: " << deleteCount2 << "\n=================" << endl;
	} while (deleteCount2 > 0);

	size_t label = 1;
	cout << "number of nodes after straightening process: " << outputSeg.nodes.size() << " ( segID = " << outputSeg.segID << " )" << endl;
	//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
	for (QList<NeuronSWC>::iterator it = outputSeg.nodes.begin() + 1; it != outputSeg.nodes.end() - 1; ++it)
	{
		if (it->parent == -1) it->parent = (it - 1)->n;
	}

	return outputSeg;
}

double NeuronGeoGrapher::segPointingCompare(const segUnit& elongSeg, const segUnit& connSeg, connectOrientation connOrt)
{
	if (elongSeg.tails.size() > 1 || connSeg.tails.size() > 1)
	{
		cerr << "Invalid input: Currently segment elongation only allows to happen between 2 non-branching segments." << endl;
		return -1;
	}

	NeuronSWC elongHeadNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(elongSeg.head)];
	NeuronSWC elongTailNode;
	if (elongSeg.tails.size() == 0)
	{
		//cerr << "Elongating segment only has head. Do nothinig and return.";
		return -1;
	}
	else elongTailNode = elongSeg.nodes[elongSeg.seg_nodeLocMap.at(*elongSeg.tails.begin())];

	NeuronSWC connHeadNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(connSeg.head)];
	NeuronSWC connTailNode;
	if (connSeg.tails.size() == 0)
	{
		if (connOrt == head_tail || connOrt == tail_tail)
		{
			//cerr << "Connecting segment only has head. Do nothing and return";
			return -1;
		}
	}
	else connTailNode = connSeg.nodes[connSeg.seg_nodeLocMap.at(*connSeg.tails.begin())];

	vector<float> elongHeadLoc;
	elongHeadLoc.push_back(elongHeadNode.x);
	elongHeadLoc.push_back(elongHeadNode.y);
	elongHeadLoc.push_back(elongHeadNode.z * zRATIO);

	vector<float> elongTailLoc;
	elongTailLoc.push_back(elongTailNode.x);
	elongTailLoc.push_back(elongTailNode.y);
	elongTailLoc.push_back(elongTailNode.z * zRATIO);

	vector<float> connHeadLoc;
	connHeadLoc.push_back(connHeadNode.x);
	connHeadLoc.push_back(connHeadNode.y);
	connHeadLoc.push_back(connHeadNode.z * zRATIO);

	vector<float> connTailLoc;
	connTailLoc.push_back(connTailNode.x);
	connTailLoc.push_back(connTailNode.y);
	connTailLoc.push_back(connTailNode.z * zRATIO);

	vector<float> elongDispUnitVec;
	vector<float> connDispUnitVec;
	if (connOrt == head_head)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = NeuronGeoGrapher::getDispUnitVector(connTailLoc, connHeadLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == head_tail)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = NeuronGeoGrapher::getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_head)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongHeadLoc, elongTailLoc);
		connDispUnitVec = NeuronGeoGrapher::getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
	else if (connOrt == tail_tail)
	{
		elongDispUnitVec = NeuronGeoGrapher::getDispUnitVector(elongTailLoc, elongHeadLoc);
		connDispUnitVec = NeuronGeoGrapher::getDispUnitVector(connHeadLoc, connTailLoc);

		double radAngle = NeuronGeoGrapher::getPiAngle(elongDispUnitVec, connDispUnitVec);
		return radAngle;
	}
}
/* ================================== END of [Segment Geometry] =================================== */



/* ============================== Polar Coordinate System Operations ============================== */
boost::container::flat_map<double, boost::container::flat_set<int>> NeuronGeoGrapher::getShellByRadius(const vector<polarNeuronSWC>& inputPolarNodeList)
{
	boost::container::flat_map<double, boost::container::flat_set<int>> outputShellMap;
	
	for (vector<polarNeuronSWC>::const_iterator it = inputPolarNodeList.begin(); it != inputPolarNodeList.end(); ++it)
	{
		if (outputShellMap.find(it->radius) != outputShellMap.end()) outputShellMap.at(it->radius).insert(it->ID);
		else
		{
			boost::container::flat_set<int> newSet;
			newSet.insert(it->ID);
			outputShellMap.insert(pair<double, boost::container::flat_set<int>>(it->radius, newSet));
		}
	}

	return outputShellMap;
}
/* ======================== END of [Polar Coordinate System Operations] =========================== */