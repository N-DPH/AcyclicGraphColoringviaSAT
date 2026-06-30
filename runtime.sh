: <<'END'
This does comments. The end does can be anything.
END

rm -f results_25.txt
./build/random_generator 20 25
for file in random_instances/*
do
    echo "$file" >> results_25.txt
    ./build/acycle "$file" -po -mc >> results_25.txt
done

rm -f results_30.txt
./build/random_generator 20 30
for file in random_instances/*
do
    echo "$file" >> results_30.txt
    ./build/acycle "$file" -po -mc >> results_30.txt
done

rm -f results_benchmarks.txt
for file in instances_git/*
do
    echo "$file" >> results_benchmarks.txt
    ./build/acycle "$file" -po -mc >> results_benchmarks.txt
done