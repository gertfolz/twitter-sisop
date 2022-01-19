#include "user.hpp"

sem_t* User::semaphore = sem_open("/user", 0x0008, 0x0080 | 0x0010 | 0x0002, 1);

/*
** Initializers
*/

User::User(std::string id)
{
    this->id = id;
}

User::~User() {}

/*
** Attributes Methods
*/

std::string User::getId()
{
    return id;
}

/*
** Database Methods
*/

QueryStatus User::login(Database & db)
{
    sem_wait(User::semaphore);

    QueryStatus status = std::make_pair(false, std::vector<std::string> ());

    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];

        sprintf(command, "select * from \"user\" where id = \'%s\';", id.c_str());
        if (db.count(std::string (command)) > 0) {
            status.first = true;
        } else {
            sprintf(command, "insert into \"user\"(id) values(\'%s\');", id.c_str());
            status.second.push_back(std::string (command));

            if (sqlite3_exec(db.sql, command, 0, 0, &errorMsg) != SQLITE_OK) {
                std::cerr << "ERROR " << errorMsg << std::endl;
                sqlite3_free(errorMsg);
            } else {
                status.first = true;
            }
        }
    }

    sem_post(User::semaphore);
    return status;
}