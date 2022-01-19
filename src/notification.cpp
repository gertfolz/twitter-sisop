#include "notification.hpp"

sem_t* Notification::semaphore = sem_open("/notification", 0x0008, 0x0080 | 0x0010 | 0x0002, 1);

/*
** Database Methods
*/

int Notification::countNotifications(Database &db, std::string userId)
{
    sem_wait(Notification::semaphore);

    char command[512];
    sprintf(command, "select * from notification where userid = \'%s\' and seen = 0;", userId.c_str());
    int count = db.count(std::string (command));

    sem_post(Notification::semaphore);
    return (count == -1 ? 0 : count);
}

std::vector<NotificationStruct> Notification::getUserNotifications(Database &db, std::string userId)
{
    sem_wait(Notification::semaphore);

    std::vector<NotificationStruct> vn;

    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];

        int notificationsCount = Notification::countNotifications(db, userId);
        if (notificationsCount) {
            sprintf(command, "select * from notification where userid = \'%s\' and seen = 0;", userId.c_str());

            if (sqlite3_exec(db.sql, command, getUserNotificationsCallback, (void *)&vn, &errorMsg) != SQLITE_OK) {
                std::cerr << "ERROR " << errorMsg << std::endl;
                sqlite3_free(errorMsg);
            } else {
                while (vn.size() < notificationsCount) { }
            }
        }
    }

    sem_post(Notification::semaphore);
    return vn;
}

QueryStatus Notification::setToSeen(Database &db, int messageId, const char *userId)
{
    sem_wait(Notification::semaphore);

    QueryStatus status = std::make_pair(false, std::vector<std::string> ());

    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];

        sprintf(command, "update notification set seen = 1 where messageId =  \'%d\' and userId = \'%s\';",
                messageId, userId);
        status.second.push_back(std::string (command));

        if (sqlite3_exec(db.sql, command, 0, 0, &errorMsg) != SQLITE_OK) {
            std::cerr << "ERROR " << errorMsg << std::endl;
            sqlite3_free(errorMsg);
        } else {
            status.first = true;
        }
    }

    sem_post(Notification::semaphore);
    return status;
}

QueryStatus Notification::create(Database &db, Message &m)
{
    sem_wait(Notification::semaphore);

    QueryStatus status = std::make_pair(true, std::vector<std::string> ());
    std::vector<FollowStruct> followers = Follow::getFollowers(db, m.getUserId());

    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];

        for (FollowStruct follower : followers) {
            sprintf(command, "insert into notification(messageid, userid) values(%d, \'%s\');",
                    m.getId(), follower.followerId.c_str());
            status.second.push_back(std::string (command));

            if (sqlite3_exec(db.sql, command, 0, 0, &errorMsg) != SQLITE_OK) {
                std::cerr << "ERROR " << errorMsg << std::endl;
                sqlite3_free(errorMsg);
                status.first = false;
            }
        }
    } else {
        status.first = false;
    }

    sem_post(Notification::semaphore);
    return status;
}

/*
** Database Callbacks
*/

int Notification::getUserNotificationsCallback(void *data, int argc, char **argv, char **azColName)
{
    NotificationStruct ns;
    ns.messageId = atoi(argv[0] ? argv[0] : "");
    ns.userId = argv[1] ? argv[1] : "";
    ns.seen = atoi(argv[2] ? argv[2] : "") == 1;
    (*(std::vector<NotificationStruct> *)data).push_back(ns);
    return 0;
}