: <<'END'
This does comments. The end does can be anything.
END

./build/random_generator 2 20
time ./build/acycle_k random_instances/random_size_20_0.col 15
time ./build/acycle_k random_instances/random_size_20_1.col 15