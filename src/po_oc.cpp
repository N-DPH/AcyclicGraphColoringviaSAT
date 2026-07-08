#include "po_oc.hpp"
#include "po_mc.hpp"
#include "po_fv.hpp"
#include "Graph.h"
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <tuple>
#include <queue>
#include <ctime>
#include "../cadical/src/cadical.hpp"

void PartialOrder::find_cycle_oc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles){
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

bool PartialOrder::check_colouring_oc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver){
	std::vector<std::vector<int>> cycles = std::vector<std::vector<int>>(0);
	find_cycle_oc(G,k,colour, cycles);
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

int PartialOrder::solve_oc(Graph::Graph &G, int k) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	initialize_solver(G,k, solver);
	int res;
	bool no_cycle = false;
	std::vector<int> colouring;

	int counter=0;
	std::clock_t t_before_solve;
	std::clock_t t_after_solve;
	std::clock_t t_before_cycle;
	std::clock_t t_after_cycle;
	double t_solving;
	double t_finding_cycles;

	while(!no_cycle){
		t_before_solve = std::clock();
		res = solver-> solve();
		t_after_solve = std::clock();
		t_solving = t_solving + 1000.0*(t_after_solve - t_before_solve)/ CLOCKS_PER_SEC;

		if(res != 10){
			std::cout<<"Unsatisfiable"<<std::endl;
			std::cout<<(counter+1)<<" calls to solver. Time spent on solver: "<< t_solving<<"ms. Time spent finding cycles: "<< t_finding_cycles<<"ms."<<std::endl;
			delete solver;
			return 1;
		}
		colouring = read_sol(G,k, solver);

		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/
		t_before_cycle = std::clock();
		no_cycle = check_colouring_oc(G, k, colouring, solver);
		t_after_cycle = std::clock();
		t_finding_cycles = t_finding_cycles + 1000.0*(t_after_cycle - t_before_cycle)/CLOCKS_PER_SEC;
		counter++;
	}
	std::cout<<"Acyclic colouring found!"<<std::endl;
	std::cout<<counter<<" calls to solver. Time spent on solver: "<< t_solving<<"ms. Time spent finding cycles: "<< t_finding_cycles<<"ms."<<std::endl;
	delete solver;
	/*std::cout<<"The colouring is:";
	for(int i=0;i<G.ncount();i++){
		std::cout<<" "<<colouring[i];
	}
	std::cout<<std::endl;*/
	return 0;
	
}
