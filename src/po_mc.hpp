#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef PO_MC_HPP
#define PO_MC_HPP

namespace PartialOrder{
    void find_cycle_mc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles);

    void update_sat_mc(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver);

    bool check_colouring_mc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver);

    int solve_mc(char* filename, int k);
}
#endif