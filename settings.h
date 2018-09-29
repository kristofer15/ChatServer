#include <string.h>
#include <map>
#include <iostream>
#include <vector>

namespace settings {
    void set_new_id();
    std::string get_id();
    std::map<std::string, int>& get_knock_status();
    std::map<std::string, int>& get_users();
    std::vector<int>& get_server_sockets();
    std::vector<int>& get_anon_sockets();
};