#include <iostream>
#include <string>
#include <stdexcept>

int main(int argc, char *argv[]) {
try{
	if(argc<2){throw std::runtime_error("No file given");}
    if(argc==2){throw std::runtime_error("No upper bound given");}
    int b = std::stoi(argv[2]);
    for(int i=b;i>=0;i--){
        std::string file = argv[1];
        std::string command = "./build/acycle "+file+" " +std::to_string(i)+" -po -mc";
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
catch(std::runtime_error e){
    std::cout<<"Error caught: "<<e.what()<<std::endl;
    return 1;
}
catch(...){
    std::cout<<"Unknown error caught"<<std::endl;
    return 1;
}
}