#include <bits/stdc++.h>
#include "../Str.h"
using namespace std;


inline uint64_t getns() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

uint32_t getRand() {
  uint32_t num = rand() & 0xffff;
  num <<= 16;
  num |= rand() & 0xffff;
  return num;
}

string dest;
char buf[1024];

template<uint32_t Size>
void bench() {
  using NumStr = Str<Size>;
  uint64_t mod = 1;
  for (int i = 0; i < Size; i++) mod *= 10;

  const int datasize = 1000;
  const int loop = 1000;
  vector<uint64_t> nums(datasize);
  vector<NumStr> strs(datasize);
  vector<string> strings(datasize);
  for (int i = 0; i < datasize; i++) {
    uint64_t num = getRand();
    num <<= 32;
    num += getRand();
    num %= mod;
    string str = to_string(num);
    while (str.size() < Size) str = string("0") + str;
    NumStr numstr = str.data();
    assert(numstr.toi64() == num);
    assert(stoll(str) == num);
    assert(strtoll(str.data(), NULL, 10) == num);
    NumStr teststr;
    teststr.fromi(num);
    assert(teststr == numstr);
    sprintf(buf, "%0*lld", Size, num);
    assert(str == buf);
    nums[i] = num;
    strs[i] = numstr;
    strings[i] = str;
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& str : strs) {
        sum += str.toi64();
      }
    }
    auto after = getns();
    cout << "bench " << Size << " toi64: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& str : strings) {
        sum += stoll(str);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " stoll: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto& str : strings) {
        sum += strtoll(str.data(), NULL, 10);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " strtoll: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    union
    {
      uint64_t num;
      char str[Size];
    } res;
    res.num = 0;
    uint64_t sum = 0;
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto num : nums) {
        (*(NumStr*)res.str).fromi(num);
        sum += res.num;
      }
    }
    auto after = getns();
    cout << "bench " << Size << " fromi: " << (double)(after - before) / (loop * datasize) << " res: " << sum << endl;
  }

  {
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto num : nums) {
        dest = to_string(num);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " to_string: " << (double)(after - before) / (loop * datasize) << " res: " << dest
         << endl;
  }

  {
    auto before = getns();
    for (int l = 0; l < loop; l++) {
      for (auto num : nums) {
        sprintf(buf, "%0*lld", Size, num);
      }
    }
    auto after = getns();
    cout << "bench " << Size << " sprintf: " << (double)(after - before) / (loop * datasize) << " res: " << dest
         << endl;
  }

  cout << endl;
}


int main() {
  srand(time(NULL));
  bench<1>();
  bench<2>();
  bench<3>();
  bench<4>();
  bench<5>();
  bench<6>();
  bench<7>();
  bench<8>();
  bench<9>();
  bench<10>();
  bench<11>();
  bench<12>();
  bench<13>();
  bench<14>();
  bench<15>();
  bench<16>();
  bench<17>();
  bench<18>();
}

