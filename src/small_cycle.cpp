#include "Graph.h"
#include <vector>
#include <queue>
#include <iostream>

void find_small_cycles(Graph::Graph &G, std::vector<std::vector<int>> &cycles){
	int n = G.ncount();
    std::vector<std::vector<int>> neighbors = G.get_neighbor_list();
    std::vector<int> parent = std::vector<int>(n,-1);
    std::vector<bool> visited = std::vector<bool>(n,false);
    for(int i=0;i<n;i++){
        for(auto v: neighbors[i]){
            if(v>i){//if v<i then this cycle has already been found on the tour from v
                visited[v]=true;
                parent[v]=i;
                for(auto w:neighbors[v]){
                    if(w>i  && parent[w]!=i && visited[w]==false){
                        visited[w]=true;
                        parent[w]=v;
                    }
                    else if(w>i &&parent[w]!=i && visited[w]==true){
                        cycles.push_back({-1,i,v,parent[w],w});
                    }
                }
            }
        }
        std::fill(parent.begin(),parent.end(),-1);
        std::fill(visited.begin(),visited.end(),false);
    }
    return;
}
