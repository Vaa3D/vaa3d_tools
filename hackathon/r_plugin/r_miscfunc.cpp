// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4;  tab-width: 4; -*-
//
// Simple test that did not trigger the bug reported by Miguel Lechón
//
// Copyright (C) 2009 Dirk Eddelbuettel and GPL'ed 
// Heavily edited 2012 by Brad Busse, continuing GPLvWhatever

// for the embedded R via RInside
#include <RInside.h> 
#include <vector>
#include <QtGui>
#include <v3d_interface.h>
#include "r_miscfunc.h"
#include <exception>
#include <stdio.h>
#include <fstream>

using namespace std;

const int SVM_SLICE = 1000;
int R_STARTED = 0;
RInside *R;
const int R_DEBUG = 0; //prints voluminous messages indicating what and where R is located

//this function ensures RInside is only instantiated once.
//Unfortunately it does not work with repeated plugin calls,
//as v3d deallocates plugin memory, but R still runs expecting it
//TODO: try passing RInside back wrapped in a V3D image, then recreating it
void checkR(){   
    if (!R_STARTED)//R == NULL)
    {
        cout<<"Starting a new instance of R"<<endl;
        if (R_DEBUG) printf("R obj at %p\n",R);
        R_STARTED=1;
        R = new RInside(0,0);//&Robj;
        if (R_DEBUG) printf("R obj at %p\n",R);
        if (R_DEBUG) cout<<"R instance created"<<endl;
        return;
    }
    cout<<"R listed as already created"<<endl;
}


std::string hello(){
    (*R)["txt"] = "Hello, world!\n";	// assign a char* (string) to 'txt'
    return (*R).parseEval("txt");           // eval the init string, ignoring any returns    
}

//converts a double vector to an Rcpp matrix
Rcpp::NumericMatrix vectvect2rmat(vector<vector<double> > in)
{
    if (in.empty()) 
    {
        Rcpp::NumericMatrix M(1,1);
        return  M;//crash basically
    }   
    Rcpp::NumericMatrix M(in.size(),in[0].size());
    for (int i=0; i<in.size(); i++) {
        for (int j=0; j<in[i].size(); j++) {
            M(i,j) = in[i][j]; 
        }
    }
    return(M);
}

//TODO: vector<vector<double> > rmat2vectvect (Rcpp::NumericMatrix in)

//runs SVM classification on passed-in args
vector<double> r_SVMclassify(vector<vector<double> > trainX,vector<double> trainY,vector<vector<double> > testX){
    cout<<"Beginning SVM classification"<<endl;
    int cost = 100; //TODO: figure out a way to pass in optional parameters like this, my compiler just did not want to for some reason

    checkR();
    if (R_DEBUG) printf("R pointer location (beginning of SVM): %p\n",R); 
    
    vector<double> testY;
    (*R)["costparam"]=cost; //doubles, ints and such can be assigned directly
    cout<<"Adding training vector"<<endl; 
    (*R)["trainY"]=trainY; //as can vectors

    cout<<"Converting matrices"<<endl;
    Rcpp::NumericMatrix M = vectvect2rmat(trainX); //but 2D vectors need manual translation
    (*R)["trainX"]=M;
    M = vectvect2rmat(testX);
    (*R)["testX"]=M;
    cout<<"Matrices converted, running classifier..."<<endl;

    string str = //TODO: play with SVM outputs, see if a better probability rep is possible
//        "print(trainX);print(trainY);print(testX);"
        "suppressMessages(library(e1071)); "                //loads svm library
        "classifier <- svm(trainX,trainY,cost=costparam);"   //trains the svm
        "testY=predict(classifier,testX);"                   //uses the svm
        "testY";            //returns predictions
//attr(pred, "probabilities")[1:4,]

    testY= Rcpp::as<vector<double> >((*R).parseEval(str)); //vectors can be directly cast back
    if (R_DEBUG) printf("R pointer location (end of SVM): %p\n",R);
    return testY;
}

//runs a simple gabor filter over the image
//each pixel gets a 3-vector with the x,y,z gabor measurements
//literally just the difference of the sumas of pixels in either direction
vector<vector<double> > runGabor(Image4DSimple* p4DImage, V3DLONG channel)
{
    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG c=channel;
    V3DLONG offsetc = c*pagesz;

    vector<vector<double> > features;
    
    double positive = 0, negative = 0;
    cout<<"Processing channel "<< c <<endl;
    for (V3DLONG x=1; x< N-1; x++) //shrink the ROI by 1 pixel to accomodate a 3x3x3 kernel
        for (V3DLONG y=1;y<M-1;y++)
            for (V3DLONG z=1;z<P-1;z++)
            {
                vector<double> gaborFeats(3, 1 ); //x,y,z
                //x filter
                positive=negative=0;
                for (int dy=-1;dy<2;dy++)
                    for (int dz=-1;dz < 2;dz++)
                    {
                        positive += data1d[offsetc + (z+dz)*M*N + (y+dy)*N + x+1];
                        negative += data1d[offsetc + (z+dz)*M*N + (y+dy)*N + x-1];
                    }
                gaborFeats[0] = positive-negative;

                //y filter
                positive=negative=0;
                for (int dx=-1;dx<2;dx++)
                    for (int dz=-1;dz < 2;dz++)
                    {
                        positive += data1d[offsetc + (z+dz)*M*N + (y+1)*N + x+dx];
                        negative += data1d[offsetc + (z+dz)*M*N + (y-1)*N + x+dx];
                    }
                gaborFeats[1] = positive-negative;

                //z filter
                positive=negative=0;
                for (int dx=-1;dx<2;dx++)
                    for (int dy=-1;dy < 2;dy++)
                    {
                        positive += data1d[offsetc + (z+1)*M*N + (y+dy)*N + x+dx];
                        negative += data1d[offsetc + (z-1)*M*N + (y+dy)*N + x+dx];
                    }
                gaborFeats[2] = positive-negative;

                features.push_back(gaborFeats);                                
            }
    return features;
}

//demonstrates classification process on active image
void r_demo_processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
		return;
	}

    Image4DSimple* p4DImage = callback.getImage(curwin);

	if (!p4DImage)
	{
		QMessageBox::information(0, "", "The image pointer is invalid. Ensure a 4D image  is active and try again.");
		return;
	}

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    
    cout<<"Welcome to the r_plugin image demonstration."<<endl;
    cout<<"In this demonstration, we will take the first channel of the active image, use it to train a classifier for image segmentation (a simple gabor filter), and use the classifier to process the other channels as well."<<endl;

    V3DLONG c=0; //start with the first channel
    V3DLONG offsetc = c*pagesz;
    int zeroclass=0, oneclass=0;
    vector<vector<double> > trainX, firstX;
    vector<double> trainY,firstY;

    firstX= runGabor(p4DImage,0); 

    //load up firstY results to demonstrate "ground truth" for first channel    
    for (int i=0;i<firstX.size() ;i++){
        if (firstX[i][0] < -0)// -0 is everything on the right. -360 gives a good right sillouette
        {
            firstY.push_back(1);
        }else{         
            firstY.push_back(0);
        }
    }
      
    //now generate a random training set by sampling from the results
    //-TODO add TRAINING_SIZE or some similar variable to disconnect 
    //training set size from SVM batch operation-
    for (int i=0;i<SVM_SLICE ;i++){
        V3DLONG r = (rand()%firstX.size());
        trainX.push_back(firstX[r]);
        if (firstX[r][0] < -0)//360)
        {            
            oneclass++;            
            trainY.push_back(1);
        }else{
            zeroclass++;
            trainY.push_back(0);
        }
        
    }
    cout <<"Chosen "<< zeroclass<<" negative examples, "<<oneclass<<" positive examples."<<endl;        
    cout << "Training set acquired.  Allocating memory for output..."<<endl;
         
              
    //get new blank image
    //one extra sc channel for gold standard data
    unsigned char *outimg = new unsigned char [N*M*P*(sc+1)]; 
    

    if (!outimg)
    {
        cout<<"Failed to allocate memory for output image.  Crashing horribly now..."<<endl;
        return;
    }
    
    for (V3DLONG i=0;i<N*M*P*(sc+1);i++) outimg[i]=0; //sets everything to 0 initially     
    
    const QString title = QObject::tr("Machine learned segmentation");
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg, N, M, P, sc+1, V3D_UINT8);
    
    cout<<"Extracting data to predict and splicing predictions into output..."<<endl;


    //test variables
    int tP,tN,fP,fN;
    tP=tN=fP=fN=0;

    for (V3DLONG c=0;c<sc;c++){
        vector<vector<double> > testX=runGabor(p4DImage,c); //this will provide its own cout feedback
        vector<double> testY;
        vector<vector<double> > sliceX;
        vector<double> sliceY;
        for (V3DLONG s=0;s*SVM_SLICE < testX.size();s++)
        {
            sliceX.clear();
            cout<<"\tRunning classifier: slice "<<s<<" of "<< ceil(testX.size()/SVM_SLICE) <<endl;
            for (V3DLONG ss = 0 ; ss < SVM_SLICE && (s*SVM_SLICE+ss) < testX.size(); ss++)
            {
                sliceX.push_back(testX[s*SVM_SLICE+ss]);
            }
            //cout<<trainY.size()<<"  "<<trainX.size()<<","<<trainX[0].size()<<"  "<<sliceX.size()<<","<<sliceX[0].size()<<endl;
            try
            {        
                //printf("R pointer location (just before SVM): %p\n",R);
                sliceY = r_SVMclassify(trainX,trainY,sliceX);
                //printf("R pointer location (just after SVM): %p\n",R);

                //there might be a more concise way of parsing prediction results.  meh.
                for (int i=0;i<SVM_SLICE ;i++){
                    if (sliceX[i][0] < -0)
                    {
                        if (sliceY[i] > 0.5)
                        {
                            tP++;
                        }else{
                            fN++;
                        }
                    }else{
                        if (sliceY[i] > 0.5)
                        {
                            fP++;
                        }else{
                            tN++;
                        }
                    }
                }
        
            cout<<"tP:"<<tP<<" tN:"<<tN<<" fP:"<<fP<<" fN:"<<fN<<endl;
            cout<<"Accuracy:"<<(100.0*(tP+tN))/(tP+tN+fP+fN)<<"%"<<endl;
            }catch(exception & e){
                cout<<"ERROR: "<< e.what() <<", you might try reducing your SVM_SLICE const"<< endl;
            }
            testY.insert(testY.end(),sliceY.begin(),sliceY.end()); //appends sliceY onto testY
        } 
        //cout<<testY.size()<<endl;
        

        cout<<"\tTransferring to output image..."<<endl;
        vector<double>::iterator it=testY.begin();
        offsetc = c*pagesz;

        for (V3DLONG x=1; x< N-1; x++) //shrink the ROI by 1 pixel to accomodate a 3x3x3 kernel
            for (V3DLONG y=1;y<M-1;y++)
                for (V3DLONG z=1;z<P-1;z++)
                {
                    it++;
                    outimg[offsetc + (z)*M*N + (y)*N + x]= round(*it) ? 255 : 0;
                }
    }    

    offsetc = (sc)*pagesz; //gold std data gets put into the extra channel
    vector<double>::iterator it=firstY.begin();
    for (V3DLONG x=1; x< N-1; x++) //shrink the ROI by 1 pixel to accomodate a 3x3x3 kernel
        for (V3DLONG y=1;y<M-1;y++)
            for (V3DLONG z=1;z<P-1;z++)
            {
                it++;
                outimg[offsetc + (z)*M*N + (y)*N + x]= round(*it) ? 255 : 0;
            }
    

    //display
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);
    
}


