## Str
`Str` is a char array wrapper providing some frequently used operations in the most efficient way, including string comparison and string to int.

## StrHash
`StrHash` is an adaptive open addressing hash table template taking `Str` as key and providing a find function in the most efficient way. It's adaptive in that it can extract features from the keys contained in the table and trains its hashing parameters dynamically to better distribute the keys to avoid collision as much as possible.

`StrHash` is actually a subclass of `std::map`, so user can use whatever funcitons it provides to modify the table, and then call `doneModify` to train the table and `fastFind` to find keys in the table. Note that `doneModify` is pretty slow so it's not efficient to modify the table frequently between `fastFind`.

`StrHash` currently supports 6 hash functions and one of which can be selected using template parameter `HashFunc`:
* 0: djb ver1(default)
* 1: djb ver2
* 2: sax
* 3: fnv
* 4: oat
* 5: murmur

User can also add other hash functions easily.

## Benchmark
Tests show that `StrHash` is 7x faster than `std::unordered_map` and 3x faster than other open addressing hash table implementations such as `tsl::hopscotch_map` and `tsl::robin_map`, and that `Str` is 2x faster than `std::string` with SSO.

`bench.cc` tests the performance of multiple string search solutions using the same data set. The data set contains the KRX option issue codes of Feb 2019 that we are interested in and are to be inserted into the table, and the first 1000 option issue codes we received from the market data(which are mostly of Feb 2019 but some are of other months) and are to be searched in the table.
In `bench.cc`: 
* `bench_hash<0~5>` compair the performance of different hash functions `StrHash` supports.
* `bench_hash` vs other bench solutions shows how `StrHash` is faster than others.
* `bench_map` vs `bench_string_map` and `bench_bsearch` vs `bench_string_bsearch` show how `Str` is faster than `std::string`.
