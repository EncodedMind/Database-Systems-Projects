#ifndef BP_DATANODE_H
#define BP_DATANODE_H
/* Στο αντίστοιχο αρχείο .h μπορείτε να δηλώσετε τις συναρτήσεις
 * και τις δομές δεδομένων που σχετίζονται με τους Κόμβους Δεδομένων.*/

#include <stdlib.h>
#include <string.h>

#include "record.h"

#define BPLUS_DATANODE_MAX_RECORDS 2

typedef struct {
    Record records[BPLUS_DATANODE_MAX_RECORDS + 1];
    int record_count;
} BPlusDataNode;

typedef enum {
    BPLUS_ADD_RECORD_RETURN_VALUE_RECORD_ADDED,
    BPLUS_ADD_RECORD_RETURN_VALUE_MAX_RECORD_NUM_EXCEEDED,
    BPLUS_ADD_RECORD_RETURN_VALUE_DUPLICATE_KEY,
} BPlusAddRecordReturnValue;

BPlusDataNode *bplus_datanode_create();
BPlusAddRecordReturnValue bplus_datanode_add_record(BPlusDataNode *node,
                                                    TableSchema *schema,
                                                    const Record *record);
Record *bplus_datanode_find_record(BPlusDataNode *node,
                                   const TableSchema *schema, int key);
void bplus_datanode_destroy(BPlusDataNode *node);

#endif