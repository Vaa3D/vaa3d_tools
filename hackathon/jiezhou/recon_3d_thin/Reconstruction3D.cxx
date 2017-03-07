/*
Programmer: Edward Hottendorf 
For use in the Big Neuron Project

Reconstruction3D is a blend of two programs, AnalyzeSkeleton(http://fiji.sc/AnalyzeSkeleton) and 
itkBinaryThinningImageFilter3D(http://www.insight-journal.org/browse/publication/181), 
and combines them together to thin and analyze a binarized image. Afterwards original code
was added to for certain functionalities.

More information on the thinning filter can be
found on http://fiji.sc/wiki/index.php/Skeletonize3D which references the itk version and also has an imagej version

The current output is in the form of an swc file
*/




#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThinningImageFilter3D.h"

#include "pixPoint.h"
#include "Vertex.h"
#include "Edge.h"
#include "Preprocessing.h"
#include "fillHoles.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <set>
#include <ctime>

using namespace std;

const int UNDEFINED = -1;
const int SLAB = 0;
const int JUNCTION = 1;
const int END_POINT = 2;


//test
struct graph
{
	vector<Vertex*> vertices; //the actual vertices and edges are declared dynamically
	vector<Edge*> edges;
};

//To avoid calling more pixPoints when they are unecessary
struct listOfCoordinates
{
	vector<int> x;
	vector<int> y;
};

struct vertexPair
{
	Vertex* v1;
	Vertex* v2;
	vector<Edge*> path;
	int distance;
	
};

bool fncomp (Vertex* lhs, Vertex* rhs) {return lhs->distance < rhs->distance;}

vector<pixPoint*> revisitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool ***);
vector<pixPoint*> visitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool ***, graph * g);
void trueAnalyze(itk::Image<signed short, (unsigned)3> *image, char* outfilename, itk::Image<signed short, (unsigned)3> *, int);
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z);
void outputTestSWC(vector<pixPoint*> &, char*, itk::Image<signed short, (unsigned)3> *, long);
void outputSWC(vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> *, long);
bool loopCleanup(graph * g, vector<pixPoint*> & Points, itk::Image<signed short, (unsigned)3> *);
void loopDetectionForPrinting(graph * g);
void spurCleanup(graph * g, vector<pixPoint*> & Points, itk::Image<signed short, (unsigned)3> *, itk::Image<signed short, (unsigned)3> *);
void findLoopEdges(graph * g, vector<Edge*> & loopEdges);
vector<Edge*> buildLoops(Edge* );
void reverseIterate(Edge* , Vertex* , vector<Edge*> &);
void findLongestShortestPath(vector <Edge*> &, vector <Edge*> &);
void getShortestPath(vertexPair* , vector<Edge*> & );
void pruneLoop(vector <Edge*> &, vector <Edge*> &, graph &);
void findConnectionAndAdd(Vertex*, vector <Edge*> &, vector <Edge*> &, vector <Edge*> &);
void getNeighbors(itk::Image<signed short, (unsigned)3> *image, vector<pixPoint*> & neigh, pixPoint* p);
int numberOfNeighbors(itk::Image<signed short, (unsigned)3> *image, int xCoord, int yCoord, int zCoord);
bool *** resetVisited(itk::Image<signed short, (unsigned)3> *image);
Vertex* createVertex(itk::Image<signed short, (unsigned)3> *, pixPoint*);
int findInVertex(vector<Vertex*>&, pixPoint*);
int findRadius(pixPoint* crntPnt, itk::Image<signed short, (unsigned)3> * image);

bool visited( vector<Vertex*> &, Vertex*);
bool visited(vector<Edge*> &, Edge*);
int foundInWatch(vector<pixPoint*>&, int x, int y, int z);
bool visited(listOfCoordinates& LOC, int x, int y);

void mergeLoops(vector< vector <Edge*> > fullLoops, graph * g, itk::Image<signed short, (unsigned)3> * image, vector<pixPoint*>& Points);

void startPreprocess(itk::Image<signed short, (unsigned)3> *, itk::Image<signed short, (unsigned)3> *, int);
void createITKImage(unsigned char *datald, itk::Image<signed short, (unsigned)3> * I, long x, long y, long z, int threshold);

//test
void findMidPoint(vector<Edge*>, pixPoint&);
int flood2DHole(vector<Edge*> loop, itk::Image<signed short, (unsigned)3> * image, pixPoint& centerPoint, listOfCoordinates& LOC);
void makeNewEdge(Vertex* v1, Vertex* v2, Edge* newEdge, vector<pixPoint*>& Points);
void merge(vector <Edge*>& currentLoop, graph * g, itk::Image<signed short, (unsigned)3> * image, pixPoint& centerPoint, listOfCoordinates& LOC, graph * toBeDeleted, vector<pixPoint*>& Points);
void findNewRadius(pixPoint* crntPnt, itk::Image<signed short, (unsigned)3> * image, listOfCoordinates& LOC, vector<Edge*>& currentLoop);
bool inside(int x, int y, int lowY, int highY, int lowX, int highX, vector<Edge*>& loop);
bool outsideLoopConnection(vector<Edge*>& currentLoop, Vertex* V, graph * toBeDeleted, listOfCoordinates& LOC);
bool isBorderPoint(vector<Edge*> loop, int x, int y);

void linkConnectivitySWC(pixPoint* root, vector<pixPoint*>& Points, bool *** image, bool *** visited, vector<pixPoint*>& watchList, int width, int height, int depth);
void fillNewImage(bool *** newImage, vector<pixPoint*>& Points, vector<pixPoint*>& watchList);
bool inRadius(vector<Edge*>& currentLoop, int x, int y);

int main(int argc, char* argv[])
{
	// Verify the number of parameters in the command line
	if (argc <= 2)
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << argv[0] << " inputImageFile outputImageFile" << std::endl;
		return EXIT_FAILURE;
	}
	char* infilename = argv[1];
	char* outfilename = argv[2];

	int thresh;
	int treeNum = 15;
	
	cout << "What threshold do you want? (500 for already preprocessed image)(5000 for already skeletonized. Verify you put in skeleton) ";
	cin >> thresh;

	cout << "How many trees do you want displayed? ";
	cin >> treeNum;


	time_t now = time(0);
	char * dt = ctime(&now);
	cout << "The current time is: " << dt << endl;

	const   unsigned int Dimension = 3;
	typedef signed short PixelType;   // must be signed for CT since Hounsfield units can be < 0

	//typedef unsigned char PixelType;
	typedef itk::Image< PixelType, Dimension > ImageType;

	// Read image
	typedef itk::ImageFileReader< ImageType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(infilename);
	try
	{
	reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
	std::cout << ex << std::endl;
	return EXIT_FAILURE;
	}
	cout << infilename << " sucessfully read." << endl;


	
	// Define the thinning filter
	typedef itk::BinaryThinningImageFilter3D< ImageType, ImageType > ThinningFilterType;
	ThinningFilterType::Pointer thinningFilter = ThinningFilterType::New();

	//preprocess takes the input image and then returns the preprocessed image

	ImageType::Pointer image = ImageType::New(); //create the itk image here and pass it in as a pointer parameter because of scope limitations

	//this is added so that i can send it already skeletonized images if i choose to do so
	if (thresh == 5000){
		image = reader->GetOutput();

		typedef itk::ImageFileReader< ImageType > ReaderType;
		ReaderType::Pointer reader1 = ReaderType::New();
		reader1->SetFileName("preProcessedTest.tif");
		try
		{
			reader1->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
			std::cout << ex << std::endl;
			return EXIT_FAILURE;
		}
		cout << "preProcessedTest.tif" << " sucessfully read." << endl;

		ImageType::Pointer preprocessed = ImageType::New(); //create the itk image here and pass it in as a pointer parameter because of scope limitations
		preprocessed = reader1->GetOutput();

		//add threshold and tree arguments
		//image is the preprocessed image
		trueAnalyze(image, outfilename, preprocessed, treeNum);
	}
	else {



		if (thresh == 500){
			image = reader->GetOutput();
		}
		else{
			startPreprocess(reader->GetOutput(), image, thresh);
		}
		thinningFilter->SetInput(image);



		//testing

		char * outfile = "preProcessedTest.tif";
		typedef itk::Image< PixelType, Dimension > ImageType;
		typedef itk::ImageFileWriter< ImageType > WriterType;
		WriterType::Pointer writer = WriterType::New();
		writer->SetInput(image);
		writer->SetFileName(outfile);

		try
		{
			writer->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
			std::cout << ex << std::endl;

		}
		cout << outfile << " sucessfully written." << endl;

		//testing



		thinningFilter->Update();


		outfile = "skeletonizedTest.tif";
		writer->SetInput(thinningFilter->GetOutput());
		writer->SetFileName(outfile);

		try
		{
			writer->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
			std::cout << ex << std::endl;

		}
		cout << outfile << " sucessfully written." << endl;

		//add threshold and tree arguments
		//image is the preprocessed image
		trueAnalyze(thinningFilter->GetOutput(), outfilename, image, treeNum);
	}
	now = time(0);
	char * dt2 = ctime(&now);
	cout << endl << "The current time is: " << dt << endl;
	cout << "Program terminated normally." << endl;

	return 0;
}


//ImageOperation: convert Vaa3D to itk image
void createITKImage(unsigned char *datald, itk::Image<signed short, (unsigned)3> * I, long x, long y, long z, int threshold)
{
	typedef itk::Image<signed short, 3> ImageType; //signed short is required for the thinning algorithm
	int SN = x;
	int SM = y;
	int SZ = z;


	ImageType::SizeType size;
	size[0] = SN;
	size[1] = SM;
	size[2] = SZ;

	ImageType::IndexType idx;
	idx.Fill(0);
	ImageType::RegionType region;
	region.SetSize(size);
	region.SetIndex(idx);

	I->SetRegions(region);
	I->Allocate();
	I->FillBuffer(0);

	signed short PixelValue;
	for (int iz = 0; iz < SZ; iz++)
	{
		int offsetk = iz*SM*SN;
		for (int iy = 0; iy < SM; iy++)
		{
			int offsetj = iy*SN;
			for (int ix = 0; ix < SN; ix++)
			{
				if ((signed short)threshold < (signed short)datald[offsetk + offsetj + ix])
				{
					PixelValue = 244;
				}
				else
				{
					PixelValue = 0;
				}
				itk::Index<3> indexX;
				indexX[0] = ix;
				indexX[1] = iy;
				indexX[2] = iz;
				I->SetPixel(indexX, PixelValue);
			}
		}
	}


}

void startPreprocess(itk::Image<signed short, (unsigned)3> * image, itk::Image<signed short, (unsigned)3> * output, int threshold){

	typedef itk::Image< signed short, (unsigned)3> ImageType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	long x = size[0];
	long y = size[1];
	long z = size[2];

	unsigned char *pData = NULL;
	try
	{
		pData = (unsigned char *)(new unsigned char[x*y*z]);
	}
	catch (...)
	{
		cout << "something went wrong";
		if (pData) { delete[]pData; pData = NULL; }
		return;
	}


	long mCount = y * x;
	
	for (long i = 0; i<z; i++)
	{
		for (long j = 0; j<y; j++)
		{
			for (long k = 0; k<x; k++)
			{
				long curpos = i * mCount + j*x + k;
				pData[curpos] = (unsigned char) getPixel(image, k, j, i);
				
			}
		}
	}
	
	pData = Preprocess(pData, x, y, z, 1, threshold);
	createITKImage(pData, output, x, y, z, threshold);
	cout << "ITK image created\n";
	fillHoles(output);
	cout << "Holes Filled\n";

}














//now the param includes the original image
void trueAnalyze(itk::Image<signed short, (unsigned)3> *image, char* outfilename, itk::Image<signed short, (unsigned)3> * origImage, int treeNumber){

	cout << "Analyze is starting\n";

	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	vector <pixPoint*> endPoints;

	vector<vector<pixPoint*> > skeletonPoints;
	//num of trees can be found by calling skeletonPoints.size()
	//num of pixels in a tree can be found by calling skeletonPoints[someNum].size();

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	int width = size[0];
	int height = size[1];
	int depth = size[2];


	signed short crntPoint;

	//Loop through each pixel to find every foreground pixel
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			for (int z = 0; z < depth; z++)
			{

				crntPoint = getPixel(image, x, y, z);

					if (crntPoint != 0){
						//if foreground pixel then check for number of neighbors 
						//if only 1 it is an end_point
						if (numberOfNeighbors(image, x, y, z) == 1){
							endPoints.push_back(new pixPoint(x, y, z, END_POINT));
						}
					}
			}

	bool *** visited = resetVisited(image);

	cout << "Visiting the skeletons\n";
	vector<graph*> skeletonGraphs;   

	//for every end point that is not already visit traverse the skeleton
	for (int i = 0; i < endPoints.size(); i++){

		if (visited[endPoints[i]->x][endPoints[i]->y][endPoints[i]->z] != true){
			graph * skelGraph = new graph(); 
			skeletonPoints.push_back(visitSkeleton(image, endPoints[i], visited, skelGraph));
			skeletonGraphs.push_back(skelGraph); 
		}
	}


	

	//bubble sort ascending order so the last tree is the largest
	//replace this with just inserting into a set because it is nlog(n) instead of n^2
	int* indexNum = new int[skeletonPoints.size()];
	for (int i = 0; i < skeletonPoints.size(); i++)
	{
		indexNum[i] = i;
	}


	int i, j;
	int temp;
	bool swapped = true;

	cout << "Sorting the trees\n";
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


	//beging retraversal test
	//we already know that the root will always be on the zeroeth place of any skeleton
	//and these visits and array for theimage happen to be for the entire image so they only need to be declared once. I'm thinking before the tree iteration and then we just use the tree iteration to handle the tree for the retraversal instead
	bool *** newImage = resetVisited(image);
	bool *** newVisited = resetVisited(image);
	vector<pixPoint*> retraversedSkel;
	vector<pixPoint*> watchList;

	//output certain number of largest trees to a swc file, will be a parameter

	cout << "Preparing for output\n";

	if (treeNumber >  skeletonPoints.size())
		treeNumber = skeletonPoints.size();

	long startVisitNum = 0;
	bool edgesDeleted = false;
	bool atLeastOneMerged = false;
	//do post processing and then print
	for (int i = 1; i <= treeNumber; i++)
	{
		
		do
		{
			//pass in image which is the skeletonized image
			edgesDeleted = loopCleanup(skeletonGraphs[indexNum[skeletonGraphs.size() - i]], skeletonPoints[indexNum[skeletonPoints.size() - i]], origImage); //also have the graph class as input
			if (edgesDeleted == true){
				atLeastOneMerged = true;
			}
		} while (edgesDeleted);

		spurCleanup(skeletonGraphs[indexNum[skeletonGraphs.size() - i]], skeletonPoints[indexNum[skeletonPoints.size() - i]], origImage, image); //also have the graph class as input

		//debug test
		atLeastOneMerged = true;

		//retraversal test
		if (atLeastOneMerged){
			fillNewImage(newImage, skeletonPoints[indexNum[skeletonPoints.size() - i]], watchList);
			linkConnectivitySWC(skeletonPoints[indexNum[skeletonPoints.size() - i]][0], retraversedSkel, newImage, newVisited, watchList, width, height, depth);
			outputTestSWC(retraversedSkel, outfilename, origImage, startVisitNum);
			startVisitNum += retraversedSkel.size();
			cout << retraversedSkel.size() << ' ' << watchList.size() << endl;
			retraversedSkel.clear();
			watchList.clear();
		}
		else{
			outputTestSWC(skeletonPoints[indexNum[skeletonPoints.size() - i]], outfilename, origImage, startVisitNum); //currently outputing the largest tree
			startVisitNum += skeletonPoints[indexNum[skeletonPoints.size() - i]].size();
		}

		//testing
		//will be these two lines below but i'm currently just testing output
		//outputSWC(skeletonPoints[indexNum[skeletonPoints.size() - i]], outfilename, origImage,startVisitNum); //currently outputing the largest tree
		//startVisitNum += skeletonPoints[indexNum[skeletonPoints.size() - i]].size();
	}

	cout << outfilename << " Current verion: Successfully written to swc file.";
	
	//4/7
	/*
	delete  visited;
	delete visitedRetraversal;
	*/
}//end trueAnalyze



//delete revisitSkeleton after this is done
void linkConnectivitySWC(pixPoint* root, vector<pixPoint*>& Points, bool *** image, bool *** visited, vector<pixPoint*>& watchList, int width, int height, int depth){
	if (Points.size() != 0){
		cerr << "We have an error! The vector is already filled\n";
		exit(-1);
	}
	int num = 0;

	if (root->parent != -1){
		cerr << "We have an error! The point is not a root\n";
		exit(-1);
	}

	if (root->visitNum != 1){
		cerr << "We have an error! The point is not a root\n";
		exit(-1);
	}
	root->pixType = 0;

	pixPoint* crntPoint = root;

	crntPoint->parent = -1;
	crntPoint->pixType = 0;

	pixPoint* neighbor;

	queue<pixPoint*> next;
	next.push(crntPoint);
	visited[crntPoint->x][crntPoint->y][crntPoint->z] = true;
	while (!next.empty()){

		num++;
		crntPoint = next.front();
		crntPoint->visitNum = num;
		next.pop();
		Points.push_back(crntPoint);
		int radius;


		//find all of the neighbors. If they are valid pixels and if they are also not visited then add them to the neighbor list and set them as visited. Otherwise just continue on
		for (int x = -1; x <= 1; x++){
			for (int y = -1; y <= 1; y++){
				for (int z = -1; z <= 1; z++){
					if (!(crntPoint->x + x < width && crntPoint->y + y < height && crntPoint->z + z < depth && crntPoint->x + x > 0 && crntPoint->y + y > 0 && crntPoint->z + z > 0)){
						continue;
					}

					if (image[crntPoint->x + x][crntPoint->y + y][crntPoint->z + z] == true && visited[crntPoint->x + x][crntPoint->y + y][crntPoint->z + z] == false){

						neighbor = new pixPoint(crntPoint->x + x, crntPoint->y + y, crntPoint->z + z, 0);

						//if they already have a radius give them a pixType value of 3
						radius = foundInWatch(watchList, neighbor->x, neighbor->y, neighbor->z);
						if (radius != -1){
							neighbor->pixType = 3;
							neighbor->radius = radius;
						}
						else{
							neighbor->pixType = 0;
						}

						neighbor->parent = num;
						next.push(neighbor); 
						visited[crntPoint->x + x][crntPoint->y + y][crntPoint->z + z] = true;
					}
				}
			}
		}

	}

}



//add all of the pixels to the new image data as long as they are not labeled to be ignored with -1 pixtype
//and populate a watch list of pixels that already have radii found
void fillNewImage(bool *** newImage, vector<pixPoint*>& Points, vector<pixPoint*>& watchList){
	for (int i = 0; i < Points.size(); i++){
		if (Points[i]->pixType != -1){ 
			newImage[Points[i]->x][Points[i]->y][Points[i]->z] = true;
			if (Points[i]->pixType == 3){
				watchList.push_back(Points[i]);
			}
		}
	}
}


//see if the watchlist contains the coordinates
int foundInWatch(vector<pixPoint*>& p, int x, int y, int z){

	for (int i = 0; i < p.size(); i++){
		if (p[i]->x == x && p[i]->y == y && p[i]->z == z){
			return p[i]->radius;
		}
	}
	return -1;

}






//Starting from the root point (decided by the parameter and assumed to be an endpoint), traverse all of the connecting skeleton points
//in a breadth first search. This ensures that slab pixels are contiguous in storage which allows for easier classifications of the edges and vertices.
vector<pixPoint*> visitSkeleton(itk::Image<signed short, (unsigned)3> *image, pixPoint* root, bool *** visited, graph * g){

	pixPoint* crntPoint = new pixPoint(root->x, root->y, root->z, root->pixType);  //dynamically allocate to deal with scope issues

	int num = 1;    //the visiting num. this starts at 1 for logical reasons so when refering to the vector of skeletonpoints it must be num - 1
	int count = 0;  //used in the slab portion of the code in order to determine if the next neighbor is an already visited junction
	int vnum = 0;   //the index of the vertex in the vector vertices

	visited[crntPoint->x][crntPoint->y][crntPoint->z] = true;
	crntPoint->visitNum = num;
	crntPoint->parent = -1;   //-1 parent means it does not exist. so this is the root

	queue<pixPoint*> pathQ;
	pathQ.push(crntPoint);    //start the root as the beginning of a path

	vector<pixPoint*> crntSkelPoints;         //the list of points that will be returned 
	crntSkelPoints.push_back(crntPoint);  

	vector<pixPoint*> neighbor;

	Edge* e;    //a temporary edge. this updated until it is complete and ready to be pushed

	//set up the edge and vertices with the root and prepare for traversal
	g->vertices.push_back(createVertex(image, crntPoint)); //vertex
	e = new Edge();
	e->v2 = NULL;
	e->v1 = g->vertices.back();
	g->vertices.back()->edges.push_back(e);  //test

	getNeighbors(image, neighbor, crntPoint);     //neighbor passed in as reference
	//all of the neighbors will only have x,y,z data at this point
	neighbor[0]->parent = crntPoint->visitNum;
	crntPoint = neighbor[0];

	neighbor.clear();   

	//start the modified breadth first traversal. Where the first slab in each edge is put into the queue and the queue is popped 
	//whenever the edge paths end (i.e. contact with a vertex or an end point)
	while (!pathQ.empty()) {

		//because multiple points can add their neighbors onto the queue we only consider the crnt points that are not visited
		if (visited[crntPoint->x][crntPoint->y][crntPoint->z] == true) {
			pathQ.pop();
			if (!pathQ.empty()){
				crntPoint = pathQ.front();
			}
			continue;
		}

		num++;
		crntPoint->visitNum = num;
		crntSkelPoints.push_back(crntPoint);
		visited[crntPoint->x][crntPoint->y][crntPoint->z] = true;
		getNeighbors(image, neighbor, crntPoint); //neighbor passed in by reference
		//all of the neighbors will only have x, y, z at this point

		if (neighbor.size() == 2){
			crntPoint->pixType = SLAB;

			count = 0;

			if (e->v2 != NULL){ //check for v2 != NULL because v2 will always be NULL until a new path is ready to be created. checking for parent junction lead to occassional abend because the Root and the junction could be adjacent and would only give a v1 to the edge before starting a new path (since endpoint to junction is not going to be a v1->v2 path)

				vnum = findInVertex(g->vertices, crntSkelPoints[crntPoint->parent - 1]);
				
				e = new Edge();
				e->v2 = NULL;
				e->v1 = g->vertices[vnum];		
				g->vertices[vnum]->edges.push_back(e);   //test
			}
			
			e->slab.push_back(crntPoint); //Add one pixel to the most current edge
			

			for (int i = 0; i < neighbor.size(); i++)
			{
				if (visited[neighbor[i]->x][neighbor[i]->y][neighbor[i]->z] == false)
				{
					neighbor[i]->parent = crntPoint->visitNum;

					count++;
					crntPoint = neighbor[i];

				}
			}

			if (count == 0){ //if the next neighbor is already visited, the path is done and next neighbor is junction

				//compare a neighbor against crntPoint parent in order to see which one is not the parent. The non-parent should be an already visited junction
				if (neighbor[0]->x == crntSkelPoints[crntPoint->parent - 1]->x
					&& neighbor[0]->y == crntSkelPoints[crntPoint->parent - 1]->y
					&& neighbor[0]->z == crntSkelPoints[crntPoint->parent - 1]->z){
					vnum = findInVertex(g->vertices, neighbor[1]);
					e->v2 = g->vertices[vnum];
					g->edges.push_back(e);

					g->vertices[vnum]->edges.push_back(e);   //test
				}
				else{
					vnum = findInVertex(g->vertices, neighbor[0]);
					e->v2 = g->vertices[vnum];
					g->edges.push_back(e);

					g->vertices[vnum]->edges.push_back(e);   //test
				}

				pathQ.pop();
				if (!pathQ.empty()){
					crntPoint = pathQ.front();
				}
			}

		}
		else if (neighbor.size() > 2){
			crntPoint->pixType = JUNCTION;

			//check the point against the list of vertices you have
			//create new vertex if this point is not on any vertex
			//otherwise add the already existing vertex to the current edge 
			//if the parent is a slab 
			vnum = findInVertex(g->vertices, crntPoint);
			if (vnum == -1){
				g->vertices.push_back(createVertex(image, crntPoint)); //vertex

				e->v2 = g->vertices.back(); 
				g->edges.push_back(e);

				g->vertices.back()->edges.push_back(e);   
			}
			else{

				//testing
				//added in on 3/12/2016 in order to solve the data issue. it seems that I have been declaring a seperate pixPoint* with the same xyz values in my createVertex function
				//so i am adding in this synchronizer little block of code in order to find the corresponding point and update its information

				for (int i = 0; i < g->vertices[vnum]->points.size(); i++){
					if (g->vertices[vnum]->points[i]->x == crntPoint->x && g->vertices[vnum]->points[i]->y == crntPoint->y && g->vertices[vnum]->points[i]->z == crntPoint->z){
						g->vertices[vnum]->points[i]->pixType = crntPoint->pixType;
						g->vertices[vnum]->points[i]->parent = crntPoint->parent;
						g->vertices[vnum]->points[i]->visitNum = crntPoint->visitNum;
					}
				}

				if (crntSkelPoints[crntPoint->parent - 1]->pixType == SLAB){
					e->v2 = g->vertices[vnum];
					g->edges.push_back(e);

					g->vertices[vnum]->edges.push_back(e);
				}
			}

			 
			for (int i = 0; i < neighbor.size(); i++)
			{
				if (visited[neighbor[i]->x][neighbor[i]->y][neighbor[i]->z] == false)
				{
					neighbor[i]->parent = crntPoint->visitNum;

					pathQ.push(neighbor[i]);

				}
			}

			pathQ.pop();				//pop here and at junctions because they are the end destinations of an edge
			if (!pathQ.empty()){
				crntPoint = pathQ.front();
			}

		}
		else{

			crntPoint->pixType = END_POINT;

			//If the parent is a SLAB, it is not possible for the end point to already be on a vertex so we make a new vertex
			if (crntSkelPoints[crntPoint->parent - 1]->pixType == SLAB){
				g->vertices.push_back(createVertex(image, crntPoint)); //vertex
				e->v2 = g->vertices.back(); //if we created a new vertex it means no one else has had contact yet and the only thing it is connected to so far is the parent (definitely a slab)
				g->edges.push_back(e);

				g->vertices.back()->edges.push_back(e);   //test
			}
			else {
				//otherwise we just update the pixel information for the corresponding pixelPoint contained on whatever vertex it may be on
				//testing
				//added in on 3/12/2016 in order to solve the data issue. it seems that I have been declaring a seperate pixPoint* with the same xyz values in my createVertex function
				//so i am adding in this synchronizer little block of code in order to find the corresponding point and update its information
				vnum = findInVertex(g->vertices, crntPoint);
				for (int i = 0; i < g->vertices[vnum]->points.size(); i++){
					if (g->vertices[vnum]->points[i]->x == crntPoint->x && g->vertices[vnum]->points[i]->y == crntPoint->y && g->vertices[vnum]->points[i]->z == crntPoint->z){
						g->vertices[vnum]->points[i]->pixType = crntPoint->pixType;
						g->vertices[vnum]->points[i]->parent = crntPoint->parent;
						g->vertices[vnum]->points[i]->visitNum = crntPoint->visitNum;
					}
				}
			}
			pathQ.pop();					//pop here and at junctions because they are the end destinations of an edge
			if (!pathQ.empty()){
				crntPoint = pathQ.front();
			}

		}

		neighbor.clear();
	} 

	return crntSkelPoints;

}//end visitSkeleton





bool *** resetVisited(itk::Image<signed short, (unsigned)3> *image){
	
	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

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



//find all of the neighbors and keep a count rather than updating the vector
int numberOfNeighbors(itk::Image<signed short, (unsigned)3> *image, int xCoord, int yCoord, int zCoord){

	int n = 0;

	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++)
			{

				if (x == 0 && y == 0 && z == 0)
					continue;

				if (getPixel(image, xCoord + x, yCoord + y, zCoord + z) != 0)
					n++;

			}
	return n;
}//end numberOfNeighbors



//get all of the pixels in the neighborhood and add them to a vector that is passed in by reference
//loop caution: for obvious reasons this does not clear out the neighbor vector when finished. 
void getNeighbors(itk::Image<signed short, (unsigned)3> *image, vector<pixPoint*> & neigh, pixPoint* p){



	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++)
			{

				if (x == 0 && y == 0 && z == 0)
					continue;

				if (getPixel(image, p->x + x, p->y + y, p->z + z) != 0)
					neigh.push_back(new pixPoint(p->x + x, p->y + y, p->z + z, UNDEFINED));

			}
}//end getNeighbors



//Get pixel from the specified image. It does check the bounds of the image. If an x,y,z coord is check that does not exist then 0 is returned
signed short getPixel(itk::Image<signed short, (unsigned)3> *image, int x, int y, int z){

	typedef itk::Image< signed short, (unsigned)3> ImageType;
	typedef signed short PixelType;

	//This sets the region to the entire image
	ImageType::RegionType region = image->GetLargestPossibleRegion();

	ImageType::SizeType size = region.GetSize();

	int width = size[0];
	int height = size[1];
	int depth = size[2];

	ImageType::IndexType pixelIndex;
	pixelIndex[0] = x;
	pixelIndex[1] = y;
	pixelIndex[2] = z;

	if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth)
		return image->GetPixel(pixelIndex);
	else{
		return 0;
	}
}//end getpixel



void outputTestSWC(vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> * origImage, long startVisitNum){


	ofstream outFile;
	
	if (startVisitNum == 0) //first tree
	{
		outFile.open(outfilename, ios::trunc);
		outFile << "#name reconstruction3d \n#comment \n##n,type,x,y,z,radius,parent\n";
	}
	else //subsequent trees
		outFile.open(outfilename, ios::app);

	for (int i = 0; i < Points.size(); i++)
	{
		if (Points[i] == NULL)
		{
			continue;
		}
		
		//debug int pixType;   // -1 - "deleted" / 0 - slab / 1 - junction / 2 - endpoint / 3 - radius already found. mergePoint
		if (Points[i]->pixType == 0)
			Points[i]->radius = findRadius(Points[i], origImage);
		else if (Points[i]->pixType != 3)
			Points[i]->radius = findRadius(Points[i], origImage);
			
		bool skip = false;
		if (Points[i]->pixType == -1){
			//uncomment line below if skip is set to false or commented out. Would be done if it is desired to see the skeleton before merging
			//Points[i]->radius = 2;

			skip = true;
		}
		
		if (!skip){
			if (Points[i]->parent == -1)  //root of a separate tree
				outFile << Points[i]->visitNum + startVisitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << Points[i]->radius << ' ' << -1 << endl;

			else
				outFile << Points[i]->visitNum + startVisitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << Points[i]->radius << ' ' << Points[i]->parent + startVisitNum << endl;
		}
	}

	//test debug
	//This outputs the new pixels created by the merging process to the swc file.
	//purely for testing/debugging purposes.
	
	outFile.close();
	


}//end outputSWC

void outputSWC(vector<pixPoint*> & Points, char* outfilename, itk::Image<signed short, (unsigned)3> * origImage, long startVisitNum){


	ofstream outFile;

	if (startVisitNum == 0) //first tree
	{
		outFile.open(outfilename, ios::trunc);
		outFile << "#name reconstruction3d \n#comment \n##n,type,x,y,z,radius,parent\n";
	}
	else //subsequent trees
		outFile.open(outfilename, ios::app);

	for (int i = 0; i < Points.size(); i++)
	{
		if (Points[i] == NULL)
		{
			continue;
		}
		
		if (Points[i]->pixType != 3){
			Points[i]->radius = findRadius(Points[i], origImage);
		}
		if (Points[i]->parent == -1)  //root of a separate tree
			outFile << Points[i]->visitNum + startVisitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << Points[i]->radius << ' ' << -1 << endl;

		else
			outFile << Points[i]->visitNum + startVisitNum << ' ' << 7 << ' ' << Points[i]->x << ' ' << Points[i]->y << ' ' << Points[i]->z << ' ' << Points[i]->radius << ' ' << Points[i]->parent + startVisitNum << endl;
	}

	outFile.close();
	


}//end outputSWC

//Create a vertex at the crntPnt parameter and include all of the junctions and endpoints that are connected to it through junctions.
//In other words, its neighbors that are junctions and/or endpoints and the neighbors of those junctions that are junctions and/or endpoints and so on until there are none left
Vertex* createVertex(itk::Image<signed short, (unsigned)3> *image, pixPoint* crntPnt){

	Vertex* v1 = new Vertex();

	v1->points.push_back(crntPnt);

	queue<pixPoint*> neighborQue;

	vector<pixPoint*> tempNeigh;
	vector <pixPoint*> neighbors;
	neighborQue.push(crntPnt); //crnt is passed in

	//gather the neighbor of neighbors with a search that finds all connected points that have more than 2 neighbors or only one neighbor
	while (!neighborQue.empty()){
		crntPnt = neighborQue.front();

		getNeighbors(image, neighbors, crntPnt);
		for (int i = 0; i < neighbors.size(); i++){

			//populate tempNeigh with pixPoints and then check the number of neighbors. neighbors > 3 = vertex point (add to vertex). neighbors == 1 end point (add to vertex). neighbors == 2 slab (do not add to vertex)
			getNeighbors(image, tempNeigh, neighbors[i]);
			if (tempNeigh.size() > 2){ 
				//if number of neighbors is greater than 2 add all points to the vertex that are not already contained. 
				//Add these points to the queue to check their neighbors and repeat the process
				if (!v1->contains(neighbors[i])){
					v1->points.push_back(neighbors[i]);
					neighborQue.push(neighbors[i]);
				}
			}
			else if (tempNeigh.size() == 1){
				//if number of neighbors is 1 then add the point to the vertex and continue to the next point on the queue
				//having a neighborhood the size of 1 means that apoint is an end point
				if (!v1->contains(neighbors[i])){
					v1->points.push_back(neighbors[i]);
				}
			}

			tempNeigh.clear(); 
		}
		neighbors.clear();
		neighborQue.pop();
	}

	return v1;
}


int findInVertex(vector<Vertex*>& vertices, pixPoint* pnt){
	//loop through the vertex
	//if pnt is found return its index location
	//otherwise return -1
	for (int i = 0; i < vertices.size(); i++){
		if (vertices[i]->contains(pnt)){
			return i;
		}
	}
	return -1;
}


//Find the radius of a point
int findRadius(pixPoint* crntPnt, itk::Image<signed short, (unsigned)3> * image){ //added default value for radius to simplify old codes' calls of this function
	
	int radius = 0,
		val = 1,
		x,
		y;

	int count = 0;

	do {  

		radius++;

		//check every pixel at distance "radius" from the point
		//if the val of the pixel is 0 then increment a count
		for (x = -radius; x <= radius; x++){
			y = sqrt(((radius * radius) - (x * x)));
			val = getPixel(image, crntPnt->x + x, crntPnt->y + y, crntPnt->z);
			if (val == 0){
				count++;
			}

			val = getPixel(image, crntPnt->x + x, crntPnt->y - y, crntPnt->z);
			if (val == 0){
				count++;
			}

		} //possibly should change below line to area comparison instead of perimeter. look at later
	} while ((double)count / (6.28 * radius) < 0.16); //stop loop when the count surpasses 16% of the current perimeter

	return radius;
	

}

//compares every branch that has an endpoint with the radius of the junction to eliminate spurs
void spurCleanup(graph * g, vector<pixPoint*> & Points, itk::Image<signed short, (unsigned)3> * origImage, itk::Image<signed short, (unsigned)3> * image){
	
	int sum;

	sum = 0;
	
	
	//YOU MIGHT NOT WANT TO DELETE THE ROOT EVEN IF IT IS A SPUR BECAUSE OF CONNECTIVITY
	//SO THATS WHY I COMMENTED IT OUT FOR NOW
	//AND IT"S IN ALL CAPS SO THAT YOU CAN FIND IT EASIER LATER
	
	/*
	//Because vertices may have multiple points we find the sum
	for (int u = 0; u < g->vertices[0]->edges[0]->v2->points.size(); u++){
		sum += findRadius(g->vertices[0]->edges[0]->v2->points[u], origImage);
	}

	//the sum is then divided to become an average
	//we compare the length of the branch we are looking to delete with this average
	if ((double)g->vertices[0]->edges[0]->slab.size() < 1.30*((double)sum / g->vertices[0]->edges[0]->v2->points.size())){
		for (int cnt = 0; cnt < g->vertices[0]->edges[0]->slab.size(); cnt++){
			//Point is then marked null. Output swc checks for null and skips over any points that are null during it's output loop
			Points[g->vertices[0]->edges[0]->slab[cnt]->visitNum - 1] = NULL;
		}
		for (int i = 0; i < g->vertices[0]->points.size(); i++){
			Points[g->vertices[0]->points[i]->visitNum -1] = NULL;
		}
	}
	*/

	signed short PixelValue = 0;
	itk::Index<3> indexX;

	//v1 was always selected as the junction and v2 as the endpoint because of the way the program iterates through the skeleton. it will always expand outward from the inside
	for (int i = 1; i < g->vertices.size(); i++){
		if (g->vertices[i]->edges.size() == 1){
			sum = 0;
			//sum for the average
			for (int u = 0; u < g->vertices[i]->edges[0]->v1->points.size(); u++){
				sum += findRadius(g->vertices[i]->edges[0]->v1->points[u], origImage);
			}

			//branch length compared to average and then removed by marking null if appropriate
			if ((double)g->vertices[i]->edges[0]->slab.size() < 1.30*((double)sum / g->vertices[i]->edges[0]->v1->points.size())){
				for (int cnt = 0; cnt < g->vertices[i]->edges[0]->slab.size(); cnt++){

					indexX[0] = g->vertices[i]->edges[0]->slab[cnt]->x;
					indexX[1] = g->vertices[i]->edges[0]->slab[cnt]->y;
					indexX[2] = g->vertices[i]->edges[0]->slab[cnt]->z;
					image->SetPixel(indexX, PixelValue);

					Points[g->vertices[i]->edges[0]->slab[cnt]->visitNum - 1]->pixType = -1;
				}
				for (int u = 0; u < g->vertices[i]->edges[0]->v2->points.size(); u++){

					indexX[0] = Points[g->vertices[i]->edges[0]->v2->points[u]->visitNum - 1]->x;
					indexX[1] = Points[g->vertices[i]->edges[0]->v2->points[u]->visitNum - 1]->y;
					indexX[2] = Points[g->vertices[i]->edges[0]->v2->points[u]->visitNum - 1]->z;
					image->SetPixel(indexX, PixelValue);

					Points[g->vertices[i]->edges[0]->v2->points[u]->visitNum -1]->pixType = -1;
				}
			} 
			
		}
	}

}



//this is the real post processing function
bool loopCleanup(graph * g, vector<pixPoint*> & Points, itk::Image<signed short, (unsigned)3> * image){


	cerr << "Loop Cleanup Started\n";
	//find the loopEdges. Edges that are in at least one loop
	//all loopEdges found are pushed onto the container
	vector<Edge*> loopEdges;
	findLoopEdges(g, loopEdges);

	if (loopEdges.size() == 0){
		cout << "No Loops Found\n";
		return false;
	}
	
	//find all edges in the loop and store it
	//loop one time per loopEdge
	vector< vector <Edge*> > fullLoops; //the vector that contains all of built loops

	//for each loopEdge
	for (int i = 0; i < loopEdges.size(); i++){
		
		//reinitialize all of the distances
		//parents do not need to be reset because they are never checked without being initialized in the algorithms
		for (int u = 0; u < g->vertices.size(); u++){
			g->vertices[u]->distance = 99999999;
		}
		
		//call full loops which builds a loop out of every loopEdge
		fullLoops.push_back(buildLoops(loopEdges[i])); 
	}

	
	vector<Edge*> LSPath; //the longest shortest path
	graph deleteGraph;

	mergeLoops(fullLoops, g, image, Points);

	loopEdges.clear();

	for (int i = 0; i < fullLoops.size(); i++){
		fullLoops[i].clear();
	}
	fullLoops.clear();

	//end testing
	cout << "Loop Cleanup Ended\n";


	return false; //testing only for now just to make sure it only loops once
} 



	/*
	* rudimentary psuedocode for dev purposes
	* start at v1.
	* add v2 to queue
	* add edge to parentList queue
	* 
	* while queue is not empty loop starts here
	* we are now at v2
	* 
	* if v2 is visited, then add the front parent to the loopEdge vector and pop v2 from the queue and continue
	* otherwise just mark v2 as visited
	*  (v2 will not already be visited so it wont be a problem)
	* 
	* find all of the edges associated with v2 that are not already visited and the parent and for each
	* find opposite vertex and push that onto the queue along with the current edge to the parentList
	* 
	* pop queue
	* pop parentList queue
	* goto while
	*/
void findLoopEdges(graph * g, vector<Edge*> & loopEdges){
	
	vector<Vertex*> visitList;
	queue<Vertex*> vertexQ;
	
	//v1 is the root(end point) so we can skip past it
	Vertex* crntVertex;
	
	//assign the edge that is connected to the loop as the parent to the next vertex in the graph.
	if (g->vertices[0]->edges[0] == NULL){
		return;
	}
	if (g->vertices[0]->edges[0]->v2 == NULL){
		return;
	}

	g->vertices[0]->edges[0]->v2->parent = g->vertices[0]->edges[0];
	vertexQ.push(g->vertices[0]->edges[0]->v2);

	//parentQ.push(g->vertices[0]->edges[0]); no longer needed but left in the code to illustrate what i decided not to do because i realized that i was using improper logic
	
	//while the queue is not empty do a breadth first search through the graph in order to find all of the edges that are contained in loops.
	//when a visited vertex is on the front of the queue, add its parent to the loopEdges list. 
	while (!vertexQ.empty()){
		crntVertex = vertexQ.front();
		
		//if crntVertex is already visited, add it's parent to the loopEdges
		if (visited(visitList, crntVertex))
		{
			loopEdges.push_back(crntVertex->parent);
			vertexQ.pop();
			//continue because there is no need to check for neighbors because none of them will be added to the queue
			continue;
		}

		//instead of a visit list I am considering just adding that information to the class of Vertex and Edge
		//this would save a lot of time but at the moment i am just thinking about it
		visitList.push_back(crntVertex);
		
		//find all of the edges associated with v2 that are not already visited
		//find opposite vertex and push that onto the queue along with the current edge to the parentList
		//for each of the crntVertex's branches
		for (int i = 0; i < crntVertex->edges.size(); i++){
			//check if the opposite vertex is already visited
			if (!visited(visitList, crntVertex->edges[i]->getOppositeVertex(crntVertex))){
				//if it is not already visited, set the opposite vertex's parent to the crntBranch that we are looking at
				crntVertex->edges[i]->getOppositeVertex(crntVertex)->parent = crntVertex->edges[i];
				//push that vertex onto the queue
				vertexQ.push(crntVertex->edges[i]->getOppositeVertex(crntVertex));
				//parentQ.push(crntVertex->edges[i]);
			}
		}
		
		//we can pop here because the vertex has already been processed
		vertexQ.pop();
	}

}

//a visited list is not needed for Dijkstra's algorithm when computing because the distance check
//already does that for you
vector<Edge*> buildLoops(Edge* loopEdge){
	


	//create a multiset container to hold and sort Vertices
	bool(*fn_pt)(Vertex* ,Vertex*) = fncomp;
	std::multiset<Vertex*,bool(*)(Vertex* ,Vertex* )> adjVerts (fn_pt);
	//multiset<Vertex*> adjVerts;
	
	Vertex* target;
	Vertex* crntVert;
	
	int altDistance;

	//we define v1 of the edge to be the source or crntVertex and initialize the distance along with labeling its parent
	crntVert = loopEdge->v1;
	crntVert->distance = 0;
	crntVert->parent = loopEdge;
	
	//we define v2 of the edge to be the target
	target = loopEdge->v2;
	
	//insert the source vertex into the set (which we will access from the front) to start the loop
	adjVerts.insert(crntVert);
	while (!adjVerts.empty()){
	
		//*adjVerts should be the vertex with the smallest distance
		//confirmed
		crntVert = *adjVerts.begin();

		//after retrieving the shortest vertex we no longer need to keep it in the container
		adjVerts.erase(adjVerts.begin());
		
		//for each of the crntVertex's branches
		for (int i = 0; i < crntVert->edges.size(); i++){
			if (crntVert->edges[i] != crntVert->parent){
				//if the currentVertex is not the current parent then we find the alternative distance to the opposite vertex
				altDistance = crntVert->distance + crntVert->edges[i]->slab.size();
				
				if ( altDistance < crntVert->edges[i]->getOppositeVertex(crntVert)->distance) {
					
					//if the alternative distance is less than the current distance to the oppositeVertex then we replace the distance and set a new parent
					crntVert->edges[i]->getOppositeVertex(crntVert)->distance = altDistance;
					crntVert->edges[i]->getOppositeVertex(crntVert)->parent = crntVert->edges[i];
					
					//we also insert the opposite vertex into the container to be visited later possibly
					adjVerts.insert(crntVert->edges[i]->getOppositeVertex(crntVert));
					if (crntVert->edges[i]->getOppositeVertex(crntVert) == target){
						
						//if we have reached the target then we create an empty container and send it to reverseIterate, which will build the loop
						vector<Edge*> builtLoop;
						reverseIterate(loopEdge, crntVert->edges[i]->getOppositeVertex(crntVert), builtLoop); //follow the parent list backwards while filling one element of the builtLoop vector until you hit the original edge
						adjVerts.clear();
						
						//end the loop and return the container of the loop
						return builtLoop;
					}
				}
			}
		}
	}
		
		
	
}



void reverseIterate(Edge* targetEdge, Vertex* vert, vector<Edge*> & builtLoop){
	while (vert->parent != targetEdge){
		
		builtLoop.push_back(vert->parent);
		vert = vert->parent->getOppositeVertex(vert);
	}
	
	//test for null because this function is also used with the vertex pair call
	if (vert->parent != NULL){
		builtLoop.push_back(vert->parent);
	}
}


bool visited( vector<Vertex*> & vList, Vertex* vert){
	
	for (int i = 0; i < vList.size(); i++){
		
		if (vList[i] == vert)
			return true;
	}
	
	return false;
	
} 



//for the initial psuedocode writing you are just passing through each loop once for simplicities sake. But in the future you will be adding adjacent loops and such
void mergeLoops(vector< vector <Edge*> > fullLoops, graph * g, itk::Image<signed short, (unsigned)3> * image, vector<pixPoint*>& Points){

	//test debug
	int numSuccess = 0;
	int numFail = 0;
	int percentSum = 0;

	//place holder. will be used to delete edges and vertices
	graph toBeDeleted;

	for (int i = 0; i < fullLoops.size(); i++){

		//get the midPoint.
		pixPoint centerPoint;
		findMidPoint(fullLoops[i], centerPoint);
		
		//if (centerPoint.x == 0 && centerPoint.y == 0 && centerPoint.z == 0){
		//	cout << "we got one\n";
		//	continue;
		//}

		listOfCoordinates LOC;
		//find the percentage of pixels that are background intensity
		int bgPercent = flood2DHole(fullLoops[i], image, centerPoint, LOC);

		percentSum += bgPercent;
		//if bgPercent is lower than our threshold then the loop is merge-able
		if (bgPercent < 25)
		{
			//test debug
			numSuccess++;
			merge(fullLoops[i], g, image, centerPoint, LOC, &toBeDeleted, Points);
		}


		for (int i = 0; i < toBeDeleted.edges.size(); i++){
			for (int u = 0; u < toBeDeleted.edges[i]->slab.size(); u++){
				toBeDeleted.edges[i]->slab[u]->pixType = -1;
			}
		}

		for (int i = 0; i < toBeDeleted.vertices.size(); i++){
			for (int u = 0; u < toBeDeleted.vertices[i]->points.size(); u++){
				toBeDeleted.vertices[i]->points[u]->pixType = -1;
			}
		}

		LOC.x.clear();
		LOC.y.clear();

		//test debug
		numFail++;
	}
	if (numFail > 0){
		int average = percentSum / (numFail);
		cout << average << " " << numSuccess << " " << numFail << endl;
	}
}





void findMidPoint(vector<Edge*> loop, pixPoint& centerPoint){

	int xSum = 0,
		ySum = 0,
		zSum = 0,
		num = 0,
		x,
		y,
		z,
		xBalance = 0,
		yBalance = 0;

	for (int i = 0; i < loop.size(); i++){
		
		for (int u = 0; u < loop[i]->slab.size(); u++){
			
			xSum+=loop[i]->slab[u]->x;
			ySum+=loop[i]->slab[u]->y;
			zSum+=loop[i]->slab[u]->z;
			num++;
		}
	}
	x = xSum/num;
	y = ySum/num;
	z = zSum/num;

	vector<int> xCoords;
	vector<int> yCoords;

	//at the moment you are disregarding any centers that end up outside of the loop
	for (int i = 0; i < loop.size(); i++){
		
		for (int u = 0; u < loop[i]->slab.size(); u++){
			
			if (x == loop[i]->slab[u]->x){
				yCoords.push_back(loop[i]->slab[u]->y);
			}
			if (y == loop[i]->slab[u]->y){
				xCoords.push_back(loop[i]->slab[u]->x);
			}
			
		}
	}

	//test later you need to find out if you need to deal with the irregular loops or not
	//if (xBalance == 0 && yBalance == 0){
		centerPoint.x = x;
		centerPoint.y = y;
		centerPoint.z = z; 
	/*}
	else {
		centerPoint.x = 0;
		centerPoint.y = 0;
		centerPoint.z = 0;
	}*/



}



int flood2DHole(vector<Edge*> loop, itk::Image<signed short, (unsigned)3> * image, pixPoint& centerPoint, listOfCoordinates & LOC){
	
	int lowestX = 999999,
		lowestY = 999999,
		highestX = 0,
		highestY = 0,
		highestZ = 0,
		lowestZ = 999999;

	for (int i = 0; i < loop.size(); i++){
		for (int u = 0; u < loop[i]->slab.size(); u++){
			
			if (loop[i]->slab[u]->x < lowestX){
				lowestX = loop[i]->slab[u]->x;
			}
			if (loop[i]->slab[u]->x > highestX){
				highestX = loop[i]->slab[u]->x;
			}

			if (loop[i]->slab[u]->y < lowestY){
				lowestY = loop[i]->slab[u]->y;
			}
			if (loop[i]->slab[u]->y > highestY){
				highestY = loop[i]->slab[u]->y;
			}
			if (loop[i]->slab[u]->z > highestZ){
				highestZ = loop[i]->slab[u]->z;
			}
			if (loop[i]->slab[u]->z < lowestZ){
				lowestZ = loop[i]->slab[u]->z;
			}
		}
	}

	int totalCount = 0;
	int bgCount = 0;


	bool borderPoint = false;
	bool prevBorderPoint = false;
	bool inside = false;
	//for (int j = lowestZ; j <= highestZ; j++){
		for (int i = lowestY; i <= highestY; i++){
			for (int u = lowestX; u <= highestX; u++){
				if (getPixel(image, u, i, centerPoint.z) == 0){
					bgCount++;
				}
				totalCount++;

				//if we hit a border point we first check if the previous point was also a border point. If it was then we know that are still on the same border point so it is still considered to be inside.
				//if the previous point was not then we flip the value of bool inside because it means we hit a new edge of the shape.
				borderPoint = isBorderPoint(loop, u, i);
				if (borderPoint == true){
					if (prevBorderPoint == false){

						if (inside == true){
							inside = false;
						}
						else{
							inside = true;
						}

					}
				}
				prevBorderPoint = borderPoint;

				if (inside){
					LOC.x.push_back(u);
					LOC.y.push_back(i);
				}
				else if (borderPoint){ //we have this here because we still consider borderPoints to be inside even if they are transitioning to the outside of the shape
					LOC.x.push_back(u);
					LOC.y.push_back(i);
				}

			}
		}
	//}
	//multiply by 100 to just have an int represent the whole number percentage
	bgCount *= 100;
	return (bgCount/totalCount);

}

//iterate through all of the points of the loop until either the end or a point matching the x and y value is found. If one isn't found, then return false
bool isBorderPoint(vector<Edge*> loop, int x, int y){

	for (int i = 0; i < loop.size(); i++){
		for (int u = 0; u < loop[i]->slab.size(); u++){
			if (loop[i]->slab[u]->x == x && loop[i]->slab[u]->y == y){
				return true;
			}
		}
	}
	return false;
}



//set edges as things to be deleted (like we did for prune loop)
void merge( vector <Edge*>& currentLoop, graph * g, itk::Image<signed short, (unsigned)3> * image, pixPoint& centerPoint, listOfCoordinates& LOC, graph * toBeDeleted, vector<pixPoint*>& Points){
	
	vector<Vertex*> verts;

	for (int i = 0; i < currentLoop.size(); i++){
		
		if (!visited(verts, currentLoop[i]->v1)){
			if (outsideLoopConnection(currentLoop, currentLoop[i]->v1, toBeDeleted, LOC)){
				verts.push_back(currentLoop[i]->v1);
			}
			else{
				toBeDeleted->vertices.push_back(currentLoop[i]->v1);
			}
			
		}
		if (!visited(verts, currentLoop[i]->v2)){
			if (outsideLoopConnection(currentLoop, currentLoop[i]->v2, toBeDeleted, LOC)){
				verts.push_back(currentLoop[i]->v2);
			}
			else{
				toBeDeleted->vertices.push_back(currentLoop[i]->v2);
			}
		}
		
		toBeDeleted->edges.push_back(currentLoop[i]);

	}
	
	Vertex* centerVert = new Vertex();
	pixPoint* cPoint = new pixPoint(centerPoint.x, centerPoint.y, centerPoint.z, 2);
	centerVert->points.push_back(cPoint); //make sure to make centerPoint dynamic or have a way around it

	//i'm thinking for now that the best way to connect it in the swc file would be to just have one edge lead into the center and have the center expand outward at every other edge
	//because it's already a long shot to even assume that we can connect everything so this will at least have it connected to the tree in some way
	for (int i = 0; i < verts.size(); i++){
		
		//only for vertices that are connected to OUTSIDE of the loop because there can be spurs on the inside
		//toBeAdded is going to be a cumulative list of the edges and vertices that need to be added to the graph
		
		//create edge and pixels
		Edge* newEdge = new Edge();
		makeNewEdge(centerVert, verts[i], newEdge, Points);//the list of pixels to be outputted for swc //i think moving one pixel closer in ever direction is the most valid approach right now

		//attach edge to both verts
		verts[i]->edges.push_back(newEdge);
		centerVert->edges.push_back(newEdge);

		//attach verts[i], centerVert, and newEdge to the graph and add it to the points vertex
		g->vertices.push_back(verts[i]);
		
		//it is important to add this vertex to the points vector because otherwise you lose connectivity for the retraversal
		g->vertices.push_back(centerVert);
		Points.push_back(centerVert->points[0]);

		g->edges.push_back(newEdge);
	}
	
	//initialize the radii of the points on the loop. set the pixType to 3 so that the radius of them isn't found again redundantly
	for (int i = 0; i < currentLoop.size(); i++){
		for (int u = 0; u < currentLoop[i]->slab.size(); u++){
			currentLoop[i]->slab[u]->radius = findRadius(currentLoop[i]->slab[u], image);
			currentLoop[i]->slab[u]->pixType = 3;
		}
	}


	findNewRadius(centerVert->points[0], image, LOC, currentLoop);
	for (int i = 0; i < centerVert->edges.size(); i++){
		for (int u = 0; u < centerVert->edges[i]->slab.size(); u++){
			findNewRadius(centerVert->edges[i]->slab[u], image, LOC, currentLoop); //i might actually just use the simplified box that i used before hand as the minimum radius. or i coul use the percentage result from that and just add that to the threshold...hmmm
		}
	}

	//graph->verts.push_back(centerVert);

	
}




//check and see if the loop has ANY outside connection while also taking care of any inside edges/vertices that are not a part of the loop
//does not take into account edges that are not on the same z plane at the moment
bool outsideLoopConnection(vector<Edge*>& currentLoop, Vertex* V, graph * toBeDeleted, listOfCoordinates& LOC){

	Vertex* v2;
	bool outside = false;

	for (int i = 0; i < V->edges.size(); i++){//to-do:
		if (!visited(currentLoop, V->edges[i])/* && !visited(toBeDeleted->edges, V->edges[i])*/){ //i dont see anything wrong with adding these but i want to make sure the simple version works before adding recursion. but the idea is that the recursion will add the ability to delete all edges and vertices inside of the loop
			v2 = V->edges[i]->getOppositeVertex(V);
			if (!visited(LOC, v2->points[0]->x, v2->points[0]->y)){
				outside = true;
			}
			else{
				toBeDeleted->edges.push_back(V->edges[i]);
				toBeDeleted->vertices.push_back(v2);

				//to-do:
				//outsideLoopConnection(currentLoop, v2, toBeDeleted, LOC);
			}
		}
	}
	return outside;
}


bool visited(vector<Edge*> & eList, Edge* e){

	for (int i = 0; i < eList.size(); i++){

		if (eList[i] == e)
			return true;
	}

	return false;

}



void findNewRadius(pixPoint* crntPnt, itk::Image<signed short, (unsigned)3> * image, listOfCoordinates& LOC, vector<Edge*>& currentLoop){ //added default value for radius to simplify old codes' calls of this function
	
	crntPnt->pixType = 3;
	int radius = 0,
		val = 1,
		x,
		y;

	int count = 0;

	do {

		radius++;

		//check every pixel at distance "radius" from the point
		//if the val of the pixel is 0 then increment a count
		for (x = -radius; x <= radius; x++){
			y = sqrt(((radius * radius) - (x * x)));
			val = getPixel(image, crntPnt->x + x, crntPnt->y + y, crntPnt->z);
			if (val == 0){
				if (!visited(LOC, crntPnt->x + x, crntPnt->y + y)){
					if (!inRadius(currentLoop, crntPnt->x + x, crntPnt->y + y)){
						count++;
					}
				}
			}

			val = getPixel(image, crntPnt->x + x, crntPnt->y - y, crntPnt->z);
			if (val == 0){
				if (!visited(LOC, crntPnt->x + x, crntPnt->y - y)){
					if (!inRadius(currentLoop, crntPnt->x + x, crntPnt->y - y)){
						count++;
					}
				}
			}

		} //possibly should change below line to area comparison instead of perimeter. look at later
	} while ((double)count / (6.28 * radius) < 0.10); //stop loop when the count surpasses 10% of the current perimeter. /10% was chosen this time because we already have hole tolerance with the merge loops algorithm. Attempting to add an extra threshold could push the radius too far. Although there should still be some leeway. Testing for best value

	crntPnt->radius = radius;

}

bool inRadius(vector<Edge*>& currentLoop, int x, int y){
	int distance = 0;
	int x2 = 0;
	int y2 = 0;
	for (int i = 0; i < currentLoop.size(); i++){
		for (int u = 0; u < currentLoop[i]->slab.size(); u++){

			x2 = currentLoop[i]->slab[u]->x - x;
			y2 = currentLoop[i]->slab[u]->y - y;
			distance = sqrt((x2*x2) + (y2*y2));

			if (distance < currentLoop[i]->slab[u]->radius){
				return true;
			}
		}
	}

	return false;
}

bool visited(listOfCoordinates& LOC, int x, int y){

	for (int i = 0; i < LOC.x.size(); i++){
		if (LOC.x[i] == x && LOC.y[i] == y){
			return true;
		}
	}
	return false;
}



void makeNewEdge(Vertex* v1, Vertex* v2, Edge* newEdge, vector<pixPoint*>& Points){//, vector<pixPoint*> pixelPoints){ //the list of pixels to be outputted for swc
	
	newEdge->v1 = v1;
	newEdge->v2 = v2;

	pixPoint* start = v1->points[0];
	pixPoint* finish = v2->points[0];

	bool notThereYet = true;
	pixPoint* crntPixel = new pixPoint(start->x, start->y, start->z, 1);

	//test debug
	if (crntPixel->x == finish->x && crntPixel->y == finish->y && crntPixel->z == finish->z){
		cout << "This was strange" << crntPixel->x << ' ' << finish->x << ' ' << crntPixel->y << ' ' << finish->y << ' ' << crntPixel->z << ' ' << finish->z << endl;
	}

	//adjust each dimension by one in the correct direction until the newest pixel is at the target
	while (notThereYet){
		if (crntPixel->x < finish->x){
			crntPixel->x++;
		}
		else if (crntPixel->x > finish->x){
			crntPixel->x--;
		}

		if (crntPixel->y < finish->y){
			crntPixel->y++;
		}
		else if (crntPixel->y > finish->y){
			crntPixel->y--;
		}

		if (crntPixel->z < finish->z){
			crntPixel->z++;
		}
		else if (crntPixel->z > finish->z){
			crntPixel->z--;
		}


		if (crntPixel->x == finish->x && crntPixel->y == finish->y && crntPixel->z == finish->z){
			notThereYet = false;
		}
		else{
			newEdge->slab.push_back(crntPixel);
			Points.push_back(crntPixel);
			pixPoint tmp(crntPixel->x, crntPixel->y, crntPixel->z, 1);
			crntPixel = new pixPoint(tmp.x, tmp.y, tmp.z, 1);
		}
	}

}












