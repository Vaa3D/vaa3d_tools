#include "example_func.h"
#include <vector>
#include <iostream>
using namespace std;

/* function used in DOMENU typically takes 2 inputs:
 * "callback" - provide information from the plugin interface, and 
 * "parent"   - the parent widget of the Vaa3D main window
 */

const QString title="Image Thresholding";
int image_threshold(V3DPluginCallback2 &callback,QWidget *parent){
	// 1 - Obtain the current 4D image pointer
	v3dhandle curwin=callback.currentImageWindow();

	if(!curwin){
		QMessageBox::Information(0,title,QObject);
		return -1;
	}

	Image4DSimple *p4DImage=callback.getImage(curwin);

	// 2 - Ask for parameters
	//asking for subject channel
	bool ok;
	int c=-1;
	c=QInputDialog::getInteger(parent,"Channel Number","Set the subject channel number",0,0,p4DImage->getCDim()-1,1,&ok);
	if(!ok) return -1;

	//asking for threshold
	int thres=-1;
	thres=QInputDialog::getInteger(parent,"Threshold","Set threshold:",0,0,255,1,&ok);
	if (!ok) return -1;

	//3-create a new image and do binary thresholding
	V3DLONG sz[3];
	sz[0]=p4DImage->getXDim();
	sz[1]=p4DImage->getYDim();
	sz[2]=p4DImage->getZDim();
	usigned char *inimg1d=p4DImage->getRawDataAtChannel(c);
	V3DLONG tb=sz[0]*sz[1]*sz[2]*p4DImage->getUnitBytes();

	usigned char *nm=NULL;
	try{
		num=new unsigned char[tb];
	}catch(...){
		throw("Fail to allocate memory in Image Threshold plugin.");
	}
	for(V3DLONG i=0;i<tb;i++){
		if(inimgi1d[i]>=thres) nm[i]=255;
		else nm[i]=0;
	}

	//4-set and show the thresholded image in a new window
	v3dhandle newwin=callback.newImageWindow();
	p4DImage->setData(nm,sz[0],sz[1],sz[2],1,p4DImage->getDatatype());//setData() will free the original memory automatically
	callback.setImage(newwin,p4DImage);
	callback.setImageName(newwin,QObject::tr("Image Threshold"));
	callback.updateImageWindow(newwin);
	return 1;

}

/* functions in DOFUNC takes 2 parameters
 * "input" arglist has 2 positions reserved for input and parameter:
 *            input.at(0).p returns a pointer to vector<char*> that pass the arglist following the input option '-i'
 *                                  items are splitted by ' ', which is often used as input data 
 *                                  [required]
 *            input.at(1).p returns a pointer to vector<char*> that pass the arglist following the input option '-p'. 
 *                                  items are splitted by ' ', it is reserved for you to define your own parameters 
 *				    [not required, if '-p' is not specified, input only contains one member]
 * "output" arglist has a size of 1:
 *            output.at(0).p returns a pointer to vector<char*> that pass the arglist following the input option '-o' 
 *                                  items are splitted by ' ', which is often used as output data 
 *                                  [required]
 */

int image_threshold(const V3DPluginArgList &input,V3DPluginArgList &output){
	cout << "Welcome yo image_threshold function" << endl;
	if(input.size()!=2||output.size()!=0){
		cout << "illegal input!" << endl;
		printHelp();
		return -1;
	}

	//1-read input image
	vector<char*>* inlist=(vector<char*>*)(input.at(0).p);
	
	if(inlist->size()!=1){
		cout << "You must specift 1 input file!" << endl;
		return -1;
	}
	char *infile=inlist->at(0);
	cout << "input file: " << infile << endl;
	usigned char *inimg1d=NULL;
	V3DLONG *sz=NULL;
	int datatype;
	if(!loadImage(infile,inimg1d,sz,datatype)) return -1;

	//2-read color channel and threshold parameter
	vector<char*>* paralist=(vector<char*>*)(input.at(1).p);
	if(paralist->size()!=2){
		cout << "illegal parameter!" << endl;
		printHelp();
		return -1;
	}

	int c=atoi(paralist->at(0));
	int thres=atoi(paralist->at(1));
	cout < "color channel: " << c << endl;
	cout << "threshold: " << thres << endl;
	if(c<0||c>=sz[3]){
		cout << "the color channel dees not exist!" << endl;
		return -1;
	}
	loadImage(infile,inimg1d,sz,datatype,c);

	//3-read output filename
	vector<char*>* outlist=(vector<char*>*)(output.at(0).p);
	if(outlist->size()!=1){
		cout << "you must specify 1 output file!" << endl;
		return -1;
	}

	char *outfile=outlist->at(0);
	cout << "output file: " << outfile << endl;

	//4-do binary segmentation
	V3DLONG tb=sz[0]*sz[1]*sz[2]*datatype;
	usigned char* nm=NULL;


}