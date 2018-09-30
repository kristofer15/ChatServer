#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <fstream>
#include <vector>

class Knocker {
    public:
        Knocker(char* h, int p[3]) {
            host = h;

            // C++ methods are being obtuse as usual.
            // Just copy manually
            for(int i = 0; i < 3; ++i) {
                ports[i] = p[i];
            }
        }
        
        int knock() {

            struct sockaddr_in destination_in;
            struct hostent *destination;

            destination = gethostbyname(this->host);
            if(destination == NULL) {
                std::cout << "Could not find " << this->host << std::endl;
                return -1;
            }

            bzero((char *) &destination_in, sizeof(destination_in));

            destination_in.sin_family = AF_INET;

            bcopy((char *)destination->h_addr,
            (char *)&destination_in.sin_addr.s_addr,
            destination->h_length);

            int s;
            for(int port : ports) {
                s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
                destination_in.sin_port = htons(port);
                connect(s, (struct sockaddr *) &destination_in, sizeof(destination_in));
            }

            return s;
        }

    private:
        char* host;
        int ports[3];
};