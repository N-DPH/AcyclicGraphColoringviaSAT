#include "Graph.h"
#include <vector>
#include <iostream>
#include "../cadical/src/cadical.hpp"
#include "as_fv.hpp"
#include "as_mc.hpp"

int main(int argc, char *argv[]){
    if(argc<2){std::cout<<"No file given"<<std::endl; return 1;}
	if(argc==2){std::cout<<"No colour number given"<<std::endl; return 1;}
    if(argc==3){std::cout<<"No flags specified so we use the standard -as -fv"<<std::endl;}
	if(argc==4){std::cout<<"Using the -fv"<<std::endl;}
    
    if(argc==3 || std::string(argv[3])=="-as"){
        if(argc<=4 || std::string(argv[4])=="-fv"){
            return assignment::solve_fv(argv[1],std::stoi(argv[2]));
        }
        else if(std::string(argv[4])=="-mc"){
            return assignment::solve_mc(argv[1],std::stoi(argv[2]));
        }
    }
}