: <<'END'
This does comments. The end does can be anything.
END

rm -f logs_results/log_15.csv
echo "catergory,time" >>logs_results/log_15.csv
rm -f logs_results/results_15_rp.txt
./build/random_generator 10 15 0.4
for file in random_instances/*
do
    echo "$file" >> logs_results/results_15_rp.txt
    ./build/acycle "$file" -rp >> logs_results/results_15_rp.txt
done

rm -f logs_results/results_15_po.txt
for file in random_instances/*
do
    echo "$file" >> logs_results/results_15_po.txt
    ./build/acycle "$file" -po >> logs_results/results_15_po.txt
done

: <<'END'

rm -f logs_results/results_15.txt
./build/random_generator 20 15 0.5
for file in random_instances/*
do
    echo "$file" >> logs_results/results_15.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_15.txt
done

rm -f logs_results/results_15.txt
./build/random_generator 20 15 0.8
for file in random_instances/*
do
    echo "$file" >> logs_results/results_15.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_15.txt
done

rm -f logs_results/results_benchmarks.txt
for file in instances_git/*
do
    echo "$file" >> logs_results/results_benchmarks.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_benchmarks.txt
done
END