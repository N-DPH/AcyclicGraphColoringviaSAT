#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef RP_MC_HPP
#define RP_MC_HPP

namespace Representatives{
    void find_cycle_mc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles);

    void update_sat_mc(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &variables, std::vector<int> &representatives);
    
    bool check_colouring_mc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &variables, std::vector<int> &representatives);

    int solve_mc(Graph::Graph &G, int k,std::vector<std::vector<int>> &previous_cycles);
}
#endif