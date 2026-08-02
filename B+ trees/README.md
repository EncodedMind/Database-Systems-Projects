# B+ Tree Assignment

This project implements the B+ tree assignment.

## Implemented Functionality

The following functions were implemented according to the behavior specified in `bplus_file_funcs.h`:

- `bplus_create_file`
- `bplus_open_file`
- `bplus_close_file`
- `bplus_record_insert`
- `bplus_record_find`

## Main Assumptions

### Fixed Number of Keys and Records Per Node

The maximum number of keys in an index node and the maximum number of records in a data node are defined statically and do not depend on the actual block size or record size:

```c
#define BPLUS_INDEX_NODE_MAX_KEYS 2
#define BPLUS_DATANODE_MAX_RECORDS 2
```

This choice keeps the implementation simple and makes insertions and general tree operations easier to manage. It also avoids the need for more complex node packing logic. The trade-off is that blocks are not fully utilized, and search is implemented linearly inside the nodes because the node sizes are intentionally small.

### Metadata Remains in Main Memory

The file metadata is loaded through `bplus_open_file`, and the returned pointer must refer directly to the metadata stored in block 0. This ensures that updates are always reflected in the real block contents.

In addition, the metadata block must remain pinned in memory for as long as the file is open, so the tree can always access up-to-date information such as the root block number.

### The Initial Root Is a Data Node

When the file is created, the root is initialized as a data node. There is no need for an index node at that stage because the tree has not yet split.

### Nodes Are Addressed By Block Number

Each node, whether it is an index node or a data node, is accessed through the block number assigned during creation. This simplifies pointer management between nodes and makes traversal straightforward.

## Function Overview

### `bplus_create_file`

Creates a new B+ tree file and initializes its structure. Block 0 stores the metadata (`BPlusMeta`), and the initial root is created as a data block containing an empty `BPlusDataNode`.

### `bplus_open_file`

Opens the file through the BF layer and returns a pointer to the `BPlusMeta` structure stored in block 0. This metadata is used by all later operations.

### `bplus_close_file`

Before closing the file, the metadata block is updated so that any changes, such as a new root after a split, are written back to disk. The file is then closed normally.

### `bplus_record_insert`

1. The correct data block is located by descending the tree with `bplus_find_data_block_num`.
2. If the record fits, it is inserted into the data node and the operation completes.
3. If the data node overflows, the block is split into two new blocks.
4. The middle key is promoted to the parent.
5. If the parent overflows, the split is propagated upward recursively.
6. If there is no parent, a new root index node is created.

This process keeps the tree balanced. Data-node splits and index-node splits are handled separately, and sibling pointers are updated during splits to preserve structural consistency.

### `bplus_record_find`

Record search follows the standard B+ tree logic:

1. Starting from the root, the algorithm follows the appropriate child pointers through index nodes.
2. Once the target data block is reached, the records inside it are scanned linearly.
3. If the record is found, a copy is returned to the caller; otherwise, the result is `NULL`.

## Project Structure

The provided project layout is:

- `build/` for generated binaries and object files
- `include/` for header files
- `lib/` for the BF library
- `src/` for source files
- `examples/` for the example `main` program

## Notes For Testing

- The assignment provides `examples/bplus_main.c` as a starting point.
- The example creates two B+ trees, one for employees and one for students.
- The `main` function can be extended to test more insertions and searches.

## Important Constraints

- The functions declared in `bplus_file_funcs.h` must not be changed.
- The `record` library must also remain unchanged.
- When a `BF_Block` is modified, `BF_Block_SetDirty` must be called.
- When a block is no longer needed, `BF_UnpinBlock` must be called.
- The project is expected to compile and run on Linux with GCC 5.4+.

## Build And Run

The provided `Makefile` supports the assignment-specific targets, including:

```bash
make bplus_main_compile
make bplus_main_run
make bplus_main_run_valgrind
```