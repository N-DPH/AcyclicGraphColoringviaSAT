#include <vector>
#include "Graph.h"

#ifndef PREPROCESSING_IT_HPP
#define PREPROCESSING_IT_HPP

//check if the given vertices form a clique
bool form_clique(Graph::Graph &G, std::vector<int> neighbors);

//calculate size of N_2 of v
int size_N_2(Graph::Graph &G, int v);

//look at the neighborhoods of all vertices and run the needed checks
int preprocessing(Graph::Graph &G, int lb);

#endif