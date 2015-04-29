#ifndef _STALKER_H_
#define _STALKER_H_

#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;


class Stalker{
public:
    struct Position
    {
		float x, y, z;
	};

    struct Vblocksize 
    {
		int x, y, z;
	};

	struct SphericalAngle
	{
	    float theta, phi;	
	};

	Position GetPosition();
	Position* GetTrace();
	Mat GetVisionBlock();

    bool Go(float); // Go one step according to the decision made by the learning model, return bool if should commit suicide
    void Suicide(); // Commit a suicide

    Stalker(float, float, float, float, float, int, int, int, Mat*); // Constructor
    ~Stalker();
    template<typename T> static Ptr<T> load_classifier(const string& filename_to_load);

protected:
	Mat vblock; // A 3D-voxel block
	Mat * img; // The pointer to the whole image, it does not change the original image
	Vblocksize vblocksize;
	Position coord; // Cartesian Coordinates of this stalker in float
    Position* trace; // The trace of this stalker
    
    SphericalAngle lastdirection;
    SphericalAngle direction;

    Ptr<RTrees> ThetaTree; // The Random Forest For Decision Making
    Ptr<RTrees> PhiTree; // The Random Forest For Decision Making

	bool LoadRandomForest(); // Load a Trained Random Forest for .mat File
	void UpdateVblock();

};

#endif /* _STALKER_H_ */