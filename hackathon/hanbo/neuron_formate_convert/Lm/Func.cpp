#include "Elaboration.h"
#include "Segment.h"
#include "Neuron.h"
#include "Func.h"
//#include "stdafx.h"
using namespace std;
;

Func *  
Dim::getPreFunc(){ return this;}

int 
Dim::countSegment(Segment *t)
{
	//  int tmp=1;
	//  if(t->getNext1()!=NULL)
	//    tmp= tmp+countSegment(t->getNext1());
	//  if(t->getNext2()!=NULL)
	//   tmp+=countSegment(t->getNext2());
	//      return tmp;
	//cerr<<"id:"<<t->getId()<<"X:"<<t->getEnd()->getX()<<"type:"<<t->getType()<<"\n";
	if(t->getNext1() == NULL && t->getNext2() == NULL){
		//cerr<<"id:"<<t->getId()<<"X:"<<t->getEnd()->getX()<<"type:"<<t->getType()<<"\n";
		return 1;
	}

	if(t->getNext1() != NULL && t->getNext2() == NULL)
		return countSegment(t->getNext1())+1;
	else if(t->getNext1() == NULL && t->getNext2()!= NULL)
		return countSegment(t->getNext2())+1;
	else
		return ( countSegment(t->getNext1()) +  countSegment(t->getNext2()))+1;

}


void Dim::fill_new(Segment* s, float **data) {

	idx=idy=idz=1;

	Segment* current = s;
	Segment* prev_segment = NULL;

	//Recursion ID , for marking the segemnts. The segments are marked with ID if they are visited.
	//This helps in forward and backward traversal across segments.

	int avoidRecursionMethodId;

//	      if(s->getNext1()->avoidRecursiveId > 1000 || s->getNext1()->avoidRecursiveId < 1)
//	    	   avoidRecursionMethodId = s->getNext1()->avoidRecursiveId+1;
//	       else
	//SP: increase the range of random#s generation, to avoid runnin into segmentation faults when running large # files at one 10/11/12
	avoidRecursionMethodId = rand() % 10000;
	//to avoid the possibility of matching random #s even before traversing, esp. when executed in batch mode- sri 10/07/12
	if(avoidRecursionMethodId == current->avoidRecursiveId){ //re-generate random number
		cerr<<"checking random number generation:"<<avoidRecursionMethodId <<"~ "<<current->avoidRecursiveId<<"\t";
		avoidRecursionMethodId = rand() % 100000;
	}
	bool next1Covered, next2Covered;
	int ctr2=0;
	int ctr4=0;

	//Continious loop
	while(true){


		//Check if the node is traversed, and if not
		if(current->avoidRecursiveId != avoidRecursionMethodId){
			//The core functionality of find dupilcate w/o recursion
			//findDuplicate1(current);
			//if(current->getType() == 2)
				//cerr<<"stop here";
			if(current->getType() > 0){
				fill(current, data);
				//cerr<<"width:"<<idx<<"height:"<<idy<<"depth:"<<idz<<"\n";
			}

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

			if(current == NULL){
				//cerr<<"current is null"<<current->getType()<<","<<current->getEnd()->getX()<<","<<current->getEnd()->getY()<<","<<current->getEnd()->getZ();
				break;
			}

			//If back to the first segment then break
			if(current->getId() == 1 && current->getType()!= -1 && current->getNext2() == NULL){
				//cerr<<"current id is 1 and next2() is null"<<current->getType()<<","<<current->getEnd()->getX()<<","<<current->getEnd()->getY()<<","<<current->getEnd()->getZ();
				break;
			}
			else if(current->getId() == 1 && current->getType()!= -1 && current->getNext2()->avoidRecursiveId == avoidRecursionMethodId){
				cerr<<"current id is 1 and next2() is traversed"<<current->getType()<<","<<current->getEnd()->getX()<<","<<current->getEnd()->getY()<<","<<current->getEnd()->getZ();
				break;
			}
		}


	}

}




void Dim::fill(Segment * t, float **data){

	// static int idx=1,idy=1,idz=1;
	//all three idx,idy,idz variables should be intialized to 1. Otherwise, the data array is not populated for all the x or y or z values. sri 05/10/2010.
	//  if(t->getId()==1){
	//    idx=1;idy=1;idz=1;
	//  }
	//cerr<<lc++<<",";
	//cerr<<limit->computeStep(t)<<t->getType();
	//cerr<<t->getId()<<","<<t->getType()<<","<<t->getEnd()->getX()<<","<<t->getEnd()->getY()<<","<<t->getEnd()->getZ()<<","<<t->getPid()<<"\n";
	if(limit==0 || limit->computeStep(t)!=0 )
	{

		if(x==1){
			// if (t->getPrev() == NULL && t->getPrev2()== NULL)
			//idx =1;

			data[idx][2]=t->getLength();
			data[idx][1]=t->getEnd()->getX();
			//cerr<<idx<<":"<<data[idx][1]<<","<<data[idx][2]<<"\n";
			idx++;


		} else if(y==1){
			//if (t->getPrev() == NULL && t->getPrev2()== NULL)
			//idy =1;
			data[idy][2]=t->getLength();
			data[idy][1]=t->getEnd()->getY();
			//cerr<<idy<<":"<<data[idy][1]<<","<<data[idy][2]<<"\n";
			idy++;
		}else {
			// if (t->getPrev() == NULL && t->getPrev2()== NULL)
			//idz =1;
			data[idz][2]=t->getLength();
			data[idz][1]=((float) ((int) (t->getEnd()->getZ() * 100))) / 100;
			//cerr<<idz<<":"<<data[idz][1]<<","<<data[idz][2]<<"\n";
			idz++;
		}



	}
	//  if(t->getNext1()!=NULL)
	//    fill(t->getNext1(), data);
	//  if(t->getNext2()!=NULL)
	//    fill(t->getNext2(), data);
	//

}



double  
Dim::computeStep(Segment *t){ 
	float** data;
	float** ArrayOf95Perc;
	static bool first=false;
	static bool xflg=false;
	static bool yflg=false;
	static bool zflg=false;
	static int id1=0;
	static double correctOutput;
	static double correctOutputx = -1;
	static double correctOutputy = -1;
	static double correctOutputz = -1;

	int i;

	//Not calculating for virtual segemnts
	if(t->getType() == -1) return VOID;

	//defined a new variable to keep track of new neurons in case of multiple files. sri 05/10/2010
	static const char* nameorig = t->getNeuronName();

	const char* nametmp = t->getNeuronName();

	//whenever the neuron changes all the flags for computing fill are reset. For each neuron and each function only once fill is computed. sri 05/10/2010.
	if(strncmp(nameorig,nametmp,strlen(nameorig))!=0){
		nameorig = t->getNeuronName();
		xflg = false;yflg = false; zflg = false;
	}
	//new neuron is being processed
	if(strncmp(nameorig,nametmp,strlen(nameorig))==0 && (( x==1 && xflg == false)  || (y==1 && yflg == false) || (z==1 && zflg == false))){
		//set pointer to the root node of the tree.
		while(t->getPid()!= -1){ //getId()!=1){
			t = t->getPrev();
		}


		id1=t->getId();


		if(x==1){
			setName("Width");
			//cerr<<"Width";
			xflg=true;
		} else if(y==1){
			setName("Height");
			//cerr<<"Height";
			yflg=true;
		}else if(z==1){
			zflg=true;
			//cerr<<"Depth";
			setName("Depth");
		}

		//count segment
		int seg=0;
		seg=countSegment(t);
		//cerr<<"total seg:"<<seg<<"\n";
		data=matrix(seg,2);
		float initval = 0.00;
		for(i=1;i<=seg;i++)
			for(int j=1;j<=2;j++){
				data[i][j]=initval;
			}
		//if(t->getPid()== -1)
			//cerr<<"t with id = "<<t->getId()<<" "<<t->getType()<<" "<<t->getEnd()->getX()<<","<<t->getEnd()->getY()<<","<<t->getEnd()->getZ()<<"\n";
		//fills the data array with X or Y or Z coordinates
		fill_new(t,data);

		//sort by position of X/Y/Z in ascending order
		Quicksort(data,1,seg);

		//get the smallest element to the array to transform all points positive
		float bias=data[1][1];
		//cerr<<"data values*******"<<data[1][1]<<"\n";

		for( i=1;i<=seg;i++){

			//subtracting the min value from all the values
			data[i][1]-=bias;
			//cerr<<data[i][1]<<",";
		}
		//cerr<<"changed data array for "<<i<<"# segments"<<"\n";
		//cerr<<"\n";
		//for( i=1;i<=seg;i++){
			//cerr<<data[i][2]<<",";
		//}
		//cerr<<"\n";

		//get total length
		double tot=0;

		for( i=1;i<=seg;i++){
			tot+=data[i][2];
		}
		//cerr<<"total length "<<tot<<" ";
		//get 5% of total length
		percent = 0.05;
		tot*=percent;
		tot= ((double) ((int) (tot * 100))) / 100;

		//determine the max size of ArrayOf95Perc that can hold 5% of the total length.
		int percInd=-1;
		int minpercInd = -1;
		double val = 0;
		//double coord_val = 0;
		for( percInd=1;percInd<=seg;percInd++){
			val+=data[percInd][2];
			if(floor(val)>tot){
				//cerr<<"val:"<<val;
				break;
			}
		}
		//cerr<<"size of 95% array:"<<percInd<<"\n";

		/**
		 * compute the minimum width that entails 95% of length
		 * try all combinations to truncate 5% from either ends
		 */

		float firstPos = initval;
		float secondPos =initval;
		float smallestVal = initval;
		float truncate = 0.00;
		float left_trunc_val=0.00;

		//set ArrayOf95Perc size to percInd+1
		//percInd +=1;
		//cerr<<"size of ArrayOf95Perc is initialized to:"<<percInd<<"\n";
		ArrayOf95Perc = matrix(percInd,1);
		//initialize ArrayOf95Perc
		for( i=1;i<=percInd;i++){
			ArrayOf95Perc[i][1]= initval;
		}


		//cerr<<"95% of length "<<tot;
		for(int lind=1;floor(left_trunc_val)<=tot;lind++){//loop for summing up lengths from left side
			//add up the lengths of each compartment cumulatively
			for(int left_cum_trunc_ind=1;left_cum_trunc_ind<=lind;left_cum_trunc_ind++){
				left_trunc_val += data[lind][2];
				truncate = left_trunc_val;
			}
			//get the current X/Y/Z position
			firstPos = data[lind][1];
			//fill up truncate upto 5% of length from the other side
			for(int rind=seg;floor(truncate)<=tot;rind--){
				truncate += data[rind][2];
				//get the corresponding X position from the other end
				secondPos = data[rind][1];
			}
			if(secondPos==initval)//if the tot value is cumulatively reached by left hand side itself, then, set secondPos to the farthest end on the right.
				secondPos = data[seg][1];
			//store the width values

			ArrayOf95Perc[lind][1]=fabs(firstPos-secondPos);//store all possible values
			//cerr<<"compare:"<<firstPos<<","<<secondPos<<"--"<<truncate;
			truncate = 0; //reset for next possible width
			firstPos = initval;//reset every time before calculating the new width
			secondPos = initval;
			//cerr<<"#width values:"<<lind<<","<<ArrayOf95Perc[lind][1]<<"\n";
			minpercInd = lind;
		}
		//cerr<<"minpercInd:"<<minpercInd<<"\n";
		//if(minpercInd>-1)//mark end of width values
			//ArrayOf95Perc[minpercInd][1] = -99999999;

		float tmp;
		//sort ArrayOf95Perc in ascending order
		if(minpercInd>-1){//mark index at the end of width values in the array
			for (int i1=1; i1<minpercInd; i1++) {
				for (int j1=i1; j1<i1+1; j1++){
					//cerr<<"before swap:"<<ArrayOf95Perc[j1+1][1]<<","<<ArrayOf95Perc[j1][1]<<"\n";
					if (ArrayOf95Perc[j1+1][1] < ArrayOf95Perc[i1][1]) { // compare the two neighbors
						tmp = ArrayOf95Perc[i1][1]; // swap a[i] and a[j+1]
						ArrayOf95Perc[i1][1] = ArrayOf95Perc[j1+1][1];
						ArrayOf95Perc[j1+1][1] = tmp;
					}
				}
			}
		}


		//cerr<<"printing all possible values:"<<minpercInd<<"\n";
		//print ArrayOf95Perc
		/*
		for(int i2=1;i2<minpercInd;i2++){
			cerr<<ArrayOf95Perc[i2][1]<<",";
		}*/
		//cerr<<"\n";
		smallestVal = ArrayOf95Perc[1][1];
		cerr<<":"<<smallestVal<<"\n";
		//free memory from data and ArrayOf95Perc, after processing every neuron
		free_matrix(data,seg,2);
		free_matrix(ArrayOf95Perc,percInd,2);

		/*
		for (int i2=1; i2<minpercInd && smallestVal<=0; i2++) {
			smallestVal = ArrayOf95Perc[i2][1];
		}*/

		//if(smallestVal == initval)
			//smallestVal = 0.00;
		//cerr<<"smallest:"<<smallestVal<<"\n";
		/*
		//cerr<<"2.5% is"<<tot<<"\n";
		//to remove 2.5% of data from leftmost end
		double tot_left=0;
		for( i=1;(i<=seg && tot>tot_left);i++){
			tot_left+=data[i][2];
			//cerr<<i<<"->"<<tot_left<<" "<<data[i][1]<<" "<<data[i][2]<<"\n";
		}

		if(i>seg) i=seg;

		double min=data[i][1];
		//cerr<<"min:"<<min<<"\n";
		//to remove 2.5% of data from rightmost end
		double tot_right= 0;
		for(i=seg;(i>=1 && tot>tot_right);i--){
			tot_right+=data[i][2];
			//cerr<<i<<"->"<<tot_right<<" "<<data[i][1]<<" "<<data[i][2]<<"\n";
		}
		if(i<1) i=1;
		double max=data[i][1];
		//cerr<<"max:"<<max<<"\n";
		free_matrix(data,seg,2);
		correctOutput=max-min;
		*/
		if(x==1){
			//correctOutputx=correctOutput;
			correctOutputx = smallestVal;
			return correctOutput;
		} else if(y==1){
			//correctOutputy=correctOutput;
			correctOutputy = smallestVal;
			return correctOutput;
		}else if(z==1){
			//correctOutputz=correctOutput;
			correctOutputz = smallestVal;
			return correctOutput;
		}
		return correctOutput;

	}
	else{
		if(x==1){
			return correctOutputx;
		} else if(y==1){
			return correctOutputy;
		}else if(z==1){
			return correctOutputz;
		}


	}
	return VOID;
}


void 
Dim::SelectionSort(float** data, int left, int right)
{
	for(int i = left; i < right; i++) {
		int min = i;
		for(int j=i+1; j <= right; j++)
			if(data[j][1] < data[min][1]) min = j;
		swap(min,i,data);

	}
}



int 
Dim::Partition( float ** d, int left, int right)
{
	//sorting by the position val
	float val =d[left][1];
	int lm = left-1;
	int rm = right+1;
	for(;;) {
		do
			rm--;

		while (d[rm][1] > val);
		do
			lm++;
		while( d[lm][1] < val);

		if(lm < rm) {
			swap(rm,lm,d);

		}
		else
			return rm;
	}
}


void 
Dim::Quicksort( float** d, int left, int right)
{

	if(left < (right- 15)) {
		int split_pt = Partition(d,left, right);
		Quicksort(d, left, split_pt);
		Quicksort(d, split_pt+1, right);
	}
	else SelectionSort(d, left, right);
};




void 
Dim::swap(int min, int i,float **data)
{
	//switch part 1
	float temp = data[min][1];
	data[min][1] = data[i][1];
	data[i][1] = temp;
	//switch part 2
	temp = data[min][2];
	data[min][2] = data[i][2];
	data[i][2] = temp;

}
