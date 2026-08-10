#include "Graph.h"
#include <vector>
#include <map>
#include <stack>
#include "../cadical/src/cadical.hpp"

#ifndef PROP_RP_HPP
#define PROP_RP_HPP

namespace Propagator_rp{

    void initialize_solver(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables);

    class ActivePropagator;

    int solve(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif