#include "client.hpp"

int main(int argc, char **argv)
{
    if (argc < 4) {
        std::cerr << "Use: " << argv[0] << " <username> <server_address> <port>" << std::endl;
        return 1;
    }

    try {
        Client client(argv[1], argv[2], argv[3]);
        client.setup();
    }
    catch (const std::invalid_argument &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
    catch (const std::runtime_error &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    return 0;
}