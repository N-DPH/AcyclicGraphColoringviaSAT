#include <vector>
#include "Graph.h"

#ifndef PREPROCESSING_CL_HPP
#define PREPROCESSING_CL_HPP

//check if the given vertices form a clique
bool form_clique_cl(Graph::Graph &G, std::vector<int> neighbors);

//calculate size of N_2 of v
int size_N_2_cl(Graph::Graph &G, int v);

//look at the neighborhoods of all vertices and run the needed checks
int preprocessing_cl(Graph::Graph &G, int lb);

#endif