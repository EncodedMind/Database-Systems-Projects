#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H
/* Στο αντίστοιχο αρχείο .h μπορείτε να δηλώσετε τις συναρτήσεις
 * και τις δομές δεδομένων που σχετίζονται με τους Κόμβους Δεδομένων.*/
#include <stdio.h>
#include <stdlib.h>

#include "record.h"

#define BPLUS_INDEX_NODE_MAX_KEYS 2

typedef struct {
    int keys[BPLUS_INDEX_NODE_MAX_KEYS + 1];
    int next_block_nums[BPLUS_INDEX_NODE_MAX_KEYS + 2];
    int key_count;
} BPlusIndexNode;

typedef enum {
    BPLUS_ADD_KEY_RETURN_VALUE_KEY_ADDED,
    BPLUS_ADD_KEY_RETURN_VALUE_MAX_KEY_NUM_EXCEEDED,
} BPlusAddKeyReturnValue;

BPlusIndexNode *bplus_indexnode_create();
int bplus_indexnode_get_child_block_num(BPlusIndexNode *index_node, int record_key);
BPlusAddKeyReturnValue bplus_indexnode_add_key(BPlusIndexNode *node, int key,
                                               int left_child_num,
                                               int right_child_num);
void bplus_indexnode_destroy(BPlusIndexNode *node);

#endif