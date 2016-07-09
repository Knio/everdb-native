


Begin
=====

-push {modified{}, file_len} to txn stack

-copy db root block to secondary block and increment txn id

-modified blocks are allocated from freelist (or file growth)
    -add {oldblock, newblock} to modified{}

-modified blocks are marked as modified

-freed oldblocks are added to modified{} as {oldblock: old, newblock:NULL}

-freed newblocks are added back to freelist, and modified{} entries changed to {oldblock: old, newblock:NULL}



Commit
======

-pop modified{}, file_len from txn stack

-add all oldblock from modified{} to the freelist

-flush all newblock to disk

-flip comitted bit on root db block



Abort
=====

-pop modified{}, file_len from txn stack

-truncate file to file_len

-add all newblocks (< file_len) to freelist


