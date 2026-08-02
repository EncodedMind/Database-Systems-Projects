// Μπορείτε να προσθέσετε εδώ βοηθητικές συναρτήσεις για την επεξεργασία Κόμβων
// Δεδομένων.
#include "bplus_index_node.h"

BPlusIndexNode *bplus_indexnode_create() {
    BPlusIndexNode *node = calloc(1, sizeof(BPlusIndexNode));
    node->key_count = 0;
    return node;
}

// based on the key, returns the child block num to follow
int bplus_indexnode_get_child_block_num(BPlusIndexNode *index_node, int record_key) {
    int next_block_num = index_node->next_block_nums[0];

    for (int i = index_node->key_count - 1; i >= 0; i--) {
        if (record_key >= index_node->keys[i]) {
            next_block_num = index_node->next_block_nums[i + 1];
            break;
        }
    }

    return next_block_num;
}

BPlusAddKeyReturnValue bplus_indexnode_add_key(BPlusIndexNode *node,
                                               int new_key, int left_child_num,
                                               int right_child_num) {
    
    // If there are no keys, just add the new key at position 0
    if (node->key_count == 0) {
        node->keys[0] = new_key;
        node->next_block_nums[0] = left_child_num;
        node->next_block_nums[1] = right_child_num;
        node->key_count++;

        return BPLUS_ADD_KEY_RETURN_VALUE_KEY_ADDED;
    }

    int new_key_index = node->key_count;

    for (int i = 0; i < node->key_count; i++) {
        if (node->keys[i] > new_key) {
            new_key_index = i;
            break;
        }
    }

    // Shift keys and next_block_nums to make space for the new key
    for (int i = node->key_count - 1; i >= new_key_index; i--) {
        node->keys[i + 1] = node->keys[i];
    }
    for (int i = node->key_count; i >= new_key_index; i--) {
        node->next_block_nums[i + 1] = node->next_block_nums[i];
    }

    // Insert the new key and child pointers
    node->keys[new_key_index] = new_key;
    node->next_block_nums[new_key_index + 1] = right_child_num;
    node->next_block_nums[new_key_index] = left_child_num;

    node->key_count++;

    if (node->key_count > BPLUS_INDEX_NODE_MAX_KEYS) {
        return BPLUS_ADD_KEY_RETURN_VALUE_MAX_KEY_NUM_EXCEEDED;
    }
    return BPLUS_ADD_KEY_RETURN_VALUE_KEY_ADDED;
}

void bplus_indexnode_destroy(BPlusIndexNode *node) { free(node); }