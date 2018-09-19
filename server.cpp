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
    int sock, other_sock;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval t;
    fd_set socks;

    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);


    bzero((char *) &serv_addr, sizeof(serv_addr));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sock  , (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Failed to bind" << std::endl;
        exit(0);
    }

    if(listen(sock, 1) < 0) {
        std::cout << "Failed to listen" << std::endl;
        exit(0);
    }

    /* Initialize the set of active sockets. */
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    t.tv_sec = 10;

    while(true) {
        std::cout << "Socket: " << sock << std::endl;
        
        if(select(sock+1, &socks, NULL, NULL, NULL) < 0) {
            return 0;
        }

        if(FD_ISSET(sock, &socks)) {
            std::cout << "Socket received a connection" << std::endl;
            clilen = sizeof(cli_addr);
            other_sock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);

            if(other_sock < 0) {
                std::cout << "Failed to accept" << std::endl;
            }
            else {
                std::cout << "Accept successful" << std::endl;
            }
        }

    }
}