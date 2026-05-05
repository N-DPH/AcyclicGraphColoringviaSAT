sources := src/solver.cpp src/as_fv.cpp src/as_mc.cpp src/Graph.cpp 
executables := build/random_generator build/acycle build/acycle_k
includes := -I/cadical/src cadical/build/libcadical.a
CXXFLAGS := -Wall -Werror
CCX := clang++

all: $(executables)

build/acycle_k : colouring_number.cpp
	$(CCX) $(CXXFLAGS) $< -o $@

build/acycle : $(sources)
	$(CCX) $(CXXFLAGS) $(sources) -o $@ $(includes)

build/random_generator : create_random_graphs.cpp
	clang++ $(CXXFLAGS) create_random_graphs.cpp src/Graph.cpp -o build/random_generator

acycle_k : colouring_number.cpp
	clang++ $(CXXFLAGS) colouring_number.cpp -o acycle_k

clean: 
	rm -f $(executables)