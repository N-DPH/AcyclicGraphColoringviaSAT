#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef AS_RC_HPP
#define AS_RC_HPP

namespace Assignment{
    void initialize_solver_rc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles);

    void update_sat_rc(int k, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles);
    
    bool check_colouring_rc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &found_cycles);

    int solve_rc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif