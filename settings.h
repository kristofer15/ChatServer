#include <string.h>
#include <map>
#include <iostream>

namespace settings {
    void set_new_id();
    std::string get_id();
    std::map<int, std::string>& get_users();
};