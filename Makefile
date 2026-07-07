sources := src/solver.cpp src/as_fv.cpp src/as_mc.cpp src/po_fv.cpp src/po_mc.cpp src/Graph.cpp  src/upper_bound.cpp src/lower_bound.cpp
executables := build/random_generator build/acycle
objects := build/as_fv.o build/as_mc.o build/po_fv.o build/po_mc.o build/upper_bound.o build/lower_bound.o
includes := -I/cadical/src cadical/build/libcadical.a
folder := build random_instances
CXXFLAGS := -O3 -g -Wall -Werror -fsanitize=address
CCX := clang++

all: createfolder $(executables)

build/acycle : src/solver.cpp $(objects)
	$(CCX) $(CXXFLAGS) src/solver.cpp src/Graph.cpp $(objects) -o $@ $(includes)

build/%.o : src/%.cpp
	$(CCX) $(CXXFLAGS) -c $< -o $@

build/random_generator : src/create_random_graphs.cpp
	$(CCX) $(CXXFLAGS) src/create_random_graphs.cpp src/Graph.cpp -o build/random_generator

createfolder:
	mkdir -p ${folder}

clean: 
	rm -f $(executables) $(objects)