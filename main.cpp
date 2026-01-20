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
	//still need to work this part out
	return std::vector<int>(0);
}

bool check_colouring(Graph::Graph &G, int k, std::vector<int> colour){
	std::vector<int> cycle = find_cycle(G,k,colour);
	//todo: adapt the sat_inst to forbid the cycle
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
