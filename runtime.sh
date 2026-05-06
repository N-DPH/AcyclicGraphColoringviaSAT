: <<'END'
This does comments. The end does can be anything.
END

./build/random_generator 2 15
time ./build/acycle_k random_instances/random_size_15_0.col 10
time ./build/acycle_k random_instances/random_size_15_1.col 10