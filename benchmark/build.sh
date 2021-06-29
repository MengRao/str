g++ -std=c++17 -march=native -O3 -I. benchfindstr.cc -o benchfindstr
# run: ./benchfindstr < data.txt

g++ -std=c++17 -march=native -O3 -I. benchfindint.cc -o benchfindint
# run: ./benchfindint < integers.txt

g++ -march=native -O3 benchnum.cc -o benchnum
# run: ./benchnum

g++ -march=native -O3 benchcmp.cc -o benchcmp
# run: ./benchcmp

