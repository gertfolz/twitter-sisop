#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <regex>
#include <chrono>
#include <ctime>   
#include <sstream> 
#include <iomanip> 
#include <tuple>

#define NAME_FORMAT "[A-Za-z][A-Za-z0-9\\.]{3,19}" // Qual tamanho maximo e meninimo de um username??
#define SEND_CMD "SEND "
#define FOLLLOW_CMD "FOLLOW @"
#define EXIT_CMD "EXIT"

#define IP_FORMAT "([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})"
#define PORT_FORMAT "[0-9]{1,5}"

class Client
{
private:
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address, newserver_address;
    int localPort;
    std::string username;
    std::string server_ip;
    std::string localhost = "127.0.0.1";
    int port;
   
    std::string message;

public:
    Client(std::string username, std::string serverIp, std::string port);
    ~Client();
    std::tuple<int, int> openClientSocket();
    static void frontend(void *arg, bool* , struct sockaddr_in);
    static void getData(void *arg, bool*);
    void setup();
};

#endif
