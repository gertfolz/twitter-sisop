#include "../include/session.hpp"

Session::Session(std::string username, int port, std::string serverAddress)
{
    this->username = username;
    this->port = port;
    this->serverAddress = serverAddress;
}

Session::~Session() { }

std::string Session::getUsername()
{
    return this->username;
}

std::string Session::getServerAddress()
{
    return this->serverAddress;
}

int Session::getPort()
{
    return this->port;
}

std::vector<NotificationStruct> Session::getNotifications()
{
    Database db;
    return Notification::getUserNotifications(db, this->username);
}

void Session::sendNotification(std::string message)
{
    try {
        Database db;
        Notification notification;
        Message msg = Message(message, this->username);
        msg.create(db);
        notification.create(db, msg);
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
    }
}
