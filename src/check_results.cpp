#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>


//mostly written by googleai. But just meant as a check
int main(int argc, char *argv[]){
try{
    //enter the results file
    if(argc<2){throw std::runtime_error("No file given");}
    std::ifstream file(argv[1]);
    if(not file){
        throw std::runtime_error("Cannot open file.");
    }
    std::string line;
    bool inBlock = false;
    std::string expectedResultPattern = "";
    int linesInCurrentBlock = 1;

    while (std::getline(file, line)) {
        // Check if a new block starts
        if (line.rfind("random_instances", 0) == 0) {
            // If we were already in a block, verify it had exactly n lines before starting a new one
            // Reset trackers for the new block
            inBlock = true;
            linesInCurrentBlock = 1;
            expectedResultPattern = ""; 
            continue;
        }
        // Process lines inside a block
        if (inBlock && line.rfind("The colouring number is", 0) == 0) {
            // Set the baseline pattern for the first result line in this block
            if (linesInCurrentBlock == 1) {
                expectedResultPattern = line;
            } else if (line != expectedResultPattern) {
                std::cout << "There are differing results on the same instance"<<std::endl;
                return 1;
            }
            linesInCurrentBlock++;
        }
    }
    std::cout<<"All results match"<<std::endl;
    return 0;
}
catch(std::runtime_error e){
    std::cout<<"Error caught: "<<e.what()<<std::endl;
    return 1;
}
catch(...){
    std::cout<<"Unknown exception caught."<<std::endl;
    return 1;
}
}