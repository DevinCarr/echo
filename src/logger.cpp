/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/logger.h"

Log::Log() { }

Log::Log(std::string file): fileName(file) { }

Log::~Log() { delete logFile; }

// Open the log file with <date>_fileName.log or <date>.log
void Log::open_file() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer [80];
    time(&rawtime);
    logger_time = rawtime;
    timeinfo = localtime(&rawtime);
    strftime(buffer,80,"%F",timeinfo);
    fileName = std::string(buffer) + (fileName.size()?"_":"") + fileName + ".log";

    logFile = new std::ofstream(fileName, std::ofstream::out | std::ofstream::app);
}

// Close the currently open log file
void Log::close_file() {
    if(logFile->is_open()) {
        logFile->close();
    }
}

// Print the time string
// Also checks for a new day to open a new log
std::string Log::print_time() {
    struct tm* timeinfo;
    time_t t;
    time(&t);
    // Check time
    double diff_seconds = difftime(t,logger_time);
    // If greater than a day old, open a new one.
    if (diff_seconds >= 86400) {
        close_file();
        open_file();
    }
    char buffer[80];
    timeinfo = localtime(&t);
    strftime(buffer,80,"[%c]",timeinfo);
    return buffer;
}

// Info message
void Log::i(std::string msg) {
    if(logFile->is_open()) {
        *logFile << print_time()
            << "[INFO ] "
            << msg << std::endl;
        logFile->flush();
    } else {
        open_file();
    }
}

// Debug message
void Log::d(std::string msg) {
    if(logFile->is_open()) {
        if (DEBUG) {
            *logFile << print_time()
                << "[DEBUG] "
                << msg << std::endl;
            logFile->flush();
        }
    } else {
        open_file();
    }
}
     
// Warning message
void Log::w(std::string msg) {
    if(logFile->is_open()) {
        *logFile << print_time()
                << "[WARN ] "
                << msg << std::endl;
        logFile->flush();
    } else {
        open_file();
    }
}

// Error message
void Log::e(std::string msg) {
    if(logFile->is_open()) {
        *logFile << print_time()
                << "[ERROR] "
                << msg << std::endl;
        logFile->flush();
    } else {
        open_file();
    }
}
