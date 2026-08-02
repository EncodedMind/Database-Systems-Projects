# Heap File Assignment

This project implements a simple heap file layer on top of the Block File (BF) library.

The BF layer is provided as a prebuilt library and must not be modified. The goal of this assignment is to implement record management for heap files using BF blocks.

## What The Heap File Does

The heap file layer is responsible for:

- creating and opening heap files
- inserting records into blocks
- scanning and printing records
- closing heap files and releasing resources

Heap files store records of the form:

```c
typedef struct {
    int id;
    char name[20];
    char surname[20];
    char city[20];
} Record;
```

The first block of every heap file stores metadata about the file. The actual records begin in block 1.

## Project Layout

- `src/` contains the source files
- `include/` contains the header files
- `examples/` contains example main programs
- `lib/` contains the provided BF library
- `build/` contains the generated executables

## Files To Implement

The assignment requires the heap-file functionality to be implemented only in:

- `src/hp_file.c`
- `include/hp_file_structs.h`

Changes in other files are ignored during grading.

## Required Functions

The heap-file API is defined in `include/hp_file_funcs.h`. The main functions are:

- `HeapFile_Create`
- `HeapFile_Open`
- `HeapFile_Close`
- `HeapFile_InsertRecord`
- `HeapFile_CreateIterator`
- `HeapFile_GetNextRecord`

The helper operations provided by the assignment include:

- `HeapFile_GetRecordCounter`
- `HeapFile_GetIdOfLastBlock`
- `HeapFile_GetMaxRecordsInBlock`
- `HeapFile_PrintAllEntries`
- `HeapFile_PrintBlockEntries`
- `HeapFile_GetRecord`
- `HeapFile_UpdateRecord`
- `HeapFile_Unpin`

## BF Library Notes

The BF library provides block-level file management and caching. The important rules are:

- call `BF_Block_SetDirty` whenever a block is modified
- call `BF_UnpinBlock` when a block is no longer needed
- call `BF_Close` at the end of execution to flush remaining blocks

The relevant BF constants are:

- `BF_BLOCK_SIZE = 512`
- `BF_BUFFER_SIZE = 100`
- `BF_MAX_OPEN_FILES = 100`

## Build And Run

The provided `Makefile` supports the following commands:

```bash
make bf
make hp
make run-bf
make run-hp
```

## Notes From The Assignment

- The heap file may contain multiple records with the same `id`.
- The implementation should minimize disk reads and writes.
- The project is intended for Linux and GCC.

## Requirements

- GCC compiler
- Make
- The provided `libbf.so` library in `lib/`