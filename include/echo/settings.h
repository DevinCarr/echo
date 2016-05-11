/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#pragma once

#include "spdlog/spdlog.h"
#include "tinyxml2.h"

#include <string>
#include <iostream>
#include <locale>

#ifdef _WIN32
#include <shlobj.h>
#include <codecvt>
#include <Windows.h>
#endif

const std::wstring WECHO_FOLDER = L"\\echo";
const std::wstring WLOGS_FOLDER = L"\\logs";
const std::string LOGS_FOLDER = "\\logs\\echo";
const std::string SETTINGS_FILE_NAME = "\\settings.xml";

// XML Element names
using namespace tinyxml2;

static const char* ELEMENT_SETTINGS = "settings";
static const char* ELEMENT_USER = "username";
static const char* ELEMENT_PASS = "password";
static const char* ELEMENT_CHANS = "channels";
static const char* ELEMENT_CHAN = "channel";

// Default template values for XML
static const char* USER_DUMMY = "Kappa";
static const char* PASS_DUMMY = "kappa123";
static const char* CHAN_DUMMY = "pajlada";

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
private:
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
};