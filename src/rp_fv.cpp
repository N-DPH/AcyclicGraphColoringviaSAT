#include "rp_rc.hpp"
#include "rp_fv.hpp"
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

void Representatives::initialize_solver_fv(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables){
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
        if(!neighbors[i].empty()){
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
        }
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
}

void Representatives::find_cycle_fv(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles){
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
							cycles.push_back(cycle);
							return;//should work because we only have even cycles
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
	return;
}

void Representatives::update_sat_fv(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables, std::vector<int> &representatives){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
	//we need at least 2 colors for any of these to work
	std::vector<int> new_found = {2};
	for(std::vector<int> &cycle : cycles){
		std::vector<int> new_clause;
		//std::cout<<"pushed cycle: ";
		for(int i=0;i<cycle.size();i++){
			if(representatives[cycle[i]]==cycle[i]){
				solver->add(-cycle[i]-1);
				//this will be added again next k. "-" means this was a representative
				new_found.push_back(-cycle[i]-1);
				//std::cout<<-cycle[i]-1<<", ";

			}
			else{
				solver->add(-variables[representatives[cycle[i]]][cycle[i]]);
				//this will be added again next k. "+" means the first one is representing the second
				
				new_found.push_back(representatives[cycle[i]]);
				new_found.push_back(cycle[i]);
				//std::cout<<representatives[cycle[i]]<<", "<<cycle[i]<<", ";
			}
		}
		//std::cout<<std::endl;
		solver->add(0);
		found_cycles.push_back(new_found);
	}
}

bool Representatives::check_colouring_fv(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables, std::vector<int> &representatives){
	std::vector<std::vector<int>> cycles = std::vector<std::vector<int>>(0);
	find_cycle_fv(G,k,colour, cycles);
	if(cycles.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycle: ";
	for(std::vector<int> cycle : cycles){
		for(int i=0;i<cycle.size();i++){
			std::cout<<cycle[i]+1<<" ";
		}
		std::cout<<std::endl;
	}*/

	update_sat_fv(k,colour,cycles,solver, found_cycles, variables, representatives);
	return false;
}

int Representatives::solve_fv(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
try{
	//saves the number given to the variable standing for i is representative of j (i<j)
	std::vector<std::vector<int>> variables = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
	initialize_solver_fv(G,k, solver, previous_cycles, variables);
	
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
			throw std::runtime_error("The colouring is not valid.");
		}
		/*std::cout<<"The colouring so far:";
		for(int i=0;i<G.ncount();i++){
			std::cout<<" "<<colouring[i];
		}
		std::cout<<std::endl;*/
		t_before_cycle = std::clock();
		no_cycle = check_colouring_fv(G, k, colouring, solver, previous_cycles, variables, representatives);
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
