#include "Graph.h"
#include <vector>
#include <map>
#include <stack>
#include "../cadical/src/cadical.hpp"

#ifndef PROP_AS_TREE_HPP
#define PROP_AS_TREE_HPP

namespace Propagator_as{

    void initialize_solver_tree(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles);

    int solve_tree(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}

class ActiveTreePropagator;
#endif