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
#include <x86intrin.h>

#if __cplusplus >= 201703L
#include <string_view>
#endif

template<size_t SIZE>
class Str
{
public:
  static const int Size = SIZE;
  // Str is not required to align with AlignSize, in order to provide flexibility as a pure char array wrapper
  // but aligning Str could speed up comparison operations
  static const int AlignSize = SIZE >= 7 ? 8 : 4;
  char s[SIZE];

  Str() {}
  Str(const char* p) { *this = *(const Str<SIZE>*)p; }

  char& operator[](int i) { return s[i]; }
  char operator[](int i) const { return s[i]; }

  bool operator==(const char* p2) const {
#if defined(__AVX512VL__) && defined(__AVX512BW__)
    return simdEQ(s, p2, SIZE);
#else
    return genericEQ(s, p2, SIZE);
#endif
  }

  bool operator==(const Str<SIZE>& rhs) const { return operator==(rhs.s); }
  bool operator!=(const char* p) const { return !operator==(p); }
  bool operator!=(const Str<SIZE>& rhs) const { return !operator==(rhs.s); }

  static bool genericEQ(const char* p1, const char* p2, size_t len) {
    while (len >= 8) {
      if (*(uint64_t*)p1 != *(uint64_t*)p2) return false;
      p1 += 8;
      p2 += 8;
      len -= 8;
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

  int compare(const char* p2) const {
#if defined(__AVX512VL__) && defined(__AVX512BW__)
    return simdCompare(s, p2, SIZE);
#else
    return genericCompare(s, p2, SIZE);
#endif
  }
  int compare(const Str<SIZE>& rhs) const { return compare(rhs.s); }
  bool operator<(const char* p2) const { return compare(p2) < 0; }
  bool operator<(const Str<SIZE>& rhs) const { return compare(rhs.s) < 0; }

  static int genericCompare(const char* p1, const char* p2, size_t len) {
    while (len >= 8) {
      uint64_t mask = *(uint64_t*)p1 ^ *(uint64_t*)p2;
      if (mask) {
        int i = __builtin_ctzll(mask) >> 3;
        return (int)(uint8_t)p1[i] - (int)(uint8_t)p2[i];
      }
      p1 += 8;
      p2 += 8;
      len -= 8;
    }
    for (size_t i = 0; i < len; i++) {
      int res = (int)(uint8_t)p1[i] - (int)(uint8_t)p2[i];
      if (res) return res;
    }
    return 0;
  }

#if __cplusplus >= 201703L
  std::string_view tosv() const { return std::string_view(s, SIZE); }
#endif

#if defined(__AVX512VL__) && defined(__AVX512BW__)
  static bool simdEQ(const char* p1, const char* p2, size_t len) {
    while (len >= 64) {
      uint64_t mask = _mm512_cmpneq_epu8_mask(_mm512_loadu_si512(p1), _mm512_loadu_si512(p2));
      if (mask) return false;
      p1 += 64;
      p2 += 64;
      len -= 64;
    }

    if (len >= 32) {
      uint32_t mask = _mm256_cmpneq_epu8_mask(_mm256_loadu_si256((__m256i*)p1), _mm256_loadu_si256((__m256i*)p2));
      if (mask) return false;
      p1 += 32;
      p2 += 32;
      len -= 32;
    }
    if (len >= 16) {
      uint16_t mask = _mm_cmpneq_epu8_mask(_mm_loadu_si128((__m128i*)p1), _mm_loadu_si128((__m128i*)p2));
      if (mask) return false;
      p1 += 16;
      p2 += 16;
      len -= 16;
    }
    return genericEQ(p1, p2, len);
  }

  static int simdCompare(const char* p1, const char* p2, size_t len) {
    while (len >= 64) {
      uint64_t mask = _mm512_cmpneq_epu8_mask(_mm512_loadu_si512(p1), _mm512_loadu_si512(p2));
      if (mask) {
        int i = __builtin_ctzll(mask);
        return (int)(uint8_t)p1[i] - (int)(uint8_t)p2[i];
      }
      p1 += 64;
      p2 += 64;
      len -= 64;
    }
    if (len >= 32) {
      uint32_t mask = _mm256_cmpneq_epu8_mask(_mm256_loadu_si256((__m256i*)p1), _mm256_loadu_si256((__m256i*)p2));
      if (mask) {
        int i = __builtin_ctz(mask);
        return (int)(uint8_t)p1[i] - (int)(uint8_t)p2[i];
      }
      p1 += 32;
      p2 += 32;
      len -= 32;
    }
    if (len >= 16) {
      uint16_t mask = _mm_cmpneq_epu8_mask(_mm_loadu_si128((__m128i*)p1), _mm_loadu_si128((__m128i*)p2));
      if (mask) {
        int i = __builtin_ctz(mask);
        return (int)(uint8_t)p1[i] - (int)(uint8_t)p2[i];
      }
      p1 += 16;
      p2 += 16;
      len -= 16;
    }
    return genericCompare(p1, p2, len);
  }
#endif

  uint32_t toi() const {
    uint32_t ret = 0;
    switch (SIZE) {
      case 10: ret += (s[SIZE - 10] - '0') * 1000000000;
      case 9: ret += (s[SIZE - 9] - '0') * 100000000;
#ifdef __SSE4_1__
      case 8: ret += simdtoi(s + SIZE - 8); return ret;
#else
      case 8: ret += (s[SIZE - 8] - '0') * 10000000;
#endif
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

  uint64_t toi64() const {
    uint64_t ret = 0;
    switch (SIZE) {
      case 19: ret += (s[SIZE - 19] - '0') * 1000000000000000000LL;
      case 18: ret += (s[SIZE - 18] - '0') * 100000000000000000LL;
      case 17: ret += (s[SIZE - 17] - '0') * 10000000000000000LL;
#ifdef __SSE4_1__
      case 16: ret += simdtoi64(s + SIZE - 16); return ret;
#else
      case 16: ret += (s[SIZE - 16] - '0') * 1000000000000000LL;
#endif
      case 15: ret += (s[SIZE - 15] - '0') * 100000000000000LL;
      case 14: ret += (s[SIZE - 14] - '0') * 10000000000000LL;
      case 13: ret += (s[SIZE - 13] - '0') * 1000000000000LL;
      case 12: ret += (s[SIZE - 12] - '0') * 100000000000LL;
      case 11: ret += (s[SIZE - 11] - '0') * 10000000000LL;
      case 10: ret += (s[SIZE - 10] - '0') * 1000000000LL;
      case 9: ret += (s[SIZE - 9] - '0') * 100000000LL;
#ifdef __SSE4_1__
      case 8: ret += simdtoi(s + SIZE - 8); return ret;
#else
      case 8: ret += (s[SIZE - 8] - '0') * 10000000LL;
#endif
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

#ifdef __SSE4_1__
  // covert 8 digits into int
  // https://arxiv.org/pdf/1902.08318.pdf, Fig.7
  static uint32_t simdtoi(const char* p) {
    __m128i ascii0 = _mm_set1_epi8('0');
    __m128i mul_1_10 = _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
    __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
    __m128i mul_1_10000 = _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);
    // we should've used _mm_loadu_si64 here, but seems _mm_loadu_si128 is faster
    __m128i in = _mm_sub_epi8(_mm_loadu_si128((__m128i*)p), ascii0);
    __m128i t1 = _mm_maddubs_epi16(in, mul_1_10);
    __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
    __m128i t3 = _mm_packus_epi32(t2, t2);
    __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);
    return _mm_cvtsi128_si32(t4);
  }

  // covert 16 digits into int64
  static uint64_t simdtoi64(const char* p) {
    __m128i ascii0 = _mm_set1_epi8('0');
    __m128i mul_1_10 = _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
    __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
    __m128i mul_1_10000 = _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);
    __m128i in = _mm_sub_epi8(_mm_loadu_si128((__m128i*)p), ascii0);
    __m128i t1 = _mm_maddubs_epi16(in, mul_1_10);
    __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
    __m128i t3 = _mm_packus_epi32(t2, t2);
    __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);
    // the above code is exactly the same as simdtoi
    uint64_t t5 = _mm_cvtsi128_si64(t4);
    return (t5 >> 32) + (t5 & 0xffffffff) * 100000000LL;
  }
#endif

  template<typename T>
  void fromi(T num) {
    if (Size & 1) {
      s[Size - 1] = '0' + (num % 10);
      num /= 10;
    }
    switch (Size & -2) {
      case 18: *(uint16_t*)(s + 16) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 16: *(uint16_t*)(s + 14) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 14: *(uint16_t*)(s + 12) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 12: *(uint16_t*)(s + 10) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 10: *(uint16_t*)(s + 8) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 8: *(uint16_t*)(s + 6) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 6: *(uint16_t*)(s + 4) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 4: *(uint16_t*)(s + 2) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
      case 2: *(uint16_t*)(s + 0) = *(uint16_t*)(digit_pairs + ((num % 100) << 1)); num /= 100;
    }
  }

  static constexpr const char* digit_pairs = "00010203040506070809"
                                             "10111213141516171819"
                                             "20212223242526272829"
                                             "30313233343536373839"
                                             "40414243444546474849"
                                             "50515253545556575859"
                                             "60616263646566676869"
                                             "70717273747576777879"
                                             "80818283848586878889"
                                             "90919293949596979899";
};

template<size_t SIZE>
std::ostream& operator<<(std::ostream& os, const Str<SIZE>& str) {
  os.write(str.s, SIZE);
  return os;
}

