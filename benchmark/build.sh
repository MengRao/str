g++ -march=native -O3 benchfindstr.cc -o benchfindstr
# run: ./benchfindstr < data.txt

g++ -march=native -O3 benchfindint.cc -o benchfindint
# run: ./benchfindint < integers.txt

g++ -march=native -O3 benchnum.cc -o benchnum
# run: ./benchnum

g++ -march=native -O3 benchcmp.cc -o benchcmp
# run: ./benchcmp

