#include "as_mc.hpp"
#include "as_oc.hpp"
#include "as_rc.hpp"
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

void Assignment::initialize_solver_rc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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
	//adjacent vertices shouldn't have the same colour
	std::vector<Graph::VertexType> edges = G.elist();
	for (int i=0;i<G.ecount();i++){
		int index_v = edges[2*i] *k+1;
		int index_w = edges[2*i+1] *k+1;
		for(int j=0;j<k;j++){
			solver->clause(-index_v-j,-index_w-j);
		}
	}

	//adding all the previous found problem cycles
	for(auto &cycle : found_cycles){
		if(cycle[0]<k && cycle[0]!=-1){
			for(int i=1; i<cycle.size();i++){
				//std::cout<<"cycle before: "<<cycle[i];
				int col_v = (abs(cycle[i])-1)%(k+1)+1;
                int v = (abs(cycle[i])-col_v)/(k+1);
				if(std::signbit(cycle[i])==0){
					cycle[i]=abs(cycle[i])-v;
				}
				else{
					cycle[i]=-(abs(cycle[i])-v);
				}
				//std::cout<<"cycle after: "<<cycle[i]<<std::endl;
				solver->add(cycle[i]);
			}
			solver->add(0);
		}
	}
}

void Assignment::update_sat_rc(int k, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
	//adding the constraints forbidding any colour permutation of that cycle
	for(std::vector<int> cycle : cycles){//for every found cycle
		for(int j=0;j<k;j++){//for any two colours not equal
			for(int l=0;l<k;l++){
				if(l!=j){
					std::vector<int> new_clause;
					int max_colour = std::max(j,l);
					new_clause.push_back(max_colour);
					solver->add(-(cycle[0]*k+1+j));//the variable for the first vertex in cycle and the colour j
					new_clause.push_back(-(cycle[0]*k+1+j));
					//std::cout<<"adding clause: "<<-(cycle[0]*k+1)-j;
					for(int a=1;a<cycle.size()/2;a++){//for the rest
						if(a%2==1){
							solver->add(-(cycle[2*a]*k+1+l)),solver->add(-(cycle[2*a-1]*k+1+l));
							new_clause.push_back(-(cycle[2*a]*k+1+l));
							new_clause.push_back(-(cycle[2*a-1]*k+1+l));
							//std::cout<<", "<<-(cycle[2*a]*k+1)-l<<", "<<-(cycle[2*a-1]*k+1+l);
						}
						if(a%2==0){
							solver->add(-(cycle[2*a]*k+1+j)),solver->add(-(cycle[2*a-1]*k+1+j));
							new_clause.push_back(-(cycle[2*a]*k+1+j));
							new_clause.push_back(-(cycle[2*a-1]*k+1+j));
							//std::cout<<", "<<-(cycle[2*a]*k+1)-j<<", "<<-(cycle[2*a-1]*k+1)-j;
						}
					}
					if(cycle.size()%4==0){
						solver->add(-(cycle[cycle.size()-1]*k+1+j)), solver->add(0);
						new_clause.push_back(-(cycle[cycle.size()-1]*k+1+j));
						//std::cout<<", "<<-(cycle[cycle.size()-1]*k+1)-j<<std::endl;
					}
					else{
						solver->add(-(cycle[cycle.size()-1]*k+1+l)), solver->add(0);
						new_clause.push_back(-(cycle[cycle.size()-1]*k+1+l));
						//std::cout<<", "<<-(cycle[cycle.size()-1]*k+1)-l<<std::endl;
					}
					if(max_colour<k-1){
						found_cycles.push_back(new_clause);
					}
				}
			}
		}
	}
}


bool Assignment::check_colouring_rc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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

	update_sat_rc(k,cycles,solver,found_cycles);
	return false;
}

int Assignment::solve_rc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
try{	
	initialize_solver_rc(G,k, solver, previous_cycles);
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

		no_cycle = check_colouring_rc(G, k, colouring, solver, previous_cycles);
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