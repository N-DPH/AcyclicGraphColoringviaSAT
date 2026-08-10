#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef PO_SC_HPP
#define PO_SC_HPP

namespace PartialOrder{
    void initialize_solver_sc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles);

    int solve_sc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif