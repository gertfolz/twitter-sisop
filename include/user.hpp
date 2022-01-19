#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <semaphore.h>

#include "database.hpp"

class User
{
private:
    std::string id;

    static sem_t* semaphore;

public:
    User(std::string);
    ~User();

    std::string getId();

    QueryStatus login(Database&);
};

#endif