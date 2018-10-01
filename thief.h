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
    char response[256];

    memset(response, 0, sizeof(response));

    struct timeval t;
    t.tv_sec = 3;

    fd_set socks;
    std::string commands[2] = {"CONNECT TOLVUTHRJOTUR", "ID"};    
    for(std::string command : commands) {
        send(sock, command.c_str(), command.length(), MSG_NOSIGNAL);

        FD_ZERO(&socks);
        FD_SET(sock, &socks);
        select(sock+1, &socks, NULL, NULL, &t);

        if(FD_ISSET(sock, &socks)) {
            memset(response, 0, sizeof(response));
            int response_length = read(sock, response, 255);

            if(command.compare("ID") == 0) {
                std::cout << response << std::endl;

                command = "CHANGE ID";
                send(sock, command.c_str(), command.length(), MSG_NOSIGNAL);
                return response;
            }
        }
    }

    return "";
}

// Brute force given port numbers and log any response from get_id() that looks like a message
void steal() {

    // Ignore broken pipes
    signal (SIGPIPE, SIG_IGN);

    File_io io;
    Knocker knocker;

    // Ports are added to the file manually
    std::vector<int> ports = settings::get_io().get_lines<int>("ports.txt");
    std::string host = "localhost";

    std::cout << "Stealing IDs" << std::endl;

    // Assume that groups actually want to be found and
    // only knock ports in ascending orde
    for(int port1 : ports) {
        for(int port2 : ports) {
            for(int port3 : ports) {

                if(port1 == port2 || port2 == port3 || port1 == port3) { continue; }

                // Wait for 10ms to avoid stack smashing
                // usleep(10000);

                int ports[3] = {port1, port2, port3};
                int sock = knocker.knock(host.c_str(), ports);

                // Knock returned a seemingly valid socket
                if(sock < 1) { continue; }

                std::cout << "Connected to port sequence " << port1 << " " << port2 << " " << port3 << std::endl;

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
