#include "prop_as_tree.hpp"
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


void Propagator_as::initialize_solver_tree(Graph::Graph &G, int k, CaDiCaL::Solver *solver,std::vector<std::vector<int>> &found_cycles){
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
	/*adding all the previous found problem cycles
	for(auto cycle : found_cycles){
		if(cycle[0]<k  && cycle[0]!=-1){
			for(int i=1; i<cycle.size();i++){
				solver->add(cycle[i]);
			}
			solver->add(0);
		}
	}*/
}

class ActiveTreePropagator : CaDiCaL::ExternalPropagator {
private:
    CaDiCaL::Solver *solver;

    Graph::Graph &G;
    int k;
    bool solved;
    std::stack<std::vector<int>> observed_trail;
    //is !=0 if there was a level where a cycle was found or two adjacent vertices with same color
    int is_valid_coloring;
    //if the colorig was found to be unvalid because one vertex has several color: which one?
    int has_two_colors;
    //save the colourings so far
    //save the connected components of the two colored forests, the last entry is the max number of components
    std::map<std::tuple<int,int>,std::vector<int>> forests;
    std::vector<int> coloring;
    //contains the cycles already as logic encoding
    std::vector<std::vector<int>> cycles;
    std::vector<int> clause;

public:
    ActiveTreePropagator (CaDiCaL::Solver *s, Graph::Graph &G_new, int k_new)
        : solver (s), G(G_new), k(k_new){
        is_lazy = false;
            solved = false;
        // In this example we can always cheaply re-add external clauses if
        // necessary, so we allow the SAT solver to consider them during clause
        // database reduction.
        are_reasons_forgettable = false;

        solver->connect_external_propagator (this);

        coloring = std::vector<int>(G.ncount(),0);

        //initialize the map to contain empty forests for all i<j two colour combinations
        forests=std::map<std::tuple<int,int>,std::vector<int>>();
        for(int i=1;i<k;i++){
            for(int j=i+1;j<=k;j++){
                forests[std::make_tuple(i,j)]=std::vector<int>(G.ncount()+1,0);
            }
        }
        
        // Create the root-level of the trail. This level needs to be always
        // there.
        observed_trail.push(std::vector<int> ());
        is_valid_coloring = 0;
        has_two_colors = -1;
    }

    void observe(){
        //Observe all variables of the encoding
        for(int i=0;i<G.ncount();i++){
            for(int j=1;j<=k;j++){
                solver->add_observed_var(i*k+j);
            }
        }
        return;
    }

    void is_solved(){
        solved = true;
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
        for(int i=0;i<model.size();i++){
            if(model[i]>0){
                int col = (model[i]-1)%k+1;
                int v = (model[i]-col)/k;
                colours[v]=col-1;
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
                new_clause.push_back(-(cycle[i]*k+colours[cycle[i]]+1));
            }
            //std::cout<<std::endl;
            cycles.push_back(new_clause);
        }
        return new_cycles.empty(); //returns true iff the assignment is valid and cycle free
    }

    std::vector<int> find_cycles_in_conn_comp(int col1,int col2,int i){
        std::vector<Graph::VertexType> edges = G.elist();
        Graph::NeighborList neighbors = G.get_neighbor_list();
        
        std::vector<bool> visited = std::vector<bool>(G.ncount(),false);
        std::vector<int> parent = std::vector<int>(G.ncount(),-1); 
        
        std::queue<int> q;
        q.push(i);
        visited[i]=true;
        while(!q.empty()){
            int v = q.front();
            q.pop();
            for(auto w : neighbors[v]){
                if(w != parent[v] && visited[w] && (coloring[w]==col1 || coloring[w]==col2)){
                    std::vector<int> cycle= std::vector<int>(0);
                    //std::cout<<"found cycle: "<<v<<", "<<w<<", ";
                    cycle.push_back(w);
                    cycle.push_back(v);
                    w=parent[w];
                    cycle.push_back(w);
                    //std::cout<<w<<", ";
                    w = parent[w];
                    v=parent[v];
                    //std::cout << "begin while with v = "<<v <<", w = "<<w<< std::endl;
                    while(w != v){
                        cycle.push_back(v);
                        cycle.push_back(w);
                        //std::cout<<w<<", "<<v<<", ";
                        //std::cout << "w = " << w << std::endl;
                        //std::cout << "parent(w) = " << parent[w] << std::endl;
                        w = parent[w];
                        v=parent[v];
                    }
                    //std::cout << "end while" << std::endl;
                    cycle.push_back(v);
                    //std::cout<<v<<std::endl;
                    return cycle;
                }
                else if(w != parent[v] && (coloring[w]==col1 || coloring[w]==col2)){
                    visited[w]=true;
                    parent[w]=v;
                    q.push(w);
                }
            }
        } 
    std::cout<<"No cycle found though the forest says there should be"<<std::endl;
    /*std::cout<<"The forest with colors "<<col1<<", "<<col2<< " is the following: ";
    std::tuple<int, int> forest = std::make_tuple(std::min(col1,col2),std::max(col1,col2));
    for(int j=0;j<G.ncount();j++){
        if(forests[forest][j]!=0){
            std::cout<<"("<<j<<" in "<<forests[forest][j]<<")";
        }
    }
    std::cout<<std::endl;
    std::cout<<"The coloring looks as follows: ";
    for(int j=0;j<G.ncount();j++){
        if(coloring[j]!=0){
            std::cout<<"("<<j<<" has "<<coloring[j]<<")";
        }
    }
    std::cout<<std::endl;*/
    return std::vector<int>();
    }

    std::vector<std::vector<int>> grow_forest(int v){ 
        std::vector<bool> used(k,false);
        std::tuple<int,int> forest;
        std::vector<std::vector<int>> new_cycles;
        std::vector<int> neighbors = G.get_neighbor_list()[v];
        for(auto w: neighbors){
            if(coloring[w]==coloring[v]){
                is_valid_coloring = observed_trail.size()-1;
                coloring[v]=0;
                //std::cout<<"Same color so marking "<<is_valid_coloring<<std::endl;
                return new_cycles;
            }
        }
        for(auto w:neighbors){
            //for all neighbors that already have a color
            if(coloring[w]!=0){
                forest = std::make_tuple(std::min(coloring[w],coloring[v]),std::max(coloring[w],coloring[v]));
                //if there already was a neighbor with this color, thus v was already added to the forest
                if(used[coloring[w]-1]){
                    //if v was added to the same component that w is in, we have a cycle
                    if(forests[forest][w]==forests[forest][v]){
                        new_cycles.push_back(find_cycles_in_conn_comp(coloring[v],coloring[w],v));
                        is_valid_coloring = observed_trail.size()-1;
                        //std::cout<<"Found cycle so marking "<<is_valid_coloring<<std::endl;
                        return new_cycles;
                    }
                    //w is in a different connected component of the forest so now merge both components
                    else if(forests[forest][w]!=0){
                        int comp = forests[forest][w];
                        for(int i=0;i<G.ncount();i++){
                            if(forests[forest][i]==comp){
                                forests[forest][i]=forests[forest][v]; //transfer all entries of the conn comp of w to that of v
                                //std::cout<<"merge "<<i<<" to component "<<forests[forest][v]<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                            }
                        }
                    }
                    //if w was in no tree so far
                    else{
                        forests[forest][w]=forests[forest][v];
                        //std::cout<<"add w "<<w<<" to component "<<forests[forest][v]<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                    }
                }
                else{
                    //w is the first neighbor of v with this color and so v was not added to any conn comp yet
                    if(forests[forest][w]!=0){
                        forests[forest][v]=forests[forest][w];
                        //std::cout<<"add v "<<v<<" to component "<<forests[forest][w]<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                        used[coloring[w]-1]=true;
                    }
                    //v and w form a new connected comp
                    else{
                        forests[forest][G.ncount()]++;
                        forests[forest][v]=forests[forest][G.ncount()];
                        //std::cout<<"new comp "<<v<<" to component "<<forests[forest][v]<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                        forests[forest][w]=forests[forest][v];
                        //std::cout<<"new comp "<<w<<" to component "<<forests[forest][v]<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                        used[coloring[w]-1]=true;
                    }
                }
            }
        }
        return new_cycles;
    }


    void notify_assignment (const std::vector<int> &lits) override {
        if(!solved){
            //std::cout<<"New assignments ["<<std::endl;
            /*for(int i = lits.size()-1;i>=0;i--){
                std::cout<<lits[i]<< " ";
            }
            std::cout<<std::endl;*/
            for (auto lit : lits) {
                //std::cout << lit << " ";
                // Add the new assignment to the observed trail
                observed_trail.top().push_back(lit);
                if(is_valid_coloring==0){ //we ignore everything after an unvalid assignment
                    int col = (lit-1)%k+1;
                    int v = (lit-col)/k;
                    // Update the maps based on the assignment
                    if(lit>0){
                        if(coloring[v]!=0){
                            is_valid_coloring = observed_trail.size()-1;
                            //std::cout<<"trying to assign several colors to one vertex"<<std::endl;
                            has_two_colors = v;
                            continue;
                        }
                        //std::cout<<v<< " gets color "<<col<<std::endl;
                        coloring[v]=col;
                        std::vector<std::vector<int>> new_cycles = grow_forest(v);
                        //this assignment caused a cycle
                        if(!new_cycles.empty()){
                            for(auto cycle : new_cycles){
                                std::vector<int> new_clause;
                                for(int i=0;i<cycle.size();i++){
                                    new_clause.push_back(-(cycle[i]*k+coloring[cycle[i]]));
                                }
                                cycles.push_back(new_clause);
                            }
                        }
                    }
                }
            }
            //std::cout << "]" << std::endl;
        }
        return;
    }

    

    void notify_new_decision_level () override {
        //std::cout << "[act-prop][notify_new_decision_level]" << std::endl;

        // Create a new decision level on the observed trail
        observed_trail.push(std::vector<int> ());
    }

    //if v is no longer represented by u
    void decrease_forest(int v){ 
        if(coloring[v]==0){return;}//possible since we sometimes don't finish the assignments due to found issues
        else if(has_two_colors == v){has_two_colors = -1; return;}
        std::vector<bool> del(k,false);
        std::tuple<int,int> forest;
        std::vector<int> neighbors =  G.get_neighbor_list()[v];
        for(auto w: neighbors){
            if(coloring[w]!=0){
            //for all neighbors that already have a color
                forest = std::make_tuple(std::min(coloring[w],coloring[v]),std::max(coloring[w],coloring[v]));
                if(forests[forest][w]!=0){
                    bool is_connected = false;
                    std::vector<int> neighbors_of_w = G.get_neighbor_list()[w];
                    for(auto x : neighbors_of_w){
                        if(x != v && forests[forest][x]==forests[forest][w]){
                            is_connected = true;
                            break;
                        }
                    }
                    //if w has no further connections to the connected comp, then just delete both
                    if(!is_connected){ 
                        forests[forest][v]=0;
                        forests[forest][w]=0;
                        //std::cout<<"removed "<<w<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                        //std::cout<<"removed "<<v<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                    }
                    //if w is the first neighbor of this color so we remove v
                    else if(!del[coloring[w]-1]){
                        forests[forest][v]=0;
                        //std::cout<<"removed "<<v<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                        del[coloring[w]-1]=true;
                    }
                    //make a new connected component by splitting the tree
                    else{
                        forests[forest][G.ncount()]++;
                        int new_comp = forests[forest][G.ncount()];
                        int old_comp = forests[forest][w];
                        std::queue<int> q;
                        q.push(w);
                        forests[forest][w]=new_comp;
                        //std::cout<<"changed "<<w<<"to comp "<<new_comp<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                        //find the whole conn comp from there
                        while(!q.empty()){
                            int y = q.front();
                            q.pop();
                            std::vector<int> neighbors_of_y = G.get_neighbor_list()[y];
                            for(auto z : neighbors_of_y){
                                if(forests[forest][z]==old_comp){
                                    q.push(z);
                                    forests[forest][z]=new_comp;
                                    //std::cout<<"changed "<<z<<"to comp "<<new_comp<<" in color: "<<coloring[w]<<", "<<coloring[v]<<std::endl;
                                }
                            }                        
                        }
                    }
                }
            }
        }
        coloring[v]=0;
        //std::cout<<"uncolored "<<v<<std::endl;
        return;
    }

    void notify_backtrack (size_t new_level) override {
        //std::cout << "[act-prop][notify_backtrack][ "<< observed_trail.size () - 1 << " -> " << new_level << " ]"<< "[unassigned: ";

        // Undo all the backtracked assignments on the observed trail and
        // in the assignment maps.
        while (observed_trail.size () > new_level + 1) {
            if(!solved){
                if((is_valid_coloring == 0) || (is_valid_coloring +1 >= observed_trail.size())){
                    is_valid_coloring = 0;
                    for (auto lit : observed_trail.top ()) {
                        //std::cout << lit << " ";
                        int col_v = (lit-1)%k+1;
                        int v = (lit-col_v)/k;
                        if(lit>0){
                            decrease_forest(v);
                        }
                    }
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
        bool res = check_sol(model);
        //std::cout<< "Made the check. Found a cycle: "<<!res<<std::endl;
        return res;
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


int Propagator_as::solve_tree(Graph::Graph &G, int k, std::vector<std::vector<int>> &previous_cycles) {

	CaDiCaL::Solver *solver = new CaDiCaL::Solver;
	//saves the number given to the variable standing for i is representative of j (i<j)
	std::vector<std::vector<int>> variables = std::vector<std::vector<int>>(G.ncount(),std::vector<int>(G.ncount(),0));
	Propagator_as::initialize_solver_tree(G,k, solver, previous_cycles);
	
	std::clock_t t_before_solve=std::clock();
    ActiveTreePropagator* prop = new ActiveTreePropagator(solver,G,k);
try{
    prop->observe();
    int res = solver->solve();
    std::clock_t t_after_solve=std::clock();
    if(res==10){
        std::cout<<"Acyclic colouring found!"<<std::endl;
        prop -> is_solved();
        std::cout<<"Time spent on solving: "<<1000*(t_after_solve-t_before_solve)/CLOCKS_PER_SEC <<"ms."<<std::endl;
	    delete prop;
        delete solver;
        return 0;
    }
    std::cout<<"Unsatisfiable"<<std::endl;
    prop -> is_solved();
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
