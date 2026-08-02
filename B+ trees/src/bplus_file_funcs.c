#include "bplus_file_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bplus_file_structs.h"

#define CALL_BF(call)                                                          \
    do {                                                                       \
        BF_ErrorCode code = call;                                              \
        if (code != BF_OK) {                                                   \
            BF_PrintError(code);                                               \
            exit(BF_ERROR);                                                    \
        }                                                                      \
    } while (0)

/* --------- BPlusMeta methods --------- */

BPlusMeta *bplus_meta_create(const TableSchema *schema) {
    BPlusMeta *meta = calloc(1, sizeof(BPlusMeta));

    meta->root_block_num = 1; // initially block #1 is the root
    meta->schema = *schema;

    return meta;
}

void bplus_meta_destroy(BPlusMeta *meta) {
    if (!meta)
        return;
    free(meta);
}

/* ---------- BPlusBlockMeta methods ---------- */

BPlusBlockMeta *bplus_block_meta_create(BPlusBlockType block_type,
                                        int block_num, int prev_block_num,
                                        int next_block_num,
                                        int parent_block_num) {
    BPlusBlockMeta *block_meta = calloc(1, sizeof(BPlusBlockMeta));

    // Initialize fields
    block_meta->block_type = block_type;
    block_meta->block_num = block_num;
    block_meta->next_block_num = next_block_num;
    block_meta->prev_block_num = prev_block_num;
    block_meta->parent_block_num = parent_block_num;

    return block_meta;
}

void bplus_block_meta_destroy(BPlusBlockMeta *block_meta) {
    if (!block_meta)
        return;
    free(block_meta);
}

/* ---------- bf_block helpers ---------- */

// Creates a new block in the file with the given file_handle and stores its
// address in *block
void bf_block_create(int file_handle, BF_Block **block) {
    BF_Block_Init(block);
    CALL_BF(BF_AllocateBlock(file_handle, *block));

    void *data = BF_Block_GetData(*block);
    memset(data, 0, BF_BLOCK_SIZE);

    BF_Block_SetDirty(*block);
}

void bf_block_set_data(BF_Block *block, const void *data, int data_size,
                       int offset) {
    char *block_data = BF_Block_GetData(block);
    memcpy(block_data + offset, data, data_size);
    BF_Block_SetDirty(block);
}

// Unpins block and frees memory
void bf_block_cleanup(BF_Block **block) {
    if (!block || !*block)
        return;
    CALL_BF(BF_UnpinBlock(*block));
    BF_Block_Destroy(block);
}

// Gets the block specified by the given block num
void bf_block_get(const int file_handle, int block_num, BF_Block **block) {
    BF_Block_Init(block);
    CALL_BF(BF_GetBlock(file_handle, block_num, *block));
}

/* ---------- BPlus Helper Functions ---------- */

// Returns the block number where the record with the given key is located or
// should be inserted
int bplus_find_data_block_num(const int file_handle, int block_num, int key) {
    // Load current block
    BF_Block *block;
    bf_block_get(file_handle, block_num, &block);
    BPlusBlockMeta *current_block_meta =
        (BPlusBlockMeta *)BF_Block_GetData(block);

    // If the current block is a leaf block (data block), return its number
    if (current_block_meta->block_type == BPLUS_BLOCK_TYPE_DATA_BLOCK) {
        bf_block_cleanup(&block);
        return block_num;
    }

    // Find the correct child block
    BPlusIndexNode *index_node =
        (BPlusIndexNode *)(BF_Block_GetData(block) + BPLUS_BLOCK_NODE_OFFSET);
    int child_block_num =
        bplus_indexnode_get_child_block_num(index_node, key);

    // Continue searching down the tree
    bf_block_cleanup(&block);
    return bplus_find_data_block_num(file_handle, child_block_num, key);
}

void bplus_split_index_block(const int file_handle, BPlusMeta *metadata,
                             int block_num);

void bplus_add_key_to_parent(const int file_handle, BPlusMeta *metadata, int key, int parent_block_num, int left_child_num, int right_child_num){
    // We find the parent block
    BF_Block *parent_block;
    bf_block_get(file_handle, parent_block_num, &parent_block);
    BPlusIndexNode *parent_node = (BPlusIndexNode *)(BF_Block_GetData(parent_block) + BPLUS_BLOCK_NODE_OFFSET);

    // We add the key to it and check for possible splitting
    BPlusAddKeyReturnValue returnvalue = bplus_indexnode_add_key(parent_node, key, left_child_num, right_child_num);
    if(returnvalue == BPLUS_ADD_KEY_RETURN_VALUE_MAX_KEY_NUM_EXCEEDED){
        bplus_split_index_block(file_handle, metadata, parent_block_num);
    }

    // Clean up
    BF_Block_SetDirty(parent_block);
    bf_block_cleanup(&parent_block);
}

void bplus_add_key_to_new_root(const int file_handle, BPlusMeta *metadata, int key, int left_child_num, int right_child_num){
    // create a new index node block
    BF_Block *root_block;
    int root_block_num;
    CALL_BF(BF_GetBlockCounter(file_handle, &root_block_num));
    bf_block_create(file_handle, &root_block);

    // create the new block's metadata
    BPlusBlockMeta *root_meta = bplus_block_meta_create(BPLUS_BLOCK_TYPE_INDEX_BLOCK, root_block_num, -1, -1, -1);
    BPlusIndexNode *root_node = bplus_indexnode_create();

    // Update metadata to point at new root
    BF_Block *meta_block;
    bf_block_get(file_handle, BPLUS_META_BLOCK_NUM, &meta_block);
    metadata->root_block_num = root_block_num;
    BF_Block_SetDirty(meta_block);

    bplus_indexnode_add_key(root_node, key, left_child_num, right_child_num);

    // set the data of the new root block
    bf_block_set_data(root_block, root_meta, sizeof(BPlusBlockMeta), 0);
    bf_block_set_data(root_block, root_node, sizeof(BPlusIndexNode), BPLUS_BLOCK_NODE_OFFSET);

    // We will update the two blocks' parent field to point at this new block
    BF_Block *left_child;
    bf_block_get(file_handle, left_child_num, &left_child);
    BPlusBlockMeta *left_child_meta = (BPlusBlockMeta *)BF_Block_GetData(left_child);
    left_child_meta->parent_block_num = root_block_num;
    
    BF_Block *right_child;
    bf_block_get(file_handle, right_child_num, &right_child);
    BPlusBlockMeta *right_child_meta = (BPlusBlockMeta *)BF_Block_GetData(right_child);
    right_child_meta->parent_block_num = root_block_num;
    
    // Clean up
    BF_Block_SetDirty(left_child);
    BF_Block_SetDirty(right_child);
    bplus_block_meta_destroy(root_meta);
    bplus_indexnode_destroy(root_node);
    BF_Block_Destroy(&meta_block);
    bf_block_cleanup(&root_block);
    bf_block_cleanup(&left_child);
    bf_block_cleanup(&right_child);
}

// Splits an overflowing index block and, if needed, recursively splits parent
// blocks up to the root, or creates a new root
void bplus_split_index_block(const int file_handle, BPlusMeta *metadata, int block_num){
    // We have an index block with 3 keys and 4 pointers
    BF_Block *block;
    bf_block_get(file_handle, block_num, &block);
    BPlusBlockMeta *block_meta = (BPlusBlockMeta *)BF_Block_GetData(block);
    BPlusIndexNode *node = (BPlusIndexNode *)(BF_Block_GetData(block) + BPLUS_BLOCK_NODE_OFFSET);

    // We have to break it into 2 index blocks. Create the new block
    BF_Block *new_block;
    int new_block_num;
    CALL_BF(BF_GetBlockCounter(file_handle, &new_block_num));
    bf_block_create(file_handle, &new_block);

    // Now this new block must have its own metadata and node data
    BPlusBlockMeta *new_block_meta = bplus_block_meta_create(BPLUS_BLOCK_TYPE_INDEX_BLOCK, new_block_num,
        block_meta->block_num, // prev - current block is the left sibling of the new block
        block_meta->next_block_num, // next
        block_meta->parent_block_num // parent
    );
    BPlusIndexNode *new_node = bplus_indexnode_create();

    // We must also update node's next to be newnode
    // We must also update node->next's prev to be newnode
    int old_node_next = block_meta->next_block_num;
    block_meta->next_block_num = new_block_num;

    if(old_node_next != -1){
        BF_Block *old_next_block;
        bf_block_get(file_handle, old_node_next, &old_next_block);
        BPlusBlockMeta *old_next_block_meta = (BPlusBlockMeta *)BF_Block_GetData(old_next_block);
        old_next_block_meta->prev_block_num = new_block_num;
        BF_Block_SetDirty(old_next_block);
        bf_block_cleanup(&old_next_block);
    }

    // Now we are ready to split

    int mid = node->key_count / 2;
    int mid_key = node->keys[mid];

    new_node->next_block_nums[0] = node->next_block_nums[mid+1];

    for(int i = mid+1; i < node->key_count; i++){
        new_node->keys[new_node->key_count] = node->keys[i];
        new_node->next_block_nums[new_node->key_count + 1] = node->next_block_nums[i+1];
        new_node->key_count++;
    }
    node->key_count = mid;  // Shrink old node to left half

    // Update the parent pointers for the children of the new node
    for(int i = 0; i <= new_node->key_count; i++){
        int child_block_num = new_node->next_block_nums[i];

        BF_Block *child_block;
        bf_block_get(file_handle, child_block_num, &child_block);
        BPlusBlockMeta *child_meta = (BPlusBlockMeta *)BF_Block_GetData(child_block);

        child_meta->parent_block_num = new_block_num;

        BF_Block_SetDirty(child_block);
        bf_block_cleanup(&child_block);
    }

    // Now the two nodes are ready
    bf_block_set_data(new_block, new_block_meta, sizeof(BPlusBlockMeta), 0); 
    bf_block_set_data(new_block, new_node, sizeof(BPlusIndexNode), BPLUS_BLOCK_NODE_OFFSET);

    // save parent block num for later (before clean_up!)
    int parent_block_num = block_meta->parent_block_num;

    // SetDirty and Unpin all the blocks I used
    bplus_block_meta_destroy(new_block_meta);
    bplus_indexnode_destroy(new_node);
    BF_Block_SetDirty(block);
    BF_Block_SetDirty(new_block);
    bf_block_cleanup(&block);
    bf_block_cleanup(&new_block);
    
    // We must add the mid_key to the parent index node of the upper level:
    // If it doesn't exist, we create it and we assign the root to be this block
    // If it exists and fits, everything is okay
    // If it exists and doesn't fit, we must split index block and... new splits
    if(parent_block_num == -1){ // parent doesn't exist
        bplus_add_key_to_new_root(file_handle, metadata, mid_key, block_num, new_block_num);
    }
    else{
        bplus_add_key_to_parent(file_handle, metadata, mid_key, parent_block_num, block_num, new_block_num);
    }
}

void bplus_split_data_block(const int file_handle, BPlusMeta *metadata, int block_num){
    // Here, we have a data block with 3 records.
    BF_Block *block;
    bf_block_get(file_handle, block_num, &block);
    BPlusBlockMeta *block_meta = (BPlusBlockMeta *)BF_Block_GetData(block);
    BPlusDataNode *node = (BPlusDataNode *)(BF_Block_GetData(block) + BPLUS_BLOCK_NODE_OFFSET);

    // We have to break it into 2 data blocks. Create the new block
    BF_Block *new_block;
    int new_block_num;
    CALL_BF(BF_GetBlockCounter(file_handle, &new_block_num));
    bf_block_create(file_handle, &new_block);

    // Now this new_block must have its own metadata and node data
    BPlusBlockMeta *new_block_meta = bplus_block_meta_create(BPLUS_BLOCK_TYPE_DATA_BLOCK, new_block_num,
        block_meta->block_num, // prev - current block is the left sibling of the new block
        block_meta->next_block_num, // next
        block_meta->parent_block_num // parent
    );
    BPlusDataNode *new_node = bplus_datanode_create();

    // We must also update node's next to be newnode
    // We must also update node->next's prev to be newnode
    int old_node_next = block_meta->next_block_num;
    block_meta->next_block_num = new_block_num;

    if(old_node_next != -1){
        BF_Block *old_next_block;
        bf_block_get(file_handle, old_node_next, &old_next_block);
        BPlusBlockMeta *old_next_block_meta = (BPlusBlockMeta *)BF_Block_GetData(old_next_block);
        old_next_block_meta->prev_block_num = new_block_num;
        BF_Block_SetDirty(old_next_block);
        bf_block_cleanup(&old_next_block);
    }

    // Now we are ready to split

    int mid = node->record_count / 2;
    int mid_key = record_get_key(&metadata->schema, &node->records[mid]);

    for(int i = mid; i < node->record_count; i++){
        new_node->records[new_node->record_count++] = node->records[i];
    }
    node->record_count = mid;  // Shrink old node to left half

    // Now the two nodes are ready
    bf_block_set_data(new_block, new_block_meta, sizeof(BPlusBlockMeta), 0); 
    bf_block_set_data(new_block, new_node, sizeof(BPlusDataNode), BPLUS_BLOCK_NODE_OFFSET);

    // save parent block num for later (before clean_up!)
    int parent_block_num = block_meta->parent_block_num;

    // SetDirty and Unpin all the blocks I used
    bplus_block_meta_destroy(new_block_meta);
    bplus_datanode_destroy(new_node);
    BF_Block_SetDirty(block);
    BF_Block_SetDirty(new_block);
    bf_block_cleanup(&block);
    bf_block_cleanup(&new_block);

    // We must add the key identifier (mid_key) for the two blocks to the parent index node of the upper level:
    // If it doesn't exist, we create it and we assign the root to be this block
    // If it exists and fits, everything is okay
    // If it exists and doesn't fit, we must split index block and... another drama begins: new splits
    if(parent_block_num == -1){ // parent doesn't exist
        bplus_add_key_to_new_root(file_handle, metadata, mid_key, block_num, new_block_num);
    }
    else{
        bplus_add_key_to_parent(file_handle, metadata, mid_key, parent_block_num, block_num, new_block_num);
    }
}

/* ---------- BPlus Functions ---------- */

int bplus_create_file(const TableSchema *schema, const char *file_name) 
{
    // Create and open file
    CALL_BF(BF_CreateFile(file_name));  
    int file_handle;
    CALL_BF(BF_OpenFile(file_name, &file_handle));  

    // Create metadata block
    BPlusMeta* meta = bplus_meta_create(schema);
    BF_Block* meta_block = NULL;
    bf_block_create(file_handle, &meta_block);
    bf_block_set_data(meta_block, meta, sizeof(BPlusMeta), 0);

    // Create root block
    BPlusBlockMeta* root_block_meta = bplus_block_meta_create(BPLUS_BLOCK_TYPE_DATA_BLOCK, meta->root_block_num, -1, -1, -1);
    BF_Block* root_block = NULL;
    bf_block_create(file_handle, &root_block);
    bf_block_set_data(root_block, root_block_meta, sizeof(BPlusBlockMeta), 0); 

    // We created the root block's metadata, now create the node
    BPlusDataNode* root_node = bplus_datanode_create();
    bf_block_set_data(root_block, root_node, sizeof(BPlusDataNode), BPLUS_BLOCK_NODE_OFFSET);

    // Clean up
    bf_block_cleanup(&meta_block);
    bf_block_cleanup(&root_block);
    
    bplus_datanode_destroy(root_node);
    bplus_block_meta_destroy(root_block_meta);
    bplus_meta_destroy(meta);
    
    CALL_BF(BF_CloseFile(file_handle)); 
    return 0;
}

int bplus_open_file(const char *file_name, int *file_handle, BPlusMeta **metadata)
{
	// Open file
	CALL_BF(BF_OpenFile(file_name, file_handle));

	// Get meta block info
	BF_Block* block;
    bf_block_get(*file_handle, BPLUS_META_BLOCK_NUM, &block);

	BPlusMeta* data = (BPlusMeta*)BF_Block_GetData(block);
	*metadata = data;
	BF_Block_Destroy(&block);

	return 0;
}

int bplus_close_file(const int file_handle, BPlusMeta *metadata)
{
    // Update meta block before closing the file
    BF_Block *meta_block;
    bf_block_get(file_handle, BPLUS_META_BLOCK_NUM, &meta_block);
    BF_Block_SetDirty(meta_block);
    bf_block_cleanup(&meta_block);

	// Close file
	CALL_BF(BF_CloseFile(file_handle));
	return 0;
}

int bplus_record_insert(const int file_handle, BPlusMeta *metadata, const Record *record){
    // first, we find the block where the record should be inserted with bplus_find_data_block_num
    int data_block_num = bplus_find_data_block_num(file_handle, metadata->root_block_num, record_get_key(&metadata->schema, record));

    // We have the num of the block, but we must get its data, to see if it fits
    BF_Block* data_block;
    bf_block_get(file_handle, data_block_num, &data_block);

    BPlusDataNode *data_node = (BPlusDataNode *)(BF_Block_GetData(data_block) + BPLUS_BLOCK_NODE_OFFSET);

    BPlusAddRecordReturnValue returnvalue = bplus_datanode_add_record(data_node, &metadata->schema, record);

    switch(returnvalue){
        case BPLUS_ADD_RECORD_RETURN_VALUE_RECORD_ADDED:
            BF_Block_SetDirty(data_block);
            bf_block_cleanup(&data_block);
            return data_block_num;
        case BPLUS_ADD_RECORD_RETURN_VALUE_DUPLICATE_KEY:
            bf_block_cleanup(&data_block);
            return -1;
        case BPLUS_ADD_RECORD_RETURN_VALUE_MAX_RECORD_NUM_EXCEEDED:
            // we must split the block
            bplus_split_data_block(file_handle, metadata, data_block_num);
            bf_block_cleanup(&data_block);
            // return the block num where the record ended up
            return bplus_find_data_block_num(file_handle, metadata->root_block_num, record_get_key(&metadata->schema, record));
    }
}

int bplus_record_find(const int file_handle, const BPlusMeta *metadata, const int key, Record **out_record){
    int data_block_num = bplus_find_data_block_num(file_handle, metadata->root_block_num, key);

    // We have the num of the block, but we must get its data
    BF_Block *data_block;
    bf_block_get(file_handle, data_block_num, &data_block);

    // Getting the record (or NULL)
    BPlusDataNode *data_node = (BPlusDataNode *)(BF_Block_GetData(data_block) + BPLUS_BLOCK_NODE_OFFSET);

    Record *found = bplus_datanode_find_record(data_node, &metadata->schema, key);

    // if we had assigned the record at out_record like that:
    // *out_record = bplus_datanode_find_record(data_node, &metadata->schema, key);
    // we would have memory leaks

    if(found != NULL){
        if(*out_record == NULL){
            *out_record = malloc(sizeof(Record));
        }
        memcpy(*out_record, found, sizeof(Record));
        bf_block_cleanup(&data_block);
        return 0;
    }

    // not found: free old buffer (avoids stale/garbage prints) and null it
    if(*out_record != NULL){
        free(*out_record);
        *out_record = NULL;
    }

    bf_block_cleanup(&data_block);
    return -1;
}