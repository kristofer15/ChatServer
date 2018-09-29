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
#include <vector>

#include <iostream>
#include <sstream>

#include "settings.h"

#define PORT    5555
#define MAXMSG  512

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void closeSocket(int fd) {
    if (fd >= 0) {
        if (shutdown(fd, SHUT_RDWR) < 0) { // terminate the 'reliable' delivery
            error("Unable to shutdown socket");
        }
    
        if (close(fd) < 0) { // finally call close()
            error("Unable to close socket");
        }

        std::cout << "Socket: " << fd << " closed" << std::endl;
    }
}

std::string trim_newline(std::string s) {
    std::string trimmed = s;
    while(trimmed[trimmed.length()-1] == '\n') {
        trimmed.erase(trimmed.length()-1);
    }

    return trimmed;
}

void remove_socket(int socket, fd_set& sock_set) {
    // if socket exists in users, remove
    std::string username = settings::get_client_sockets()[socket];
    settings::get_users().erase(username);

    // remove socket from client_socket map
    settings::get_client_sockets().erase(socket);

    // clear socket from socket_set
    FD_CLR(socket, &sock_set);

    // close socket
    closeSocket(socket);
}

std::string parse_message(int client_sock, fd_set& sock_set, std::string message) {

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

            // create new user and update client_socket map
            settings::get_users().insert(std::make_pair(user, client_sock));
            settings::get_client_sockets()[client_sock] = user;

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
            std::string user = message.substr(message.rfind(' ') + 1);

            if(user.compare("ALL") == 0) {
                for(auto name_socket : settings::get_users()) {
                    write(name_socket.second, (message + '\n').c_str(), message.length()+1);
                    return "Messaged all users" + '\n';
                }
            }

            if(settings::get_users().find(user) != settings::get_users().end()) {
                write(settings::get_users()[user], (message + '\n').c_str(), message.length()+1);
                return "Message to: " + user + '\n';
            }
            else {
                return "No such user\n";
            }
        }
    }

    if(message.compare("LEAVE") == 0) {
        remove_socket(client_sock, sock_set);
    }

    return "Your answer: " + message + "\nCorrect answer: " + message + "\n";
}

void setup_server_socks(int& top_sock) {
    /*
     * Creates 3 server sockets open for connections.
     * Returns updated top socket index
     */

    for(int i = 0; i < 3; ++i) {
        struct sockaddr_in serv_addr;
        int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sock < 0) {
            error("Unable to open socket");
        }

        // make socket port reusable
        int enable = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
            error("setsockopt(SO_REUSEADDR) failed");
        }           

        // update top socket index
        top_sock = std::max(top_sock, sock);

        // add newly created socket to list of server sockets
        settings::get_server_sockets().push_back(sock);

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(PORT + i);

        if(bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            error("Failed to bind");
        }

        if(listen(sock, 1) < 0) {
            error("Failed to listen");
        }
    }
}

void reset_socket_set(fd_set& sock_set) {
    // clear sock_set
    FD_ZERO(&sock_set);

    // reset client sockets
    for(std::pair<int, std::string> client : settings::get_client_sockets()) {
        // if valid file descriptor add to set
        int socket = client.first;
        if(socket > 0) {
            FD_SET(socket, &sock_set);
        }
    }

    // reset server sockets
    for(int sock : settings::get_server_sockets()) {
        // if valid file descriptor add to set
        if(sock > 0) {
            FD_SET(sock, &sock_set);
        }
    }
}

void respond_to_knock(int receiving_socket, int& top_sock, fd_set& sock_set) {
    /*
     * Responds to knock on receiving server socket.
     * Updates knock_status for knocking IP.
     * If knock_status completed, update sock_set and top_socket, 
     * and add new socket to client_socket map with username 'anon'.
     */ 

    socklen_t clilen;
    struct sockaddr_in cli_addr;
    // clear client address info
    bzero((char *) &cli_addr, sizeof(cli_addr));
    clilen = sizeof(cli_addr);

    int client_sock = accept(receiving_socket, (struct sockaddr *) &cli_addr, &clilen);
    if(client_sock < 0) {
        error("Failed to accept");
    }
    std::string client_address = inet_ntoa(cli_addr.sin_addr);

    // kock on first port in series
    if(receiving_socket == settings::get_server_sockets()[0]) {
        // first knock completed
        settings::get_knock_status()[client_address] = 1;
        
        std::cout << "IP " << client_address << " knocked on port " << PORT << std::endl;
        closeSocket(client_sock);
    }

    // knock on second port in series
    if(receiving_socket == settings::get_server_sockets()[1]) {
        // if key exists and first knock completed
        if(settings::get_knock_status()[client_address] == 1) {
            settings::get_knock_status()[client_address] = 2;
        }
        else { // reset counter or create pair             
            settings::get_knock_status()[client_address] = 0;
        }
        
        std::cout << "IP " << client_address << " knocked on port " << PORT+1 << std::endl;
        closeSocket(client_sock);
    }

    // knock on third port in series
    if(receiving_socket == settings::get_server_sockets()[2]) {
        // if key exists and second knock completed
        if(settings::get_knock_status()[client_address] == 2) {
            // add new socket to map and update top_socket
            settings::get_client_sockets()[client_sock] = "anon";
            top_sock = std::max(top_sock, client_sock);      

            std::cout << "IP " << client_address << " connected" << std::endl;

            // knock completed, remove from map
            settings::get_knock_status().erase(client_address); 
        }
        else {
            settings::get_knock_status()[client_address] = 0;

            std::cout << "IP " << client_address << " knocked on port " << PORT+2 << std::endl;
            closeSocket(client_sock);
        }
    }
}

int main(int argc, char* argv[]) {
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    struct timeval t;
    char buffer[256];

    fd_set sock_set; 
    int top_sock = 0;

    setup_server_socks(top_sock);

    t.tv_sec = 10;

    while(true) {

        // reset socket set for further messages/connections
        reset_socket_set(sock_set);
        
        std::cout << "Wating for activity on sockets" << std::endl;
        if(select(top_sock+1, &sock_set, NULL, NULL, NULL) < 0) {
            error("Select failed");
        }

        // connection to one of the server sockets
        for(int server_socket : settings::get_server_sockets()) {
            if(FD_ISSET(server_socket, &sock_set)) {
                respond_to_knock(server_socket, top_sock, sock_set);
            }
        }

        // message from user without username
        for(std::pair<int, std::string> client : settings::get_client_sockets()) {
            int client_socket = client.first;

            if(FD_ISSET(client_socket, &sock_set)) {
                //clear buffer
                bzero(buffer,256);
                // zero indicates end of file. AKA client disconnected
                if(read(client_socket, buffer, 255) == 0) {
                    remove_socket(client_socket, sock_set);
                    std::cout << "User " << client.second << " disconnected" << std::endl;
                }
                else {
                    std::string response = parse_message(client_socket, sock_set, trim_newline(buffer));
                    write(client_socket, response.c_str(), response.length());  
                }
            }
        }
    }
}