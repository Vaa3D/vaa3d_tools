#ifndef FINDINGTIPS
#define FINDINGTIPS

#include <fstream>
#include <iostream>
#include <cstring>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>


extern std::string g_sAppDir;
extern std::string g_sInputRawFileName;
extern std::string g_sImageInfoName;
extern std::string g_sOutSwcFileName;

extern std::string g_sInputRawFilePath;
extern std::string g_sImageInfoPath;
extern std::string g_sOutSwcFilePath;

extern double g_rThreshold;
extern int g_nSomaX, g_nSomaY, g_nSomaZ;
extern int width, height, zSize;
extern unsigned char *imgBuf_raw;

int findingtipsmain();

inline std::string normalizePath(const std::string& sPath){
    std::string sOut = sPath;
    size_t nL = sPath.size();
    if (nL == 0) return sOut;
    size_t iLastC = nL - 1;
    if ( (sOut[iLastC] != '\\') && (sOut[iLastC] != '/'))
            sOut.append("/");
    return sOut;
} // normalizePath

inline std::string parentPath(const std::string& sPath){
    std::string sTmp = sPath;
    char *pcSrc = &sTmp[0];
    char *pcHead = NULL;
    do{
        pcHead = strpbrk(pcSrc, "/\\:");
        if(pcHead) pcSrc = pcHead + 1;
    }while(pcHead);
    *pcSrc = 0;
    return std::string(&sTmp[0]);
}

template <class T>
static std::string toString(const T& t, int nWidth = 0, int nPrecision = 0){
  std::ostringstream buf;
  buf << t;
  return buf.str();
} // toString

#endif // FINDINGTIPS



