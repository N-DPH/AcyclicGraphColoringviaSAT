#include "Graph.h"
#include <vector>
#include <iostream>
#include "../cadical/src/cadical.hpp"
#include "as_fv.hpp"
#include "as_mc.hpp"
#include "po_fv.hpp"
#include "po_mc.hpp"

int main(int argc, char *argv[]){
    if(argc<2){std::cout<<"No file given"<<std::endl; return 1;}
	if(argc==2){std::cout<<"No colour number given"<<std::endl; return 1;}
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
            std::cout<<"The flag "<<std::string(argv[4])<<" does not exist"<<std::endl;
            return 1;
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
            std::cout<<"The flag "<<std::string(argv[4])<<" does not exist"<<std::endl;
            return 1;
        }
    }
    else{
        std::cout<<"The flag "<<std::string(argv[3])<<" does not exist"<<std::endl;
        return 1;
    }
}