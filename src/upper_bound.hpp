#include "Graph.h"
#include <vector>

#ifndef UPPER_BOUND_HPP
#define UPPER_BOUND_HPP

//according to the most saturated and largest degree
int find_next_vertex(Graph::Graph &G, std::vector<int> &colour);

bool BFS(Graph::Graph &G, std::vector<int> &colour, int s);

int acyclic_dsatur (Graph::Graph &G);

#endif