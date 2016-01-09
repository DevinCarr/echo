#ifndef _message_h_
#define _message_h_

struct Message {
    Message(std::string u, std::string t): user(u), text(t) { }
    std::string user;
    std::string text;
};
#endif
