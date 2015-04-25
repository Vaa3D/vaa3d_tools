/* Stalker.cpp
 * The implementation of the NeuroStalker Model
 * by Siqi Liu, Donghao Zhang
 * WIP!
 */

 #include "Stalker.h"


Stalker::Stalker(float x, float y, float z, float theta, float phi, string modelpath){
	this->coord.x = x;
	this->coord.y = y;
	this->coord.z = z;
	this->direction.theta = theta;
	this->direction.phi = phi;
	// TODO: Load Learnt Model ( A Random Forest ) from modelpath
	this->LoadRandomForest(modelpath);
};

Stalker::~Stalker(){

};

Stalker::Position Stalker::GetPosition(){
	return this->coord;
}

bool Stalker::Go(float stepsize){
    // TODO:	
}

void Stalker::Suicide(){
	// TODO:
}

void Stalker::LoadRandomForest(string Filepath){
    // TODO:
}