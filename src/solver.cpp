#include "Graph.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <ctime>
#include "../cadical/src/cadical.hpp"
#include "as_fv.hpp"
#include "as_mc.hpp"
#include "po_fv.hpp"
#include "po_mc.hpp"
#include "upper_bound.hpp"

int main(int argc, char *argv[]){

try{
    if(argc<2){throw std::runtime_error("No file given");}
    if(argc==2){std::cout<<"No flags specified so we use the standard -po -mc"<<std::endl;}
	if(argc==3){std::cout<<"Using the flag -mc"<<std::endl;}
    std::clock_t t_start = std::clock();
    Graph::Graph G =  Graph::Graph(argv[1]);
    std::clock_t t_read_graph = std::clock();
    std::cout<<"Time for reading the graph "<< 1000.0*(t_read_graph - t_start) / CLOCKS_PER_SEC<<"ms."<<std::endl;
    int ub = acyclic_dsatur(G);
    std::clock_t t_ub = std::clock();
    std::cout<<"Upper bound found "<<ub<<std::endl;
    std::cout<<"Time for computing upper bound "<< 1000.0 *(t_ub - t_read_graph) / CLOCKS_PER_SEC<<"ms."<<std::endl;
    for(int i = ub ; i>0 ; i--){
        if(argc==2 || std::string(argv[2])=="-po"){
            if(argc<=3 || std::string(argv[3])=="-mc"){
                if(PartialOrder::solve_mc(G,i)==1){
                    std::cout<<"The colouring number is "<<i+1<<std::endl;
                    std::clock_t t_end = std::clock();
                    std::cout<<"Time for finding actual colouring number "<< (t_end - t_ub) / CLOCKS_PER_SEC<<"s."<<std::endl;
                    std::cout<<"Time for the whole program "<<(t_end-t_start)/CLOCKS_PER_SEC <<"s."<<std::endl;
                    return 0;
                }
            }
            else if( std::string(argv[3])=="-fv"){
                if(PartialOrder::solve_fv(G,i)==1){
                    std::cout<<"The colouring number is "<<i+1<<std::endl;
                    std::clock_t t_end = std::clock();
                    std::cout<<"Time for finding actual colouring number "<< (t_end - t_ub) / CLOCKS_PER_SEC<<"s."<<std::endl;
                    std::cout<<"Time for the whole program "<<(t_end-t_start)/CLOCKS_PER_SEC <<"s."<<std::endl;
                    return 0;
                }
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
            }
        }
        else if(std::string(argv[2])=="-as"){
            if(argc<=3 || std::string(argv[3])=="-mc"){
                if(Assignment::solve_mc(G,i)==1){
                    std::cout<<"The colouring number is "<<i+1<<std::endl;
                    std::clock_t t_end = std::clock();
                    std::cout<<"Time for finding actual colouring number "<< (t_end - t_ub) / CLOCKS_PER_SEC<<"s."<<std::endl;
                    std::cout<<"Time for the whole program "<<(t_end-t_start)/CLOCKS_PER_SEC <<"s."<<std::endl;
                    return 0;
                }
            }
            else if(std::string(argv[3])=="-fv"){
                if(Assignment::solve_fv(G,i)==1){
                    std::cout<<"The colouring number is "<<i+1<<std::endl;
                    std::clock_t t_end = std::clock();
                    std::cout<<"Time for finding actual colouring number "<< (t_end - t_ub) / CLOCKS_PER_SEC<<"s."<<std::endl;
                    std::cout<<"Time for the whole program "<<(t_end-t_start)/CLOCKS_PER_SEC <<"s."<<std::endl;
                    return 0;
                }
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
            }
        }
        else if(argc==3){
            std::cout<<"Only the version not the assignment specified using -po as standard"<<std::endl;
            if(std::string(argv[2])=="-mc"){
                if(PartialOrder::solve_mc(G,i)==1){
                    std::cout<<"The colouring number is "<<i+1<<std::endl;
                    std::clock_t t_end = std::clock();
                    std::cout<<"Time for finding actual colouring number "<< (t_end - t_ub) / CLOCKS_PER_SEC<<"s."<<std::endl;
                    std::cout<<"Time for the whole program "<<(t_end-t_start)/CLOCKS_PER_SEC <<"s."<<std::endl;
                    return 0;
                }
            }
            else if( std::string(argv[2])=="-fv"){
                if(PartialOrder::solve_fv(G,i)==1){
                    std::cout<<"The colouring number is "<<i+1<<std::endl;
                    std::clock_t t_end = std::clock();
                    std::cout<<"Time for finding actual colouring number "<< (t_end - t_ub) / CLOCKS_PER_SEC<<"s."<<std::endl;
                    std::cout<<"Time for the whole program "<<(t_end-t_start)/CLOCKS_PER_SEC <<"s."<<std::endl;
                    return 0;
                }
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[2])+" does not exist");
            }
        }
        else{
            throw std::runtime_error("The flag "+std::string(argv[2])+" does not exist");
        }
    }
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