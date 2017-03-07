/* reconstruction_by_thinning_plugin_plugin.cpp
 * reconstruction_by_thinning_plugin
 *
 * 2015-6-1 : by Edward Hottendorf and Jie Zhou
 * Hackthon Test Plugin.
 *
 * 2015-6-3 : renamed the plugin to SimpleAxisAnalyzer (modify the pro file and the menu list, function list, usage)
 *
 * 2015-11-21 : added some simple preprocessing such as adaptive thresholding and histogram equalization
 *
 * 2016-1-9:  added more preprocessing such as median filter. Default #of trees is 15. Radius estimation disabled for BIGNEURON.
 *
 * ToDo:  Big/bright soma impacts the result and should be removed first.
 *
 * See details of employed libraries, see Reconstruction3D.cxx.
 *
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "basic_surf_objs.h"

//itk headers
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkTIFFImageIOFactory.h"
#include "itkTIFFImageIO.h"

#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThinningImageFilter3D.h"

#include "Point.h"
#include "pixPoint.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <ctime>

#include "reconstruction_by_thinning_plugin_plugin.h"
#include "Reconstruction3D.cxx"

Q_EXPORT_PLUGIN2(reconstruction_by_thinning_plugin, reconstruction_by_thinning_plugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel = 1;
    int threshold = 0 ;
    int treeNum = 10;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

//added functions here
//itk::Image<signed short, (unsigned) 3>* createITKImage(unsigned char *datald, int *in_sz);
void reconstructionThinning(itk::Image<signed short, (unsigned) 3> *input, QString qoutputfilename, int reconstructionTh, int treeNum);


//******** main methods for the axis analyzer start here ********/
typedef itk::Image<signed short, 3> ImageType;
ImageType::Pointer globalImage = ImageType::New();


QStringList reconstruction_by_thinning_plugin::menulist() const
{
	return QStringList() 
        <<tr("medial_axis_analysis")
		<<tr("about");
}

QStringList reconstruction_by_thinning_plugin::funclist() const
{
	return QStringList()
        <<tr("medial_axis_analysis")
		<<tr("help");
}

void reconstruction_by_thinning_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("medial_axis_analysis"))
	{
        bool bmenu = true;
        input_PARA PARA;

        //input
         bool ok1, ok2;

        PARA.threshold = QInputDialog::getInteger(parent, "Window X ",
                                       "Enter threshold (Special parameters: 0, 998, 999. Other thresholds will be used directly for binarization.) Default 0):",
                                       0, 0, 999, 1, &ok1);

        if (!ok1) return;  //have problem getting threshold

        PARA.treeNum = QInputDialog::getInteger(parent, "Window X ",
                                       "Enter number of tree you would like to reconstruct (Default 15):",
                                       15, 1, 50, 1, &ok2);

        if (!ok2) return;  //have problem getting threshold

        reconstruction_func(callback,parent,PARA,bmenu);
        
	}
	else
	{
        v3d_msg(tr("NeuronAxisAnalyser -- Input: Gray-level Image (or Binary Image); Output: Reconstructed swc\n"
            "Developed by Jie Zhou and Edward Hottendorf, 2015-6-1"));
	}
}

bool reconstruction_by_thinning_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("medial_axis_analysis"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.threshold = (paras.size() >= k+1) ? atoi(paras[k]) : 998;  k++;
        std::cout<< "PARA.threshold: " << PARA.threshold << endl;
        PARA.treeNum = (paras.size() >= k+1) ? atoi(paras[k]) : 15;  k++;
         std::cout<< "PARA.treeNum: " << PARA.treeNum << endl;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;


        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN

        printf("**** Usage of axis analyzer plugin (Jan 2016) **** \n");
        printf("vaa3d -x SimpleAxisAnalyzer -f medial_axis_analysis -i <inimg_file> -p <parameters>\n");
        printf("inimg_file       The input image\n");
        printf("threshold       Threshold (default 998). Special parameters: 0, 998, 999. Other thresholds will be used directly for binarization.\n");
        printf("numberOfTrees    Number of trees to include in reconstruction (default 15). \n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

     }
     else return false;

    return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
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
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

    cout << "width: "  << N << endl;
    cout << "height: "  << M << endl;
    cout << "depth: "  << P << endl;

    // * Note by J Zhou June 3 2015
    // * Vaa3D appears having issue reading a signed int image of only 0 and 1 which was the output skeleton of the thinning algorithm of itk.
    // *  But it is ok as long as the plugin starts directly from an unsigned image file.
  
    // * Nov 20 2015: ORNL Hackthon: add some preprocessing/thresholding method since some images in BigNeuron gold standard are noisy and not binarized.
    //  * Jan  9 2016: Call a function in Reconstruction3D to do the preprocessing
    //  The threshold is passed by reference and may be changed by the preprocessing process.
    data1d = Preprocess(data1d, N, M, P, 1, PARA.threshold);
    
    //convert to ITK
    ImageType::Pointer input = ImageType::New();
    //it also binarizes the image using the threshold (set to 244 and 0)
    createITKImage(data1d, input, N, M, P, PARA.threshold);
  

    //2. call Reconstruction3D by passing the itkimage

    QString swc_name = PARA.inimg_file + "_axis_analyzer.swc";

    reconstructionThinning(input, swc_name, PARA.threshold, PARA.treeNum);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

}

//the logic is similar as those in the main of Reconstruction3D()
void reconstructionThinning(itk::Image<signed short, (unsigned) 3> *input, QString qoutputfilename, int reconstructionTh, int treeNumber)
{
     typedef itk::Image< signed short, (unsigned)3> ImageType;
    // Define the thinning filter
    typedef itk::BinaryThinningImageFilter3D< ImageType, ImageType > ThinningFilterType;
  
    ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();
    //threshold to process images of non-zero background, anything in (0,244).
   // reconstructionTh no longer useful since the binrzation was done in createITKimage(). 
    thinningFilter->threshold = 5; //just a place holder
    thinningFilter->SetInput(input);

    v3d_msg(QString("Start thinning  ..."),0 );
    clock_t begin = clock();

    thinningFilter->Update();

    v3d_msg("after Thinning is done ", 0);
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    v3d_msg(QString("time used for thinning in secs  %1").arg(elapsed_secs), 0);


    //for debugging: save the thinning image
    /*
    char * tmpoutfile = "/Users/jiezhou/skeletonizedTest.jpeg";
    typedef signed short PixelType;
    const   unsigned int Dimension = 3;
    typedef itk::Image< PixelType, Dimension > ImageType;
    typedef itk::ImageFileWriter< ImageType > WriterType;
    //itk::ImageIOFactory::CreateImageIO("test.tif");
    //itk::ObjectFactoryBase::RegisterFactory(itk::TIFFImageIOFactory::New());
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(thinningFilter->GetOutput());
    writer->SetFileName(tmpoutfile);
    try
    {
        writer->Update();
        cout << tmpoutfile << " sucessfully written the thinned image (before postprocessing such as TrueAnalyze)." << endl;
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;

    }
    */


    //12/03/2015 call the trueAnalyze in Reconstruction3D.cxx
     v3d_msg(QString("Calling trueAnalyze in Reconstruction3D ..."), 0);
    
    //re-allocate space for the filename due to issues of cstr converted from qstring when pass to outputSWC.
    int length = strlen(qoutputfilename.toStdString().c_str());
    char * filename = new char[length];
    strcpy(filename, qoutputfilename.toStdString().c_str());    
    cout << "filename converted from QString" << filename << endl;

    trueAnalyze(thinningFilter->GetOutput(), filename, input, treeNumber);
    
}

