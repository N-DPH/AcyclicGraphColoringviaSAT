#include "prop_rp_tree.hpp"
#include "../cadical/src/cadical.hpp"
#include "Graph.h"
#include <stack>
#include <vector>
#include <map>
#include <queue>
#include <tuple>
#include <utility>
#include <cmath>
#include <queue>
#include <chrono>
#include <stdexcept>


//IMPORTANT: this only works if the solver assigns a vertex to be a representative before saying that is represents others

void Propagator_rp::initialize_solver_tree(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles, std::vector<std::vector<int>> &variables){
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
	for(auto cycle : found_cycles){
		if(cycle[0]<k  && cycle[0]!=-1){
			for(int i=1; i<cycle.size();i++){
				solver->add(cycle[i]);
			}
			solver->add(0);
		}
	}
}

class ActiveTreePropagator : CaDiCaL::ExternalPropagator {
private:
    CaDiCaL::Solver *solver;

    Graph::Graph G;
    int k;
    //variables[u,v] is the variable that says if u represents v
    std::vector<std::vector<int>> variables;
    std::map<int,std::pair<int,int>> variables_back;
    std::stack<std::vector<int>> observed_trail;
    //save the colourings so far
    int color_count;
    //save the connected components of the two colored forests, the last entry is the max number of components
    std::map<std::tuple<int,int>,std::vector<int>> forests;
    std::vector<int> coloring;
    std::vector<int> representatives;
    //contains the cycles already as logic encoding
    std::vector<std::vector<int>> cycles;
    std::vector<int> clause;

public:
    ActiveTreePropagator (CaDiCaL::Solver *s, Graph::Graph &G_new, int k_new, std::vector<std::vector<int>> var)
        : solver (s), G(G_new), k(k_new), variables(var) {
        is_lazy = false;

        // In this example we can always cheaply re-add external clauses if
        // necessary, so we allow the SAT solver to consider them during clause
        // database reduction.
        are_reasons_forgettable = false;

        solver->connect_external_propagator (this);

        coloring = std::vector<int>(G.ncount(),0);

        //initialize the map to contain empty forests for all i<j two colour combinations
        forests=std::map<std::tuple<int,int>,std::vector<int>>();
        std::vector<int> forest(G.ncount()+1,0);
        for(int i=1;i<=k;i++){
            for(int j=i+1;j<=k;j++){
                forests[std::make_pair(i,j)]=forest;
            }
        }
        color_count = 0;
        representatives = std::vector<int>(G.ncount(),-1);
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

    ~ActiveTreePropagator () {
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

    std::vector<int> find_cycles_in_conn_comp(int col1,int col2){
        std::vector<Graph::VertexType> edges = G.elist();
        Graph::NeighborList neighbors = G.get_neighbor_list();
        
        //to not get weird overlapping cycles
        std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
        std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
        for(int i=0;i<G.ncount();i++){
            if((coloring[i]==col1 || coloring[i]==col2) && !visited[i]){
                std::queue<int> q;
                q.push(i);
                while(!q.empty()){
                    int v = q.front();
                    q.pop();
                    for(auto w : neighbors[v]){
                        if(w != parent[v] && visited[w] && (coloring[w]==col1 || coloring[w]==col2)){
                            std::vector<int> cycle= std::vector<int>(0);
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
                            return cycle;
                        }
                        if(w != parent[v] && (coloring[w]==col1 || coloring[w]==col2)){
                            visited[w]=true;
                            parent[w]=v;
                            q.push(w);
                        }
                    }
                }
            }
            else{visited[i]=true;}
        }
    return std::vector<int>();
    }

    std::vector<int> grow_forest(int u, int v){
        //u represents v so give v the same color as u
        coloring[v] = coloring[u]; 
        std::vector<bool> used(k,false);
        std::vector<int> forest;
        std::vector<int> neighbors = G.get_neighbor_list()[v];
        for(auto w: neighbors){
            //for all neighbors that already have a color
            if(coloring[w]!=0){
                forest = forests[std::make_tuple(std::min(coloring[w],coloring[v]),std::max(coloring[w],coloring[v]))];
                //if there already was a neighbor with this color, thus v was already added to the forest
                if(used[coloring[w]]){
                    //if v was added to the same component that w is in, we have a cycle
                    if(forest[w]==forest[v]){
                        std::vector<int> cycle = find_cycles_in_conn_comp(coloring[v],coloring[w]);
                        return cycle;
                    }
                    //w is in a different connected component of the forest so now merge both components
                    else if(forest[w]!=0){
                        int comp = forest[w];
                        for(int i=0;i<G.ncount();i++){
                            if(forest[i]==comp){
                                forest[i]=forest[v]; //transfer all entries of the conn comp of w to that of v
                            }
                        }
                    }
                    //if w was in no tree so far
                    else{
                        forest[w]=forest[v];
                    }
                }
                else{
                    //w is the first neighbor of v with this color and so v was not added to any conn comp yet
                    if(forest[w]!=0){
                        forest[v]=forest[w];
                        used[coloring[w]]=true;
                    }
                    //v and w form a new connected comp
                    else{
                        forest[G.ncount()]++;
                        forest[v]=forest[G.ncount()];
                        forest[w]=forest[v];
                    }
                }
            }
        }
        return std::vector<int>();
    }


    void notify_assignment (const std::vector<int> &lits) override {
        for(int i = lits.size()-1;i>=0;i--){
            std::cout<<lits[i]<< " ";
        }
        std::cout<<std::endl;
        for (int i = lits.size()-1;i>=0;i--) {
            //std::cout << lit << " ";
            int lit = lits[i];
            // Add the new assignment to the observed trail
            observed_trail.top().push_back(lit);

            // Update the maps based on the assignment
            if(lit<=G.ncount() && lit >0){
                std::cout<<lit-1<< " gets color "<<color_count+1<<std::endl;
                color_count++;
                coloring[lit-1]=color_count;
                representatives[lit-1]=lit-1;
            }
            //assume that the representative has already been set as representative
            else if(lit>G.ncount()){
                std::cout<<variables_back[lit].second<< " gets represented by "<<variables_back[lit].first<<std::endl;
                representatives[variables_back[lit].second]=variables_back[lit].first;
                std::vector<int> cycle = grow_forest(variables_back[lit].first,variables_back[lit].second);
                //this assignment caused a cycle
                if(!cycle.empty()){
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
            }
        }
        //std::cout << "]" << std::endl;
        return;
    }

    

    void notify_new_decision_level () override {
        std::cout << "[act-prop][notify_new_decision_level]" << std::endl;

        // Create a new decision level on the observed trail
        observed_trail.push(std::vector<int> ());
    }

    //if v is no longer represented by u
    void decrease_forest(int u, int v){ 
        std::vector<bool> del(k,false);
        std::vector<int> forest;
        std::vector<int> neighbors =  G.get_neighbor_list()[v];
        for(auto w: neighbors){
            //for all neighbors that already have a color
            if(coloring[w]!=0){
                forest = forests[std::make_tuple(std::min(coloring[w],coloring[v]),std::max(coloring[w],coloring[v]))];
                //if w is the first neighbor of this color so we just remove v
                if(!del[coloring[w]]){
                    forest[v]=0;
                    coloring[v]=0;
                    del[coloring[w]]=true;
                }
                //make a new connected component by splitting the tree
                else{
                    forest[G.ncount()]++;
                    int i = forest[G.ncount()];
                    int j = forest[w];
                    std::queue<int> q;
                    q.push(w);
                    //find the whole conn comp from there
                    while(!q.empty()){
                        int y = q.back();
                        q.pop();
                        std::vector<int> neighbors_of_y = G.get_neighbor_list()[y];
                        for(auto z : neighbors_of_y){
                            if(forest[z]==j){q.push(z);}
                        }
                        forest[y]=i;
                    }
                }
            }
        }
        return;
    }

    void notify_backtrack (size_t new_level) override {
        std::cout << "[act-prop][notify_backtrack][ "<< observed_trail.size () - 1 << " -> " << new_level << " ]"<< "[unassigned: ";

        // Undo all the backtracked assignments on the observed trail and
        // in the assignment maps.
        while (observed_trail.size () > new_level + 1) {
            for (auto lit : observed_trail.top ()) {
                std::cout << lit << " ";

                if(lit<=G.ncount() && lit >0){
                    int v = lit-1; 
                    int col = coloring[v];
                    representatives[lit-1]=-1;
                    std::vector<int> forest(G.ncount()+1,0);
                    //empty all trees containing that color
                    for(int i=1;i<col;i++){
                        forests[std::make_tuple(i,col)]= forest;
                    }
                    for(int i=col+1;i<=k;i++){
                        forests[std::make_tuple(i,col)]= forest;
                    }
                    //deassign all vertices that had the same color
                    for(int i=0;i<G.ncount();i++){
                        if(coloring[i]==col){
                            coloring[i]=0;
                            representatives[i]=-1;
                        }
                    }
                    color_count--;
                }
                //we might have already uncolored v if the whole color class was deleted
                else if(lit>G.ncount() && coloring[variables_back[lit].second]!=0){
                    representatives[variables_back[lit].second] = -1;
                    decrease_forest(variables_back[lit].first, variables_back[lit].second);
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


int Propagator_rp::solve_tree(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	//saves the number given to the variable standing for i is representative of j (i<j)
	std::vector<std::vector<int>> variables = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
	Propagator_rp::initialize_solver_tree(G,k, solver, previous_cycles, variables);
	
	std::clock_t t_before_solve=std::clock();
    ActiveTreePropagator* prop = new ActiveTreePropagator(solver,G,k,variables);
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
