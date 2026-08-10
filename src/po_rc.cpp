#include "po_rc.hpp"
#include "po_oc.hpp"
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

void PartialOrder::initialize_solver_rc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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
	//adding all the previous found problem cycles
	for(auto cycle : found_cycles){
		if(cycle[0]<k){
			for(int i=1; i<cycle.size();i++){
				solver->add(i);
			}
			solver->add(0);
		}
	}
}

void PartialOrder::update_sat_rc(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
	for(std::vector<int> cycle : cycles){
		std::vector<int> new_clause;
		int max_colour = std::max(colour[cycle[0]],colour[cycle[1]]);
		new_clause.push_back(max_colour);
		for(int i=0;i<cycle.size();i++){
			if(colour[cycle[i]]>0){
				solver->add(-(cycle[i]*k+colour[cycle[i]])),solver->add(cycle[i]*k+colour[cycle[i]]+1);
				new_clause.push_back(-(cycle[i]*k+colour[cycle[i]]));
				new_clause.push_back(cycle[i]*k+colour[cycle[i]]+1);
				//std::cout<<-(cycle[i]*k+colour[cycle[i]])<<" , "<<cycle[i]*k+colour[cycle[i]]+1<<" , ";
			}
			else{
				solver->add(cycle[i]*k+1);
				new_clause.push_back(cycle[i]*k+1);
				//std::cout<<cycle[i]*k+colour[cycle[i]]+1<<" , ";
			}
		}
		//std::cout<<std::endl;
		solver->add(0);
		if(max_colour<k-1){
			found_cycles.push_back(new_clause);
		}
	}
}

bool PartialOrder::check_colouring_rc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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

	update_sat_rc(k,colour,cycles,solver, found_cycles);
	return false;
}

int PartialOrder::solve_rc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	initialize_solver_rc(G,k, solver, previous_cycles);
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
		if(try_sol(G,k,colouring)==false){
			delete solver;
			std::__throw_runtime_error("The colouring is not valid.");
		}
		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/
		t_before_cycle = std::clock();
		no_cycle = check_colouring_rc(G, k, colouring, solver, previous_cycles);
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
