#include "po_fv.hpp"
#include "Graph.h"
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
#include "../cadical/src/cadical.hpp"

void PartialOrder::initialize_solver(Graph::Graph &G, int k, CaDiCaL::Solver *solver){
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


std::vector<int> PartialOrder::read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver){
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

bool PartialOrder::try_sol(Graph::Graph &G, int k, std::vector<int> colour){
	std::vector<int> edges = G.elist();
	for(int i=0;i<G.ecount();i++){
		if(colour[edges[2*i]]>=k){
			return false;
		}
		if(colour[edges[2*i]]==colour[edges[2*i+1]]){
			return false;
		}
	}
	return true;
}

std::vector<int> PartialOrder::find_cycle(Graph::Graph &G, int k, std::vector<int> colour){
	std::set<std::pair<int,int>> existing_pairs;
	std::vector<Graph::VertexType> edges = G.elist();
	std::vector<int> cycle = std::vector<int>(0);
	Graph::NeighborList neighbors = G.get_neighbor_list();
	for(int i=0; i<G.ecount();i++){
		int lower_colour = std::min(colour[edges[2*i]],colour[edges[2*i+1]]);
		int higher_colour = std::max(colour[edges[2*i]],colour[edges[2*i+1]]);
		existing_pairs.emplace(std::make_pair(lower_colour,higher_colour));
	}
	for(std::pair<int,int> p : existing_pairs){
		std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
		std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
		for(int i=0;i<G.ncount();i++){
			if(visited[i]==false && (colour[i]==p.first || colour[i]==p.second)){
				std::queue<int> q;
				q.push(i);
				while(!q.empty()){
					int v = q.front();
					q.pop();
					for(auto w : neighbors[v]){
						if(w != parent[v] && visited[w]==true && (colour[w]==p.first || colour[w]==p.second)){
							cycle.push_back(w);
							cycle.push_back(v);
							w=parent[w];
							cycle.push_back(w);
							w = parent[w];
							v=parent[v];
							while(w != v){
								cycle.push_back(v);
								cycle.push_back(w);
								w = parent[w];
								v=parent[v];
							}
							cycle.push_back(v);
							return cycle;//should work because we only have even cycles
						}
						if(w != parent[v] && (colour[w]==p.first || colour[w]==p.second)){
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
	return cycle;
}

void PartialOrder::update_sat(int k,std::vector<int> &colour, std::vector<int> &cycle, CaDiCaL::Solver *solver){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
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

bool PartialOrder::check_colouring(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver){
	std::vector<int> cycle = find_cycle(G,k,colour);
	if(cycle.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycle: ";
	for(int i=0;i<cycle.size();i++){
		std::cout<<cycle[i]+1<<" ";
	}
	std::cout<<std::endl;*/

	update_sat(k,colour,cycle,solver);
	return false;
}

int PartialOrder::solve_fv(Graph::Graph &G, int k) {

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
		if(try_sol(G,k,colouring)==false){
			delete solver;
			std::__throw_runtime_error("The colouring is not valid.");
		}
		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/

		no_cycle = check_colouring(G, k, colouring, solver);
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
