#include "../../Graph.h"
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <queue>
#include "../../cadical/src/cadical.hpp"

void initialize_solver(Graph::Graph &G, int k, CaDiCaL::Solver *solver){
	int n = G.ncount();
	int num_var = n*k;
	solver->declare_more_variables(num_var);
	//say that each vertex needs a colour
	for(int i=0; i<n;i++){
		solver->clause(-(i*k+k));
	}
	std::vector<Graph::VertexType> edges = G.elist();
	//for each edge, one of the vertices needs to have colour higher 1
	for(int i=0; i<G.ecount();i++){
		solver->clause((edges[2*i]*k+1),(edges[2*i+1]*k+1));
	}
	//the sequence for one vertex over colours should just have one 1-0 switch
	for(int i=0; i<n;i++){
		for(int j=1;j<k;j++){
			solver->clause((i*k+j),-(i*k+j+1));
		}
	}
	//adjacent vertices shouldn't have the same colour
	for (int i=0;i<G.ecount();i++){
		for(int j=2;j<=k;j++){
			solver->clause(-(edges[2*i]*k+j-1),(edges[2*i]*k+j),-(edges[2*i+1]*k+j-1),(edges[2*i+1]*k+j));
		}
	}
}


std::vector<int> read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver){
	std::vector<int> colours = std::vector<int>(G.ncount());
	int res;
	//find out which colours were assigned
	for(int i=0; i<G.ncount();i++){
		for(int j=1;j<=k;j++){
			res = solver->val(i*k+j);
			if(res == -(i*k+j)){
				colours[i]=j-1;
				break;
			}
		}
	}
    return colours;
}

std::vector<std::vector<int>> find_cycle(Graph::Graph &G, int k, std::vector<int> colour){
	std::set<std::pair<int,int>> existing_pairs;
	std::vector<Graph::VertexType> edges = G.elist();
	std::vector<std::vector<int>> cycles = std::vector<std::vector<int>>(0);
	Graph::NeighborList neighbors = G.get_neighbor_list();
	for(int i=0; i<G.ecount();i++){
		int lower_colour = std::min(colour[edges[2*i]],colour[edges[2*i+1]]);
		int higher_colour = std::max(colour[edges[2*i]],colour[edges[2*i+1]]);
		existing_pairs.emplace(std::make_pair(lower_colour,higher_colour));
	}
	//to not get weird overlapping cycles
	std::vector<bool> in_cycle = std::vector<bool>(G.ncount(),false);
	for(std::pair<int,int> p : existing_pairs){
		std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
		std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
		for(int i=0;i<G.ncount();i++){
			if(!in_cycle[i] && !visited[i] && (colour[i]==p.first || colour[i]==p.second)){
				std::queue<int> q;
				q.push(i);
				while(!q.empty()){
					int v = q.front();
					q.pop();
					for(auto w : neighbors[v]){
						if(!in_cycle[w] && w != parent[v] && visited[w] && (colour[w]==p.first || colour[w]==p.second)){
							std::vector<int> cycle= std::vector<int>(0);
							cycle.push_back(w);
							in_cycle[w]=true;
							int start_of_cycle = w;
							cycle.push_back(v);
							in_cycle[v]=true;
							w=parent[w];
							cycle.push_back(w);
							in_cycle[w]=true;
							w = parent[w];
							v=parent[v];
							while(w != v){
								cycle.push_back(v);
								in_cycle[v]=true;
								cycle.push_back(w);
								in_cycle[w]=true;
								w = parent[w];
								v=parent[v];
							}
							cycle.push_back(v);
							in_cycle[v]=true;
							cycles.push_back(cycle);//should work because we only have even cycles
							//resetting everything so that we can just continue with the next vertex
							while(!q.empty()){
								v=q.front();
								if(v!= start_of_cycle){
									visited[v]=false;
								}
								q.pop();
							}
							break;
						}
						if(!in_cycle[w] && w != parent[v] && (colour[w]==p.first || colour[w]==p.second)){
							visited[w]=true;
							parent[w]=v;
							q.push(w);
						}
					}
				}
			}
			else{visited[i]=true;}
		}
	}
	return cycles;
}

void update_sat(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
	for(std::vector<int> cycle : cycles){
		for(int i=0;i<cycle.size();i++){
			if(colour[cycle[i]]!=0){
				solver->add(-(cycle[i]*k+colour[cycle[i]])),solver->add(cycle[i]*k+colour[cycle[i]]+1);
				//std::cout<<-(cycle[i]*k+colour[cycle[i]])<<" , "<<cycle[i]*k+colour[cycle[i]]+1<<" , ";
			}
			else{
				solver->add(cycle[i]*k+colour[cycle[i]]+1);
				//std::cout<<cycle[i]*k+colour[cycle[i]]+1<<" , ";
			}
		}
		//std::cout<<std::endl;
		solver->add(0);
	}
}

bool check_colouring(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver){
	std::vector<std::vector<int>> cycles = find_cycle(G,k,colour);
	if(cycles.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycle: ";
	for(std::vector<int> cycle : cycles){
		for(int i=0;i<cycle.size();i++){
			std::cout<<cycle[i]+1<<" ";
		}
		std::cout<<std::endl;
	}*/

	update_sat(k,colour,cycles,solver);
	return false;
}

int main(int argc, char *argv[]) {
	if(argc<2){std::cout<<"No file given"<<std::endl; return 1;}
	if(argc==2){std::cout<<"No colour number given"<<std::endl; return 1;}
	Graph::Graph G = Graph::Graph(argv[1]);
	int k = std::stoi(argv[2]);
	//G.print();

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	initialize_solver(G,k, solver);
	int res;
	bool no_cycle = false;
	std::vector<int> colouring;
	while(!no_cycle){
		res = solver-> solve();
		if(res != 10){
			//std::cout<<"Unsatisfiable"<<std::endl;
			return 1;
		}
		colouring = read_sol(G,k, solver);

		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/

		no_cycle = check_colouring(G, k, colouring, solver);
	}
	/*std::cout<<"Acyclic colouring found!"<<std::endl;
	std::cout<<"The colouring is:";
	for(int i=0;i<G.ncount();i++){
		std::cout<<" "<<colouring[i];
	}
	std::cout<<std::endl;*/
	return 0;
	
}
