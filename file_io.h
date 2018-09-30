#ifndef FILE_IO_H
#define FILE_IO_H

#include <fstream>
#include <vector>
#include <iostream>

class File_io {
public:
    // Read the entire contents of a file into a string
    static std::string read_file(std::string file_name) {
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

    // Read the entire contents of a file into a template vector
    template <class T>
    static std::vector<T> get_lines(std::string file_path) {
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

    static void log(std::string text, std::string file_name="log.txt") {
        std::ofstream file;
        file.open(file_name, std::ios_base::app);

        file << text << std::endl << std::endl;
    }
};

#endif