#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file_structs.h"
#include "record.h"
#include <assert.h>

#define CALL_BF(call)         \
  {                           \
	BF_ErrorCode code = call; \
	if (code != BF_OK)        \
	{                         \
	  BF_PrintError(code);    \
	  return 0;        \
	}                         \
  }

int HeapFile_Create(const char* file_name)
{

	// Create and open file
	CALL_BF(BF_CreateFile(file_name));
	int file_handle;
	CALL_BF(BF_OpenFile(file_name, &file_handle));

	// Calculate the maximum number of records in a block
	HeapFileHeader header;
	header.block_capacity = (BF_BLOCK_SIZE - sizeof(BlockHeader)) / sizeof(Record);

	// Create header block
	BF_Block* block;
	BF_Block_Init(&block);
	CALL_BF(BF_AllocateBlock(file_handle, block));
	void *data = BF_Block_GetData(block);

	// Populate header block
	memset(data, 0, BF_BLOCK_SIZE);
	memcpy(data, &header, sizeof(HeapFileHeader)); 

	// Clean up
	BF_Block_SetDirty(block);
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);                         
	CALL_BF(BF_CloseFile(file_handle));

	return 1;
}

int HeapFile_Open(const char *fileName, int *file_handle, HeapFileHeader** header_info)
{
	// Open file
	CALL_BF(BF_OpenFile(fileName, file_handle));
	// Get header info
	BF_Block* block;
	BF_Block_Init(&block);
	CALL_BF(BF_GetBlock(*file_handle, HEAPFILE_HEADER_BLOCK, block));
	char* data = BF_Block_GetData(block);
	*header_info = malloc(sizeof(HeapFileHeader));
	memcpy(*header_info, data, sizeof(HeapFileHeader));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);                     
	return 1;
}

int HeapFile_Close(int file_handle, HeapFileHeader* header_info)
{
	// Close file
	free(header_info);
	CALL_BF(BF_CloseFile(file_handle));
	return 1;
}

int HeapFile_InsertRecord(int file_handle, HeapFileHeader *header_info, const Record record)
{
	int blocks_num;
	CALL_BF(BF_GetBlockCounter(file_handle, &blocks_num));

	// If no blocks exist, allocate the first one and insert the record
	if (blocks_num == 1) {
		// Create block
		BF_Block* block;
		BF_Block_Init(&block);
		CALL_BF(BF_AllocateBlock(file_handle, block));

		// Populate block header
		char* block_data = BF_Block_GetData(block);
		memset(block_data, 0, BF_BLOCK_SIZE);
		BlockHeader* block_header = (BlockHeader*)block_data;
		block_header->records_num = 1;

		// Insert record
		Record* records = (Record*)(block_data + RECORD_OFFSET);
		records[0] = record;

		// Clean up
		BF_Block_SetDirty(block);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		return 1;
	}

	// Get the last block to check if it has space
	BF_Block* last_block;
	BF_Block_Init(&last_block);
	CALL_BF(BF_GetBlock(file_handle, blocks_num - 1, last_block));

	char* last_block_data = BF_Block_GetData(last_block);
	BlockHeader* last_block_header = (BlockHeader*)last_block_data;

	if (last_block_header->records_num < header_info->block_capacity) {
		// There is space in the last block — insert record here
		Record* records = (Record*)(last_block_data + RECORD_OFFSET);
		records[last_block_header->records_num] = record;
		last_block_header->records_num++;

		// Clean up
		BF_Block_SetDirty(last_block);
		CALL_BF(BF_UnpinBlock(last_block));
		BF_Block_Destroy(&last_block);
	} else {
		// Last block is full — allocate a new block and insert the record there
		CALL_BF(BF_UnpinBlock(last_block));
		BF_Block_Destroy(&last_block);

		// Create new block
		BF_Block* new_block;
		BF_Block_Init(&new_block);
		CALL_BF(BF_AllocateBlock(file_handle, new_block));

		char* new_block_data = BF_Block_GetData(new_block);
		memset(new_block_data, 0, BF_BLOCK_SIZE);

		// Populate header
		BlockHeader* new_block_header = (BlockHeader*)new_block_data;
		new_block_header->records_num = 1;

		// Insert record
		Record* records = (Record*)(new_block_data + RECORD_OFFSET);
		records[0] = record;

		// Clean up
		BF_Block_SetDirty(new_block);
		CALL_BF(BF_UnpinBlock(new_block));
		BF_Block_Destroy(&new_block);
	}

	return 1;
}

HeapFileIterator HeapFile_CreateIterator(int file_handle, HeapFileHeader* header_info, int search_id)
{
  HeapFileIterator iterator = {
	  .block_index = 1, // Points to the first data block
	  .record_index = 0,
	  .search_id = search_id,
	  .file_handle = file_handle
  };
  return iterator;
}

int HeapFile_GetNextRecord(HeapFileIterator* heap_iterator, Record** record){
    int blocks_num;
    CALL_BF(BF_GetBlockCounter(heap_iterator->file_handle, &blocks_num));

    for(int i = heap_iterator->block_index; i < blocks_num; ++i){
        BF_Block* block;
        BF_Block_Init(&block);
        CALL_BF(BF_GetBlock(heap_iterator->file_handle, i, block));
        char* block_data = BF_Block_GetData(block);
        BlockHeader* block_header = (BlockHeader*)block_data;
        Record* records = (Record*)(block_data + RECORD_OFFSET);

        int j = 0;
		if(i==heap_iterator->block_index) j = heap_iterator->record_index;

        for(; j < block_header->records_num; ++j){
            if(records[j].id == heap_iterator->search_id){

				// Approach 1:
					*record = &records[j];
				// this approach can create a tangling pointer: The pointer is tied to a pinned block, which is unpinned afterwards
				// It works now, because the main loop uses record immediately and doesn't store it.
				// it would be most correct to allocate memory for the record

				// Approach 2:
					// *record = malloc(sizeof(Record));
					// memcpy(*record, &records[j], sizeof(Record));
				// this approach is safer but needs a free(answer) inside the do while loop in main
				
				// With the currect main, approach 2 leaves memory leaks. So we chose approach 1,
				// altough we recognize the potential risks behind it.

                heap_iterator->block_index = i;
                heap_iterator->record_index = j + 1;
                CALL_BF(BF_UnpinBlock(block));
                BF_Block_Destroy(&block);
                return 1;
            }
        }

		heap_iterator->record_index = 0;
        CALL_BF(BF_UnpinBlock(block));
        BF_Block_Destroy(&block);
    }

    *record = NULL;
    return 0;
}