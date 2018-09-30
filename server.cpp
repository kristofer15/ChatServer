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
#define KNOCKING_TIMEOUT 120

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

void disconnect_user(int socket) {
    // if socket exists in users, remove
    std::string username = settings::get_client_sockets()[socket];
    settings::get_users().erase(username);

    // remove socket from client_socket map
    settings::get_client_sockets().erase(socket);

    // clear socket from socket_set
    FD_CLR(socket, &settings::get_socket_set());
        
    // close socket
    closeSocket(socket);

    std::cout << "User " << username << " disconnected" << std::endl;
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

// Get word #from in str
// Optionally return multiple words
std::string get_word(std::string str, int from, int count=1, char delimiter=' ') {
    int start = 0;
    int end = 0;
    int word = 0;

    for(int i = 0; i < str.length(); ++i) {
        end = i;

        if(str[i] == delimiter) {
            ++word;
            if(from == word)  { start = i+1; };
            if((from+count) == word) { break; }
        }
    }

    if(start < from) { return ""; }
    if(end == str.length()-1) { ++end; }

    return str.substr(start, end-start);
}

std::string parse_command(int client_sock, std::string command) {

    // .compare returns 0 with identical strings
    if(command.compare("ID") == 0) {
        return settings::get_id() + "\n";
    }

    if(command.compare("CHANGE ID") == 0) {
        settings::set_new_id();
        return "Set new id: " + settings::get_id() + "\n";
    }

    // Starts with "CONNECT "
    if(command.compare(0, 8, "CONNECT ") == 0) {

        // Only accept commands containing a single space
        if(std::count(command.begin(), command.end(), ' ') == 1) {
            std::string user = command.substr(command.rfind(" ") + 1);

            if(user.compare("ALL") == 0) {
                return "Stop that";
            }

            // create new user and update client_socket map
            settings::get_users().insert(std::make_pair(user, client_sock));
            settings::get_client_sockets()[client_sock] = user;

            return user + " connected\n";
        }
    }

    if(command.compare("WHO") == 0) {
        std::stringstream connectedUsers;
        connectedUsers << "Connected users: \n";

        for(auto pair : settings::get_users()) {
            connectedUsers << pair.first + "\n";
        }

        return connectedUsers.str();
    }

    if(command.compare(0, 4, "MSG ") == 0) {
        int words = std::count(command.begin(), command.end(), ' ');

        if(words > 1) {
            std::string user = get_word(command, 1);
            std::string message = get_word(command, 2, 999) + "\n" ;

            if(user.compare("ALL") == 0) {
                for(auto name_socket : settings::get_users()) {
                    write(name_socket.second, message.c_str(), message.length());
                }

                return "Messaged all users\n";
            }

            if(settings::get_users().find(user) != settings::get_users().end()) {
                write(settings::get_users()[user], message.c_str(), message.length());

                return "Messaged " + user + '\n';
            }
            else {
                return "No such user\n";
            }
        }
    }

    if(command.compare("LEAVE") == 0) {
        disconnect_user(client_sock);
    }

    return "Your answer: " + command + "\nCorrect answer: " + command + "\n";
}

void setup_server_socks() {
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
        settings::get_top_socket() = std::max(settings::get_top_socket(), sock);

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

void reset_socket_set() {
    // clear sock_set
    FD_ZERO(&settings::get_socket_set());

    // reset client sockets
    for(std::pair<int, std::string> client : settings::get_client_sockets()) {
        // if valid file descriptor add to set
        int socket = client.first;
        if(socket > 0) {
            FD_SET(socket, &settings::get_socket_set());
        }
    }

    // reset server sockets
    for(int socket : settings::get_server_sockets()) {
        // if valid file descriptor add to set
        if(socket > 0) {
            FD_SET(socket, &settings::get_socket_set());
        }
    }
}

void respond_to_knock(int receiving_socket) {
    /*
     * Responds to knock on receiving server socket.
     * Updates get_knock_status for knocking IP.
     * If get_knock_status completed, update sock_set and top_socket, 
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
        settings::get_knock_status()[client_address] = std::make_pair(1,time(NULL));
        
        std::cout << "IP " << client_address << " knocked on port " << PORT << std::endl;
        closeSocket(client_sock);
    }

    // knock on second port in series
    if(receiving_socket == settings::get_server_sockets()[1]) {
        if(settings::get_knock_status()[client_address].first == 1) {
            settings::get_knock_status()[client_address] = std::make_pair(2, time(NULL));
        }
        else { // reset counter or create pair             
            settings::get_knock_status()[client_address] = std::make_pair(0, time(NULL));
        }
        
        std::cout << "IP " << client_address << " knocked on port " << PORT+1 << std::endl;
        closeSocket(client_sock);
    }

    // knock on third port in series
    if(receiving_socket == settings::get_server_sockets()[2]) {

        // if second knock completed within timerange
        time_t then = settings::get_knock_status()[client_address].second;
        time_t t_diff = difftime(time(NULL), then);
        if(settings::get_knock_status()[client_address].first == 2 &&  t_diff < KNOCKING_TIMEOUT) {
            // add new socket to map and update top_socket
            settings::get_client_sockets()[client_sock] = "anon";
            settings::get_top_socket() = std::max(settings::get_top_socket(), client_sock);      

            std::cout << "IP " << client_address << " connected" << std::endl;

            // knock completed, remove from map
            settings::get_knock_status().erase(client_address); 

        }
        else {
            //settings::get_get_knock_status()[client_address] = 0;
            settings::get_knock_status()[client_address] = std::make_pair(0, time(NULL));

            std::cout << "IP " << client_address << " knocked on port " << PORT+2 << std::endl;
            closeSocket(client_sock);
        }
    }
}

void respond_to_command(int client_socket, std::string username, char* buffer) {
    //clear buffer
    bzero(buffer,256);
    // zero indicates end of file. AKA client disconnected
    if(read(client_socket, buffer, 255) == 0) {
        disconnect_user(client_socket);
    }
    else {
        std::string response = parse_command(client_socket, trim_newline(buffer));
        write(client_socket, response.c_str(), response.length());  
    }
}

int main(int argc, char* argv[]) {
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    struct timeval t;
    char buffer[256];

    setup_server_socks();

    t.tv_sec = 10;

    while(true) {

        // reset socket set for further commands/connections
        reset_socket_set();
        
        std::cout << "Waiting for activity on sockets" << std::endl;
        if(select(settings::get_top_socket()+1, &settings::get_socket_set(), NULL, NULL, NULL) < 0) {
            error("Select failed");
        }

        // connection to one of the server sockets
        for(int server_socket : settings::get_server_sockets()) {
            if(FD_ISSET(server_socket, &settings::get_socket_set())) {
                respond_to_knock(server_socket);
            }
        }

        int client_socket = 0;
        std::string username = "";
        // find client that sent command
        for(std::pair<int, std::string> client : settings::get_client_sockets()) {
            if(FD_ISSET(client.first, &settings::get_socket_set())) {        
                client_socket = client.first;
                username = client.second;
            }
        }

        // respond to command
        if(FD_ISSET(client_socket, &settings::get_socket_set())) {
            respond_to_command(client_socket, username, buffer);
        }
        
    }
}