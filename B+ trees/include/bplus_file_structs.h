//
// Created by theofilos on 11/4/25.
//

#ifndef BPLUS_BPLUS_FILE_STRUCTS_H
#define BPLUS_BPLUS_FILE_STRUCTS_H
#include <stdlib.h>

#include "bf.h"
#include "bplus_datanode.h"
#include "bplus_file_structs.h"
#include "bplus_index_node.h"
#include "record.h"

#define BPLUS_META_BLOCK_NUM 0
#define BPLUS_BLOCK_NODE_OFFSET sizeof(BPlusMeta)

typedef struct {
    int root_block_num;
    TableSchema schema;
} BPlusMeta;

BPlusMeta *bplus_meta_create(const TableSchema *schema);
void bplus_meta_destroy(BPlusMeta *meta);

typedef enum {
    BPLUS_BLOCK_TYPE_DATA_BLOCK,
    BPLUS_BLOCK_TYPE_INDEX_BLOCK
} BPlusBlockType;

typedef struct {
    BPlusBlockType block_type;
    int block_num;
    int next_block_num;
    int prev_block_num;
    int parent_block_num;
} BPlusBlockMeta;

BPlusBlockMeta *bplus_block_meta_create(BPlusBlockType block_type,
                                        int block_num, int prev_block_num,
                                        int next_block_num,
                                        int parent_block_num);
void bplus_block_meta_destroy(BPlusBlockMeta *block_meta);

#endif // BPLUS_BPLUS_FILE_STRUCTS_H