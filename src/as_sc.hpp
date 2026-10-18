#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef AS_SC_HPP
#define AS_SC_HPP

namespace Assignment{
    void initialize_solver_sc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles);

    int solve_sc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif