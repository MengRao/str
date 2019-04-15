#include <bits/stdc++.h>
#include "../Str.h"

using namespace std;

inline uint64_t getns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

template<size_t Size>
void fillRand(Str<Size>& str) {
  for (int i = 0; i < Size; i++) {
    str[i] = rand() & 0xff;
    if (str[i] == 0) str[i] = 1; // don't allow for null char, otherwise strncmp cheats
  }
}

template<size_t Size>
void bench() {
  const int datasize = 1000;
  const int loop = 1000;
  const int eqrate = 50; // set a equal rate of 50%
  vector<pair<Str<Size>, Str<Size>>> strs(datasize);
  for (int i = 0; i < datasize; i++) {
    fillRand(strs[i].first);
    if (rand() % 100 < eqrate) {
      strs[i].second = strs[i].first;
    }
    else {
      fillRand(strs[i].second);
    }

    int res = strncmp(strs[i].first.s, strs[i].second.s, Size);
    res = (res > 0) - (res < 0);
    int compare_res = strs[i].first.compare(strs[i].second);
    compare_res = (compare_res > 0) - (compare_res < 0);

    int mem_res = memcmp(strs[i].first.s, strs[i].second.s, Size);
    mem_res = (mem_res > 0) - (mem_res < 0);

    assert(res == mem_res);
    assert(res == compare_res);
    assert((strs[i].first == strs[i].second) == (res == 0));
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& pr : strs) {
        sum += pr.first == pr.second;
      }
    }
    auto after = getns();
    cout << "bench " << Size << " eq: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& pr : strs) {
        sum += pr.first.compare(pr.second);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " compare: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& pr : strs) {
        sum += strncmp(pr.first.s, pr.second.s, Size);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " strncmp: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& pr : strs) {
        sum += memcmp(pr.first.s, pr.second.s, Size);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " memcmp: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }
  cout << endl;
}


int main() {
  srand(time(NULL));
  bench<1>();
  bench<2>();
  bench<3>();
  bench<4>();
  bench<7>();
  bench<8>();
  bench<10>();
  bench<12>();
  bench<13>();
  bench<15>();
  bench<16>();
  bench<22>();
  bench<30>();
  bench<50>();
  bench<100>();
  bench<128>();
  bench<300>();
  bench<500>();
  bench<999>();
}


