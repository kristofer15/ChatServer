#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <fstream>
#include <vector>

std::string get_dino(std::string dino_file="dino.txt") {
    std::string dino;
    std::ifstream file (dino_file);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line)) {
            dino += line + "\n";
        }
        file.close();
    }

    // Return empty vector if file could not be read
    return dino;
}
void help(std::string file_name="client") {
    std::cout << "=== Knockmaster 5000 ===" << std::endl;
    std::string dino = get_dino();
    std::cout << dino << std::endl;



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
    char* port_sequence[] = {argv[2], argv[3], argv[4]};

    struct sockaddr_in destination_in;
    struct hostent *destination;

    destination = gethostbyname(host_name);
    if(destination == NULL) {
        std::cout << "Could not find " << host_name << std::endl;
        return 0;
    }

    bzero((char *) &destination_in, sizeof(destination_in));

    destination_in.sin_family = AF_INET;

    bcopy((char *)destination->h_addr,
    (char *)&destination_in.sin_addr.s_addr,
    destination->h_length);

    int s;
    for(char* port : port_sequence) {
        s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        destination_in.sin_port = htons(atoi(port));
        connect(s, (struct sockaddr *) &destination_in, sizeof(destination_in));
        std::cout << s << port << std::endl;
    }

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