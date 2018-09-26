#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <algorithm>
#include <map>

#include <iostream>
#include <sstream>

#include "settings.h"

#define PORT    5555
#define MAXMSG  512


// Nabbed from: https://www.gnu.org/software/libc/manual/html_node/Server-Example.html
int read_from_client(int filedes) {
    char buffer[MAXMSG];
    int nbytes;

    nbytes = read(filedes, buffer, MAXMSG);
    if (nbytes < 0) {
        /* Read error. */
        perror("read");
        exit(EXIT_FAILURE);
        }
    else if(nbytes == 0) {
        /* End-of-file. */
        return -1;
    }
    else {
        /* Data read. */
        fprintf(stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }
}

std::string parse_message(int client_sock, std::string message) {

    // .compare returns 0 with identical strings
    if(message.compare("ID") == 0) {
        return settings::get_id() + "\n";
    }

    if(message.compare("CHANGE ID") == 0) {
        settings::set_new_id();
        return "Set new id: " + settings::get_id() + "\n";
    }

    // Starts with "CONNECT "
    if(message.compare(0, 8, "CONNECT ") == 0) {

        // Only accept connections containing a single space
        if(std::count(message.begin(), message.end(), ' ') == 1) {
            std::string user = message.substr(message.rfind(" ") + 1);

            if(user.compare("ALL") == 0) {
                return "Stop that";
            }

            settings::get_users().insert(std::make_pair(user, client_sock));

            return user + " connected\n";
        }
    }

    if(message.compare("WHO") == 0) {
        std::stringstream connectedUsers;
        connectedUsers << "Connected users: \n";

        for(auto pair : settings::get_users()) {
            connectedUsers << pair.first + "\n";
        }

        return connectedUsers.str();
    }

    if(message.compare(0, 4, "MSG ") == 0) {
        if(std::count(message.begin(), message.end(), ' ') == 1) {
            std::string user = message.substr(message.rfind(" ") + 1);

            if(settings::get_users().find(user) != settings::get_users().end()) {
                write(settings::get_users()[user], message.c_str(), message.length());
                return "Message to: " + user + "\n";
            }
            else {
                return "No such user\n";
            }
        }
    }

    return "Your answer: " + message + "\nCorrect answer: " + message + "\n";
}

std::string trim_newline(std::string s) {
    std::string trimmed = s;
    while(trimmed[trimmed.length()-1] == '\n') {
        trimmed.erase(trimmed.length()-1);
    }

    return trimmed;
}

void close_socket(int fd) {

    if (fd >= 0) {
        if (shutdown(fd, SHUT_RDWR) < 0) { // terminate the 'reliable' delivery
            std::cout << "Failed to shutdown client socket" << std::endl;
        } 
    
        if (close(fd) < 0) { // finally call close()
            std::cout << "Failed to close client socket" << std::endl;  
        }

        std::cout << "done closing" << std::endl;

    }
}

void error(const char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[]) {
    int client_sock;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    struct timeval t;
    char buffer[256];

    fd_set sock_set;

    int socks[3];
    int top_sock = 0;
    FD_ZERO(&sock_set);

    for(int i = 0; i < 3; ++i) {
        struct sockaddr_in serv_addr;
        int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sock < 0) {
            error("Unable to open socket");
        }

        top_sock = std::max(top_sock, sock);
        socks[i] = sock;

        bzero((char *) &serv_addr, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(PORT + i);

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &serv_addr, sizeof(int)) == -1) {
            exit(0);
        }

        if(bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            error("Failed to bind");
        }

        if(listen(sock, 1) < 0) {
            error("Failed to listen");
        }

        FD_SET(sock, &sock_set);
    }

    t.tv_sec = 10;

    while(true) {

        if(select(top_sock+1, &sock_set, NULL, NULL, NULL) < 0) {
            error("Select failed");
        }

        for(int i = 0; i < 3; ++i) {
            int sock = socks[i];

            if(FD_ISSET(sock, &sock_set)) {
                clilen = sizeof(cli_addr);

                client_sock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);

                if(client_sock < 0) {
                    std::cout << "Failed to accept" << std::endl;
                }
                else {
                    std::cout << "Connection from: " << inet_ntoa(cli_addr.sin_addr) << " through port: " << ntohs(cli_addr.sin_port) << std::endl;

                    bzero(buffer,256);

                    int n = read(client_sock, buffer, 255);

                    if(n > 0) {
                        std::cout << "Received message" << std::endl;
                        std::string response = parse_message(client_sock, trim_newline(buffer));
                        write(client_sock, response.c_str(), response.length());
                        
                    }

                    FD_SET(client_sock, &sock_set);
                    // close_socket(client_sock);
                }
            }
            else {
                
                // Reset the sock
                FD_SET(sock, &sock_set);
            }
        }

    }

    close(client_sock);
}