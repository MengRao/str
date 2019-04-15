g++ -march=native -O3 bench.cc -o bench
# run: ./bench < data.txt

g++ -march=native -O3 benchnum.cc -o benchnum
# run: ./benchnum

g++ -march=native -O3 benchcmp.cc -o benchcmp
# run: ./benchcmp
