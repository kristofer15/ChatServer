#include <sstream>
#include "settings.h"

namespace settings {

    // Kind of like a javascript closure
    // Absolute nonsense that is.
    namespace {

        // Classless privates
        std::string ID = "";
        std::map<std::string, int> users = {};
        std::vector<int> connected_sockets = {};
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

        id << time(NULL) << std::endl << "Group x";
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

    std::vector<int>& get_connected_sockets() {
        return connected_sockets;
    }
};
