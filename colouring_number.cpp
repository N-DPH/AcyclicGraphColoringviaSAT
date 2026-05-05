#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
	if(argc<2){std::cout<<"No file given"<<std::endl; return 1;}
    if(argc==2){std::cout<<"No path to source code given"<<std::endl;return 1;}
    if(argc==3){std::cout<<"No upper bound given"<<std::endl; return 1;}
    int b = std::stoi(argv[3]);
    for(int i=b;i>=0;i--){
        std::string file = argv[1];
        std::string code_path = argv[2];
        std::string command = "./"+code_path+"/acycle "+file+" " +std::to_string(i);
        int ret = system(command.c_str());
        if(ret!=0){
            std::cout<<"The colouring number is "<<i+1<<std::endl;
            return 0;
        }
        else{
            std::cout<<i<<" is SAT"<<std::endl;
        }
    }
    return 0;
}