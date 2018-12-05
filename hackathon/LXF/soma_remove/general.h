
#pragma once

#ifndef GENERAL_H
#define GENERAL_H


//general.h
//Dr. Mengjiao Yu
//Date: Dec 2, 2004
//
//general purposed function
//Please take a look at the input.dat file
//1. The purpose here is to provide a funcition that can bypass the white spaces and comments...
//   That way, it makes the input file more readable.
//2. Standard istream operator >> can NOT take care of strings with white spaces
//   ReadAString(istream& ) is then provided to read a string in a "" pair.
//



#include <iostream>
#include <vector>
#include <list>
#include <string>

using namespace std;


//BypassComment is a function for processing input file. It can be used to
//skip the comments (start with a '#') and white spaces.
void BypassComment(istream& ins);

//Since the standard istream cannot take care strings with whitespace, we need something to do this
string ReadAString(istream& ins);

//launch an external program
//BOOL LaunchApplication(LPCTSTR pCmdLine, PROCESS_INFORMATION *pProcessInfo);
//
//





#endif // GENERAL_H
