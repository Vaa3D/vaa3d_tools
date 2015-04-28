#include "Limit.h"

Limit::Limit(){name="<NoName>";prev=0;};

Limit::Limit(char* m,Limit* pr){
  name=m;
  prev=pr;
}

int 
Limit::okSegment(Segment *t){return 1;}

char *  
Limit::print()
{ 
  if(prev!=0)
    return ( new RString( getName(),prev->getName() ))->print();
  return getName();
}

char * 
Limit::getName(){return (new RString(name,getParameters()))->print();}


double 
Limit::getValue(){return 0;}

char * 
Limit::getParameters(){return "1";}

int 
Limit::checkLimit(Segment *t)
{
  //check previous limit (recursively!!!)
  if(prev!=0) 
    if(prev->checkLimit(t)==0) return 0;
  //check actual limit
  return okSegment(t);
}




double 
OrderLimit::getValue()
{
  return order;
}


int 
OrderLimit::okSegment(Segment *t)
{
  if(t->getOrder()==order)
    return 1;
  return 0;
}


char * 
OrderLimit::getName(){return (new RString(name,getParameters()))->print();}


char * 
OrderLimit::getParameters()
{	
  return (new RString(order))->print();
}
;

