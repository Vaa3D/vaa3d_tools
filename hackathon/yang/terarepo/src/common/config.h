#ifndef TERASTITCHER_CONFIG_H
#define TERASTITCHER_CONFIG_H

#include <string>
#include <ctime>
#include <cstdarg>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>


namespace terastitcher
{
	/*******************
    *    INTERFACES    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };  //debug levels
	/*-------------------------------------------------------------------------------------------------------------------------*/


	/*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern std::string version;					   //TeraStitcher version
    extern std::string teraconverter_version;	   //TeraConverter version
    extern std::string terastitcher2_version;	   //TeraStitcher2 version
    extern std::string mdatagenerator_version;     //mdatagenerator version
	extern std::string mergedisplacement_version;  // mergedisplacement verision
    extern int DEBUG;							   //debug level of current module
	extern std::string qtversion;				   //Qt version (if used)
    /*-------------------------------------------------------------------------------------------------------------------------*/

	
	 /********************************************
     *   Cross-platform UTILITY functions	    *
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    //string-based sprintf function
    inline std::string strprintf(const std::string fmt, ...){
        int size = 100;
        std::string str;
        va_list ap;
        while (1) {
            str.resize(size);
            va_start(ap, fmt);
            int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
            va_end(ap);
            if (n > -1 && n < size) {
                str.resize(n);
                return str;
            }
            if (n > -1)
                size = n + 1;
            else
                size *= 2;
        }
        return str;
    }

	// tokenizer
	inline void	split(const std::string & theString, std::string delim, std::vector<std::string>& tokens)
	{
		size_t  start = 0, end = 0;
		while ( end != std::string::npos)
		{
			end = theString.find( delim, start);

			// If at end, use length=maxLength.  Else use length=end-start.
			tokens.push_back( theString.substr( start,
				(end == std::string::npos) ? std::string::npos : end - start));

			// If at end, use start=maxSize.  Else use start=end+delimiter.
			start = (   ( end > (std::string::npos - delim.size()) )
				?  std::string::npos  :  end + delim.size());
		}
	}

	// removes all tab, space and newline characters from the given string
	inline std::string cls(std::string string){
		string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
		string.erase(std::remove(string.begin(), string.end(), ' '),  string.end());
		string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
		return string;
	}

	//number to string conversion function and vice versa
	template <typename T>
	std::string num2str ( T Number ){
		std::stringstream ss;
		ss << Number;
		return ss.str();
	}
	template <typename T>
	T str2num ( const std::string &Text ){                              
		std::stringstream ss(Text);
		T result;
		return ss >> result ? result : 0;
	}

	inline float getMajorVersionFloat(std::string _version)
	{
		size_t pos = _version.rfind(".");
		std::string major_version = _version.substr(0, pos);
		return str2num<float>(major_version);
	}
	inline std::string getMajorVersion(){return num2str<float>(getMajorVersionFloat(version));}
	/*-------------------------------------------------------------------------------------------------------------------------*/


    //cross-platform current function macro
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
    # define __tsp__current__function__ __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
    # define __tsp__current__function__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
    # define __tsp__current__function__ __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    # define __tsp__current__function__ __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    # define __tsp__current__function__ __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    # define __tsp__current__function__ __func__
    #else
    # define __tsp__current__function__ "(unknown)"
    #endif

    /***********************************************
    *    DEBUG, WARNING and EXCEPTION FUNCTIONS    *
    ************************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
	inline std::string shortFuncName(const std::string & longname)
	{
		std::vector <std::string> tokens;
		split(longname, "::", tokens);
		tokens[0] = tokens[0].substr(tokens[0].rfind(" ")+1);
		int k=1; bool found =false;
		for(;k<tokens.size() && !found; k++)
			if(tokens[k].find("(") != std::string::npos)
			{
				found = true;
				tokens[k] = tokens[k].substr(0,tokens[k].find("("));
			}
			std::string result=tokens[0];
			for(int i=1; i<k; i++)
				result = result + "::" +  tokens[i];
			return result;

	}

	inline void warning(const char* message, const char* source = 0)
	{
			if(source)
				printf("\n**** WARNING (source: \"%s\") ****\n"
				"    |=> \"%s\"\n\n", source, message);
			else
				printf("\n**** WARNING ****: %s\n", message);
	}

	inline void debug(debug_level dbg_level, const char* message=0, const char* source=0)
	{
		if(DEBUG >= dbg_level)
		{
			if(message && source)
				printf("\n---(debug level %d)--- in \"%s\"\n"
						"                             message: %s\n\n", dbg_level, shortFuncName(source).c_str(), message);
			else if(message)
				printf("\n---(debug level %d)---       message: %s\"\n\n", dbg_level, message);
			else if(source)
				printf("\n---(debug level %d)--- in \"%s\"\n", dbg_level, shortFuncName(source).c_str());
		}
	}
}
namespace ts = terastitcher;	//a short alias for the current namespace

#endif // TERASTITCHER_CONFIG_H