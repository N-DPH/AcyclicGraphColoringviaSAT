: <<'END'
This does comments. The end does can be anything.
END

rm -f log_20.csv
echo "catergory,time" >>log_20.csv
rm -f results_20.txt
./build/random_generator 20 20 0.5
for file in random_instances/*
do
    echo "$file" >> results_20.txt
    ./build/acycle "$file" -po -mc >> results_20.txt
done

: <<'END'

rm -f results_25.txt
./build/random_generator 20 25 0.5
for file in random_instances/*
do
    echo "$file" >> results_25.txt
    ./build/acycle "$file" -po -mc >> results_25.txt
done

rm -f results_25.txt
./build/random_generator 20 25 0.8
for file in random_instances/*
do
    echo "$file" >> results_25.txt
    ./build/acycle "$file" -po -mc >> results_25.txt
done

rm -f results_benchmarks.txt
for file in instances_git/*
do
    echo "$file" >> results_benchmarks.txt
    ./build/acycle "$file" -po -mc >> results_benchmarks.txt
done
END