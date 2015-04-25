#ifndef _STALKER_H_
#define _STALKER_H_

using namespace std;
#include <string>

class Stalker{
public:
    struct Position
    {
		float x;
		float y;
		float z;
	};

	struct SphericalAngle
	{
	    float theta;	
	    float phi;
	};
    
	Position GetPosition();
	Position* GetTrace();
    bool Go(float); // Go one step according to the decision made by the learning model, return bool if should commit suicide
    void Suicide(); // Commit a suicide

    Stalker(float, float, float, float, float, string); // Constructor
    ~Stalker();

protected:
	void LoadRandomForest(string); // Load a Trained Random Forest for .mat File
	Position coord; // Cartesian Coordinates of this stalker in float
    Position* trace; // The trace of this stalker
    SphericalAngle lastdirection;
    SphericalAngle direction;
};

#endif /* _STALKER_H_ */