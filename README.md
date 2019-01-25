## Str
`Str` is a fixed-length char array wrapper providing some frequently used operations in the most efficient way, including string comparison and string to int.

## StrHash
`StrHash` is a special hash table template taking `Str` as key and providing find operation in the most efficient way. It's special in that it can extract features from the keys already in the table and trains its hashing parameters dynamically to better distribute the keys around the table.

`StrHash` is actually a subclass of `std::map`, so user can use whatever funcitons it provides to modify the table, and then call `doneModify` to train the table and `fastFind` to find keys in the table.

`StrHash` has two limitations:
 1) User should call `doneModify` after the table is modified and before the next `fastFind`, and as `doneModify` is very slow it's not efficient to modify the table frequently between `fastFind`.
 2) Currently `StrHash` support a maxmium of 32767 elements in the table.
