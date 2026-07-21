: <<'END'
This does comments. The end does can be anything.


rm -f logs_results/log_25.csv
echo "catergory,time" >>logs_results/log_25.csv
rm -f logs_results/results_25_ap.txt
./build/random_generator 10 25 0.4
for file in random_instances/*
do
    echo "$file" >> logs_results/results_25_rp.txt
    ./build/acycle "$file" -rp >> logs_results/results_25_rp.txt
done
END
rm -f logs_results/results_25_ac.txt
for file in random_instances/*
do
    echo "$file" >> logs_results/results_25_ac.txt
    ./build/acycle "$file" -po -ac >> logs_results/results_25_ac.txt
done

: <<'END'

rm -f logs_results/results_25.txt
./build/random_generator 20 25 0.5
for file in random_instances/*
do
    echo "$file" >> logs_results/results_25.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_25.txt
done

rm -f logs_results/results_25.txt
./build/random_generator 20 25 0.8
for file in random_instances/*
do
    echo "$file" >> logs_results/results_25.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_25.txt
done

rm -f logs_results/results_benchmarks.txt
for file in instances_git/*
do
    echo "$file" >> logs_results/results_benchmarks.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_benchmarks.txt
done
END