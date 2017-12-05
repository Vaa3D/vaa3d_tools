#include <iostream>

#include <qlist.h>
#include <qstringlist.h>

#include "NeuronStructNavigator.h"

using namespace std;

NeuronStructNavigator::NeuronStructNavigator(int xRadius, int yRadius, int zRadius, int imgX, int imgY, int imgZ)
{
	this->uiXradius = xRadius;
	this->uiYradius = yRadius;
	this->uiZradius = zRadius;
	this->stackX = imgX;
	this->stackY = imgY;
	this->stackZ = imgZ;
}

void NeuronStructNavigator::generateNodeQueue()
{
	QStringList fileNameParse = neuronStructFileName.split(".");

	if (*(fileNameParse.end() - 1) == "swc")
	{
		NeuronTree inputSWC = readSWC_file(this->neuronStructFileName);
		for (QList<NeuronSWC>::iterator it = inputSWC.listNeuron.begin(); it != inputSWC.listNeuron.end(); ++it)
		{
			nodeInfo currNode;
			currNode.x_coord = it->x;	currNode.nameX = int(it->x);
			currNode.y_coord = it->y;	currNode.nameY = int(it->y);
			currNode.z_coord = it->z;	currNode.nameZ = int(it->z);
			currNode.type = it->type;

			currNode.xlb = currNode.x_coord - this->uiXradius;
			currNode.xhb = currNode.x_coord + this->uiXradius;
			currNode.ylb = currNode.y_coord - this->uiYradius;
			currNode.yhb = currNode.y_coord + this->uiYradius;
			currNode.zlb = currNode.z_coord - this->uiZradius;
			currNode.zhb = currNode.z_coord + this->uiZradius;
			if (currNode.xlb < 1) currNode.xlb = 1;
			if (currNode.xhb > this->stackX) currNode.xhb = this->stackX;
			if (currNode.ylb < 1) currNode.ylb = 1;
			if (currNode.yhb > this->stackY) currNode.yhb = this->stackY;
			if (currNode.zlb < 1) currNode.zlb = 1;
			if (currNode.zhb > this->stackZ) currNode.zhb = this->stackZ;

			this->nodeQueue.push(currNode);
		}
	}
	else if (*(fileNameParse.end() - 1) == "apo")
	{
		QList<CellAPO> apoList = readAPO_file(this->neuronStructFileName);
		for (QList<CellAPO>::iterator it = apoList.begin(); it != apoList.end(); ++it)
		{
			nodeInfo currNode;
			currNode.x_coord = it->x;	currNode.nameX = int(it->x);
			currNode.y_coord = it->y;	currNode.nameY = int(it->y);
			currNode.z_coord = it->z;	currNode.nameZ = int(it->z);

			currNode.xlb = currNode.x_coord - this->uiXradius;
			currNode.xhb = currNode.x_coord + this->uiXradius;
			currNode.ylb = currNode.y_coord - this->uiYradius;
			currNode.yhb = currNode.y_coord + this->uiYradius;
			currNode.zlb = currNode.z_coord - this->uiZradius;
			currNode.zhb = currNode.z_coord + this->uiZradius;
			if (currNode.xlb < 1) currNode.xlb = 1;
			if (currNode.xhb > this->stackX) currNode.xhb = this->stackX;
			if (currNode.ylb < 1) currNode.ylb = 1;
			if (currNode.yhb > this->stackY) currNode.yhb = this->stackY;
			if (currNode.zlb < 1) currNode.zlb = 1;
			if (currNode.zhb > this->stackZ) currNode.zhb = this->stackZ;

			this->nodeQueue.push(currNode);
		}
	}
	else if (*(fileNameParse.end() - 1) == "marker")
	{
		QList<ImageMarker> markerList = readMarker_file(this->neuronStructFileName);
		for (QList<ImageMarker>::iterator it = markerList.begin(); it != markerList.end(); ++it)
		{
			nodeInfo currNode;
			currNode.x_coord = it->x - 1;	currNode.nameX = int(it->x - 1);
			currNode.y_coord = it->y - 1;	currNode.nameY = int(it->y - 1);
			currNode.z_coord = it->z - 1;	currNode.nameZ = int(it->z - 1);

			currNode.xlb = currNode.x_coord - this->uiXradius;
			currNode.xhb = currNode.x_coord + this->uiXradius;
			currNode.ylb = currNode.y_coord - this->uiYradius;
			currNode.yhb = currNode.y_coord + this->uiYradius;
			currNode.zlb = currNode.z_coord - this->uiZradius;
			currNode.zhb = currNode.z_coord + this->uiZradius;
			if (currNode.xlb < 1) currNode.xlb = 1;
			if (currNode.xhb > this->stackX) currNode.xhb = this->stackX;
			if (currNode.ylb < 1) currNode.ylb = 1;
			if (currNode.yhb > this->stackY) currNode.yhb = this->stackY;
			if (currNode.zlb < 1) currNode.zlb = 1;
			if (currNode.zhb > this->stackZ) currNode.zhb = this->stackZ;

			this->nodeQueue.push(currNode);
		}
	}
}