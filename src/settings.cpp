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
    _port(6667)
{ }

Settings::~Settings() {
    save_file();
}

std::string Settings::owner() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS)
        ->FirstChildElement(ELEMENT_USER)
        ->FirstChild()
        ->ToText()
        ->Value());
}

std::string Settings::pass() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS)
        ->FirstChildElement(ELEMENT_PASS)
        ->FirstChild()
        ->ToText()
        ->Value());
}

std::string Settings::nick() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS)
        ->FirstChildElement(ELEMENT_USER)
        ->FirstChild()
        ->ToText()
        ->Value());
}

std::string Settings::channel() {
    return std::string(doc.FirstChildElement(ELEMENT_SETTINGS)
        ->FirstChildElement(ELEMENT_CHANS)
        ->FirstChildElement(ELEMENT_CHAN)
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

    if (username.empty() || username == USER_DUMMY) return false;
    if (password.empty() || password == PASS_DUMMY) return false;
    if (chan.empty()) return false;

    return true;
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
    std::cout << folder_path << std::endl;
    int status = mkdir(folder_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status != 0) {
        if (errno != EEXIST) {
            std::cout << "Could not create directory for settings: " << strerror(errno) << std::endl;
            return false;
        }
    }

    folder_path += LOGS_FOLDER_PATH;
    std::cout << folder_path << std::endl;
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
        XMLElement* settings_node = doc.NewElement(ELEMENT_SETTINGS);
        XMLElement* user = doc.NewElement(ELEMENT_USER);
        XMLElement* pass = doc.NewElement(ELEMENT_PASS);
        XMLElement* channels_node = doc.NewElement(ELEMENT_CHANS);
        XMLElement* channel = doc.NewElement(ELEMENT_CHAN);

        XMLText* user_t = doc.NewText(USER_DUMMY);
        XMLText* pass_t = doc.NewText(PASS_DUMMY);
        XMLText* chan_t = doc.NewText(CHAN_DUMMY);
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

