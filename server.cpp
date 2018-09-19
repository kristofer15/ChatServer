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
#include <string.h>

#include <iostream>

#define PORT    5555
#define MAXMSG  512

// Nabbed from: https://www.gnu.org/software/libc/manual/html_node/Server-Example.html
int read_from_client (int filedes) {
    char buffer[MAXMSG];
    int nbytes;

    nbytes = read (filedes, buffer, MAXMSG);
    if (nbytes < 0) {
        /* Read error. */
        perror ("read");
        exit (EXIT_FAILURE);
        }
    else if (nbytes == 0) {
        /* End-of-file. */
        return -1;
    }
    else {
        /* Data read. */
        fprintf (stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }
}

int main(int argc, char* argv[]) {
    int other_sock;
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
        top_sock = std::max(top_sock, sock);
        socks[i] = sock;

        bzero((char *) &serv_addr, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(PORT + i);

        if(bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            std::cout << "Failed to bind" << std::endl;
            exit(0);
        }

        if(listen(sock, 1) < 0) {
            std::cout << "Failed to listen" << std::endl;
            exit(0);
        }

        FD_SET(sock, &sock_set);
    }

    t.tv_sec = 10;

    while(true) {

        std::cout << std::endl;
        std::cout << "Selecting" << std::endl;
        std::cout << "Top sock: " << top_sock << std::endl;
        
        if(select(top_sock+1, &sock_set, NULL, NULL, NULL) < 0) {
            return 0;
        }

        for(int i = 0; i < 3; ++i) {
            int sock = socks[i];
            
            std::cout << "Socket: " << sock << std::endl;

            if(FD_ISSET(sock, &sock_set)) {
                std::cout << "Socket received a connection" << std::endl;
                clilen = sizeof(cli_addr);

                other_sock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);

                if(other_sock < 0) {
                    std::cout << "Failed to accept" << std::endl;
                }
                else {
                    std::cout << "Accept successful" << std::endl;


                    bzero(buffer,256);

                    int n = read(other_sock, buffer, 255);

                    std::cout << "Received " << n << " bytes" << std::endl;

                    if(n > 0) {
                        std::cout << "Incoming message: " << buffer << std::endl;
                    }
                }
            }
            else {
                std::cout << "Nothing on " << sock << std::endl;
            // Reset the sock
            FD_SET(sock, &sock_set);
            }
        }

    }

    close(other_sock);
}