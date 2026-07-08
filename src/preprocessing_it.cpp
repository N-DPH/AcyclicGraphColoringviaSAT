#include <vector>
#include "Graph.h"
#include "preprocessing_it.hpp"

//check if the given vertices form a clique
bool form_clique(Graph::Graph &G, std::vector<int> neighbors){
    //put all the neighbors neighbors into sorted vertex
    std::vector<std::vector<int>> adjacence = std::vector<std::vector<int>>(neighbors.size());
    for(int i=0;i<neighbors.size();i++){
        adjacence[i] = G.get_neighbor_list()[neighbors[i]];
        std::sort(adjacence[i].begin(),adjacence[i].end());
    }
    //
    std::sort(adjacence.begin(), adjacence.end(), [](std::vector<int> a, std::vector<int> b) {return a.size() < b.size();});

    //check if all other neighbors have edges to i
    for(int i=0;i<adjacence.size();i++){
        std::vector<int> intersection;
        std::set_intersection(adjacence[i].begin(), adjacence[i].end(), neighbors.begin(), neighbors.end(),std::back_inserter(intersection));
        if(intersection.size()<neighbors.size()-1){return false;}
    }
    return true;
}

//calculate size of N_2 of v
int size_N_2(Graph::Graph &G, int v){
    std::vector<int> neighbors = G.get_neighbor_list()[v];
    int size = neighbors.size();
    neighbors.push_back(v);
    std::sort(neighbors.begin(),neighbors.end());
    for(int i=0;i<neighbors.size();i++){
        if(neighbors[i]!=v){
            std::vector<int> intersection;
            std::vector<int> this_neighbors_neighbors = G.get_neighbor_list()[neighbors[i]];
            std::sort(this_neighbors_neighbors.begin(),this_neighbors_neighbors.end());
            std::set_intersection(this_neighbors_neighbors.begin(),this_neighbors_neighbors.end(),neighbors.begin(),neighbors.end(),std::back_inserter(intersection));
            //add the number of vertices that are in the neighborhood of i but not v or its direct neighbor
            size += this_neighbors_neighbors.size() - intersection.size();
        }
    }
    return size;
}

//look at the neighborhoods of all vertices and run the needed checks
int preprocessing(Graph::Graph &G, int lb){
    int r=0;
    for(int i=0;i<G.ncount();i++){
        std::vector<int> neighbors = G.get_neighbor_list()[i];
        if(neighbors.size()<lb && size_N_2(G, i)<lb){
            G.remove_vertex(i);
            i--;
            r++;
            continue;
        }
        else if(neighbors.size()<lb-1 && form_clique(G,neighbors)){
            G.remove_vertex(i);
            i--;
            r++;
            continue;
        }
    }
    return r;
}