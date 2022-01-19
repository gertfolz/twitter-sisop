#include "server.hpp"

std::condition_variable cv;
std::mutex mtx;
Database Server::db;
std::unordered_map<int, std::queue<std::string> > Server::queriesToBackup;
static std::map<std::string, int> sessions;

Server::Server(std::string isLeader)
{
    this->isLeader=stoi(isLeader);
}

Server::~Server()
{
    close(this->server_socket);
    close(this->client_socket);
}

void Server::setup()
{
    if (this->isLeader) {
        thisPort=6789;
        std::cout << "its the leader !" << std::endl;
        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "socket failed" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(thisPort);

        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "setsockopt failed" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cerr << "setsockopt failed" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            std::cerr << "bind failed" << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        std::cout << "its not the leader !" << std::endl;
        backupServer();
    }
}

int Server::openBackupSocket()
{
    socklen_t len = sizeof(backup_address);
    if ((backup_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    backup_address.sin_family = AF_INET;
    backup_address.sin_addr.s_addr = INADDR_ANY;
    backup_address.sin_port = 0;

    int opt = 1;
    if (setsockopt(backup_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (setsockopt(backup_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (bind(backup_socket, (struct sockaddr *)&backup_address, sizeof(backup_address)) < 0) {
        std::cerr << "bind failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    getsockname(backup_socket, (struct sockaddr *)&backup_address, &len);
    return backup_address.sin_port;
}

void Server::backupServer()
{
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->leaderPort);
    server_address.sin_addr.s_addr = inet_addr(this->localhost.c_str());
    bzero(&(server_address.sin_zero), 8);

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection Failed" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string message = "server";
    send(server_socket, message.c_str(), message.size(), 0);

    thisPort=openBackupSocket();
    message=std::to_string(thisPort);
    std::cout << message << std::endl;
    send(server_socket, message.c_str(), message.size(), 0);

    std::thread heartbeatThread(heartbeat, &server_socket);
    heartbeatThread.detach();

    while (true) { }
}

void Server::heartbeat(void *arg)
{
    bool dead = false;
    int n, socket = *((int *)arg);

    while (!dead) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::vector<char> buffer = std::vector<char>(4096);
        n = recv(socket, &buffer[0], buffer.size(), 0);

        std::string received(buffer.begin(), buffer.end());
        received.erase(std::find(received.begin(), received.end(), '\0'), received.end());
        std::cout << received << std::endl;

        if (n == -1) {
            std::cout << "error reading socket";
        }
        if (received.empty()) {
            dead=true;
            std::cout << "primary died" << std::endl;
        } else if (received[0] != '1') {
            bool status = Server::db.execute(received);
            std::cout << "- success: " << status << std::endl;
        }
    }
}

void Server::listenConnection()
{
    socklen_t client_len;

    if (listen(server_socket, 5) < 0) {
        std::cerr << "listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (true) {
        std::vector<char> buffer = std::vector<char>(4096);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);

        int n = recv(client_socket, &buffer[0], buffer.size(), 0);
        std::string received(buffer.begin(), buffer.end());
        received.erase(std::find(received.begin(), received.end(), '\0'), received.end());

        std::cout << received << std::endl;

        if (n == -1) {
            std::cout << "error reading socket";
            exit(EXIT_FAILURE);
        }
        
        if (received == "client") {
            std::thread clientThread(getData, &client_socket, std::ref(connectedClientPort));
            clientThread.detach();
        } else if (received == "server") {
            std::thread serverThread(backup, &client_socket, std::ref(connectedServersPort));
            serverThread.detach();
        }
    }
}

void Server::backup(void *arg, std::vector<int>& connectedServersPort)
{
    int n, socket = *((int *)arg);
    std::string message = "1";
    std::vector<char> buffer = std::vector<char>(4096);
    
    n = recv(socket, &buffer[0], buffer.size(), 0);
    if (n == -1) {
        std::cout << "error reading socket";
    }

    Server::queriesToBackup[socket] = std::queue<std::string> ();

    std::string received(buffer.begin(), buffer.end());
    received.erase(std::find(received.begin(), received.end(), '\0'), received.end());

    connectedServersPort.push_back(stoi(received));
    for (int x : connectedServersPort) {
        std::cout << x;
    }

    while (true) {  
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (Server::queriesToBackup[socket].empty()) {
            send(socket, message.c_str(), message.size(), 0);
        } else {
            std::string query = Server::queriesToBackup[socket].front();
            Server::queriesToBackup[socket].pop();
            send(socket, query.c_str(), query.size(), 0);
        }
    }
}

void Server::getData(void *arg, std::vector<int>& connectedClientPort)
{
    std::vector<char> buffer = std::vector<char>(4096);
    int n, socket = *((int *)arg);
    bool exitThread = false;

    n = recv(socket, &buffer[0], buffer.size(), 0);
    if (n == -1) {
        std::cout << "error reading socket";
    }
    
    std::string received(buffer.begin(), buffer.end());
    received.erase(std::find(received.begin(), received.end(), '\0'), received.end());
    connectedClientPort.push_back(stoi(received));

    /*std::map<std::string, int>::iterator aux;
    aux = sessions.find(received);

    if (aux != sessions.end())
    {
        sessions.insert(make_pair(received, 1));
    }
    else
    {
        if (aux->second > 2)
        {
            std::cerr << "Connection refused: max sessions" << std::endl;
            exit(1);
        }
        else
        {
            sessions.find(received)->second++;
        }
    }*/
    n = recv(socket, &buffer[0], buffer.size(), 0);
    if (n == -1) {
        std::cout << "error reading socket";
    }
    
    std::string receivedUser(buffer.begin(), buffer.end());
    received.erase(std::find(receivedUser.begin(), receivedUser.end(), '\0'), receivedUser.end());
    User user(receivedUser);
    user.login(Server::db);
    std::thread notificationThread(getNotifications, &socket, receivedUser, &exitThread);
    notificationThread.detach();

    // apenas testando se recebeu informações certas, tirar o commando dps
    std::cout << "welcome " << receivedUser << std::endl;

    while (!exitThread) {
        std::vector<char> buffer = std::vector<char>(4096);
        n = read(socket, &buffer[0], buffer.size());

        std::string received(buffer.begin(), buffer.end());
        received.erase(std::find(received.begin(), received.end(), '\0'), received.end());

        CommandResponse response = handleCommand(received, user, &exitThread);
        if (!response.first.empty()) {
            send(socket, response.first.c_str(), response.first.size(), 0);
        }
        if (!response.second.empty()) {
            for (auto it = Server::queriesToBackup.begin(); it != Server::queriesToBackup.end(); ++it) {
                for(const std::string& query : response.second) {
                    Server::queriesToBackup[it->first].push(query);
                }
            }
        }
    }

    std::cout << "closing client " << user.getId() << " connection" << std::endl;
    close(socket);

    if (notificationThread.joinable()) {
        notificationThread.join();
    }
}

void Server::getNotifications(void *arg, std::string userId, bool *exitThread)
{
    int socket = *((int *)arg);
    std::unique_lock<std::mutex> lck(mtx);

    while (!(*exitThread)) {
        try {
            std::vector<NotificationStruct> notifications = Notification::getUserNotifications(Server::db, userId);

            if (notifications.size() > 0) {
                for (std::vector<NotificationStruct>::iterator notification = notifications.begin(); notification != notifications.end(); ++notification) {
                    Message msg;

                    std::cout << notification->messageId << std::endl;

                    if (!msg.findById(Server::db, notification->messageId)) {
                        std::cout << "message not found" << std::endl;
                    
                    }
                    std::cout << msg.getText() << std::endl;
                    std::string message = msg.getText();

                    if (*exitThread==false) {
                        send(socket, message.c_str(), message.size(), 0);
                        if (Notification::setToSeen(Server::db, notification->messageId, userId.c_str()).first) {
                            std::cout << "Mensagem visualizada com sucesso" << std::endl;
                        } else {
                            std::cout << "Erro ao setar message para visualizada" << std::endl;
                        }
                    }
                }
            }

            cv.wait(lck);
        } catch (const std::exception &error) {
            std::cerr << error.what() << std::endl;
        }
    }
}

CommandResponse Server::handleCommand(std::string command, User user, bool *exitThread)
{
    std::size_t found;
    std::unique_lock<std::mutex> lck(mtx);
    CommandResponse response = std::make_pair("", std::vector<std::string> ());

    found = command.find(SEND_CMD);
    if (found == 0) {
        command.erase(0, sizeof(SEND_CMD) - 1);

        std::string timestamp = command.substr(command.size()-19, command.size()-1);
        command.erase(command.size()-19, command.size()-1);

        Message m(command, user.getId());
        QueryStatus messageCreate = m.create(Server::db);
        for(const std::string& query : messageCreate.second) {
            response.second.push_back(query);
        }

        if (!messageCreate.first) {
            response.first = "ERROR failed to create message";
            return response;
        }

        QueryStatus notificationCreate = Notification::create(Server::db, m);
        for(const std::string& query : notificationCreate.second) {
            response.second.push_back(query);
        }
        
        if (!notificationCreate.first) {
            response.first = "ERROR failed to create notifications";
            return response;
        }
        
        response.first = "message sent";
        cv.notify_all();
        return response;
    }

    found = command.find(FOLLLOW_CMD);
    if (found == 0) {
        command.erase(0, sizeof(FOLLLOW_CMD) - 1);

        Follow f(user.getId(), command);
        QueryStatus followCreate = f.create(Server::db);
        for(const std::string& query : followCreate.second) {
            response.second.push_back(query);
        }

        if (followCreate.first) {
            response.first = "@" + f.getFollowerId() + " is following @" + f.getFollowedId();
            return response;
        } else {
            response.first = "ERROR failed to create follow";
            return response;
        }
    }

    found = command.find(EXIT_CMD);
    if (found == 0) {
        *exitThread = true;
        response.first = "exiting";
        return response;
    }

    response.first = "ERROR command don\'t exist";
    return response;
}
