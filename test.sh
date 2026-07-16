rm -f log_20.csv
echo "catergory,time" >>log_20.csv
rm -f results_20_1.txt
for file in random_instances/*
do
    echo "$file" >> results_20_1.txt
    ./build/acycle "$file" -po -rc >> results_20_1.txt
done

