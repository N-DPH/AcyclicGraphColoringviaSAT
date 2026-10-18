#include "as_mc.hpp"
#include "as_sc.hpp"
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

void Assignment::initialize_solver_sc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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

	for(auto &cycle : found_cycles){
		if(cycle[0]==-1){
			for(int i=1;i<=k;i++){
				for(int j=1;j<=k;j++){
					if(i!=j){
						std::vector<int> clause = {-(cycle[1]*k+i),-(cycle[2]*k+j),-(cycle[3]*k+j),-(cycle[4]*k+i)};
						solver->clause(clause);
					}
				}
			}
		}
	}
}

int Assignment::solve_sc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
try{	
	initialize_solver_sc(G,k, solver, previous_cycles);
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