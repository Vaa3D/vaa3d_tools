#include "Elaboration.h"
#include "Segment.h"
#include "Neuron.h"

//to look for erroneous characters and report as file error in the output
bool ferr=false;
ListElaboration::ListElaboration(){
	
	index=0;
	tmp=NULL;
}
void ListElaboration::initList(){
}
//Add a new Elaboration in the list
void ListElaboration::add(Elaboration *f){
	tmp=f;
	
	list[index]=f;

	//check if a spot is available in the list
	if(index==MAX_Elaboration){ 
		cerr<<"No more empty Elaboration slots available\n";
		return;
	}else {
		list[index]=f;
		index++;
	}
}

void ListElaboration::preCompute(Neuron* n){

	preComputeStep(n->getSoma());
	ferr = n->ferror;

	// starting from the soma compute a single step at a time
	for(int i=0;i<index;i++){
		list[i]->initialize();
	
}
}
void  ListElaboration::preComputeStep(Segment* t){
	for(int i=0;i<index;i++){
		//skip duplicate somatic segment
		if(t->getId()>0)
			list[i]->preComputeStep(t);
	}
	if (t->getNext1()!=NULL){preComputeStep(t->getNext1());}
	if (t->getNext2()!=NULL){preComputeStep(t->getNext2());}	
}
void ListElaboration::compute(Neuron* n){
	// starting from the soma compute a single step at a time
	computeStep(n->getSoma());
}
void  ListElaboration::computeStep(Segment* t){
	for(int i=0;i<index;i++){
		//skip duplicate somatic segment
		if(t->getId()>0)
			list[i]->computeStep(t);
	}
	if (t->getNext1()!=NULL){
		computeStep(t->getNext1());}
	if (t->getNext2()!=NULL){
		computeStep(t->getNext2());}	
}

void ListElaboration::printDistr(std::string str){
	if(ferr)
	   	cout<<"*";
	for(int i=0;i<index;i++){
		list[i]->printDistr(str);
	}
}

void ListElaboration::printDistrDB(std::string str){
	//printHEader
	static int head=0;
	bool flag = false;
	int i;
	if(head==0){
		head++;
		//print header as one line at start of file
		cout<<"NeuronName ";
		for( i=0;i<index;i++){
			if(i!=0 ) 
				cout<<" ";
			list[i]->printHeader();
		}
		cout<<"\n";
	} 

	//print extracted data from Neurons
	
	for( i=0;i<index;i++){
		if(i!=0 ) 
			cout<<",";
		if(flag == false){
			if(ferr)
			      	cout<<"*"<<str.c_str()<<",";
			else
				cout<<str.c_str()<<",";
			flag = true;
		}
		list[i]->printDistrDB();
	}
		cout<<"\n";
}

void ListElaboration::newNeuron(){
	for(int i=0;i<index;i++){
		list[i]->newNeuron();
	}
}
void ListElaboration::newRealNeuron(){
	for(int i=0;i<index;i++){
		list[i]->newRealNeuron();
	}
}

Elaboration::Elaboration(){};

//for Debug: check the range of input data for distribution
int Distr::minMax(double j){
	
	if(j >= VOID) 
		return 0;
	if(j <= -VOID) 
		return 0;

	if(getMin()==-1 && getMax()==-1){ 
		setMin(j);
		setMax(j); 
	}
	if(j>getMax()) setMax(j);
	if(j<getMin()) setMin(j);
	
	if(j>MAX && j<MIN){
		cerr<< "Exceeded value:"<<j<<"\n";
		return 0;
	}
	return 1;	
}
int Single::minMax(double j){
	
	if(j >= VOID) 
		return 0;
	if(j <= -VOID) 
		return 0;
	if (j>120) {
		int kkuuu=0;
	}

	if(validminmax==0){ 
		validminmax=1;
		setMin(j);
		setMax(j); 
	} else {
		if(j>getMax()) 
			setMax(j);
		if(j<getMin()) 
			setMin(j);
	}
	
	if(j>MAX && j<MIN){
		cerr<< "Exceeded value:"<<j<<"\n";
		return 0;
	}
	return 1;	
}
;
//evaluate which bin to use to store the distribution
void Distr::storeValue(double i,double j){
	
	static int OVER=0;
	
	if ((j>=VOID) || (j<=-VOID)|| (i>=VOID)|| (i<=-VOID)) {	
	
		return;
	}


	if(minMax(j)){
		
		int ind=0;
		if(direct==0){
			
			ind=(int) (((j-getMin())/widthBins)+0.5);
		} else {
			ind=(int)j;
		}
		if(ind>MAX_DATA ){
			//error over index
			if(OVER==0)
				cerr<<"Distrib with index over range allowed!\n";
			ind=MAX_DATA-1; OVER=1;
		}
		//store value
		data[tab][ind]+=i;
		dataSquare[tab][ind]+=i*i;
		counter[tab][ind]++;
		//update the number of occupied bin
		if(index[tab]<ind)
			index[tab]=ind;
		//Print Raw Data
		if(RAW_elab==1)
			cout<<i<<"\t"<<j<<"\n";
	}   //evaluate index;
}
;
// print distrib for db output: one neuron per line
void Distr::printDistrDB(){
	double tot=0;
	if(strcmp(a->getName(),b->getName())==0){
		cout<<a->getName()<<" Distribution:";
	} else {
		cout<<a->getName()<<"<"<<a->getLimit()<<"> VS "<<b->getName()<<"<"<<b->getLimit()<<"> :";
	}
	
	
	int min=getMin(),max=getMax();
	double width=widthBins;
	cout<<","<<b->getName()<<",";
	//all print start at one since the bin number is computed and then added 1
	for(int i=1;i<=index[tab];i++){
			
		cout<<(min+width*(i-.5))<<",";
	}
	cout<<a->getName()<<",";

	
		for( int i=1;i<=index[tab];i++){
			
			if(counter[tab][i]==0){
				cout<<"0"<<",";
			}else {
				if(average==0) {
					cout<<(data[tab][i])<<","; //counter[tab][i])<<" ";
				} else {
					cout<<(data[tab][i]/counter[tab][i])<<",";
				}
			
			}
			cout<<",";

	}

	cout<<"";
	
	if(average!=0) {
		cout<<" StdDev,";
		for( int i=1;i<=index[tab];i++){
				
				if(counter[tab][i]==0){
					cout<<"0"<<",";
				}else if(counter[tab][i]>1){
					
					double numerator = (counter[tab][i]*dataSquare[tab][i])-(data[tab][i]*data[tab][i]);
					double denomenator = counter[tab][i]*(counter[tab][i]-1);
					double std=(sqrt(numerator/denomenator));
					cout<<std<<" ";
				}else
						cout<<0<<" ";

				
				cout<<",";

		}
		cout<<",";
	}
	cerr<<"";

}
;
//fixed the printing for alignment of the functions in double distribution.by sri on 06/20/07
void Distr::printDistr(std::string str){
	double tot=0;
        int i;
        cout<<str.c_str()<<"\t";
	if(strcmp(a->getName(),b->getName())==0){
		cout<<a->getName()<<" Distribution:";
	} else {
		cout<<a->getName()<<a->getLimit()<<"<VS>"<<b->getName()<<b->getLimit()<<":";
	}
	//Print X
	int min = (int)getMin(), max = (int)getMax();
	double width=widthBins;
	cout<<b->getName()<<"\t";
	
	for(i=0;i<=index[tab];i++){
			
			cout<<(min+width*(i+.5))<<"\t";
	}

	cout<<"\n\t"<<a->getName()<<"\t";

	//Print Y
	for( i=0;i<=index[tab];i++){
			
			if(counter[tab][i]==0){
				cout<<"0"<<" ";
			}else {
				if(average==0) {
					cout<<(data[tab][i])<<" "; //counter[tab][i])<<" ";
				} else {
					cout<<(data[tab][i]/counter[tab][i])<<" ";
				}
			
			}
			cout<<"\t";

	}
	cout<<"\n";
	//Print Ystd
	if(average!=0) {
		cout<<" StdDev\t";
		for( i=0;i<=index[tab];i++){
				
				if(counter[tab][i]==0){
					cout<<"0"<<" ";
				}else {
			
					if(counter[tab][i]>1){
					double numerator = (counter[tab][i]*dataSquare[tab][i])-(data[tab][i]*data[tab][i]);
					double denomenator = counter[tab][i]*(counter[tab][i]-1);
					double std=(sqrt(numerator/denomenator));
					cout<<std<<" ";
					}
					else
					cout<<0<<" ";

				
				}
				cout<<"\t";
		}
		cout<<"\n";
	}
}

;
