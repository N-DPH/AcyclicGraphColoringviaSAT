: <<'END'
This does comments. The end does can be anything.
END

n="10"
d="5"

rm -f logs_results/log_"$n".csv
echo "category,time" >>logs_results/log_"$n".csv
rm -f logs_results/results_"$n".txt
./build/random_generator 20 "$n" 0."$d"
for file in random_instances/*
do
    echo "$file" >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -ac >> logs_results/results_"$n".txt
done
END
d="5"

./build/random_generator 20 "$n" 0."$d"
for file in random_instances/*
do
    echo "$file" >> logs_results/results_"$n".txt
    ./build/acycle "$file" -as -fv >> logs_results/results_"$n".txt
    ./build/acycle "$file" -as -mc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -fv >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -mc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -oc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -rc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -rp >> logs_results/results_"$n".txt
done

d="8"

./build/random_generator 20 "$n" 0."$d"
for file in random_instances/*
do
    echo "$file" >> logs_results/results_"$n".txt
    ./build/acycle "$file" -as -fv >> logs_results/results_"$n".txt
    ./build/acycle "$file" -as -mc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -fv >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -mc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -oc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -rc >> logs_results/results_"$n".txt
    ./build/acycle "$file" -rp >> logs_results/results_"$n".txt
done

: <<'END'

rm -f logs_results/results_"$n"_ac.txt
for file in random_instances/*
do
    echo "$file" >> logs_results/results_"$n"_ac.txt
    ./build/acycle "$file" -po -ac >> logs_results/results_"$n"_ac.txt
done

rm -f logs_results/results_"$n".txt
./build/random_generator 20 "$n" 0.5
for file in random_instances/*
do
    echo "$file" >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -mc >> logs_results/results_"$n".txt
done

rm -f logs_results/results_"$n".txt
./build/random_generator 20 "$n" 0.8
for file in random_instances/*
do
    echo "$file" >> logs_results/results_"$n".txt
    ./build/acycle "$file" -po -mc >> logs_results/results_"$n".txt
done

rm -f logs_results/results_benchmarks.txt
for file in instances_git/*
do
    echo "$file" >> logs_results/results_benchmarks.txt
    ./build/acycle "$file" -po -mc >> logs_results/results_benchmarks.txt
done
END