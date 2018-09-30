#include <fstream>
#include <vector>
#include <iostream>

template <class T>
std::vector<T> get_lines(std::string file_path) {
    std::vector<T> lines;

    std::ifstream file (file_path);
    if (file.is_open())
    {
        T line;
        while (file >> line)
        {
            lines.push_back(line);
        }
        file.close();
    }

    // Return empty vector if file could not be read
    return lines;
}

void log(std::string text, std::string file_name="log.txt") {
    std::ofstream file;
    file.open(file_name, std::ios_base::app);

    file << text << std::endl << std::endl;
}
