./build/random_generator 5 10
for file in random_instances/*
do
    echo "$file" >> results_10.txt
    ./build/acycle "$file" -po -mc >> results_10.txt
done