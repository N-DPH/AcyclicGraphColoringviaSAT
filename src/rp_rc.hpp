#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef RP_RC_HPP
#define RP_RC_HPP

namespace Representatives{
    void initialize_solver_rc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables);

    std::vector<int> read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &variables,std::vector<int> &representatives);

    void find_cycle_rc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles);

    void update_sat_rc(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables, std::vector<int> &representatives);
    
    bool check_colouring_rc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables, std::vector<int> &representatives);

    int solve_rc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif