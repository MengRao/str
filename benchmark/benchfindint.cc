#include <bits/stdc++.h>
#include "../StrHash.h"
#include "tsl/robin_map.h"
#include "tsl/hopscotch_map.h"

using namespace std;

inline uint64_t getns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

using IntT = uint64_t;
using Key = Str<8>;
using Value = uint16_t;
const int loop = 1000;
vector<IntT> tbl_data;
vector<IntT> find_data;

template<uint32_t HashFunc>
void bench_hash() {
  StrHash<8, Value, 0, HashFunc> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace((const char*)&tbl_data[i], i + 1);
  }
  ht.doneModify();

  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto s : find_data) {
      sum += ht.fastFind(*(const Key*)&s);
    }
  }
  auto after = getns();
  cout << "bench_hash " << HashFunc << " sum: " << sum
       << " avg lat: " << (double)(after - before) / (loop * find_data.size()) << endl;
}

void bench_map() {
  map<IntT, Value> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i], i + 1);
  }

  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto s : find_data) {
      auto it = ht.find(s);
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size()) << endl;
}

void bench_unordered_map() {
  unordered_map<IntT, Value> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i], i + 1);
  }

  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto s : find_data) {
      auto it = ht.find(s);
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_unordered_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size())
       << endl;
}

void bench_robin_map() {
  tsl::robin_map<IntT, Value, std::hash<IntT>, std::equal_to<IntT>, std::allocator<std::pair<IntT, Value>>, true> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i], i + 1);
  }
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto s : find_data) {
      auto it = ht.find(s);
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_robin_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size())
       << endl;
}

void bench_hopscotch_map() {
  tsl::hopscotch_map<IntT, Value, std::hash<IntT>, std::equal_to<IntT>, std::allocator<std::pair<IntT, Value>>, 10,
                     true>
    ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i], i + 1);
  }
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto s : find_data) {
      auto it = ht.find(s);
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_hopscotch_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size())
       << endl;
}

void bench_bsearch() {
  int n = tbl_data.size();
  vector<pair<IntT, Value>> vec(n);
  for (int i = 0; i < n; i++) {
    vec[i].first = tbl_data[i];
    vec[i].second = i + 1;
  }
  sort(vec.begin(), vec.end());
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto s : find_data) {
      int l = 0, r = n - 1;
      while (l <= r) {
        int m = (l + r) >> 1;
        if (vec[m].first == s) {
          sum += vec[m].second;
          break;
        }
        if (vec[m].first > s)
          r = m - 1;
        else
          l = m + 1;
      }
    }
  }
  auto after = getns();
  cout << "bench_bsearch sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size()) << endl;
}

int main() {
  int n;
  cin >> n;
  tbl_data.resize(n);
  for (int i = 0; i < n; i++) {
    cin >> tbl_data[i];
  }

  cin >> n;
  find_data.resize(n);
  for (int i = 0; i < n; i++) {
    cin >> find_data[i];
  }

  bench_hash<0>();
  bench_hash<1>();
  bench_hash<2>();
  bench_hash<3>();
  bench_hash<4>();
  bench_hash<5>();
  bench_hash<6>(); // 6 is for integer key
  bench_map();
  bench_unordered_map();
  bench_robin_map();
  bench_hopscotch_map();
  bench_bsearch();

  return 0;
}
