#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <iostream>
#include <regex>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <utility>
#include <queue>
#include <unordered_map>
#include <chrono>

#include "database.hpp"
#include "message.hpp"
#include "notification.hpp"
#include "follow.hpp"
#include "user.hpp"

#define NAME_FORMAT "[A-Za-z][A-Za-z0-9\\.]{3,19}" // Qual tamanho maximo e meninimo de um username??
#define SEND_CMD "SEND "
#define FOLLLOW_CMD "FOLLOW @"
#define EXIT_CMD "EXIT"

typedef std::pair<std::string, std::vector<std::string> > CommandResponse;

class Server
{
private:
    int client_socket, server_socket, backup_socket;
    int leaderPort = 6789, thisPort;
    std::string localhost = "127.0.0.1";
    int isLeader;
    struct sockaddr_in client_address, server_address, backup_address;
    std::vector<int> connectedClientPort;
    std::vector<int> connectedServersPort;
    static Database db;
    static std::unordered_map<int, std::queue<std::string> > queriesToBackup;
public:
    static std::map<std::string, int> sessions;
    Server(std::string);
    ~Server();
    void backupServer();
    void setup();
    void listenConnection();
    int openBackupSocket();
    static void heartbeat(void *arg);
    static void backup(void *arg, std::vector<int>&);
    static void getData(void *arg, std::vector<int>&);
    static void getNotifications(void *arg, std::string userId, bool*);
    static CommandResponse handleCommand(std::string command, User user, bool*);
};

#endif
