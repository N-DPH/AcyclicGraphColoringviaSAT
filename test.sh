rm -f log_15.csv
echo "catergory,time" >>log_15.csv
rm -f results_15_1.txt
for file in random_instances/*
do
    echo "$file" >> results_15_1.txt
    ./build/acycle "$file" -po -mc >> results_15_1.txt
done