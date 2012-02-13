/* example_func.cpp
 * This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
 * 2012-02-13 : by YourName
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "example_func.h"
#include <vector>
#include <iostream>
using namespace std;

/* function used in DOMENU typically takes 2 inputs:
 * "callback" - provide information from the plugin interface, and 
 * "parent"   - the parent widget of the Vaa3D main window
 */
const QString title="Image Thresholding";
int image_threshold(V3DPluginCallback2 &callback, QWidget *parent)
{
	// 1 - Obtain the current 4D image pointer
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	Image4DSimple *p4DImage = callback.getImage(curwin);

	
	// 2 - Ask for parameters
	//     asking for the subject channel
	bool ok;
	int c = -1;
	c = QInputDialog::getInteger(parent, "Channel Number", "Set the subject channel number:", 0, 0, p4DImage->getCDim()-1, 1, &ok);
	if (!ok) return -1;
	
	//      asking for threshold
	int thres = -1;
	thres = QInputDialog::getInteger(parent, "Threshold", "Set threshold:", 0, 0, 255, 1, &ok);
	if (!ok) return -1;
	

	// 3 - Create a new image and do binary thresholding
	V3DLONG sz[3];
	sz[0] = p4DImage->getXDim();
	sz[1] = p4DImage->getYDim();
	sz[2] = p4DImage->getZDim();
	unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);
	V3DLONG tb = sz[0]*sz[1]*sz[2]*p4DImage->getUnitBytes();
	unsigned char * nm = NULL;
	try {
		nm = new unsigned char [tb];
	} catch (...) {
		throw("Fail to allocate memory in Image Thresholding plugin.");
	}
	for (V3DLONG i=0;i<tb;i++)
	{
		if (inimg1d[i]>=thres) nm[i] = 255;
		else nm[i] = 0;
	}

	// 4 - Set and show the thresholded image in a new window
	v3dhandle newwin = callback.newImageWindow();
	p4DImage->setData(nm, sz[0], sz[1], sz[2], 1, p4DImage->getDatatype());//setData() will free the original memory automatically
	callback.setImage(newwin, p4DImage);
	callback.setImageName(newwin, QObject::tr("Image Thresholding"));
	callback.updateImageWindow(newwin);
	return 1;
}




/* functions in DOFUNC takes 2 parameters
 * "input" arglist has 2 positions reserved for input and parmeter:
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
int image_threshold(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to image_threshold function"<<endl;
	if(input.size() != 2 || output.size() != 1) 
	{
		cout<<"illegal input!"<<endl;
		printHelp();
		return -1;
	}


	// 1 - Read input image
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	if (inlist->size() != 1)
	{
		cout<<"You must specify 1 input file!"<<endl;
		return -1;
	}
	char * infile = inlist->at(0);
	cout<<"input file: "<<infile<<endl;
	unsigned char * inimg1d = NULL;
	V3DLONG * sz = NULL;
	int datatype;
	if (!loadImage(infile, inimg1d, sz, datatype)) return -1;


	// 2 - Read color channel and threshold parameter
	vector<char*>* paralist = (vector<char*>*)(input.at(1).p);
	if (paralist->size() != 2)
	{
		cout<<"Illegal parameter!"<<endl;
		printHelp();
		return -1;
	}
	int c = atoi(paralist->at(0));
	int thres = atoi(paralist->at(1));
	cout<<"color channel: "<<c<<endl;
	cout<<"threshold : "<<thres<<endl;
	if (c < 0 || c>=sz[3])
	{
		cout<<"The color channel does not exist!"<<endl;
		return -1;
	}
	loadImage(infile, inimg1d, sz, datatype, c);

	
	// 3 - Read output fileName
	vector<char*>* outlist = (vector<char*>*)(output.at(0).p);
	if (outlist->size() != 1)
	{
		cout<<"You must specify 1 output file!"<<endl;
		return -1;
	}
	char * outfile = outlist->at(0);
	cout<<"output file: "<<outfile<<endl;

	// 4 - Do binary segmentation
	V3DLONG tb = sz[0]*sz[1]*sz[2]*datatype;
	unsigned char * nm = NULL;
	try {
		nm = new unsigned char [tb];
	} catch (...) {
		throw("Fail to allocate memory in Image Thresholding plugin.");
	}
	for (V3DLONG i=0;i<tb;i++)
	{
		if (inimg1d[i]>=thres) nm[i] = 255;
		else nm[i] = 0;
	}

	// 5 - Save file and free memory
	sz[3] = 1;
	saveImage(outfile, nm, sz, datatype);
	if (nm) {delete []nm; nm=NULL;}

	return 1;
}

void printHelp()
{
	cout<<"\nThis is a demo plugin to perform binary thresholding in an image. by Yinan Wan 2012-02"<<endl;
	cout<<"\nUsage: v3d -x <example_plugin_name> -f image_thresholding -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
	cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
	cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
	cout<<"\t -p <subject_color_channel> <threshold>     the channel you want to perform thresholding and the threshold"<<endl;
	cout<<"\t                                            the 2 paras must come in this order"<<endl;
	cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
	return;
}


