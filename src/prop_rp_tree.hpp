#include "Graph.h"
#include <vector>
#include <map>
#include <stack>
#include "../cadical/src/cadical.hpp"

#ifndef PROP_RP_TREE_HPP
#define PROP_RP_TREE_HPP

namespace Propagator_rp{

    void initialize_solver_tree(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables);

    int solve_tree(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}

class ActiveTreePropagator;
#endif