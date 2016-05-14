/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#pragma once

#include "spdlog/spdlog.h"
#include "tinyxml2.h"

#include <atomic>
#include <condition_variable>
#include <string>
#include <iostream>
#include <locale>

#ifdef _WIN32
#include <shlobj.h>
#include <codecvt>
#include <Windows.h>

// Windows constants
const std::wstring WECHO_FOLDER = L"\\echo";
const std::wstring WLOGS_FOLDER = L"\\logs";
const std::string LOGS_FOLDER = "\\logs\\echo";
const std::string SETTINGS_FILE_NAME = "\\settings.xml";
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

// *NIX constants
const std::string ECHO_FOLDER = "/.echo";
const std::string LOGS_FOLDER_PATH = "/logs";
const std::string LOGS_FOLDER = "/logs/echo";
const std::string SETTINGS_FILE_NAME_NIX = "/settings.xml";
#endif

using namespace tinyxml2;

class Settings {
public:
    Settings();
    ~Settings();

    std::string settings_filepath;

    std::string tmi_host() { return _tmi_hostname; }
    std::string whispers_host() { return _whispers_hostname; }
    int port() { return _port; }
    std::string owner();
    std::string pass();
    std::string nick();
    std::string channel();

    bool init();
    bool verify_credentials();

    void start() { _running = true; }
    void stop() { _running = false; running_cv.notify_all(); }
    void wait();
private:
    // XMLElement names
    std::string ELEMENT_SETTINGS = "settings";
    std::string ELEMENT_USER = "username";
    std::string ELEMENT_PASS = "password";
    std::string ELEMENT_CHANS = "channels";
    std::string ELEMENT_CHAN = "channel";
    // Default template values for XML
    std::string USER_DUMMY = "Kappa";
    std::string PASS_DUMMY = "kappa123";
    std::string CHAN_DUMMY = "pajlada";

    std::shared_ptr<spdlog::logger> log;
    std::string log_filepath;
    tinyxml2::XMLDocument doc;

    std::string _tmi_hostname;
    std::string _whispers_hostname;
    int _port;

    bool check_folders();
    int open_file();
    void save_file();
    bool start_logs();

    // State condition variables
    std::atomic_bool _running;
    std::mutex running_mut;
    std::condition_variable running_cv;
};
