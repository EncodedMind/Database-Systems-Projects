#include "chunk.h"
#include <stdio.h>

CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator it;
    it.file_desc = fileDesc;
    it.current = 1; // block 0 is used for the metadata
    it.lastBlocksID = HP_GetIdOfLastBlock(fileDesc);
    it.blocksInChunk = blocksInChunk;
    return it;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    if (iterator->current > iterator->lastBlocksID) {
        return -1; // no more chunks
    }

    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;

    // Adjust the to_BlockId if it exceeds the last block ID
    if (chunk->to_BlockId > iterator->lastBlocksID) {
        chunk->to_BlockId = iterator->lastBlocksID;
    }

    // Calculate the number of blocks in the chunk
    chunk->blocksInChunk = chunk->to_BlockId - chunk->from_BlockId + 1;

    // Iterate over the blocks of the chunk to calculate the number of records in the chunk
    chunk->recordsInChunk = 0;
    for (int b = chunk->from_BlockId; b <= chunk->to_BlockId; b++) {
        chunk->recordsInChunk += HP_GetRecordCounter(chunk->file_desc, b);
    }

    // Advance the iterator
    iterator->current = chunk->to_BlockId + 1;
    return 0;
}

int CHUNK_GetIthRecordInChunk(CHUNK *chunk, int i, Record *record) {
    // Input check
    if (i < 0 || i >= chunk->recordsInChunk) return -1;

    int count = 0;
    // Iterate over the blocks of the chunk
    for (int b = chunk->from_BlockId; b <= chunk->to_BlockId; b++) {
        int recs = HP_GetRecordCounter(chunk->file_desc, b);
        // Check if the i-th record of the chunk belongs to the current block
        if (count + recs > i) {
            int cursor = i - count; // Get the index of the i-th record in the current block
            // Store the result in `record` and return 0 if the operation is successful and -1 otherwise
            return HP_GetRecord(chunk->file_desc, b, cursor, record);   
        }
        count += recs;
    }
    return -1;
}

int CHUNK_UpdateIthRecord(CHUNK *chunk, int i, Record record) {
    // Input check
    if (i < 0 || i >= chunk->recordsInChunk) return -1;

    int count = 0;
    // Iterate over the blocks of the chunk
    for (int b = chunk->from_BlockId; b <= chunk->to_BlockId; b++) {
        int recs = HP_GetRecordCounter(chunk->file_desc, b);
        // Check if the i-th record of the chunk belongs to the current block
        if (count + recs > i) {
            int cursor = i - count; // Get the index of the i-th record in the current block
            return HP_UpdateRecord(chunk->file_desc, b, cursor, record);
        }
        count += recs;
    }
    return -1;
}

void CHUNK_Print(CHUNK chunk){
    for(int bId = chunk.from_BlockId; bId <= chunk.to_BlockId; bId++){
        HP_PrintBlockEntries(chunk.file_desc, bId);
    }
}

CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator it;
    it.chunk = *chunk;
    it.currentBlockId = chunk->from_BlockId;
    it.cursor = 0;
    return it;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record *record) {
    // Input check
    if (iterator == NULL || record == NULL) return -1;

    // Iterate over the blocks of the chunk after the iterator's current position
    while (iterator->currentBlockId <= iterator->chunk.to_BlockId) {
        // Get number of records in the current block
        int recs = HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId);
        // Check if the cursor is within the bounds of the current block
        if (iterator->cursor < recs) {
            // Get the record and advance the iterator
            int res = HP_GetRecord(iterator->chunk.file_desc,
                                   iterator->currentBlockId,
                                   iterator->cursor,
                                   record);
                                   iterator->cursor++;
                                   return res;
        } else {
            // Unpin current block
            HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);  // ChatGPT did not unpin block, we added this
            // Move to the next block of the chunk
            iterator->currentBlockId++;
            iterator->cursor = 0;
        }
    }
    return -1;
}
