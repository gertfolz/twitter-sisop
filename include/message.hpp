#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string>
#include <ctime>
#include <utility>
#include <vector>
#include <semaphore.h>

#include "database.hpp"

struct MessageStruct
{
    bool wasInit = false;
    int id;
    std::string text;
    std::string userId;
    int timestamp;
};

class Message
{
private:
	int id;
	std::string text;
	std::string userId;
	int timestamp;

    static sem_t* semaphore;
    static int findByIdCallback(void*, int, char**, char**);

public:
	Message(std::string, std::string);
    ~Message();
    Message();
    std::string getText();

    int getId();
    std::string getUserId();

    bool findById(Database&, int);
    QueryStatus create(Database&);
};

#endif