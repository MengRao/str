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
#include "Str.h"
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

namespace strhash_detail {

template<bool SmallTbl>
struct HashType
{ using type = uint16_t; };

template<>
struct HashType<false>
{ using type = uint32_t; };

template<typename T>
inline uint32_t intHash(const T& s) {
  return *(uint32_t*)&s;
}

template<>
inline uint32_t intHash(const Str<2>& s) {
  return *(uint16_t*)&s;
}

template<>
inline uint32_t intHash(const Str<4>& s) {
  return *(uint32_t*)&s;
}

template<>
inline uint32_t intHash(const Str<8>& s) { // simply truncate to lower 4 bytes
  return (uint32_t)(*(uint64_t*)&s);
}

} // namespace

template<size_t StrSZ, typename ValueT, ValueT NullV = 0, uint32_t HashFunc = 0, bool SmallTbl = true>
class StrHash : public std::map<Str<StrSZ>, ValueT>
{
public:
  using KeyT = Str<StrSZ>;
  using Parent = std::map<KeyT, ValueT>;
  using HashT = typename strhash_detail::HashType<SmallTbl>::type;
  static const uint32_t MaxTblSZ = 1u << (SmallTbl ? 15 : 31);
  struct Bucket
  {
    alignas(KeyT::AlignSize) KeyT key;
    HashT hash;
    ValueT value;
    Bucket() = default;
    Bucket(const KeyT& k, const ValueT& v) : key(k), value(v) {}
  };

  bool doneModify() {
    uint32_t n = Parent::size();
    if (n >= MaxTblSZ) return false;
    std::vector<Bucket> tmp_tbl;
    tmp_tbl.reserve(n);
    for (auto& pr : *this) {
      tmp_tbl.emplace_back(pr.first, pr.second);
    }
    findBest(tmp_tbl);
    for (auto& blk : tmp_tbl) {
      blk.hash = calcHash(blk.key);
    }
    std::sort(tmp_tbl.begin(), tmp_tbl.end(), [](const Bucket& a, const Bucket& b) { return a.hash < b.hash; });
    HashT size = tbl_mask + 1;
    tbl.reset(new Bucket[size]);
    for (HashT i = 0; i < size; i++) {
      tbl[i].hash = size;
    }
    for (auto& blk : tmp_tbl) {
      for (HashT pos = blk.hash;; pos = (pos + 1) & tbl_mask) {
        if (tbl[pos].hash == size) {
          tbl[pos] = blk;
          break;
        }
      }
    }

    return true;
  }

  ValueT fastFind(const KeyT& key) const {
    HashT hash = calcHash(key);
    for (HashT pos = hash;; pos = (pos + 1) & tbl_mask) {
      if (tbl[pos].hash > hash) return NullV;
      // it's likely that tbl[pos].hash == hash so we skip checking it
      if (/*tbl[pos].hash == hash && */ tbl[pos].key == key) return tbl[pos].value;
    }
  }

private:
  bool HashFuncUseSalt() const { return HashFunc != 3; }
  bool HashFuncUsePos() const { return HashFunc != 5; }

  HashT calcHash(const KeyT& key) const {
    static_assert(HashFunc <= 6, "unsupported HashFunc");
    uint32_t hash;
    switch (HashFunc) {
      case 0: hash = djbHash1(key); break;
      case 1: hash = djbHash2(key); break;
      case 2: hash = saxHash(key); break;
      case 3: hash = fnvHash(key); break;
      case 4: hash = oatHash(key); break;
      case 5: hash = murmurHash(key); break;
      case 6: hash = intHash(key); break;
    }
    if (SmallTbl) hash ^= (hash >> 16);
    return (HashT)hash & tbl_mask;
  }

  // 0
  uint32_t djbHash1(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h = ((h << 5) + h) + ch;
    }
    return h;
  }

  // 1
  uint32_t djbHash2(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h = ((h << 5) + h) ^ ch;
    }
    return h;
  }

  // 2
  uint32_t saxHash(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h ^= (h << 5) + (h >> 2) + ch;
    }
    return h;
  }

  // 3, hash_salt is not used
  uint32_t fnvHash(const KeyT& key) const {
    uint32_t h = 2166136261;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h = (h * 16777619) ^ ch;
    }
    return h;
  }

  // 4
  uint32_t oatHash(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h += ch;
      h += (h << 10);
      h ^= (h >> 6);
    }
    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    return h;
  }

  // 5, hash_pos is not used
  uint32_t murmurHash(const KeyT& key) const {
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    int len = StrSZ;

    // Initialize the hash to a 'random' value
    unsigned int h = hash_salt ^ len;

    // Mix 4 bytes at a time into the hash
    const unsigned char* data = (const unsigned char*)key.s;
    while (len >= 4) {
      unsigned int k = *(unsigned int*)data;
      k *= m;
      k ^= k >> r;
      k *= m;
      h *= m;
      h ^= k;
      data += 4;
      len -= 4;
    }
    // Handle the last few bytes of the input array
    switch (len) {
      case 3: h ^= data[2] << 16;
      case 2: h ^= data[1] << 8;
      case 1: h ^= data[0]; h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
  }

  // 6: when key is actually an integer(e.g. uint32_t or uint64_t), return itself as hash value
  uint32_t intHash(const KeyT& key) const { return strhash_detail::intHash<KeyT>(key); }

  void findBest(std::vector<Bucket>& tmp_tbl) {
    uint64_t n = tmp_tbl.size();
    std::map<char, uint64_t> chmap[StrSZ];
    for (auto& bkt : tmp_tbl) {
      for (size_t i = 0; i < StrSZ; i++) {
        chmap[i][bkt.key.s[i]]++;
      }
    }
    std::pair<uint64_t, int> chcost[StrSZ];
    for (size_t i = 0; i < StrSZ; i++) {
      chcost[i].second = i;
      chcost[i].first = 0;
      for (auto& mppr : chmap[i]) {
        chcost[i].first += mppr.second * mppr.second;
      }
    }
    std::sort(chcost, chcost + StrSZ);
    for (size_t i = 0; i < StrSZ; i++) {
      hash_pos[i] = chcost[i].second;
    }
    uint64_t max_cost = n * n;
    uint64_t min_cost = n;
    uint64_t good_cost = n + n / 3;

    uint64_t init_tbl_size = 1;
    while (init_tbl_size <= n) init_tbl_size <<= 1;
    uint64_t max_tbl_size = std::min(init_tbl_size * 4, (uint64_t)MaxTblSZ);

    uint32_t best_pos_len = 0, best_mask = init_tbl_size - 1, best_salt = 0;
    uint64_t best_cost = max_cost + 1;

    for (hash_pos_len = 1; hash_pos_len <= StrSZ && chcost[hash_pos_len - 1].first < max_cost;
         hash_pos_len += (HashFuncUsePos() ? 1 : StrSZ)) {
      for (uint32_t tbl_size = init_tbl_size; tbl_size <= max_tbl_size; tbl_size <<= 1) {
        tbl_mask = tbl_size - 1;
        uint32_t max_salt = std::min((uint32_t)tbl_mask, 127U);
        for (hash_salt = 0; hash_salt <= max_salt; hash_salt += (HashFuncUseSalt() ? 1 : tbl_size)) {
          std::map<uint32_t, uint64_t> pos_mp;
          for (auto& blk : tmp_tbl) {
            uint32_t hash = calcHash(blk.key);
            pos_mp[hash]++;
          }
          uint64_t cost = 0;
          for (auto& mppr : pos_mp) {
            cost += mppr.second * mppr.second;
          }
          if (cost < best_cost) {
            best_cost = cost;
            best_salt = hash_salt;
            best_pos_len = hash_pos_len;
            best_mask = tbl_mask;
            /*
            std::cout << "best_cost: " << best_cost << " best_salt: " << best_salt << " best_pos_len: " << best_pos_len
                      << " best_mask: " << best_mask << " min_cost: " << min_cost << " good_cost: " << good_cost
                      << std::endl;
                      */
            if (best_cost == min_cost) return;
          }
        }
        if (best_cost <= good_cost) goto done;
      }
    }
  done:
    hash_salt = best_salt;
    hash_pos_len = best_pos_len;
    tbl_mask = best_mask;
  }

  alignas(64) std::unique_ptr<Bucket[]> tbl;
  uint32_t hash_salt;
  HashT tbl_mask;
  uint16_t hash_pos_len;
  uint16_t hash_pos[StrSZ];
};
