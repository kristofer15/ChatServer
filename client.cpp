#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <fstream>
#include <vector>

#include "file_io.h"
#include "knocker.h"


void help(std::string file_name="client") {
    std::cout << "=== Knockmaster 5000 ===" << std::endl;
    std::cout << read_file("dino.txt") << std::endl;

    std::cout << "Knock a sequence of ports on a specified server" << std::endl
                << "Communicate once connection has been established" << std::endl
                << "Usage:" << std::endl
                << file_name << " {SERVER NAME} {FIRST PORT} {SECOND PORT} {THIRD PORT}" << std::endl;
}

int main(int argc, char* argv[]) {

    if(argc < 5) {
        help(argv[0]);
        return 0;
    }

    char* host_name = argv[1];
    int port_sequence[3] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};

    Knocker k(host_name, port_sequence);
    int s = k.knock();

    if(s < 1) { 
        std::cout << "Could not connect to server" << std::endl;
        return -1;
    }

    std::cout << "Socket connected" << std::endl;

    struct timeval t;
    t.tv_sec = 100;

    fd_set socks;
    FD_ZERO(&socks);
    while(true) {
        FD_SET(s, &socks);
        FD_SET(STDIN_FILENO, &socks);

        int response = select(s+1, &socks, NULL, NULL, &t);

        if(FD_ISSET(STDIN_FILENO, &socks)) {
            std::string message;
            std::getline(std::cin, message);
            write(s, message.c_str(), message.length());  
        }

        if(FD_ISSET(s, &socks)) {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));

            if(read(s, buffer, 255)) {
                std::cout << buffer << std::endl;
            }
            else {
                std::cout << "server closed the connection" << std::endl;
                return 0;
            }
        }
    }
}