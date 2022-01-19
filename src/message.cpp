#include "message.hpp"

sem_t* Message::semaphore = sem_open("/message", 0x0008, 0x0080 | 0x0010 | 0x0002, 1);

/*
** Initializers
*/

Message::Message(std::string text, std::string userId)
{
    this->text = text;
    this->userId = userId;
}

Message::Message(){}

Message::~Message() {}

/*
** Attributes Methods
*/

int Message::getId()
{
    return id;
}

std::string Message::getText()
{
    return text;
}

std::string Message::getUserId() {
    return userId;
}

/*
** Database Methods
*/

bool Message::findById(Database & db, int id)
{
    sem_wait(Message::semaphore);
    
    bool status = false;
    
    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];
        MessageStruct result[1] = {};

        sprintf(command, "select * from message where id = %d;", id);

        if (db.count(std::string (command)) > 0) {
            if (sqlite3_exec(db.sql, command, findByIdCallback, (void*)result, &errorMsg) != SQLITE_OK) {
                std::cerr << "ERROR " << errorMsg << std::endl;
                sqlite3_free(errorMsg);
            } else {
                while (!result[0].wasInit) {}
                this->id = result[0].id;
                this->text = result[0].text;
                this->userId = result[0].userId;
                this->timestamp = result[0].timestamp;
                status = true;
            }
        }
    }

    sem_post(Message::semaphore);
    return status;
}

QueryStatus Message::create(Database & db)
{
    sem_wait(Message::semaphore);

    QueryStatus status = std::make_pair(false, std::vector<std::string> ());

    if (db.isOnline) {
        char *errorMsg = 0;
        char command[512];

        timestamp = time(0);
        sprintf(command, "insert into message(text, userid, timestamp) values(\'%s\', \'%s\', %d);",
                text.c_str(), userId.c_str(), timestamp);
        status.second.push_back(std::string (command));

        if (sqlite3_exec(db.sql, command, 0, 0, &errorMsg) != SQLITE_OK) {
            std::cerr << "ERROR " << errorMsg << std::endl;
            sqlite3_free(errorMsg);
        } else {
            this->id = sqlite3_last_insert_rowid(db.sql);
            status.first = true;
        }
    }

    sem_post(Message::semaphore);
    return status;
}

/*
** Database Callbacks
*/

int Message::findByIdCallback(void *data, int argc, char **argv, char **azColName) {
    MessageStruct ms;
    ms.id = atoi(argv[0] ? argv[0] : "-1");
    ms.text = argv[1] ? argv[1] : "";
    ms.userId = argv[2] ? argv[2] : "";
    ms.timestamp = atoi(argv[3] ? argv[3] : "1");
    ms.wasInit = true;
    ((MessageStruct *) data)[0] = ms;
    return 0;
}