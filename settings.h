#include <string.h>
#include <map>
#include <iostream>

namespace settings {
    void set_new_id();
    std::string get_id();
    std::map<std::string, int>& get_users();
};