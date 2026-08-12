#include "prop_rp.hpp"
#include "../cadical/src/cadical.hpp"
#include "Graph.h"
#include <stack>
#include <vector>
#include <map>
#include <utility>
#include <cmath>
#include <queue>
#include <chrono>
#include <stdexcept>


void Propagator_rp::initialize_solver(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables){
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
				solver->add(cycle[i]);
			}
			solver->add(0);
		}
	}
}

class ActivePropagator : CaDiCaL::ExternalPropagator {
private:
    CaDiCaL::Solver *solver;

    Graph::Graph G;
    int k;
    std::vector<std::vector<int>> variables;
    std::map<int,std::pair<int,int>> variables_back;
    std::stack<std::vector<int>> observed_trail;
    //0 is not yet assigned, 1 is true, -1 if not
    std::vector<int> assignment_is_rep;
    std::vector<std::vector<int>> assignment_represented;
    //contains the cycles already as logic encoding
    std::vector<std::vector<int>> cycles;
    std::vector<int> clause;

public:
    ActivePropagator (CaDiCaL::Solver *s, Graph::Graph &G_new, int k_new, std::vector<std::vector<int>> var)
        : solver (s), G(G_new), k(k_new), variables(var) {
        is_lazy = false;

        // In this example we can always cheaply re-add external clauses if
        // necessary, so we allow the SAT solver to consider them during clause
        // database reduction.
        are_reasons_forgettable = false;

        solver->connect_external_propagator (this);

        // This propagator ignores the FixedAssignments and relies only on
        // the standard notification messages (which might report to unassign
        // and then later reassing an actually fixed variable).
        //std::cout << "[act-prop] ActivePropagator is created and connected as ""an ExternalPropagator"<< std::endl;

        assignment_is_rep = std::vector<int>(G.ncount(),0);
        assignment_represented = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
        variables_back = std::map<int, std::pair<int,int>>();
        
        // Create the root-level of the trail. This level needs to be always
        // there.
        observed_trail.push(std::vector<int> ());
    }

    void observe(){
        //Observe all variables of the encoding
        for(int i=0;i<G.ncount();i++){
            solver->add_observed_var(i+1);
        }
        std::vector<int> not_neighbors = G.complement_elist();
        for(int i=0;i<not_neighbors.size();i+=2){
            if(not_neighbors[i]<not_neighbors[i+1]){
                solver->add_observed_var(variables[not_neighbors[i]][not_neighbors[i+1]]);
                variables_back[variables[not_neighbors[i]][not_neighbors[i+1]]]=std::make_pair(not_neighbors[i],not_neighbors[i+1]);
            }
            else{
                solver->add_observed_var(variables[not_neighbors[i+1]][not_neighbors[i]]);
                variables_back[variables[not_neighbors[i+1]][not_neighbors[i]]]=std::make_pair(not_neighbors[i+1],not_neighbors[i]);
            }
        }
        return;
    }

    ~ActivePropagator () {
        //std::cout << "[act-prop][~ActivePropagator starts]" << std::endl;
        solver->disconnect_external_propagator ();
        //std::cout << "[act-prop] ActivePropagator is disconnected." << std::endl;
    }

    bool try_sol(std::vector<int> colour){
        std::vector<int> edges = G.elist();
        for(int i=0;i<G.ecount();i++){
            if(colour[edges[2*i]]>=k || colour[edges[2*i+1]]>=k){
                throw std::runtime_error("The colouring usese more than k colours");
                return false;
            }
            if(colour[edges[2*i]]==colour[edges[2*i+1]]){
                throw std::runtime_error("The colouring is not valid");
                return false;
            }
        }
        return true;
    }

    std::vector<std::vector<int>> find_cycles(std::vector<int> colour){
        std::vector<std::vector<int>> new_cycles;
        std::vector<std::tuple<int,int,int>> counting_pairs = std::vector<std::tuple<int,int,int>>(k*k,std::make_tuple(0,0,-1));
        std::vector<Graph::VertexType> edges = G.elist();
        Graph::NeighborList neighbors = G.get_neighbor_list();
        for(int i=0; i<G.ecount();i++){
            if(colour[edges[2*i]]!=-1 && colour[edges[2*i+1]]!=-1){
                int lower_colour = std::min(colour[edges[2*i]],colour[edges[2*i+1]]);
                int higher_colour = std::max(colour[edges[2*i]],colour[edges[2*i+1]]);
                if(std::get<2>(counting_pairs[lower_colour*k+higher_colour])==-1){
                    counting_pairs[lower_colour*k+higher_colour] = std::make_tuple(lower_colour,higher_colour,1);
                }
                else{
                    counting_pairs[lower_colour*k+higher_colour] = std::make_tuple(lower_colour,higher_colour,std::get<2>(counting_pairs[lower_colour*k+higher_colour])+1);
                }
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
                    if(colour[i]!=-1 && !in_cycle[i] && !visited[i] && (colour[i]==std::get<0>(p) || colour[i]==std::get<1>(p))){
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
                                    new_cycles.push_back(cycle);//should work because we only have even cycles
                                    //resetting everything so that we can find further cycles
                                    while(!q.empty()){
                                        v=q.front();
                                        q.pop();
                                    }
                                    std::fill(visited.begin(), visited.end(),false);
                                    std::fill(parent.begin(),parent.end(),-1);
                                    break;
                                }
                                if(colour[w]!=-1 && !in_cycle[w] && w != parent[v] && (colour[w]==std::get<0>(p) || colour[w]==std::get<1>(p))){
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
    return new_cycles;
    }

    bool check_sol(const std::vector<int> &model){
        std::vector<int> colours = std::vector<int>(G.ncount(),-1);
        std::vector<int> representatives = std::vector<int>(G.ncount());
        int count = 0;
        for(int i=0;i<model.size();i++){
            if(model[i]<=G.ncount() && model[i]>0){//model i is one of the representants
                representatives[model[i]-1]=model[i]-1;
                colours[model[i]-1]=count;
                count++;
            }
            else{
                if(model[i]>0 && colours[variables_back[model[i]].second]==-1){
                    representatives[variables_back[model[i]].second]=variables_back[model[i]].first;
                    colours[variables_back[model[i]].second]=colours[variables_back[model[i]].first];
                }
            }
        }
        if(try_sol(colours)==false){
            return false;
        }
        std::vector<std::vector<int>> new_cycles = find_cycles(colours);
        for(auto cycle : new_cycles){
            std::vector<int> new_clause;
            //std::cout<<"Model not valid: found cycle ";
            for(int i=0;i<cycle.size();i++){
                if(representatives[cycle[i]]==cycle[i]){
                    new_clause.push_back(-cycle[i]-1);
                    //std::cout<<-cycle[i]-1<<" ";
                }
                else{
                    new_clause.push_back(-variables[representatives[cycle[i]]][cycle[i]]);
                    //std::cout<<-variables[representatives[cycle[i]]][cycle[i]]<<" ";
                }
            }
            //std::cout<<std::endl;
            cycles.push_back(new_clause);
        }
        return new_cycles.empty(); //returns true iff the assignment is valid and cycle free
    }

    bool check_partial_sol(){
        std::vector<int> colours = std::vector<int>(G.ncount(),-1);
        std::vector<int> representatives = std::vector<int>(G.ncount());
        int count = 0;

        //colour each representative a different colour
        for(int i=0;i<G.ncount();i++){
            if((assignment_is_rep[i])==1){
                representatives[i]=i;
                colours[i]=count;
                //std::cout<<i<<" has been given colour "<<count<<std::endl;
                count++;
            }
            else{
                for(int j=0;j<i;j++){
                    if(assignment_represented[j][i]==1){ //j is representative of i (since j<i, j already has a colour)
                        representatives[i] = j;
                        colours[i]=colours[j];
                        //std::cout<<i<<" has been given colour "<<colours[j]<<std::endl;
                        //std::cout<<representatives[i]<< " represents "<<i<<std::endl;
                        break;
                    }
                }
            }
        }
        std::vector<std::vector<int>> new_cycles = find_cycles(colours);
        for(auto cycle : new_cycles){
            std::vector<int> new_clause;
            for(int i=0;i<cycle.size();i++){
                if(representatives[cycle[i]]==cycle[i]){
                    new_clause.push_back(-cycle[i]-1);
                }
                else{
                    new_clause.push_back(-variables[representatives[cycle[i]]][cycle[i]]);
                }
            }
            cycles.push_back(new_clause);
        }
        return new_cycles.empty(); //returns true iff the assignment is valid and cycle free
    }


    void notify_assignment (const std::vector<int> &lits) override {
        // We update the assignment states of the propagator, but in this
        // example we do not check eagerly if there is any possible conflict or
        // propagation (this will be checked only when it is relevant, ie., in
        // 'cb_propagate' and 'cb_has_external_clause').
        //std::cout << "[act-prop][notify_assignment][ ";
        for (const auto lit : lits) {
            //std::cout << lit << " ";

            // Add the new assignment to the observed trail
            observed_trail.top().push_back(lit);

            // Update the maps based on the assignment
            if(lit<=G.ncount() && lit >=-G.ncount()){
                assignment_is_rep[abs(lit)-1]=1-std::signbit(lit);
            }
            else{
                assignment_represented[variables_back[abs(lit)].first][variables_back[abs(lit)].second]=1-std::signbit(lit);
            }
        }
        //std::cout << "]" << std::endl;
        return;
    }

    

    void notify_new_decision_level () override {
        //std::cout << "[act-prop][notify_new_decision_level]" << std::endl;

        // Create a new decision level on the observed trail
        observed_trail.push(std::vector<int> ());
    }

    void notify_backtrack (size_t new_level) override {
        //std::cout << "[act-prop][notify_backtrack][ "<< observed_trail.size () - 1 << " -> " << new_level << " ]"<< "[unassigned: ";

        // Undo all the backtracked assignments on the observed trail and
        // in the assignment maps.
        while (observed_trail.size () > new_level + 1) {
            for (auto lit : observed_trail.top ()) {
                //std::cout << lit << " ";

                if(lit<=G.ncount() && lit >=-G.ncount()){
                    assignment_is_rep[abs(lit)-1]=0;
                }
                else{
                    assignment_represented[variables_back[abs(lit)].first][variables_back[abs(lit)].second]=0;
                }
            }
            observed_trail.pop ();
        }
        //std::cout << "]" << std::endl;
    }

    bool cb_check_found_model (const std::vector<int> &model) override {
        /*std::cout << "[act-prop][cb_check_found_model][ ";
        for (const auto lit : model) {
        std::cout << lit << " ";
        }
        std::cout << "]" << std::endl;*/
        return check_sol(model);
    }

    //for now, I don't get involved in the actual solving except to add cycle clauses via cb_has_external_clause
    int cb_decide () override {return 0;};

    int cb_propagate () override { return 0;};

    int cb_add_reason_clause_lit (int propagated_lit) override {return 0; };

    bool cb_has_external_clause (bool &is_forgettable) override {
        //std::cout << "[act-prop][cb_has_external_clause][ ";
        is_forgettable = false;
        if(!cycles.empty()){
            clause = cycles.back();
            /*for(auto v : clause){
                std::cout<<v <<" ";
            }
            std::cout<<"]"<<std::endl;*/
            cycles.pop_back();
            return true;
        }
        bool no_cycles = check_partial_sol();
        if(!no_cycles){
            clause=cycles.back();
            /*for(auto v : clause){
                std::cout<<v <<" ";
            }
            std::cout<<"]"<<std::endl;*/
            cycles.pop_back();
            return true;
        }
        return false;
    }

    int cb_add_external_clause_lit () override {
        if(!clause.empty()){//should always be the case
            int lit = clause.back();
            clause.pop_back();
            return lit;
        }
        return 0;
    }
};


int Propagator_rp::solve(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	//saves the number given to the variable standing for i is representative of j (i<j)
	std::vector<std::vector<int>> variables = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
	Propagator_rp::initialize_solver(G,k, solver, previous_cycles, variables);
	
	std::clock_t t_before_solve=std::clock();
    ActivePropagator* prop = new ActivePropagator(solver,G,k,variables);
try{
    prop->observe();
    int res = solver->solve();
    std::clock_t t_after_solve=std::clock();
    if(res==10){
        std::cout<<"Acyclic colouring found!"<<std::endl;
        std::cout<<"Time spent on solving: "<<1000*(t_after_solve-t_before_solve)/CLOCKS_PER_SEC <<"ms."<<std::endl;
	    delete prop;
        delete solver;
        return 0;
    }
    std::cout<<"Unsatisfiable"<<std::endl;
	std::cout<<"Time spent on solving: "<<1000*(t_after_solve-t_before_solve)/CLOCKS_PER_SEC <<"ms."<<std::endl;
	delete prop;
    delete solver;
	/*std::cout<<"The colouring is:";
	for(int i=0;i<G.ncount();i++){
		std::cout<<" "<<colouring[i];
	}
	std::cout<<std::endl;*/
	return 1;
}
catch(std::runtime_error e){
    std::cout<<"Error caught: "<<e.what()<<std::endl;
    delete prop;
    delete solver;
    return 1;
}
catch(...){
    std::cout<<"Unknown exception caught."<<std::endl;
    delete prop;
    delete solver;
    return 1;
}
}
