#include <bits/stdc++.h>
#include "../Str.h"

using namespace std;

inline uint64_t getns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

template<uint32_t Size>
void bench() {
  using NumStr = Str<Size>;
  uint32_t mod = 1;
  for (int i = 0; i < Size; i++) mod *= 10;

  const int datasize = 1000;
  const int loop = 1000;
  vector<uint32_t> nums(datasize);
  vector<NumStr> strs(datasize);
  for (int i = 0; i < datasize; i++) {
    uint32_t num = rand() & 0xffff;
    num <<= 16;
    num |= rand() & 0xffff;
    num %= mod;
    string str = to_string(num);
    while (str.size() < Size) str = string("0") + str;
    NumStr numstr = str.data();
    assert(numstr.toi() == num);
    NumStr teststr;
    teststr.fromi2(num);
    assert(teststr == numstr);
    nums[i] = num;
    strs[i] = numstr;
  }

  uint64_t sum = 0;
  auto before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto& str : strs) {
      sum += str.toi();
    }
  }
  auto after = getns();
  cout << "bench " << Size << " toi: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;

  union
  {
    uint64_t num;
    char str[Size];
  } res;
  res.num = 0;
  before = getns();
  for (int l = 0; l < loop; l++) {
    for (auto num : nums) {
      (*(NumStr*)res.str).fromi2(num);
      sum += res.num;
    }
  }
  after = getns();
  cout << "bench " << Size << " fromi: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
}


int main() {
  bench<1>();
  bench<2>();
  bench<3>();
  bench<4>();
  bench<5>();
  bench<6>();
  bench<7>();
  bench<8>();
  bench<9>();
}

