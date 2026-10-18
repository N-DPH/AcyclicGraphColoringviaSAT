#include "as_mc.hpp"
#include "as_oc.hpp"
#include "as_fv.hpp"
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
#include <tuple>
#include "../cadical/src/cadical.hpp"

void Assignment::find_cycle_oc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles){
std::vector<std::tuple<int,int,int>> counting_pairs = std::vector<std::tuple<int,int,int>>(k*k,std::make_tuple(0,0,-1));
	std::vector<Graph::VertexType> edges = G.elist();
	Graph::NeighborList neighbors = G.get_neighbor_list();
	for(int i=0; i<G.ecount();i++){
		int lower_colour = std::min(colour[edges[2*i]],colour[edges[2*i+1]]);
		int higher_colour = std::max(colour[edges[2*i]],colour[edges[2*i+1]]);
		if(std::get<2>(counting_pairs[lower_colour*k+higher_colour])==-1){
			counting_pairs[lower_colour*k+higher_colour] = std::make_tuple(lower_colour,higher_colour,1);
		}
		else{
			counting_pairs[lower_colour*k+higher_colour] = std::make_tuple(lower_colour,higher_colour,std::get<2>(counting_pairs[lower_colour*k+higher_colour])+1);
		}
	}
	std::sort(begin(counting_pairs), end(counting_pairs), [](auto const &t1, auto const &t2) {return std::get<2>(t1) < std::get<2>(t2); });
	
	
	//to not get weird overlapping cycles
	std::vector<bool> in_cycle = std::vector<bool>(G.ncount(),false);
	std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
	std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
	for(std::tuple<int,int,int> p : counting_pairs){
		if(std::get<2>(p)!=0){
			std::fill(visited.begin(), visited.end(),false);
			std::fill(parent.begin(),parent.end(),-1); 
			for(int i=0;i<G.ncount();i++){
				if(!in_cycle[i] && !visited[i] && (colour[i]==std::get<0>(p) || colour[i]==std::get<1>(p))){
					std::queue<int> q;
					q.push(i);
					while(!q.empty()){
						int v = q.front();
						q.pop();
						for(auto w : neighbors[v]){
							if(!in_cycle[w] && w != parent[v] && visited[w] && (colour[w]==std::get<0>(p) || colour[w]==std::get<1>(p))){
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
							if(!in_cycle[w] && w != parent[v] && (colour[w]==std::get<0>(p) || colour[w]==std::get<1>(p))){
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
	}
	return;
}

bool Assignment::check_colouring_oc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver){
	std::vector<std::vector<int>> cycles = std::vector<std::vector<int>>(0);
	find_cycle_oc(G,k,colour,cycles);
	if(cycles.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycles: ";
	for(std::vector<int> cycle: cycles){
		for(int i=0;i<cycle.size();i++){
			std::cout<<cycle[i]+1<<" ";
		}
		std::cout<<std::endl;
	}*/

	update_sat_mc(k,cycles,solver);
	return false;
}

int Assignment::solve_oc(Graph::Graph &G, int k) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
try{	
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
			std::runtime_error("The colouring is not valid.");
		}
		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/

		no_cycle = check_colouring_oc(G, k, colouring, solver);
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
catch(std::runtime_error e){
    std::cout<<"Error caught: "<<e.what()<<std::endl;
    delete solver;
    return 1;
}
catch(...){
    std::cout<<"Unknown exception caught."<<std::endl;
    delete solver;
    return 1;
}
}