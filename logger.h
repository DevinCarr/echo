/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <fstream>
#include <string>
#include <ctime>
#include <iostream>

/*
Set to 1 for any DEBUG messages to be sent to the output
Set to 0 to remove only the DEBUG (logger.d()) messages from the log output
***Make sure to set this in your main file to get a DEBUG messages.
*/

#define DEBUG 1

class Log {
public:
    Log();
    Log(std::string);
    Log(const Log& l) { this->logFile = l.logFile; }
    ~Log();
    void open_file();
    void close_file();
    void i(std::string msg);
    void d(std::string msg);
    void w(std::string msg);
    void e(std::string msg);   

private:
    std::ofstream* logFile;
    std::string fileName;
    time_t logger_time;    
    std::string print_time();
};
#endif // LOGGER_H_
