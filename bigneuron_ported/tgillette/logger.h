//
//  logger.h
//  ConsensusBuilder
//
//  Created by Todd Gillette on 6/28/15.
//
//

#ifndef __ConsensusBuilder__logger__
#define __ConsensusBuilder__logger__

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stack>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

enum LogLevel{logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4};

class Logger{
    LogLevel level;
    std::stack<std::string> prefix_stack;
    std::string prefix;
    bool write_level;
    std::string string_format(std::string fmt_str, va_list arg_ptr);
public:
    Logger(LogLevel level=logERROR);
    Logger(int level=0);
    LogLevel get_level();
    void push_prefix(std::string message_prefix);
    void pop_prefix();
    std::string get_prefix();
    void log(LogLevel level, std::string message, ...);
    void log(LogLevel level, std::string message, va_list arg_ptr);
    void error(std::string message, ...);
    void warn(std::string message, ...);
    void info(std::string message, ...);
    void debug(std::string message, ...);
    void debug1(std::string message, ...);
    void debug2(std::string message, ...);
    void debug3(std::string message, ...);
    void debug4(std::string message, ...);
    void set_write_level(bool write_level);
    std::string get_level_string();
    std::string get_level_string(LogLevel level);
};

#endif /* defined(__ConsensusBuilder__logger__) */
