: <<'END'
This does comments. The end does can be anything.
END

rm random_instances/*
./random_generator 2 25
time ./acycle_k random_instances/random_size_25_0.col assignment_version/more_than_one_cycle 20
time ./acycle_k random_instances/random_size_25_1.col assignment_version/more_than_one_cycle 20