#include "server.hpp"

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Use: " << argv[0] << " <primary or secundary>" << std::endl;
        return 1;
    }
    
    try {
        Server server(argv[1]);
        server.setup();
        server.listenConnection();    
    } catch (const std::runtime_error &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    return 0;
}