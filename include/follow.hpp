#ifndef FOLLOW_H
#define FOLLOW_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <semaphore.h>

#include "database.hpp"

struct FollowStruct
{
    std::string followerId;
    std::string followedId;
};

class Follow
{
private:
    std::string followerId;
    std::string followedId;

    static sem_t* semaphore;
    static int getFollowersCallback(void*, int, char**, char**);

public:
    Follow(std::string, std::string);
    ~Follow();

    std::string getFollowerId();
    std::string getFollowedId();

    QueryStatus create(Database&);

    static int countFollowers(Database&, std::string);
    static std::vector<FollowStruct> getFollowers(Database&, std::string);
};

#endif