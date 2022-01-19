#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <vector>
#include <exception>
#include <iostream>

#include "database.hpp"
#include "message.hpp"
#include "notification.hpp"

class Session
{
private:
    std::string username;
    int port;
    std::string serverAddress;

public:
    Session(std::string username, int port, std::string serverAddress);
    ~Session();
    std::string getUsername();
    std::string getServerAddress();
    int getPort();
    std::vector<NotificationStruct> getNotifications();
    void sendNotification(std::string message);
};

#endif