#include "settings.h"

namespace settings {

    // Kind of like a javascript closure
    // Absolute nonsense that is.
    namespace {

        // Classless privates
        std::string ID = "";

        // User name - fd
        std::map<std::string, int> users = {};

        // fd - username
        std::map<int, std::string> client_sockets = {};
        std::vector<int> server_sockets = {};

        // IP - <status,timestamp>
        std::map<std::string, std::pair<int, time_t>> knock_status = {};

        fd_set socket_set;
        int top_socket;

        int server_ports[3] = {34532, 34533, 34534};

        File_io io;
    };

    // Yoinked from: https://stackoverflow.com/questions/44610978/popen-writes-output-of-command-executed-to-cout
    void set_new_id() {
        std::stringstream id;
        char buffer[128];

        FILE* pipe = popen("fortune -s", "r");
        if(!pipe) {
            std::cout << "Failed to open pipe" << std::endl;
            exit(0);
        }
        
        while (fgets(buffer, 128, pipe) != NULL) {
            id << buffer;
        }

        id << time(NULL) << std::endl << "Group 49";
        ID = id.str();
    }

    // Singleton
    std::string get_id() {
        if(ID == "") { set_new_id(); }
        return ID;
    }

    std::map<std::string, int>& get_users() {
        return users;
    }

    std::map<int, std::string>& get_client_sockets() {
        return client_sockets;
    }

    std::vector<int>& get_server_sockets() {
        return server_sockets;
    }

    std::map<std::string, std::pair<int, time_t>>& get_knock_status() {
        return knock_status;
    }

    fd_set& get_socket_set() {
        return socket_set;
    }

    int& get_top_socket() {
        return top_socket;
    }

    int* get_server_ports() {
        return server_ports;
    }

    File_io get_io() {
        return io;
    }
};
