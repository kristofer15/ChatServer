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
        Knocker() { }
        
        int knock(const char* host, int ports[3]) {

            struct sockaddr_in destination_in;
            struct hostent *destination;

            destination = gethostbyname(host);
            if(destination == NULL) {
                std::cout << "Could not find " << host << std::endl;
                return -1;
            }

            bzero((char *) &destination_in, sizeof(destination_in));

            destination_in.sin_family = AF_INET;

            bcopy((char *)destination->h_addr,
            (char *)&destination_in.sin_addr.s_addr,
            destination->h_length);

            int s;
            for(int i = 0; i < 3; i++) {
                int port = ports[i];
                s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
                destination_in.sin_port = htons(port);
                connect(s, (struct sockaddr *) &destination_in, sizeof(destination_in));
                
                int error_code;
                socklen_t error_code_size = sizeof(error_code);
                getsockopt(s, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
                
                // Socket is not connected
                if(error_code == 111) {
                    return -1;
                }
            }

            return s;
        }

    private:
};