#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef RP_OC_HPP
#define RP_OC_HPP

namespace Representatives{
    void find_cycle_oc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles);

    bool check_colouring_oc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &variables, std::vector<int> &representatives);

    int solve_oc(Graph::Graph &G, int k,std::vector<std::vector<int>> &previous_cycles);
}
#endif