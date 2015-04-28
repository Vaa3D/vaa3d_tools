#ifndef RSTRING_H
#define RSTRING_H
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <cmath>
#include <stdio.h>
using namespace std;
// needed for linux compilation
void reverse(char s[]);

void itoa1(int n, char s[], int begin);




class RString{
	char* name;
	int length;

	int getLength(){ return length;}

	char* getName(){return name;}
public:

	RString(){name="";length=0;}

	RString(const char* S) {
		length=strlen(S) + 1;
		name = new char[length];
		strcpy(name,S);
	}

	RString(char * a,char * b){
		length=strlen(a)+strlen(b)+1;
		name=new char[length];
		strcpy(name,a);
		strcat(name,b);
	}

	RString(int value){

	 	int i= (int)(log10(value)+1.0);
		length=i;
		name=new char[i+1];
		itoa1((int) value,name,10);


	}

	void add(char *a){
		char* tmp=new char[length+1];
		strcpy(tmp,name);
		delete name;

		length+=+strlen(a);
		name=new char[length+1];
		strcpy(name,tmp);
		strcat(name,a);
		delete [] tmp;
	}

	void add(int value){

		char* tmp=new char[length+1];
		strcpy(tmp,name);
		delete name;

	 	int i= (int)(log10(value)+2.0);
		length+=i;
		name=new char[length+1];

		char* iSt=new char[i];
		itoa1((int) value,iSt,10);

		strcpy(name,tmp);
		strcat(name,iSt);
		delete [] tmp;
	}

	RString(RString* a,RString* b){
		int len=a->getLength()+b->getLength();
		if(len>100){
			cerr<<"String too long;";
			exit(-1);
		}
		char tmp[100];
		strcpy(tmp,a->getName());
		strcat(tmp,b->getName());
		name=tmp;
		length=len;
	}

	~RString() { delete name; }

	char * print(){return name;}
}

;

#endif
