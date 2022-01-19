#include "follow.hpp"

sem_t* Follow::semaphore = sem_open("/follow", 0x0008, 0x0080 | 0x0010 | 0x0002, 1);

/*
** Initializers
*/

Follow::Follow(std::string followerId, std::string followedId)
{
    this->followerId = followerId;
    this->followedId = followedId;
}

Follow::~Follow() {}

/*
** Attributes Methods
*/

std::string Follow::getFollowerId()
{
    return followerId;
}

std::string Follow::getFollowedId()
{
    return followedId;
}

/*
** Database Methods
*/

QueryStatus Follow::create(Database & db)
{
    sem_wait(Follow::semaphore);
        
    QueryStatus status = std::make_pair(false, std::vector<std::string> ());

    if (db.isOnline && followerId.compare(followedId) != 0) {
        char *errorMsg = 0;
        char command[512];

        sprintf(command, "select * from \"user\" where id = \'%s\' or id = \'%s\';",
                followerId.c_str(), followedId.c_str());

        if (db.count(std::string (command)) == 2) {
            sprintf(command, "insert into follow(followerid, followedid) values(\'%s\', \'%s\');",
                    followerId.c_str(), followedId.c_str());
            status.second.push_back(std::string (command));
            
            if (sqlite3_exec(db.sql, command, 0, 0, &errorMsg) != SQLITE_OK) {
                std::cerr << "ERROR " << errorMsg << std::endl;
                sqlite3_free(errorMsg);
            } else {
                status.first = true;
            }
        }
    }

    sem_post(Follow::semaphore);
    return status;
}

int Follow::countFollowers(Database & db, std::string followedId)
{
    sem_wait(Follow::semaphore);

    char command[512];
    sprintf(command, "select * from follow where followedid = \'%s\';", followedId.c_str());
    int count = db.count(std::string (command));

    sem_post(Follow::semaphore);
    return (count == -1 ? 0 : count);
}

std::vector<FollowStruct> Follow::getFollowers(Database & db, std::string followedId)
{
    sem_wait(Follow::semaphore);

    std::vector<FollowStruct> vf;

    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];

        int followers = Follow::countFollowers(db, followedId);
        if (followers) {
            sprintf(command, "select * from follow where followedid = \'%s\';", followedId.c_str());

            if (sqlite3_exec(db.sql, command, getFollowersCallback, (void*)&vf, &errorMsg) != SQLITE_OK) {
                std::cerr << "ERROR " << errorMsg << std::endl;
                sqlite3_free(errorMsg);
            } else {
                while (vf.size() < followers) {}
            }
        }
    }
    
    sem_post(Follow::semaphore);
    return vf;
}

/*
** Database Callbacks
*/

int Follow::getFollowersCallback(void *data, int argc, char **argv, char **azColName)
{
    FollowStruct fs;
    fs.followerId = argv[0] ? argv[0] : "";
    fs.followedId = argv[1] ? argv[1] : "";
    (*(std::vector<FollowStruct> *) data).push_back(fs);
    return 0;
}