# External Merge Sort Assignment

This project implements the external merge-sort assignment.

## Implemented Functionality

The assignment required the implementation of the following functions and helper structures:

- `CHUNK_GetNext`
- `CHUNK_GetIthRecordInChunk`
- `CHUNK_UpdateIthRecord`
- `CHUNK_Print(CHUNK chunk)`
- `CHUNK_CreateRecordIterator`
- `CHUNK_GetNextRecord`
- `merge`
- `sort_FileInChunks`

The behavior of these functions is defined in `chunk.h`.

## Overview Of The Implementation

The solution is organized around three phases:

1. **Chunk discovery**: the heap file is divided into chunks of consecutive blocks.
2. **Chunk sorting**: each chunk is sorted in place.
3. **Multi-way merge**: sorted chunks are merged into new heap files until only one run remains.

The implementation relies on the heap-file layer from Assignment 1 and uses the BF library for block access and buffering.

## Function Descriptions

### `CHUNK_CreateIterator`

Creates and initializes a `CHUNK_Iterator` for the file. The iterator starts from block 1 because block 0 contains metadata. The last valid block is obtained with `HP_GetIdOfLastBlock(fileDesc)`.

### `CHUNK_GetNext`

Uses the iterator to compute the boundaries of the next chunk, including the starting block, ending block, number of blocks, and number of records. The record count is computed by summing the result of `HP_GetRecordCounter` for every block in the chunk. The iterator is then advanced to the next chunk.

### `CHUNK_GetIthRecordInChunk`

Traverses the blocks of a chunk and uses `HP_GetRecordCounter` to locate the block that contains the requested record. Once the correct position is found, the record is retrieved with `HP_GetRecord`.

### `CHUNK_UpdateIthRecord`

Works in the same way as `CHUNK_GetIthRecordInChunk`, but updates the selected record through `HP_UpdateRecord`.

### `CHUNK_Print`

Prints all records contained in the chunk by iterating over its blocks and calling `HP_PrintBlockEntries` for each block.

### `CHUNK_CreateRecordIterator`

Creates a `CHUNK_RecordIterator` that starts from the first record of the first block in the chunk. The iterator is initialized with `currentBlockId = chunk->from_BlockId` and `cursor = 0`.

### `CHUNK_GetNextRecord`

Scans the current block of the chunk and returns the next record that is available. When the iterator moves to the next block, the previous block is unpinned with `HP_Unpin`, so the buffer does not fill up unnecessarily.

### `merge`

Performs a `bWay` merge over a group of already sorted chunks and writes the merged result to a new heap file. The function:

1. collects the next available chunks with `CHUNK_GetNext`
2. creates a record iterator for each active chunk
3. repeatedly selects the smallest current record
4. writes that record to the output file
5. advances the corresponding iterator

If a chunk runs out of records, it is marked inactive and excluded from future comparisons. The process stops when no active chunk remains.

### `sort_FileInChunks`

Sorts every chunk of the input file in place. The implementation uses Bubble Sort on each chunk and relies on `CHUNK_GetIthRecordInChunk` and `CHUNK_UpdateIthRecord` for record access and updates. After each chunk has been sorted, all blocks in that chunk are unpinned.

## Experimental Results

We tested external sorting with chunk size 5 blocks and merge fan-out values of 2 and 10. The total number of passes, including the sort pass and all merge passes, was:

| Chunk size (blocks) | Merge fan-out | Total passes |
| --- | --- | --- |
| 5 | 2-way | 9 |
| 5 | 10-way | 4 |

## Design Choices And Assumptions

1. In the provided `mergePhases` code, `outputFileDesc` is not initialized when `chunkSize` exceeds the number of blocks in the file. In that case, the file already consists of a single sorted run, so the correct approach is to initialize `outputFileDesc` with `inputFileDesc`.
2. During the merge phase, blocks are not explicitly loaded all at once. Instead, `CHUNK_GetNextRecord` keeps only one block per active chunk pinned at a time.
3. After sorting a chunk, all of its blocks are unpinned so that the next chunk can be loaded safely.
4. For easier testing, the Makefile includes extra commands for running the program and checking memory behavior.

## Implementation Notes

The following adjustments were made in the generated code:

- `CHUNK_Print`
  The generated code printed records directly with `printf`. This was replaced by `HP_PrintBlockEntries` to keep the implementation simpler and consistent.

- `CHUNK_GetNextRecord`
  The iterator was extended so that the previous block is unpinned when the scan moves to the next block.

- `merge`
  The helper function `findMin` was kept as a regular function instead of a `static` helper. In the main merge loop, the safety check `assert(minIdx >= 0)` was used because an active chunk set should always produce a valid minimum.

- `sort_FileInChunks`
  The redundant `if (chunk.recordsInChunk > 1)` check was removed before calling `sort_Chunk`, and blocks are explicitly unpinned after each chunk is sorted.

## Known Issues Observed During Testing

While running the program under Valgrind, we observed errors and leaks that were not caused by the functions implemented for this assignment. In particular:

- Some leaks disappear when `BF_Close()` is called at the end of `sort_main.c`.
- Some errors disappear when each `Record` is explicitly initialized in `record.c` with:

```c
memset(&record, 0, sizeof(Record));
```

## Build And Run

The Makefile provides the following commands:

```bash
make sort
make run-sort
make run-valgrind
make 2way
make run-2way
make 10way
make run-10way
```

## Requirements

- C / C++ on Linux
- GCC 5.4 or later
- The provided BF and HP libraries in `lib/`
