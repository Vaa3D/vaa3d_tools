/* reconstruction_by_thinning_plugin_plugin.cpp
 * reconstruction_by_thinning_plugin
 *
 * 2015-6-1 : by Edward Hottendorf and Jie Zhou
 * Hackthon Test Plugin.
 *
 * ToDo: add parameters to decide if linking all the structures and the fg/bg foreground
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
Q_EXPORT_PLUGIN2(reconstruction_by_thinning_plugin, reconstruction_by_thinning_plugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    int threshold;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

//added functions here
itk::Image<signed short, (unsigned) 3>* createITKImage(unsigned char *datald, int *in_sz);
void reconstructionThinning(itk::Image<signed short, (unsigned) 3> *input, QString qoutputfilename, int reconstructionTh);
vector<pixPoint*> visitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool ***);
void trueAnalyze(itk::Image<signed short, (unsigned)3> *image, QString outfilename);
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z);
void outputSWC(vector<pixPoint*> & Points, QString outfilename);
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z);
void getNeighbors(itk::Image<signed short, (unsigned)3> *image, vector<pixPoint*> & neigh, pixPoint* p);
int numberOfNeighbors(itk::Image<signed short, (unsigned)3> *image, int xCoord, int yCoord, int zCoord);
bool *** resetVisited(itk::Image<signed short, (unsigned)3> *image);

typedef itk::Image<signed short, 3> ImageType;
ImageType::Pointer globalImage = ImageType::New();


QStringList reconstruction_by_thinning_plugin::menulist() const
{
	return QStringList() 
		<<tr("reconstruct_thinning")
		<<tr("about");
}

QStringList reconstruction_by_thinning_plugin::funclist() const
{
	return QStringList()
		<<tr("reconstruct_thinning")
		<<tr("help");
}

void reconstruction_by_thinning_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("reconstruct_thinning"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("reconstruction_by_thinning_plugin. "
            "Developed by Jie Zhou, Edward Hottendorf, 2015-6-1"));
	}
}

bool reconstruction_by_thinning_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("reconstruct_thinning"))
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
        PARA.threshold = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;

        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of reconstruction_by_thinning_plugin tracing **** \n");
        printf("vaa3d -x reconstruction_by_thinning_plugin -f reconstruct_thinning -i <inimg_file> -p <parameters>\n");
        printf("inimg_file       The input image\n");
        printf("threshold        Threshold by binarization (default 0).\n");
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
        PARA.threshold = 0;
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

    //1. convert vaa3D image to itk image
    int imageSize[3];
    imageSize[0] = N;
    imageSize[1] = M;
    imageSize[2] = P;

    cout << "width: "  << N << endl;
    cout << "height: "  << M << endl;
    cout << "depth: "  << P << endl;

    /* //debugging
    // * Note by J Zhou June 3 2015
    // * Vaa3D appears having issue reading a signed int image of only 0 and 1 which was the output skeleton of the thinning algorithm of itk.
    // *  But it is ok as long as start directly from an unsigned image file.
    long num = 0;

    for(int iz = 0; iz < P; iz++)
    {
        int offsetk = iz*M*N;
        for(int iy = 0; iy < M; iy++)
        {
           int offsetj = iy*N;
           for(int ix = 0; ix < N; ix++)
           {

              int PixelValue =  data1d[offsetk + offsetj + ix];
              if (PixelValue > 0)
              {
                num++;
                if (num < 10)
                cout << ix << ":" << iy << ":" << iz << " ";

              }
           }
       }
    }

    v3d_msg(QString("Total foreground points:%1").arg(num), 0);
    */


    //itk::Image<signed short, (unsigned) 3> *input;
    itk::Image<signed short, (unsigned) 3> *input = createITKImage(data1d, imageSize);
    //2. call Reconstruction3D by passing the itkimage

    QString swc_name = PARA.inimg_file + "_ron.swc";
    int reconstructionTH = PARA.threshold;
    reconstructionThinning(input, swc_name, reconstructionTH);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

}

//the logic is similar as those in the main of Reconstruction3D()
void reconstructionThinning(itk::Image<signed short, (unsigned) 3> *input, QString qoutputfilename, int reconstructionTh)
{
     typedef itk::Image< signed short, (unsigned)3> ImageType;
    // Define the thinning filter
    typedef itk::BinaryThinningImageFilter3D< ImageType, ImageType > ThinningFilterType;

    ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();
    thinningFilter->threshold = reconstructionTh; //threshold to process images of non-zero background
    thinningFilter->SetInput(input);

    v3d_msg(QString("Start thinning with reconstruction threshold %1 ...").arg(reconstructionTh),0 );
    clock_t begin = clock();


    thinningFilter->Update();

    v3d_msg("after Thinning is done ", 0);
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    v3d_msg(QString("time used for thinning in secs  %1").arg(elapsed_secs), 0);

    //call true analyze which outputs swc file
    trueAnalyze(thinningFilter->GetOutput(), qoutputfilename);

}


//ImageOperation: convert Vaa3D to itk image
itk::Image<signed short, (unsigned) 3>* createITKImage(unsigned char *datald, int *in_sz)
{
             typedef itk::Image<signed short, 3> ImageType; //signed short is required for the thinning algorithm
             int SN = in_sz[0];
             int SM = in_sz[1];
             int SZ = in_sz[2];

             ImageType::Pointer I  = ImageType::New();

             ImageType::SizeType size;
             size[0] = SN;
             size[1] = SM;
             size[2] = SZ;

             ImageType::IndexType idx;
             idx.Fill(0);
             ImageType::RegionType region;
             region.SetSize( size );
             region.SetIndex( idx );

             I->SetRegions(region);
             I->Allocate();
             I->FillBuffer(0);

             signed short PixelValue;
             for(int iz = 0; iz < SZ; iz++)
             {
                 int offsetk = iz*SM*SN;
                 for(int iy = 0; iy < SM; iy++)
                 {
                    int offsetj = iy*SN;
                    for(int ix = 0; ix < SN; ix++)
                    {

                       PixelValue = (signed short) datald[offsetk + offsetj + ix];
                       itk::Index<3> indexX;
                       indexX[0] = ix;
                       indexX[1] = iy;
                       indexX[2] = iz;
                       I->SetPixel(indexX, PixelValue);
                    }
                }
             }

             //return the filled image pointer
             globalImage = I;
             return I;


}




void trueAnalyze(itk::Image<signed short, (unsigned)3> *image, QString outfilename){


        typedef itk::Image< signed short, (unsigned)3> ImageType;
        typedef signed short PixelType;

        vector <pixPoint*> endPoints;

        vector <Point*> points;

        vector<vector<pixPoint*> > skeletonPoints;
        //num of trees can be found by calling skeletonPoints.size()
        //num of pixels in a tree can be found by calling skeletonPoints[sumNum].size();

        //This sets the region to the entire image
        ImageType::RegionType region = image->GetLargestPossibleRegion();

        //returns a sizetype data type that is an array of three elements
        ImageType::SizeType size = region.GetSize();
        int width = size[0];
        int height = size[1];
        int depth = size[2];


        signed short crntPoint;

        for (int x = 0; x < width; x++)
                for (int y = 0; y < height; y++)
                        for (int z = 0; z < depth; z++)
                        {

                                crntPoint = getPixel(image, x, y, z);

                                        if (crntPoint != 0)
                                         {
                                             points.push_back(new Point(x, y, z));
                                          }
                        }


        v3d_msg(QString("in trueAnalyze: number of fg points: %1").arg(points.size()), 0);
        for (int i = 0; i < points.size(); i++)
        {
             if (numberOfNeighbors(image, points[i]->x, points[i]->y, points[i]->z) == 1){
            //int neighbor = numberOfNeighbors(globalImage, points[i]->x, points[i]->y, points[i]->z);
            //cout << points[i]->x << " " << points[i]->y << " " << points[i]->z << " " << neighbor << endl;
            //if (neighbor == 1){
                        endPoints.push_back(new pixPoint(points[i]->x, points[i]->y, points[i]->z));

                }
        }


        v3d_msg(QString("in trueAnalyze: number of end points in skeleton: %1").arg(endPoints.size()), 0);
        bool *** visited = resetVisited(image);

        for (int i = 0; i < endPoints.size(); i++)
                if (visited[endPoints[i]->x][endPoints[i]->y][endPoints[i]->z] != true)
                      skeletonPoints.push_back(visitSkeleton(image, endPoints[i], visited));
                    //skeletonPoints.push_back(visitSkeleton(globalImage, endPoints[i], visited));

        v3d_msg(QString("Number of total trees we get:  %1").arg(skeletonPoints.size()), 0);

        //bubble sort ascending order so the last tree
        int* indexNum = new int[skeletonPoints.size()];
        for (int i = 0; i < skeletonPoints.size(); i++)
        {
                indexNum[i] = i;
        }


        int i, j;
        int temp;
        bool swapped = true;

        for (i = (skeletonPoints.size() - 1); i >= 0 && swapped; i--)
        {
                swapped = false;

                for (j = 1; j <= i; j++)
                {
                        if (skeletonPoints[indexNum[j - 1]].size() > skeletonPoints[indexNum[j]].size())
                        {
                                temp = indexNum[j - 1];
                                indexNum[j - 1] = indexNum[j];
                                indexNum[j] = temp;
                                swapped = true;
                        }
                }
        }


        vector<pixPoint*> Points = skeletonPoints[indexNum[skeletonPoints.size() - 1 ]];

        /*
        for (int i = 0; i < Points.size(); i++){
                        cout << Points[i]->visitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << 1 << ' ' << Points[i]->parent << endl;
                }
         */

        outputSWC(skeletonPoints[indexNum[skeletonPoints.size() - 1 ]], outfilename); //currently outputing the largest tree


        points.clear();
        endPoints.clear();

        delete  visited;
}//end trueAnalyze



vector<pixPoint*> visitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool *** visited){

        pixPoint* crntPoint = new pixPoint(root->x, root->y, root->z);
        int num = 1;

        visited[crntPoint->x][crntPoint->y][crntPoint->z] = true;
        crntPoint->visitNum = num;
        crntPoint->parent = -1;

        queue<pixPoint*> pntQ;
        pntQ.push(crntPoint);

        vector<pixPoint*> crntSkelPoints;
        crntSkelPoints.push_back(crntPoint);

        vector<pixPoint*> neighbor;


        while (!pntQ.empty()){

                getNeighbors(image, neighbor, pntQ.front()); //neighbor passed in by reference
                //all of the neighbors will only have x, y, z at this point

                for (int i = 0; i < neighbor.size(); i++)
                {
                        if (visited[neighbor[i]->x][neighbor[i]->y][neighbor[i]->z] == false)
                        {
                                num++;
                                visited[neighbor[i]->x][neighbor[i]->y][neighbor[i]->z] = true;
                                neighbor[i]->visitNum = num;
                                neighbor[i]->parent = pntQ.front()->visitNum;


                                pntQ.push(neighbor[i]);
                                crntSkelPoints.push_back(neighbor[i]);

                        }
                }

                neighbor.clear();
                pntQ.pop();

        }


        return crntSkelPoints;

}//end visitSkeleton


bool *** resetVisited(itk::Image<signed short, (unsigned)3> *image){

        typedef itk::Image< signed short, (unsigned)3> ImageType;
        typedef signed short PixelType;
        //This sets the region to the entire image
        ImageType::RegionType region = image->GetLargestPossibleRegion();

        //returns a sizetype data type that is an array of three elements
        ImageType::SizeType size = region.GetSize();

        int width = size[0];
        int height = size[1];
        int depth = size[2];

        // Reset visited variable
        bool *** visited = new bool**[width];
        for (int i = 0; i < width; i++){
                visited[i] = new bool*[height];
                for (int u = 0; u < height; u++){
                        visited[i][u] = new bool[depth];
                }
        }

        for (int i = 0; i < width; i++)
                for (int j = 0; j < height; j++)
                        for (int k = 0; k < depth; k++)
                                visited[i][j][k] = false;

        return visited;
}//end resetVisited



int numberOfNeighbors(itk::Image<signed short, (unsigned)3> *image, int xCoord, int yCoord, int zCoord){

        int n = 0;

        for (int x = -1; x <= 1; x++)
                for (int y = -1; y <= 1; y++)
                        for (int z = -1; z <= 1; z++)
                        {

                                if (x == 0 && y == 0 && z == 0)
                                {
                                    //int self = getPixel(globalImage, xCoord + x, yCoord + y, zCoord + z);
                                    //cout << "self" << " " << self << " ";

                                        continue;
                                }

                               if (getPixel(image, xCoord + x, yCoord + y, zCoord + z) != 0)
                               // if (getPixel(globalImage, xCoord + x, yCoord + y, zCoord + z) != 0)
                                        n++;

                        }
        return n;
}//end numberOfNeighbors




void getNeighbors(itk::Image<signed short, (unsigned)3> *image, vector<pixPoint*> & neigh, pixPoint* p){



        for (int x = -1; x <= 1; x++)
                for (int y = -1; y <= 1; y++)
                        for (int z = -1; z <= 1; z++)
                        {

                                if (x == 0 && y == 0 && z == 0)
                                        continue;

                                if (getPixel(image, p->x + x, p->y + y, p->z + z) != 0)
                                {
                                        neigh.push_back(new pixPoint(p->x + x, p->y + y, p->z + z));
                                }

                        }

}//end getNeighbors




signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z){
        typedef itk::Image< signed short, (unsigned)3> ImageType;
        typedef signed short PixelType;
        //This sets the region to the entire image
        ImageType::RegionType region = image->GetLargestPossibleRegion();
        //ImageType::RegionType region = globalImage->GetLargestPossibleRegion();

        //returns a sizetype data type that is an array of three elements
        ImageType::SizeType size = region.GetSize();

        int width = size[0];
        int height = size[1];
        int depth = size[2];

        typedef itk::Image< signed short, 3 > ImageType;
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = x;
        pixelIndex[1] = y;
        pixelIndex[2] = z;

        if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth)
             return image->GetPixel(pixelIndex);
        else
                return 0;
}//end getpixel


void outputSWC(vector<pixPoint*> & Points, QString qoutputfilename){


       const char *outfilename = qoutputfilename.toStdString().c_str();
       cout << "filename converted from QString" << outfilename << endl;
       ofstream outFile(outfilename);
       outFile << "#name reconstruction3d \n#comment \n##n,type,x,y,z,radius,parent\n";

       for (int i = 0; i < Points.size(); i++){
                        outFile << Points[i]->visitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << 1 << ' ' << Points[i]->parent << endl;
                }

        outFile.close();
        cout << outfilename << " Successfully written to swc file.";

}//end outputSWC


