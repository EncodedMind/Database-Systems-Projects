#include "sort.h"

bool shouldSwap(Record *rec1, Record *rec2) {
    // Input check
    if (rec1 == NULL || rec2 == NULL) return false;

    // Compare first names
    int cmpName = strcmp(rec1->name, rec2->name);
    if (cmpName > 0) return true;
    if (cmpName < 0) return false;

    // cmpName == 0 -> first names are equal -> compare surnames
    int cmpSurname = strcmp(rec1->surname, rec2->surname);
    if (cmpSurname > 0) return true;

    return false;
}

void sort_Chunk(CHUNK *chunk) {
    // Input check
    if (chunk == NULL || chunk->recordsInChunk <= 1) return;

    Record rec1, rec2;

    // Simple in-place bubble sort
    for (int i = 0; i < chunk->recordsInChunk - 1; i++) {
        for (int j = 0; j < chunk->recordsInChunk - i - 1; j++) {
            if (CHUNK_GetIthRecordInChunk(chunk, j, &rec1) != 0) continue;
            if (CHUNK_GetIthRecordInChunk(chunk, j + 1, &rec2) != 0) continue;

            if (shouldSwap(&rec1, &rec2)) {
                CHUNK_UpdateIthRecord(chunk, j, rec2);
                CHUNK_UpdateIthRecord(chunk, j + 1, rec1);
            }
        }
    }
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    if (file_desc < 0 || numBlocksInChunk <= 0) return;

    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK chunk;

    while (CHUNK_GetNext(&iterator, &chunk) == 0) {
        // ChatGPT: if (chunk.recordsInChunk > 1) { // unnecessary check, it is performed in sort_Chunk
            sort_Chunk(&chunk);
        // }

        for (int id = chunk.from_BlockId; id <= chunk.to_BlockId; id++) {
            HP_Unpin(file_desc, id);    // ChatGPT: did not unpin blocks, we added this
        }
    }
}
