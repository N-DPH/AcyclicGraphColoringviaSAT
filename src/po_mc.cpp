#include "po_mc.hpp"
#include "po_fv.hpp"
#include "Graph.h"
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <queue>
#include "../cadical/src/cadical.hpp"

void PartialOrder::find_cycle_mc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles){
	std::set<std::pair<int,int>> existing_pairs;
	std::vector<Graph::VertexType> edges = G.elist();
	Graph::NeighborList neighbors = G.get_neighbor_list();
	for(int i=0; i<G.ecount();i++){
		int lower_colour = std::min(colour[edges[2*i]],colour[edges[2*i+1]]);
		int higher_colour = std::max(colour[edges[2*i]],colour[edges[2*i+1]]);
		existing_pairs.emplace(std::make_pair(lower_colour,higher_colour));
	}
	//to not get weird overlapping cycles
	std::vector<bool> in_cycle = std::vector<bool>(G.ncount(),false);
	std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
	std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
	for(std::pair<int,int> p : existing_pairs){
		std::fill(visited.begin(), visited.end(),false);
		std::fill(parent.begin(),parent.end(),-1); 
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
							//resetting everything so that we can find further cycles
							while(!q.empty()){
								v=q.front();
								q.pop();
							}
							std::fill(visited.begin(), visited.end(),false);
							std::fill(parent.begin(),parent.end(),-1);
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
	return;
}

void PartialOrder::update_sat_mc(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
	for(std::vector<int> cycle : cycles){
		for(int i=0;i<cycle.size();i++){
			if(colour[cycle[i]]>0){
				solver->add(-(cycle[i]*k+colour[cycle[i]])),solver->add(cycle[i]*k+colour[cycle[i]]+1);
				//std::cout<<-(cycle[i]*k+colour[cycle[i]])<<" , "<<cycle[i]*k+colour[cycle[i]]+1<<" , ";
			}
			else{
				solver->add(cycle[i]*k+1);
				//std::cout<<cycle[i]*k+colour[cycle[i]]+1<<" , ";
			}
		}
		//std::cout<<std::endl;
		solver->add(0);
	}
}

bool PartialOrder::check_colouring_mc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver){
	std::vector<std::vector<int>> cycles = std::vector<std::vector<int>>(0);
	find_cycle_mc(G,k,colour, cycles);
	if(cycles.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycle: ";
	for(std::vector<int> cycle : cycles){
		for(int i=0;i<cycle.size();i++){
			std::cout<<cycle[i]+1<<" ";
		}
		std::cout<<std::endl;
	}*/

	update_sat_mc(k,colour,cycles,solver);
	return false;
}

int PartialOrder::solve_mc(char* filename, int k) {
	Graph::Graph G = Graph::Graph(filename);
	//G.print();

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	initialize_solver(G,k, solver);
	int res;
	bool no_cycle = false;
	std::vector<int> colouring;
	while(!no_cycle){
		res = solver-> solve();
		if(res != 10){
			std::cout<<"Unsatisfiable"<<std::endl;
			delete solver;
			return 1;
		}
		colouring = read_sol(G,k, solver);

		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/

		no_cycle = check_colouring_mc(G, k, colouring, solver);
	}
	std::cout<<"Acyclic colouring found!"<<std::endl;
	delete solver;
	/*std::cout<<"The colouring is:";
	for(int i=0;i<G.ncount();i++){
		std::cout<<" "<<colouring[i];
	}
	std::cout<<std::endl;*/
	return 0;
	
}
