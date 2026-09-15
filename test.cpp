#include "src/Graph.h"
#include "cadical/src/cadical.hpp"
#include "src/prop_as_tree.hpp"
#include <vector>

int main(int argc, char *argv[]){
    Graph::Graph G = Graph::Graph(argv[1]);
    std::vector<std::vector<int>> cycles_found = std::vector<std::vector<int>>();
    Propagator_as::solve_tree(G,35, cycles_found);
    return 0;
}