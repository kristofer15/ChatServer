#include "settings.h"

namespace settings {

    // Kind of like a javascript closure
    // Absolute nonsense that is.
    namespace { std::string ID = ""; };

    // Yoinked from: https://stackoverflow.com/questions/44610978/popen-writes-output-of-command-executed-to-cout
    void set_new_id() {
        std::string id = "";
        char buffer[128];

        FILE* pipe = popen("fortune -s", "r");
        if(!pipe) {
            std::cout << "Failed to open pipe" << std::endl;
            exit(0);
        }
        
        while (fgets(buffer, 128, pipe) != NULL) {
            id += buffer;
        }

        ID = id += "\n-Mr.Andersen";
    }

    // Singleton
    std::string get_id() {
        if(ID == "") { set_new_id(); }
        return ID;
    }
};
