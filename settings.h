#include <string.h>
#include <sstream>
#include <map>
#include <iostream>
#include <vector>
#include "file_io.h"

namespace settings {
    void set_new_id();
    std::string get_id();

    std::map<std::string, int>& get_users();
    std::map<int, std::string>& get_client_sockets();
    std::map<std::string, std::pair<int, time_t>>& get_knock_status();

    std::vector<int>& get_server_sockets();
    
    fd_set& get_socket_set();
    int& get_top_socket();
    int* get_server_ports();

    File_io get_io();
};