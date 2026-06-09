#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef PO_FV_HPP
#define PO_FV_HPP

namespace PartialOrder{

void initialize_solver(Graph::Graph &G, int k, CaDiCaL::Solver *solver);

std::vector<int> read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver);

std::vector<int> find_cycle(Graph::Graph &G, int k, std::vector<int> colour);

void update_sat(int k,std::vector<int> &colour, std::vector<int> &cycle, CaDiCaL::Solver *solver);

bool check_colouring(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver);

int solve_fv(Graph::Graph &G, int k) ;
}
#endif