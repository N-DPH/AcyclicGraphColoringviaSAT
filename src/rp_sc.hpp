#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef RP_SC_HPP
#define RP_SC_HPP

namespace Representatives{
    void initialize_solver_sc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables);

    int solve_sc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif