#include <fstream>
#include <vector>
#include <iostream>

std::string read_file(std::string file_name) {
    std::string text;
    std::ifstream file (file_name);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line)) {
            text += line + "\n";
        }
        file.close();
    }

    return text;
}

void log(std::string text, std::string file_name="log.txt") {
    std::ofstream file;
    file.open(file_name, std::ios_base::app);

    file << text << std::endl << std::endl;
}
