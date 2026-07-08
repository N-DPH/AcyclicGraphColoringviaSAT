#include "src/preprocessing_it.hpp"
#include "src/Graph.h"
#include "src/lower_bound.hpp"

int main(int argc, char *argv[]){
    Graph::Graph G = Graph::Graph(argv[1]);
    int lb = find_lower_bound(argv[1]);
    int r = preprocessing(G, lb);
    std::cout<<"removed "<<r<<" vertices"<<std::endl;
    return 0;
}