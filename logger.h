/*
 * The MIT License (MIT) Copyright (c) 2015 Devin Carr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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
