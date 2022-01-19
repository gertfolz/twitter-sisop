#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <semaphore.h>

#include "database.hpp"
#include "message.hpp"
#include "follow.hpp"

struct NotificationStruct
{
    int messageId;
    std::string userId;
    bool seen;
};

class Notification
{
private:
    int messageId;
    std::string userId;
    bool seen;

    static sem_t* semaphore;
    static int getUserNotificationsCallback(void *, int, char **, char **);

public:
    Notification();
    ~Notification();

    static int countNotifications(Database &, std::string);
    static std::vector<NotificationStruct> getUserNotifications(Database &, std::string );
    static QueryStatus setToSeen(Database &, int, const char*);
    static QueryStatus create(Database &, Message &);
};

#endif