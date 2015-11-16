//
//  logger.cpp
//  ConsensusBuilder
//
//  Created by Todd Gillette on 6/28/15.
//
//

#include "logger.h"

Logger::Logger(){
    this->level = logERROR;
    write_level = false;
}
Logger::Logger(LogLevel level){
    this->level = level;
    write_level = false;
};
Logger::Logger(int level){
    this->level = (LogLevel)level;
    write_level = false;
};
LogLevel Logger::get_level(){
    return level;
};
void Logger::set_level(LogLevel level){
    this->level = level;
};
void Logger::set_level(int level){
    this->level = (LogLevel)level;
};
void Logger::log(LogLevel level, std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(level, message, arg_ptr);
    va_end(arg_ptr);
};
void Logger::log(LogLevel level, std::string message, va_list arg_ptr){
    if (write_level){
        message = get_level_string(level)+": "+message;
    }
    message = string_format(message, arg_ptr);
    if (level == logERROR){
        std::cerr << message;
        std::cerr << "\n";
    }else if (this->level >= level){
        std::clog << message;
        std::clog << "\n";
    }
};
void Logger::push_prefix(std::string message_prefix){
    prefix_stack.push(message_prefix);
    this->prefix = get_prefix();
};
void Logger::pop_prefix(){
    prefix_stack.pop();
    this->prefix = get_prefix();
};
std::string Logger::get_prefix(){
    prefix = "";
    std::stack<std::string> temp_stack = prefix_stack;
    while (!prefix_stack.empty()){
        std::string cur_top = prefix_stack.top();
        prefix_stack.pop();
        temp_stack.push(cur_top);
        prefix = cur_top + prefix;
    }
    while (!temp_stack.empty()){
        prefix_stack.push(temp_stack.top());
        temp_stack.pop();
    }
    return prefix;
};
/** Currently having a problem sometimes when there are more than 1 arguments in arg_ptr **/
std::string Logger::string_format(const std::string fmt_str, va_list arg_ptr) {
    int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
//    va_list arg_ptr;
    int count = 0;
    while(1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
//        va_start(arg_ptr, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), arg_ptr);
//        va_end(arg_ptr);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}
void Logger::error(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logERROR,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::warn(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logWARNING,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::info(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logINFO,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::debug(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logDEBUG,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::debug1(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logDEBUG1,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::debug2(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logDEBUG2,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::debug3(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logDEBUG3,message,arg_ptr);
};
void Logger::debug4(std::string message, ...){
    va_list arg_ptr;
    va_start(arg_ptr, message);
    log(logDEBUG4,message,arg_ptr);
    va_end(arg_ptr);
};
void Logger::new_line(){
    std::clog << "\n";
}
std::string Logger::get_level_string(){
    return get_level_string(level);
};
void Logger::set_write_level(bool write_level){
    this->write_level = write_level;
};
std::string Logger::get_level_string(LogLevel level){
    switch(level){
        case logERROR:
            return "ERROR";
            break;
        case logWARNING:
            return "WARN";
            break;
        case logINFO:
            return "INFO";
            break;
        case logDEBUG:
            return "DEBUG";
            break;
        case logDEBUG1:
            return "DEBUG1";
            break;
        case logDEBUG2:
            return "DEBUG2";
            break;
        case logDEBUG3:
            return "DEBUG3";
            break;
        case logDEBUG4:
            return "DEBUG4";
            break;
    }
};
