#include "Graph.h"
#include <vector>
#include <queue>

//according to the most saturated and largest degree
int find_next_vertex(Graph::Graph &G, std::vector<int> &colour){
    int n = G.ncount();
    int max_sat=0;
    int max_degree=0;
    int next_vertex;
    Graph::NeighborList neighbors = G.get_neighbor_list();
    for(int v = 0;v<n;v++){
        if(colour[v]==-1){
            int sat_v= 0;
            for(int w : neighbors[v]){
                if(colour[w]!=-1){
                    sat_v ++;
                }
            }
            if(sat_v>max_sat || (sat_v==max_sat && neighbors[v].size()>max_degree)){
                max_sat=sat_v;
                next_vertex=v;
                max_degree= neighbors[v].size();
            }
        }
    }
    return next_vertex;
}

bool BFS(Graph::Graph &G, std::vector<int> &colour, int s){
    std::set<int> other_colour;
	std::vector<Graph::VertexType> edges = G.elist();
	Graph::NeighborList neighbors = G.get_neighbor_list();
	for(int w : neighbors[s]){
        if(colour[w]==-1){continue;}
		other_colour.emplace(colour[w]);
	}
	for(int c : other_colour){
        std::queue<int> q;
        q.push(s);
		std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
        visited[s]=true;
		std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
        int v;
		while(!q.empty()){
            v = q.front();
            q.pop();
            for(int w :neighbors[v]){
                if(w != parent[v] && visited[w]==true){
                    //found cycle
                    return true;
                }
                if(w != parent[v] && (colour[w]==colour[s] || colour[w]==c)){
                    visited[w]=true;
                    q.push(w);
                    parent[w] = v;
                }
            }
        }
	}
    return false;
}

int acyclic_dsatur (Graph::Graph &G){
    int n = G.ncount();
    int size_U = n;
    std::vector<int> colour = std::vector<int>(n,-1);
    int num_colour = 0;
    std::vector<bool> U = std::vector<bool>(n,true);
    int v;
    std::vector<bool> forbidden = std::vector<bool>();
    Graph::NeighborList neighbors = G.get_neighbor_list();
    while(size_U!=0){
        std::fill(forbidden.begin(),forbidden.end(),false);
        v = find_next_vertex(G,colour); //could be done faster through heaps
        for(int w : neighbors[v]){
            if(colour[w]==-1){continue;}
            forbidden[colour[w]]=true;
        }
        bool valid = false;
        while(!valid){
            int old_colour = colour[v];
            for(int i=old_colour+1;i<num_colour;i++){
                if(!forbidden[i]){
                    colour[v]=i;
                    break;
                }
            }
            if(colour[v]==old_colour){ //if none of the so far used colours work
                num_colour++;
                colour[v]=num_colour-1;
                forbidden.push_back(false);
                break;
            }
            if(!BFS(G,colour,v)){
                valid=true;
            }
            else{
                forbidden[colour[v]]=true;
            }
        }
        if(U[v]==true){
            size_U--;
        }
        U[v] = false;
    }
    return num_colour;
}