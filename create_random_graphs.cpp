#include <iostream> 
#include <string>
#include <vector>
#include <cstdlib>
#include "src/Graph.h"

void create_graph(int n, int number){
    std::vector<int> elist=std::vector<int>(0);
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            if(rand()%2==1){
                elist.push_back(i);
                elist.push_back(j);
            }
        }
    }
    Graph::Graph G = Graph::Graph(n,elist.size()/2,elist);
    std::string filename = "random_instances/random_size_"+std::to_string(n)+"_"+std::to_string(number)+".col";
    G.write_dimacs(filename);
}

int main(int argc, char *argv[]) {
	if(argc<2){std::cout<<"No number of graphs given"<<std::endl; return 1;}
    if(argc==2){std::cout<<"No size of graph given"<<std::endl;return 1;}
	for(int i = 0; i<std::stoi(argv[1]);i++){
        create_graph(std::stoi(argv[2]),i);
    }

    return 0;
}