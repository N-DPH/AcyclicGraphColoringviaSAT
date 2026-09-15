#include "rp_rc.hpp"
#include "rp_sc.hpp"
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

void Representatives::initialize_solver_sc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables){
	int n = G.ncount();
	int num_var = n+n*(k+1); //first the variables that say if v is a representative and then n*(k-1) to mean that there should only be k colour classes
	solver->declare_more_variables(num_var);
	std::vector<int> not_neighbors = G.complement_elist();
	for(int i=0;i<not_neighbors.size();i+=2){
		if(not_neighbors[i]<not_neighbors[i+1]){
			variables[not_neighbors[i]][not_neighbors[i+1]]=solver->declare_one_more_variable(); //this is the variable for not_neighbors[i] is representative of not_neighbors[i+1]
			solver->clause(not_neighbors[i]+1,-(variables[not_neighbors[i]][not_neighbors[i+1]])); //but u can only represent v if it is indeed a representative
			//std::cout<<not_neighbors[i]<<" represents "<<not_neighbors[i+1]<<" if "<<variables[not_neighbors[i]][not_neighbors[i+1]]<<std::endl;
		}
		else{
			variables[not_neighbors[i+1]][not_neighbors[i]]=solver->declare_one_more_variable();
			solver->clause(not_neighbors[i+1]+1,-(variables[not_neighbors[i+1]][not_neighbors[i]]));
			//std::cout<<not_neighbors[i+1]<<" represents "<<not_neighbors[i]<<" if "<<variables[not_neighbors[i+1]][not_neighbors[i]]<<std::endl;
		}
	}	

	
	std::vector<std::vector<int>> neighbors = G.get_neighbor_list();

	//say that each vertex needs a representative
	for(int i=0; i<n;i++){
		solver->add(i+1);//if i is a representative
		//std::cout<<i+1<<" ";
		std::sort(neighbors[i].begin(),neighbors[i].end());
		int next_neighbor_index = 0;
		for(int j=0;j<i;j++){
			if(j != neighbors[i][next_neighbor_index]){
				//j is not a neighbor of i and smaller so a possible representative
				solver->add(variables[j][i]);
				//std::cout<<variables[j][i]<<" ";
			}
			else if(next_neighbor_index<neighbors[i].size()-1){
				next_neighbor_index++;
			}
		}
		solver->add(0);
		//std::cout<<std::endl;
	}

	std::vector<int> elist = G.elist();
	//adjacent vertices can't have the same representative
	for(int u=0;u<n;u++){ //for all u
		for(int i=0;i<elist.size();i+=2){
			int v = elist[i];
			int w = elist[i+1];
			if(u<v && u<w && std::find(neighbors[v].begin(),neighbors[v].end(),u)==neighbors[v].end() && std::find(neighbors[w].begin(),neighbors[w].end(),u)==neighbors[w].end()){ //u could represent v and w
				solver->clause(-variables[u][v],-variables[u][w]); //u cannot represent two adjacent vertices
				//std::cout<<-variables[u][w] <<" "<<-variables[u][v]<<std::endl;
				solver->clause(-variables[u][v],u+1);//if one of them is represented by u, then u needs to be a representative
				solver->clause(-variables[u][w],u+1);
			}
		}
	}

	//there should only be k colour classes
	solver->clause(-1,n+1); 
	//std::cout<<-1<<" "<<n+1<<std::endl;
	for(int j=2;j<=k+1;j++){
		solver->clause(-(n+j));
		//std::cout<<-(n+j)<<std::endl;
	}
	for(int i=2;i<=n;i++){
		solver->clause(n+(i-1)*(k+1)+1,-(n+(i-2)*(k+1)+1));
		solver->clause(n+(i-1)*(k+1)+1,-i);
		//std::cout<<n+(i-1)*(k+1)+1<<" "<<-(n+(i-2)*(k+1)+1)<<std::endl;
		//std::cout<<n+(i-1)*(k+1)+1<<" "<<-i<<std::endl;
	}
	for(int i=2;i<=n;i++){
		for(int j=2;j<=k+1;j++){
			solver->clause(n+(i-1)*(k+1)+j,-(n+(i-2)*(k+1)+j));
			solver->clause(n+(i-1)*(k+1)+j,-(n+(i-2)*(k+1)+j-1),-i);
			//std::cout<<n+(i-1)*(k+1)+j<<" "<<-(n+(i-2)*(k+1)+j)<<std::endl;
			//std::cout<<n+(i-1)*(k+1)+j<<" "<<-(n+(i-2)*(k+1)+j-1)<<" "<<-i<<std::endl;
		}
	}
	solver->clause(-(n+(n-1)*(k+1)+k+1));
	//std::cout<<-(n+(n-1)*(k+1)+k+1)<<std::endl;

	//adding all the previous found problem cycles
	for(auto &cycle : found_cycles){
		if(cycle[0]<k  && cycle[0]!=-1){
			//std::cout<<"adding cycle: ";
			for(int i=1; i<cycle.size();i++){
				if(cycle[i]<0){
					solver->add(cycle[i]);
					//std::cout<<cycle[i]<<", ";
				}
				else{
					solver->add(-variables[cycle[i]][cycle[i+1]]);
					//std::cout<<cycle[i]<<", "<<cycle[i+1]<<", ";
					i++;
				}
			}
			solver->add(0);
			//std::cout<<std::endl;
		}
	}

	for(auto cycle : found_cycles){
		if(cycle[0]==-1){
			int min_rep = std::min(cycle[2],cycle[3]);
			int represented = std::max(cycle[2],cycle[3]);
			solver->clause(-(cycle[1]+1),-(min_rep+1),-(variables[min_rep][represented]),-(variables[cycle[1]][cycle[4]]));
			for(int i=0;i<cycle[0];i++){ //for every i that could be representative of the colour class of cycle[1] and cycle[4]
				//if it actually can be i.e. is not neighbor of either
				if(std::find(neighbors[i].begin(),neighbors[i].end(),cycle[1])==neighbors[i].end() && std::find(neighbors[i].begin(),neighbors[i].end(),cycle[4])==neighbors[i].end()){
					solver->clause(-(variables[i][cycle[1]]),-(min_rep+1),-(variables[min_rep][represented]),-(variables[i][cycle[4]]));
					for(int j=0;j<min_rep;j++){
						if(i!=j && std::find(neighbors[j].begin(),neighbors[j].end(),cycle[2])==neighbors[j].end() && std::find(neighbors[j].begin(),neighbors[j].end(),cycle[3])==neighbors[j].end()){
							solver->clause(-(variables[i][cycle[1]]),-(variables[j][cycle[2]]),-(variables[j][cycle[3]]),-(variables[i][cycle[4]]));
						}
					}
				}
			}
			for(int j=0;j<min_rep;j++){
				if(std::find(neighbors[j].begin(),neighbors[j].end(),cycle[2])==neighbors[j].end() && std::find(neighbors[j].begin(),neighbors[j].end(),cycle[3])==neighbors[j].end()){
					solver->clause(-(cycle[1]+1),-(variables[j][cycle[2]]),-(variables[j][cycle[3]]),-(variables[cycle[1]][cycle[4]]));
				}
			}
		}
	}
}


int Representatives::solve_sc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
try{
	//saves the number given to the variable standing for i is representative of j (i<j)
	std::vector<std::vector<int>> variables = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
	initialize_solver_sc(G,k, solver, previous_cycles, variables);
	
	int res;
	bool no_cycle = false;
	std::vector<int> colouring;
	std::vector<int> representatives;

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
		colouring = read_sol(G,k, solver, variables,representatives);
		if(try_sol(G,k,colouring)==false){
			delete solver;
			std::runtime_error("The colouring is not valid.");
		}
		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/
		t_before_cycle = std::clock();
		no_cycle = check_colouring_rc(G, k, colouring, solver, previous_cycles, variables, representatives);
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
