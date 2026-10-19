: <<'END'
This does comments. The end does can be anything.


n="20"
d="3"
t="800"

rm -f logs_results/log_"$n".csv
echo "category,time" >>logs_results/log_"$n".csv
rm -f logs_results/results_"$n".txt
./build/random_generator 50 "$n" 0."$d"
for file in random_instances/*
do
    echo "$file $t" >> logs_results/results_"$n".txt
    echo -n "$n $d -as-fv," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -as -fv >> logs_results/results_"$n".txt
    echo -n "$n $d -as-mc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -as -mc >> logs_results/results_"$n".txt
    echo -n "$n $d -as-oc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -as -oc >> logs_results/results_"$n".txt
    echo -n "$n $d -as-rc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -as -rc >> logs_results/results_"$n".txt
    echo -n "$n $d -as-sc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -as -sc >> logs_results/results_"$n".txt
    echo -n "$n $d -po-fv," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -po -fv >> logs_results/results_"$n".txt
    echo -n "$n $d -po-mc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -po -mc >> logs_results/results_"$n".txt
    echo -n "$n $d -po-oc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -po -oc >> logs_results/results_"$n".txt
    echo -n "$n $d -po-rc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -po -rc >> logs_results/results_"$n".txt
    echo -n "$n $d -po-sc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -po -sc >> logs_results/results_"$n".txt
    echo -n "$n $d -rp-fv,">> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -rp -fv >> logs_results/results_"$n".txt
    echo -n "$n $d -rp-mc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -rp -mc >> logs_results/results_"$n".txt
    echo -n "$n $d -rp-oc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -rp -oc >> logs_results/results_"$n".txt
    echo -n "$n $d -rp-rc," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -rp -rc >> logs_results/results_"$n".txt
    echo -n "$n $d -rp-sc,">> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -rp -sc >> logs_results/results_"$n".txt
done
./build/check_results logs_results/results_"$n".txt >> logs_results/results_"$n".txt
echo "Done with the 20 3"


t=10
for n in {10..70..5}
do
    last_successful_value=0

    rm -f logs_results/log_"$n".csv
    rm -f logs_results/results_"$n".txt
    # Outer loop: values from 1 to 9
    for d in {1..9};
    do
        success_count=0
        echo "category,time" >>logs_results/log_"$n".csv
        ./build/random_generator 10 "$n" 0."$d"
        for file in random_instances/*
        do
            echo -n "$n $d -rp-sc," >> logs_results/log_"$n".csv
            echo "$file $t" >> logs_results/results_"$n".txt
            if ./build/acycle "$file" "$t" -rp -sc >> logs_results/results_"$n".txt; then
                ((success_count++))
            fi
        done

        if [ "$success_count" -lt $((10 / 2)) ]; then
            echo "Over half failed $n 0.$d"
            break
        else
            echo "Passed $n 0.$d"
            last_successful_value=$d
        fi
    done

    echo "Execution stopped. The last passing value was: $last_successful_value"
done
END

n="30"
d="5"
t="800"

rm -f logs_results/log_"$n".csv
echo "category,time" >>logs_results/log_"$n".csv
rm -f logs_results/results_"$n".txt
./build/random_generator 20 "$n" 0."$d"
for file in random_instances/*
do
    echo "$file $t" >> logs_results/results_"$n".txt
    echo -n "$n $d -as-fv," >> logs_results/log_"$n".csv
    ./build/acycle "$file" "$t" -rp -sc >> logs_results/results_"$n".txt
done