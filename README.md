everdb-native
=============

*everdb-native* is the C language implementation of [everdb](https://github.com/Knio/everdb), an extremely fast, efficient, and safe data storage engine.

[![Build Status][buildlogo-native]](https://travis-ci.org/Knio/everdb-native)
[![Coverage Status][coveragelogo-native]](https://coveralls.io/r/Knio/everdb-native)

[buildlogo-native]: https://travis-ci.org/Knio/everdb-native.svg?branch=master
[coveragelogo-native]: https://img.shields.io/coveralls/Knio/everdb-native.svg?branch=master

## Should I Use this?

No. Check back in Q4 2017.

TODO
----

- [ ] Test windows build
- [x] Setup coverage
- [x] Implement transactions API
- [x] Test transactions API
- [x] Port array structs to use transactions
- [x] Make root object API
- [x] Prototype B+ tree
    - [x] Add
    - [x] Split when full
        - [x] Test
    - [x] Pop
    - [x] Combine when empty
        - [x] Test
    - [x] First/Last
    - [ ] Root API
        - [x] Grow from root
- [ ] Add Checksumming
    - [ ] Page tables
    - [ ] Btree
    - [ ] Array

Features:
- [ ] use `mprotect` on pages
