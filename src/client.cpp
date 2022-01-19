#include "client.hpp"

Client::Client(std::string username, std::string serverIp, std::string port)
{
    if (!std::regex_match(username, std::regex(NAME_FORMAT))) {
        throw std::invalid_argument("invalid username");
    }
    if (!std::regex_match(serverIp, std::regex(IP_FORMAT))) {
        std::cout << serverIp;
        throw std::invalid_argument("invalid ip address");
    }
    if (!std::regex_match(port, std::regex(PORT_FORMAT))) {
        throw std::invalid_argument("invalid port");
    }

    this->username = username;
    this->server_ip = serverIp;
    this->port = stoi(port);
}

Client::~Client()
{
    if (this->server_socket > 0) {
        close(this->server_socket);
    }
}

void Client::getData(void *arg, bool* exitThread)
{
    int n, socket = *((int *)arg);

    while (*exitThread==false) {
        std::vector<char> buffer = std::vector<char>(4096);
        n = recv(socket, &buffer[0], buffer.size(),0);

        if (n==-1) {
            std::cout << "error reading socket from server";
        }

        std::string received(buffer.begin(), buffer.end());
        received.erase(std::find(received.begin(), received.end(), '\0'), received.end());

        if (!received.empty()) {
            std::cout << received << std::endl;

            if (received=="exiting") {
                *exitThread=true;
            }
        }
    }

    close(socket);
}

void Client::frontend(void *arg, bool* exitThread, struct sockaddr_in newserver_address)
{
     int socket = *((int *)arg);
     socklen_t newserver_len;

    if (listen(socket, 5) < 0) {
        std::cerr << "listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (true) {
        accept(socket, (struct sockaddr *)&newserver_address, &newserver_len);
    }
}

    std::tuple<int, int> Client::openClientSocket()
{
    socklen_t len = sizeof(client_address);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = 0;

    int opt = 1;
    if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (bind(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) < 0) {
        std::cerr << "bind failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    getsockname(client_socket, (struct sockaddr *)&client_address, &len); 
    return std::make_tuple(client_address.sin_port, client_socket);
}


void Client::setup()
{
    bool exitThread=false;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);
    server_address.sin_addr.s_addr = inet_addr(this->server_ip.c_str());
    bzero(&(server_address.sin_zero), 8);

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection Failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string message = "client";
    send(server_socket, message.c_str(), message.size(), 0);

    std::tie(localPort, client_socket) = openClientSocket();

    message=std::to_string(localPort);
    send(server_socket, message.c_str(), message.size(), 0);

    std::thread frontendThread (frontend, &client_socket, &exitThread, newserver_address);
    frontendThread.detach();

    message = this->username;
    send(server_socket, message.c_str(), message.size(), 0);

    std::thread outputThread (getData, &server_socket, &exitThread);
    outputThread.detach();

    while ((exitThread == false) && (message.find(EXIT_CMD))) {
        getline(std::cin, message);

        if (!message.find(SEND_CMD)) {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream timestr;
            timestr << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
            std::string newmessage = message + " " + timestr.str();
            message = newmessage;
        }

        if (!message.find(EXIT_CMD)) {
            std::cout << "client closing connection" << std::endl;
        }

        send(server_socket, message.c_str(), message.size(), 0);
    }

    if (outputThread.joinable()) {
        outputThread.join();
    }
}
