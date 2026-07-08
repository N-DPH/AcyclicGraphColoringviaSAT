#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef PO_OC_HPP
#define PO_OC_HPP

namespace PartialOrder{
    void find_cycle_oc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles);

    bool check_colouring_oc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver);

    int solve_oc(Graph::Graph &G, int k);
}
#endif