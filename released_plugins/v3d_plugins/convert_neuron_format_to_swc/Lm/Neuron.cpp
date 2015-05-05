#include "Neuron.h"
#include "Limit.h"
#include "Func.h"
#include <fstream>
#include <cstdlib>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "ParameterList.h"
#include "Random.h"
//#include <direct.h>
#include <sstream>
#include <stdio.h>
#include <sys/stat.h>

#include <unistd.h>
#include "pca.h"
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define SIGN(a, b) ( (b) < 0 ? -fabs(a) : fabs(a) )
using namespace std;
Neuron::Neuron() {
	setDefaults();
}
;

void Neuron::setDefaults() {

	d[0][0] = 0;

	ifstream* in_d;

	soma = NULL;
	nrSeg = 0;
	lp = NULL;
	setShrinkX(1);
	setShrinkY(1);
	setShrinkZ(1);

	char def[50];
	strcpy(def, "empty");
	name = def; //strcpy(name,def);

	oriented = false;

}
;

void Neuron::init() {

	cerr<<"init \n";
	findDuplicate(soma);

	buildFathers(soma, soma);


	//insert a new compartment for SWC conversions for single soma files only. This modification helps in converting the SWCs to .hoc files. sri 01/07/2010.
	//Check if the cell has one stem or more. If next2 is null, then single stem, otherwise, two stems. SP on 02/25/11.
	if (soma->getType() == 1 && soma->getNext2() == NULL
			&& !(soma->getNext1()->getType() == 1)) {

		cerr
				<< "single soma..adding new soma compartment to a single stemmed existing soma point"
				<< "\n";

		addCylinderSoma(soma);

	} else if (soma->getType() == 1 && !(soma->getNext1()->getType() == 1
			|| soma->getNext2()->getType() == 1)) {
		cerr
				<< "single soma..adding new soma compartment to a two or more stemmed soma point"
				<< "\n";
		cerr << "soma:" << soma->getType() << " " << "next1:"
				<< soma->getNext1()->getType() << " " << "next2:"
				<< soma->getNext2()->getType() << "\n";
		addCylinderSoma(soma);

	}



	renumerateId();




	setOrder(soma);



	setParameters(soma);



}
;

//add a new soma to the existing soma as the beginning point to convert a sphere to cylinder approximation of soma part.
// modified by sridevi on 12/08/2010
void Neuron::addCylinderSoma(Segment* soma) {
	if (soma->getType() != 1) {
		if (soma->getNext1() != NULL)
			addCylinderSoma(soma->getNext1());
		if (soma->getNext2() != NULL)
			addCylinderSoma(soma->getNext2());
	} else {
		cerr << "first soma point" << soma->getId() << " "
				<< soma->getEnd()->getX() << " " << soma->getEnd()->getY()
				<< " " << soma->getEnd()->getZ() << " " << soma->getType()<<"\n";

		double x = soma->getEnd()->getX();
		double y1 = soma->getEnd()->getY() -  soma->getRadius();
		double z = soma->getEnd()->getZ() ;//approximation to covert spehere to cylinder assumption for soma point.

		//New Soma Z
		double y2 = soma->getEnd()->getY() +  soma->getRadius();//approximation to covert spehere to cylinder assumption for soma point.

		Vector* a1 = new Vector(x, y1, z);

		Vector* a2 = new Vector(x, y2, z);

		//we are in the soma
		Segment* newsoma1 = soma->clone();

		Segment* newsoma2 = soma->clone();
		//Ghost segment

		newsoma1->setEnd(a1);
		newsoma1->setPrev(soma);
		//SG code added 26/08/2011
		newsoma1->setPrevFather(soma);

		newsoma2->setEnd(a2);
		newsoma2->setPrev(soma);
		newsoma2->setPrevFather(soma);

		soma->addNewSomaChild(newsoma1);
		soma->addNewSomaChild(newsoma2);

		cerr<<"Cycl soma added \n";
	}
}

/*
void Neuron::zeroing() {
	if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0
			|| soma->getEnd()->getZ() != 0)
		translation(soma->getEnd()->getX(), soma->getEnd()->getY(),
				soma->getEnd()->getZ());
}
;*/

//grow a virtual neuron based on the list of parameters
Neuron::Neuron(ParameterList* lps) {

	OpenPRM(lps);

}
;

//grow a virtual neuron based on the list of parameters

void Neuron::OpenPRM(ParameterList* lps) {
	double d, rho, diam, az, ele, torch;
	int met, id, pid;

	setDefaults();

	setName(lps->getFileName());
	rnd = new Random();

	lp = lps;

	lp->setType(1);
	torch = 0;
	met = lp->getParameter(METHOD);
	//create soma
	d = lp->getParameter(SOMA_DIAMETER);
	pid = -1;
	id = 1;
	addPolar(id, 1, 1, 0, 0, 0, d, -1);
	pid = id;

	int maxType = lp->getParameter(MAX_TYPE);

	for (int ii = 3; ii <= maxType; ii++) {
		//3:Apical....
		lp->setType(ii);
		d = lp->getParameter(NUM_TREE);

		for (int i = 0; i < d; i++) {
			//first virtual segment long 1
			if (lp->isPresent(CONSLENGTH) == 1)
				rho = lp->getParameter(CONSLENGTH);
			if (lp->isPresent(LENGTH) == 1)
				rho = lp->getParameter(LENGTH);

			az = lp->getParameter(TREE_AZY);
			az *= M_PI / 180;//az=0;
			ele = lp->getParameter(TREE_ELEV);
			ele *= M_PI / 180;
			diam = lp->getParameter(INIT_DIAM);
			id++;
			id = addPolar(id, 3, rho, az, ele, torch, diam, pid);
			switch (met) {
			case 0:
				id = growBurke(diam, ii, id);
				break;
			case 1:
				id = growTamori(diam, ii, id);
				break;
			case 2:
				id = growHillman(diam, ii, id);
				break;
			default:
				cerr << "No Valid method specified!\n";
			}
		}
	}
	if (soma != NULL) {
		soma->polarRelativeToabsolute();
		soma->polarToCartesian();
		init();
	}

}
;

int Neuron::growBurke(double diameter, int type, int pid) {

	int id = pid;

	double length = lp->getParameter(LENGTH); //Sample_Distribution (dist_bin_length [tree_type]);
	double taper = lp->getParameter(TAPER); //Sample_Distribution (dist_taper [tree_type]);
	double k1_overlap = lp->getParameter(K1_OVERLAP); //Sample_Distribution (dist_k1_overlap [tree_type]);
	double k2_overlap = lp->getParameter(K2_OVERLAP); //Sample_Distribution (dist_k2_overlap [tree_type]);
	double k1_nonoverlap = lp->getParameter(K1_NONOVERLAP); //Sample_Distribution (dist_k1_nonoverlap [tree_type]);
	double k2_nonoverlap = lp->getParameter(K2_NONOVERLAP); //Sample_Distribution (dist_k2_nonoverlap [tree_type]);
	double k1_terminate = lp->getParameter(K1_TERMINATE); //Sample_Distribution (dist_k1_terminate [tree_type]);
	double k2_terminate = lp->getParameter(K2_TERMINATE); //Sample_Distribution (dist_k2_terminate [tree_type]);
	double pbr, pnonoverlap, ptr;
	double torch = 0;

	diameter += taper * length;

	pbr = k1_overlap * exp(k2_overlap * diameter);
	ptr = k1_terminate * exp(k2_terminate * diameter);
	pnonoverlap = k1_nonoverlap * exp(k2_nonoverlap * diameter);

	if (pnonoverlap < pbr)
		pbr = pnonoverlap;

	/* decide whether to branch, terminate or grow */

	if (rnd->rnd01() < pbr * length) {
		/* branch */
		double r1 = lp->getParameter(GAUSS_BRANCH); // Sample_Distribution (dist_gaussian_branch [tree_type]);
		double r2 = lp->getParameter(GAUSS_BRANCH); //Sample_Distribution (dist_gaussian_branch [tree_type]);
		double burkea = lp->getParameter(LINEAR_BRANCH);//Sample_Distribution (dist_linear_branch [tree_type]);
		double diam1 = diameter * (r1 + r2 * burkea);
		double diam2 = diameter * (r2 + r1 * burkea);
		double totbifang = lp->getParameter(BIFAMPLI); //Sample_Distribution (dist_bifurcating_amplitude_angle [tree_type]);
		double bifang1 = rnd->rnd01() * totbifang;
		double bifang2 = bifang1 - totbifang;

		id = pid + 1;
		id = addPolar(id, type, length, bifang1, 0, torch, diam1, pid);

		id = growBurke(diam1, type, id);

		id++;
		id = addPolar(id, type, length, bifang2, 0, torch, diam2, pid);

		pid = growBurke(diam2, type, id);

		return pid;

	} else if (rnd->rnd01() < ptr * length) { /* terminate */
		return id;

	} else { /* grow a stem */
		double azi = lp->getParameter(EXTEND_AZIMUTH);
		double ele = lp->getParameter(EXTEND_ELE);
		id = pid + 1;
		torch = 0;
		id = addPolar(id, type, length, azi, ele, torch, diameter, pid);

		id = growBurke(diameter, type, id);

	}
	return pid;
}

int Neuron::growTamori(double diameter, int type, int pid) {
	double length, taper;
	length = lp->getParameter(CONSLENGTH);
	taper = lp->getParameter(TAPER);
	diameter *= (1 - taper);

	int id = pid;

	if (diameter < lp->getParameter(THRESHOLD)) {
		length = lp->getParameter(TERMLENGTH);
		id++;
		id = addPolar(id, type, length, 0, 0, 0, diameter, pid);

		//if less than min then terminate
		return id;
	}

	//bifurcate
	double ratio, power, rall, diam1, diam2;

	ratio = lp->getParameter(BIFRATIO);
	power = lp->getParameter(BIFPOWER);
	rall = pow((1 + pow(ratio, power)), -1 / power);
	diam2 = diameter * rall * pow(lp->getParameter(PK), 1 / power);
	diam1 = ratio * diam2;

	double dimension, eq5_4_n1, eq5_4_n2, eq5_4_den, bifang1, bifang2;
	dimension = 1.0 + rnd->rnd01() * (power - 1);

	/* Equation 5.4 from Tamori, 1993 */
	eq5_4_n1 = pow(1 + pow(ratio, power), 2 * dimension / power);
	eq5_4_n2 = pow(ratio, 2 * dimension);
	eq5_4_den = 2 * pow(1 + pow(ratio, power), dimension / power);
	bifang1 = acos(
			(eq5_4_n1 + eq5_4_n2 - 1) / (eq5_4_den * pow(ratio, dimension)));

	bifang2 = acos((eq5_4_n1 - eq5_4_n2 + 1) / (eq5_4_den));

	//for display purpose
	if (rnd->rnd01() < 0.5) {
		bifang1 *= -1;
		bifang2 *= -1;
	}

	double torch = lp->getParameter(BIFORIENT) * M_PI / 180;
	double azi, ele;

	id = pid + 1;

	azi = bifang1;
	ele = 0;
	id = addPolar(id, type, length, azi, ele, torch, diam1, pid);

	id = growTamori(diam1, type, id);

	id++;

	azi = bifang2;
	ele = 0;
	id = addPolar(id, type, length, azi, ele, torch, diam2, pid);

	id = growTamori(diam2, type, id);

	return id;
}

int Neuron::growHillman(double diameter, int type, int pid) {
	// model of Hillman
	double length, taper;
	length = lp->getParameter(CONSLENGTH);
	taper = lp->getParameter(TAPER);
	double diamOld = diameter;
	diameter *= (1 - taper);

	int id = pid;

	if (diameter < lp->getParameter(THRESHOLD)) {
		length = lp->getParameter(TERMLENGTH);
		id++;
		id = addPolar(id, type, length, 0, 0, 0, diameter, pid);

		//if less than min then terminate
		return id;
	}

	//bifurcate
	double ratio, power, rall, diam1, diam2, totbifang;

	ratio = lp->getParameter(BIFRATIO);
	power = lp->getParameter(BIFPOWER);
	rall = pow((1 + pow(ratio, power)), -1 / power);
	diam2 = diameter * rall * pow(lp->getParameter(PK), 1 / power);
	diam1 = ratio * diam2;

	double bifang1, bifang2;

	totbifang = lp->getParameter(BIFAMPLI);
	totbifang *= M_PI / 180;
	bifang1 = rnd->rnd01() * totbifang;
	bifang2 = bifang1 - totbifang;

	double torch = lp->getParameter(BIFORIENT) * M_PI / 180;
	double azi, ele;

	azi = bifang1;
	ele = 0;
	id = pid + 1;
	id = addPolar(id, type, length, azi, ele, torch, diam1, pid);

	id = growHillman(diam1, type, id);

	id++;

	azi = bifang2;
	ele = 0;
	addPolar(id, type, length, azi, ele, torch, diam2, pid);

	pid = growHillman(diam2, type, id);

	return pid;
}

//build a neuron from an input file

Neuron::Neuron(char* c,int neuroClass, bool isDiaPresent, char * dia_n){
	cerr << "creating neuron:" << c << "\n";
	ferror = false;
	foundsoma = false;
	//reset comment field
	d[0][0] = 0;
	neuronalClass = neuroClass;
	soma = NULL;
	strtseg = NULL;
	nrSeg = 0;
	setShrinkX(1);
	setShrinkY(1);
	setShrinkZ(1);
	name = c;
	name_str = c;
	namectr = 1;

	setIsDiaSet(isDiaPresent);
	if(isDiaSet)
		dia = dia_n;

	int startind;
	#ifdef OS_WIN
		//Windows specific stuff
		startind = name_str.rfind("\\");
	#else
		//Normal stuff
		startind = name_str.rfind("/");
	#endif

	int endind = name_str.rfind(".") - 1;

	dir_str = name_str.substr(0, startind + 1);

	name_str = name_str.substr(startind + 1, endind - startind);
	//cerr<<name_str<<"\n";
	if(name_str == "03204L2.CNG.swc")
		cerr<<"stop here.."<<name_str<<"\n";

	oriented = false;

	rf = 0;
	//flag variable to intiate rf and first segment
	flag = 0;
	ctr = 0;
	//replace '*' in the file path with ' '
	int jj = 0;
	while (c[jj] != 0) {
		if (c[jj] == '*')
			c[jj] = ' ';
		jj++;
	}

	in = new ifstream(c, ios::in | ios::binary);

	//SG Code
	in_d = new ifstream(c, ios::in | ios::binary);

	char * ext;

	//look for last point in the file name to extract extension
	int i;
	for (i = 0; i < 1024 && c[i] != 0; i++) {
		if (c[i] == '.')
			ext = c + i;
	}
	int h = ext - c;
	int jk = strlen(c);
	//error if file does not exist or extension is smaller than 2 chars
	if (in == NULL || strlen(c) <= (ext - c)) {

		cerr << "Wrong FileName:" << c
				<< " !\n----------------------------------\n";
		soma = new Segment(0, 1, new Vector(0, 0, 0), 1, -1);
		return;
	}

	// This code extracts the last 4 characters of a file name which
	// consist of the file extension and is used for comparison of file types.
	int typ = -1;

	char *substr = new char[4];

	for (int i = 0; i <= 4; i++)
	{
	    substr[i] = c[strlen(c) - (4 - i)];
	}

//	string string(c),substr;
//	size_t pos;
//	pos = string.find_last_of(".");
//	substr = string.substr(pos);

	//if (strstr(c, ".swc") != NULL || strstr(c, ".SWC") != NULL)				// Bug : Checks for a .swc file extension anywhere in the filename.
	//if (strcmp(substr,".swc") == 0 || strcmp(substr,".SWC") == 0)
		//typ = 0;
	if (strcmp(substr,".hoc") == 0 || strcmp(substr,".dat") == 0)			// filters out .dat and .hoc files right at the
		typ = 5;																// beginning to prevent extra computation.
	else
		typ = LookForFileType(in);
	cerr<<"returning.."<<typ;
	switch (typ) {
	case 0:

		//computes SWC files
		OpenSWC();

		if (soma->getType() != 1 && foundsoma == true) {

			//SG Code changes Sep 02 2011
			//Commented to fix the problem where the conversion not taking place if the
			//soma not placed on first line of swc file.
			//addVirtualSoma(soma);

			buildFathers(soma, soma);
			//initializing this vairable for every new neuron sri 03/25/10

			found = 0;
			Segment * tmp = rearrangeSWC(soma);

			soma = tmp;
			tmp = NULL;

			soma->setPrev(NULL);
			renumerateId(soma);
		}
		break;
	case 1:
		//computes Amaral, Henze, Miller Eutectic files
		OpenAMA();
		break;
	case 2:
		//computes NeuroLucida files
		OpenNeuroL();
		break;
	case 3:
		//computes Claiborne files
		OpenClaiborne();
		//OpenSEG();
		break;
	case 4:

		OpenAmira();

		break;
	default:
		cout << "Given File type is not supported!!\n";
		cerr << "Given File type is not supported!!\n";
		break;
	}

	char buff[30];

	//the tree_ext variable is used to create subtrees (only for swc files). commented out this line by mistake in 2.8.6 version.
	tree_ext = ".swc";

	if (soma == NULL) {

		cout << "Unable to create Neuron:" << c
				<< " !\n----------------------------------\n";
		Vector * e = new Vector(0, 0, 0);
		soma = new Segment(0, 1, e, 1, -1);
		delete e;
		return;
	}


	in->close();
	delete in;
	in = NULL;


	init();

	soma->setClass(neuronalClass);

 cerr<<"Neuron() done \n";
}
void Neuron::doPCA() {

	if (oriented == false) {
		if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0
				|| soma->getEnd()->getZ() != 0)
			translation(soma->getEnd()->getX(), soma->getEnd()->getY(),
					soma->getEnd()->getZ());

		pca();

		if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0
				|| soma->getEnd()->getZ() != 0)
			translation(soma->getEnd()->getX(), soma->getEnd()->getY(),
					soma->getEnd()->getZ());

		setOrder(soma);
		setParameters(soma);

	}
	oriented = true;
}

//LookForFileType function looks for unique tags to find the File extension implemented by sridevi 11/29/04
int Neuron::LookForFileType(ifstream* in) {
	char * AMH_TAG1 = "No.";
	char * AMH_TAG2 = "points";
	char * AMH_TAG3 = "Point";
	char * NL_SOMA_TAG1 = "(CellBody)";
	char * NL_SOMA_TAG2 = "(Closed)";
	char * CL_TAG1 = "S";
	char * SWC_TAG1 = "1";
	char * SWC_TAG2 = "-1";
	char * SWC_TAG3 = "2";
	char * SWC_TAG4 = "1";

	char * SWC_COMMENT = "#";
	int type = -1;

	if (lookFor("AmiraMesh", in) == true)
		return 4;

	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	if (lookFor("Point", in) == true)
		if (lookFor("Type", in) == true)
			if (lookFor("Tag", in) == true)
				return 1;

	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	if (lookFor(NL_SOMA_TAG1, in) == true) {

		return 2;
	}

	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	if (lookFor(NL_SOMA_TAG2, in) == true) {

		return 2;
	}

	//neurolucida files which doesn't have a CellBody keyword.
	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	if (lookFor("(Axon)", in) == true) {
		//  cout<<"found Axon......... \n";
		return 2;
	}
	//SP added in->peek() ==-1 for moving the 'in' pointer to the beginning of the file. The change is made consistently everywhere. 10/10/12
	//neurolucida files which doesn't have a CellBody keyword.
	if (in->peek()==-1 || in->fail()){
		in->clear();
		in->seekg(0, ios::beg);
	}
	//cerr<<"c in peek.."<<in->peek();
	if (lookFor("(Dendrite)", in) == true){
		//cout<<"found Dendrite......... \n";
		return 2;
	}

	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	char whole_line[500];
	//skip the lines that start with # or %
	while (in->peek() == 35 || in->peek() == 37) {
		in->getline(whole_line, 500, '\n');
		//cerr<<"skipping in eutectic"<<whole_line<<"\n";
	}
	if (lookFor("1", in) == true)
		if (lookFor("S", in) == true)
			if (lookFor("C", in) == true)
				if (lookFor("B", in) == true)
					if (lookFor("T", in) == true)
						return 3;//type=3;
	/*
	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	if (lookFor("1", in) == true)
		if (lookFor("P", in) == true)
			return 3;
			*/
	//checking for the first compartment id = 1 and pid = -1, also count of #tokens in the first line == 7 only.
	if (in->fail() || in->peek() == -1){
		in->clear();
		in->seekg(0, ios::beg);
	}
	//skip the lines that start with # or %
	while (in->peek() == 35 || in->peek() == 37) {
		in->getline(whole_line, 500, '\n');
		//cerr<<"skipping in SWC format"<<whole_line<<"\n";
	}
	//cerr<<"beginning char.."<<in->peek();
	// Parses the input file to check if the file is in SWC format. it looks for 1 1 and -1 mathces.
	if (lookFor(SWC_TAG1, in) == true){
			if (lookFor(SWC_TAG2, in) == true)
				if (lookFor(SWC_TAG3, in) == true)
					if (lookFor(SWC_TAG4, in) == true)
						return 0;
	}

	return 5;
}

//modified the return type to bool from void  by sridevi on 11/29/04
bool Neuron::lookFor(char * string, ifstream* in) {
	char c[500];
	*in >> c;
	//cerr<<"starting string:"<<c<<"\n";
	//cerr<<string<<" "<<strlen(string)<<"\n";
	//cerr<<"checking.."<<in->peek()<<"\n";
	//cerr<<"checking again.."<<in->peek()<<"\n";
	//while (strncmp(string, c, strlen(string)) != 0 && !in->eof() && in->peek()!= -1) {
	//skip lines that start with '#'
	while((!in->eof() || !in->fail()) && strncmp(c,string,strlen(string))!=0){
		//cout<<"c............... :"<<c<<"\n";
		*in >> c;
		//cerr<< ".."<<c<<" "<<in->peek()<<"\n";
	}
	//cerr << "Input line"<< c;
	//cerr<<"string:"<<string<<"c:"<<c<<"\n";
	if (memcmp(string, c, strlen(string)) == 0) {
	//if(strncmp(string,c,strlen(string))==0){
		cerr<<"c............... :"<<c<<"  String........: "<<string<<"\n";
		return true;
	} else
		return false;
}

double * Neuron::getValues(double * ret) {
	char c = 'y';
	double xSum = 0, ySum = 0, zSum = 0, rSum = 0, xSq = 0, ySq = 0, zSq = 0,
			x, y, z, r;
	int count = 0;

	{
		x = -1;
		y = -1;
		z = -1;
		while (c != -1) {

			while (c != '(' && c != -1 && c != ')') {
				in->peek();
				c = in->get();
			}
			if (c == ')')
				break;
			//remove spaces after '('. added on 09/10/2010 by Sridevi. Otherwise the soma points are skipped
			while (in->peek() == ' ' && in->peek() != -1)
				in->get();
			if ((in->peek() >= '0' && in->peek() <= '9') || in->peek() == '-') {
				*in >> x;
				*in >> y;
				*in >> z;
				*in >> r;
				//cout<<x<<" "<<y<<" "<<z<<" "<<r<<endl;
				xSum += x;
				ySum += y;
				zSum += z;
				rSum += r;
				xSq += x * x;
				ySq += y * y;
				zSq += z * z;
				count++;
			}
			//go to end line
			while (c != 13 && c != 10) {
				in->peek();
				c = in->get();

			}

			if (in->peek() == ')')
				break;
		}

		ret[0] = xSum / count;
		ret[1] = ySum / count;
		ret[2] = zSum / count;

		double tmp = 0;
		double xvar = 0, yvar = 0, zvar = 0;
		tmp = xSq / count - ret[0] * ret[0];
		if (tmp > 0)
			xvar = sqrt(tmp);
		tmp = ySq / count - ret[1] * ret[1];
		if (tmp > 0)
			yvar = sqrt(tmp);
		tmp = zSq / count - ret[2] * ret[2];
		if (tmp > 0)
			zvar = sqrt(tmp);
		double soma = (xvar + yvar + zvar) / 3 * 4;
		ret[3] = soma;

		c = in->peek();
	}//else
	return ret;

}

int Neuron::NeurolAdd(int id, int type, double x0, double y0, double z0) {

	int biforc[1000];
	biforc[0] = 1;
	biforc[1] = 1;
	int index = 0;
	int lastindex = 0;
	int lookForCloseParenthesis = 0;
	int pid;
	int markctr = 0;
    pid = 1;
	int endTree = 0;
	while (in->peek() != -1 && endTree == 0) {
		int save = 0;
		int foundBar = 0;
		index = 0;
		lookForCloseParenthesis = 0;
		save = 0;
		char c = 'y';
		double x, y, z, d;

		while (c != '(' && c != '|' && in->peek() != -1) {
			in->peek();
			c = in->get();
			if (lookForCloseParenthesis == 1 && c == ')')
				lookForCloseParenthesis = 2;

			if (c == 10 || c == 13) {
				index = 0;
				lookForCloseParenthesis = 1;
			}
			if (c == ' ')
				index++;
			if (lookForCloseParenthesis == 2) {

				char k[100];
				*in >> k;

				if (strcmp(k, "tree") == 0) {
					index = 0;
					lastindex = 0;
					biforc[2] = 1;
					pid = 1;
					endTree = 1;
					break;
				} else if (strcmp(k, "split") == 0) {

					lookForCloseParenthesis = 0;
				}
			}
		}

		if (c == '|')
			foundBar = 1;

		//remove spaces after '('
		while (in->peek() == ' ' && in->peek() != -1)
			in->get();

		//if the line does not contains anything then save the previous id
		if (in->peek() == 10 || in->peek() == 13)
			save = 1;
		// when found a '|' correct for pid
		if (lastindex == index && foundBar == 1) {
			int jj = 0;
			foundBar = 0;
			pid = biforc[index];
		}
		if (lastindex > index && save == 1) {
			//after a termination
			pid = biforc[index];
			lastindex = index;
		}
		if (lastindex < index && save == 1) {
			//new biforcation
			biforc[index] = id;
			lastindex = index;
		}
		//probable location for error!!
		c = in->peek();

		if (c == -1) {
			int stop = 1;
		}
		// to skip marker blocks. when R is not present the line is skipped.
		int pos;
		pos = in->tellg();
		char tmp[200];
		in->getline(tmp, 200);
		if (strstr(tmp, "Marker") != NULL || strstr(tmp, "Spines") != NULL
				|| strstr(tmp, "spines") != NULL || strstr(tmp, "marker")
				!= NULL) {
			markctr++;

			while (strstr(tmp, "End of markers") == NULL) {
				pos = in->tellg();
				in->getline(tmp, 200);

			}

		}
		//adding second parameter here or eclipse is showing it as invalid argument error.
		in->seekg(pos,ios::beg);

		//if a number follow get the segment cooordinates
		if ((in->peek() >= '0' && in->peek() <= '9') || in->peek() == '-') {
			id++;
			//get segment
			c = in->peek();
			x = -1;
			y = -1;
			z = -1;
			*in >> x;
			*in >> y;
			*in >> z;
			*in >> d;
			//cout<<x<<' '<<y<<' '<<z<<' '<<d<<"\n";
			if (soma == 0) {
				//create a soma
				add(1, type, x - x0 - 0.001, y - y0 - 0.001, z - z0 - 0.001, d,
						-1);
				id = 2;
				pid = 1;
            }
			add(id, type, x - x0, y - y0, z - z0, d, pid);
			pid = id;
		}
		//go to end line
		while (c != 13 && c != -1 && c != 10) {
			in->peek();
			c = in->get();

		}

	}
	//cout<<"markctr:"<<markctr<<"\n";
	return id;
}
;

void Neuron::OpenNeuroL() {
	//translation cood, to have soma always in 0,0,0
	double x0 = 0, y0 = 0, z0 = 0;

	strcpy(
			d[0],
			"# Neurolucida to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;

	int cellBody_cnt = 0;
	char buffer [33];


	//store biforc
	double x = 0, y = 0, z = 0;

	double ret[5];
	//initialize the ret array to a big integer
	ret[0] = ret[1] = ret[2] = ret[3] = ret[4] = -999999999;

	//modified the code such that all cellbody tags are searched at once sri 07/22/2010
	if (in->fail())
		in->clear();
	in->seekg(0, ios::beg);
	int id = 1; int pid = -1;
	while (!in->fail()) {
		if (lookFor("(CellBody)", in)) {
			cerr << "CellBody tag for cell body.." << endl;
			getValues(ret);
		}

		if((ret[0]!=-999999999) && (ret[1]!=-999999999)){

			cerr<<"adding soma.."<<":"<<id<<" 1 "<<ret[0]<<" "<<ret[1]<<" "<<ret[2]<<" "<<ret[3]<<" "<<pid<<"\n";

			add(id,1,ret[0],ret[1],ret[2],ret[3],pid);
			pid = id;id = id+1;
			cellBody_cnt++;
		}

		ret[0]=ret[1]=ret[2]=ret[3]=ret[4]=-999999999;
	}
	if (in->fail())
		in->clear();
	in->seekg(0, ios::beg);

	ret[0]=ret[1]=ret[2]=ret[3]=ret[4]=-999999999;

	while (!in->fail()) {
		if (lookFor("(Closed)", in)) {
			cerr << "Closed tag for cell body.." << endl;
			getValues(ret);
		}

		//adding the centroid point of each contour to the coverted file
		//insert soma. modified the code because soma insertion is failing hence checking on the ret[] array to add soma sri 07/22/2010
		if((ret[0]!=-999999999) && (ret[1]!=-999999999)){
			cerr<<"adding soma.."<<":"<<id<<" 1 "<<ret[0]<<" "<<ret[1]<<" "<<ret[2]<<" "<<ret[3]<<" "<<pid<<"\n";
			add(id,1,ret[0],ret[1],ret[2],ret[3],pid);
			pid = id;id = id+1;
			cellBody_cnt++;
		}

		ret[0]=ret[1]=ret[2]=ret[3]=ret[4]=-999999999;

	}

	//RString rstring = new RString(cellBody_cnt);

	stringstream ss;
	ss << cellBody_cnt;

	if(cellBody_cnt > 1){
		strcpy(d[2], "# The original file has ");
		//itoa(cellBody_cnt,buffer,10);
		strcat(d[2],ss.str().c_str());
		strcat(d[2], " soma contours that are averaged into ");
		strcat(d[2], ss.str().c_str());
		strcat(d[2], " soma points");
	}else if(cellBody_cnt == 1){
		strcpy(d[2], "#The original file has a single soma contour that is averaged into 3 soma points");

	}else{
		strcpy(d[2], "#The original file has no soma");

	}

	//insert soma. modified the code because soma insertion is failing hence checking on the ret[] array to add some sri 07/22/2010
	//	if ((ret[0] != -999999999) && (ret[1] != -999999999)) {
	//		cerr << "adding soma.." << ret[0] << " " << ret[1] << " " << ret[2]
	//				<< " " << ret[3] << "\n";
	//		add(1, 1, ret[0], ret[1], ret[2], ret[3], -1);
	//	}
	if (in->fail())
		in->clear();
	in->seekg(0, ios::beg);
	//int id = 1;
	int dendctr = 0;
	while (in->peek() != -1) {
		dendctr++;
		lookFor("(Dendrite)", in);
		//cout<<"looked for dendrite in openNeurolucida.... \n";
		id = NeurolAdd(id, 3, x0, y0, z0);
		//cout<<"added dendrite neuron in openneurolucida.... \n";
	}
	//cout<<"dendctr:"<<dendctr<<"\n";
	//cout<<"id-dendrite-------------------"<<id<<"\n";

	if (in->fail())
		in->clear();
	in->seekg(0, ios::beg);
	int axonctr = 0;
	while (!in->eof()) {
		axonctr++;
		if (in->peek() == -1) {
			//cout<<"axonctr:"<<in->eof()<<"\n";
			break;
		}
		if (lookFor("(Axon)", in)) {
			//cout<<"looked for Axon in openNeurolucida.... \n";
			id = NeurolAdd(id, 2, x0, y0, z0);
			//cout<<"added axon neuron in openneurolucida.... \n";
		}
	}
	cerr<<"axonctr:"<<axonctr<<"\n";
	//	cout<<"id-axon-------------------"<<id<<"\n";

	in->close();
	delete in;
	in = new ifstream(name, ios::in | ios::binary);
	int apictr = 0;
	while (in->peek() != -1) {
		apictr++;

		lookFor("(Apical)", in);
		id = NeurolAdd(id, 4, x0, y0, z0);
	}
	//cerr<<"apictr:"<<apictr<<"id-apical-------------------"<<id<<"\n";
}

void Neuron::OpenSEG() {
	strcpy(d[0],
			"# SEG to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;
	int commId = 4;
	int const MM = 5000;
	char type[100];
	int NR = 0, id, tag, tagRead, myid = 0, lastparid, lastbra, braid[MM],
			myparid, stem = 1;
	;
	for (int i = 0; i < MM; i++)
		braid[i] = 0;
	double thick, x, y, z;

	while (in->peek() == 32 || in->peek() == 10 || in->peek() == 13) {
		in->get();
	}

	while (in->peek() == 35) {
		in->getline(d[commId], 99, '\n');
		commId++;
		while (in->peek() == 32) {
			in->get();
		}
	}

	while (in->peek() != -1) {
		char c = in->peek();
		NR++;
		*in >> id;
		*in >> type;
		*in >> tagRead;
		*in >> x;
		*in >> y;
		*in >> z;
		*in >> thick;

		if (id == 1463) {
			int stop = 1;
		}

		//remove all returns
		while (in->peek() == 13 || in->peek() == 10) {
			if (in->peek() == 13)
				in->get();
			if (in->peek() == 10)
				in->get();
		}
		//remove beginning space
		while (in->peek() == 32) {
			in->get();
		}

		if (strcmp(type, "S") == 0) {
			tag = tagRead;
			myid++;
			lastparid = myid;
			myparid = -1;
			lastbra = 0;

			add(myid, tag, x, y, z, 0.5 * thick, myparid);
		}

		if (strcmp(type, "P") == 0) {
			stem++;
		}

		if (strcmp(type, "C") == 0) {
			tag = tagRead;
			myid++;
			myparid = lastparid;
			lastparid = myid;

			add(myid, tag, x, y, z, 0.5 * thick, myparid);
		}

		if (strcmp(type, "B") == 0) {
			tag = tagRead;
			myid++;
			myparid = lastparid;
			lastparid = myid;
			lastbra++;
			braid[lastbra] = myid;

			add(myid, tag, x, y, z, 0.5 * thick, myparid);
		}

		if (strcmp(type, "T") == 0) {
			tag = tagRead;
			myid++;
			myparid = lastparid;
			if (braid[lastbra] != 0) {
				lastparid = braid[lastbra];
			} else if (stem > 0) {
				lastparid = 1;
				stem--;
			} else {
				cerr << " # error! not enough stems!\n";
				break;
			}
			lastbra--;

			//correct if no more bif
			if (lastbra == -2) {
				lastbra = -1;
				stem--;
				lastparid = 1;
			}

			add(myid, tag, x, y, z, 0.5 * thick, myparid);
		}
	}

}
;

void Neuron::add(int id, int type, double x, double y, double z, double diam,
		int pid) {

	double radius = diam / 2;
	if (x == 3.189 || y == 3.189 || z == 3.189) {
		double yy = y + z;
	}
	Vector* a = new Vector(x, y, z);
	Segment* s = new Segment(id, type, a, radius, pid);
	delete a;
	if (soma == NULL) {
		soma = s;
		soma->setPrevFather(s);
	} else {
		addSegment(s);
	}
}
;

int Neuron::addPolar(int id, int type, double rho, double azi, double ele,
		double torch, double diam, int pid) {
	if (type != 1) {

		//fragmentation & contraction
		int fragment = (int) lp->getParameter(FRAGMENTATION) - 1;
		double contract = lp->getParameter(CONTRACTION) * M_PI / 180;

		//if fragmentation or contraction are not defined just add the segment
		if (fragment < 2 || contract == -1) {
			addPolarSegment(id, type, rho, azi, ele, torch, diam, pid);
			return id;
		}

		double plusminus, alpha;
		if (rnd->rnd01() < 0.5)
			plusminus = 1;
		else
			plusminus = -1;

		alpha = acos(contract) * plusminus;

		rho = rho / fragment;
		fragment = fragment / 2;
		//add first segment with all angles right
		addPolarSegment(id, type, rho, azi, ele, torch, diam, pid);

		for (int i = 0; i < fragment; i++) {
			pid = id;
			id++;
			addPolarSegment(id, type, rho, contract, 0, 0, diam, pid);
			pid = id;
			id++;
			addPolarSegment(id, type, rho, -contract, 0, 0, diam, pid);
		}
		return id;
	}

	//else just add one segment
	addPolarSegment(id, type, rho, azi, ele, torch, diam, pid);
	return id;

}

void Neuron::addPolarSegment(int id, int type, double rho, double azi,
		double ele, double torch, double diam, int pid) {
	//Create a Segment for each line
	double radius = diam / 2;
	Vector* a = new Vector();
	a->setPolar(rho, azi, ele, torch);
	Segment* s = new Segment(id, type, a, radius, pid);

	if (soma == NULL) {
		soma = s;
		soma->setPrevFather(s);
	} else {
		addSegment(s);
	}
}

void Neuron::OpenAMA() {

	strcpy(
			d[0],
			"# Amaral to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;

	int branches = 0, terminals = 0;
	int ok = 0;
	int stems = 0;

	double xsoma = 0, ysoma = 0, zsoma = 0, countsoma = 0, xsqsoma = 0,
			ysqsoma = 0, zsqsoma = 0;
	char c[100], name[100], type[100];
	int NR = 0, id, tag, tagRead, skip, myid = 2, lastparid, lastbra,
			braid[5000], myparid;
	double thick, x, y, z;

	in->getline(c, 100, '\n');

	add(1, 1, 0, 0, 0, 0, -1);
	while (in->peek() != -1) {

		while (in->peek() == 32) {
			in->get();
		}
		NR++;
		*in >> id;
		*in >> type;
		*in >> tagRead;
		*in >> x;
		*in >> y;
		*in >> z;
		*in >> thick;

		while (in->peek() == 32 && in->peek() == 9) {
			in->get();
		}

		if (in->peek() != 13 && in->peek() != 10 && in->peek() != 9)
			*in >> name;
		else
			strcpy(name, "");

		int ch = (int) in->peek();
		while (ch != 10 && ch != 13 && ch != -1) {
			char c = in->get();
			ch = in->peek();

		}

		while (in->peek() == 13)
			if (in->peek() == 13) {
				in->get();
				if (in->peek() == 10)
					in->get();
			}

		while (in->peek() == 32 && in->peek() == 9) {
			in->get();
		}

		if (strcmp("SOS", type) == 0 || strcmp("SCP", type) == 0 || strcmp(
				"SOE", type) == 0) {
			xsoma += x;
			ysoma += y;
			zsoma += z;
			xsqsoma += x * x;
			ysqsoma += y * y;
			zsqsoma += z * z;

			countsoma++;
			ok = 1;
		}
		if (strcmp("MTO", type) == 0 || strcmp("TTO", type) == 0) {
			stems++;
			skip = 0;
			if (strchr(name, 'd') > 0) {

				if (strchr(name, 'a') > 0) {

					tag = 4;
				} else {

					tag = 3;
				}
			} else if (strchr(name, 'x') > 0) {

				myparid = 1;
				tag = 2;
			} else if (strchr(name, 'a') > 0) {

				tag = 4;
			} else if (strchr(name, 'b') > 0) {

				tag = 3;
			} else if (strchr(name, 'o') > 0) {

				myparid = 1;
				tag = 10;
			} else {
				myparid = 1;
				tag = 5;
			}
			if (skip == 0) {
				myid++;
				lastparid = myid;
				if (myparid != 2)
					myparid = 1;
				lastbra = 0;
				braid[0] = -1;

				add(myid, tag, x, y, z, thick, myparid);
				ok = 1;
			}
		}

		if (strcmp("CP", type) == 0 || strcmp("FS", type) == 0) {
			if (skip == 0) {
				myid++;
				myparid = lastparid;
				lastparid = myid;

				add(myid, tag, x, y, z, thick, myparid);
				ok = 1;
			}
		}

		if (strcmp("BP", type) == 0) {
			if (skip == 0) {
				branches++;

				myid++;
				myparid = lastparid;
				lastparid = myid;
				lastbra++;
				braid[lastbra] = myid;

				add(myid, tag, x, y, z, thick, myparid);
				ok = 1;
			}
		}

		if (strcmp("NE", type) == 0 || strcmp("BAE", type) == 0 || strcmp("ES",
				type) == 0 || strcmp("MAE", type) == 0 || strcmp("TAE", type)
				== 0) {
			if (skip == 0) {

				myid++;
				myparid = lastparid;
				lastparid = braid[lastbra];
				lastbra--;

				add(myid, tag, x, y, z, thick, myparid);
				ok = 1;
			}
		}

		if (ok != 1)
			cerr << "unread id " << id;

	}

	if (countsoma == 0)
		return;
	double x0, y0, z0;
	x0 = xsoma / countsoma;
	y0 = ysoma / countsoma;
	z0 = zsoma / countsoma;
	soma->getEnd()->setX(x0);
	soma->getEnd()->setY(y0);
	soma->getEnd()->setZ(z0);
	double xvar = sqrt(xsqsoma / countsoma - x0 * x0);
	double yvar = sqrt(ysqsoma / countsoma - y0 * y0);
	double zvar = sqrt(zsqsoma / countsoma - z0 * z0);
	soma->setRadius(2 * (xvar + yvar + zvar) / 3);

}

double * Neuron::addSomaClaiborne(double * ret) {

	double xSum = 0, ySum = 0, zSum = 0, rSum = 0, xSq = 0, ySq = 0, zSq = 0,
			x, y, z, thick;
	int count = 0;
	char id[100], type[100];

	if (in->fail())
		in->clear();
	in->seekg(0, ios::beg);
	while (in->peek() != -1) {

		while (in->peek() == 32) {
			in->get();
		}
		if (in->peek() == '/') {
			in->get();
			if (in->peek() == '*') {
				int foundEnd = 0;
				while (foundEnd == 0) {
					in->get();
					if (in->peek() == '*') {
						in->get();
						if (in->peek() == '/') {
							foundEnd = 1;
							in->get();
						}
					}
				}
			}
		}

		char c = in->peek();
		*in >> id;
		if (strcmp(id, "794") == 0) {
			int iii = 6;
		}
		*in >> type;

		*in >> x;
		*in >> y;
		*in >> z;
		*in >> thick;

		while (in->peek() == 32 && in->peek() != -1) {
			in->get();
		}

		while (in->peek() != 13 && in->peek() != 10 && in->peek() != -1) {
			char c = in->get();

		}

		while (in->peek() == 13 && in->peek() != -1)
			if (in->peek() == 13) {
				in->get();
				if (in->peek() == 10)
					in->get();
			}

		while (in->peek() == 32 && in->peek() != -1) {
			in->get();
		}

		if (strcmp("S", type) == 0 || strcmp("P", type) == 0) {
			xSum += x;
			ySum += y;
			zSum += z;
			xSq += x * x;
			ySq += y * y;
			zSq += z * z;
			count++;
		}
	}

	double x0 = xSum / count;
	double y0 = ySum / count;
	double z0 = zSum / count;

	double xvar = sqrt(xSq / count - x0 * x0);
	double yvar = sqrt(ySq / count - y0 * y0);
	double zvar = sqrt(zSq / count - z0 * z0);

	double radius = (xvar + yvar + zvar) / 3;

	ret[0] = x0;
	ret[1] = y0;
	ret[2] = z0;

	ret[3] = radius * 2;
	return ret;

}

void Neuron::OpenClaiborne() {

	strcpy(
			d[0],
			"# Claiborne to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;

	char spec[10];
	int NR = 0, id, myid = 1, braid[5000], braIndex = 0, lastSpec = -1, type =
			3;
	double thick, x, y, z;

	double ret[4];
	addSomaClaiborne(ret);
	double x0 = ret[0], y0 = ret[1], z0 = ret[2];
	add(1, 1, 0, 0, 0, ret[3], -1);
	int ID = 2;
	int pid = 1;

	in->close();
	delete in;

	in = new ifstream(name, ios::in | ios::binary);

	while (in->peek() != -1) {

		while (in->peek() == 32) {
			in->get();
		}
		if (in->peek() == '/') {
			in->get();
			if (in->peek() == '*') {
				int foundEnd = 0;
				while (foundEnd == 0) {
					in->get();
					if (in->peek() == '*') {
						in->get();
						if (in->peek() == '/') {
							foundEnd = 1;
							in->get();
						}
					}
				}
			}
		}

		char c = in->peek();
		NR++;
		*in >> id;
		*in >> spec;

		*in >> x;
		*in >> y;
		*in >> z;
		*in >> thick;

		while (in->peek() == 32 && in->peek() != -1)
			in->get();

		if (lastSpec == 5) {

			type = 3;

		}

		while (in->peek() != 10 && in->peek() != 13 && in->peek() != -1) {
			char c = in->get();

		}

		while (in->peek() == 13 || in->peek() == 10) {
			in->get();
			if (in->peek() == -1)
				break;
		}

		while (in->peek() == 32 && in->peek() != -1) {
			in->get();
		}

		if (strcmp("S", spec) == 0 || strcmp("P", spec) == 0) {
			lastSpec = 0;
		} else if (strcmp("C", spec) == 0 || strcmp("F", spec) == 0) {

			if (lastSpec == 0) {
				type = 3;

			}
			add(ID, type, x, y, z, thick, pid);
			pid = ID;
			ID++;

			lastSpec = 2;
		} else if (strcmp("B", spec) == 0) {

			add(ID, type, x, y, z, thick, pid);

			braid[braIndex] = ID;
			pid = ID;
			ID++;
			braIndex++;
			lastSpec = 3;
		} else if (strcmp("T", spec) == 0 && lastSpec != 0) {

			add(ID, type, x, y, z, thick, pid);
			ID++;
			braIndex--;
			pid = braid[braIndex];
			lastSpec = 4;
			if (braIndex < 0) {
				pid = 1;
				braIndex = 0;
				lastSpec = 5;
			}

		}

	}

}

void Neuron::OpenAmira() {
	double x = 0, y = 0, z = 0, diam = 0, conn = 0;
	strcpy(
			d[0],
			"# Amiramesh format to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;
	char tmp[100];
	int id = 0;
	int const MAX = 5;
	int nfls = 5;
	int nfvs;
	bool isDiaPresent = false;



	lookFor("Lines", in);//define Lines 231
	in->peek();//skip space
	*in >> nfls;//read the given numer

	lookFor("Vertices", in);
	in->peek();
	*in >> nfvs;

	//cout<<"nfls:"<<nfls;
	//cout<<"nfvs:"<<nfvs;



	double (*xyzdiam)[5] = new double[nfvs][5];
	double (*connarr) = new double[nfls];

	if(lookFor("@3", in_d)){
		isDiaPresent = true;

	}
	//in->seekg(0, ios::beg);

	//in->seekg(0, ios::beg);
	lookFor("@1", in);
	lookFor("@1", in);
	in->getline(tmp, 99, '\n');



	while (in->peek() != 64 && id < nfvs) {


		*in >> x;
		*in >> y;
		*in >> z;



		xyzdiam[id][0] = (double) id + 1;
		xyzdiam[id][1] = x;
		xyzdiam[id][2] = y;
		xyzdiam[id][3] = z;
		if(!isDiaPresent)
			if(isDiaSet)
				xyzdiam[id][4] = atof(dia);
			else
				xyzdiam[id][4] = .5;

		//cout<<"-id:"<<id<<" X: "<<xyzdiam[id][1]<<" --Y: "<<xyzdiam[id][2]<<" --Z: "<<xyzdiam[id][3]<<"\n";

		id++;


	}
	if (in->fail())
		in->clear();

	if(isDiaPresent){
		in->seekg(0, ios::beg);
		lookFor("@2", in);
		lookFor("@2", in);
		in->getline(tmp, 99, '\n');
		int id1 = 0;
		while (in->peek() != 64 && id1 < nfvs) {

			*in >> diam;
			xyzdiam[id1][4] = diam;
			id1++;
		}
	}
	//cout<<"id:"<<id<<"id1:"<<id1<<endl<<"\n";

	if (in->fail())
		in->clear();

	in->seekg(0, ios::beg);

	if(isDiaPresent){
		lookFor("@3", in);
		lookFor("@3", in);
	}else{
		lookFor("@2", in);
		lookFor("@2", in);
	}
	in->getline(tmp, 99, '\n');
	int id2 = 0;
	char lnseg[2000];

	//char delim = ' ';
	char * tok;

	while (in->peek() != -1) {
		in->getline(lnseg, 2000, '\n');
		//cout<<"getline22......... : "<<lnseg<<"\n";
		tok = strtok(lnseg, " ");
		//	cout<<"tok......... : "<<lnseg<<"\n";
		while (tok != NULL) {
			//	cout<<"1234...\n";
			connarr[id2] = atof(tok);
			//cout<<"connarr......... : "<<connarr[id2]<<"\n";
			tok = strtok(NULL, " \n");
			id2++;
		}
		//cout<<"the id is 11111....: "<<id2<<"\n";
	}

	int connarr_size = id2;

	//SG Code
	//Code fix to achieve Amira2.0 version conv.
	rearrangeArray(connarr, connarr_size);

	int ind = 0;
	Segment* s = NULL;
	Segment* t = NULL;
	bool foundseg = false;
	while (ind < connarr_size) {
		foundseg = false;
		int id = (int) connarr[ind];
		if (id == 0)
			cerr << "id should not be 0 more than once.." << ind << endl;
		double pid = -1;
		if (id > 0 && ((int) connarr[ind - 1]) != -1)
			pid = ((int) connarr[ind - 1]) + 1;
		else if (id > 0 && ((int) connarr[ind - 1] == -1))
			if ((t = findSegment(id + 1, soma)) != NULL) {
				//cout<<"i m in find segments......";
				pid = t->getPid();
				foundseg = true;
			} else {
				pid = -100; //setting the pid of first segment in every new line to -100 
			}

		if (id == 0) {
			//cout<<"i m in id =0....";
			if (soma == NULL) {

				x = xyzdiam[id][1];

				y = xyzdiam[id][2];

				z = xyzdiam[id][3];

				diam = xyzdiam[id][4];

				Vector* pt = new Vector(x, y, z);

				s = new Segment(id + 1, 2, pt, diam, pid);

				soma = s;

				soma->setPrevFather(s);

				s = NULL;

			}
		} else if (id > 0 && !foundseg) {

			x = xyzdiam[id][1];
			y = xyzdiam[id][2];
			z = xyzdiam[id][3];
			diam = xyzdiam[id][4];
			Vector* pt = new Vector(x, y, z);
			s = new Segment(id + 1, 2, pt, diam, pid);
			addSegment(s);
			s = NULL;

		}

		ind++;

	}

	if (strtseg != NULL) {
		processSkippedSegments(strtseg);
		int ctr = 0;
		ctr = remove(strtseg);
	}

}
;

void Neuron::rearrangeArray(double (*connarr), int connarr_size){

	int reverseFrom;
	int reverseTo;
	bool inReverse = false;

	for(int i = 1; i < connarr_size; i++)
	{

		if(connarr[i-1] == -1 && inReverse == false){

			if(!idExistsInArray(connarr,connarr[i],i-1)){
				reverseFrom = i;
				inReverse = true;
			}

		}else if(connarr[i+1] == -1 && inReverse == true){

			if(idExistsInArray(connarr,connarr[i],i)){
				reverseTo = i;
				reversePlaces(connarr,reverseFrom, reverseTo);
				inReverse = false;

			}

		}

	}

}

bool Neuron::idExistsInArray(double (*connarr), int id, int findTill){

	for(int i = 0; i < findTill; i++)
	{
		if(id == connarr[i])
			return true;
	}

	return false;

}

void Neuron::reversePlaces(double (*connarr), int reverseFrom, int reverseTo){

	double (*tempArr) = new double[reverseTo-reverseFrom+2];
	int i;
		for(i = reverseFrom; i <= reverseTo; i++)
		{
			tempArr[i-reverseFrom] = connarr[i];

		}

		for(i = reverseFrom; i <= reverseTo; i++)
		{
			connarr[i] = tempArr[reverseTo-i];

		}

}

void Neuron::processSkippedSegments(Segment* ss) {
	Segment* p = findSegment(ss->getId(), soma);
	if (p != NULL) {
		if (ss->getPid() == -100)
			ss->setPid(p->getId());
		addSegment(ss);
		reduceSkippedSegments(ss);
	} else if (ss->getNext1() != NULL)
		processSkippedSegments(ss->getNext1());
}

void Neuron::reduceSkippedSegments(Segment* r) {
	Segment* prev = r->getPrev();
	Segment* nxt = r->getNext1();
	if (prev != NULL && nxt != NULL) {
		r->getPrev()->setNext1(r->getNext1());
		r->getNext1()->setPrev(r->getPrev());
	}
	r = NULL;

}

void Neuron::OpenANAT() {
	int const MAX = 5000;
	int dend_num[MAX], br_order[MAX], br_num[MAX], seg_num[MAX], seg_type[MAX],
			pid[MAX];
	int x1[MAX], y1[MAX], z1_75[MAX], x2[MAX], y2[MAX], z2_75[MAX],
			seg_len[MAX];
	double seg_diam[MAX], z1_corr[MAX];
	Segment * old = new Segment();

	strcpy(d[0],
			"# ANAT to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;

	int NR = 0;
	while (!in->eof()) {

		while (in->peek() == 32) {
			in->get();
		}

		NR++;
		*in >> dend_num[NR];
		*in >> br_order[NR];
		*in >> br_num[NR];
		*in >> seg_num[NR];
		*in >> seg_type[NR];
		*in >> x1[NR];
		*in >> y1[NR];
		*in >> z1_75[NR];
		z1_corr[NR] = z1_75[NR];
		*in >> x2[NR];
		*in >> y2[NR];
		*in >> z2_75[NR];
		*in >> seg_len[NR];
		*in >> seg_diam[NR];
		seg_diam[NR] *= 0.01;

	}

	double somadiam = 60, deltaquadro = 0, sec_thick = 75, z2_choice[MAX],
			z_posroot, z_negroot, calc_len[MAX], sum_calc_len = 0;
	;

	for (int line = 1; line <= NR; line++) {
		if (br_order[line] == 0 && seg_num[line] == 1)
			seg_len[line] = (int) (seg_len[line] + 0.5 * somadiam); // accounts for the soma diameter
		deltaquadro = ((seg_len[line]) * (seg_len[line])
				- (x1[line] - x2[line]) * (x1[line] - x2[line]) - (y1[line]
				- y2[line]) * (y1[line] - y2[line]));
		if (deltaquadro < 0) {
			// it means that reported length is shorter than possible in this section
			if (z1_corr[line] < (z2_75[line] - 0.5 * sec_thick)) {
				z2_choice[line] = (z2_75[line] - sec_thick);
			} else {
				z2_choice[line] = z2_75[line];
			} // end if deltaquadro < 0
		} else {
			z_posroot = z1_corr[line] + sqrt(deltaquadro);
			z_negroot = z1_corr[line] - sqrt(deltaquadro);
			if ((z2_75[line] >= z_posroot && z_posroot >= (z2_75[line]
					- sec_thick)) || (z2_75[line] >= z_negroot && z_negroot
					>= (z2_75[line] - sec_thick))) {
				// Found a good z2!
				if ((z2_75[line] >= z_posroot && z_posroot >= (z2_75[line]
						- sec_thick)) && (z2_75[line] >= z_negroot && z_negroot
						>= (z2_75[line] - sec_thick))) {

					if (min(abs(z_posroot-z2_75[line]), abs(z_posroot-(z2_75[line]-sec_thick)))<min(abs(z_negroot-z2_75[line]), abs(z_posroot-(z2_75[line]-sec_thick))) ) {
						z2_choice[line]=z_posroot;
					} else {
						z2_choice[line]= z_negroot;
					}
				} else {
					if (z2_75[line] >= z_posroot && z_posroot >= (z2_75[line]-sec_thick)) {
						z2_choice[line]=z_posroot;
					} else {
						z2_choice[line]=z_negroot;
					}
				}// end else two candidates
			} // end if found a good z2
			else {

				if((min(abs(z_posroot-z2_75[line]), abs(z_negroot-z2_75[line]))) < (min(abs(z_posroot-(z2_75[line]-sec_thick)), abs(z_posroot-(z2_75[line]-sec_thick)))) ) {
					z2_choice[line]=(z2_75[line]);
				} else {
					z2_choice[line]= (z2_75[line]-sec_thick);
				}
			} // end else found a good z2
		} // end else deltaquadro <0
		calc_len[line]=sqrt((x1[line]-x2[line])*(x1[line]-x2[line])+(y1[line]-y2[line])*(y1[line]-y2[line])+(z1_corr[line]-z2_choice[line])*(z1_corr[line]-z2_choice[line]));
		if (br_order[line]==0 && seg_num[line]==1)
		calc_len[line]=calc_len[line]-0.5*somadiam; // accounts for the soma diameter
		sum_calc_len=sum_calc_len+calc_len[line];
		if (z2_choice[line] != z2_75[line]) { // need to correct z1
			for (int scannext=line+1; scannext<NR; scannext++) {
				if (dend_num[scannext]==dend_num[line] && x1[scannext]==x2[line] && y1[scannext] == y2 [line])
				z1_corr[scannext]=z2_choice[line];
			} // end for scannext
		}// end if need to correct z1
		pid[line]=-1;
		for (int findpid=1; findpid<line; findpid++) {
			if (dend_num[findpid]==dend_num[line] && x2[findpid]==x1[line] && y2[findpid]==y1[line] && (abs(z2_choice[findpid]-z1_corr[line])<0.0001))
			pid[line]=findpid;
			if(x1[line]==0 &&y1[line]==0 && z1_corr[line]==0)
			pid[line]=0;
		} // end for findpid
	} // end for line


	Vector* a=new Vector(0,0,0);
	Segment* s=new Segment(1,1,a,somadiam,-1);
	delete a;
	soma=s;
	soma->setPrevFather(s);
	old=s;
	int pidNr=1;

	pid[1]=0;
	for (int outline=1; outline <NR; outline++) {

		Vector* a=new Vector(x2[outline],y2[outline],z2_choice[outline]);
		pidNr=(pid[outline]+1);

		if(pidNr==558)
		int j=0;

		Segment* s=new Segment((outline+1),3,a,seg_diam[outline],pidNr);
		delete a;
		if(soma==NULL) {
			soma=s;
			soma->setPrevFather(s);
		} else if(old->getId()==pidNr) {

			old->addNewChild(s);
		} else {
			addSegment(s);
		}
		old=s;
	}

}

void Neuron::OpenSWC() {

	//cerr<<"Entered OpenSWC \n";

	//SG 06/15/22031
	//Added this piece of code
	strcpy(d[0],
			"# SWC to SWC conversion from L-Measure. Sridevi Polavaram: spolavar@gmu.edu");
	strcpy(d[1], "# Original fileName:");
	strcat(d[1], getFileName(name));
	strcpy(d[2], "#");
	d[3][0] = 0;

	//End of SG chnages

	//cerr<<"Copied comments \n";

	int id = 0, type = 0, pid = 0, tt = 0;
	int commId = 3;
	double x = 0, y = 0, z = 0, radius = 0, read = 0;



	if (in->fail())
		in->clear();

	//cerr<<"Seeking start \n";
	in->seekg(0, ios::beg);

	//cerr<<"before decl \n";

	Segment * old = new Segment();

	//cerr<<"decl old \n";

	while (!in->eof()) {

		while (in->peek() == 32) {

			in->get();
		}

		while (in->peek() == 35 || in->peek() == 37) {

			in->getline(d[commId], 200, '\n');

			float x, y, z;
			x = 0;
			y = 0;
			z = 0;

			if (sscanf(d[commId], "# SHRINKAGE_CORRECTION %e %e %e", &x, &y, &z)
					== 3) {
				setShrinkX(x);
				setShrinkY(y);
				setShrinkZ(z);
			}

			while (in->peek() == 32 || in->peek() == 10 || in->peek() == 13) {

				in->get();
			}


			if (commId < 99) {
				commId++;
				d[commId][0] = 0;
			}
		}

		id = 0;
		type = 0;
		x = 0;
		y = 0;
		z = 0;
		radius = 0;
		type = 0;
		pid = 0;

		*in >> id;

		//Commented to take multiple files at a time
		//if (in->eof() || in->peek() == -1) {
		//	break;
		//}
		//read++;


		*in >> type;

		*in >> x;

		*in >> y;

		*in >> z;

		*in >> radius;

		*in >> pid;

		//if after reading something is left output "Maybe error"
		if (in->peek() != 32 && in->peek() != 10 && in->peek() != 13
				&& in->peek() != -1) {
			cerr << "Maybe Unsupported format at Id:" << id << "\n";
			exit(1);
			char c = in->peek();
		}
		//Create a Segment for each line
		Vector* a = new Vector(x, y, z);
		Segment* s = new Segment(id, type, a, radius, pid);
		delete a;

		if (soma != NULL && pid == -1) {

			s->setPid(1);

		}
		//added this if condition, so that for files that have soma could be rearranged such that soma comes first. 

		if (type == 1) {

			foundsoma = true;

		}
		if (soma == NULL) {

			soma = s;

			soma->setPrevFather(s);
		} else if (old->getId() == pid) {

			old->addNewChild(s);
		} else {

			addSegment(s);
		}
		old = s;

	}

}
;
//searching for first soma compartment with type == 1
Segment * Neuron::lookForSoma(Segment *s) {
	static Segment * f = NULL;

	if (s->getType() == 1) {
		if (f == NULL) {
			f = s;
		} else {
			//f is a bifurcation and s is either continuation or tip
			if (s->getNext2() == NULL && f->getNext2() != NULL)
				f = s;
		}

	}
	if (s->getNext1() != NULL)
		lookForSoma(s->getNext1());
	if (s->getNext2() != NULL)
		lookForSoma(s->getNext2());

	return f;

}

;
//add virtual soma as the beginning point and traverse the tree from there onwards.
void Neuron::addVirtualSoma(Segment* s) {

	if (s->getType() != 1) {
		if (s->getNext1() != NULL)
			addVirtualSoma(s->getNext1());
		if (s->getNext2() != NULL)
			addVirtualSoma(s->getNext2());
	} else {

		//we are in the soma
		Segment* virt = s->clone();
		//Ghost segment
		virt->setId(s->getId() + 1);
		virt->setPid(-1);
		//reset the type of virtual segment to -1 from 1. Sri 05/24/2011
		virt->setType(-1);
		virt->setRadius(s->getRadius());
		s->setType(1);
		virt->setPrev(NULL);
		virt->setNext1(NULL);
		virt->setNext2(NULL);
		s->addNewChild(virt);
	}
}
// rearrage the swc file recursively such that the pointers are going out of the soma
Segment * Neuron::rearrangeSWC(Segment* s) {
	static Segment * ret = NULL;

	//SG Code chnages Sep 02 2011
	//Commented to fix the problem where the conversion not taking place if the
	//soma not placed on first line of swc file.
	//if (s->getType() == 1 && s->getNext1() == NULL && s->getNext2() == NULL
	//		&& found == 0) {
	if (s->getType() == 1 && found == 0) {
		found = 1;
		//s->setNext1(s->getPrev());
		//s->setPrev(s);

		rearrange(s->getPrev(), s);
		ret = s;

	} else {
		if (s->getNext1() != NULL)
			rearrangeSWC(s->getNext1());
		if (s->getNext2() != NULL)
			rearrangeSWC(s->getNext2());
	}

	return ret;
}

/*
 * SG Code added
 * New Rearrange method.
 */
void Neuron::rearrange(Segment* t, Segment* father) {

	Segment * prevOld = t->getPrev();

	//Add the soma
	if(father->getNext1() == NULL){

		father->setNext1(t);
	}else if(father->getNext2() == NULL){

		father->setNext2(t);
	}else {

		Segment * virtualSegment = father->clone();
		virtualSegment->setType(-1);
		virtualSegment->setId(1);

		Segment * next2 = father->getNext2();
		father->setNext2(virtualSegment);
		virtualSegment->setNext1(next2);
		virtualSegment->setNext2(t);
		virtualSegment->setPrev(father);
		t->setPrev(virtualSegment);
	}

	if(t->getNext1()->getType() == 1)
		t->setNext1(NULL);
	else if(t->getNext2()->getType() == 1)
		t->setNext2(NULL);

	//Rearranging backwards
	while(t->getId() != 1){

		if(prevOld->getNext1() == t){
			prevOld->setNext1(NULL);
		}else if(prevOld->getNext2() == t){
			prevOld->setNext2(NULL);
		}

		if(t->getNext1() == NULL){
			t->setNext1(prevOld);
		}else if(t->getNext2() == NULL){
			t->setNext2(prevOld);
		}
		t = prevOld;
		prevOld = prevOld->getPrev();

	}

	if(t->getNext1() == NULL){
		if(t->getNext2() != NULL){
			t->setNext1(t->getNext2());
			t->setNext2(NULL);
		}
	}

}

//void Neuron::rearrange(Segment* t, Segment* father) {
//	static Segment* prev;
//	prev = t->getPrev();
//	static Segment* next1;
//	next1 = t->getNext1();
//	static Segment* next2;
//	next2 = t->getNext2();
//
//	//if next1 or next2 are fathers then rearrange pointers
//	if (next1 == father) {
//		t->setPrev(next1);
//		if (prev != NULL) {
//			t->setNext1(prev);
//			t->setNext2(next2);
//		} else {
//			t->setNext1(next2);
//			t->setNext2(NULL);
//
//		}
//
//	} else if (next2 == father) {
//
//		t->setPrev(next2);
//		t->setNext2(prev);
//		t->setNext1(next1);
//
//	}
//
//	if (t->getNext1() != NULL)
//		rearrange(t->getNext1(), t);
//	if (t->getNext2() != NULL)
//		rearrange(t->getNext2(), t);
//}

void Neuron::setParameters() {
	setParameters(soma);
}

//set all necessary parameters inside each single segment required by successive Elaborations

void Neuron::setParameters(Segment* s) {

	static double dist = 0;
	if (s->getPid() == -1) {

		s->setPathDistance(0);

	} else {

		s->setPathDistance(
				s->getPrev()->getPathDistance()
						+ s->getPrev()->getEnd()->distance(s->getEnd()));

	}


	Vector * v1 = s->getEnd();

	Vector * v2 = soma->getEnd();

	dist = s->getEnd()->distance(soma->getEnd());
	s->setSomaDistance(dist);

	s->setNeuronName(name);

	if (s->getPid() == -1) {
		s->setLength(0);
	} else {
		s->setLength(s->getEnd()->distance(s->getPrev()->getEnd()));
	}



	//check if terminate then set nrseg=1;
	if (s->isTerminate())
		s->setNrTip(1);

	if (s->getNext1() != NULL) {

		setParameters(s->getNext1());
		s->setNrTip(s->getNext1()->getNrTip());

	}

	if (s->getNext2() != NULL) {

		setParameters(s->getNext2());

		s->setNrTip(s->getNext1()->getNrTip() + s->getNext2()->getNrTip());

	}

}

void Neuron::setOrder(Segment* s) {

	if (s->getType() == 1 || s->getId() == 1 || s == soma) {


		s->setOrder(0);

	} else if (s->getPrev()->getNext1() != NULL && s->getPrev()->getNext2()
			!= NULL && s->getPrev()->getType() > 1) {


		s->setOrder(s->getPrev()->getOrder() + 1);

	} else {

		s->setOrder(s->getPrev()->getOrder());

	}


	if (s->getNext1() != NULL)
		setOrder(s->getNext1());
	if (s->getNext2() != NULL)
		setOrder(s->getNext2());

}
// find the right spot to insert s given its pid
void Neuron::addSegment(Segment* s) {

	int pid = s->getPid();

	Segment* p = findSegment(pid, soma);

	//SG Code added
	//Add this piece of code to avoid * marked.... erro on gui
	if(s->getId() == 0){

	}
	//End of SG code
	else if (p == NULL) {
		ferror = true;
		cerr << "Did not found PARENT!!!!!!!!  " << s->getId() << " "
				<< s->getEnd()->getX() << " " << s->getEnd()->getY() << " "
				<< s->getEnd()->getZ() << "\n";

	} else {

		p->addNewChild(s);
	}
}

void Neuron::addSkippedSegments(Segment* firstseg, Segment* prev, Segment* p) {

	if (firstseg->getNext1() != NULL) {
		addSkippedSegments(firstseg->getNext1(), firstseg, p);
	} else {
		firstseg->setNext1(p);
		firstseg->setPrev(prev);
	}
}

// Return the segment that with the given id starting from Father, NULL if it is missing.
Segment* Neuron::findSegment(int id, Segment* father) {

	Segment* tmp = NULL;

	if (father->getId() == id) {
		tmp = father;

	}

	if (tmp == NULL && father->getNext1() != NULL) {
		tmp = findSegment(id, father->getNext1());

	}
	if (tmp == NULL && father->getNext2() != NULL) {
		tmp = findSegment(id, father->getNext2());

	}

	return tmp;
}

Segment* Neuron::buildFathers(Segment* start, Segment * prevFather) {

	//cerr<<"Start for seg:"<<start->getId()<<"\n";
	Segment* tmp = NULL;
	Segment* tmp1 = NULL;
	Segment* tmp2 = NULL;

	Segment* tmpNext1 = start->getNext1();
	Segment* tmpNext2 = start->getNext2();

	start->setPrevFather(prevFather);

	if (tmpNext1 == NULL && tmpNext2 == NULL) {
		//termination point acts as father
		return start;
	}

	//Check if start is a Father!!
	if (start->isFather()) {
		prevFather = start;
		tmp = start;
	}

	//for every child recurse
	if (tmpNext1 != NULL) {

		tmpNext1->setPrev(start);
		tmp1 = buildFathers(tmpNext1, prevFather);
		start->setNextFather1(tmp1);
	}
	if (tmpNext2 != NULL) {
		//set prev
		tmpNext2->setPrev(start);
		tmp2 = buildFathers(tmpNext2, prevFather);
		start->setNextFather2(tmp2);
	}

	if (tmp == NULL && tmp1 != NULL) {
		tmp = tmp1;
	}
	if (tmpNext1 == NULL && tmpNext2 == NULL) {
		//if terminating point, return it's pointer
		tmp = start;
	}
	//cerr<<"end for seg:"<<"\n";
	return tmp;
}

//Look for segments with same x,y,z and remove them!

//The method which removes recursion
//and controls the taversal through the segments.
void Neuron::findDuplicate(Segment* s) {

       Segment* current = s;
       Segment* prev_segment = NULL;

	   //Recursion ID , for marking the segemnts. The segments are marked with ID if they are visited.
       //This helps in forward and backward traversal across segments.
       int avoidRecursionMethodId;

       if(s->getNext1()->avoidRecursiveId != 0)
    	   avoidRecursionMethodId = s->getNext1()->avoidRecursiveId+1;
       else
    	   avoidRecursionMethodId = rand() % 1000;


       bool next1Covered, next2Covered;

	   //Continious loop
       while(true){



				//Check if the node is traversed, and if not
               if(current->avoidRecursiveId != avoidRecursionMethodId){
					//The core functionality of find dupilcate w/o recursion
                       findDuplicate1(current);

					   //Marking the segment, that it has been traversed
                       current->avoidRecursiveId = avoidRecursionMethodId;
               }


               next1Covered = false;
               next2Covered = false;

			   //Checking if next1 is present
               if(current->getNext1() != NULL){

						//Checking if next1 has been traversed or not, if not traversed
                       if(current->getNext1()->avoidRecursiveId != avoidRecursionMethodId){
								//Setting Next1 to be traversed next
                    	   	   prev_segment = current;
                               current = current->getNext1();
                               current->setPrev(prev_segment);
                               next1Covered = true;
                       }

               }

			   //Checking if next2 is present
               if(current->getNext2() != NULL && (!next1Covered)){

				   //Checking if next2 has been traversed or not, if not traversed
                       if(current->getNext2()->avoidRecursiveId != avoidRecursionMethodId){

						   //Setting Next2 to be traversed next
                    	   	   prev_segment = current;
                               current = current->getNext2();
                               current->setPrev(prev_segment);
                               next2Covered = true;
                       }
               }


			   //Checking if next one and next two both are traversed, if true
               if((!next1Covered) && (!next2Covered)){


					 //Sets for backward traversal
                       current = current->getPrev();

                       if(current == NULL)
                    	   break;

					   //If back to the first segment then break
                       if(current->getId() == 1 && current->getNext2() == NULL)
                    	   break;
                       else if(current->getId() == 1 && current->getNext2()->avoidRecursiveId == avoidRecursionMethodId){
                    	   break;
                       }
       }


       }


       }





void Neuron::findDuplicate1(Segment* s) {

       if(s == NULL)
               return;

       if (s->getNext1() == NULL && s->getNext2() == NULL)
               return;
       Segment *tmp;
       double x, y, z;
       x = s->getEnd()->getX();
       y = s->getEnd()->getY();
       z = s->getEnd()->getZ();

       if (s->getNext1() != NULL && s->getEnd()->getX()
                       == s->getNext1()->getEnd()->getX() && s->getEnd()->getY()
                       == s->getNext1()->getEnd()->getY() && s->getEnd()->getZ()
                       == s->getNext1()->getEnd()->getZ()) {

               cerr << "Found Duplicate type:" << s->getType() << " X:"
                               << s->getEnd()->getX() << " " << s->getEnd()->getY() << " "
                               << s->getEnd()->getZ() << " ";

               if (s->getNext2() == NULL) {
                       tmp = s->getNext1();
                       s->setNext1(tmp->getNext1());

                       if (tmp->getNext1() != NULL)
                               tmp->getNext1()->setPrev(s);

                       s->setNext2(tmp->getNext2());
                       if (tmp->getNext2() != NULL)
                               tmp->getNext2()->setPrev(s);

                       tmp->~Segment();
                       cerr << "removed!\n";

               } else {
                       if (s->getNext1()->getType() > 0) {
                               cerr << "Triforcation Readjusted!\n";
                               s-> adjustTriforc();
                       } else {
                               cerr << s->getId();
                               cerr << "virtual segment-> skipped!\n";

                       }

               }
       }

       else if (s->getNext2() != NULL && s->getEnd()->getX()
                       == s->getNext2()->getEnd()->getX() && s->getEnd()->getY()
                       == s->getNext2()->getEnd()->getY() && s->getEnd()->getZ()
                       == s->getNext2()->getEnd()->getZ()) {

               cerr << "Found Duplicate type:" << s->getType() << " X:"
                               << s->getEnd()->getX() << " " << s->getEnd()->getY() << " "
                               << s->getEnd()->getZ() << " ";

               if (s->getNext2()->getType() > 0) {
                       cerr << "Triforcation Readjusted!\n";
                       s->adjustTriforc();

               } else {
                       cerr << "virtual segment-> skipped!\n";

               }
       }

	   //Removed the recusrion code from this place
}

void Neuron::exploreExecute(void*f) {

}

void Neuron::adjustShrinkage(Segment* s) {
	static double x, y, z;
	static Vector* a;
	a = s->getEnd();
	x = a->getX() * getShrinkX();
	y = a->getY() * getShrinkY();
	z = a->getZ() * getShrinkZ();
	a->setX(x);
	a->setY(y);
	a->setZ(z);

	if (s->getNext1() != NULL) {
		adjustShrinkage(s->getNext1());
	}
	if (s->getNext2() != NULL) {
		adjustShrinkage(s->getNext2());
	}
}

void Neuron::renumerateId() {
	renumerateId(soma);
}

void Neuron::renumerateId(Segment* s) {

	static int id = 0;

	if (s == soma) {
		id = 0;

	};

	if (s->getType() > 0) {

		id++;
		s->setId(id);

		if (s->getId() > 0 && s->getType() > 0) {

			if (s->getPrev() != NULL) {
				Segment * tmp = s->getPrev();

				s->setPid(tmp->getId());
			} else {
				s->setPid(-1);
			}
		}

	}
	nrSeg++;
	if (s->getNext1() != NULL) {

		renumerateId(s->getNext1());
	}
	if (s->getNext2() != NULL) {

		renumerateId(s->getNext2());
	}
}

void Neuron::LookForTrees(Segment* tmp, std::string dirname) {

	Segment *next1 = tmp->getNext1();
	Segment *next2 = tmp->getNext2();

	if (tmp->getPid() != -1 && tmp->getPrev() != NULL) {

		if (tmp->getType() != tmp->getPrev()->getType() && tmp->getType() > 1
				&& (tmp->getPrev()->getType() == -1
						|| tmp->getPrev()->getType() == 1)) {


			#ifdef OS_WIN
			//Windows specific stuff
			std::string treename = name_str+"tree"+itoa(namectr,buff,10);
						treename+=tree_ext;
						//SG Code to identify the tree type
						new_dir_parent = dir_str+dirname+"\\";
						if(_mkdir(new_dir_parent.c_str())==0){
							cerr<<"created a new subdirectory for sub trees! \n";
						}
						if(tmp->getType() == 2)
							new_dir = dir_str+dirname+"\\Axon\\";
						else if(tmp->getType() == 3)
							new_dir = dir_str+dirname+"\\Basal\\";
						else if(tmp->getType() == 4)
							new_dir = dir_str+dirname+"\\Apical\\";
						else
							new_dir = dir_str+dirname+"\\";
						cerr<<new_dir.c_str()<<"\n";
						if(_mkdir(new_dir.c_str())==0){
							cerr<<"created a new tree type for sub trees! \n";
						}
						//End of SG code chnages
						std::string fulltreename = new_dir+treename;

			#else
			//Normal stuff

			sprintf(buff, "%d", namectr);
			//cerr<<"name_str:"<<name_str<<"\n";
			//cerr<<"buff:"<<buff<<"\n";	
			std::string treename = name_str + "tree" + buff;


			//SG Code to identify the tree type
			std::string treetype = "";
			if(tmp->getType() == 2)
				treetype = "/Axon";
			else if(tmp->getType() == 3)
				treetype = "/Basal";
			else if(tmp->getType() == 4)
				treetype = "/Apical";
			else
				treetype = "";

			treename += tree_ext;
			new_dir = dir_str + dirname + treetype;
			//	cout<<"---"<<new_dir.c_str()<<"\n";

			std::string mk = "mkdir -p " + new_dir;


			//End of SG code chnages

			//SG code fix 12/17/2011
			//The condition commented to fix the code.
			//if (LookForTreesFlag == 0) {
				system(mk.c_str());
			//	LookForTreesFlag = 1;
				//cout << "created a new subdirectory for subtrees!";
			//}
			std::string fulltreename = new_dir + "//" + treename;
			//cerr<<"treename "<<fulltreename.c_str()<<"\n";

			#endif

			ofstream treefile(fulltreename.c_str(), ios::binary);
			flag = 0;
			cerr << "tmp:" << tmp->getId() << " " << tmp->getEnd()->getX()
					<< " " << tmp->getEnd()->getY() << " "
					<< tmp->getEnd()->getZ() << " " << tmp->getRadius() << " "
					<< tmp->getPid() << "\n";
			saveTree(tmp, treefile);
			namectr++;

		}
	}

	if (next1 != NULL) {

		LookForTrees(next1, dirname);
	}
	if (next2 != NULL) {

		LookForTrees(next2, dirname);
	}
}

//to save the tree relative to its position in a seperate file
void Neuron::saveTree(Segment *newtree, ofstream & treeout) {
	double newID = 0, newPID = 0;
	//std::string str = nname;
	//open file
	//redirect print to file

	//print prev of newtree
	// print newtree
	//call recursive on saveTreeSWC
	Segment *firstSegment = newtree;
	if (flag == 0) {
		ctr = 0;
		rf = firstSegment->getId();
		newID = 1;//firstSegment->getPrev()->getId()-rf+1;
		newPID = -1;//firstSegment->getPrev()->getPid()-rf+1;
		if (firstSegment->getPrev()->getPid() == -1)
			cerr << "virtual id " << firstSegment->getPrevFather()->getId()
					<< "pid " << firstSegment->getPrevFather()->getPid()
					<< "\n";
		//SG code change
		//To avoid the virtual compartments we chnage type -1 to 1
		if(firstSegment->getPrev()->getType() == -1)
				treeout<<newID<<" "<<1<<" "<<firstSegment->getPrev()->getEnd()->getX()<<" "<<firstSegment->getPrev()->getEnd()->getY()<<" "<<firstSegment->getPrev()->getEnd()->getZ()<<" "<<firstSegment->getPrev()->getRadius()<<" "<<newPID<<"\n";
			else
				treeout<<newID<<" "<<firstSegment->getPrev()->getType()<<" "<<firstSegment->getPrev()->getEnd()->getX()<<" "<<firstSegment->getPrev()->getEnd()->getY()<<" "<<firstSegment->getPrev()->getEnd()->getZ()<<" "<<firstSegment->getPrev()->getRadius()<<" "<<newPID<<"\n";
		//End of SG code change

		flag = 1;
	}

	newID = firstSegment->getId() - rf + 2;
	newPID = firstSegment->getPid() - rf + 2;
	if (newPID == 0)
		newPID = -1;
	if (newID == 2)
		newPID = 1;
	treeout << newID << " " << firstSegment->getType() << " "
			<< firstSegment->getEnd()->getX() << " "
			<< firstSegment->getEnd()->getY() << " "
			<< firstSegment->getEnd()->getZ() << " "
			<< firstSegment->getRadius() << " " << newPID << "\n";

	if (firstSegment->getNext1() != NULL) {

		saveTree(firstSegment->getNext1(), treeout);
	}
	cerr << "\n";
	if (firstSegment->getNext2() != NULL) {

		saveTree(firstSegment->getNext2(), treeout);
	}

}

void Neuron::printSWC(char* fname) {
    int i = 0;
    char nm[300];

    char lineData[1000];

    strcpy(nm, fname);

    ofstream tfile(nm);//, ios::binary );

    //SG Code chnages 03/23/2011
    //The D crossed the size by large margin in case of bigger comments in the file
    while (d[i][0] != NULL) {
        tfile << d[i++] << "\n";
        //tfile<<d[i++];
        if (i == 3)
            break;

    }

    in_d->seekg(0, ios::beg);
    while (in_d->peek() == 35 || in_d->peek() == 37) {

        in_d->getline(lineData, 999, ' sg\n');
        tfile << lineData << "\n";

    }

    //End of the SG code chnages

    printSWC(soma, tfile);

    tfile.close();
}

void Neuron::printSWC() {
	int i = 0;
	char nm[300];

	char lineData[1000];

	strcpy(nm, name);

	strcat(nm, ".swc");

	ofstream tfile(nm);//, ios::binary );

	//SG Code chnages 03/23/2011
	//The D crossed the size by large margin in case of bigger comments in the file
	while (d[i][0] != NULL) {
		tfile << d[i++] << "\n";
		//tfile<<d[i++];
		if (i == 3)
			break;

	}

	in_d->seekg(0, ios::beg);
	while (in_d->peek() == 35 || in_d->peek() == 37) {

		in_d->getline(lineData, 999, ' sg\n');
		tfile << lineData << "\n";

	}

	//End of the SG code chnages

	printSWC(soma, tfile);

	tfile.close();
}

void Neuron::printSWC(Segment * s, ofstream & out) {
	//SG 03/30/2011
	//round2Digits func is added in order to truncate the
	//values of SWC converted files.
	if (s->getId() > 0 && s->getType() > 0)
		out << " " << s->getId() << " " << s->getType() << " " << round2Digits(
				s->getEnd()->getX()) << " "
				<< round2Digits(s->getEnd()->getY()) << " " << round2Digits(
				s->getEnd()->getZ()) << " " << s->getRadius() << " "
				<< s->getPid() << "\n";

	//End of SG chnages
	if (s->getNext1() != NULL) {
		printSWC(s->getNext1(), out);
	}
	if (s->getNext2() != NULL) {
		printSWC(s->getNext2(), out);
	}
}

void Neuron::printDXF(Segment *s, ofstream & out) {
	out << "999\nL-Measure\n999\n3d faces List\n0\nSECTION\n2\nENTITIES\n0\n";
	printDXFint(s, out);
	out << "ENDSEC\n0\nEOF\n#";

}

void Neuron::printDXF() {
	int i = 0;
	char nm[300];
	strcpy(nm, name);
	strcat(nm, ".dxf");

	ofstream tfile(nm);//, ios::binary );

	printDXF(soma, tfile);

}

void Neuron::printDXFint(Segment*s, ofstream & out) {
	Segment *prev = s->getPrev();
	if (prev != NULL && s != prev && s->getType() > 0) {

		printDXF(s, prev, out);
	}

	if (s->getNext1() != NULL) {
		printDXFint(s->getNext1(), out);
	}
	if (s->getNext2() != NULL) {
		printDXFint(s->getNext2(), out);
	}

}

void Neuron::printDXF(Segment*s, Segment*prev, ofstream & out) {
	char* color = "62\n100\n";
	;

	double RATIO = 1000;
	int ord = s->getOrder();
	if (s->getPid() > 0) {
		switch (ord) {

		case 2:
			color = "62\n100\n";
			break;

		}
	}

	static double fac = 1;
	double rad = prev->getRadius();
	double length = prev->getEnd()->distance(s->getEnd());
	double x, y, z, x1, y1, z1;
	x = (prev->getEnd()->getX());
	y = (prev->getEnd()->getY());
	z = (prev->getEnd()->getZ());
	x1 = (s->getEnd()->getX());
	y1 = (s->getEnd()->getY());
	z1 = (s->getEnd()->getZ());
	double xref, yref, zref;
	xref = x1 - x;
	yref = y1 - y;
	zref = z1 - z;
	double tm = sqrt(xref * xref + yref * yref + zref * zref);
	xref /= tm;
	yref /= tm;
	zref /= tm;

	Vector * ref, *point, *xcur, *ycur, *zcur, *xdir, *ydir, *org;
	//direction fo extrusion for obtaining the cylinder
	ref = new Vector(xref, yref, zref);
	org = new Vector(0, 0, 0);
	point = new Vector(x, y, z);

	xcur = new Vector(1, 0, 0);
	ycur = new Vector(0, 1, 0);
	zcur = new Vector(0, 0, 1);
	xdir = new Vector();
	ydir = new Vector();

	Vector *a, *b;
	double aa, gg, u;
	u = M_PI / 2;
	a = new Vector(1, 0, 0);
	a->rotate(M_PI / 4, 0, 0);
	a = new Vector(1234, -123, 20);
	b = new Vector(a->getX(), a->getY(), 0);
	aa = org->angleR(b, xcur);

	a->rotate(0, 0, -aa);

	b = new Vector(a->getX(), 0, a->getZ());
	gg = org->angleR(b, xcur);
	a->rotate(0, -gg, 0);

	//arbitrary axis algorithm
	if (xref < (1 / 64) && yref < (1 / 64)) {
		xdir->crossProduct(org, ycur, ref);
	} else {
		xdir->crossProduct(org, zcur, ref);

	}
	ydir->crossProduct(org, ref, xdir);

	double alpha, beta, gamma;

	gamma = org->angleR(new Vector(ycur->getX(), ycur->getY(), 0), ydir);
	xcur->rotate(0, 0, -gamma);
	ycur->rotate(0, 0, -gamma);
	alpha = org->angleR(ycur, ydir);
	ref->rotate(0, -alpha, 0);
	beta = org->angleR(zcur, ref) - M_PI / 2;

	point->rotate(alpha, beta, gamma);
	x = point->getX();
	y = point->getY();
	z = point->getZ();

	if (false) {

		out << "CIRCLE\n";
		out << color;

		//X
		out << "10\n" << x / RATIO << "\n";
		//Y
		out << "20\n" << y / RATIO << "\n";
		//Z
		out << "30\n" << z / RATIO << "\n";
		//length
		out << "39\n" << length / RATIO << "\n";

		//radius
		out << "40\n" << rad / RATIO << "\n";

		out << "210\n" << xref / RATIO << "\n";
		//Y1
		out << "220\n" << yref / RATIO << "\n";
		//Z1
		out << "230\n" << zref / RATIO << "\n0\n";

	}
	if (true) {

		out << "3DFACE\n8\n2\n";
		out << color;
		//X1
		out << "10\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y1
		out << "20\n" << (s->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z1
		out << "30\n" << (s->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X2
		out << "11\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y2
		out << "21\n" << (s->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z2
		out << "31\n" << (s->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X3
		out << "12\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y3
		out << "22\n" << (prev->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z3
		out << "32\n" << (prev->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X4
		out << "13\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y4
		out << "23\n" << (prev->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z4
		out << "33\n" << (prev->getEnd()->getZ() + fac * rad) / RATIO
				<< "\n0\n";
		//---------------------------------------------------------------------------
		//second face
		out << "3DFACE\n8\n2\n";
		out << color;
		//X1
		out << "10\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y1
		out << "20\n" << (s->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z1
		out << "30\n" << (s->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X2
		out << "11\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y2
		out << "21\n" << (s->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z2
		out << "31\n" << (s->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X3
		out << "12\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y3
		out << "22\n" << (prev->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z3
		out << "32\n" << (prev->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X4
		out << "13\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y4
		out << "23\n" << (prev->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z4
		out << "33\n" << (prev->getEnd()->getZ() - fac * rad) / RATIO
				<< "\n0\n";
		//---------------------------------------------------------------------------
		//third face
		out << "3DFACE\n8\n2\n";
		out << color;
		//X1
		out << "10\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y1
		out << "20\n" << (s->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z1
		out << "30\n" << (s->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X2
		out << "11\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y2
		out << "21\n" << (s->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z2
		out << "31\n" << (s->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X3
		out << "12\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y3
		out << "22\n" << (prev->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z3
		out << "32\n" << (prev->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X4
		out << "13\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y4
		out << "23\n" << (prev->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z4
		out << "33\n" << (prev->getEnd()->getZ() + fac * rad) / RATIO
				<< "\n0\n";
		//---------------------------------------------------------------------------
		//fourth face
		out << "3DFACE\n8\n2\n";
		out << color;
		//X1
		out << "10\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y1
		out << "20\n" << (s->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z1
		out << "30\n" << (s->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X2
		out << "11\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y2
		out << "21\n" << (s->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z2
		out << "31\n" << (s->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X3
		out << "12\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y3
		out << "22\n" << (prev->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z3
		out << "32\n" << (prev->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X4
		out << "13\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y4
		out << "23\n" << (prev->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z4
		out << "33\n" << (prev->getEnd()->getZ() + fac * rad) / RATIO
				<< "\n0\n";
		//---------------------------------------------------------------------------
		//fifth face : closing the box
		out << "3DFACE\n8\n2\n";
		out << color;
		//X1
		out << "10\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y1
		out << "20\n" << (s->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z1
		out << "30\n" << (s->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X2
		out << "11\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y2
		out << "21\n" << (s->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z2
		out << "31\n" << (s->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X3
		out << "12\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y3
		out << "22\n" << (s->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z3
		out << "32\n" << (s->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X4
		out << "13\n" << (s->getEnd()->getX()) / RATIO << "\n";
		//Y4
		out << "23\n" << (s->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z4
		out << "33\n" << (s->getEnd()->getZ() + fac * rad) / RATIO << "\n0\n";
		//---------------------------------------------------------------------------
		//six face
		out << "3DFACE\n8\n2\n";
		out << color;
		//X1
		out << "10\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y1
		out << "20\n" << (prev->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z1
		out << "30\n" << (prev->getEnd()->getZ() + fac * rad) / RATIO << "\n";

		//X2
		out << "11\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y2
		out << "21\n" << (prev->getEnd()->getY() - fac * rad) / RATIO << "\n";
		//Z2
		out << "31\n" << (prev->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X3
		out << "12\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y3
		out << "22\n" << (prev->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z3
		out << "32\n" << (prev->getEnd()->getZ() - fac * rad) / RATIO << "\n";

		//X4
		out << "13\n" << (prev->getEnd()->getX()) / RATIO << "\n";
		//Y4
		out << "23\n" << (prev->getEnd()->getY() + fac * rad) / RATIO << "\n";
		//Z4
		out << "33\n" << (prev->getEnd()->getZ() + fac * rad) / RATIO
				<< "\n0\n";
		//---------------------------------------------------------------------------
	}

}

void Neuron::electrotonic(Segment * t, double rm, double ri, int type) {
	electrotonic(t, rm, ri, type, 0, 0, 0);
}

void Neuron::electrotonic(Segment * t, double rm, double ri, int type,
		double xOffset, double yOffset, double zOffset) {

	double xLoc = 0, yLoc = 0, zLoc = 0;
	if (t->getPrev() != NULL) {
		//perfom offset, so if there was any correction the part will have the same direction
		Vector * end = t->getEnd();
		end->setX(end->getX() + xOffset);
		end->setY(end->getY() + yOffset);
		end->setZ(end->getZ() + zOffset);

		{
			Vector * prev = t->getPrev()->getEnd();
			double scale = sqrt(t->getDiam() / 4.0) * sqrt(rm / ri);
			scale = 2;

			//rescale in proportion to the amount on each axis
			xLoc = scale * (end->getX() - prev->getX());
			yLoc = scale * (end->getY() - prev->getY());
			zLoc = scale * (end->getZ() - prev->getZ());
			end->setX(prev->getX() + xLoc);
			end->setY(prev->getY() + yLoc);
			end->setZ(prev->getZ() + zLoc);

		}

	}
	//recursion
	if (t->getNext1() != NULL)
		electrotonic(t->getNext1(), rm, ri, type, xOffset + xLoc,
				yOffset + yLoc, zOffset + zLoc);
	if (t->getNext2() != NULL)
		electrotonic(t->getNext2(), rm, ri, type, xOffset + xLoc,
				yOffset + yLoc, zOffset + zLoc);

}
;

void Neuron::translation(double x, double y, double z, Segment *s) {
	s->getEnd()->setX(s->getEnd()->getX() - x);
	s->getEnd()->setY(s->getEnd()->getY() - y);
	s->getEnd()->setZ(s->getEnd()->getZ() - z);
	if (s->getNext1() != NULL)
		translation(x, y, z, s->getNext1());
	if (s->getNext2() != NULL)
		translation(x, y, z, s->getNext2());

}
;

void Neuron::translation(double x, double y, double z) {
	translation(x, y, z, soma);
}
;

void Neuron::rotate(double x, double y, double z) {
	rotate(x, y, z, soma);
}
;

void Neuron::rotateTo(Vector * a) {
	Vector *b = new Vector();
	Vector *yAxis = new Vector(0, 1, 0);
	Vector * org = new Vector();
	b->crossProduct(org, yAxis, a);
	double theta;
	theta = org->angle(a, yAxis);

	rotateAxis(b, theta * M_PI / 180, soma);

	delete b;
	delete yAxis;
	delete org;
}
;

void Neuron::rotateAxis(Vector * a, double theta, Segment *s) {
	if (s == soma) {
		doPCA();
	}

	s->getEnd()->rotateAxis(a, theta);

	if (s->getNext1() != NULL)
		rotateAxis(a, theta, s->getNext1());
	if (s->getNext2() != NULL)
		rotateAxis(a, theta, s->getNext2());

}
;

void Neuron::rotate(double alpha, double beta, double gamma, Segment *s) {
	static double cx = 0, sx = 0, cy = 0, sy = 0, cz = 0, sz = 0;
	if (s == soma) {
		alpha = alpha * M_PI / 180;
		beta = beta * M_PI / 180;
		gamma = gamma * M_PI / 180;

		cx = cos(alpha);
		sx = sin(alpha);
		cy = cos(beta);
		sy = sin(beta);
		cz = cos(gamma);
		sz = sin(gamma);
	}
	double x = s->getEnd()->getX();
	double y = s->getEnd()->getY();
	double z = s->getEnd()->getZ();
	double x1, y1, z1;
	x1 = x, y1 = y;
	z1 = z;

	x1 = x * cy * cz + y * cy * sz - z * sy;
	y1 = x * (sx * sy * cz - cx * sz) + y * (sx * sy * sz + cx * cz) + z * sx
			* cy;
	z1 = x * (cx * sy * cz + sx * sz) + y * ((cx * sy * sz - sx * cz)) + z
			* (cx * cy);

	s->getEnd()->setX(x1);
	s->getEnd()->setY(y1);
	s->getEnd()->setZ(z1);
	if (s->getNext1() != NULL)
		rotate(alpha, beta, gamma, s->getNext1());
	if (s->getNext2() != NULL)
		rotate(alpha, beta, gamma, s->getNext2());

}
;

void Neuron::rotateP(Vector * P) {
	rotateP(P, soma);
}
;

void Neuron::rotateP(Vector *P, Segment *s) {

	s->getEnd()->rotateP(P);

	if (s->getNext1() != NULL)
		rotateP(P, s->getNext1());
	if (s->getNext2() != NULL)
		rotateP(P, s->getNext2());

}
;
/**
 * This function checks if the neurite has the given type or not.
 */
int Neuron::hasDendriteType(Segment * t) {
	static int id = 0;
	if (t == soma)
		id = 1;
	//cerr<<t->getType()<<":";
	//modified  >=2 to include the axon values too. This will ensure that all points are shifted around center of mass for PCA. SP 07/082012.
	if (t->getType() == 3 || t->getType() == 4) {
		id++;
	}

	if (t->getNext1() != NULL)
		hasDendriteType(t->getNext1());
	if (t->getNext2() != NULL)
		hasDendriteType(t->getNext2());

	return id;
}
;

void Neuron::pca() {
	float **data, **datacopy, **symmat, **symmat2, *evals, *interm; //create a matrix that contains all points
	//set flag for center surround soma. turning it off works better with asymmetry check.
	int centeronsoma=0;

	int i, j;

	int n = nrSeg;
	cerr<<"#seg:"<<n<<"\n";

	int m = 3;
	//initialize data matrix n x m, n = #compartments and m =3
	data = matrix(n, m);
	for (i = 1; i <= n; i++)
		for (j = 1; j <= m; j++) {
			if (j % 100 == 0) {
				int kk = j;
			}
			data[i][j] = 0;
		}

	if (data == NULL)
		exit(-1);

	int dendriteVal = -1; int zresolution = -1;
	//check if data array has dendrite points populated
	int hasDend = hasDendriteType(soma);
	//cerr<<"total:"<<hasDend<<"\n";
	if(hasDend>1)
		dendriteVal = 1;//sets to 1, if the input file has dendrites.
	else
		dendriteVal = 0;//if there are no dendrites then, pca uses everything to populate the array.
	//check for zresolution, if >0.3, then fill z-coordinate of PCA data array with zeros, else, no channge.

	double zctr = zresolutionEstimator(soma);
	double zthreshold = 0.3; // a 30% threshold will ensure that 30% of the Z coordinates are same.
	cerr<<"z proportion.."<<zctr/n<<"\n";
	if (zctr/n >= zthreshold){
		zresolution = 1;
	}


	//fill data array just with dendrite points
	int npoints = pcaDataPoints(soma, data, dendriteVal);

	int zsize = -1;
	//cerr<<"found dendrite:"<<dendriteVal<<" pca points:"<<npoints<<"\n";

	//make a copy of set the Z values to
	if (zresolution == 1){
		datacopy = matrix(n,2);
		cerr<<"saving zcoords.."<<"\n";
		zsize = saveZcoords(false, soma, datacopy);
		//cerr<<"z coords#"<<zsize<<"last element"<<datacopy[zsize][1]<<"-"<<datacopy[zsize][2]<<"\n";
		//after saving z-coordinates, reset all Z values to zero, before PCA.
		for (i = 1; i <= n; i++){
			data[i][3] = 0; //re-initialize z coordinate zero.
		}
	}

	symmat = matrix(m, m); /* Allocation of correlation (etc.) matrix */
	float *avg = vector(m);//vector that store center of mass point.

	/*
	for (i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			//just printing to check
			cerr<<data[i][1]<<","<<data[i][2]<<","<<data[i][3]<<"\n";
		}
	}*/
	/*
	 *compute the covariance matrix.
	 *depending to centeronsoma is turned on or not, the PCA uses center of soma or center of mass. SP 07/11/2012.
	 */
	//n = npoints; //the PCA array is adjusted to dendritic points, which is npoints
	covcol1(data, npoints, m, symmat, avg, centeronsoma); //avg is populated in this function
	//cerr<<"avg at first: "<<avg[1]<<" "<<avg[2]<<" "<<avg[3]<<"\n";
	//print symmat here
	/*
	for (i = 1; i <= m; i++) {
		for (int j = 1; j <= m; j++) {
		//just printing to check
			cerr<<symmat[i][j]<<"\n";
		}
	}
	*/
	evals = vector(m); /* Storage alloc. for vector of eigenvalues */
	interm = vector(m); /* Storage alloc. for 'intermediate' vector */
	symmat2 = matrix(m, m); /* Duplicate of correlation (etc.) matrix */
	for (i = 1; i <= m; i++) {
		for (int j = 1; j <= m; j++) {
			symmat2[i][j] = symmat[i][j]; /* Needed below for col. projections */
		}
	}

	/**
	 * Although, this code seem to be using rotation matrix symmat from the function parameters, it is not clear how it is used
	 */
	tred2(symmat, m, evals, interm); /* Triangular decomposition */
	tqli(evals, interm, m, symmat); /* Reduction of sym. trid. matrix */
	//print symmat again

	/*
	cerr<<"printing symmat again..."<<"\n";
	for (i = 1; i <= m; i++) {
		for (int j = 1; j <= m; j++) {
			cerr<<symmat[i][j]<<"\n";
		}
	}*/

	/*
	float a = evals[1];float b = evals[2];float c=evals[3];
	evals[3]= a; evals[1]=c;//testing by flipping the evals to match with the PCA order as seen in the resulting file
	a = evals[1]; c = evals[3];//re-initializing the temporary variables
	cerr<<"evals order: "<<evals[1]<<" "<<evals[2]<<" "<<evals[3]<<"\n";

	//
	float **symmat_tmp = matrix(m,m);
	for (i = 1; i <= m; i++) {
		for (int j = 1; j <= m; j++) {
			//cerr<<symmat[i][j]<<"\n";
			symmat_tmp[i][j] = symmat[i][j];
		}
	}


	//rearranging the eigen values in evals and rotation matrix in symmat in descending order didn't seem to matter for PCA rotation.
	//the PCA rotation seem to be done using the xcur,absX variables computed from cursor and cursorAbs function.

	if(a>b && a>c){//a is first
		if(c>b){//acb
			evals[2]=c;evals[3]=b;
			symmat[1][2] = symmat_tmp[1][3];symmat[1][3] = symmat_tmp[1][2];
			symmat[2][2] = symmat_tmp[2][3];symmat[2][3] = symmat_tmp[2][2];
			symmat[3][2] = symmat_tmp[3][3];symmat[3][3] = symmat_tmp[3][2];
		}//else abc-- nothing to do
	}else if(b>a && b>c){//b is first
		symmat[1][1] = symmat_tmp[1][2];
		symmat[2][1] = symmat_tmp[2][2];
		symmat[3][1] = symmat_tmp[3][2];
		if(a>c){//bac
			evals[1]=b;evals[2]=a;//evals[3] =c won't change
			symmat[1][2] = symmat_tmp[1][1];
			symmat[2][2] = symmat_tmp[2][1];
			symmat[3][2] = symmat_tmp[3][1];
		}else{//bca
			evals[1]=b;evals[2]=c;evals[3]=a;

			symmat[1][3] = symmat_tmp[1][1];
			symmat[2][3] = symmat_tmp[2][1];
			symmat[3][3] = symmat_tmp[3][1];
			//231
			symmat[1][2] = symmat_tmp[1][3];
			symmat[2][2] = symmat_tmp[2][3];
			symmat[3][2] = symmat_tmp[3][3];

		}
	}else{//c is first
		symmat[1][1] = symmat_tmp[1][3];
		symmat[2][1] = symmat_tmp[2][3];
		symmat[3][1] = symmat_tmp[3][3];
		if(b>a){//cba
			evals[1] =c ;evals[3]= a;
			symmat[1][3] = symmat_tmp[1][1];
			symmat[2][3] = symmat_tmp[2][1];
			symmat[3][3] = symmat_tmp[3][1];
		}else{//cab
			evals[1] =c ;evals[2]= a; evals[3]=b;
			symmat[1][2] = symmat_tmp[1][1];
			symmat[2][2] = symmat_tmp[2][1];
			symmat[3][2] = symmat_tmp[3][1];

			symmat[1][3] = symmat_tmp[1][2];
			symmat[2][3] = symmat_tmp[2][2];
			symmat[3][3] = symmat_tmp[3][2];
		}

	}*/


	//cerr<<"evals new order: "<<evals[1]<<" "<<evals[2]<<" "<<evals[3]<<"\n";


	//rotate the neuron to the PCA.
	multiply(soma, symmat, avg, evals,zresolution);
	//cerr<<"avg same here: "<<avg[1]<<" "<<avg[2]<<" "<<avg[3]<<"\n";
	//move soma to 0,0,0
	if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0|| soma->getEnd()->getZ() != 0)
		translation(soma->getEnd()->getX(), soma->getEnd()->getY(),soma->getEnd()->getZ());


	//fill matrix with the newly rotated dendritic points.
	int refill = pcaDataPoints(soma, data, dendriteVal);

	//for (i = 1; i <= n; i++){
		//cerr<<data[i][3]<<"\n";
	//}
	//cerr<<"data refilled with:"<<refill<<" <="<<n<<"\n";
	//place back the Z values in the data array
	//cerr<<"placing back zcoords.."<<"\n";
	//int zcnt = 0;
	//if(zsize != -1)
	//	zcnt = setZcoords(false, zsize, soma, datacopy);
	//setZcoords(soma, datacopy);
	//cerr<<"replaced z vals:"<<zcnt<<"\n";

	if (true) {
		/**
		 * this part does the rotation for PCA using cosine similarity functions cursor and cursorAbs.
		 * cursor value includes the direction and magnitude information, whereas cursorAbs gives just the magnitude.
		 * this rotation also ensures that Y is oriented as height axis, X as width and Z as depth.
		 */
		Vector *e = new Vector();
		e->set(1, 0, 0);
		double xcur = cursor(e, soma,dendriteVal);
		e->set(0, 1, 0);
		double ycur = cursor(e, soma,dendriteVal);
		e->set(0, 0, 1);
		double zcur = cursor(e, soma,dendriteVal);
		e->set(1, 0, 0);
		double absX = cursorAbs(e, soma, dendriteVal);//passing the dendriteVal flag, to notify whether to consider dendrites only or not for rotation.
		e->set(0, 1, 0);
		double absY = cursorAbs(e, soma, dendriteVal);
		e->set(0, 0, 1);
		double absZ = cursorAbs(e, soma, dendriteVal);
		//compute sd for all three coordinates
		float * stddev = vector(m);
		stdDevAvg(data, n, m, avg, stddev);
		//cerr<<"avg tracking: "<<avg[1]<<" "<<avg[2]<<" "<<avg[3]<<"\n";
		double xd = stddev[1], yd = stddev[2], zd = stddev[3];
		//if all three components are close to each other, then the value will be equal to 6.
		double perc;
		double percthresh;
		if(zresolution==1){
			perc = absX / absY + absY / absX;
			percthresh = 2.1;
		}else{
			perc = absX / absY + absX / absZ + absY / absZ + absY / absX
							+ absZ / absX + absZ / absY;
			percthresh = 6.1;
		}
		cerr<<"perc:"<<perc<<"\n";
		//cerr<<"perc:"<<perc<<"\n";
		cerr<<"xd:"<<xd<<" yd:"<<yd<<" zd:"<<zd<<"\n";
		cerr<<"xcur:"<<xcur<<" ycur:"<<ycur<<" zcur:"<<zcur<<"\n";
		cerr<<"absX:"<<absX<<" absY:"<<absY<<" absZ:"<<absZ<<"\n";

		//rotate such that height is aligned along Y-axis

		if (perc > percthresh) {
			if (absX > absY && absX > absZ) {//if X is the first component, then rotate on Z such that X becomes Y. because we want the first component aligned along Y. the absX will have greater value than xcur.
				if (xcur > 0)
					rotate((double) 0, 0, -90);//turns x in to Y
				if (xcur < 0)
					rotate((double) 0, 0, 90);
			} else if (absY > absX && absY > absZ) {//if Y is the first component, then we are happy.
				if (ycur > 0)
					rotate((double) 0, 0, 0);//ok
				if (ycur < 0)
					rotate((double) 0, 0, 180);//Y is positive
			}else if (absZ > absX && absZ > absY) {//if Z is the first component, then rotate on X such that Z becomes Y.
				if (zcur > 0)
					rotate(90, 0, 0);//ok	//turns Z into Y
				if (zcur < 0)
					rotate(-90, 0, 0);//ok
			}
		} else {//if the cosine condition is too small, then compare with SD.
			//rotate depending on stddev
			if (xd > yd && xd > zd) {
				if (xcur > 0)
					rotate(0, 0, -90);
				if (xcur < 0)
					rotate(0, 0, 90);
			} else if (yd > xd && yd > zd) {
				if (ycur > 0)
					rotate(0, 0, 0);//ok
				if (ycur < 0)
					rotate(0, 0, 180);
			} else if (zd > xd && zd > yd) {
				if (zcur > 0)
					rotate(90, 0, 0);//ok
				if (zcur < 0)
					rotate(-90, 0, 0);//ok
			}

		}


/*
		//move soma to 0,0,0
		if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0
				|| soma->getEnd()->getZ() != 0)
			translation(soma->getEnd()->getX(), soma->getEnd()->getY(),
					soma->getEnd()->getZ());

		pcaRefill(soma, data);

		stdDev(data, n, m, stddev);

		e->set(1, 0, 0);
		xcur = cursor(e, soma);
		e->set(0, 1, 0);
		ycur = cursor(e, soma);
		e->set(0, 0, 1);
		zcur = cursor(e, soma);
		e->set(1, 0, 0);
		absX = cursorAbs(e, soma);
		e->set(0, 1, 0);
		absY = cursorAbs(e, soma);
		e->set(0, 0, 1);
		absZ = cursorAbs(e, soma);

		stddev = vector(m);
		stdDev(data, n, m, stddev);
		xd = stddev[1], yd = stddev[2], zd = stddev[3];

		perc = absX / absY + absX / absZ + absY / absZ + absY / absX + absZ
				/ absX + absZ / absY;

		if (ycur < 0) {

			rotate(180, 0, 0);
		}
*/
		//move soma to 0,0,0
		//if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0|| soma->getEnd()->getZ() != 0)
			//translation(soma->getEnd()->getX(), soma->getEnd()->getY(),soma->getEnd()->getZ());

		//to flip apical dendrites upright.
		//if (ycur < 0) {

			//rotate(180, 0, 0);
		//}
		/**
		 * this part compares sets second and third components along X and Z axes.
		 * refill matrix and compute std dev
		 */

		//move soma to 0,0,0
		if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0 || soma->getEnd()->getZ() != 0)
			translation(soma->getEnd()->getX(), soma->getEnd()->getY(), soma->getEnd()->getZ());
		refill = pcaDataPoints(soma, data,dendriteVal);//everytime the pca points are rotated, the data array needs to be refilled.
		//cerr<<"data refilled with:"<<refill<<" <="<<n<<"\n";
		//stdDevAvg(data, n, m, avg, stddev);

		if(zresolution != 1){
			e->set(1, 0, 0);
			xcur = cursor(e, soma,dendriteVal);
			e->set(0, 1, 0);
			ycur = cursor(e, soma,dendriteVal);
			e->set(0, 0, 1);
			zcur = cursor(e, soma,dendriteVal);
			e->set(1, 0, 0);
			absX = cursorAbs(e, soma, dendriteVal);
			e->set(0, 1, 0);
			absY = cursorAbs(e, soma, dendriteVal);
			e->set(0, 0, 1);
			absZ = cursorAbs(e, soma, dendriteVal);

			stddev = vector(m);
			stdDevAvg(data, n, m, avg, stddev);

			xd = stddev[1], yd = stddev[2], zd = stddev[3];

			perc = absX / absY + absX / absZ + absY / absZ + absY / absX + absZ
					/ absX + absZ / absY;

			//cerr<<"perc:"<<perc<<"\n";
			//cerr<<"xd:"<<xd<<" yd:"<<yd<<" zd:"<<zd<<"\n";
			//cerr<<"xcur:"<<xcur<<" ycur:"<<ycur<<" zcur:"<<zcur<<"\n";
			//cerr<<"absX:"<<absX<<" absY:"<<absY<<" absZ:"<<absZ<<"\n";

			if (perc > 7) {
				//if (zd > xd || absZ > absX) {
				if (absZ > absX) {//make sure that second component is X and third is Z.
					//cerr<<"don't be here if"<<absZ <<">"<<absX<<"\n";
					rotate((double) 0, -90, 0);//flipping z and x
				}
			} else {
				//absCursor too close to each other
				if (zd > xd) {
					//cerr<<"don't be here:"<<absZ <<">"<<absX<<"\n";
					rotate((double) 0, -90, 0);//flipping z and X
				}
			}

			/**
			 * rotate the axis that is closest to soma-COM vector as the 1st component and keep the second and third from PCA as it is
			 */


			if (soma->getEnd()->getX() != 0 || soma->getEnd()->getY() != 0 || soma->getEnd()->getZ() != 0)
				translation(soma->getEnd()->getX(), soma->getEnd()->getY(), soma->getEnd()->getZ());
			refill = pcaDataPoints(soma, data, dendriteVal);//ensure the dataarray is re-initialized with the last rotated values
			stddev = vector(m);
			//avg = vector(m);
			stdDevAvg(data, n, m, avg, stddev);
			xd = stddev[1], yd = stddev[2], zd = stddev[3];
		}
		//place back the Z values in the data array
		cerr<<"placing back zcoords.."<<"\n";
		int zcnt = 0;
		if(zsize != -1)
			zcnt = setZcoords(false, zsize, soma, datacopy);
		cerr<<"replaced z vals:"<<zcnt<<"\n";

		//cerr<<"DataPoints:"<<refill<<"\n";
		//cerr<<"avg at last: "<<avg[1]<<" "<<avg[2]<<" "<<avg[3]<<"\n";

	/* comment out asymmetry correction SP 12/10/2012
		Vector *com_v = new Vector();//center of mass vector
		com_v->set(avg[1],avg[2],avg[3]);
		Vector *orig_v = new Vector();//vector of origin (soma)
		orig_v->set(0,0,0);
		e->set(1,0,0);
		xcur = e->scalar(orig_v,e,com_v);//scalar product of X and COM
		e->set(0,1,0);
		ycur = e->scalar(orig_v,e,com_v);//scalar product of Y and COM
		e->set(0,0,1);
		zcur = e->scalar(orig_v,e,com_v);//scalar product of Z and COM

		absX = fabs(xcur);
		absY = fabs(ycur);
		absZ = fabs(zcur);

		//cerr<<"xd:"<<xd<<" yd:"<<yd<<" zd:"<<zd<<"\n";
		//cerr<<"xcur:"<<xcur<<" ycur:"<<ycur<<" zcur:"<<zcur<<"\n";
		//cerr<<"absX:"<<absX<<" absY:"<<absY<<" absZ:"<<absZ<<"\n";

		double dist_soma_com = soma->getEnd()->distance(com_v);
		double dist_com =  distFromPointsToCOM(soma,com_v,dendriteVal);
		double	avgdist_com = dist_com/n; //divide sum of distances from COM by #PCA rotated points.
		double dist_ratio = dist_soma_com/avgdist_com;
		//cerr<<dist_soma_com<<"/"<<avgdist_com<<" "<<dist_ratio<<"\n";

		if(dist_ratio > 0.34){//compare to the threshold to decide if further rotation is required or not
			if(absX>absY && absX>absZ){ //if X axis is the closest to the COM vector, it will have the highest value.
				if (xcur > 0)
					rotate((double) 0, 0, -90);//turns x in to Y
				else// xcur <= 0
					rotate((double) 0, 0, 90);

			}else if(absY>absX && absY>absZ){
				if(ycur < 0)
					rotate((double)0,0,180);//Y is made positive
			}
			else if(absZ>absY && absZ>absX){//if Z axis is the closest. then Z becomes Y, Y becomes X and X becomes Z.
				//cerr<<absX<<","<<absY<<","<<absZ<<"\n";
				if (zcur > 0){
					rotate(90, 0, 0);//turns Z into Y
					rotate(0, 90, 0);//turns Y into X
				}else{// zcur <= 0
					rotate(-90, 0, 0);
					rotate(0, 90, 0);
				}
			}
		}
		delete orig_v;
		delete com_v;
	*/


		/**
		 * after the rotation is performed as height = 1st, width = 2nd and depth = 3rd.
		 * we will check for the axis with the highest asymmetry and re-orient it along the Y axis.
		 */

		/*
		//re-calculate all before calculating symX,symY and symZ
		pcaRefill(soma, data);
		stdDev(data, n, m, stddev);

		e->set(1, 0, 0);
		xcur = cursor(e, soma,dendriteVal);
		e->set(0, 1, 0);
		ycur = cursor(e, soma,dendriteVal);
		e->set(0, 0, 1);
		zcur = cursor(e, soma,dendriteVal);
		e->set(1, 0, 0);
		absX = cursorAbs(e, soma, dendriteVal);//passing the dendriteVal flag, to notify whether to consider dendrites only or not for rotation.
		e->set(0, 1, 0);
		absY = cursorAbs(e, soma, dendriteVal);
		e->set(0, 0, 1);
		absZ = cursorAbs(e, soma, dendriteVal);


		//pcaRefill(soma, data);
		//stdDev(data, n, m, stddev);
		//xd = stddev[1], yd = stddev[2], zd = stddev[3];
		//symmetry check
		float symmetry_threshold1 = 0.75;
		float symmetry_threshold2 = 1.5;
		float symX =fabs(xcur)/absX;
		float symY = fabs(ycur)/absY;
		float symZ = fabs(zcur)/absZ;
		cerr<<"symX:"<<symX<<" symY:"<<symY<<" symZ:"<<symZ<<"\n";
		//cerr<<"symX/symY:"<<symX/symY<<" symX/symZ:"<<symX/symZ<<"\n";
		//cerr<<"symY/symX:"<<symY/symX<<" symY/symZ:"<<symY/symZ<<"\n";
		//cerr<<"symZ/symY:"<<symZ/symY<<" symZ/symX:"<<symZ/symX<<"\n";
		cerr<<"xd:"<<xd<<" yd:"<<yd<<" zd:"<<zd<<"\n";
		cerr<<"xcur:"<<xcur<<" ycur:"<<ycur<<" zcur:"<<zcur<<"\n";
		cerr<<"absX:"<<absX<<" abxY:"<<absY<<" absZ:"<<absZ<<"\n";

		//float zvalCheck = zd/max(xd,yd);
		float zvalCheck = absZ/absY;
		cerr<<"zvalCheck:"<<zvalCheck<<"\n";

		//checking for asymmetry around the principal axis, which is probable due to the soma center approach.
		if( symX > symmetry_threshold1 && symX/symY > symmetry_threshold2 && (symX/symZ > symmetry_threshold2  || zvalCheck < .25)){//larger asymmetry around X, make that that the height axis.
			if (xcur > 0)
				rotate((double) 0, 0, -90);//turns x in to Y
			if (xcur < 0)
				rotate((double) 0, 0, 90);
		}else if (symY > symmetry_threshold1 && symY/symX > symmetry_threshold2 && (symY/symZ > symmetry_threshold2  || zvalCheck < .25)){//larger asymmetry around Y, nothing changes, except when the points are on negative side, they are flipped to the postive.
			if (ycur > 0)
				rotate((double) 0, 0, 0);//ok
			if (ycur < 0)
				rotate((double) 0, 0, 180);//Y is positive
		}else if (zvalCheck > .25 && symZ > symmetry_threshold1 && symZ/symX > symmetry_threshold2  && symZ/symY > symmetry_threshold2 ){//larger asymmetry around Z, make that the height axis.
			if (zcur > 0)
				rotate(90, 0, 0);//ok	//turns Z into Y
			if (zcur < 0)
				rotate(-90, 0, 0);//ok
		}

		*/
		delete e;
		free_vector(stddev, m);
		free_vector(avg, m);
	}//end of rotation procedure

	//cerr<<"removing data"<<"\n";
	free_matrix(data, n, m);
	//cerr<<"removing symmat"<<"\n";
	free_matrix(symmat, m, m);
	//cerr<<"removing symmat2"<<"\n";
	free_matrix(symmat2, m, m);
	//cerr<<"removing datacopy"<<"\n";
	if(zresolution == 1)
		free_matrix(datacopy, n, 2);
	free_vector(evals, m);

	free_vector(interm, m);

}//pca()
;
/**
 * compute inter point distances. dir is the direction vector e.g., (0,0,1) for Z.
 */
double Neuron::zresolutionEstimator(Segment *t){
	static int zpropCtr = 0;
	if(t == soma)
		zpropCtr = 1;
	if (t->getPrev() != NULL) {
		double z1 = t->getEnd()->getZ();
		double z2 = t->getPrev()->getEnd()->getZ();
		if(z1==z2){
			zpropCtr++;
		}
	}
	if (t->getNext1() != NULL)
		zresolutionEstimator(t->getNext1());
	if (t->getNext2() != NULL)
		zresolutionEstimator(t->getNext2());
	return zpropCtr;
}
;

/**
 * compute the sum of distances from each data point to its Center of mass (COM).
 */
/* commented out this function as asymmetry adjustment is removed
double Neuron::distFromPointsToCOM(Segment *t, Vector *com, int dendriteOnly){
	static Vector * seg = new Vector();
		double dis = 0;
		if(dendriteOnly == 1){
			if (t->getPrev() != NULL && (t->getType()>2 && t->getType()<6)) {
				seg->setX(t->getEnd()->getX());
				seg->setY(t->getEnd()->getY());
				seg->setZ(t->getEnd()->getZ());
				dis = seg->distance(com);
			}
		}
		else {
			if (t->getPrev() != NULL && (t->getType() >= 2 && t->getType() < 6)) {
				seg->setX(t->getEnd()->getX());
				seg->setY(t->getEnd()->getY());
				seg->setZ(t->getEnd()->getZ());
				dis = seg->distance(com);
				//cerr<<"cos:"<<tmp<<":";
			}
		}
		if (t->getNext1() != NULL)
			dis += distFromPointsToCOM(t->getNext1(),com, dendriteOnly);
		if (t->getNext2() != NULL)
			dis += distFromPointsToCOM(t->getNext2(),com, dendriteOnly);
		return dis;
}
;
*/

/**
 * this method computes the sum of cosines for each datapoint.
 * a larger value suggests highest variance around that axis.
 */
double Neuron::cursor(Vector *v, Segment *t,int dendriteOnly) {
	static Vector * orig = new Vector(0, 0, 0);
	static Vector * seg = new Vector();
	double tmp = 0;

	if(dendriteOnly == 1){
		if (t->getPrev() != NULL && (t->getType()>2 && t->getType()<6)) {
			seg->setX(t->getEnd()->getX());
			seg->setY(t->getEnd()->getY());
			seg->setZ(t->getEnd()->getZ());
			//tmp = v->scalar(orig, v, seg);
			tmp = v->angle(orig, v, orig, seg);
			//if(isnan(tmp))
				//cerr<<"tmp:"<<tmp<<"point:"<<t->getId()<<" "<<t->getEnd()->getX()<<","<<t->getEnd()->getY()<<","<<t->getEnd()->getZ()<<"\n";
			tmp = cos(tmp * M_PI / 180);
		}
	}else {
		if (t->getPrev() != NULL && (t->getType() >= 2 && t->getType() < 6)) {
			seg->setX(t->getEnd()->getX());
			seg->setY(t->getEnd()->getY());
			seg->setZ(t->getEnd()->getZ());
			//tmp = v->scalar(orig, v, seg);
			tmp = v->angle(orig, v, orig, seg);
			tmp = cos(tmp * M_PI / 180);
		//cerr<<"cos:"<<tmp<<":";
		}
	}
	if (t->getNext1() != NULL)
		tmp += cursor(v, t->getNext1(),dendriteOnly);
	if (t->getNext2() != NULL)
		tmp += cursor(v, t->getNext2(),dendriteOnly);

	return tmp;
}
;
/**
 * this method is computing the sum of cosines w.r.t the given axis.
 * the axis that has the highest value determines the major component.
 */
double Neuron::cursorAbs(Vector *v, Segment *t, int dendriteOnly) {
	static Vector * orig = new Vector(0, 0, 0);
	static Vector * seg = new Vector();
	double tmp = 0;
	if(dendriteOnly == 1){
		if (t->getPrev() != NULL && (t->getType()>2 && t->getType()<6)) {
			seg->setX(t->getEnd()->getX());
			seg->setY(t->getEnd()->getY());
			seg->setZ(t->getEnd()->getZ());
			//tmp = fabs(v->scalar(orig, v, seg));
			tmp = v->angle(orig, v, orig, seg);
			tmp = fabs(cos(tmp * M_PI / 180));//closer the points are to vector v, the larger the value
		}
	}else{
		if (t->getPrev() != NULL && (t->getType() >= 2 && t->getType() < 6)) {
				seg->setX(t->getEnd()->getX());
				seg->setY(t->getEnd()->getY());
				seg->setZ(t->getEnd()->getZ());
				//tmp = fabs(v->scalar(orig, v, seg));
				tmp = v->angle(orig, v, orig, seg);
				tmp = fabs(cos(tmp * M_PI / 180));//closer the points are to vector v, the larger the value
			}
	}
	if (t->getNext1() != NULL)
		tmp += cursorAbs(v, t->getNext1(),dendriteOnly);
	if (t->getNext2() != NULL)
		tmp += cursorAbs(v, t->getNext2(),dendriteOnly);

	return tmp;
}
;

/**
 * stores the z-coordinate values along with compartment Ids in datcopy array
 * to fill the array, starting from soma, the neuron is traversed in next1 and next2 order.
 */
int Neuron::saveZcoords(bool flg, Segment *t, float **datcopy){
	static int id = 0;

	//checking the *t starts from the beginning of the tree.
	//if(t->getId() == 1 && flg == false && t->getPid() == -1){
	//if(t == soma && flg == false){
	if(t == soma){
		//cerr<<t->getId()<<" "<<t->getEnd()->getZ()<<"\n";
		id = 1;
		//flg = true;
	}
	//cerr<<"comparing.."<<t->getId()<<","<<datcopy[id][1]<<"\n";
	if(t->getPrev()!=NULL){// && flg == true
		//cerr<<t->getId()<<"--"<<id<<"\n";
		datcopy[id][1] = (float) t->getId();
		datcopy[id][2] = t->getEnd()->getZ();
		id++;
	}
	if (t->getNext1() != NULL)
		saveZcoords(flg, t->getNext1(), datcopy);
	if (t->getNext2() != NULL)
		saveZcoords(flg, t->getNext2(), datcopy);
	return id;
}
;

/**
 * this function replaces the z-coordinates with the dat array values
 * returns id, which represents the no.of values that were replaced from dat array
 */
int Neuron::setZcoords(bool flg, int ncompartments, Segment *t, float **dat){
	static int id = 1;
	//cerr<<"first ele in dat:"<<dat[1][1]<<"\n";
	//if(t->getId() == 1 && flg == false && t->getPid() == -1){// && t->getId() == dat[id][1]
	if(t == soma){ //  && flg == false
		flg = true;
		id = 1;
		//cerr<<"id"<<id;
		cerr<<"starting at "<<id <<" "<<t->getId()<<","<<dat[id][1]<<"\n";
	}
	if(t->getPrev()!=NULL && t->getId()==(int)dat[id][1] && id <= ncompartments){// && flg == true
		t->getEnd()->setZ(dat[id][2]);
		//cerr<<t->getPrev()->getId()<<" "<<id<<" "<<t->getId()<<" "<<t->getEnd()->getX()<<","<<t->getEnd()->getY()<<","<<t->getEnd()->getZ()<<"\n";
		id++;
	}else{
		//cerr<<"skipped.."<<t->getId()<<"\n";
	}

	if (t->getNext1() != NULL)
		setZcoords(flg, ncompartments, t->getNext1(), dat);
	if (t->getNext2() != NULL)
		setZcoords(flg, ncompartments, t->getNext2(), dat);
	return id;
}
;

void Neuron::multiply(Segment *t, float **mat, float *avg, float *evals, int zres) {
	float * interm = vector(3);
	float * data = vector(3);
	float * out = vector(3);
	static int i = 0;
	if (t == soma)
		i = 1;
	//testing nan values in Z
	//if(t->getId()==74 || t->getId() == 149)
		//cerr<<"z nan:"<<t->getEnd()->getX()<<","<<t->getEnd()->getY()<<","<<t->getEnd()->getZ();
	//if PCA is shifting around soma, then the avg array is zero. SP 07/10/12
	data[1] = t->getEnd()->getX() - avg[1];
	data[2] = t->getEnd()->getY() - avg[2];
	data[3] = t->getEnd()->getZ() - avg[3];



	/**
	 * rotating the data using the rotation matrix, which is mat
	 */
	int m = 3;
	//skip the z component, when low resolution
	if(zres == 1){
		m = 2;
		//make sure to initialize the third column of the out the out vector.
		out[3]=0;
	}

	for (int k = 1; k <= m; k++) {
		out[k] = 0;
		for (int k2 = 1; k2 <= m; k2++) {

			out[k] += data[k2] * mat[k2][m - k + 1];
			//cerr<<"out["<<k<<"]"<<out[k]<<"\n";
			if(isnan(out[k])){
				cerr<<t->getId()<<" out["<<k<<"]"<<out[k]<<" k2:"<<k2<<" data[k2]:"<<data[k2]<<" mat[k2][m - k + 1]:"<<mat[k2][m - k + 1]<<" m:"<<m<<"\n";
			}
		}
		/*
		if (0 == 1) {
			if (evals[m - k + 1] > 0.0005) // Guard against zero eigenvalue
				out[k] /= (float) sqrt(evals[m - k + 1]); // Rescale
			else
				out[k] = 0.0; // Standard kludge
		}*/
		if (fabs(out[k]) < 0.000001)
			out[k] = 0;
	}
	t->getEnd()->setX(out[1]);
	t->getEnd()->setY(out[2]);
	t->getEnd()->setZ(out[3]);
	if(isnan(out[3])){
		cerr<<t->getId()<<" "<<out[3]<<" m:"<<m<<"\n";
	}
	//cerr<<"data*mat "<<out[1]<<" "<<out[2]<<" "<<out[3]<<"\n";
	i++;

	if (t->getNext1() != NULL)
		multiply(t->getNext1(), mat, avg, evals, zres);
	if (t->getNext2() != NULL)
		multiply(t->getNext2(), mat, avg, evals, zres);

	free_vector(interm, 3);
	free_vector(data, 3);
	free_vector(out, 3);

}
;
/*
void Neuron::moveToMean(Segment * t) {
	static int x = 0, y = 0, z = 0, i = 0;
	if (t->getId() > 0) {
		x += (int) t->getEnd()->getX();
		y += (int) t->getEnd()->getY();
		z += (int) t->getEnd()->getZ();
		i++;
	}
	if (t->getNext1() != NULL)
		moveToMean(t->getNext1());
	if (t->getNext2() != NULL)
		moveToMean(t->getNext2());
	if (t == soma)
		translation(x / i, y / i, z / i);
}
*/
//
//int Neuron::pca(Segment * t,float **data){
//	static int id=0;
//	//Sg Changes added t->getType() ==1
//		//This condition is added to avoid virtual soma.
//		//Original soma has type =1 only.
//		if(t==soma && t->getType() ==1)
//
//			id=1;
//
//
//	if(t->getType()>2 && t->getType()<6){
//		double d;
//		d=1;
//		if(t->getPathDistance()>300){
//			d=0.001;
//		}
//
//		if(d==0) d=1;
//
//		//SG chnages, added the pre increment instead of post
//
//				id++;
//		data[id][1]=t->getEnd()->getX()/d;
//		data[id][2]=t->getEnd()->getY()/d;
//		data[id][3]=t->getEnd()->getZ()/d;
//
//	}
//	if(t->getNext1()!=NULL)
//		pca(t->getNext1(),data);
//	if(t->getNext2()!=NULL)
//		pca(t->getNext2(),data);
//
//	return id;
//
//}
//
//;

/**
 * this method only fills data array with X,Y,Z points
 */
int Neuron::pcaDataPoints(Segment * t, float **data, int dendriteOnly) {
	static int id = 0;
	if (t == soma)
		id = 1;

	//modified  >=2 to include the axon values too. This will ensure that all points are shifted around center of mass for PCA. SP 07/082012.
	if(dendriteOnly == 1){
		if (t->getPrev() != NULL && (t->getType()>2 && t->getType()<6)) {
			//double d;
			//d = 1;
			/*
				if (t->getPathDistance() > 300) {
				d = 0.001;
			}*/
			// removing multuplied by d (which is set to 0.001) to remove the weighting method to eliminate the outliers specifically for granule cells. But now, we are using this code generically. SP 07/11/2012.
			data[id][1] = t->getEnd()->getX();
			data[id][2] = t->getEnd()->getY();
			data[id][3] = t->getEnd()->getZ();
			//cerr<<data[id][1]<<"\t"<<data[id][2]<<"\t"<<data[id][3]<<"\n";
			id++;
		}
	}else{
		if (t->getPrev() != NULL && (t->getType() >= 2 && t->getType() < 6)) {

			data[id][1] = t->getEnd()->getX();
			data[id][2] = t->getEnd()->getY();
			data[id][3] = t->getEnd()->getZ();
			//cerr<<data[id][1]<<"\t"<<data[id][2]<<"\t"<<data[id][3]<<"\n";
			id++;
		}
	}

	if (t->getNext1() != NULL)
		pcaDataPoints(t->getNext1(), data,dendriteOnly);
	if (t->getNext2() != NULL)
		pcaDataPoints(t->getNext2(), data,dendriteOnly);

	return id;

}

;
/**
 * pcaRefill will load the data array with all points, including the axon.
 */
int Neuron::pcaRefill(Segment * t, float **data) {
	static int id = 0;
	if (t == soma)
		id = 1;

	if (t->getType() >= 2 && t->getType() < 6) {
		data[id][1] = t->getEnd()->getX();
		data[id][2] = t->getEnd()->getY();
		data[id][3] = t->getEnd()->getZ();
		//cerr<<data[id][1]<<"\t"<<data[id][2]<<"\t"<<data[id][3]<<"\n";
		id++;
	}
	if (t->getNext1() != NULL)
		pcaRefill(t->getNext1(), data);
	if (t->getNext2() != NULL)
		pcaRefill(t->getNext2(), data);

	return id;

}

;
/*
void Neuron::pcaStore(Segment * t, float **data) {
	//rotate and recenter all points
	static int id = 0;
	if (t == soma)
		id = 1;
	if (id % 610 == 0) {
		int hh = id;
	}
	if (t->getEnd() == NULL)
		return;

	t->getEnd()->setX(data[id][1]);
	t->getEnd()->setY(data[id][2]);
	t->getEnd()->setZ(data[id][3]);
	id++;
	if (t->getNext1() != NULL && t->getNext1()->getEnd() == NULL)
		t->setNext1(NULL);
	if (t->getNext1() != NULL)
		pcaStore(t->getNext1(), data);
	if (t->getNext2() != NULL)
		pcaStore(t->getNext2(), data);

}
;
*/

void stdDevAvg(float **data, int n, int m, float *avg, float *stddev)

{
	float eps = (float) 0.005;
	//float *mean;// *stddev;//, *vector();
	int j, i;

	/* Allocate storage for mean and std. dev. vectors */

	//avg = vector(m);

	/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		avg[j] = 0.0;
		for (i = 1; i <= n; i++) {
			avg[j] += data[i][j];
		}
		avg[j] /= (float) n;

	}

	for (j = 1; j <= m; j++) {
		stddev[j] = 0.0;
		for (i = 1; i <= n; i++) {
			stddev[j] += ((data[i][j] - avg[j]) * (data[i][j] - avg[j]));
		}
		stddev[j] /= (float) n;
		stddev[j] = (float) sqrt(stddev[j]);

		if (stddev[j] <= eps)
			stddev[j] = 1.0;
	}

	//free_vector(mean, m);
}
;

/**  Correlation matrix: creation  ***********************************/

void corcol(float **data, int n, int m, float ** symmat, float *avg)

{
	float eps = (float) 0.005;
	float x, *mean, *stddev;//, *vector();
	int i, j, j1, j2;

	/* Allocate storage for mean and std. dev. vectors */

	mean = vector(m);
	stddev = vector(m);

	/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		for (i = 1; i <= n; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= (float) n;
		avg[j] = mean[j];

	}

	/* Determine standard deviations of column vectors of data matrix. */
	stddev = vector(m);

	for (j = 1; j <= m; j++) {
		stddev[j] = 0.0;
		for (i = 1; i <= n; i++) {
			stddev[j] += ((data[i][j] - mean[j]) * (data[i][j] - mean[j]));
		}
		stddev[j] /= (float) n;
		stddev[j] = (float) sqrt(stddev[j]);

		if (stddev[j] <= eps)
			stddev[j] = 1.0;
	}

	/* Center and reduce the column vectors. */

	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			data[i][j] -= mean[j];
			x = (float) sqrt((float) n);
			x *= stddev[j];
			data[i][j] /= x;
		}
	}

	/* Calculate the m * m correlation matrix. */
	for (j1 = 1; j1 <= m - 1; j1++) {
		symmat[j1][j1] = 1.0;
		for (j2 = j1 + 1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += (data[i][j1] * data[i][j2]);
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}
	symmat[m][m] = 1.0;
	free_vector(mean, m);
	free_vector(stddev, m);

	return;

}

/** Standard Dev */
void stddev(float **data, float *std) {

}
/**  Variance-covariance matrix: creation  *****************************/

void covcol1(float **data, int n, int m, float ** symmat, float *avg, int somacenter)

{
	float *mean;//, *vector();
	int i, j, j1, j2;

	/* Allocate storage for mean vector */

	mean = vector(m);

	/* Determine mean of column vectors of input data matrix */

	for (j = 1; j <= m; j++) {
		mean[j] = 0.0;
		//caluclate the mean only for center around mass. SP 07/11/12
		if (somacenter!=1){
			for (i = 1; i <= n; i++) {
				mean[j] += data[i][j];
			}
			mean[j] /= (float) n;
		}
		//otherwise, mean is zero. SP 07/11/12
		avg[j] = mean[j];
		//cerr<<"average of "<<j<<" "<<avg[j]<<"\n";
	}

	/* Determine standard deviations of column vectors of data matrix. */
	float *stddev = vector(m);

	for (j = 1; j <= m; j++) {
		stddev[j] = 0.0;
		for (i = 1; i <= n; i++) {
			stddev[j] += ((data[i][j] - mean[j]) * (data[i][j] - mean[j]));
		}
		stddev[j] /= (float) n;
		stddev[j] = (float) sqrt(stddev[j]);

		if (stddev[j] <= 0.0001)
			stddev[j] = 1.0;
	}

	/* Center the column vectors. */

	for (i = 1; i <= n; i++) {
		for (j = 1; j <= m; j++) {
			data[i][j] -= mean[j];
		}
	}

	/* Calculate the m * m covariance matrix. */
	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += data[i][j1] * data[i][j2];
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}

	return;

}


/**  Sums-of-squares-and-cross-products matrix: creation  **************/
/*
void scpcol(float **data, int n, int m, float ** symmat)

{
	int i, j1, j2;

	// Calculate the m * m sums-of-squares-and-cross-products matrix.

	for (j1 = 1; j1 <= m; j1++) {
		for (j2 = j1; j2 <= m; j2++) {
			symmat[j1][j2] = 0.0;
			for (i = 1; i <= n; i++) {
				symmat[j1][j2] += data[i][j1] * data[i][j2];
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}

	return;

}
*/

/**  Error handler  **************************************************/

void erhand(char *err_msg)

/* Error handler */
{
	fprintf(stderr, "Run-time error:\n");
	fprintf(stderr, "%s\n", err_msg);
	fprintf(stderr, "Exiting to system.\n");
	exit(1);
}

/**  Allocation of vector storage  ***********************************/
/* Allocates a float vector with range [1..n]. */

float *vector(int n) {
	float *v;

	v = (float *) malloc((unsigned) n * sizeof(float));
	if (!v)
		erhand("Allocation failure in vector().");
	return v - 1;

}

/**  Allocation of float matrix storage  *****************************/
/* Allocate a float matrix with range [1..n][1..m]. */

float **matrix(int n, int m) {
	int i;
	float **mat;

	/* Allocate pointers to rows. */
	mat = (float **) malloc((unsigned) (n) * sizeof(float*));
	if (!mat)
		erhand("Allocation failure 1 in matrix().");
	mat -= 1;

	/* Allocate rows and set pointers to them. */
	for (i = 1; i <= n; i++) {
		mat[i] = (float *) malloc((unsigned) (m) * sizeof(float));
		if (!mat[i])
			erhand("Allocation failure 2 in matrix().");
		mat[i] -= 1;
	}

	/* Return pointer to array of pointers to rows. */
	return mat;

}

/**  Deallocate vector storage  *********************************/

void free_vector(float *v, int n)

/* Free a float vector allocated by vector(). */
{
	free((char*) (v + 1));
}

/**  Deallocate float matrix storage  ***************************/

void free_matrix(float **mat, int n, int m)

/* Free a float matrix allocated by matrix(). */
{
	int i;
	//cerr<<"starting from ";
	for (i = n; i >0 ; i--) {
		//cerr<< i<<"-"<< mat[i] + 1;
		free((char*) (mat[i] + 1));
		//free((mat[i] + 1));
	}
	//cerr<<"\n";
	free((char*) (mat + 1));
}

/**  Reduce a real, symmetric matrix to a symmetric, tridiag. matrix. */

void tred2(float **a, int n, float *d, float *e)

/* Householder reduction of matrix a to tridiagonal form.
 Algorithm: Martin et al., Num. Math. 11, 181-195, 1968.
 Ref: Smith et al., Matrix Eigensystem Routines -- EISPACK Guide
 Springer-Verlag, 1976, pp. 489-494.
 W H Press et al., Numerical Recipes in C, Cambridge U P,
 1988, pp. 373-374.  */
{
	int l, k, j, i;
	float scale, hh, h, g, f;

	for (i = n; i >= 2; i--) {
		l = i - 1;
		h = scale = 0.0;
		if (l > 1) {
			for (k = 1; k <= l; k++)
				scale += (float) fabs(a[i][k]);
			if (scale == 0.0)
				e[i] = a[i][l];
			else {
				for (k = 1; k <= l; k++) {
					a[i][k] /= scale;
					h += a[i][k] * a[i][k];
				}
				f = a[i][l];
				g = (float) (f > 0 ? -sqrt(h) : sqrt(h));
				e[i] = scale * g;
				h -= f * g;
				a[i][l] = f - g;
				f = 0.0;
				for (j = 1; j <= l; j++) {
					a[j][i] = a[i][j] / h;
					g = 0.0;
					for (k = 1; k <= j; k++)
						g += a[j][k] * a[i][k];
					for (k = j + 1; k <= l; k++)
						g += a[k][j] * a[i][k];
					e[j] = g / h;
					f += e[j] * a[i][j];
				}
				hh = f / (h + h);
				for (j = 1; j <= l; j++) {
					f = a[i][j];
					e[j] = g = e[j] - hh * f;
					for (k = 1; k <= j; k++)
						a[j][k] -= (f * e[k] + g * a[i][k]);
				}
			}
		} else
			e[i] = a[i][l];
		d[i] = h;
	}
	d[1] = 0.0;
	e[1] = 0.0;
	for (i = 1; i <= n; i++) {
		l = i - 1;
		if (d[i]) {
			for (j = 1; j <= l; j++) {
				g = 0.0;
				for (k = 1; k <= l; k++)
					g += a[i][k] * a[k][j];
				for (k = 1; k <= l; k++)
					a[k][j] -= g * a[k][i];
			}
		}
		d[i] = a[i][i];
		a[i][i] = 1.0;
		for (j = 1; j <= l; j++)
			a[j][i] = a[i][j] = 0.0;
	}
}

/**  Tridiagonal QL algorithm -- Implicit  **********************/

void tqli(float d[], float e[], int n, float **z)

{
	int m, l, iter, i, k;
	float s, r, p, g, f, dd, c, b;

	for (i = 2; i <= n; i++)
		e[i - 1] = e[i];
	e[n] = 0.0;
	for (l = 1; l <= n; l++) {
		iter = 0;
		do {
			for (m = l; m <= n - 1; m++) {
				dd = (float) (fabs(d[m]) + fabs(d[m + 1]));
				if (fabs(e[m]) + dd == dd)
					break;
			}
			if (m != l) {
				if (iter++ == 30)
					erhand("No convergence in TLQI.");
				g = (float) ((d[l + 1] - d[l]) / (2.0 * e[l]));
				r = (float) (sqrt((g * g) + 1.0));
				g = (float) (d[m] - d[l] + e[l] / (g + SIGN(r, g)));
				s = c = 1.0;
				p = 0.0;
				for (i = m - 1; i >= l; i--) {
					f = s * e[i];
					b = c * e[i];
					if (fabs(f) >= fabs(g)) {
						c = g / f;
						r = (float) sqrt((c * c) + 1.0);
						e[i + 1] = f * r;
						c = (float) (c * (s = (float) 1.0 / r));
					} else {
						s = f / g;
						r = (float) sqrt((s * s) + 1.0);
						e[i + 1] = g * r;
						s = (float) (s * (c = (float) 1.0 / r));
					}
					g = d[i + 1] - p;
					r = (float) ((d[i] - g) * s + 2.0 * c * b);
					p = s * r;
					d[i + 1] = g + p;
					g = c * r - b;
					for (k = 1; k <= n; k++) {
						f = z[k][i + 1];
						z[k][i + 1] = s * z[k][i] + c * f;
						z[k][i] = c * z[k][i] - s * f;
					}
				}
				d[l] = d[l] - p;
				e[l] = g;
				e[m] = 0.0;
			}
		} while (m != l);
	}
}
;

//modified this method to print the output into a sperate .wrl file instead of Lmout.txt. Sri 08/16/2010
void Neuron::printWRL() {
	int i = 0;
	char nm[300];
	strcpy(nm, name);
	strcat(nm, ".wrl");

	ofstream tfile(nm);//, ios::binary );

	cout << "\n";
	tfile << "#VRML V2.0 utf8\n";

	printWRL(soma, tfile);

}
;

void Neuron::printWRL(Segment *t, ofstream & out) {
	if (t->getId() > 0 && t->getPrev() != NULL) {
		//DIV transform um in mm
		static int RATIO = 2, DIV = 1000;
		double x, y, z, rad;
		out << "\n# Segment #:" << t->getId() << "\n";
		out << "Shape {\n";
		out << "  appearance Appearance {\n";
		out << "    material Material {\n";
		out << "      diffuseColor 1 1 1\n";
		out << "    } \n";// # end material\n";
		out << "  } \n";// # end appearance\n";

		out << "  geometry IndexedFaceSet {\n";

		out << "    solid TRUE\n";
		out << "    coord Coordinate {\n";
		out << "      point [\n";

		x = t->getPrev()->getEnd()->getX() / DIV;
		y = t->getPrev()->getEnd()->getY() / DIV;
		z = t->getPrev()->getEnd()->getZ() / DIV;
		rad = t->getRadius() / DIV;
		rad = 0.001;
		out << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";
		out << x + rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		out << x - rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		out << x - rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		x = t->getEnd()->getX() / DIV;
		y = t->getEnd()->getY() / DIV;
		z = t->getEnd()->getZ() / DIV;

		out << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";
		out << x + rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		out << x - rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		out << x - rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		out << "        ] \n";//  # end point\n";//
		out << "    } \n";// # end coord\n";
		out << "    coordIndex [ \n";

		out << "0, 4, 7, 3, -1,\n";
		out << "2, 3, 7, 6, -1,\n";
		out << "1, 2, 6, 5, -1,\n";
		out << "1, 0, 4, 5, -1,\n";

		out << "    ] \n";// # end coordIndex\n";

		out << "  } \n";// # end geometry\n";
		out << "} \n";// # end shape\n";
	}
	if (t->getNext1() != NULL) {
		printWRL(t->getNext1(), out);
	}
	if (t->getNext2() != NULL) {
		printWRL(t->getNext2(), out);
	}
}
;
void Neuron::printLineWRL() {

	cout << "#VRML V2.0 utf8\n";
	printLineWRL(soma);
	cout << "#";
}
;

void Neuron::printLineWRL(Segment *t) {
	if (t->getId() > 0 && t->getPrev() != NULL) {

		static int RATIO = 1, DIV = 1000;
		double x, y, z, rad;
		cout << "\n# Segment #:" << t->getId() << "\n";
		cout << "Shape {\n";
		cout << "  appearance Appearance {\n";
		cout << "    material Material {\n";
		cout << "      diffuseColor 1 1 1\n";
		cout << "    } \n";// # end material\n";
		cout << "  } \n";// # end appearance\n";

		cout << "  geometry IndexedLineSet  {\n";

		cout << "    solid TRUE\n";
		cout << "    coord Coordinate {\n";
		cout << "      point [\n";

		x = t->getPrev()->getEnd()->getX() / DIV;
		y = t->getPrev()->getEnd()->getY() / DIV;
		z = t->getPrev()->getEnd()->getZ() / DIV;
		rad = t->getRadius() / DIV;
		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		x = t->getEnd()->getX() / DIV;
		y = t->getEnd()->getY() / DIV;
		z = t->getEnd()->getZ() / DIV;

		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		cout << "        ] \n";//  # end point\n";//
		cout << "    } \n";// # end coord\n";
		cout << "    coordIndex [ \n";

		cout << "0, 1, -1,\n";

		cout << "    ] \n";// # end coordIndex\n";

		cout << "  } \n";// # end geometry\n";
		cout << "} \n";// # end shape\n";
	}
	if (t->getNext1() != NULL) {
		printLineWRL(t->getNext1());
	}
	if (t->getNext2() != NULL) {
		printLineWRL(t->getNext2());
	}
}
;
;
void Neuron::printFatherLineWRL() {

	cout << "#VRML V2.0 utf8\n";
	printFatherLineWRL(soma);
	cout << "#";
}
;

void Neuron::printFatherLineWRL(Segment *t) {
	if (t->getId() > 0 && t->getType() > 0 && t->getPrevFather() != NULL) {

		static int RATIO = 1, DIV = 1000;
		double x, y, z, rad;
		x = t->getPrevFather()->getEnd()->getX() / DIV;
		y = t->getPrevFather()->getEnd()->getY() / DIV;
		z = t->getPrevFather()->getEnd()->getZ() / DIV;
		cout << "\n# Segment #:" << t->getId() << "\n";
		cout << "Shape {\n";
		cout << "  appearance Appearance {\n";
		cout << "    material Material {\n";
		cout << "      diffuseColor 1 1 1\n";
		cout << "    } \n";// # end material\n";
		cout << "  } \n";// # end appearance\n";

		cout << "  geometry IndexedLineSet  {\n";

		cout << "    solid TRUE\n";
		cout << "    coord Coordinate {\n";
		cout << "      point [\n";

		rad = t->getRadius() / DIV;
		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		x = t->getEnd()->getX() / DIV;
		y = t->getEnd()->getY() / DIV;
		z = t->getEnd()->getZ() / DIV;

		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		cout << "        ] \n";//  # end point\n";//
		cout << "    } \n";// # end coord\n";
		cout << "    coordIndex [ \n";

		cout << "0, 1, -1,\n";

		cout << "    ] \n";// # end coordIndex\n";

		cout << "  } \n";// # end geometry\n";
		cout << "} \n";// # end shape\n";


	}
	if (t->getNextFather1() != NULL) {
		printFatherLineWRL(t->getNextFather1());
	}
	if (t->getNextFather2() != NULL) {
		printFatherLineWRL(t->getNextFather2());
	}
}
;

void Neuron::printFatherWRL() {

	cout << "#VRML V2.0 utf8\n";
	printFatherWRL(soma);
	cout << "#";
}
;

void Neuron::printFatherWRL(Segment *t) {
	if (t->getId() > 0 && t->getPrevFather() != NULL) {

		static int RATIO = 2, DIV = 1000, found = 0;
		double x, y, z, rad;
		x = t->getPrevFather()->getEnd()->getX() / DIV;
		y = t->getPrevFather()->getEnd()->getY() / DIV;
		z = t->getPrevFather()->getEnd()->getZ() / DIV;
		rad = t->getRadius() / DIV;

		cout << "\n# Segment #:" << t->getId() << "\n";
		cout << "Shape {\n";
		cout << "  appearance Appearance {\n";
		cout << "    material Material {\n";
		cout << "      diffuseColor 1 1 1\n";
		cout << "    } \n";// # end material\n";
		cout << "  } \n";// # end appearance\n";

		cout << "  geometry IndexedFaceSet {\n";

		cout << "    solid TRUE\n";
		cout << "    coord Coordinate {\n";
		cout << "      point [\n";

		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";
		cout << x + rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		x = t->getEnd()->getX() / DIV;
		y = t->getEnd()->getY() / DIV;
		z = t->getEnd()->getZ() / DIV;

		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";
		cout << x + rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		cout << "        ] \n";//  # end point\n";//
		cout << "    } \n";// # end coord\n";
		cout << "    coordIndex [ \n";

		cout << "0, 4, 7, 3, -1,\n";
		cout << "2, 3, 7, 6, -1,\n";
		cout << "1, 2, 6, 5, -1,\n";
		cout << "1, 0, 4, 5, -1,\n";

		cout << "    ] \n";// # end coordIndex\n";

		cout << "  } \n";// # end geometry\n";
		cout << "} \n";// # end shape\n";
	}
	if (t->getNextFather1() != NULL) {
		printFatherWRL(t->getNextFather1());
	}
	if (t->getNextFather2() != NULL) {
		printFatherWRL(t->getNextFather2());
	}
}
;

void Neuron::printFatherIV() {

	cout << "#Inventor V2.0 ascii\n";
	printFatherIV(soma);
	cout << "#";
}
;

void Neuron::printFatherIV(Segment *t) {
	if (t->getId() > 3 && t->getPrevFather() != NULL) {

		static int RATIO = 2, DIV = 1000;
		double x, y, z, rad;

		cout << "\n# Segment #:" << t->getId() << "\n";

		cout << "DEF object Separator {\n";
		cout << "Coordinate3 {\n";
		cout << "point  [\n";

		x = t->getPrevFather()->getEnd()->getX() / DIV;
		y = t->getPrevFather()->getEnd()->getY() / DIV;
		z = t->getPrevFather()->getEnd()->getZ() / DIV;
		rad = t->getRadius() / DIV;
		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";
		cout << x + rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";

		x = t->getEnd()->getX() / DIV;
		y = t->getEnd()->getY() / DIV;
		z = t->getEnd()->getZ() / DIV;

		cout << x + rad * RATIO << " " << y << " " << z + rad * RATIO << ",\n";
		cout << x + rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z - rad * RATIO << ",\n";
		cout << x - rad * RATIO << " " << y << " " << z + rad * RATIO << "\n";

		cout << "]}\n";
		cout << "IndexedFaceSet {\n";
		cout << "coordIndex [\n";

		cout << "0, 4, 7, 3, -1,\n";
		cout << "2, 3, 7, 6, -1,\n";
		cout << "1, 2, 6, 5, -1,\n";
		cout << "1, 0, 4, 5, -1\n";

		cout << "]\n";
		cout << "}\n";
		cout << "}\n";

	}
	if (t->getNextFather1() != NULL) {
		printFatherIV(t->getNextFather1());
	}
	if (t->getNextFather2() != NULL) {
		printFatherIV(t->getNextFather2());
	}
}
;

void Neuron::printCylWRL() {
	printCylWRL(soma);
	cout << "#";
	return;

	cout << "#VRML V2.0 utf8\n";
	cout << "WorldInfo\n";
	cout << "PROTO Arrow\n";
	cout << "[\n";
	cout << "exposedField SFVec3f From        0 0 0\n";
	cout << "    exposedField SFVec3f To          0 1 0\n";

	cout << "    exposedField SFFloat CapLength  .25\n";
	cout << "    exposedField SFFloat CapRadius  .08\n";
	cout << "    exposedField SFFloat LineRadius .028\n";

	cout
			<< "    exposedField SFString CapMode   \"keepRadius\" # or \"keepSize\" or \"keepAspect\"\n";

	cout << "    exposedField SFNode  appearance  NULL\n";
	cout << "]\n";
	cout << "{\n";
	cout << "    Group\n";
	cout << "    {\n";
	cout << "        children\n";
	cout << "        [\n";
	cout << "            DEF TransLine Transform\n";
	cout << "            {\n";
	cout << "                children DEF SwLine Switch\n";
	cout << "                {\n";
	cout << "                    choice Shape \n";
	cout << "                    {\n";
	cout << "                        appearance IS appearance\n";
	cout << "                        geometry Cylinder {}\n";
	cout << "                    }\n";
	cout << "                }\n";
	cout << "            }\n";
	cout << "            DEF TransCap Transform\n";
	cout << "            {\n";
	cout << "                children DEF SwCap Switch\n";
	cout << "                {\n";
	cout << "                    choice Shape \n";
	cout << "                    {\n";
	cout << "                        appearance IS appearance\n";
	cout << "                        geometry Cone {}\n";
	cout << "                    }\n";
	cout << "                }\n";
	cout << "            }\n";
	cout << "        ]\n";
	cout << "    }\n";
	cout << "    PROTO EFFS \n";
	cout << "    [\n";
	cout << "        exposedField SFVec3f From  0 0 0\n";
	cout << "        exposedField SFVec3f To    0 1 0 \n";
	cout << "        exposedField SFFloat CapLength  .25\n";
	cout << "        exposedField SFFloat CapRadius  .08\n";
	cout << "        exposedField SFFloat LineRadius .028\n";
	cout << "        exposedField SFString CapMode   \"keepRadius\"\n";
	cout << "    ]\n";
	cout << "      { Group {} }\n";
	cout << "\n";
	cout << "\n";
	cout << "    DEF EFFS EFFS \n";
	cout << "    {\n";
	cout << "        From IS From\n";
	cout << "        To   IS To\n";
	cout << "\n";
	cout << "        CapLength  IS CapLength\n";
	cout << "        CapRadius  IS CapRadius\n";
	cout << "        LineRadius IS LineRadius\n";
	cout << "        CapMode    IS CapMode\n";
	cout << "    }\n";
	cout << "\n";
	cout << "\n";
	cout << "\n";
	cout << "\n";
	cout << "    DEF Worker Script\n";
	cout << "    {\n";
	cout << "        field SFNode EFFS USE EFFS \n";
	cout << "\n";
	cout << "        field SFVec3f From 0 0 0\n";
	cout << "        field SFVec3f To   0 1 0\n";
	cout << "\n";
	cout << "        field SFFloat CapLength     .25\n";
	cout << "        field SFFloat CapRadius     .08\n";
	cout << "        field SFFloat LineRadius    .028\n";
	cout << "\n";
	cout << "        eventIn SFVec3f set_From\n";
	cout << "        eventIn SFVec3f set_To\n";
	cout << "\n";
	cout << "        eventIn SFFloat set_CapLength\n";
	cout << "        eventIn SFFloat set_CapRadius\n";
	cout << "        eventIn SFFloat set_LineRadius\n";
	cout << "\n";
	cout << "        eventIn SFString set_CapMode\n";
	cout << "        \n";
	cout << "\n";
	cout << "\n";
	cout << "        eventOut SFVec3f LineTranslation\n";
	cout << "        eventOut SFRotation LineRotation\n";
	cout << "        eventOut SFVec3f       LineScale \n";
	cout << "\n";
	cout << "        eventOut SFVec3f CapTranslation\n";
	cout << "        eventOut SFRotation CapRotation\n";
	cout << "        eventOut SFVec3f       CapScale \n";
	cout << "\n";
	cout << "        eventOut SFInt32  whichChoiceLine\n";
	cout << "        eventOut SFInt32  whichChoiceCap\n";
	cout << "\n";
	cout << "        field    SFBool  IsCortona        FALSE\n";
	cout << "\n";
	cout << "\n";
	cout << "        field    SFInt32 cKeepRadius 1\n";
	cout << "        field    SFInt32 cKeepSize   2 \n";
	cout << "        field    SFInt32 cKeepAspect 3 \n";
	cout << "\n";
	cout << "        field    SFInt32 CapMode     1\n";
	cout << "\n";
	cout << "\n";
	cout << "        url \"vrmlscript:\n";
	cout << "\n";
	cout << "        function initialize()\n";
	cout << "        {\n";
	cout
			<< "            IsCortona= Browser.getName() == 'Cortona VRML Client';\n";
	cout << "\n";
	cout << "            From= EFFS.From;\n";
	cout << "            To=   EFFS.To;\n";
	cout << "            CapLength=  EFFS.CapLength;\n";
	cout << "            CapRadius=  EFFS.CapRadius;\n";
	cout << "            LineRadius= EFFS.LineRadius;\n";
	cout << "            whichChoiceLine= -1;\n";
	cout << "            whichChoiceCap=  -1;\n";
	cout << "            eventsProcessed();\n";
	cout << "\n";
	cout << "            set_CapMode(EFFS.CapMode);\n";
	cout << "\n";
	cout << "        }\n";
	cout << "\n";
	cout << "        function set_From(f)       {From=       f; }\n";
	cout << "        function set_To(t)         {To=         t; }\n";
	cout << "        function set_CapLength(l)  {CapLength=  l; }\n";
	cout << "        function set_CapRadius(r)  {CapRadius=  r; }\n";
	cout << "        function set_LineRadius(r) {LineRadius= r; }\n";
	cout << "\n";
	cout << "        function set_CapMode(m)\n";
	cout << "        {\n";
	cout << "            if(m == 'keepRadius')  CapMode= cKeepRadius;\n";
	cout << "            if(m == 'keepSize')    CapMode= cKeepSize;\n";
	cout << "            if(m == 'keepAspect')  CapMode= cKeepAspect;\n";
	cout << "        }\n";
	cout << "\n";
	cout << "        function eventsProcessed()\n";
	cout << "        {\n";
	cout << "            var Middle= To.subtract(From).length() > CapLength\n";
	cout
			<< "                          ? From.add(To.subtract(From).normalize().multiply(To.subtract(From).length() - CapLength))\n";
	cout << "                          : From\n";
	cout << "                          ;\n";
	cout << "\n";
	cout << "            var LineLength= Middle.subtract(From).length();\n";
	cout << "\n";
	cout << "            if(   LineRadius != 0\n";
	cout << "               && LineLength != 0\n";
	cout << "              )\n";
	cout << "            {\n";
	cout << "                var a= From;\n";
	cout << "                var b= Middle;\n";
	cout << "                LineTranslation= a.add(b).multiply(0.5);\n";
	cout
			<< "                LineRotation=    new SFRotation(new SFVec3f(0, 1, 0), b.subtract(a));\n";
	cout
			<< "                LineScale=       new SFVec3f(LineRadius, LineLength / 2, LineRadius);\n";
	cout << "\n";
	cout << "                if(whichChoiceLine != 0)   whichChoiceLine= 0;\n";
	cout << "            }else{\n";
	cout << "                if(whichChoiceLine != -1)  whichChoiceLine= -1;\n";
	cout << "            }\n";
	cout << "            \n";
	cout << "//            var Cap_Length= To.subtract(Middle).length();\n";
	cout << "            \n";
	cout << "            if(   CapRadius  != 0 \n";
	cout << "               && CapLength  != 0\n";
	cout << "              )\n";
	cout << "            {\n";
	cout << "                a= Middle;\n";
	cout << "                b= To;\n";
	cout << "\n";
	cout
			<< "                var fraction= To.subtract(Middle).length() / CapLength;\n";
	cout << "\n";
	cout << "                CapTranslation= a.add(b).multiply(0.5);\n";
	cout
			<< "                CapRotation=    new SFRotation(new SFVec3f(0, 1, 0), b.subtract(a));\n";
	cout
			<< "                if(CapMode == cKeepSize)   CapScale= new SFVec3f(CapRadius, CapLength / 2, CapRadius);\n";
	cout
			<< "                if(CapMode == cKeepRadius) CapScale= new SFVec3f(CapRadius, CapLength * fraction / 2, CapRadius);\n";
	cout
			<< "                if(CapMode == cKeepAspect) CapScale= new SFVec3f(CapRadius * fraction, CapLength / 2 * fraction, CapRadius * fraction);\n";
	cout << "                if(whichChoiceCap != 0)   whichChoiceCap= 0;\n";
	cout << "            }else{\n";
	cout << "                if(whichChoiceCap != -1)  whichChoiceCap= -1;\n";
	cout << "            }\n";
	cout << "            \n";
	cout << "        }\n";
	cout << "\n";
	cout << "\n";
	cout << "        function MultiplyOri(a, b)\n";
	cout << "        {\n";
	cout << "            if(IsCortona)\n";
	cout << "        return b.multiply(a);\n";
	cout << "            else\n";
	cout << "        return a.multiply(b);\n";
	cout << "        }\n";
	cout << "\n";
	cout << "\n";
	cout << "        \"\n";
	cout << "    }\n";
	cout << "\n";
	cout << "    ROUTE EFFS.From TO Worker.set_From\n";
	cout << "    ROUTE EFFS.To   TO Worker.set_To\n";
	cout << "\n";
	cout << "    ROUTE EFFS.CapLength  TO Worker.set_CapLength\n";
	cout << "    ROUTE EFFS.CapRadius  TO Worker.set_CapRadius\n";
	cout << "    ROUTE EFFS.LineRadius TO Worker.set_LineRadius\n";
	cout << "\n";
	cout << "    ROUTE EFFS.CapMode    TO Worker.set_CapMode\n";
	cout << "\n";
	cout << "\n";
	cout << "    ROUTE Worker.LineTranslation TO TransLine.translation\n";
	cout << "    ROUTE Worker.LineRotation    TO TransLine.rotation\n";
	cout << "    ROUTE Worker.LineScale       TO TransLine.scale\n";
	cout << "    ROUTE Worker.whichChoiceLine TO SwLine.whichChoice\n";
	cout << "\n";
	cout << "    ROUTE Worker.CapTranslation  TO TransCap.translation\n";
	cout << "    ROUTE Worker.CapRotation     TO TransCap.rotation\n";
	cout << "    ROUTE Worker.CapScale        TO TransCap.scale\n";
	cout << "    ROUTE Worker.whichChoiceCap  TO SwCap.whichChoice\n";
	cout << "\n";
	cout << "}\n";

	printCylWRL(soma);
	cout << "#";
}
;
void Neuron::printCylWRL(Segment *t) {
	static int RATIO = 1000;
	if (t->getId() > 0 && t->getPrev() != NULL) {
		cout << "Arrow{\n";
		cout << " CapLength 0\n";
		cout << " LineRadius " << 2 * t->getRadius() / RATIO << "\n";
		cout << " To " << t->getEnd()->getX() / RATIO << " "
				<< t->getEnd()->getY() / RATIO << " " << t->getEnd()->getZ()
				/ RATIO << "\n";
		cout << " From " << t->getPrev()->getEnd()->getX() / RATIO << " "
				<< t->getPrev()->getEnd()->getY() / RATIO << " "
				<< t->getPrev()->getEnd()->getZ() / RATIO << "\n";
		cout << "} \n";

	}
	if (t->getNext1() != NULL) {
		printCylWRL(t->getNext1());
	}
	if (t->getNext2() != NULL) {
		printCylWRL(t->getNext2());
	}
}
;

void Neuron::maxDim(Segment *t) {
	static double x, y, z;
	x = t->getEnd()->getX();
	y = t->getEnd()->getY();
	z = t->getEnd()->getZ();
	if (t->getId() == 1) {
		minX = maxX = x;
		minY = maxY = y;
		minZ = maxZ = z;
	}
	if (x > maxX)
		maxX = x;
	if (x < minX)
		minX = x;

	if (y > maxY)
		maxY = y;
	if (y < minY)
		minY = y;

	if (z > maxZ)
		maxZ = z;
	if (z < minZ)
		minZ = z;
	if (t->getNext1() != NULL) {
		maxDim(t->getNext1());
	}
	if (t->getNext2() != NULL) {
		maxDim(t->getNext2());
	}

}
;
#define WIDTH 1000
#define HEIGTH 500 

void Neuron::printPGM() {

	float** data;//[800];//[600];

	data = matrix(WIDTH, HEIGTH);
	int i, j;

	for (i = 1; i <= WIDTH; i++)
		for (int j = 1; j <= HEIGTH; j++) {
			if (j % 100 == 0) {
				int kk = j;
			}
			data[i][j] = 9;
		}

	maxDim(soma);

	printPGM(soma, data);

	char nm[100];
	strcpy(nm, name);
	strcat(nm, ".pgm");
	ofstream tfile(nm, ios::binary);

	tfile << "P2\n# L-Measure\n" << (WIDTH) << " " << (HEIGTH) << "\n9\n";
	for (j = HEIGTH; j > 0; j--) {
		for (i = 1; i <= WIDTH; i++) {
			tfile << (int) data[i][j] << " ";
		}
		tfile << "\n";
	}
	tfile.close();

	free_matrix(data, WIDTH, HEIGTH);

}
;

void Neuron::printPGM(Segment *t, float** data) {
	static double x, y, x1, y1, m, b, ka = 0, kb = 0, kc = 0, kd = 0;
	static int i, j;
	static double partWIDTH = WIDTH / 3.0;
	if (t->getEnd() != NULL) {

		if (t->getId() == 893) {
			int ii = 0;
		}
		for (int axis = 0; axis < 3; axis++) {
			if (axis == 0) {
				x = t->getEnd()->getX();
				y = t->getEnd()->getY();
			} else if (axis == 1) {
				x = t->getEnd()->getZ();
				y = t->getEnd()->getY();
			} else if (axis == 2) {
				x = t->getEnd()->getX();
				y = t->getEnd()->getZ();
			}
			if (ka == 0 && kb == 0) {
				//compute all multipling factor 
				ka = (WIDTH / 3 - 1) / (maxX - minX);
				kb = (HEIGTH - 1) / (maxY - minY);
				kc = (WIDTH / 3 - 1) / (maxZ - minZ);
				kd = (HEIGTH - 1) / (maxZ - minZ);
				//choose the the min
				double mx = min(ka,kb);
				mx = min(mx,kc);
				mx = min(mx,kd);
				ka = mx;
			}

			i = (int) ((x - minX) * ka + 1);
			j = (int) ((y - minY) * ka + 1);
			i = (int) (i + (int) axis * partWIDTH);

			if (i > WIDTH)
				i = WIDTH;
			if (j > HEIGTH)
				j = HEIGTH;
			if (i < 1)
				i = 1;
			if (j < 1)
				j = 1;
			data[i][j] = 0;

			if (t->getNext1() != NULL) {
				if (axis == 0) {
					x1 = t->getNext1()->getEnd()->getX();
					y1 = t->getNext1()->getEnd()->getY();
				} else if (axis == 1) {
					x1 = t->getNext1()->getEnd()->getZ();
					y1 = t->getNext1()->getEnd()->getY();
				} else if (axis == 2) {
					x1 = t->getNext1()->getEnd()->getX();
					y1 = t->getNext1()->getEnd()->getZ();
				}

				m = (y1 - y) / (x1 - x);
				b = y1 - m * x1;
				int st = 10;
				double step = abs(x1 - x) / (st + 0.0);
				for (int h = 0; (h < st && m > 0); h++) {
					x += step;
					y = m * x + b;
					i = (int) ((x - minX) * ka + 1);
					j = (int) ((y - minY) * ka + 1);
					i = (int) (i + (int) axis * partWIDTH);
					if (i > WIDTH)
						i = WIDTH;
					if (j > HEIGTH)
						j = HEIGTH;
					if (i < 1)
						i = 1;
					if (j < 1)
						j = 1;
					data[i][j] = 0;
				}
			}
		}
	}
	if (t->getNext1() != NULL) {
		printPGM(t->getNext1(), data);
	}
	if (t->getNext2() != NULL) {
		printPGM(t->getNext2(), data);
	}
}

void Neuron::orientTrans1(Vector* Trans, Vector *Perp, Vector *centerMass) {
	rotateTo(Perp);

	double x, y, z;
	x = Trans->getX();
	y = Trans->getY();
	z = Trans->getZ();
	if (fabs(x) > 100) {
		int hh = 0;
	}

	//look for virtual segment with id=-99... to extract end point
	Vector * zv = lookNeuronForVirtual(soma);
	double angle = 0;
	if (zv != NULL) {
		Vector * cc = new Vector(0, 0, 0);

		angle = cc->anglePlane(zv, centerMass, Perp);

		delete cc;

	}
	if (angle != 0) {
		rotateAxis(Perp, -angle * M_PI / 180, soma);
	}

	x *= 1000;
	y *= 1000;
	z *= 1000;
	translation(-x, -y, -z);
}
;

Vector * Neuron::lookNeuronForVirtual(Segment *t) {
	static int found = 0;
	Vector *x1, *x2;
	x1 = NULL;
	x2 = NULL;
	if (t->getType() == -99) {
		found = 1;
		return t->getEnd();
	}
	if (t->getNext1() != NULL && found == 0) {
		x1 = lookNeuronForVirtual(t->getNext1());
	}
	if (t->getNext2() != NULL && found == 0) {
		x2 = lookNeuronForVirtual(t->getNext2());
	}
	if (x1 != NULL)
		return x1;
	if (x2 != NULL)
		return x2;
	return NULL;
}
;

Neuron::~Neuron() {
	int rem = 0;
	rem = remove(soma);
	cerr<<"removed all "<<rem <<" segs";
	//checking to make sure if all the segs are removed!
	/*
	if(soma!=NULL){
		cerr<<"soma "<<soma->getEnd()->getX()<<"\n";
		//rem = remove(soma);
		//cerr<<"removed all 2"<<rem <<" segs";
	}*/
	name = NULL;
	type = NULL;
	in = NULL;
	soma = NULL;

}
;

//SP made changes 10/09/12 to ensure that all nodes are deleted and memory is freed.
int Neuron::remove(Segment * t) {
	int removedNodes = 0;
	if(t == NULL) return 0;

	//if node is a tip, then remove
	if(t->getNext1() == NULL && t->getNext2() == NULL){
		//cerr<<"removing 1:"<<"id:"<<t->getId()<<"X:"<<t->getEnd()->getX()<<"type:"<<t->getType()<<"\n";
		delete (t);
		t = NULL;
		return 1;
	}
	/*
	if(t->isTerminate()){
		cerr<<"removing 2:"<<"id:"<<t->getId()<<"X:"<<t->getEnd()->getX()<<"type:"<<t->getType()<<"\n";
		delete (t);
		t = NULL;
		return 1;
	}*/
	//check left node
	if(t->getNext1() != NULL && t->getNext2() == NULL)
		removedNodes += remove(t->getNext1());
	//check right node
	else if(t->getNext1() == NULL && t->getNext2()!= NULL)
		removedNodes += remove(t->getNext2());
	else
		removedNodes += remove(t->getNext1()) + remove(t->getNext2());

	//delete node
	//cerr<<"removing:"<<"id:"<<t->getId()<<"X:"<<t->getEnd()->getX()<<"type:"<<t->getType()<<"\n";
	delete (t);
	t = NULL;
	return removedNodes+1;


	/*old code
	if (t->getNext1() != NULL) {
		remove(t->getNext1(),ctr);

	}

	if (t->getNext2() != NULL && t->getNext1() != t->getNext2()) {
		remove(t->getNext2(),ctr);

	}
	delete (t);
	t = NULL;
	ctr++;
	return ctr;*/
}
;

Neuron * Neuron::clone() {
	Neuron *n;
	//create empty neuron;
	n = new Neuron();
	n->setCloned();
	n->setName(name);
	//fill with segments
	Segment *s = clone(n, soma);
	n->setSoma(s);
	n->init();
	s = NULL;
	return n;
}
;

Segment * Neuron::clone(Neuron * n, Segment * t) {
	Segment * newSeg, *t1, *t2;
	t1 = NULL;
	t2 = NULL;

	if (t->getNext1() != NULL) {
		t1 = clone(n, t->getNext1());
	}
	if (t->getNext2() != NULL) {
		t2 = clone(n, t->getNext2());
	}
	newSeg = t->clone();

	if (t1 != NULL) {
		newSeg->setNext1(t1);
		t1->setPrev(newSeg);
	}
	if (t2 != NULL) {
		newSeg->setNext2(t2);
		t2->setPrev(newSeg);
	}
	t1 = NULL;
	t2 = NULL;
	return newSeg;
}
;

//SG 03/31/2011
//This function truncates the values upto 2 significant digits after decimal
double Neuron::round2Digits(double num) {

	double toRet = ((double) ((int) (num * 100))) / 100;

	return toRet;
}
;
//End of SG chnages

//SG July 10 2011
//This function will return the file name from the path.
//Needed for the online LM

char * Neuron::getFileName(char * filePath) {
	int i = 0;
	int pos = 0;
	int j = 0;
	int needCov = -1;
	char * fileName;

	while (filePath[i] != '\0') {

		if (filePath[i] == '\\') {
			pos = i;
		} else if (filePath[i] == '/') {
			pos = i;
		} else if (filePath[i] == '$') {
			needCov = 1;
		}
		i++;
	}

	if (needCov == -1) {

		return filePath;
	}

	fileName = new char[i - pos - 2];
	i = pos;

	while (filePath[i + 1] != '\0') {

		if (filePath[i + 1] == '#') {
			fileName[i - pos] = ' ';

		} else if (filePath[i + 1] == '%') {
			fileName[i - pos] = '/';
		} else if (filePath[i + 1] == '$') {
			fileName[i - pos] = ':';
		} else {
			fileName[i - pos] = filePath[i + 1];
		}
		i++;

	}

	fileName[i - pos] = '\0';

	return fileName;
}
;

