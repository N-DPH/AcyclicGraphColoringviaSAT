#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef PO_AC_HPP
#define PO_AC_HPP

namespace PartialOrder{
    void update_sat_ac(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles);
    
    bool check_colouring_ac(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &found_cycles);

    int solve_ac(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles);
}
#endif