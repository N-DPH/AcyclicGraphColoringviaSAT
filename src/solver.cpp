#include "Graph.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <csignal>
#include <sys/resource.h>
#include <string.h>
#include <fcntl.h>
#include "../cadical/src/cadical.hpp"
#include "as_fv.hpp"
#include "as_mc.hpp"
#include "po_fv.hpp"
#include "po_mc.hpp"
#include "po_oc.hpp"
#include "po_rc.hpp"
#include "po_ac.hpp"
#include "po_sc.hpp"
#include "rp_fv.hpp"
#include "rp_rc.hpp"
#include "rp_sc.hpp"
#include "prop_rp.hpp"
#include "prop_as_tree.hpp"
#include "upper_bound.hpp"
#include "lower_bound.hpp"
#include "preprocessing_it.hpp"
#include "preprocessing_cl.hpp"
#include "small_cycle.hpp"

char file[50];

void signal_handler(int signal)
{
    int fd = open(file, O_WRONLY | O_APPEND);
    if (fd == -1) {
        // Equivalent to your 'throw' statement, but safe for a signal handler
        const char err[] = "Unable to open file to write log to\n";
        write(STDERR_FILENO, err, sizeof(err) - 1);
        _exit(EXIT_FAILURE); 
    }
    const char timeout[] = "TIMEOUT\n";
    write(fd, timeout, sizeof(timeout)-1);
    close(fd);
    _exit(1);
}

int main(int argc, char *argv[]){

try{
    if(argc<2){throw std::runtime_error("No file given");}
    if(argc==2){throw std::runtime_error("No timeout defined");}
    if(argc==3){std::cout<<"No flags specified so we use the standard -rp -rc"<<std::endl;}
	if(argc==4){std::cout<<"Using the flag -rc"<<std::endl;}

    rlimit cpu_limit;
    int time_limit;
    try{time_limit = std::stoi(argv[2]);}
    catch(...){std::cout<<"Timeout must be a whole number."<<std::endl;return 1;}
    cpu_limit.rlim_cur = time_limit;
    cpu_limit.rlim_max = time_limit+2;

    if (setrlimit(RLIMIT_CPU, &cpu_limit) != 0) {
        throw std::runtime_error("Failed to set CPU time limit limit.");
    }
    
    std::clock_t t_start = std::clock();
    Graph::Graph G =  Graph::Graph(argv[1]);
    int n = G.ncount();

    //prep to log the time
    std::string filename = "logs_results/log_";
    filename += std::to_string(n);
    filename += ".csv";
    strcpy(file, filename.c_str());

    signal(SIGXCPU, signal_handler);
    std::clock_t t_read_graph = std::clock();
    std::cout<<"Time for reading the graph "<< 1000.0*(t_read_graph - t_start) / CLOCKS_PER_SEC<<"ms."<<std::endl;

    int lb = find_lower_bound(argv[1]);
    std::clock_t t_lb = std::clock();
    std::cout<<"Lower bound found "<<lb<<std::endl;
    std::cout<<"Time for computing lower bound "<< 1000.0 *(t_lb - t_read_graph) / CLOCKS_PER_SEC<<"ms."<<std::endl;
    
    int r_local = preprocessing_cl(G,lb);
    int r = r_local;
    while(r_local!=0){r_local=preprocessing_cl(G,lb);r+=r_local;} //repeat until nothing changes
    std::clock_t t_pre = std::clock();
    std::cout<<"Preprocessing removed "<<r <<" vertices"<<std::endl;
    std::cout<<"Time for preprocessing "<< 1000.0 *(t_pre - t_lb) / CLOCKS_PER_SEC<<"ms."<<std::endl;
    
    int ub = acyclic_dsatur(G);
    std::clock_t t_ub = std::clock();
    std::cout<<"Upper bound found "<<ub<<std::endl;
    std::cout<<"Time for computing upper bound "<< 1000.0 *(t_ub - t_pre) / CLOCKS_PER_SEC<<"ms."<<std::endl;

    if(ub<=lb){
        std::cout<<"After preprocessing the upper bound is lower or equal to the lower bound."<<std::endl<<"The colouring number is "<< lb<<std::endl;
        std::cout<<"Time for whole program "<< 1000.0 *(t_ub - t_start) / CLOCKS_PER_SEC<<"ms."<<std::endl<<std::endl;
        return 0;
    }
    int colouring_number = 0;
    std::vector<std::vector<int>> cycles_found;
    find_small_cycles(G,cycles_found);
    std::clock_t t_cycles = std::clock();
    std::cout<<"Found "<<cycles_found.size()<<" many small cycles."<<std::endl;
    std::cout<<"Time for computing small cycles "<< 1000.0 *(t_cycles - t_ub) / CLOCKS_PER_SEC<<"ms."<<std::endl;
    for(int i = ub-1 ; i>=lb ; i--){
        if(argc==3 || std::string(argv[3])=="-rp"){
            if(argc<=4 || std::string(argv[4])=="-rc"){
                if(Representatives::solve_rc(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-sc"){
                if(Representatives::solve_sc(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-fv"){
                if(Representatives::solve_fv(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
            }
        }
        else if(std::string(argv[3])=="-prop"){
            if(argc<=4 || std::string(argv[4])=="-rp"){
                if(Propagator_rp::solve(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-as-tree"){
                if(Propagator_as::solve_tree(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                } 
            }
        }
        else if(std::string(argv[3])=="-po"){
            if(argc<=4 || std::string(argv[4])=="-rc"){
                if(PartialOrder::solve_rc(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-sc"){
                if(PartialOrder::solve_sc(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-ac"){
                if(PartialOrder::solve_ac(G,i,cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-oc"){
                if(PartialOrder::solve_oc(G,i)==1){
                    colouring_number= i+1;
                    break;
                }    
            }
            else if(std::string(argv[4])=="-mc"){
                if(PartialOrder::solve_mc(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if( std::string(argv[4])=="-fv"){
                if(PartialOrder::solve_fv(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
            }
        }
        else if(std::string(argv[3])=="-as"){
            if(argc<=4 || std::string(argv[4])=="-mc"){
                if(Assignment::solve_mc(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if(std::string(argv[4])=="-fv"){
                if(Assignment::solve_fv(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
            }
        }
        else if(argc==4){
            std::cout<<"Only the version not the assignment specified using -po as standard"<<std::endl;
            if(std::string(argv[3])=="-rc"){
                if(PartialOrder::solve_rc(G,i, cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if(std::string(argv[3])=="-sc"){
                if(PartialOrder::solve_sc(G,i, cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if(std::string(argv[3])=="-ac"){
                if(PartialOrder::solve_ac(G,i, cycles_found)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if(std::string(argv[3])=="-oc"){
                if(PartialOrder::solve_oc(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if(std::string(argv[3])=="-mc"){
                if(PartialOrder::solve_mc(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else if( std::string(argv[3])=="-fv"){
                if(PartialOrder::solve_fv(G,i)==1){
                    colouring_number= i+1;
                    break;
                }
            }
            else{
                throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
            }
        }
        else{
            throw std::runtime_error("The flag "+std::string(argv[3])+" does not exist");
        }
    }
    if(colouring_number==0){colouring_number=lb;}
    std::cout<<"The colouring number is "<<colouring_number<<std::endl;
    std::clock_t t_end = std::clock();
    std::cout<<"Time for finding actual colouring number "<< 1000.0*(t_end - t_cycles) / CLOCKS_PER_SEC<<"ms."<<std::endl;
    std::cout<<"Time for the whole program "<<1000.0*(t_end-t_start)/CLOCKS_PER_SEC <<"ms."<<std::endl<<std::endl;
    
    //log the time
    std::ofstream out_file(filename,std::ios::app);
    if(not out_file.is_open()){
        throw std::runtime_error("Unable to open file to write log to");
    }
    out_file <<1000*(t_end-t_start)/CLOCKS_PER_SEC<<std::endl;
    out_file.close();
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