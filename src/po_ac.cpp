#include "po_ac.hpp"
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

void PartialOrder::update_sat_ac(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
	for(std::vector<int> cycle : cycles){
		for(int a=0;a<k;a++){
			for(int b=0;b<k;b++){
				if(a!=b){
					std::vector<int> new_clause;
					int max_colour = std::max(a,b);
					new_clause.push_back(max_colour);
					if(a>0){
							solver->add(-(cycle[0]*k+a)),solver->add(cycle[0]*k+a+1);
							new_clause.push_back(-(cycle[0]*k+a));
							new_clause.push_back(cycle[0]*k+a+1);
							//std::cout<<-(cycle[0]*k+a)<<" , "<<cycle[0]*k+a+1<<" , ";
						}
						else{
							solver->add(cycle[0]*k+1);
							new_clause.push_back(cycle[0]*k+1);
							//std::cout<<cycle[0]*k+1<<" , ";
						}
					for(int i=1;i<cycle.size()-1;i+=2){
						if((i/2)%2){
							if(a>0){
								solver->add(-(cycle[i]*k+a)),solver->add(cycle[i]*k+a+1);
								new_clause.push_back(-(cycle[i]*k+a));
								new_clause.push_back(cycle[i]*k+a+1);
								solver->add(-(cycle[i+1]*k+a)),solver->add(cycle[i+1]*k+a+1);
								new_clause.push_back(-(cycle[i+1]*k+a));
								new_clause.push_back(cycle[i+1]*k+a+1);
								//std::cout<<-(cycle[i]*k+a)<<" , "<<cycle[i]*k+a+1<<" , "<<-(cycle[i+1]*k+a)<<" , "<<cycle[i+1]*k+a+1<<" , ";
							}
							else{
								solver->add(cycle[i]*k+1);
								new_clause.push_back(cycle[i]*k+1);
								solver->add(cycle[i+1]*k+1);
								new_clause.push_back(cycle[i+1]*k+1);
								//std::cout<<cycle[i]*k+1<<" , "<<cycle[i+1]*k+1<<" , ";
							}
						}
						else{
							if(b>0){
								solver->add(-(cycle[i]*k+b)),solver->add(cycle[i]*k+b+1);
								new_clause.push_back(-(cycle[i]*k+b));
								new_clause.push_back(cycle[i]*k+b+1);
								solver->add(-(cycle[i+1]*k+b)),solver->add(cycle[i+1]*k+b+1);
								new_clause.push_back(-(cycle[i+1]*k+b));
								new_clause.push_back(cycle[i+1]*k+b+1);
								//std::cout<<-(cycle[i]*k+b)<<" , "<<cycle[i]*k+b+1<<" , "<<-(cycle[i+1]*k+b)<<" , "<<cycle[i+1]*k+b+1<<" , ";
							}
							else{
								solver->add(cycle[i]*k+1);
								new_clause.push_back(cycle[i]*k+1);
								solver->add(cycle[i+1]*k+1);
								new_clause.push_back(cycle[i+1]*k+1);
								//std::cout<<cycle[i]*k+1<<" , "<<cycle[i+1]*k+1<<" , ";
							}
						}
						if(a>0){
							solver->add(-(cycle[cycle.size()-1]*k+a)),solver->add(cycle[cycle.size()-1]*k+a+1);
							new_clause.push_back(-(cycle[cycle.size()-1]*k+a));
							new_clause.push_back(cycle[cycle.size()-1]*k+a+1);
							//std::cout<<-(cycle[cycle.size()-1]*k+a)<<" , "<<cycle[cycle.size()-1]*k+a+1<<std::endl;
						}
						else{
							solver->add(cycle[cycle.size()-1]*k+1);
							new_clause.push_back(cycle[cycle.size()-1]*k+1);
							//std::cout<<cycle[cycle.size()-1]*k+1<<std::endl;
						}
					}
					//std::cout<<std::endl;
					solver->add(0);
					if(max_colour<k-1){
						found_cycles.push_back(new_clause);
					}
				}
			}
		}
	}
}

bool PartialOrder::check_colouring_ac(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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

	update_sat_ac(k,colour,cycles,solver, found_cycles);
	return false;
}

int PartialOrder::solve_ac(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

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
		no_cycle = check_colouring_ac(G, k, colouring, solver, previous_cycles);
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
