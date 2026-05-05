#include "../Graph.h"
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
		int index=i*k+1;
		for(int j=0;j<k;j++){
			solver->add (index);
			index++;
		}
		solver->add(0);
	}
	//each vertex may only have one colour
	for(int i=0; i<n;i++){
		int index=i*k+1;
		for(int j=0;j<k;j++){
			for(int l=j+1;l<k;l++){
				solver->clause(-index-j,-index-l);
			}
		}
	}
	//adjacent vertices shouldn't have the same colour
	std::vector<Graph::VertexType> edges = G.elist();
	for (int i=0;i<G.ecount();i++){
		int index_v = edges[2*i] *k+1;
		int index_w = edges[2*i+1] *k+1;
		for(int j=0;j<k;j++){
			solver->clause(-index_v-j,-index_w-j);
		}
	}
}


std::vector<int> read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver){
	std::vector<int> colours = std::vector<int>(G.ncount());
	int res;
	//find out which colours were assigned
	for(int i=0; i<G.ncount();i++){
		for(int j=0;j<k;j++){
			res = solver->val(i*k+1+j);
			if(res == i*k+1+j){
				colours[i]=j;
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

void update_sat(int k, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver){
	//adding the constraints forbidding any colour permutation of that cycle
	for(std::vector<int> cycle : cycles){//for every found cycle
		for(int j=0;j<k;j++){//for any two colours not equal
			for(int l=0;l<k;l++){
				if(l!=j){
					solver->add(-(cycle[0]*k+1+j));//the variable for the first vertex in cycle and the colour j
					//std::cout<<"adding clause: "<<-(cycle[0]*k+1)-j;
					for(int a=1;a<cycle.size()/2;a++){//for the rest
						if(a%2==1){
							solver->add(-(cycle[2*a]*k+1+l)),solver->add(-(cycle[2*a-1]*k+1+l));
							//std::cout<<", "<<-(cycle[2*a]*k+1)-l<<", "<<-(cycle[2*a-1]*k+1+l);
						}
						if(a%2==0){
							solver->add(-(cycle[2*a]*k+1+j)),solver->add(-(cycle[2*a-1]*k+1+j));
							//std::cout<<", "<<-(cycle[2*a]*k+1)-j<<", "<<-(cycle[2*a-1]*k+1)-j;
						}
					}
					if(cycle.size()%4==0){
						solver->add(-(cycle[cycle.size()-1]*k+1+j)), solver->add(0);
						//std::cout<<", "<<-(cycle[cycle.size()-1]*k+1)-j<<std::endl;
					}
					else{
						solver->add(-(cycle[cycle.size()-1]*k+1+l)), solver->add(0);
						//std::cout<<", "<<-(cycle[cycle.size()-1]*k+1)-l<<std::endl;
					}
				}
			}
		}
	}
}

bool check_colouring(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver){
	std::vector<std::vector<int>> cycles = find_cycle(G,k,colour);
	if(cycles.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycles: ";
	for(std::vector<int> cycle: cycles){
		for(int i=0;i<cycle.size();i++){
			std::cout<<cycle[i]+1<<" ";
		}
		std::cout<<std::endl;
	}*/

	update_sat(k,cycles,solver);
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
