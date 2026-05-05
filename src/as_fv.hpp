#include "Graph.h"
#include <vector>
#include "../cadical/src/cadical.hpp"

#ifndef AS_FV_HPP
#define AS_FV_HPP

namespace assignment{

void initialize_solver(Graph::Graph &G, int k, CaDiCaL::Solver *solver);

std::vector<int> read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver);

std::vector<int> find_cycle(Graph::Graph &G, int k, std::vector<int> colour);

void update_sat(int k, std::vector<int> &cycle, CaDiCaL::Solver *solver);

bool check_colouring(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver);

int solve_fv(char* filename, int k) ;
}
#endif