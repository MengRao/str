/*
MIT License

Copyright (c) 2019 Meng Rao <raomeng1@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#pragma once
#include <iostream>

#if __cplusplus >= 201703L
#include <string_view>
#endif

template<size_t SIZE>
class Str
{
public:
  static const int AlignSize = SIZE >= 7 ? 8 : 4;
  char s[SIZE];

  Str() {}
  Str(const char* p) { *this = *(const Str<SIZE>*)p; }

  bool operator==(const char* p2) const {
    const char* p1 = s;
    uint32_t len = SIZE;
    while (len >= 8) {
      if (*(uint64_t*)p1 != *(uint64_t*)p2) return false;
      p1 += 8;
      p2 += 8;
      len -= 8;
    }
    if (len == 7) {
      return ((*(uint64_t*)p1 ^ *(uint64_t*)p2) & 0x00ffffffffffffffUL) == 0UL;
    }
    if (len >= 4) {
      if (*(uint32_t*)(p1) != *(uint32_t*)(p2)) return false;
      p1 += 4;
      p2 += 4;
      len -= 4;
    }
    switch (len) {
      case 1: return *p1 == *p2;
      case 2: return *(uint16_t*)p1 == *(uint16_t*)p2;
      case 3: return *(uint16_t*)p1 == *(uint16_t*)p2 && p1[2] == p2[2];
    };
    return true;
  }
  bool operator==(const Str<SIZE>& rhs) const { return operator==(rhs.s); }
  bool operator!=(const char* p) const { return !operator==(p); }
  bool operator!=(const Str<SIZE>& rhs) const { return !operator==(rhs.s); }

  int compare(const char* p2) const {
    const char* p1 = s;
    int len = SIZE;
    while (len >= 8) {
      if (*(uint64_t*)p1 != *(uint64_t*)p2) {
        for (int i = 0; i < 8; i++) {
          int res = p1[i] - p2[i];
          if (res) return res;
        }
        // impossible to go here!
        return 0;
      }
      p1 += 8;
      p2 += 8;
      len -= 8;
    }
    for (int i = 0; i < len; i++) {
      int res = p1[i] - p2[i];
      if (res) return res;
    }
    return 0;
  }
  int compare(const Str<SIZE>& rhs) const { return compare(rhs.s); }
  bool operator<(const char* p2) const { return compare(p2) < 0; }
  bool operator<(const Str<SIZE>& rhs) const { return compare(rhs.s) < 0; }

#if __cplusplus >= 201703L
  std::string_view tosv() const { return std::string_view(s, SIZE); }
#endif

  int32_t toi() const {
    int32_t ret = 0;
    switch (SIZE) {
      case 10: ret += (s[SIZE - 10] - '0') * 1000000000;
      case 9: ret += (s[SIZE - 9] - '0') * 100000000;
      case 8: ret += (s[SIZE - 8] - '0') * 10000000;
      case 7: ret += (s[SIZE - 7] - '0') * 1000000;
      case 6: ret += (s[SIZE - 6] - '0') * 100000;
      case 5: ret += (s[SIZE - 5] - '0') * 10000;
      case 4: ret += (s[SIZE - 4] - '0') * 1000;
      case 3: ret += (s[SIZE - 3] - '0') * 100;
      case 2: ret += (s[SIZE - 2] - '0') * 10;
      case 1: ret += (s[SIZE - 1] - '0');
    };
    return ret;
  }

  int64_t toi64() const {
    int64_t ret = 0;
    switch (SIZE) {
      case 19: ret += (s[SIZE - 19] - '0') * 1000000000000000000LL;
      case 18: ret += (s[SIZE - 18] - '0') * 100000000000000000LL;
      case 17: ret += (s[SIZE - 17] - '0') * 10000000000000000LL;
      case 16: ret += (s[SIZE - 16] - '0') * 1000000000000000LL;
      case 15: ret += (s[SIZE - 15] - '0') * 100000000000000LL;
      case 14: ret += (s[SIZE - 14] - '0') * 10000000000000LL;
      case 13: ret += (s[SIZE - 13] - '0') * 1000000000000LL;
      case 12: ret += (s[SIZE - 12] - '0') * 100000000000LL;
      case 11: ret += (s[SIZE - 11] - '0') * 10000000000LL;
      case 10: ret += (s[SIZE - 10] - '0') * 1000000000LL;
      case 9: ret += (s[SIZE - 9] - '0') * 100000000LL;
      case 8: ret += (s[SIZE - 8] - '0') * 10000000LL;
      case 7: ret += (s[SIZE - 7] - '0') * 1000000LL;
      case 6: ret += (s[SIZE - 6] - '0') * 100000LL;
      case 5: ret += (s[SIZE - 5] - '0') * 10000LL;
      case 4: ret += (s[SIZE - 4] - '0') * 1000LL;
      case 3: ret += (s[SIZE - 3] - '0') * 100LL;
      case 2: ret += (s[SIZE - 2] - '0') * 10LL;
      case 1: ret += (s[SIZE - 1] - '0');
    };
    return ret;
  }
};

template<size_t SIZE>
std::ostream& operator<<(std::ostream& os, const Str<SIZE>& str) {
  os.write(str.s, SIZE);
  return os;
}

