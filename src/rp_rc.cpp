#include "rp_rc.hpp"
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

void Representatives::initialize_solver_rc(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables){
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
				if(std::find(neighbors[u].begin(),neighbors[u].end(),v)==neighbors[u].end() && std::find(neighbors[u].begin(),neighbors[u].end(),w)==neighbors[u].end()){//v and w are not neighbors of u
					solver->clause(-variables[u][v],-variables[u][w]); //u cannot represent two adjacent vertices
					//std::cout<<-variables[u][w] <<" "<<-variables[u][v]<<std::endl;
				}
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
	for(auto cycle : found_cycles){
		if(cycle[0]<k  && cycle[0]!=-1){
			for(int i=1; i<cycle.size();i++){
				solver->add(i);
			}
			solver->add(0);
		}
	}
}

std::vector<int> Representatives::read_sol(Graph::Graph &G, int k, CaDiCaL::Solver *solver, std::vector<std::vector<int>> &variables,std::vector<int> &representatives){
	std::vector<int> colours = std::vector<int>(G.ncount());
	representatives = std::vector<int>(G.ncount());
	int count = 0;
	std::vector<std::vector<int>> neighbors = G.get_neighbor_list();
	//colour each representative a different colour
	for(int i=0;i<G.ncount();i++){
		if((solver->val(i+1))==(i+1)){
			representatives[i]=i;
			colours[i]=count;
			//std::cout<<i<<" has been given colour "<<count<<std::endl;
			count++;
		}
		else{
			std::sort(neighbors[i].begin(),neighbors[i].end());
			int next_neighbor_index = 0;
			for(int j=0;j<i;j++){
				if(j != neighbors[i][next_neighbor_index]){
					//j is not a neighbor of i and smaller so a possible representative
					if(solver->val(variables[j][i])==variables[j][i]){ //j is representative of i (since j<i, j already has a colour)
					representatives[i] = j;
					colours[i]=colours[j];
					//std::cout<<i<<" has been given colour "<<colours[j]<<std::endl;
					//std::cout<<representatives[i]<< " represents "<<i<<std::endl;
					break;
				}
				}
				else if(next_neighbor_index<neighbors[i].size()-1){
					next_neighbor_index++;
				}
			}
		}
	}
	//std::cout<<"there are "<<count<<" colour classes"<<std::endl;
    return colours;
}

bool Representatives::try_sol(Graph::Graph &G, int k, std::vector<int> colour){
	std::vector<int> edges = G.elist();
	for(int i=0;i<G.ecount();i++){
		if(colour[edges[2*i]]>=k || colour[edges[2*i+1]]>=k){
			std::runtime_error("The colouring uses more colours");
			return false;
		}
		if(colour[edges[2*i]]==colour[edges[2*i+1]]){
			std::runtime_error("The colouring is not valid");
			return false;
		}
	}
	return true;
}

void Representatives::find_cycle_rc(Graph::Graph &G, int k, std::vector<int> colour, std::vector<std::vector<int>> &cycles){
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

void Representatives::update_sat_rc(int k,std::vector<int> &colour, std::vector<std::vector<int>> &cycles, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables, std::vector<int> &representatives){
	//forbid this specific colouring
	//std::cout<<"adding clause: ";
	for(std::vector<int> cycle : cycles){
		std::vector<int> new_clause;
		int max_colour = std::max(colour[cycle[0]],colour[cycle[1]]);
		new_clause.push_back(max_colour);
		for(int i=0;i<cycle.size();i++){
			if(representatives[cycle[i]]==cycle[i]){
				solver->add(-cycle[i]-1);
				new_clause.push_back(-cycle[i]-1);
				//std::cout<<-cycle[i]-1<<" ";
			}
			else{
				solver->add(-variables[representatives[cycle[i]]][cycle[i]]);
				//std::cout<<-variables[representatives[cycle[i]]][cycle[i]]<<" ";
				new_clause.push_back(-variables[representatives[cycle[i]]][cycle[i]]);
			}
		}
		//std::cout<<std::endl;
		solver->add(0);
		if(max_colour<k-1){
			found_cycles.push_back(new_clause);
		}
	}
}

bool Representatives::check_colouring_rc(Graph::Graph &G, int k, std::vector<int> colour, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables, std::vector<int> &representatives){
	std::vector<std::vector<int>> cycles = std::vector<std::vector<int>>(0);
	find_cycle_rc(G,k,colour, cycles);
	if(cycles.empty()){return true;} //no cycle found
	
	/*std::cout<<"Found cycle: ";
	for(std::vector<int> cycle : cycles){
		for(int i=0;i<cycle.size();i++){
			std::cout<<cycle[i]+1<<" ";
		}
		std::cout<<std::endl;
	}*/

	update_sat_rc(k,colour,cycles,solver, found_cycles, variables, representatives);
	return false;
}

int Representatives::solve_rc(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
try{
	//saves the number given to the variable standing for i is representative of j (i<j)
	std::vector<std::vector<int>> variables = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
	initialize_solver_rc(G,k, solver, previous_cycles, variables);
	
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
