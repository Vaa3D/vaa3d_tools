#include "DandelionTracer.h"
#include "utils/matmath.h"

Dandelion::Dandelion(PressureSampler sampler, Point3D position, float step, 
	                 float fromth, float fromphi, LabelImagePointer wallimg,
	                 DandelionTree* tree): 
sampler(sampler), 
position(position), 
step(step),
fromth(fromth), 
fromphi(fromphi),
wallimg(wallimg),
tree(tree)
{
	this->EstimateRadius();
}


vector<Dandelion*> Dandelion::walk()
{
	vector<Dandelion*> nextgen;
	// Get Peaks from its pressure sampler
    this->sampler.RandRotateSph();
    this->sampler.UpdatePosition(this->position.x, this->position.y, this->position.z);
    this->sampler.RandSample();
    vectype peakth = this->sampler.GetPeakTh();
    vectype peakphi = this->sampler.GetPeakPhi();
    cout<<"peakth size: "<<peakth.size()<<"peakphi size: "<<peakphi.size();
    cout<<"peakth vaule: "<<peakth[0]<<"peakphi vaule: "<<peakphi[0]<<endl;
    assert(peakth.size() == peakphi.size());

    // TODO: Filtering the peaks
    int ndir = peakth.size();


    // Calculate the displacement vectors for cartisian
    vectype dx (ndir);
    vectype dy (ndir);
    vectype dz (ndir);
    vectype rvec(ndir, this->step);
    sph2cart(peakth, peakphi, rvec, &dx, &dy, &dz);
    cout<<"dx: "<<dx[0]<<"dy: "<<dy[0]<<"dz: "<<dz[0]<<endl;


    // Create a new dandelion for new directions, except its coming direction    
    // The last one in the list is itself
    for (int i=0; i<ndir; i++)
    {

    	// Calculate the new position
    	Point3D newposition;
    	// TODO: Hinder by the wall
        cout<<"rounddx: "<<round(dx[i])<<"rounddy: "<<round(dy[i])<<"rounddz: "<<round(dz[i])<<endl;
    	newposition.x = round(dx[i]) + this->position.x;
     	newposition.y = round(dy[i]) + this->position.y;
    	newposition.z = round(dz[i]) + this->position.z;
        cout<<"new x: "<<newposition.x<<"new y: "<<newposition.y<<"new z: "<<newposition.z<<endl;


        Dandelion * d = new Dandelion (this->sampler, // Send a copy of its own sampler
            	         newposition, 
            	         this->step, 
			            // assign the from direction as the reversed 
            	         peakth[i],
            	         peakphi[i],
            	         this->wallimg,
            	         this->tree
            	         );
        nextgen.push_back(d);

        //TODO: Update Tree
    }
}


DandelionTree Dandelion::GetTree()
{
	return *(this->tree);
}


void Dandelion::EstimateRadius(){
	this->sampler.GetRadius();
}
