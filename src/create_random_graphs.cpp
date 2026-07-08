#include <iostream> 
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include "Graph.h"

void create_graph(int n,double density, int number){
    std::vector<int> elist=std::vector<int>(0);
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            if(rand()%10<density*10){
                elist.push_back(i);
                elist.push_back(j);
            }
        }
    }
    Graph::Graph G = Graph::Graph(n,elist.size()/2,elist);
    std::string filename = "random_instances/random_size_"+std::to_string(n)+"_"+std::to_string(number)+"_"+std::to_string(int(density*10))+".col";
    G.write_dimacs(filename);
}

int main(int argc, char *argv[]) {
try{
	if(argc<2){throw std::runtime_error("No number of graphs given");}
    if(argc<2){throw std::runtime_error("No size of graph given");}
    if(argc==3){throw std::runtime_error("No density given");}
    system("rm -f random_instances/*");
    std::srand(std::time({}));
    if(!(std::atoi(argv[1])>0)){throw std::runtime_error("The input should be a positive number");}
    if(!(std::atoi(argv[2])>0)){throw std::runtime_error("The input should be a positive number");}
    if(!(std::atof(argv[3])>0)){throw std::runtime_error("The input should be a positive number");}
	for(int i = 0; i<std::atoi(argv[1]);i++){
        create_graph(std::atoi(argv[2]), std::atof(argv[3]),i);
    }

    return 0;
}
catch(std::runtime_error e){
    std::cout<<"Error caught: "<<e.what()<<std::endl;
    return 1;
}
catch(...){
    std::cout<<"Unknown error caught"<<std::endl;
    return 1;
}
}