#include "lower_bound.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

int find_lower_bound (const char* filename){
    std::string command = "./CliSAT/bin/CliSAT_2025_05_21.exe ";
    command += filename;
    command += " 100 1 0 > lower_bound.txt";
    system(command.c_str());
    
    std::ifstream file("lower_bound.txt");
    if(not file){
        throw std::runtime_error("Cannot open file.");
    }
    std::string token;
    int foundNumber;
     // Regular expression looks for "w:" followed by digits (and an optional decimal)
    std::regex pattern("w:([0-9]+(?:\\.[0-9]+)?)");
    std::smatch match;

    if (!file.is_open()) {
        std::cout << "Unable to open file\n";
        return 1;
    }

    while (file >> token) {
        // Search the token for the "w:<number>" pattern
        if (std::regex_search(token, match, pattern)) {
            // match[1] contains just the numeric part captured inside the parenthesis
            std::string numberStr = match[1].str();

            foundNumber = std::stoi(numberStr);
        }
        }
    file.close();
    return foundNumber;
}