
#include "general.h"

//general.cpp
//Dr. Mengjiao Yu
//Date: Dec 2, 2004
//
//general purposed function
//Please take a look at the input.dat file
//1. The purpose here is to provide a funcition that can bypass the white spaces and comments...
//   That way, it makes the input file more readable.
//2. Standard istream operator >> can NOT take care of strings with whit spaces
//   ReadAString(istream& ) is then provided to read a string in a "" pair.
//


//BypassComment is a function for processing input file. It can be used to
//skip the comments (start with a '#') and white spaces.
//


void BypassComment(istream& ins)
{
    ins>>ws;//skip white space....
    if (ins.eof()) return;//end of file?
    //then skip comments...
    while( (ins.peek()=='#' || ins.peek()=='\n') && (!ins.eof()) )
    {
        string tmp=""; //used only for display purpose..
        char c;
        do
        {
            ins.get(c);//
            tmp+=c;//for display purpose only
        }while (c!='\n' && !ins.eof());//repeatedly
        cout<<"Skipped: "<<tmp<<endl; //display purpose only. you can comment it out...
        if (ins.eof()) return;//end of file?
        ins>>ws;//skip white spaces....
    }

};

//Since the standard istream cannot take care strings with whitespace, we need something to do this
string ReadAString(istream& ins)
{
    string tmp="";
    ins>>ws;//skip white space.....
    if (ins.peek()!='"') //not a string within a "" pair
        {
            ins>>tmp;
        }
    else
       {
        char c;
        //The implementation is not quite efficient. You may use getline() with delim set as '"'...
        //But not a big deal here.
        ins>>c;//skip "
        do{
            ins>>noskipws>>c;//read a char, do NOT skip any white spaces!!!
            if(c!='"') tmp+=c;//add it into the string if it is not the terminator "
          }while (c!='"' && !ins.eof());//
       };
    try
    {
        ins>>ws;//skip white spaces
    }catch(...)
    {
        //if eof, just return.
    }
    //cout<<"Read the following string: "<<tmp<<endl;
    return tmp;
}
//launch an external program
//BOOL LaunchApplication(LPCTSTR pCmdLine, PROCESS_INFORMATION *pProcessInfo)
//{
//   STARTUPINFO stStartUpInfo;
//    memset(&stStartUpInfo, 0, sizeof(STARTUPINFO));

//    stStartUpInfo.cb = sizeof(STARTUPINFO);

//    stStartUpInfo.dwFlags = STARTF_USESHOWWINDOW;
 //   stStartUpInfo.wShowWindow = SW_SHOWDEFAULT;
//	stStartUpInfo.wShowWindow = SW_SHOW;//SW_HIDE;
//	//stStartUpInfo.wShowWindow = SW_SHOW;

//	/*
//    return CreateProcess(NULL, (LPTSTR)pCmdLine, NULL, NULL, FALSE,
//        //IDLE_PRIORITY_CLASS,
//		NORMAL_PRIORITY_CLASS,
//		NULL,
//        NULL, &stStartUpInfo, pProcessInfo);
//	*/
//	return CreateProcess(NULL, (LPSTR)pCmdLine, NULL, NULL, FALSE,
 //       //IDLE_PRIORITY_CLASS,
//		NORMAL_PRIORITY_CLASS,
//		//JobPriority, //priority...
//		NULL,
//      NULL, &stStartUpInfo, pProcessInfo);
//}
