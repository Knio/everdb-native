
Linear hashing is an algorithm to resize a hash table by a single bucket at a
time, which is parameterize by three numbers: N, L, S.

The algoritm logically divides the total buckets (N) into two sections: the
first of group which is always a power of two in size (L), and the second group
which new buckets are allocated to.

Each bucket in the second (growing) half of the table is paired with a bucket
in the first half of the table, with the relation i = (j - L). To expand the table,
a bucket in the first section of the table is split in half such that half its
contents remain in place, and half are moved to a newly allocated bucket in the
growing section. The final parameter (S) points to the next such block that will
be split when the table needs to grow, so that S = N - L.

Consider an example:

    N = 6

           L = 4       Growing
     ______/\______  __/\__
    /              \/      \
      0   1   2   3   4   5   6
     ___ ___ ___ ___ ___ ___
    |   |   |   |   |   |   |
    |___|___|___|___|___|___|

              ^
              |
              S = 2


This hash table has 6 buckets, four in the fixed secrion, and two in the growing
section. In this hash table, buckes 4 and 0 are paired, and 5 and one are paired.
it helps to visualize this algoritm as the two sections being distict tables
stacked on top of eachother:

    N = 6

           L = 4
     ______/\______
    /              \
     ___ ___ ___ ___
    | 0 | 1 | 2 | 3 |
    |___|___|___|___|
     ___ ___  ^
    | 4 | 5 |  \
    |___|___|   S = 2

    \___  __/
        \/
        Growing



To grow this table to N = 7, we split bucket 2 into itself and a newly allocated
bucket in the growing section:

    N = 7

           L = 4
     ______/\______
    /              \
     ___ ___ ___ ___
    | 0 | 1 | 2 | 3 |
    |___|___|   |___|
     ___ ___ > <  ^
    | 4 | 5 | 6 |  \
    |___|___|___|   S = 3

    \____  ____/
         \/
         Growing


Let's grow again::

    N = 8

           L = 4
     ______/\______
    /              \
     ___ ___ ___ ___
    | 0 | 1 | 2 | 3 |
    |___|___|___|   |
     ___ ___ ___ >_<  ^
    | 4 | 5 | 6 | 7 |   \
    |___|___|___|___|    S = 4

    \______  ______/
           \/
           Growing


Notice that S no longer points to a valid bucket in the first section. When
this happens (S == L), we double L, and reset S to 0, so our stacked
visualization now looks like this:

    N = 8

                    L = 8
     _______________/\_____________
    /                              \
     ___ ___ ___ ___ ___ ___ ___ ___
    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    |___|___|__ |___|___|___|___|___|
      ^
       \
    ||  S = 0
    \/
    Growing


And we continue:

    N = 9

                    L = 8
     _______________/\_____________
    /                              \
     ___ ___ ___ ___ ___ ___ ___ ___
    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    |   |___|__ |___|___|___|___|___|
     > <  ^
    | 8 |  \
    |___|   S = 1

    \  /
     \/
    Growing



Hashing

To determine the bucket for a hash value H, we first assume that the hash table
is larger than it actually is, (2*L) and compute bucket = H % (2*L).
If the result of the computaion is less than N, then we use it as-is.

If the computed bucket is >= N, then this can be thought of as a bucket in the
growing section that does not yet exist. Using the relation between the two sets
of buckets, we look up the value in (bucket - L) instead (alertately, H % L).

It is this hashing relation that allows us to know for sure that when bucket S
is split, its entries will end up divided between buckets S and S + L.
