//#include "Elaboration.h"
#include "Segment.h"
#include "Neuron.h"
#include "Func.h"


;

Func *  
Dim::getPreFunc(){ return this;}

int 
Dim::countSegment(Segment *t)
{
  int tmp=1;
  if(t->getNext1()!=NULL)
    tmp= tmp+countSegment(t->getNext1());  
  if(t->getNext2()!=NULL)
   tmp+=countSegment(t->getNext2());
      return tmp;
}



void Dim::fill(Segment * t, float **data){

  static int idx=1,idy=1,idz=1;
  //all three idx,idy,idz variables should be intialized to 1. Otherwise, the data array is not populated for all the x or y or z values. sri 05/10/2010.
  if(t->getId()==1){
    idx=1;idy=1;idz=1;
  }
  
  
 	if(limit==0 || limit->computeStep(t)!=0 )
  {
   if(x==1){
     if (t->getPrev() == NULL && t->getPrev2()== NULL)
	 idx =1;
	
	   data[idx][2]=t->getLength();
      data[idx][1]=t->getEnd()->getX();
	  idx++;
	 
    } else if(y==1){
	if (t->getPrev() == NULL && t->getPrev2()== NULL)
	 idy =1;
     data[idy][2]=t->getLength();
      data[idy][1]=t->getEnd()->getY();
	  idy++;
    }else {
     if (t->getPrev() == NULL && t->getPrev2()== NULL)
	 idz =1;
      data[idz][2]=t->getLength();
     data[idz][1]=t->getEnd()->getZ();
      idz++;
    }
   
    
    
  }
  if(t->getNext1()!=NULL)
    fill(t->getNext1(), data);
  if(t->getNext2()!=NULL)
    fill(t->getNext2(), data);
  
  
}



double  
Dim::computeStep(Segment *t){ 
  float** data;
 
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


//defined a new variable to keep track of new neurons in case of multiple files. sri 05/10/2010  
  static const char* nameorig = t->getNeuronName();
	  
  const char* nametmp = t->getNeuronName();
  
  //whenever the neuron changes all the flags for computing fill are reset. For each neuron and each function only once fill is computed. sri 05/10/2010.
  if(strncmp(nameorig,nametmp,strlen(nameorig))!=0){
	  nameorig = t->getNeuronName();
	  xflg = false;yflg = false; zflg = false;
		  }

	if(strncmp(nameorig,nametmp,strlen(nameorig))==0 && (( x==1 && xflg == false)  || (y==1 && yflg == false) || (z==1 && zflg == false))){
	 
	  while(t->getId()!=1){
		  t = t->getPrev();
	  }
	  
	
	id1=t->getId();
   
	
    if(x==1){
      setName("Width");
	  
	  xflg=true;
    } else if(y==1){
      setName("Height");
	  yflg=true;
    }else if(z==1){
		zflg=true;
		setName("Depth");
    }
    
    //count segment
    int seg=0;
    seg=countSegment(t);
    //cerr<<"first seg:"<<seg<<"\n";
    data=matrix(seg,2);
    
    for(i=1;i<=seg;i++)
      for(int j=1;j<=2;j++){
        data[i][j]=0;
      }
     
	  //fills the data array with X or Y or Z coordinates 
    fill(t,data);
    
    Quicksort(data,1,seg);

    //add the smallest element to the array to have all point positive
    float bias=data[1][1];

    for( i=1;i<=seg;i++){
    
		//subtracting the min value from all the values
      data[i][1]-=bias;
    }
    
    
    
    //get total length
    double tot=0;
	
    for( i=1;i<=seg;i++)
      tot+=data[i][2];
	//get only 2.5% of total length
	int sum =tot;
	percent = 0.025;
    tot*=percent;
	//to remove 2.5% of data from leftmost end 
    double tot_left=0;
    for( i=1;(i<=seg && tot>tot_left);i++)
      tot_left+=data[i][2];
    
    if(i>seg) i=seg;
    
    double min=data[i][1];
	
	//to remove 2.5% of data from rightmost end
	double tot_right= 0;
	for(i=seg;(i>=1 && tot>tot_right);i--)
		tot_right+=data[i][2];
	if(i<1) i=1;
	double max=data[i][1];

    free_matrix(data,seg,2);
    correctOutput=max-min;
   if(x==1){
      correctOutputx=correctOutput;return correctOutput;
    } else if(y==1){
      correctOutputy=correctOutput;return correctOutput;
    }else if(z==1){
		correctOutputz=correctOutput;return correctOutput;
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
