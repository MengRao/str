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

template<size_t StrSZ, typename ValueT, ValueT NullV = 0>
class StrHash : public std::map<Str<StrSZ>, ValueT>
{
  using KeyT = Str<StrSZ>;
  using Parent = std::map<KeyT, ValueT>;
  static const uint32_t MaxTblSZ = 1 << 15;
  struct Bucket
  {
    alignas(KeyT::AlignSize) KeyT key;
    uint16_t hash;
    ValueT value;
  };

public:
  bool doneModify() {
    uint32_t n = Parent::size();
    if (n >= MaxTblSZ) return false;
    std::vector<Bucket> tmp_tbl;
    tmp_tbl.reserve(n);
    for (auto& pr : *this) {
      tmp_tbl.emplace_back();
      tmp_tbl.back().key = pr.first;
      tmp_tbl.back().value = pr.second;
    }
    findBest(tmp_tbl);
    for (auto& blk : tmp_tbl) {
      blk.hash = calcHash(blk.key);
    }
    std::sort(tmp_tbl.begin(), tmp_tbl.end(), [](const Bucket& a, const Bucket& b) { return a.hash < b.hash; });
    uint16_t size = tbl_mask + 1;
    tbl.reset(new Bucket[size]);
    for (uint16_t i = 0; i < size; i++) {
      tbl[i].hash = size;
    }
    for (auto& blk : tmp_tbl) {
      for (uint16_t pos = blk.hash;; pos = (pos + 1) & tbl_mask) {
        if (tbl[pos].hash == size) {
          tbl[pos] = blk;
          break;
        }
      }
    }

    return true;
  }

  ValueT fastFind(const KeyT& key) {
    uint16_t hash = calcHash(key);
    for (uint16_t pos = hash;; pos = (pos + 1) & tbl_mask) {
      if (tbl[pos].hash > hash) return NullV;
      // it's likely that tbl[pos].hash == hash so we skip checking it
      if (tbl[pos].key == key) return tbl[pos].value;
    }
  }

private:
  uint16_t calcHash(const KeyT& key) const {
    uint32_t hash = djbHash1(key); // try using different hash functions to benchmark
    hash ^= (hash >> 16);
    return (uint16_t)(hash & tbl_mask);
  }

  uint32_t djbHash1(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h = ((h << 5) + h) + ch;
    }
    return h;
  }

  uint32_t djbHash2(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h = ((h << 5) + h) ^ ch;
    }
    return h;
  }

  uint32_t saxHash(const KeyT& key) const {
    uint32_t h = hash_salt;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h ^= (h << 5) + (h >> 2) + ch;
    }
    return h;
  }

  // hash_salt is not used
  uint32_t fnvHash(const KeyT& key) const {
    uint32_t h = 2166136261;
    for (int i = 0; i < hash_pos_len; i++) {
      char ch = key.s[hash_pos[i]];
      h = (h * 16777619) ^ ch;
    }
    return h;
  }

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

  // hash_pos is not used
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

  void findBest(std::vector<Bucket>& tmp_tbl) {
    uint32_t n = tmp_tbl.size();
    std::map<char, uint32_t> chmap[StrSZ];
    for (auto& bkt : tmp_tbl) {
      for (int i = 0; i < StrSZ; i++) {
        chmap[i][bkt.key.s[i]]++;
      }
    }
    std::pair<uint32_t, int> chcost[StrSZ];
    for (int i = 0; i < StrSZ; i++) {
      chcost[i].second = i;
      chcost[i].first = 0;
      for (auto& mppr : chmap[i]) {
        chcost[i].first += mppr.second * mppr.second;
      }
    }
    std::sort(chcost, chcost + StrSZ);
    for (int i = 0; i < StrSZ; i++) {
      hash_pos[i] = chcost[i].second;
    }
    uint32_t max_cost = n * n;
    uint32_t min_cost = n;
    uint32_t good_cost = n + n / 3;

    uint32_t init_tbl_size = 1;
    while (init_tbl_size <= n) init_tbl_size <<= 1;
    uint32_t max_tbl_size = std::min(init_tbl_size * 4, MaxTblSZ);

    uint16_t best_pos_len, best_mask;
    uint32_t best_salt, best_cost = max_cost + 1;

    for (hash_pos_len = 1; hash_pos_len <= StrSZ && chcost[hash_pos_len - 1].first < max_cost; hash_pos_len++) {
      for (uint32_t tbl_size = init_tbl_size; tbl_size <= max_tbl_size; tbl_size <<= 1) {
        tbl_mask = tbl_size - 1;
        for (hash_salt = 0; hash_salt <= tbl_mask; hash_salt++) {
          std::map<uint16_t, uint32_t> pos_mp;
          for (auto& blk : tmp_tbl) {
            uint16_t hash = calcHash(blk.key);
            pos_mp[hash]++;
          }
          uint32_t cost = 0;
          for (auto& mppr : pos_mp) {
            cost += mppr.second * mppr.second;
          }
          if (cost < best_cost) {
            best_cost = cost;
            best_salt = hash_salt;
            best_pos_len = hash_pos_len;
            best_mask = tbl_mask;
            // std::cout << "best_cost: " << best_cost << " best_salt: " << best_salt << " best_pos_len: " <<
            // best_pos_len
            //<< " best_mask: " << best_mask << std::endl;
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
  uint16_t tbl_mask;
  uint16_t hash_pos_len;
  uint32_t hash_pos[StrSZ];
};
