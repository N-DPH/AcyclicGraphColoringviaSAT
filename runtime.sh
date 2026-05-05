: <<'END'
This does comments. The end does can be anything.
END

rm random_instances/*
./random_generator 2 25
time ./acycle_k random_instances/random_size_25_0.col 20
time ./acycle_k random_instances/random_size_25_1.col 20