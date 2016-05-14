/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#include "echo/settings.h"

Settings::Settings() :
    settings_filepath(std::string()),
    log_filepath(std::string()),
    _tmi_hostname("irc.twitch.tv"),
    _whispers_hostname("irc.chat.twitch.tv"),
    _port(6667),
    _running(false)
{ }

Settings::~Settings() {
    save_file();
}

std::string Settings::owner() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS.c_str())
        ->FirstChildElement(ELEMENT_USER.c_str())
        ->FirstChild()
        ->ToText()
        ->Value());
}

std::string Settings::pass() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS.c_str())
        ->FirstChildElement(ELEMENT_PASS.c_str())
        ->FirstChild()
        ->ToText()
        ->Value());
}

std::string Settings::nick() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS.c_str())
        ->FirstChildElement(ELEMENT_USER.c_str())
        ->FirstChild()
        ->ToText()
        ->Value());
}

std::string Settings::channel() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS.c_str())
        ->FirstChildElement(ELEMENT_CHANS.c_str())
        ->FirstChildElement(ELEMENT_CHAN.c_str())
        ->FirstChild()
        ->ToText()
        ->Value());
}

// Public function to intitialize the Settings folders for logs and credientials for IRC
bool Settings::init() {
    if (!check_folders()) return false;
    if (open_file() != 0) return false;
    if (!start_logs()) return false;
    return true;
}

// Verify the username and password along with channel as valid
bool Settings::verify_credentials() {
    std::string username = nick();
    std::string password = pass();
    std::string chan = channel();

    if (username.empty() || username == USER_DUMMY.c_str()) return false;
    if (password.empty() || password == PASS_DUMMY.c_str()) return false;
    if (chan.empty()) return false;

    return true;
}

// Wait for the client to stop running
void Settings::wait() {
    std::unique_lock<std::mutex> lock(running_mut);
    while (_running) running_cv.wait(lock);
}

// Open or create a new settings file
bool Settings::check_folders() {
#ifdef _WIN32

    // get folder path for %APPDATA%/Roaming/echo/
    wchar_t* path = 0;
    HRESULT result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &path);
    if (result != S_OK) {
        std::cout << "Could not initialize or find %APPDATA% folder." << std::endl;
        return false;
    }
    // convert the filename to a std::string from std::wstring
    std::wstring wpath(path);
    CoTaskMemFree(static_cast<void*>(path)); 
    // set the filepath for later
    wpath += WECHO_FOLDER;
    settings_filepath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wpath);
    log_filepath = settings_filepath + LOGS_FOLDER;
    settings_filepath += SETTINGS_FILE_NAME;

    // try and create the directory for settings
    if (CreateDirectory(wpath.c_str(), NULL) == 0) {
        // check if it already exists
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            std::cout << "Could not create directory for settings" << std::endl;
            return false;
        }
    }

    // try and create the directory for logs in settings
    wpath += WLOGS_FOLDER;
    if (CreateDirectory(wpath.c_str(), NULL) == 0) {
        // check if it already exists
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            std::cout << "Could not create directory for logs" << std::endl;
            return false;
        }
    }
#else
    const char* path_c = std::getenv("HOME");
    std::string path(path_c);
    // make sure the folder for #HOME exists
    if (path.empty()) return false;

    // set the filepath
    settings_filepath = path + ECHO_FOLDER;
    log_filepath = settings_filepath + LOGS_FOLDER;
    settings_filepath += SETTINGS_FILE_NAME_NIX;

    // create the echo directory
    std::string folder_path = path + ECHO_FOLDER;
    int status = mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status != 0) {
        if (errno != EEXIST) {
            std::cout << "Could not create directory for settings: " << strerror(errno) << std::endl;
            return false;
        }
    }

    folder_path += LOGS_FOLDER_PATH;
    status = mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status != 0) {
        if (errno != EEXIST) {
            std::cout << "Could not create directory for logs: " << strerror(errno) << std::endl;
            return false;
        }
    }

#endif
    return true;
}

// Open the settings file
int Settings::open_file() {
    // load the settings file
    XMLError error = doc.LoadFile(settings_filepath.c_str());

    if (error == XML_ERROR_FILE_NOT_FOUND) {
        // create a default template file
        XMLElement* settings_node = doc.NewElement(ELEMENT_SETTINGS.c_str());
        XMLElement* user = doc.NewElement(ELEMENT_USER.c_str());
        XMLElement* pass = doc.NewElement(ELEMENT_PASS.c_str());
        XMLElement* channels_node = doc.NewElement(ELEMENT_CHANS.c_str());
        XMLElement* channel = doc.NewElement(ELEMENT_CHAN.c_str());

        XMLText* user_t = doc.NewText(USER_DUMMY.c_str());
        XMLText* pass_t = doc.NewText(PASS_DUMMY.c_str());
        XMLText* chan_t = doc.NewText(CHAN_DUMMY.c_str());
        user->LinkEndChild(user_t);
        pass->LinkEndChild(pass_t);
        channel->LinkEndChild(chan_t);

        channels_node->LinkEndChild(channel);
        settings_node->LinkEndChild(user);
        settings_node->LinkEndChild(pass);
        settings_node->LinkEndChild(channels_node);

        doc.LinkEndChild(settings_node);
        doc.SaveFile(settings_filepath.c_str());
        std::cout << "Echo has been initialized.\n"
            << "To begin, enter your twitch.tv username and password in the settings.xml\n"
            << "file located here:\n" << settings_filepath << std::endl;
        return -1;
    }
    else if (error == XML_SUCCESS) {
        // file loaded properly
        return 0;
    }
    else {
        std::cout << "XMLError: " << error << std::endl;
        return error;
    }
}

// Save the current settings file
void Settings::save_file() {
    doc.SaveFile(settings_filepath.c_str());
}

// Check and start the logger
bool Settings::start_logs() {
    try {
        log = spdlog::daily_logger_mt("echo", log_filepath, 0, 0, true);
        spdlog::set_level(spdlog::level::debug);
        log->debug("Starting echo");
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log failed to initalize: " <<  ex.what() << std::endl;
        return false;
    }
    return true;
}

