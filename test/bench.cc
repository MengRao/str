#include <bits/stdc++.h>
#include "../StrHash.h"

using namespace std;

inline uint64_t getns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

using Key = Str<12>;
using Value = uint16_t;
const int loop = 1000;
std::vector<std::string> tbl_data;
std::vector<std::string> find_data;

void bench_hash() {
  StrHash<12, Value> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i].data(), i + 1);
  }
  ht.doneModify();

  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& s : find_data) {
      sum += ht.fastFind(*(const Key*)s.data());
    }
  }
  auto after = getns();
  cout << "bench_hash sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size()) << endl;
}

void bench_map() {
  StrHash<12, Value> ht; // StrHash is itself a std::map
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i].data(), i + 1);
  }

  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& s : find_data) {
      auto it = ht.find(*(const Key*)s.data());
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size()) << endl;
}

void bench_string_map() {
  map<string, Value> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i], i + 1);
  }
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& s : find_data) {
      auto it = ht.find(s);
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_string_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size())
       << endl;
}

void bench_unordered_map() {
  unordered_map<string, Value> ht;
  for (int i = 0; i < tbl_data.size(); i++) {
    ht.emplace(tbl_data[i], i + 1);
  }
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& s : find_data) {
      auto it = ht.find(s);
      if (it != ht.end()) sum += it->second;
    }
  }
  auto after = getns();
  cout << "bench_unordered_map sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size())
       << endl;
}

void bench_bsearch() {
  int n = tbl_data.size();
  vector<pair<Key, Value>> vec(n);
  for (int i = 0; i < n; i++) {
    vec[i].first = tbl_data[i].data();
    vec[i].second = i + 1;
  }
  sort(vec.begin(), vec.end());
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& s : find_data) {
      const Key& key = *(const Key*)s.data();
      int l = 0, r = n - 1;
      while (l <= r) {
        int m = (l + r) >> 1;
        int cmp = key.compare(vec[m].first);
        if (cmp == 0) {
          sum += vec[m].second;
          break;
        }
        if (cmp < 0)
          r = m - 1;
        else
          l = m + 1;
      }
    }
  }
  auto after = getns();
  cout << "bench_bsearch sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size()) << endl;
}

void bench_string_bsearch() {
  int n = tbl_data.size();
  vector<pair<string, Value>> vec(n);
  for (int i = 0; i < n; i++) {
    vec[i].first = tbl_data[i];
    vec[i].second = i + 1;
  }
  sort(vec.begin(), vec.end());
  int64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& key : find_data) {
      int l = 0, r = n - 1;
      while (l <= r) {
        int m = (l + r) >> 1;
        int cmp = key.compare(vec[m].first);
        if (cmp == 0) {
          sum += vec[m].second;
          break;
        }
        if (cmp < 0)
          r = m - 1;
        else
          l = m + 1;
      }
    }
  }
  auto after = getns();
  cout << "bench_string_bsearch sum: " << sum << " avg lat: " << (double)(after - before) / (loop * find_data.size())
       << endl;
}

int main(int argc, char** argv) {
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

  bench_hash();
  bench_map();
  bench_string_map();
  bench_unordered_map();
  bench_bsearch();
  bench_string_bsearch();

  return 0;
}
