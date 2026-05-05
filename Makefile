path_assignment_more_cycle := assignment_version/more_than_one_cycle/
path_assignment_first := assignment_version/first_version/
path_ordering_more_cycle := partial_ordering/more_than_one_cycle/
path_ordering_first := partial_ordering/first_version/
executables := $(path_assignment_first)acycle $(path_assignment_more_cycle)acycle $(path_ordering_first)acycle $(path_ordering_more_cycle)acycle random_generator acycle_k
CXXFLAGS := -Wall -Werror

all: $(executables)

$(path_assignment_more_cycle)acycle:  $(path_assignment_more_cycle)main.cpp
	clang++ $(CXXFLAGS) $(path_assignment_more_cycle)main.cpp Graph.cpp -o $(path_assignment_more_cycle)acycle -I/cadical/src cadical/build/libcadical.a

$(path_assignment_first)acycle:  $(path_assignment_first)main.cpp
	clang++ $(CXXFLAGS) $(path_assignment_first)main.cpp Graph.cpp -o $(path_assignment_first)acycle -I/cadical/src cadical/build/libcadical.a
	
$(path_ordering_more_cycle)acycle:  $(path_ordering_more_cycle)main.cpp
	clang++ $(CXXFLAGS) $(path_ordering_more_cycle)main.cpp Graph.cpp -o $(path_ordering_more_cycle)acycle -I/cadical/src cadical/build/libcadical.a

$(path_ordering_first)acycle:  $(path_ordering_first)main.cpp
	clang++ $(CXXFLAGS) $(path_ordering_first)main.cpp Graph.cpp -o $(path_ordering_first)acycle -I/cadical/src cadical/build/libcadical.a

random_generator : create_random_graphs.cpp
	clang++ $(CXXFLAGS) create_random_graphs.cpp Graph.cpp -o random_generator

acycle_k : colouring_number.cpp
	clang++ $(CXXFLAGS) colouring_number.cpp -o acycle_k

clean: 
	rm -f $(executables)