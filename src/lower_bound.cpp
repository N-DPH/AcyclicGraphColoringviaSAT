#include "lower_bound.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

int find_lower_bound (const char* filename){
    std::string command = "./CliSAT/bin/CliSAT_2025_05_21.exe ";
    command += filename;
    command += " 100 1 0 > lower_bound.txt";
    system(command.c_str());

     std::ifstream file("lower_bound.txt");
    if(not file){
        throw std::runtime_error("Cannot open file.");
    }

    std::string line;
    std::getline(file, line);
    while(line[0] != 'w'){
        std::getline(file, line);
    }
    int lb;
    std::stringstream ss(line.substr(2));
    ss >> lb;            
    
    file.close();
    return lb;
}