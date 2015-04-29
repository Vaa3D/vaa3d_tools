/* Stalker.cpp
 * The implementation of the NeuroStalker Model
 * by Siqi Liu, Donghao Zhang
 * WIP!
 */

 #include "Stalker.h"


Stalker::Stalker(float x, float y, 
	             float z, float theta, 
	             float phi, int blocksizex, 
	             int blocksizey, int blocksizez, 
	             Mat* srcimg)
{
	this->coord.x = x;
	this->coord.y = y;
	this->coord.z = z;
	this->direction.theta = theta;
	this->direction.phi = phi;
	this->vblocksize.x = blocksizex;
	this->vblocksize.y = blocksizey;
	this->vblocksize.z = blocksizez;
	this->img = srcimg;
	// TODO: Load Learnt Model ( A Random Forest ) from modelpath
	this->LoadRandomForest();
};


Stalker::~Stalker(){};


bool Stalker::Go(float stepsize)
{
    // -------1. Make decisions based on the Random Forest	
    float theta = this->ThetaTree->predict(this->vblock);
    float phi = this->PhiTree->predict(this->vblock);

    // -------2. Move towards the decided direction in position 
    
    // -------3. Save the Trace

    // -------4. Update its vision block at the new position
};


void Stalker::Suicide()
{
	// TODO:
};


bool Stalker::LoadRandomForest()
{  
    this->ThetaTree = load_classifier<RTrees>("ThetaTree.xml");
    this->PhiTree = load_classifier<RTrees>("PhiTree.xml");

    if( this->ThetaTree.empty() || this->PhiTree.empty() )
    {
        cout << "Could not find the trained Forests " << endl;
        return false;
    } 
    return true;
};


Stalker::Position Stalker::GetPosition()
{
	return this->coord;
};


Mat Stalker::GetVisionBlock()
{
	return this->vblock;
};


void Stalker::UpdateVblock(){
	// TODO: check boundaries
	Range ranges[3];
	ranges[0] = Range(this->coord.x - (int)(this->vblocksize.x / 2), this->coord.x + (int)(this->vblocksize.x / 2));
    ranges[1] = Range(this->coord.y - (int)(this->vblocksize.y / 2), this->coord.y + (int)(this->vblocksize.y / 2));
	ranges[2] = Range(this->coord.z - (int)(this->vblocksize.z / 2), this->coord.z + (int)(this->vblocksize.z / 2));
    this->vblock = (*(this->img))(ranges).clone();
}

template<typename T>
Ptr<T> Stalker::load_classifier(const string& filename_to_load)
{
    // load classifier from the specified file
    Ptr<T> model = StatModel::load<T>( filename_to_load );
    if( model.empty() )
        cout << "Could not read the classifier " << filename_to_load << endl;
    else
        cout << "The classifier " << filename_to_load << " is loaded.\n";

    return model;
}