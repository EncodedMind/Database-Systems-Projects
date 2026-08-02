#ifndef HP_FILE_STRUCTS_H
#define HP_FILE_STRUCTS_H

#include <record.h>
#define HEAPFILE_HEADER_BLOCK 0
#define RECORD_OFFSET (sizeof(BlockHeader))

/**
 * @file hp_file_structs.h
 * @brief Data structures for heap file management
 */

/* -------------------------------------------------------------------------- */
/*                              Data Structures                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Heap file header containing metadata about the file organization
 */
typedef struct HeapFileHeader {
    int block_capacity;     // Number of records per block
} HeapFileHeader;

typedef struct BlockHeader {
    int records_num;
} BlockHeader;

/**
 * @brief Iterator for scanning through records in a heap file
 */
typedef struct HeapFileIterator{
    int block_index;
    int record_index;
    int search_id;
    int file_handle;
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */
