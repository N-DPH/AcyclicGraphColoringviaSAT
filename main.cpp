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

void write_colour_sat(Graph::Graph &G, int k){
	std::ofstream SAT("sat_inst.cnf");
	int n = G.ncount();
	int num_var = n*k;
	int num_clauses = n+k*G.ecount();
	for (int i=1;i<=k;i++){
		 num_clauses = num_clauses + n*(k-i);
	}
	SAT<< "p cnf "<<num_var<<" "<<num_clauses<<std::endl;
	for(int i=0; i<n;i++){
		int index=i*k+1;
		for(int j=0;j<k;j++){
			SAT<<index<<" ";
			index++;
		}
		SAT<<"0"<<std::endl;
	}
	for(int i=0; i<n;i++){
		int index=i*k+1;
		for(int j=0;j<k;j++){
			for(int l=j+1;l<k;l++){
				SAT<<-index-j<<" "<<-index-l<<" 0"<<std::endl;
			}
		}
	}
	std::vector<Graph::VertexType> edges = G.elist();
	for (int i=0;i<G.ecount();i++){
		int index_v = edges[2*i] *k+1;
		int index_w = edges[2*i+1] *k+1;
		for(int j=0;j<k;j++){
			SAT<<-index_v-j<<" "<<-index_w-j<<" 0"<<std::endl;
		}
	}
	SAT.close();
}


std::vector<int> read_sol(Graph::Graph &G, int k){
	std::ifstream file("sat_res.sol");
	if(not file){throw std::runtime_error("Cannot open file.");}
	std::string line;
	std::getline(file, line);//read in the file line by line
	
	while(line[0] == 'c' or line[0] == '\r' or line[0] == '\0')	
	{std::getline(file,line);}                                                               //ignore comments
	if(line[0] != 's'){
        throw std::runtime_error("File is not in correct dimacs format.");}         //a different case was expected
	std::string s, sat;
	std::stringstream ss(line);
	ss >> s >> sat;
	                                                      //read in number of nodes and edges
	if(sat != "SATISFIABLE"){throw std::runtime_error("Instance not solvable.");}
	std::getline(file, line);
	std::string v, a;
	int index;
	std::vector<int> colours = std::vector<int>(G.ncount());
	ss.clear();
        ss.str(std::string());
        ss << line;
        ss >> v >> a;
        
        while(a != "0"){
        	index = std::stoi(a);
        	if(index>0){
			int vertex = (index)/k;
			int colour = (index-1)%k+1;
			colours[vertex]=colour;}
        	ss >> a;        	
        }
        return colours;
}

std::vector<int> find_cycle(Graph::Graph &G, int k, std::vector<int> colour){
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
							return cycle;//should work because we only have even cycles
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
	return cycle;
}

bool check_colouring(Graph::Graph &G, int k, std::vector<int> colour){
	std::vector<int> cycle = find_cycle(G,k,colour);
	if(cycle.empty()){return true;} //no cycle found
	int i=0;
	std::ofstream SAT("sat_inst.cnf",std::ios::app);
	for(int j=0;j<k;j++){ //adding the constraints forbidding any colour permutation of that cycle
		for(int l=0;l<k;l++){
			if(l!=j){
				SAT<<-(cycle[0]*k+1)-j<<" ";
				for(int a=1;a<cycle.size()/2;a++){
					if(a%2==1){
						SAT<<-(cycle[2*a]*k+1)-l<<" "<<-(cycle[2*a-1]*k+1)-l<<" ";
					}
					if(a%2==0){
						SAT<<-(cycle[2*a]*k+1)-j<<" "<<-(cycle[2*a-1]*k+1)-j<<" ";
					}
				}
				SAT<<-(cycle[cycle.size()]*k+1)-j<<" 0"<<std::endl;
				i++;
			}
		}
	}
	SAT.close();
	std::fstream SAT2("sat_inst.cnf", std::ios::in| std::ios::out);
	std::streampos start= SAT2.tellp();
	std::string line;
	std::getline(SAT2, line);
	std::string p, cnf, var, clauses;
	std::stringstream ss(line);
	ss >> p >> cnf >> var >>clauses;
	SAT.seekp(start);
	i = i + std::stoi(clauses);
	std::string newprint = p +" "+cnf + " "+var+" "+std::to_string(i);
	SAT.write(newprint.c_str(), newprint.size()); //does not work yet
	SAT2.close();
	return true;
}

int main(int argc, char *argv[]) {
	if(argc==0){std::cout<<"No file given"<<std::endl; return 1;}
	if(argc==1){std::cout<<"No colour number given"<<std::endl; return 1;}
	Graph::Graph G = Graph::Graph(argv[1]);
	int k = std::stoi(argv[2]);
	G.print();
	write_colour_sat(G,k);
	std::system("./cadical/build/cadical -q sat_inst.cnf > sat_res.sol");
	std::vector<int> colouring = read_sol(G,k);
	std::cout<<colouring[0]<<" "<<colouring[1]<<" "<<colouring[2]<<std::endl;
	bool no_cycle = check_colouring(G, k, colouring);
	while(!no_cycle){
		std::system("./cadical/build/cadical -q sat_inst.cnf > sat_res.sol");
		colouring = read_sol(G,k);
		no_cycle = check_colouring(G, k, colouring);
	}
	std::cout<<"Acyclic colouring found!"<<std::endl;
	return 0;
	
}
