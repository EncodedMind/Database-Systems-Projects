// Μπορείτε να προσθέσετε εδώ βοηθητικές συναρτήσεις για την επεξεργασία Κόμβων
// toy Ευρετηρίου.
#include "bplus_datanode.h"

#include <stdio.h>

BPlusDataNode *bplus_datanode_create() {
    BPlusDataNode *node = calloc(1, sizeof(BPlusDataNode));
    node->record_count = 0;
    return node;
}

BPlusAddRecordReturnValue bplus_datanode_add_record(BPlusDataNode *node,
                                                    TableSchema *schema,
                                                    const Record *record) {
    int new_record_index = node->record_count;
    int new_record_key = record_get_key(schema, record);

    for (int i = 0; i < node->record_count; i++) {
        int current_record_key = record_get_key(schema, &node->records[i]);

        if (current_record_key == new_record_key) {
            return BPLUS_ADD_RECORD_RETURN_VALUE_DUPLICATE_KEY;
        }

        if (current_record_key > new_record_key) {
            new_record_index = i;
            break;
        }
    }

    for (int i = node->record_count - 1; i >= new_record_index; i--) {
        node->records[i + 1] = node->records[i];
    }

    node->records[new_record_index] = *record;
    node->record_count++;
    if (node->record_count > BPLUS_DATANODE_MAX_RECORDS) {
        return BPLUS_ADD_RECORD_RETURN_VALUE_MAX_RECORD_NUM_EXCEEDED;
    }
    return BPLUS_ADD_RECORD_RETURN_VALUE_RECORD_ADDED;
}

Record *bplus_datanode_find_record(BPlusDataNode *node,
                                   const TableSchema *schema, int key) {
    for (int i = 0; i < node->record_count; i++) {
        if (record_get_key(schema, &node->records[i]) == key) {
            return &node->records[i];
        }
    }
    return NULL;
}

void bplus_datanode_destroy(BPlusDataNode *node) { free(node); }