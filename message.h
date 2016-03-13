#pragma once

struct Message {
    Message() { }
    Message(std::string u, std::string t): user(u), text(t) { }
    std::string user;
    std::string text;
    bool bad() { return text.empty(); }
};
