#include "Graph.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include "../cadical/src/cadical.hpp"
#include "as_fv.hpp"
#include "as_mc.hpp"
#include "po_fv.hpp"
#include "po_mc.hpp"

int main(int argc, char *argv[]){
try{
    if(argc<2){throw std::runtime_error("No file given");}
	if(argc==2){throw std::runtime_error("No colour number given");}
    if(argc==3){std::cout<<"No flags specified so we use the standard -po -mc"<<std::endl;}
	if(argc==4){std::cout<<"Using the -mc"<<std::endl;}
    
    if(argc==3 || std::string(argv[3])=="-po"){
        if(argc<=4 || std::string(argv[4])=="-mc"){
            return PartialOrder::solve_mc(argv[1],std::stoi(argv[2]));
        }
        else if( std::string(argv[4])=="-fv"){
            return PartialOrder::solve_fv(argv[1],std::stoi(argv[2]));
        }
        else{
            throw std::runtime_error("The flag "+std::string(argv[4])+" does not exist");
        }
    }
    else if(std::string(argv[3])=="-as"){
        if(argc<=4 || std::string(argv[4])=="-mc"){
            return Assignment::solve_mc(argv[1],std::stoi(argv[2]));
        }
        else if(std::string(argv[4])=="-fv"){
            return Assignment::solve_fv(argv[1],std::stoi(argv[2]));
        }
        else{
            throw std::runtime_error("The flag "+std::string(argv[4])+" does not exist");
        }
    }
    else{
        throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
    }
}
catch(std::runtime_error e){
    std::cout<<"Error caught: "<<e.what()<<std::endl;
}
catch(...){
    std::cout<<"Unknown exception caught."<<std::endl;
}
}