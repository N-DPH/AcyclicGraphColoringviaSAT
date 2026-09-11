sources := src/solver.cpp src/as_fv.cpp src/as_mc.cpp src/po_fv.cpp src/po_mc.cpp src/po_oc.cpp src/Graph.cpp src/po_rc.cpp src/po_ac.cpp src/po_sc.cpp src/rp_rc.cpp src/rp_sc.cpp src/prop_rp.cpp src/prop_as_tree.cpp src/upper_bound.cpp src/lower_bound.cpp src/preprocessing_it.cpp src/preprocessing_cl.cpp src/small_cycle.cpp src/check_results.cpp
executables := build/random_generator build/acycle build/check_results
objects := build/as_fv.o build/as_mc.o build/po_fv.o build/po_mc.o build/po_oc.o build/po_rc.o build/po_ac.o build/po_sc.o build/rp_rc.o build/rp_sc.o build/prop_rp.o build/prop_as_tree.o build/upper_bound.o build/lower_bound.o build/preprocessing_it.o build/preprocessing_cl.o build/small_cycle.o
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

build/check_results : src/check_results.cpp
	$(CCX) $(CXXFLAGS) $< -o $@

createfolder:
	mkdir -p ${folder}

clean: 
	rm -f $(executables) $(objects)